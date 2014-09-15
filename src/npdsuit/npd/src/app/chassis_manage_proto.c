
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* chassis_manage_proto.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used for chassis manage.
*
* DATE:
*		03/16/2010	
*06/11/2010              wuh@autelan.com                 Fix bugs.
*09/21/2010              wuh@autelan.com                 Add SWITCHOVER status.
*01/25/2011		         wuhao@autelan.com		         Fix bugs: 
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "board/ts_product_feature.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "circle.h"
#include "chasm_manage_proto.h"
#include "auteware_tail.h"

board_type_t *board_type_head=NULL;
auteware_tail_t *image_tail_head=NULL;



typedef struct chasm_circle_data_s
{
    int sock;
    int flag;
    board_param_t *board;
	int timer_evt;   /*Timer消息附带的event消息*/
	void *timer_priv; /*Timer消息附带的私有数据*/
}chasm_circle_data_t;

struct chasm_circle_data_s *chasm_circle_data;

char ** module_type_str = NULL;

pthread_mutex_t chasm_mutex = PTHREAD_MUTEX_INITIALIZER;

int chasm_debug_pkt = 0;  

/* file to save system startup state */
#define NPD_SYSTEM_STARTUP_STATE_PATH	"/var/run/aw.state"

#define NOT_RESET		0
#define HAS_RESET		!NOT_RESET

int chasm_set_product_reset_state(int state);
int chasm_get_product_reset_state(int * state);


int g_reset_product_state = NOT_RESET;

extern int npd_dbtable_thread_main();
extern int npd_dbtable_slot_sync_begin(int slot_index);
extern int nbm_get_boot_img_name(char* imgname);
extern int npd_dbtable_sync_alldone();
extern int npd_dbtable_slot_sync_done(int slot_index); 
extern int property_file_read(char *filename,char *property);
extern int npd_dbtable_init();
extern int nam_chassis_info_notify(int chassis_type, int slotnum, 
	int actmaster_slot, int sdymaster_slot, int local_slot, int state);
extern int nam_board_info_notify(unsigned int slot_index, 
	int board_type, int run_state, int work_mode, int online_remove);
	
int npd_get_sw_type_by_board_type(char **sw_type, char *board_type);
int npd_get_image_info(char *name,char *version,unsigned int *build_time);
int npd_board_info_notify(unsigned int slot_index);
int npd_chassis_board_info_notify(unsigned int chassisId);
unsigned int chasm_local_check(unsigned int slot_no);
long chasm_config_board(int slot_id, int subslot_id, int board_type);
long chasm_no_board(int slot_id, int subslot_id);
long chasm_switchover(int slot_index);
void chasm_lock();
void chasm_unlock();
void chasm_timeout(void *circle_data, void *user_data);
long chasm_isstandby(int slot_index);


/* ************* For D-Bus ************************* */

int npd_reset_module_info(struct module_info_s * module_info)
{
	memset(module_info->modname, 0, 32);
	memset(module_info->sn, 0, 32);

	(*localmoduleinfo->fix_spec_param->board_man_param_get)(&(localmoduleinfo->man_param));
	return NPD_TRUE;
}

int npd_dbus_get_config_type(char * type_name, int * config_type)
{
	
	int type_index ;

	for(type_index = PPAL_BOARD_TYPE_NONE; type_index < PPAL_BOARD_TYPE_MAX; type_index++)
	{
		if(module_type_str[type_index] == NULL)
			continue;

		//npd_syslog_dbg("current type name is %s.\n", module_type_str);
			
		if (!strcasecmp(type_name, module_type_str[type_index]))
		{
			npd_syslog_dbg("Board type ID: %d, type name is %s.\n", type_index, module_type_str[type_index]);
			*config_type = type_index;
			return BOARD_RETURN_CODE_ERR_NONE;
		}
	}
	
	return BOARD_RETURN_CODE_NO_SUCH_TYPE;

}

int npd_get_board_type_name(int config_type, char * type_name)
{
	if(config_type >= PPAL_BOARD_TYPE_MAX || config_type <= PPAL_BOARD_TYPE_NONE)
	{
		return BOARD_RETURN_CODE_NO_SUCH_TYPE;
	}
	//strncpy(type_name, module_basic_info[config_type]->short_name, 24);
	if(NULL == module_type_str[config_type])
		memset(type_name, 0, 24);
	else
	    strncpy(type_name, module_type_str[config_type], 24);
	return BOARD_RETURN_CODE_ERR_NONE;

}

DBusMessage * npd_dbus_boardmng_show_board_attr(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int slot_no = 0;
	unsigned int slot_index = 0;
	int runstate = 0;

	unsigned int inserted = 0;
	unsigned int online_removed = 0;
	int workmode = 0;
	int redundancystate = 0;
	unsigned char hw_ver = 0;
	char *sn = "";
	char *modname = "";

	unsigned long configure_type = 0;
	unsigned long led_status = 0;

	int board_code = 0;
	int board_type = 0;
	char *full_name = "";
	char *short_name = "";
	
	unsigned int sw_version = 0;
	char *sw_type = "";

	unsigned int ams_type = 0;
	

	unsigned int ret = 0;
	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID)))
	{
		syslog_ax_product_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			syslog_ax_product_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
	    }
	    return NULL;
	}

	ret = chasm_local_check(slot_no);

	if(BOARD_RETURN_CODE_ERR_NONE == ret )
	{
		board_param_t *board;
		slot_index = SYS_CHASSIS_SLOT_NO2INDEX(slot_no);

		if((PRODUCT_IS_BOX) && (slot_index > 0))
		{
			board = localmoduleinfo->sub_board[slot_index];
			runstate = localmoduleinfo->sub_board[slot_index]->runstate;
		}
		else
		{
			board = chassis_slots[slot_index];
    		if(slot_index != localmoduleinfo->slot_index)
    		{
    			runstate = MODULE_STATUS_ON_SLOT_INDEX(slot_index);
    		}
    		else
    		{
    			runstate = localmoduleinfo->rmtstate[localmoduleinfo->slot_index];
    		}
		}
		
		syslog_ax_product_dbg("Slot %d, runstate is %d.\n", slot_index, runstate);
        chasm_lock();
		inserted = board->inserted;
		online_removed = board->online_removed;
		workmode = board->workmode;
		redundancystate = board->redundancystate;
		hw_ver = board->man_param.hw_version;
		sn = board->man_param.sn;
		modname = board->man_param.modname;
		if((strlen(modname) == 0) && board->fix_param)
			modname = board->fix_param->short_name;
		
		ams_type = ASM_TYPE_ON_SLOT_INDEX(slot_index);

		configure_type = CONFIG_TYPE_ON_SLOT_INDEX(slot_index);
		led_status = LED_STATUS_ON_SLOT_INDEX(slot_index);

		board_code = BOARD_CODE_ON_SLOT_INDEX(slot_index);
		board_type = MODULE_TYPE_ON_SLOT_INDEX(slot_index);
        if(runstate == RMT_BOARD_NOEXIST)
        {
            if(online_removed == FALSE)
            {
                board_type = PPAL_BOARD_TYPE_NONE;
                modname = "-";
            }
        }

		full_name = FULL_NAME_ON_SLOT_INDEX(slot_index);
		short_name = SHORT_NAME_ON_SLOT_INDEX(slot_index);

		/* Added for show sw_version */
		sw_version = npd_query_sw_version(SYS_LOCAL_MODULE_TYPE);

		if(!strcmp("", short_name))
		{
			sw_type = "-";
		}
		else
		{
			if(npd_get_sw_type_by_board_type(&sw_type, short_name))
			{
				sw_type = "-";
			}
		}
		chasm_unlock();

	}

	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &runstate);


	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inserted);


	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &online_removed);

	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &workmode);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &redundancystate);


	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &hw_ver);


	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &sn);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &modname);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT64, 
									 &configure_type);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT64, 
									 &led_status);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &board_code);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &board_type);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &ams_type);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &full_name);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &short_name);

	dbus_message_iter_append_basic (&iter,
								 	 DBUS_TYPE_UINT32,
								 	 &sw_version);

	dbus_message_iter_append_basic (&iter,
								 	 DBUS_TYPE_STRING,
								 	 &sw_type);

	return reply;
	
}

DBusMessage * npd_dbus_boardmng_get_next_slot(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret = 0;
	unsigned int slot_no = 0;

	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


    npd_syslog_dbg("Get the next slot of slot %d\n", slot_no);

	slot_no++;

	ret = chasm_local_check(slot_no);

	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &slot_no);

	return reply;
}


DBusMessage * npd_dbus_boardmng_config_slot_type(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter	 iter;
	unsigned int slot_no = 0;
	char *  type = 0;
	int board_type = 0;
	unsigned int slot_index = 0;
	unsigned int opt_ret = BOARD_RETURN_CODE_ERR_NONE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_STRING,&type,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    npd_syslog_dbg("Config slot %d as board type: %s\n", slot_no, type);

	opt_ret = npd_dbus_get_config_type(type, &board_type);
	if(BOARD_RETURN_CODE_ERR_NONE != opt_ret)
	{
		goto retcode;
	}
    slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);

	opt_ret = chasm_local_check(slot_no);
	if (BOARD_RETURN_CODE_ERR_NONE != opt_ret)
	{
		/*对盒式交换机原有的配置做升级兼容*/
		if(PRODUCT_IS_BOX)
		{
			if(slot_index > 0)
			{
				/*为了向前兼容*/
				if((slot_index == 1) && (board_type == localmoduleinfo->fix_param->board_type))
					;
				else
					goto retcode;
			}
			else
				goto retcode;
		}
		else		
		    goto retcode;
	}

	{
		if(PRODUCT_IS_BOX)
		{
			if(slot_index > 0)
			{
				/*为了向前兼容*/
				if((slot_index == 1) && (board_type == localmoduleinfo->fix_param->board_type))
					opt_ret = chasm_config_board(0, 0, board_type);
				else
				    opt_ret = chasm_config_board(0, slot_index, board_type);
			}
			else
				opt_ret = chasm_config_board(0, 0, board_type);
#if 0			
            for(slot_num = 0; 
                slot_num < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; 
                slot_num ++)
            {
        		config_ret = chasm_config_board( slot_index, slot_num,  board_type);
        		if (NPD_SUCCESS != config_ret)
        		{
        			npd_syslog_dbg("config_ret is not NPD_SUCCESS\n");
        			opt_ret = BOARD_RETURN_CODE_CONFIG_ERROR;
        		}
            }
#endif				
		}
		else
		{
			opt_ret = chasm_config_board(slot_index, 0, board_type);
		}
	}

retcode:	
	
	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
	return reply;
}


DBusMessage * npd_dbus_boardmng_no_config_slot(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter	 iter;
	unsigned int slot_no = 0;
	unsigned int slot_index = 0;
	unsigned int opt_ret = BOARD_RETURN_CODE_ERR_NONE;
	long config_ret = 0 ;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


    npd_syslog_dbg("Remove pre-config of slot %d.\n", slot_no);

	opt_ret = chasm_local_check(slot_no);
	if(BOARD_RETURN_CODE_ERR_NONE == opt_ret)
	{
		slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);
        if(PRODUCT_IS_BOX)
        {
			if(slot_index > 0)
				opt_ret = chasm_no_board(0, slot_index);
			else
				opt_ret = chasm_no_board(0, 0);
        }
		else
		    opt_ret = chasm_no_board( slot_index, 0);
		
		if (NPD_SUCCESS != config_ret)
		{
			npd_syslog_dbg("config_ret is not NPD_SUCCESS\n");
			opt_ret = BOARD_RETURN_CODE_CONFIG_ERROR;
		}
	}
	
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
	

	return reply;
}

DBusMessage * npd_dbus_boardmng_board_switchover(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter	 iter;
	unsigned int slot_no = 0;
	unsigned int slot_index = 0;
	unsigned int opt_ret = BOARD_RETURN_CODE_ERR_NONE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    npd_syslog_dbg("Switch master board to slot %d.\n", slot_no);

	opt_ret = chasm_local_check(slot_no);
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	if(BOARD_RETURN_CODE_ERR_NONE == opt_ret)
	{
		slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);

		opt_ret = chasm_switchover(slot_index);
	}
#endif	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
	
	return reply;
}

DBusMessage * npd_dbus_boardmng_board_isstandby(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter	 iter;
	unsigned int slot_no = 0;
	unsigned int slot_index = 0;
	unsigned int opt_ret = BOARD_RETURN_CODE_ERR_NONE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    npd_syslog_dbg("Switch master board to slot %d.\n", slot_no);

	opt_ret = chasm_local_check(slot_no);
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)		
	if(BOARD_RETURN_CODE_ERR_NONE == opt_ret)
	{
		slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);

		opt_ret = chasm_isstandby(slot_index);
	}
#endif	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
	
	return reply;
}

DBusMessage * npd_dbus_boardmng_board_reboot(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;

	DBusMessageIter	 iter;
	unsigned int slot_no = 0;
	unsigned int slot_index = 0;
	unsigned int opt_ret = BOARD_RETURN_CODE_ERR_NONE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&slot_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}


    npd_syslog_dbg("Reboot slot %d.\n", slot_no);

	if(-1 == slot_no)
	{
        slot_index = SYS_LOCAL_MODULE_SLOT_INDEX;
		if(chassis_sys_reset_ext)
    	    (*chassis_sys_reset_ext)(slot_index);
	}
    else if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        slot_index = slot_no -1;
        if(chassis_sys_reset_ext)
    	    (*chassis_sys_reset_ext)(slot_index);
            
    }
    else
    {
        opt_ret = BOARD_RETURN_CODE_NOT_ACTMASTER;
    }
	
	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
	
	return reply;
}


DBusMessage * npd_dbus_boardmng_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusMessageIter	 iter;

	char *showStr = NULL;
	int totalLen = 0;
    int slot_no = 0;
	int board_type = 0;
	char board_type_name[24];
	unsigned int slot_index = 0;
	unsigned int ret = BOARD_RETURN_CODE_ERR_NONE;
	showStr = (char*)malloc(4096);
	if(NULL == showStr)
	{
		npd_syslog_dbg("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,4096);
	slot_no = CHASSIS_SLOT_INDEX2NO(slot_no);
    while(chasm_local_check(slot_no) == BOARD_RETURN_CODE_ERR_NONE)
    {
		slot_index = SYS_CHASSIS_SLOT_NO2INDEX(slot_no);
		board_type = MODULE_TYPE_ON_SLOT_INDEX(slot_index);
		memset(board_type_name, 0, 24);
		ret = npd_get_board_type_name(board_type, board_type_name);
		if(ret == BOARD_RETURN_CODE_ERR_NONE)
		{
			totalLen += sprintf(showStr + totalLen, "board %d %s\r\n", slot_no, board_type_name);
		}
		slot_no++;
    }
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}

DBusMessage * npd_dbus_boardmng_board_range(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusMessageIter	 iter;

	unsigned int chassis_slotnum = SYS_CHASSIS_SLOTNUM;
	char *board_type_str = NULL;
	int totalLen = 0;
	int board_type = 0;
	board_type_str = (char*)malloc(4096);
	if(NULL == board_type_str)
	{
		npd_syslog_dbg("memory malloc error\n");
		return NULL;
	}
	
	memset(board_type_str,0,4096);
	
	for(board_type = PPAL_BOARD_TYPE_NONE; board_type < PPAL_BOARD_TYPE_MAX; board_type++)
	{
		if(module_basic_info[board_type] == NULL)
			continue;

		totalLen += sprintf(board_type_str + totalLen, 
			"%s|", module_basic_info[board_type]->short_name);
	}
	
	//remove unwanted '!';
	board_type_str[totalLen-1] = '\0'; 
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&chassis_slotnum);

	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_STRING,
									&board_type_str);
	
	npd_syslog_dbg("board type string is %s\n",board_type_str);

	free(board_type_str);
	board_type_str = NULL;
	return reply;

}


/* ******************for D-BUS************************* */
unsigned int chasm_local_check(unsigned int slot_no)
{
	if(CHASSIS_SLOTNO_ISLEGAL(slot_no))
	{
		return BOARD_RETURN_CODE_ERR_NONE;
	}

	return BOARD_RETURN_CODE_NO_SUCH_SLOT;
}


void chasm_lock()
{
    pthread_mutex_lock(&chasm_mutex);
}

void chasm_unlock()
{
    pthread_mutex_unlock(&chasm_mutex);
}

void chasm_write_board_type_string()
{
    int i, ret;
	int str_len;
	char buf[512];
	unsigned short buf_len=0;
	int is_box=NPD_TRUE;
	char board_file_name[64];
	char board_type_name[64];

	is_box = PRODUCT_IS_BOX;
		
	if (module_type_str != NULL)
		return ;
	memset(buf, 0, 512);
	
	module_type_str = (char **)malloc(sizeof(char *)*PPAL_BOARD_TYPE_MAX);
	if (module_type_str == NULL)
	{
		npd_syslog_err("No Resource to alloc for module_type_str\n");
		return ;
	}
	memset(module_type_str, 0, sizeof(char *)*PPAL_BOARD_TYPE_MAX);
#if 0
	if (is_box)
	{
		ret = -1;
		memset(board_file_name, 0, 64);
		memset(board_type_name, 0, 64);
		i = SYS_LOCAL_MODULE_TYPE;
		str_len = strlen(module_basic_info[i]->short_name);
		if(str_len < 24)
		{
		    sprintf(board_file_name, "/devinfo/%s", module_basic_info[i]->short_name);
			ret = property_file_read(board_file_name, board_type_name);
			if(ret == 0)
			{
				int j = 0;
				str_len = strlen(board_type_name);
				for(j = 0; j < str_len; j++)
				{
					if(board_type_name[j] == '\r' || board_type_name[j] == '\n')
					{
						board_type_name[j] = 0;
					}
				}
			}
		}
	    npd_syslog_dbg("Pizza-box system: %s.\n", module_basic_info[i]->short_name);
		module_type_str[SYS_LOCAL_MODULE_TYPE] = (char *)malloc(sizeof(char)*(str_len+1));
		if (module_type_str == NULL)
		{
			npd_syslog_err("No Resource to alloc for board type string %s\n", 
				module_basic_info[SYS_LOCAL_MODULE_TYPE]->short_name);
			return ;
		}
		if(ret == 0)
		{
		    memcpy(module_type_str[SYS_LOCAL_MODULE_TYPE], board_type_name, str_len+1);
		}
		else
		{
		    memcpy(module_type_str[SYS_LOCAL_MODULE_TYPE], module_basic_info[i]->short_name, str_len+1);
		}
		if(strlen(module_type_str[SYS_LOCAL_MODULE_TYPE]))
		{
		    buf_len += sprintf(buf+buf_len, "%s/", module_type_str[SYS_LOCAL_MODULE_TYPE]);
		}
	}
	else
#endif		
	{
		for(i = 0; i < PPAL_BOARD_TYPE_MAX; i ++)
	    {
			if(module_basic_info[i])
			{
				ret = -1;
        		memset(board_file_name, 0, 64);
        		memset(board_type_name, 0, 64);
				str_len = strlen(module_basic_info[i]->short_name);
        		if(str_len < 24)
        		{
        		    sprintf(board_file_name, "/devinfo/%s", module_basic_info[i]->short_name);
        			ret = property_file_read(board_file_name, board_type_name);
        			if(ret == 0)
        			{
						int j = 0;
        				str_len = strlen(board_type_name);
						for(j = 0; j < str_len; j++)
						{
							if(board_type_name[j] == '\r' || board_type_name[j] == '\n')
							{
								board_type_name[j] = 0;
							}
						}
        			}
        		}
				module_type_str[i] = (char *)malloc(sizeof(char)*(str_len+1));
				if (module_type_str[i] == NULL)
				{
					npd_syslog_err("No Resource to alloc for board type string %s\n", 
						module_basic_info[i]->short_name);
					return ;
				}
				if(ret == 0)
				{
				    memcpy(module_type_str[i], board_type_name, str_len+1);
				}
				else
				{
				    memcpy(module_type_str[i], module_basic_info[i]->short_name, str_len+1);
				}
		        if(strlen(module_type_str[i]))
		        {
				    buf_len += sprintf(buf+buf_len, "%s/", module_type_str[i]);
		        }
			}                
	    }
	}
	buf[buf_len-1]='\0';
	npd_syslog_dbg("Chassis board type string is %s.\n", buf);
	write_to_file("/var/run/boardtype.string", buf, strlen(buf));
}

long chasm_write_board_info()
{
	char buf[100];
	int i;
	int slot_index;
	int master_num;

    if(PRODUCT_IS_BOX)
    {
        memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", SYS_LOCAL_MODULE_SUBSLOTNUM);
    	write_to_file(CHASSIS_MAN_SLOT_NUM_FILE, buf, strlen(buf));

		memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", (int)SYS_MASTER_ACTIVE_SLOT_INDEX);
    	write_to_file(CHASSIS_MAN_ACTMASTER_SLOT_FILE, buf, strlen(buf));
    }
	else
	{
        memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", SYS_CHASSIS_SLOTNUM);
    	write_to_file(CHASSIS_MAN_SLOT_NUM_FILE, buf, strlen(buf));
		
    	memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", (int)SYS_MASTER_ACTIVE_SLOT_INDEX+1);
    	write_to_file(CHASSIS_MAN_ACTMASTER_SLOT_FILE, buf, strlen(buf));
	}
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", SYS_LOCAL_MODULE_ISMASTERACTIVE); 
	write_to_file(CHASSIS_MAN_ACTMASTER_STATE_FILE, buf, strlen(buf));

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", SYS_LOCAL_MODULE_WORKMODE_ISMASTER);
	write_to_file(CHASSIS_MAN_MASTER_STATE_FILE, buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", PRODUCT_IS_BOX);
    write_to_file(CHASSIS_MAN_BOX_STATE_FILE, buf, strlen(buf));

    if(!PRODUCT_IS_BOX)
    {
    	memset(buf, 0, sizeof(buf));
    	master_num = SYS_CHASSIS_MASTER_SLOTNUM;
    	for (i = 0; i < master_num; i++)
    	{
    		slot_index = SYS_CHASSIS_MASTER_SLOT_INDEX(i); 
    		if(SYS_MODULE_ISMASTERSTANDBY(slot_index))
    		{
    			sprintf(buf+strlen(buf), "%d", slot_index+1);
    		}
    	}
    	write_to_file(CHASSIS_MAN_SBYMASTER_SLOT_FILE, buf, strlen(buf));
    }

	if (!PRODUCT_IS_BOX)
	{
	    memset(buf, 0, sizeof(buf));
	    sprintf(buf, "%d", NPD_BOARD_CTRL_NUM);
	    write_to_file("/var/run/ctrl.num", buf, strlen(buf));

	    memset(buf, 0, sizeof(buf));
	    sprintf(buf, "%d", NPD_BOARD_CTRL_SWITCH);
	    write_to_file("/var/run/ctrl.switch", buf, strlen(buf));
	}
	if(PRODUCT_IS_BOX)
	{
    	memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", SYS_LOCAL_MODULE_SLOT_INDEX);
    	write_to_file(CHASSIS_MAN_SLOT_NO_FILE, buf, strlen(buf));
	}
	else
	{
    	memset(buf, 0, sizeof(buf));
    	sprintf(buf, "%d", SYS_LOCAL_MODULE_SLOT_INDEX+1);
    	write_to_file(CHASSIS_MAN_SLOT_NO_FILE, buf, strlen(buf));
	}
	memset(buf,0,sizeof(buf));
	sprintf(buf, "%.2x%.2x%.2x%.2x%.2x%.2x", SYS_PRODUCT_BASEMAC[0],SYS_PRODUCT_BASEMAC[1],SYS_PRODUCT_BASEMAC[2],
					SYS_PRODUCT_BASEMAC[3],SYS_PRODUCT_BASEMAC[4],SYS_PRODUCT_BASEMAC[5]);
	write_to_file("/devinfo/mac", buf, strlen(buf));

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE
        && !SYS_MODULE_SLOT_ISMCU(SYS_LOCAL_MODULE_SLOT_INDEX))
    {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", 1);
        write_to_file(CHASSIS_MAN_SLAVE_RUNNING_INDPNT_FILE, buf, strlen(buf));
    }
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		system("ifconfig mng1 up");
		system("ifconfig mng2 up");
	}
	else
	{
		/*system("ifconfig mng1 down");*/
		/*system("ifconfig mng2 down");*/
	}
	return NPD_SUCCESS;
}

int chasm_board_support_product(int board_type, int product_type, int slot_id, int subslot_id)
{
    int i;
	int ret = FALSE;
    for(i = 0; ;i++)
    {
        int temp_ptype;
        temp_ptype = 
            module_basic_info[board_type]->board_support_product[i];
        if(temp_ptype == product_type)
        {
            ret = TRUE;
			break;
        }
        else if(PRODUCT_DUMMY == temp_ptype)
            return FALSE;
    }
	
    /*检查板子是否支持某个槽位*/
	if(module_basic_info[board_type]->board_support_slot)
	{
		ret = (*module_basic_info[board_type]->board_support_slot)(product_type, slot_id, subslot_id);
	}

    return ret;
}
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
int chasm_switchover_event()
{
    npd_syslog_dbg("Switch over app event notify.\n");
    netif_app_notify_event(NPD_GLOBAL_NETIF_INDEX, NOTIFIER_SWITCHOVER, NULL, 0);
	return NPD_SUCCESS;
}
#endif
long chasm_wait_startup_config_done()
{
	int aw_state_fd = -1;
	char buf[4] = {0};
	int ret = 0;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	do
	{
	  if(aw_state_fd < 0) 
	  {
		  aw_state_fd = open("/var/run/aw.state",O_RDONLY);
		  if(aw_state_fd < 0) 
		  {
			syslog_ax_product_err("open product startup state file %s error\n","/var/run/aw.state");
			close(aw_state_fd);
			aw_state_fd = -1;
		  }
		  
	  }
	  else 
	  {	  
		  ret = read(aw_state_fd,buf,1);
		  if((ret > 0) && ('1' == buf[0]))
		  {
			close(aw_state_fd);
			aw_state_fd = -1;

		  	break;
		  }
		  else
		  {
			close(aw_state_fd);
			aw_state_fd = -1;
		  }
	  }

	  sleep(1);
	}while(1);

	return NPD_SUCCESS;
}

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
long chasm_pdu_print(char* pdu, int len)
{

	int i;
	char bufstring[100] = {0};
	
    for(i = 0; i < len; i++)
    {
		if(i%16 == 0)
		{
			npd_syslog_dbg(bufstring);
			npd_syslog_dbg("\r\n");
			memset(bufstring, 0, sizeof(bufstring));
		}
        sprintf(bufstring+strlen(bufstring), "%.02x ", (unsigned char)pdu[i]);
    }

	npd_syslog_dbg(bufstring);
	npd_syslog_dbg("\r\n");

	return NPD_SUCCESS;
	
}


unsigned long chasm_protocol_version(void)
{
    return CHASM_PROTOCOL_VERSION;
}

#endif


/* pangxiaofeng functions */

void npd_release_board_type_list()
{
    board_type_t *free_entry=board_type_head;
    board_type_t *tmp=board_type_head;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    while(tmp)
    {
        free_entry=tmp;
        tmp=tmp->next;
        free(free_entry);
    }
}

void npd_release_auteware_tail_list()
{
    auteware_tail_t *tmp=image_tail_head;
    auteware_tail_t *free_entry=image_tail_head;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    while(tmp)
    {
        free_entry=tmp;
        tmp=tmp->next;
        free(free_entry);
    }
}


static inline unsigned int npd_file_split_swap32(unsigned int x)
{
    return ((unsigned int)((((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) |
                       (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) |
                       (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) |
                       (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ));
}

int npd_get_image_auteware_tail(char *file_name,auteware_img_pack_tail *tail)
{
    char tmp[256]={0};
    int ret=-1;
    FILE *fp=NULL;
    auteware_img_pack_tail *tmp_tail=NULL;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    fp=fopen(file_name,"rb");        
    if(!fp)
    {
        npd_syslog_dbg("Cannot open file %s\n",file_name);
        return -1;
    }
    fseek(fp,-sizeof(auteware_img_pack_tail),SEEK_END);
    ret=fread(tmp, sizeof(auteware_img_pack_tail), 1, fp);
    fclose(fp);
    if(ret<1)
    {        
        return -1;
    }
    if(memcmp(tmp,AUTEWARETAIL,16))
    {
        return -1;
    } 
    memcpy(tail,tmp,sizeof(auteware_img_pack_tail));
    tmp_tail=(auteware_img_pack_tail*)tmp;
    tail->time=npd_file_split_swap32(tmp_tail->time);
    return 0;        
}

int npd_image_is_bak(char *name)
{
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(strncasecmp(name + strlen(name) - 4, ".bak", 4))
	{
		return 0;
	}
    return 1;
}

auteware_tail_t *npd_init_image_tail_list(char * img_type)
{
    char full_name[128];
    int ret=-1;
    struct dirent *ptr; 
    struct stat sb;   
    DIR *dir;
    auteware_tail_t *auteware_tail_entry=NULL;
    auteware_tail_t *tmp_head=image_tail_head;
    auteware_img_pack_tail tail_entry;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(image_tail_head)
    {
		if (img_type)
		{
			auteware_tail_t *auteware_tail_head=image_tail_head;

		    while(auteware_tail_head)
		    {
		        if(!strncmp(auteware_tail_head->tail_entry.type,img_type,strlen(auteware_tail_head->tail_entry.type)))
		        {
					return image_tail_head;
		        }
				auteware_tail_head=auteware_tail_head->next;
		    }
			/* have check all tail. can't find the img type, reload the tail head */

		}
		else
		{
			return image_tail_head;
		}
        
    }
    dir=opendir("/mnt");
	if(NULL == dir)
	{
		npd_syslog_err("%s: Open dir failed (%s).\n", strerror (errno));
		return NULL;
	}
    while((ptr=readdir(dir))!=NULL)
    {
        if(stat(ptr->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode) )
        {
            continue;
        }
        if(npd_image_is_bak(ptr->d_name))
        {
            continue;
        }
        memset(full_name,0,128);
        sprintf(full_name,"/mnt/%s",ptr->d_name);
        memset(&tail_entry,0,sizeof(auteware_img_pack_tail));
        ret=npd_get_image_auteware_tail(full_name,&tail_entry);
        if(!ret)
        {
            //add into the list
            auteware_tail_entry=(auteware_tail_t*)malloc(sizeof(auteware_tail_t));
            if(!auteware_tail_entry)
            {
                npd_syslog_dbg("Malloc for auteware_tail_entry failed \r\n");
                closedir(dir);
                npd_release_auteware_tail_list();
                return NULL;
            }
            memset(auteware_tail_entry,0,sizeof(auteware_tail_t));
            memcpy(&(auteware_tail_entry->tail_entry),&tail_entry,sizeof(auteware_img_pack_tail));
            //save the image's real name
            memset(auteware_tail_entry->tail_entry.filename,0,sizeof(auteware_tail_entry->tail_entry.filename));
            memcpy(auteware_tail_entry->tail_entry.filename,ptr->d_name,strlen(ptr->d_name));
            auteware_tail_entry->next=NULL;
            if(tmp_head)
            {
                tmp_head->next=auteware_tail_entry;               
                tmp_head=tmp_head->next;
            }
            else
            {
                tmp_head=auteware_tail_entry;
                image_tail_head=tmp_head;
            }
            
        }
        
    }
    closedir(dir);
    return image_tail_head;
    
}


void npd_show_auteware_tail_list()
{
    auteware_tail_t *tmp=image_tail_head;
    while(tmp)
    {
        npd_syslog_dbg("Name=%s,time=%d,type=%s,version=%s\r\n",tmp->tail_entry.filename,tmp->tail_entry.time,tmp->tail_entry.type,tmp->tail_entry.version);
        tmp=tmp->next;
    }
}

board_type_t *npd_init_board_type_list()
{
    board_type_t *board_type_entry=NULL;
    board_type_t *tmp_head=board_type_head;
    FILE *fp=fopen(HW_COMPAT_FILE,"r");
    char buffer[64];
    int len=0;
    char *delimiter=NULL;
    if(board_type_head)
    {
		if(fp)
		{
			fclose(fp);
		}
        return board_type_head;
    }
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!fp)
    {
        npd_syslog_dbg("Open file %s failed\r\n",HW_COMPAT_FILE);
        return NULL;
    }
    while(1)
    {
        board_type_entry=(board_type_t *)malloc(sizeof(board_type_t));
        if(!board_type_entry)
        {
            npd_syslog_dbg("Malloc board_type_entry failed\r\n");
            fclose(fp);
            npd_release_board_type_list();
            return NULL;
        }
        memset(board_type_entry,0,sizeof(board_type_t));
        memset(buffer,0,64);
        fgets(buffer,64,fp);
        len=strlen(buffer);
        if(len<3)
        {
            fclose(fp);
            break;
        }
        delimiter=strchr(buffer,' ');
        if(!delimiter)
        {
            npd_syslog_dbg("Parse file %s failed\r\n",HW_COMPAT_FILE);
            fclose(fp);
            npd_release_board_type_list();
            return NULL;
        }
        memcpy(board_type_entry->board_type,buffer,strlen(buffer)-strlen(delimiter));
        memcpy(board_type_entry->image_type,delimiter+1,strlen(delimiter)-1);
        board_type_entry->next=NULL;
        if(tmp_head)
        {
            tmp_head->next=board_type_entry;
            tmp_head=tmp_head->next;
        }
        else
        {
            tmp_head=board_type_entry;
            board_type_head=tmp_head;
        }
    }
    return board_type_head;
    
}

void npd_show_board_type_list()
{
    board_type_t *tmp=board_type_head;
    while(tmp)
    {
        npd_syslog_dbg("Board type=%s,image type=%s\r\n",tmp->board_type,tmp->image_type);
        tmp=tmp->next;
    }
}

int npd_get_sw_type_by_board_type(char **sw_type, char *board_type)
{
	board_type_t *tmp=NULL;
    if(!npd_init_board_type_list())
    {
        npd_syslog_dbg("Function npd_init_board_type_list failed\r\n ");
        return -1;
    }
    tmp=board_type_head;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    while(tmp)
    {
        if(!strncmp(tmp->board_type,board_type,strlen(board_type)))
        {
            *sw_type = tmp->image_type;
            return 0;
        }
        tmp=tmp->next;
    }
    npd_syslog_dbg("Function npd_get_image_type_by_board_type failed:b_t=%s\r\n ",board_type);
    return -1;
}

int npd_get_image_type_by_board_type(char *img_type,char *board_type)
{
    board_type_t *tmp=NULL;
    if(!npd_init_board_type_list())
    {
        npd_syslog_dbg("Function npd_init_board_type_list failed\r\n ");
        return -1;
    }
    tmp=board_type_head;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    while(tmp)
    {
        if(!strncmp(tmp->board_type,board_type,strlen(board_type)))
        {
            memcpy(img_type,tmp->image_type,strlen(tmp->image_type));
            return 0;
        }
        tmp=tmp->next;
    }
    npd_syslog_dbg("Function npd_get_image_type_by_board_type failed:b_t=%s,img_t=%s\r\n ",board_type,img_type);
    return -1;
}

int npd_board_type_has_same_image(char *board_t)
{
    int ret=-1;
    char img_type_self[64]={0};
    char img_type[64]={0};
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Board type=%s,local type=%s",board_t,localmoduleinfo->fix_param->short_name);
    ret=npd_get_image_type_by_board_type(img_type_self,localmoduleinfo->fix_param->short_name);
    
    if(ret)
    {
        return -1;
    }
    ret=npd_get_image_type_by_board_type(img_type,board_t);
    if(ret)
    {
        return -1;
    }
    if(!strncmp(img_type_self,img_type,strlen(img_type)))
    {
        return 0;
    }
    return -1;
    
}

int npd_get_image_tail_by_board_type(char *board_type,auteware_img_pack_tail *tail_entry)
{
    int ret=-1;
    char img_type[16]={0};
    auteware_tail_t *auteware_tail_head=NULL;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);    
    ret=npd_get_image_type_by_board_type(img_type,board_type);
    if(ret)
    {
        return -1;
    }
    if(!npd_init_image_tail_list(img_type))
    {
        return -1;
    }
    auteware_tail_head=image_tail_head;
    while(auteware_tail_head)
    {
        if(!strncmp(auteware_tail_head->tail_entry.type,img_type,strlen(auteware_tail_head->tail_entry.type)))
        {
			char file_name[256], version_no[32];
			unsigned int build_time = 0;
			memset(version_no, 0, sizeof(version_no));
			
            memcpy(tail_entry,&(auteware_tail_head->tail_entry),sizeof(auteware_img_pack_tail));
			
			ret = npd_get_image_info(file_name, version_no, &build_time);
			if(ret == 0 
				&& build_time != CHASM_DEFAULT_BUILD_TIME
				&& strncmp(version_no, auteware_tail_head->tail_entry.version, 32) == 0)
			{
                return 0;
			}
        }
        auteware_tail_head=auteware_tail_head->next;
    }
    return -1;
}


int npd_get_image_info_by_board_type(char *name,char *version,unsigned int *time,char *board_type)
{
    auteware_img_pack_tail tail_entry;
    int ret=-1;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    ret=npd_board_type_has_same_image(board_type);
    if(!ret)
    {
        npd_syslog_dbg("Same image type\r\n");
        return npd_get_image_info(name,version,time);
    }
    npd_init_board_type_list();
    npd_init_image_tail_list(NULL);
    if(!board_type_head||!image_tail_head)
    {
        npd_syslog_dbg("Get hw compat list or image tail list failed\r\n");
        return -1;
    }
    npd_show_auteware_tail_list();
    npd_show_board_type_list();
    ret= npd_get_image_tail_by_board_type(board_type,&tail_entry);
    if(ret)
    {
        npd_syslog_dbg("Function npd_get_image_tail_by_board_type failed\r\n");
    }
    else
    {
        memcpy(name,tail_entry.filename,strlen(tail_entry.filename));
        memcpy(version,tail_entry.version,strlen(tail_entry.version));
        *time=tail_entry.time;
    }
    return ret;
       
}

//check if the verison error--pangxf

long chasm_version_err(char *version,unsigned int build_t)
{
    char localversion[32]={0};
    char file_name[256]={0};
    unsigned int build_time=0;
    int ret=-1;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	/* This not check the version */
	if (0 == strcmp(version, "NOT_CHECK_VER"))
	{
		return FALSE;
	}
	
    ret=npd_get_image_info(file_name,localversion,&build_time);
    if(ret)
    {
        return FALSE;
    }
    npd_syslog_dbg("Self build time=%d,other time=%d\r\n",build_time,build_t);
	npd_syslog_dbg("Remote board running version: %s. Local running version: %s.\r\n", version, localversion);
    if(0 == strcmp(version, localversion))
    {
        if(build_t<=build_time)
        {
            return FALSE;
        }
    }
    return TRUE;
}

int chassis_only_one_mcu()
{
    int i;

    for(i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
    {
        int master_slot = SYS_CHASSIS_MASTER_SLOT_INDEX(i);
        if(master_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
        else if (!CHASSIS_SLOT_INSERTED(master_slot))
			continue;
		else
			return FALSE;
    }
    return TRUE;
    
}

int chassis_only_one_mcu_running()
{
    int i;

    for(i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
    {
        int master_slot = SYS_CHASSIS_MASTER_SLOT_INDEX(i);
        if(master_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
        else if (!CHASSIS_SLOT_INSERTED(master_slot))
			continue;
		else if (SYS_MODULE_RUNNINGSTATE(master_slot) == RMT_BOARD_RUNNING)
			return FALSE;
    }
    return TRUE;
    
}

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
int chassis_manage_high_priority(int slot_index)
{
    int i;

    for(i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
    {
        int master_slot = SYS_CHASSIS_MASTER_SLOT_INDEX(i);
        if(slot_index > master_slot)
                return FALSE;
    }

    return TRUE;
}
#endif
/*BOOT 里将boot img的名字都变成小写了.
所以带来这个麻烦*/
int npd_find_real_img_filename(char *img_file, char *real_img_file)
{
    DIR		*pDir = NULL;		/* ptr to directory descriptor */
    struct dirent	*pDirEnt = NULL;	/* ptr to dirent */
	pDir = opendir("/mnt");
	if(pDir == NULL)
	{
		return -1;
	}
	while(NULL != (pDirEnt = readdir(pDir)))
	{
		if(strncasecmp(img_file, pDirEnt->d_name, 255) == 0)
		{
			/*本来就是权宜之计。算了，不考虑是文件夹的情况了*/
			snprintf(real_img_file, 255, pDirEnt->d_name);
	        closedir(pDir);
			return 0;
		}
	}
	closedir(pDir);
	return -1;
}

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
//image upgrade--pangxf
long  chasm_os_upgrade(unsigned int slot_index)
{
	char img_file[256] = {0};
	char cmd[256] = {0};
    char version[32]={0};
    unsigned int build_t;
    int ret=-1;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	memset(img_file, 0, 256);
	ret=npd_get_image_info_by_board_type(img_file,version,&build_t, SHORT_NAME_ON_SLOT_INDEX(slot_index));
    if(!ret)
    {
    	npd_syslog_dbg("Put img file %s to slot %d.\r\n", img_file, slot_index + 1);
        /*run file client as daemon*/
    	sprintf(cmd, "file_client -d -i -r %d /mnt/%s /mnt/%s.sav\n", slot_index + 1, img_file, img_file);
    	system(cmd);
        return NPD_SUCCESS;
    }
    return NPD_FAIL;
}
#endif
long chasm_board_ready_config(unsigned int slot_index)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("No need board ready config for board %d\r\n", slot_index+1 );
	return NPD_SUCCESS;
}

int npd_get_image_info(char *name,char *version,unsigned int *build_time)
{
    char img_file[256] = {0};
	char real_img_file[256] = {0};
    int ret=-1;
    char img_path[256]={0};
    unsigned int ver;
    auteware_img_pack_tail tail_entry;
	if(nbm_get_boot_img_name(img_file) != 0)
	{
        npd_syslog_dbg("Get image name failed\r\n");
		goto dft_ver;
	}
	if(npd_find_real_img_filename(img_file, real_img_file) != 0)
	{
		npd_syslog_dbg("Can not find img file %s.\r\n", img_file);
		goto dft_ver;
	}
    memcpy(name,img_file,strlen(img_file));
    sprintf(img_path,"/mnt/%s",real_img_file);
    memset(&tail_entry,0,sizeof(auteware_img_pack_tail));
    ret=npd_get_image_auteware_tail(img_path,&tail_entry);
    if(ret)
    {    
		npd_syslog_dbg("Can not get img %s tail.\r\n", img_path);        
		goto dft_ver;
    }
    memcpy(version,tail_entry.version,strlen(tail_entry.version));
    *build_time=tail_entry.time;    
    return NPD_SUCCESS;
dft_ver:
	ver = npd_query_sw_version(SYS_LOCAL_MODULE_TYPE);
    snprintf(version, 31, "%x", ver);
    *build_time=CHASM_DEFAULT_BUILD_TIME;
	memcpy(name,img_file,strlen(img_file));
    return NPD_SUCCESS;
}

long chasm_ready_check(board_param_t *src_board)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);	
	npd_syslog_dbg("chasm board ready check for %d.\r\n", src_board->slot_index + 1);
	
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		return NPD_SUCCESS;
	}

	if(src_board->fix_param->board_ready_config)
		src_board->fix_param->board_ready_config(src_board->slot_index);

	return NPD_SUCCESS;
}
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
long chasm_board_issbymaster_connected(board_param_t *board)
{
	int i;
	int connected = FALSE;
	int fd;
    chasm_circle_data_t *data;


	for (i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
	{
		if (i == SYS_MASTER_ACTIVE_SLOT_INDEX)
			continue;

		 data = &chasm_circle_data[SYS_CHASSIS_MASTER_SLOT_INDEX(i)];
   		 fd = data->sock;

		 if (0 != fd)
		 {
		 	npd_syslog_dbg("board %d connected to master board %d is TRUE",
				board->slot_index+1,SYS_CHASSIS_MASTER_SLOT_INDEX(i)+1);
			connected = TRUE;
			break;
		 }	
	}

	return connected;
}
board_param_t * chasm_get_sbymaster()
{
	int i;
	int fd;
    chasm_circle_data_t *data;

	for (i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
	{
		if (i == SYS_MASTER_ACTIVE_SLOT_INDEX)
			continue;

		 data = &chasm_circle_data[SYS_CHASSIS_MASTER_SLOT_INDEX(i)];
   		 fd = data->sock;

		 if (0 != fd)
		 {
		 	return data->board;
		 }	
	}

	return NULL;
}

long chasm_isstandby(int slot_index)
{
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        npd_syslog_dbg("Local board can not execute switchover command\n");
        return BOARD_RETURN_CODE_NOT_ACTMASTER;
    }
    else
    {
        if(slot_index == SYS_LOCAL_MODULE_SLOT_INDEX)
            return BOARD_RETURN_CODE_WRONG_SLOTNO;
        if(!SYS_CHASSIS_ISMASTERSLOT(slot_index))
            return BOARD_RETURN_CODE_WRONG_SLOTNO;
        if(RMT_BOARD_RUNNING != MODULE_STATUS_ON_SLOT_INDEX(slot_index))
            return BOARD_RETURN_CODE_WRONG_STATE;
        if(localmoduleinfo->runstate != LOCAL_ACTMASTER_RUNNING)
        {
            npd_syslog_dbg("Local board can not execute switchover command when it is not normally running\n");
            return BOARD_RETURN_CODE_WRONG_STATE;
        }
    }
    return BOARD_RETURN_CODE_ERR_NONE;
}

long chasm_switchover(int slot_index)
{
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        npd_syslog_dbg("Local board can not execute switchover command\n");
        return BOARD_RETURN_CODE_NOT_ACTMASTER;
    }
    else
    {
        int state = localmoduleinfo->runstate;
        int evt = SWITCHOVER_REPORT;
        int ret;

        if(slot_index == SYS_LOCAL_MODULE_SLOT_INDEX)
            return BOARD_RETURN_CODE_WRONG_SLOTNO;
        if(!SYS_CHASSIS_ISMASTERSLOT(slot_index))
            return BOARD_RETURN_CODE_WRONG_SLOTNO;
        if(RMT_BOARD_RUNNING != MODULE_STATUS_ON_SLOT_INDEX(slot_index))
            return BOARD_RETURN_CODE_WRONG_STATE;
        if(localmoduleinfo->runstate != LOCAL_ACTMASTER_RUNNING)
        {
            npd_syslog_dbg("Local board can not execute switchover command when it is not normally running\n");
            return BOARD_RETURN_CODE_WRONG_STATE;
        }
		/* Added by wangquan 20120608 for 'force-switchover' command start */
		if(SYS_CHASSIS_MASTER_SET_FUNC)
		    (*SYS_CHASSIS_MASTER_SET_FUNC)(FALSE);
		/* Added by wangquan 20120608 for 'force-switchover' command end */
        chasm_lock();
        ret = (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, chassis_slots[slot_index], evt, NULL);
        chasm_unlock();
        return ret;
    }
}

long  chasm_close_ctrlintf(board_param_t *src_board)
{
	char cmd[300] = {0};

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	npd_syslog_dbg("board %d close connection to other line card\r\n", src_board->slot_index+1);

	/*close connection of  line card */
	sprintf(cmd, "tipc-config -b|grep eth:|awk -F \":\" '{print \"ifconfig \" $2 \" down\"}'| sh");
	system(cmd);
	
    return NPD_SUCCESS;
}
#endif

int chasm_subboard_init_eth_ports(board_param_t *board, board_param_t *sub_board, int state)
{
    int subboard_slot;

    npd_syslog_dbg("Initializing ethernet interfaces of board %d...\n", CHASSIS_SLOT_INDEX2NO(board->slot_index));
    if(board == sub_board)
        subboard_slot = 0;
    else
        subboard_slot = sub_board->slot_index;
	if(PRODUCT_IS_BOX)
		npd_init_subslot_eth_ports(0, SYS_LOCAL_CHASSIS_INDEX, subboard_slot, state);
	else
        npd_init_subslot_eth_ports(SYS_LOCAL_CHASSIS_INDEX, board->slot_index, subboard_slot, state);
	return NPD_SUCCESS;
}

int chasm_subboard_online_remove(board_param_t *board, board_param_t *sub_board)
{
    int subboard_slot;

    npd_syslog_event("\nRemoving ethernet interfaces of board %d...\r\n", CHASSIS_SLOT_INDEX2NO(board->slot_index));
    if(board == sub_board)
        subboard_slot = 0;
    else
        subboard_slot = sub_board->slot_index;
	if(PRODUCT_IS_BOX)
        npd_remove_subslot_eth_ports(0, SYS_LOCAL_CHASSIS_INDEX, subboard_slot, PORT_ONLINE_REMOVED);
    else
        npd_remove_subslot_eth_ports(SYS_LOCAL_CHASSIS_INDEX, board->slot_index, subboard_slot, PORT_ONLINE_REMOVED);

	
	return NPD_SUCCESS;
}

int chasm_subboard_online_delete(board_param_t *board, board_param_t *sub_board)
{
    int subboard_slot;

    npd_syslog_event("\nDeleting ethernet interfaces of board %d...\r\n", CHASSIS_SLOT_INDEX2NO(board->slot_index));
    if(board == sub_board)
        subboard_slot = 0;
    else
        subboard_slot = sub_board->slot_index;
	if(PRODUCT_IS_BOX)
        npd_delete_subslot_eth_ports(0, SYS_LOCAL_CHASSIS_INDEX, subboard_slot, PORT_ONLINE_REMOVED);
	else
        npd_delete_subslot_eth_ports(SYS_LOCAL_CHASSIS_INDEX, board->slot_index, subboard_slot, PORT_NORMAL);
    npd_key_database_lock();
	netif_notify_event(board->slot_index, NOTIFIER_SLOT_DELETE);
	npd_key_database_unlock();
	return NPD_SUCCESS;
}

int chasm_subboard_online_insert(board_param_t *board, board_param_t *sub_board)
{
    int subboard_slot;

    npd_syslog_event("\nInserting ethernet interfaces of board %d...\n", CHASSIS_SLOT_INDEX2NO(board->slot_index));
    if(board == sub_board)
        subboard_slot = 0;
    else
        subboard_slot = sub_board->slot_index;
	if(PRODUCT_IS_BOX)
        npd_insert_subslot_eth_ports(0, SYS_LOCAL_CHASSIS_INDEX, subboard_slot, PORT_ONLINE_REMOVED);
	else
        npd_insert_subslot_eth_ports(SYS_LOCAL_CHASSIS_INDEX, board->slot_index, subboard_slot, PORT_NORMAL);
    
	return NPD_SUCCESS;
}
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)

int chasm_board_isneed_reported(int slot_index)
{
	int need_report = FALSE;
	
    if(chassis_slots[slot_index]->runstate >= RMT_BOARD_REGISTERED)
    {
		need_report = TRUE;
	}
	else if (chassis_slots[slot_index]->online_removed == TRUE)
	{
		if (chassis_slots[slot_index]->runstate == RMT_BOARD_NOEXIST
			|| chassis_slots[slot_index]->runstate == RMT_BOARD_HWINSERTED)
		{
			need_report = TRUE;
		}
		else
		{
			need_report = FALSE;
		}
	}
	else
	{
		need_report = FALSE;
	}

	return need_report;
}
int chasm_board_isready(board_param_t *board)
{
    int i;

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
        if(chassis_slots[i] == board)
        {
			if (chassis_slots[i]->rmtstate[i] < RMT_BOARD_READY)
			{
				npd_syslog_dbg("board %d's rmtstate %d less RMT_BOARD_READY\r\n",i+1, board->slot_index+1);
				return FALSE;
			}				
			else
				continue;
		}
            
        if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
        {
            if(board->rmtstate[i] < RMT_BOARD_READY)
            {
				npd_syslog_dbg("board %d's rmtstate %d less RMT_BOARD_READY\r\n",
					board->slot_index+1, i+1);
                return FALSE;
            }
            if(chassis_slots[i]->rmtstate[board->slot_index] < RMT_BOARD_READY)
            {
				npd_syslog_dbg("chassis_slots %d's rmtstate %d less RMT_BOARD_READY\r\n",
					i+1, board->slot_index+1);
                return FALSE;
            }
        }
    }
    return TRUE;
}

int chasm_board_isremoved(board_param_t *board)
{
    int i;

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
		
		if(chassis_slots[i] == board) // add this for fix the rmt board can't remove correctly
            continue;

		
        if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
        {
            if(board->rmtstate[i] != RMT_BOARD_NOEXIST)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

int chasm_board_is_onlyone(void)
{
    int i;
	if (1 == app_slave_indpnt_runget())
	{
		return TRUE;
	}

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
		if (CHASSIS_SLOT_INSERTED(i))
		{
			/* have other board inserted, need to wait other */
			return FALSE;
		}
    }
    return TRUE;
}

void chasm_ipp_exception(int sock, void *circle_data, void *user_data)
{
    chasm_circle_data_t *data = (chasm_circle_data_t *)circle_data;
    board_param_t *board = data->board;
    int error; 
    unsigned int n;
    int state;
    int evt;

    n = sizeof(error);
    if(getsockopt(data->sock, SOL_SOCKET, SO_ERROR, &error, &n) < 0)
    {
        int evt = TIPC_BREAK;
        npd_syslog_err("Can not connect slave server. reset the slave board %d\n",
            board->slot_index+1);
        close(data->sock);
        data->sock = 0;
        chasm_lock();
        (*(*(board->state_function)[state].funcs)[evt])
            (board, board, evt, NULL);
        chasm_unlock();
        return;
    }
    switch(error)
    {
        case ENOLINK:
            chasm_lock();
            state = board->runstate;
            evt = TIPC_BREAK;
            (*(*(board->state_function)[state].funcs)[evt])
                (board, board, evt, NULL);
            chasm_unlock();
            break;
        default:
            npd_syslog_err("The tipc socket for board %d have error, errno is 0x%x.\n",
                board->slot_index, error);
            break;
    }
    
}

void chasm_accept_tipc(int sock, void *circle_data, void * user_data)
{
    int sd;
    struct sockaddr_tipc client = {0};
    unsigned int len = sizeof(client);

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
    sd = accept(sock, (struct sockaddr*)&client, &len);
    if(-1 == sd)
    {
        npd_syslog_err("Accept master tipc client error.\n");
        return;
    }
    else
    {
        int slot_index = tipc_node(client.addr.id.node) - 1;
        board_param_t *board = chassis_slots[slot_index];
        int state;
        int evt = TIPC_CONNECT;
        chasm_circle_data_t *client_data;
        npd_syslog_dbg("Accept connection from remote board %d\n", slot_index+1);

        chasm_lock();
        client_data = &chasm_circle_data[slot_index];
		if (client_data->sock > 0)
		{
			npd_syslog_err("board %d, sock %d, is not 0 \r\n", slot_index+1, client_data->sock);
		}
        client_data->sock = sd;
        client_data->board = board;
        state = localmoduleinfo->runstate;

		(*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, board, evt, NULL);
        chasm_unlock();
    }
    
}


chasm_board_reginfo_tlv_t* chasm_board_reginfo_tlv_ntoh(chasm_board_reginfo_tlv_t* tlv)
{
	if(tlv == NULL)
		return tlv;

	tlv->type = ntohs(tlv->type);
    tlv->length = ntohs(tlv->length);
    tlv->chassis_id = ntohs(tlv->chassis_id);
	tlv->slotid = ntohs(tlv->slotid);
	tlv->subslot_id = ntohs(tlv->subslot_id);
	tlv->board_type = ntohs(tlv->board_type);

	return tlv;
}

chasm_board_reginfo_tlv_t* chasm_board_reginfo_tlv_hton(chasm_board_reginfo_tlv_t* tlv)
{
	if(tlv == NULL)
		return tlv;

	tlv->type = htons(tlv->type);
    tlv->length = htons(tlv->length);
    tlv->chassis_id = htons(tlv->chassis_id);
	tlv->slotid = htons(tlv->slotid);
	tlv->subslot_id = htons(tlv->subslot_id);
	tlv->board_type = htons(tlv->board_type);

	return tlv;
}

chasm_ams_reginfo_tlv_t* chasm_board_asm_reginfo_tlv_ntoh(chasm_ams_reginfo_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;

	tlv->type = ntohs(tlv->type); 
    tlv->length = ntohs(tlv->length);
    tlv->ams_type = ntohs(tlv->ams_type);
	tlv->vender_id = ntohs(tlv->vender_id);
	tlv->revid = ntohs(tlv->revid);
	tlv->pci_id = ntohs(tlv->pci_id);

	return tlv;
}

chasm_ams_reginfo_tlv_t* chasm_board_asm_reginfo_tlv_hton(chasm_ams_reginfo_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;

	tlv->type = htons(tlv->type); 
    tlv->length = htons(tlv->length);
    tlv->ams_type = htons(tlv->ams_type);
	tlv->vender_id = htons(tlv->vender_id);
	tlv->revid = htons(tlv->revid);
	tlv->pci_id = htons(tlv->pci_id);

	return tlv;
}

chasm_board_statusinfo_tlv_t* chasm_board_statusnfo_tlv_ntoh(chasm_board_statusinfo_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;
	
	tlv->type = ntohs(tlv->type); 
    tlv->length = ntohs(tlv->length);
    tlv->chassis_id = ntohs(tlv->chassis_id);
	tlv->slotid = ntohs(tlv->slotid);
	tlv->subslot_id = ntohs(tlv->subslot_id);
	tlv->board_type = ntohs(tlv->board_type);
	tlv->runstate = ntohs(tlv->runstate);
	tlv->online_removed = ntohs(tlv->online_removed);

	return tlv;
}

chasm_board_statusinfo_tlv_t* chasm_board_statusnfo_tlv_hton(chasm_board_statusinfo_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;
	
	tlv->type = htons(tlv->type); 
    tlv->length = htons(tlv->length);
    tlv->chassis_id = htons(tlv->chassis_id);
	tlv->slotid = htons(tlv->slotid);
	tlv->subslot_id = htons(tlv->subslot_id);
	tlv->board_type = htons(tlv->board_type);
	tlv->runstate = htons(tlv->runstate);
	tlv->online_removed = htons(tlv->online_removed);

	return tlv;
}

chasm_product_info_tlv_t* chasm_product_info_tlv_ntoh(chasm_product_info_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;
	
	tlv->type = ntohs(tlv->type); 
    tlv->length = ntohs(tlv->length);
    tlv->product_type = ntohs(tlv->product_type);
    tlv->build_time=ntohl(tlv->build_time);

	return tlv;
}

chasm_switchover_tlv_t* chasm_switchover_tlv_hton(chasm_switchover_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;
	
	tlv->type = htons(tlv->type); 
    tlv->length = htons(tlv->length);
    tlv->slot_id = htons(tlv->slot_id);
	tlv->pre_act_master = htons(tlv->pre_act_master); 
    tlv->reason = htons(tlv->reason);
    tlv->runstate = htons(tlv->runstate);

	return tlv;
}

chasm_board_regres_tlv_t* chasm_board_regres_tlv_hton(chasm_board_regres_tlv_t *tlv)
{
	if(tlv == NULL)
		return tlv;
	
	tlv->type = htons(tlv->type); 
	tlv->length = htons(tlv->length);
	tlv->product_type = htons(tlv->product_type);
    tlv->build_time=htonl(tlv->build_time);

	return tlv;
}



long chasm_assemble_reg_req_pdu(board_param_t *board, board_param_t *sub_board, char**pdu, int *len)
{
    chasm_pdu_head_t *head;
    chasm_board_reginfo_tlv_t *tlv;
    chasm_ams_reginfo_tlv_t *ams_tlv;
    char *buf;
    int i;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	if(board->fix_spec_param == NULL)
	{
		npd_syslog_dbg("%s: Not local board.\r\n", __FUNCTION__);
		return NPD_SUCCESS;
	}
    *len = sizeof(chasm_pdu_head_t);
    for(i = 0; i < board->fix_param->subboard_fix_param->sub_slotnum; i++)
    {
        if(NULL == board->sub_board[i])
			continue;
		if(NULL == board->sub_board[i]->fix_param)
			continue;
		if(NULL == board->sub_board[i]->fix_spec_param)
			continue;
		
        {
            int j;
            *len += sizeof(chasm_board_reginfo_tlv_t);
            for(j =0 ; j < ASIC_TYPE_MAX; j++)
            {
                if(NULL != board->sub_board[i]->fix_spec_param->ams_param[j]) 
                {
                    *len += sizeof(chasm_ams_reginfo_tlv_t)
                             * board->sub_board[i]->fix_spec_param->ams_param[j]->num;
				}
            }
        }
    }
    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (chasm_pdu_head_t*)(*pdu);

    head->version = htonl(chasm_protocol_version());
    head->length = htonl(*len);
    buf = (char*)(head+1);

    for(i = 0; i < board->fix_param->subboard_fix_param->sub_slotnum; i++)
    {
         if(NULL == board->sub_board[i])
			continue;
		if(NULL == board->sub_board[i]->fix_param)
			continue;
		if(NULL == board->sub_board[i]->fix_spec_param)
			continue;
        {
            int j;
            char localversion[32]={0};
            char file_name[64]={0};
            unsigned int build_time=0;
            int ret=-1;

            tlv = (chasm_board_reginfo_tlv_t *)buf;
            tlv->type = CHASM_PDU_TYPE_REG_REQ;
            tlv->length = *len - sizeof(chasm_pdu_head_t);
            tlv->slotid = board->slot_index;
            if(i == 0)
               tlv->subslot_id = 0;
            tlv->board_type = board->sub_board[i]->fix_param->board_type;
            strncpy((char*)tlv->board_short_name, (char*)board->sub_board[i]->man_param.modname, 16);
            strncpy((char*)tlv->sn, (char*)board->sub_board[i]->man_param.sn,32);
/*            strncpy((char*)tlv->hw_version, (char*)&board->sub_board[i]->man_param.hw_version, 32);*/
            ret=npd_get_image_info(file_name,localversion,&build_time);
            if(ret != 0)
            {
                sprintf((char*)tlv->sw_version, "%x", npd_query_sw_version(tlv->board_type));
            }
			else
			{
				sprintf((char*)tlv->sw_version, "%s", localversion);
			}

			tlv = chasm_board_reginfo_tlv_hton(tlv);
			
            buf = (char*)(tlv+1);
            for(j =0 ; j < ASIC_TYPE_MAX; j++)
            {
                if(NULL != board->sub_board[i]->fix_spec_param->ams_param[j]) 
                {
                    int k;

                    board_param_t *temp = board->sub_board[i];
                    for(k = 0; k < temp->fix_spec_param->ams_param[j]->num; k++)
                    {
                        ams_tlv = (chasm_ams_reginfo_tlv_t *)buf;
                        ams_tlv->type = CHASM_PDU_TYPE_AMS_REG;
                        ams_tlv->length = sizeof(chasm_ams_reginfo_tlv_t);
                        ams_tlv->ams_type = temp->ams_info[j][k].type;
/*                        ams_tlv->vender_id = temp->ams_info[j][k].pci_id;*/
                        ams_tlv->revid = temp->ams_info[j][k].rev_id;
                        ams_tlv->pci_id = temp->ams_info[j][k].dev_id;
                        strncpy((char*)ams_tlv->name, (char*)temp->ams_info[j][k].name, 32);
						ams_tlv = chasm_board_asm_reginfo_tlv_hton(ams_tlv);
                        buf = (char*)(ams_tlv+1);
                    }
                }
            }
        }
    }
    if(chasm_debug_pkt)
    {
		chasm_pdu_print(*pdu, *len);
		npd_syslog_dbg("\r\nThe reg_req pdu length is %d\n", *len);
        npd_syslog_dbg("\r\n");
    }
	return NPD_SUCCESS;
}

long chasm_assemble_reg_res_pdu(board_param_t * board, board_param_t * sub_board, board_param_t *src_board, char ** pdu, int * len)
{
    chasm_pdu_head_t *head;
    chasm_board_regres_tlv_t *tlv;
    chasm_board_statusinfo_tlv_t *board_tlv;
    char *buf;
    int i;
    int ret=-1;
    char file_name[64];
    char version[64]={0};
    unsigned int build_t=0;
    int image_info_flag=0;
	int slot_index = -1;
    //get the compatible image's version and build time

	if(src_board->fix_param->get_image_info)
	{
		src_board->fix_param->get_image_info(file_name,version, &build_t, src_board->fix_param->short_name);		
		image_info_flag=1;
	}
	else
	{
	    ret=npd_get_image_info_by_board_type(file_name,version, &build_t, src_board->fix_param->short_name);
	    if(ret)
	    {
	        npd_syslog_dbg("Function npd_get_image_info_by_board_type %s failed\r\n", src_board->man_param.modname);
			ret = npd_get_image_info_by_board_type(file_name,version, &build_t, localmoduleinfo->fix_param->short_name);
			if(ret == 0)
			{
				build_t = 0;
	            image_info_flag=1;
			}			
	    }
	    else
	    {
			image_info_flag=1;
	    }
	}
	
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    *len = sizeof(chasm_pdu_head_t)+sizeof(chasm_board_regres_tlv_t);
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            int j;
            for(j = 0; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == localmoduleinfo->sub_board[j])
        			continue;
        		if(NULL == localmoduleinfo->sub_board[j]->fix_param)
        			continue;

                {
                    *len += sizeof(chasm_board_statusinfo_tlv_t);
                }
            }
			continue;
        }
        //if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
        if(chasm_board_isneed_reported(i))
        {
            int j;
            for(j = 0; j < chassis_slots[i]->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == chassis_slots[i]->sub_board[j])
        			continue;
        		if(NULL == chassis_slots[i]->sub_board[j]->fix_param)
        			continue;
                {
                    *len += sizeof(chasm_board_statusinfo_tlv_t);
                }
            }
        }
    }
    
    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\r\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (chasm_pdu_head_t*)(*pdu);

    head->version = htonl(chasm_protocol_version());
    head->length = htonl(*len);

    tlv = (chasm_board_regres_tlv_t *)(*pdu+sizeof(chasm_pdu_head_t));
    tlv->type = CHASM_PDU_TYPE_REG_RES;
    tlv->length = sizeof(chasm_board_regres_tlv_t);
    tlv->product_type = SYS_PRODUCT_TYPE;
    memcpy(tlv->base_mac, SYS_PRODUCT_BASEMAC, 6);
    if(image_info_flag)
    {
        sprintf((char*)tlv->sw_version, "%s", version);
        tlv->build_time=build_t;
    }
    else
    {
        sprintf((char*)tlv->sw_version, "%x", npd_query_sw_version(src_board->fix_param->board_type));
		tlv->build_time = CHASM_DEFAULT_BUILD_TIME;
    }
	tlv = chasm_board_regres_tlv_hton(tlv);
    buf = (char*)(tlv+1);
    npd_syslog_dbg("\r\nRemote MAC address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
		tlv->base_mac[0], tlv->base_mac[1], tlv->base_mac[2],
		tlv->base_mac[3], tlv->base_mac[4], tlv->base_mac[5]);
    npd_syslog_dbg("\r\nSoftware version: %s\r\n", tlv->sw_version);
	if(chasm_debug_pkt)
    {
		npd_syslog_dbg("\r\nthe reg_res pdu length is %d byte\r\n", *len);
		chasm_pdu_print(*pdu, *len);
    }

	npd_syslog_dbg("SYS_CHASSIS_SLOTNUM is %d.\r\n", SYS_CHASSIS_SLOTNUM);
	/*ensure master board info be sent at first, then other boards*/
	for(i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
    {
    	slot_index = SYS_CHASSIS_MASTER_SLOT_INDEX(i);
		
        if(slot_index == SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            int j;
            for(j = 0; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == localmoduleinfo->sub_board[j])
        			continue;
        		if(NULL == localmoduleinfo->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = localmoduleinfo->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = localmoduleinfo->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = localmoduleinfo->sub_board[j]->rmtstate[localmoduleinfo->slot_index];
					board_tlv->online_removed = localmoduleinfo->sub_board[j]->online_removed;
                    board_tlv->workmode = localmoduleinfo->sub_board[j]->workmode;
                    board_tlv->active_flags = localmoduleinfo->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);

					npd_syslog_dbg("localmoduleinfo's statusinfo type:%d, state is %d.\r\n",
						board_tlv->type, board_tlv->runstate);
                }				
            }

			continue;
        }
		
		if(chasm_board_isneed_reported(slot_index))		
        {
            int j;
            for(j = 0; j < chassis_slots[slot_index]->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == chassis_slots[slot_index]->sub_board[j])
        			continue;
        		if(NULL == chassis_slots[slot_index]->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = chassis_slots[slot_index]->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = chassis_slots[slot_index]->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = chassis_slots[slot_index]->sub_board[j]->runstate;
					board_tlv->online_removed = chassis_slots[slot_index]->sub_board[j]->online_removed;
                    board_tlv->workmode = chassis_slots[slot_index]->sub_board[j]->workmode;
                    board_tlv->active_flags = chassis_slots[slot_index]->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);

					npd_syslog_dbg("board %d 's statusinfo is  type:%d, state is %d.\r\n"
						,chassis_slots[slot_index]->slot_index + 1, board_tlv->type, board_tlv->runstate);
                }
            }
        }
    }	
	
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
    	if(SYS_MODULE_WORKMODE_ISMASTER(i))
			continue;
		
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            int j;
            for(j = 0; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
            {

                if(NULL == localmoduleinfo->sub_board[j])
        			continue;
        		if(NULL == localmoduleinfo->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = localmoduleinfo->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = localmoduleinfo->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = localmoduleinfo->sub_board[j]->rmtstate[localmoduleinfo->slot_index];
					board_tlv->online_removed = localmoduleinfo->sub_board[j]->online_removed;
                    board_tlv->workmode = localmoduleinfo->sub_board[j]->workmode;
                    board_tlv->active_flags = localmoduleinfo->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);

					npd_syslog_dbg("localmoduleinfo's statusinfo type:%d, state is %d.\r\n",
						board_tlv->type, board_tlv->runstate);
                }				
            }

			continue;
        }

		if(chasm_board_isneed_reported(i))
        {
            int j;
            for(j = 0; j < chassis_slots[i]->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == chassis_slots[i]->sub_board[j])
        			continue;
        		if(NULL == chassis_slots[i]->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = chassis_slots[i]->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = chassis_slots[i]->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = chassis_slots[i]->sub_board[j]->runstate;
					board_tlv->online_removed = chassis_slots[i]->sub_board[j]->online_removed;
                    board_tlv->workmode = chassis_slots[i]->sub_board[j]->workmode;
                    board_tlv->active_flags = chassis_slots[i]->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);

					npd_syslog_dbg("board %d 's statusinfo is  type:%d, state is %d.\r\n"
						,chassis_slots[i]->slot_index + 1, board_tlv->type, board_tlv->runstate);
                }
            }
        }
    }

	npd_syslog_dbg("board %d, subboard %d.\r\n", 
		board->slot_index +1, sub_board->slot_index +1);
	
    if(chasm_debug_pkt)
    {		
		chasm_pdu_print(*pdu, *len);
		npd_syslog_dbg("\r\nthe reg_res pdu length is %d byte\r\n", *len);
        npd_syslog_dbg("\r\n");
    }
	return NPD_SUCCESS;
}

long chasm_assemble_status_report_pdu(board_param_t * board, board_param_t * sub_board, char ** pdu, int * len)
{
    chasm_pdu_head_t *head;
    chasm_board_statusinfo_tlv_t *tlv;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    *len = sizeof(chasm_pdu_head_t)+sizeof(chasm_board_statusinfo_tlv_t);
    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\r\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (chasm_pdu_head_t*)(*pdu);

    head->version = htonl(chasm_protocol_version());
    head->length = htonl(*len);

    tlv = (chasm_board_statusinfo_tlv_t *)(*pdu+sizeof(chasm_pdu_head_t));
    tlv->type = CHASM_PDU_TYPE_STATUS;
    tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
    tlv->slotid = board->slot_index;
    if(board == sub_board)
        tlv->subslot_id = 0;
    else
        tlv->subslot_id = sub_board->slot_index;
    tlv->board_type = sub_board->fix_param->board_type;
    if(sub_board == localmoduleinfo)
        tlv->runstate = sub_board->rmtstate[board->slot_index];
    else
        tlv->runstate = sub_board->runstate;
	tlv->online_removed = sub_board->online_removed;
    tlv->workmode = sub_board->workmode;
    tlv->active_flags = sub_board->redundancystate;
	tlv = chasm_board_statusnfo_tlv_hton(tlv);

	npd_syslog_dbg("board %d, subboard %d, runstate is %d.\r\n", 
		board->slot_index +1, sub_board->slot_index +1, tlv->runstate);

    if(chasm_debug_pkt)
    {
		chasm_pdu_print(*pdu, *len);
		npd_syslog_dbg("\r\n the status_report pdu length is %d byte\r\n", *len);
        npd_syslog_dbg("\r\n");
    }
	
	return NPD_SUCCESS;
}

long chasm_assemble_status_report_all(char **pdu, int *len)
{
    chasm_pdu_head_t *head;
    chasm_board_statusinfo_tlv_t *board_tlv;
    int i;
    char *buf;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
    *len = sizeof(chasm_pdu_head_t);
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            int j;
            for(j = 0; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == localmoduleinfo->sub_board[j])
        			continue;
        		if(NULL == localmoduleinfo->sub_board[j]->fix_param)
        			continue;
                {
                    *len += sizeof(chasm_board_statusinfo_tlv_t);
                }
            }
			continue;
        }
        //if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
		if(chasm_board_isneed_reported(i))
        {
            int j;
            for(j = 0; j < chassis_slots[i]->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == chassis_slots[i]->sub_board[j])
        			continue;
        		if(NULL == chassis_slots[i]->sub_board[j]->fix_param)
        			continue;
                {
                    *len += sizeof(chasm_board_statusinfo_tlv_t);
                }
            }
        }
    }

    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\r\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (chasm_pdu_head_t*)(*pdu);

    head->version = htonl(chasm_protocol_version());
    head->length = htonl(*len);
    buf = (char *)(head+1);

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            int j;
            for(j = 0; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == localmoduleinfo->sub_board[j])
        			continue;
        		if(NULL == localmoduleinfo->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = localmoduleinfo->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = localmoduleinfo->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = localmoduleinfo->sub_board[j]->rmtstate[localmoduleinfo->slot_index];
					board_tlv->online_removed = localmoduleinfo->online_removed;
					board_tlv->workmode = localmoduleinfo->sub_board[j]->workmode;
                    board_tlv->active_flags = localmoduleinfo->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);
                }
            }
			continue;
        }
        //if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
        if(chasm_board_isneed_reported(i))
        {
            int j;
            for(j = 0; j < chassis_slots[i]->fix_param->subboard_fix_param->sub_slotnum; j++)
            {
                if(NULL == chassis_slots[i]->sub_board[j])
        			continue;
        		if(NULL == chassis_slots[i]->sub_board[j]->fix_param)
        			continue;
                {
                    board_tlv = (chasm_board_statusinfo_tlv_t *)buf;
                    board_tlv->type = CHASM_PDU_TYPE_STATUS;
                    board_tlv->length = sizeof(chasm_board_statusinfo_tlv_t);
                    board_tlv->slotid = chassis_slots[i]->slot_index;
                    board_tlv->subslot_id = j;
                    board_tlv->board_type = chassis_slots[i]->sub_board[j]->fix_param->board_type;
                    board_tlv->runstate = chassis_slots[i]->sub_board[j]->runstate;
					board_tlv->online_removed= chassis_slots[i]->sub_board[j]->online_removed;
                    board_tlv->workmode = chassis_slots[i]->sub_board[j]->workmode;
                    board_tlv->active_flags = chassis_slots[i]->sub_board[j]->redundancystate;
					board_tlv = chasm_board_statusnfo_tlv_hton(board_tlv);
                    buf = (char*)(board_tlv+1);
                }
            }
        }
    }
    if(chasm_debug_pkt)
    {
		npd_syslog_dbg("\r\nthe length is %d byte\r\n", *len);
		chasm_pdu_print(*pdu, *len);
        npd_syslog_dbg("\r\n");
    }
	return NPD_SUCCESS;
}

long chasm_assemble_switchover_report_pdu(board_param_t * board, char ** pdu, int * len)
{
    chasm_pdu_head_t *head;
    chasm_switchover_tlv_t *tlv;
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
    *len = sizeof(chasm_pdu_head_t)+sizeof(chasm_switchover_tlv_t);

    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\r\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (chasm_pdu_head_t*)(*pdu);

    head->version = htonl(chasm_protocol_version());
    head->length = htonl(*len);

    tlv = (chasm_switchover_tlv_t *)(*pdu+sizeof(chasm_pdu_head_t));
    tlv->type = CHASM_PDU_TYPE_SWITCHOVER;
    tlv->length = sizeof(chasm_switchover_tlv_t);
    tlv->slot_id = board->slot_index;
    tlv->runstate = board->runstate;
	tlv = chasm_switchover_tlv_hton(tlv);

    if(chasm_debug_pkt)
    {
		npd_syslog_dbg("the length is %d byte\r\n", *len);
		chasm_pdu_print(*pdu, *len);
        npd_syslog_dbg("\r\n");
    }
	return NPD_SUCCESS;
}


void chasm_pdu_recv(int sock, void *circle_data, void *user_data)
{
    chasm_circle_data_t *data = (chasm_circle_data_t*)circle_data;
    board_param_t *board = data->board;
    char *pdu;
    int len, pktType, pktLen;
    chasm_pdu_head_t *head;
    chasm_tlv_t *tlv;
    char *buf;
    int state;
    int evt;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("from sock fd %d\r\n", sock);
    pdu = malloc(2048);
    
    if(NULL == pdu)
    {
        npd_syslog_err("Can not alloc memory for chasm packet receiving.\r\n");
        return;
    }
        
    len = recv(sock, pdu, 2048, 0);
    if(len < 0)
    {
        int state;
        int evt;
        free(pdu);
        npd_syslog_official_event("Remote board %d software connection is broken.", board->slot_index+1);
        chasm_lock();
        state = localmoduleinfo->runstate;
        evt = TIPC_BREAK;
        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, board, evt, NULL);
        chasm_unlock();
		sleep(1);
        return;
    }
	
	if(chasm_debug_pkt)
    {
        npd_syslog_dbg("chasm_pdu_recv<header>: Receive packet from board %d:\r\n", board->slot_index+1);		
		chasm_pdu_print(pdu, len);
		npd_syslog_dbg("\r\n Receive packet length is %ld byte\r\n", len);
        npd_syslog_dbg("\r\n");
		
    }

    if(len < sizeof(chasm_pdu_head_t))
    {
        npd_syslog_err("TIPC packets error, lenght is %d\r\n", len);
        free(pdu);
        return;
    }

    head = (chasm_pdu_head_t *)pdu;

    if(ntohl(head->length) > CHASM_PACKET_MAX_LENGTH)
    {
        npd_syslog_err("The packet length %u is longer than allowed length.\r\n",
            head->length);
        free(pdu);
        return;
    }
    if(len < ntohl(head->length)-sizeof(chasm_pdu_head_t))
    {
        npd_syslog_err("TIPC packets error, length is %d, should be %d\r\n", len, 
            ntohl(head->length)-sizeof(chasm_pdu_head_t));
        free(pdu);
        return;
    }

    {
        int pdu_len = sizeof(chasm_pdu_head_t);
		buf = pdu + sizeof(chasm_pdu_head_t);
        while(pdu_len < ntohl(head->length))
        {
            tlv = (chasm_tlv_t *)buf;
			pktType = ntohs(tlv->type);
			pktLen = ntohs(tlv->length);
			
			if(chasm_debug_pkt)
    		{
				npd_syslog_dbg("chasm_pdu_recv<header>: Receive packet from board %d:\r\n", board->slot_index+1);		
				chasm_pdu_print(buf, pktLen);
				npd_syslog_dbg("\r\n process packet length is %d byte\r\n", pktLen);
		        npd_syslog_dbg("\r\n");
    		}			
			if(pktLen == 0)
			{
			    break;
			}
            switch(pktType)
            {
                case CHASM_PDU_TYPE_REG_REQ:
                    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                    {
						npd_syslog_dbg("BEGIN: tlv->type == CHASM_PDU_TYPE_REG_REQ BEGIN.\r\n");
						
                        chasm_lock();
                        state = localmoduleinfo->runstate;
                        evt = REGISTER_REQUEST;
                        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                            (localmoduleinfo, board, evt, buf);
                        chasm_unlock();

						npd_syslog_dbg("END: tlv->type == CHASM_PDU_TYPE_REG_REQ END.\r\n");
                    }
                    break;
                case CHASM_PDU_TYPE_REG_RES:
                    if(SYS_MODULE_ISMASTERACTIVE(board->slot_index))
                    {
						npd_syslog_dbg("BEGIN:tlv->type == CHASM_PDU_TYPE_REG_RES BEGIN.\r\n");
						
                        chasm_lock();
                        state = localmoduleinfo->runstate;
                        evt = REGISTER_RESPONSE;
                        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                            (localmoduleinfo, board, evt, buf);
                        chasm_unlock();

						npd_syslog_dbg("END: tlv->type == CHASM_PDU_TYPE_REG_RES END.\r\n");

                    }
					npd_syslog_dbg("workmode is MASTER %d , reduncy is ACTIVE %d, \r\n",
						(board->workmode == MASTER_BOARD), (board->redundancystate == MASTER_ACTIVE));
                    break;
                case CHASM_PDU_TYPE_STATUS:
					npd_syslog_dbg("BEGIN: tlv->type == CHASM_PDU_TYPE_STATUS .\r\n");

					
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = STATUS_REPORT;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();

					npd_syslog_dbg("END: tlv->type == CHASM_PDU_TYPE_STATUS END.\r\n");

                    break;
                case CHASM_PDU_TYPE_CMD:
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = MASTER_CMD;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();
                    break;
                case CHASM_PDU_TYPE_SWITCHOVER:
					npd_syslog_dbg("BEGIN: tlv->type == CHASM_PDU_TYPE_SWITCHOVER .\r\n");

                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = SWITCHOVER_REPORT;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();

					npd_syslog_dbg("END: tlv->type == CHASM_PDU_TYPE_SWITCHOVER .\r\n");

                    break;
                case CHASM_PDU_TYPE_QUERY:
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = QUERY;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();
                    break;
                case CHASM_PDU_TYPE_RESET:
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = RESET;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();
                    break;
                case CHASM_SUBBOARD_HWREMOVE:
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = HW_REMOVE;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();
                    break;
                default:
                    npd_syslog_err("Error chasm packets.\r\n");
                    break;
            }
			//npd_syslog_dbg("ORIGIN: pdu_len %d \r\n", pdu_len);

            pdu_len = pdu_len + pktLen;
            buf = buf + pktLen;
			
			//npd_syslog_dbg("NOW: pdu_len %d \r\n", pdu_len);

        }
    }
	//npd_syslog_dbg("free pdu .\r\n");
    free(pdu);
	npd_syslog_dbg("END: chasm_pdu_recv .\r\n");
    return;
}

long chasm_send_pdu(board_param_t *dst_board, char *pdu, int len)
{
    chasm_circle_data_t *data = &chasm_circle_data[dst_board->slot_index];
    int sock = data->sock;
    int ret;

    if(0 == sock)
        return 0;
	npd_syslog_dbg("Send packet to board %d:\r\n", dst_board->slot_index+1);
    if(chasm_debug_pkt)
    {
		chasm_pdu_print(pdu, len);
		npd_syslog_dbg("the length is %d byte\r\n", len);
        npd_syslog_dbg("\r\n");
    }
    ret = send(sock, pdu, len, 0);
    if(ret < 0)
    {
		data->timer_evt = TIPC_BREAK;

		circle_register_timeout(0, 0, chasm_timeout, data, NULL);
		/*此处有可能带来状态机嵌套问题，利用超时可以使逻辑简单化*/
#if 0		
        int evt;
        int state;
        //chasm_lock();
        npd_syslog_official_event("Remote board %d software connection is broken.", dst_board->slot_index+1);
        state = localmoduleinfo->runstate;
        evt = TIPC_BREAK;
        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, dst_board, evt, NULL);
        //chasm_unlock();
#endif        
        return NPD_FAIL;
    }
	return NPD_SUCCESS;
}

long chasm_broadcast_pdu(char *pdu, int len)
{
    board_param_t *board;
    int i;
	npd_syslog_dbg("broadcast_pdu.\r\n");
	
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        board = chassis_slots[i];
        if(board == localmoduleinfo)
            continue;
        if(board->runstate >= RMT_BOARD_REGISTERED
			&& board->runstate < RMT_BOARD_REMOVING)
        {
            chasm_send_pdu(board, pdu, len);
        }
    }
    return NPD_SUCCESS;
}


int chassis_sbymaster_connectto_board(board_param_t* board)
{
    struct sockaddr_tipc server_addr = {0};
    int fd;
    int ret;
    chasm_circle_data_t *data;

	if(board->workmode == MASTER_BOARD)
	{
		npd_syslog_dbg("board %d is MASTER_BOARD, don't connect.\r\n", board->slot_index+1);
		return NPD_FAIL;
	}
		

    data = &chasm_circle_data[board->slot_index];
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket!\r\n");
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAME;
    server_addr.addr.name.name.type = SERVER_CHASM_MANAGE;
    server_addr.addr.name.name.instance = board->slot_index+1;
    server_addr.addr.name.domain = 0;
	
    data->sock = fd;
    data->board = board;
    
    /* Only nonblock connect, when switch over check the connection whether ok*/
	npd_syslog_dbg("local sbymaster connect board %d\r\n", board->slot_index +1);
	npd_syslog_dbg("the circle_data board %d runstate is %d.\r\n", 
		board->slot_index +1, board->runstate);
    ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_err("Failed to connect to chasm server, current errno is %d : %s.\r\n", errno, strerror(errno));
        close(fd);
        data->sock = 0;
        return ret;
    }
    npd_syslog_dbg("Local standby master build internal control path.\r\n");     
    circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);

	return NPD_SUCCESS;
}
#endif
long chasm_config_board(int slot_id, int subslot_id, int board_type)
{
	struct board_param_s * board_ptr;
	int ret;
	
    if((slot_id == SYS_LOCAL_MODULE_SLOT_INDEX)
		&& (subslot_id == 0))
    {
        if(board_type != SYS_LOCAL_MODULE_TYPE)
            return BOARD_RETURN_CODE_CONFIG_ERROR;
    }	
	board_ptr = chassis_slots[slot_id]->sub_board[subslot_id];
	if(board_ptr == NULL)
	{
		return BOARD_RETURN_CODE_ERR_NONE;
	}
    if(board_ptr->runstate != RMT_BOARD_NOEXIST
		&& board_ptr->runstate != RMT_BOARD_HWINSERTED)
        return BOARD_RETURN_CODE_WRONG_STATE;

    if(board_ptr->online_removed != FALSE)
    {
	    npd_syslog_dbg("Status online_removed is %d!\n", board_ptr->online_removed);
        return BOARD_RETURN_CODE_WRONG_STATE;
    }

    if((board_type <= PPAL_BOARD_TYPE_NONE)
        || (board_type >= PPAL_BOARD_TYPE_MAX))
    {
		
	    npd_syslog_dbg("board_type is  %d ok!\n", board_type);

        return BOARD_RETURN_CODE_NO_SUCH_TYPE;
    }
	
    if(!chasm_board_support_product(board_type, SYS_PRODUCT_TYPE, slot_id, subslot_id))
    {
	    npd_syslog_dbg("Product %d is not support board type %d ok!\n", SYS_PRODUCT_TYPE, board_type);

        return BOARD_RETURN_CODE_NO_SUPPORT;
    }
		
    chasm_lock();
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)	
    if(slot_id != SYS_LOCAL_MODULE_SLOT_INDEX)
    {
        if(SYS_CHASSIS_ISMASTERSLOT(slot_id))
        {
            if((SYS_PRODUCT_TOPO == CENTRAL_FABRIC)
                &&(!MODULE_IS_MCU(board_type)))
            {
				ret = BOARD_RETURN_CODE_NOT_ACTMASTER;
                goto failcode;
            }
        }
        if(SYS_CHASSIS_ISSLAVESLOT(slot_id))
        {
            if(MODULE_IS_MCU(board_type))
            {
				ret = BOARD_RETURN_CODE_NOT_ACTMASTER;
                goto failcode;
            }
        }
        board_ptr->fix_param = module_basic_info[board_type];
        board_ptr->fix_spec_param = module_spec_info[board_type];
        board_ptr->online_removed = TRUE;
        board_ptr->configure_type = board_type;
#if 0	/*here is preconfig, the board is not exist, should not init connection and send status report*/	
        if(localmoduleinfo->fix_spec_param->system_conn_init)
            (*localmoduleinfo->fix_spec_param->system_conn_init)(SYS_PRODUCT_TYPE,
                    board_type, slot_id);
		/*  */
#endif
        if(npd_startup_end)
        {
    		strcpy(board_ptr->man_param.modname, board_ptr->fix_param->short_name);
        	{
                char * new_pdu;
                int len;
                
                chasm_assemble_status_report_pdu(chassis_slots[slot_id], 
                    board_ptr, &new_pdu, &len);
                chasm_broadcast_pdu(new_pdu, len);
                free(new_pdu);        
            } 
    		sleep(1);
        }
        chasm_subboard_init_eth_ports(chassis_slots[slot_id], board_ptr, PORT_ONLINE_REMOVED);
    }
    else
#endif		
    if(subslot_id == 0)
	{
		
		if (localmoduleinfo->runstate == LOCAL_MASTER_INIT)
		{
	        board_ptr->online_removed = TRUE;
	        chasm_subboard_init_eth_ports(chassis_slots[slot_id], 
	            board_ptr, PORT_NORMAL);		
		}
		else
		{
			ret = BOARD_RETURN_CODE_WRONG_STATE;
			goto failcode;
		}
    }
	else
	{
		if(board_ptr->fix_param)
		{
			if(board_ptr->fix_param->board_type != board_type)
			{
			    ret = BOARD_RETURN_CODE_WRONG_STATE;
				goto failcode;
			}
		}
        board_ptr->fix_param = module_basic_info[board_type];
        board_ptr->fix_spec_param = module_spec_info[board_type];
        board_ptr->online_removed = TRUE;
        board_ptr->configure_type = board_type;
		board_ptr->slot_index = subslot_id;
        chasm_subboard_init_eth_ports(chassis_slots[slot_id], board_ptr, PORT_ONLINE_REMOVED);		
	}
    
	chasm_unlock();	
	return BOARD_RETURN_CODE_ERR_NONE;
failcode:
	chasm_unlock();
	return ret;	
}

long chasm_no_board(int slot_id, int subslot_id)
{
	int ret = BOARD_RETURN_CODE_ERR_NONE;
	
    if((slot_id == SYS_LOCAL_MODULE_SLOT_INDEX)
		&& (subslot_id == 0))
        return BOARD_RETURN_CODE_WRONG_SLOTNO;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("runstate  is  %d ok!\n", chassis_slots[slot_id]->sub_board[subslot_id]->runstate);

    if(chassis_slots[slot_id]->sub_board[subslot_id]->runstate != RMT_BOARD_NOEXIST
		&& chassis_slots[slot_id]->sub_board[subslot_id]->runstate != RMT_BOARD_HWINSERTED)
        return BOARD_RETURN_CODE_WRONG_STATE;

	npd_syslog_dbg("online_removed is  %d ok!\n", chassis_slots[slot_id]->sub_board[subslot_id]->online_removed);

    if(chassis_slots[slot_id]->sub_board[subslot_id]->online_removed != TRUE)
        return BOARD_RETURN_CODE_WRONG_STATE;

    chasm_lock();

	npd_syslog_dbg("chasm online delete \r\n");
    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                chassis_slots[slot_id]->sub_board[subslot_id]->configure_type,
                slot_id);
	chasm_subboard_online_delete(chassis_slots[slot_id],
					chassis_slots[slot_id]->sub_board[subslot_id]);
    chassis_slots[slot_id]->sub_board[subslot_id]->configure_type = PPAL_BOARD_TYPE_NONE;
    chassis_slots[slot_id]->sub_board[subslot_id]->online_removed = FALSE;

    /*here sleep to wait for dbtable finish sync*/
    npd_syslog_cslot_event("\n%% Deleting board %d...\r\n", slot_id);
    sleep(1);	
	{
		//tell other board
        char * new_pdu;
        int len;
        
        chasm_assemble_status_report_pdu(chassis_slots[slot_id], 
            chassis_slots[slot_id]->sub_board[subslot_id], &new_pdu, &len);
        chasm_broadcast_pdu(new_pdu, len);
        free(new_pdu);
       
    } 
	chassis_slots[slot_id]->sub_board[subslot_id]->fix_param = NULL;
	chassis_slots[slot_id]->sub_board[subslot_id]->fix_spec_param = NULL;

	chasm_unlock();
#endif	
	return ret;
   
}

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
void wait_chasm_slave_server(int sd, void *circle_data, void *user_data)
{
    chasm_circle_data_t *data = (chasm_circle_data_t *)circle_data;
    board_param_t *board = data->board;
    int state;
    int evt;

    chasm_lock();
    state = board->runstate;
    evt = TIPC_CONNECT;
    (*(*(board->state_function)[state].funcs)[evt])
        (board, board, evt, NULL);
    chasm_unlock();
}

void chasm_timeout(void *circle_data, void *user_data)
{
    chasm_circle_data_t *data = (chasm_circle_data_t *)circle_data;
    board_param_t *board = data->board;
    int state, evt;

	evt = data->timer_evt;
	
	npd_syslog_dbg("TIMEOUT chasm_timeout:board %d , state %d\r\n",
		board->slot_index+1, board->runstate);
    chasm_lock();
    state = board->runstate;
    (*(*(board->state_function)[state].funcs)[evt])
        (board, board, evt, user_data);
    chasm_unlock();
	npd_syslog_dbg("TIMEOUT END\r\n");
	
}

long chasm_board_cancel_timeout(int slot_index)
{
	circle_cancel_timeout(chasm_timeout, &chasm_circle_data[slot_index], circle_ALL_CTX);
	return NPD_SUCCESS;
}
#endif
long board_state_event_error(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Chasm state error, the board %d in state %s receive event %d.\r\n",
        board->slot_index +1, ((board->state_function)[board->runstate]).description,
        event );
    return NPD_SUCCESS;
}

long rmt_board_noexist_hw_insert(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("\nRemote board %d is physically inserted.\r\n", board->slot_index+1);     
    board->inserted = TRUE;
    board->runstate = RMT_BOARD_HWINSERTED;
    board->led_status = 0;

    data->board = board;

    /*for dbtable sync flag*/
    npd_dbtable_slot_sync_begin(board->slot_index);
	
    return NPD_SUCCESS;
}

long rmt_board_noexist_sw_insert(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
   	struct sockaddr_tipc server_addr = {0};
    int fd;
    int ret;
    chasm_circle_data_t *data;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d is inserted.\r\n", board->slot_index+1);     
    data = &chasm_circle_data[board->slot_index];
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket!\r\n");
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAME;
    server_addr.addr.name.name.type = SERVER_CHASM_MANAGE;
    server_addr.addr.name.name.instance = board->slot_index+1;
    server_addr.addr.name.domain = 0;

    data->sock = fd;
    data->board = board;
    
    /* Make server available: */
    ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_cslot_err("\n%% Failed to connect to chasm server, ret is %d(%s)\n", errno, strerror(errno));
        close(fd);
		data->sock = 0;
        board->runstate = RMT_BOARD_NOEXIST;
        chassis_sys_reset_ext(board->slot_index);
        return NPD_FAIL;
    }

    npd_syslog_dbg("register sockfd %d chasm_pdu_recv to READ.\r\n", fd);
    circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
    circle_register_timeout(CHASM_REG_TIMEOUT, 0, chasm_timeout, data, NULL);

	npd_syslog_cslot_event("\n%% Remote board %d is registering...\r\n", board->slot_index+1);
    board->runstate = RMT_BOARD_REGISTERING;
#endif
    return NPD_SUCCESS;
    
}


long rmt_board_noexist_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;
		
		board->rmtstate[src_board->slot_index] = board_tlv->runstate;
		npd_syslog_dbg("board %d 's rmtstate %d is state %d.\r\n", board->slot_index +1,
			src_board->slot_index+1, board_tlv->runstate);
		return NPD_SUCCESS;
        /*must use register request report*/
        //return board_state_event_error(board, src_board, event, pdu);
    }
    else
    {
        /*only receive subboard 0 status report, other subboard will be received on
          other state*/
        chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;
		char *status_pdu;
		int len;

		npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
		npd_syslog_dbg("Board_tlv  type %d, runstate is %d.\r\n", board_tlv->board_type,board_tlv->runstate);


		if (board_tlv->runstate == RMT_BOARD_NOEXIST
			|| board_tlv->runstate == RMT_BOARD_HWINSERTED)
		{
            if(localmoduleinfo->fix_spec_param && localmoduleinfo->fix_spec_param->system_conn_deinit)
                (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                        board_tlv->board_type, board->slot_index);
            if(snros_system_param->product_pp_feature_set)
                (*snros_system_param->product_pp_feature_set)();

			if (TRUE == board_tlv->online_removed)
			{
				board->fix_param = module_basic_info[board_tlv->board_type];
				board->fix_spec_param = module_spec_info[board_tlv->board_type];
				//add board default name for show 
		        memcpy(board->man_param.modname, board->fix_param->short_name, 32);
			}
			else /* THIS is For cancel preconfig */
			{
				board->fix_param = NULL;
				board->fix_spec_param = NULL;				
			}
			board->online_removed = board_tlv->online_removed;

			npd_board_info_notify(board->slot_index);
			if (NULL != board->fix_param)  /*  */
			{
	            chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
           		chasm_send_pdu(chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX], status_pdu, len);	
			}
            /*here no need to delete fix_param*/
		}
		else if ((board_tlv->runstate >= RMT_BOARD_REGISTERED)
			&& (board_tlv->runstate < RMT_BOARD_REMOVING))
		{

			if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
            	return NPD_SUCCESS;

			board->fix_param = module_basic_info[board_tlv->board_type];
			board->fix_spec_param = module_spec_info[board_tlv->board_type];
			
			
            board->inserted = TRUE;
            board->rmtstate[SYS_MASTER_ACTIVE_SLOT_INDEX] = board_tlv->runstate;
			board->runstate = RMT_BOARD_READY;
			if (SYS_MASTER_ACTIVE_SLOT_INDEX == board->slot_index)
			{	/* active board report its status only once to standby */
				/* so it must tell its true state to slave */
				board->runstate = board_tlv->runstate;
			}
			
			board->online_removed = FALSE;
			//add board default name for show 
	        memcpy(board->man_param.modname, board->fix_param->short_name, 32);
			
            if(localmoduleinfo->fix_spec_param->system_conn_init)
                (*localmoduleinfo->fix_spec_param->system_conn_init)(SYS_PRODUCT_TYPE,
                        board_tlv->board_type, board->slot_index);

            if(snros_system_param->product_pp_feature_set)
                (*snros_system_param->product_pp_feature_set)();

			if((board_tlv->runstate == RMT_BOARD_RUNNING)
				&& SYS_LOCAL_MODULE_ISMASTERSTANDBY)
			{
                int ret;
				if (SYS_MASTER_ACTIVE_SLOT_INDEX != board->slot_index)
					ret = chassis_sbymaster_connectto_board(board);
                if(0 == ret)
                {
    				board->runstate = RMT_BOARD_RUNNING;
    				npd_syslog_cslot_event("\n%% Remote board %d is normally running.\r\n", board->slot_index+1);
                }
			} 
			else if (board_tlv->runstate == RMT_BOARD_RUNNING)
			{
				/* other board just update the status */
				board->runstate = board_tlv->runstate;
           	 	npd_syslog_cslot_event("\n%% Remote board %d is ready.\r\n", board->slot_index+1);
			}
			
			npd_board_info_notify(board->slot_index);
            chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
            chasm_send_pdu(chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX], status_pdu, len);
		}            
        return NPD_SUCCESS;
    }
#endif
    return NPD_FAIL;
}

state_event_func_t rmt_board_noexist_funcs = 
{
    &board_state_event_error,
    &rmt_board_noexist_hw_insert,
    &rmt_board_noexist_sw_insert,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_noexist_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long rmt_board_hwinserted_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(board->reset_times == 3)
    {
        npd_syslog_cslot_err("\nRemote board %d can not be connected.\r\n", board->slot_index+1);
        board->reset_times = 0;
        /*todo: build a snmp trap*/
        
        return NPD_SUCCESS;
    }
    else
    {
        int state = board->runstate;
        int evt = RESET;

        return (*(*(board->state_function)[state].funcs)[evt])
            (board, src_board, evt, NULL);
        
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_hwinserted_sw_insert(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_noexist_sw_insert(board, src_board, event, pdu);
}


long rmt_board_hwinserted_hw_remove(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d is removed.\r\n", board->slot_index+1);
    board->inserted = FALSE;
    board->led_status = 0;
    board->runstate = RMT_BOARD_NOEXIST;
#endif	
	return NPD_SUCCESS;
}


long rmt_board_hwinserted_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];

    data->board = board;
    board->reset_times++;
    board->led_status = 0;
    (*chassis_sys_reset_ext)(board->slot_index);
    circle_register_timeout(CHASM_CONNECT_TIMEOUT, 0, chasm_timeout,data, NULL);
	npd_syslog_cslot_event("\n%% Remote board %d expires wait connection timer.\r\n", data->board->slot_index+1);
#endif
	return NPD_SUCCESS;
}

long rmt_board_hwinserted_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return rmt_board_noexist_status_report(board, src_board, event, pdu);
}


state_event_func_t rmt_board_hwinserted_funcs = 
{
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_hwinserted_sw_insert,
    &rmt_board_hwinserted_hw_remove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_hwinserted_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_hwinserted_reset,
    &board_state_event_error
};

long rmt_board_swinserted_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
    int sd = data->sock;
    int state = board->runstate;
    int evt = RESET;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d expire the wait connect timer, will be rebooted.\r\n", board->slot_index+1);
    circle_unregister_sock(sd, EVENT_TYPE_READ);
    circle_unregister_sock(sd, EVENT_TYPE_WRITE);
    circle_unregister_sock(sd, EVENT_TYPE_EXCEPTION);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    close(sd);
    data->sock = 0;

    return (*(*(board->state_function)[state].funcs)[evt])
        (board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}


long rmt_board_swinserted_hw_remove(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    board->inserted = FALSE;
    board->led_status = 0;
    board->runstate = RMT_BOARD_NOEXIST;
    data = &(chasm_circle_data[board->slot_index]);

    npd_syslog_cslot_event("\n%% Remote board %d is removed.\r\n", board->slot_index);

    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    circle_unregister_sock(data->sock, EVENT_TYPE_WRITE);
    circle_unregister_sock(data->sock, EVENT_TYPE_EXCEPTION);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    close(data->sock);
    data->sock = 0;
#endif	
	return NPD_SUCCESS;
}

long rmt_board_swinserted_sw_remove(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_swinserted_hw_remove(board, src_board, event, pdu);
}

long rmt_board_swinserted_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    /*get board type and init board_param_t*/
    int error;
    unsigned int n;
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d connection is built.\r\n", board->slot_index+1);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    circle_unregister_sock(data->sock, EVENT_TYPE_WRITE);
    circle_unregister_sock(data->sock, EVENT_TYPE_EXCEPTION);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    n = sizeof(error);
    if(getsockopt(data->sock, SOL_SOCKET, SO_ERROR, &error, &n) < 0
        || error != 0)
    {
        int evt = RESET;
		int state = board->runstate;
        npd_syslog_dbg("Can not connect slave server. reset the slave board %d\r\n",
            board->slot_index+1);
        close(data->sock);
        data->sock = 0;

        (*(*(board->state_function)[state].funcs)[evt])
            (board, src_board, evt, NULL);
        return NPD_SUCCESS;
    }
    fcntl(data->sock, F_SETFL, data->flag);
	
    npd_syslog_dbg("Register CHASM_REG_TIMEOUT to board %d", data->board->slot_index+1);
    circle_register_timeout(CHASM_REG_TIMEOUT, 0, chasm_timeout, data, NULL);
    circle_register_sock(data->sock, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
    board->runstate = RMT_BOARD_REGISTERING;
    npd_syslog_cslot_event("\n%% Remote board %d is registering...\r\n", board->slot_index+1);
#endif	
	return NPD_SUCCESS;
}

long rmt_board_swinserted_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
	char *status_pdu;
	int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(data->sock)
    {
		npd_syslog_dbg("close the sock fd %d.\r\n", data->sock);
        circle_unregister_sock(data->sock, EVENT_TYPE_READ);
        circle_unregister_sock(data->sock, EVENT_TYPE_WRITE);
        circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
        close(data->sock);
        data->sock = 0;
    }
	
    if(CHASSIS_SLOT_INSERTED(board->slot_index) &&
		board->runstate != RMT_BOARD_SW_VERERR)
    {
        board->inserted = TRUE;
        board->runstate = RMT_BOARD_HWINSERTED;
		circle_register_timeout(CHASM_CONNECT_TIMEOUT,  0, chasm_timeout, data, NULL);
    }
    else
    {
        board->inserted = FALSE;
        board->runstate = RMT_BOARD_NOEXIST;
    }

	if (board->fix_param)
	{
	    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
	    chasm_broadcast_pdu(status_pdu, len);
	    free(status_pdu);
	}
	
#endif	
    return NPD_SUCCESS;
        
}

long rmt_board_swinserted_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return rmt_board_noexist_status_report(board, src_board, event, pdu);
}



state_event_func_t rmt_board_swinserted_funcs = 
{
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_hwinserted_hw_remove,
    &rmt_board_swinserted_sw_remove,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_swinserted_tipc_connect,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_swinserted_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_hwinserted_reset,
    &board_state_event_error
};


long rmt_board_registering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
    int sd = data->sock;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(sd)
    {
        circle_unregister_sock(sd, EVENT_TYPE_READ);
        circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
        close(sd);
        data->sock = 0;
    }

    npd_syslog_cslot_event("\n%% Remote board %d expired the time for registering and will be reset.\r\n", board->slot_index+1);
#endif	
    return rmt_board_swinserted_reset(board, src_board, RESET, NULL);

}


long rmt_board_registering_hw_remove(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;
	char *status_pdu;
	int len = 0;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    board->inserted = FALSE;
    board->led_status = 0;
    board->runstate = RMT_BOARD_NOEXIST;
    data = &(chasm_circle_data[board->slot_index]);

    npd_syslog_cslot_event("\n%% Remote board %d is removed.\r\n", board->slot_index+1);

    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    close(data->sock);
    data->sock = 0;

	if (board->fix_param)
	{
	    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
	    chasm_broadcast_pdu(status_pdu, len);
	    free(status_pdu);
	}	
#endif
	return NPD_SUCCESS;
}

long rmt_board_registering_sw_remove(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d is connection is break.\r\n", board->slot_index+1);
    return rmt_board_registering_timer(board, src_board, TIMER, NULL);
}

long rmt_board_registering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d connection is break.\r\n", board->slot_index+1);
    
    return rmt_board_registering_timer(board, src_board, TIMER, NULL);
}

long rmt_board_subboard_reg_req(board_param_t *board, board_param_t *src_board, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_reginfo_tlv_t *sub_tlv = chasm_board_reginfo_tlv_ntoh((chasm_board_reginfo_tlv_t *)pdu);
    chasm_ams_reginfo_tlv_t *ams_tlv = chasm_board_asm_reginfo_tlv_ntoh((chasm_ams_reginfo_tlv_t *)(sub_tlv+1));
    int i;
	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Remote board %d register request received\r\n", 
        board->slot_index+1);
    npd_syslog_dbg("Remote board type is %d\r\n", 
            sub_tlv->board_type);

	if(chasm_debug_pkt)
    {
		chasm_pdu_print(pdu, sub_tlv->length);
		npd_syslog_dbg("the length board_reginfo is %d byte\r\n", sub_tlv->length);
        npd_syslog_dbg("\r\n");
    }
#if 0
    if(chasm_version_err((char*)sub_tlv->sw_version))
    {
        board->runstate = RMT_BOARD_SW_VERERR;
        npd_syslog_cslot_event("\n%% Remote board software version is error and is upgrading... \r\n", board->slot_index+1);
        circle_register_timeout(CHASM_SWVERERR_TIMEOUT, 0, chasm_timeout, 
            &chasm_circle_data[board->slot_index], NULL);
        board->fix_param->os_upgrade(board->slot_index);
		return NPD_SUCCESS;
    }
#endif
    if(0 != sub_tlv->subslot_id)
    {
        npd_syslog_cslot_err("\nError, the remote board %d subboard %d register without mother board info.\r\n", 
            sub_tlv->slotid+1, sub_tlv->subslot_id);
        return NPD_FAIL;
    }
    else if(!chasm_board_support_product(sub_tlv->board_type, SYS_PRODUCT_TYPE, sub_tlv->slotid, sub_tlv->subslot_id))
    {
        npd_syslog_official_event("Remote board %d is not supported by this product.\r\n",
                                         src_board->slot_index+1);
        rmt_board_registering_sw_remove(board, src_board, SW_REMOVE, pdu);
        return NPD_SUCCESS;
    }
    else
    {
        if(board->online_removed == TRUE)
        {
            if(board->fix_param->board_type != sub_tlv->board_type)
            {
                npd_syslog_official_event("\nRemote board %d board type is not same with preset type.\r\n",
                  board->slot_index);
                npd_syslog_official_event("\nDeleting all configuration related preset board...\r\n");
                chasm_subboard_online_delete(board, board);
				board->online_removed = FALSE;
                if(localmoduleinfo->fix_spec_param->system_conn_deinit)
                    (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                            board->fix_param->board_type, board->slot_index);
                if(snros_system_param->product_pp_feature_set)
                    (*snros_system_param->product_pp_feature_set)();

            }
			npd_syslog_dbg("board %d 's online_removed is TRUE.\r\n", board->slot_index+1);
        }
        
        board->fix_param = module_basic_info[sub_tlv->board_type];
        board->fix_spec_param = module_spec_info[sub_tlv->board_type];
        board->configure_type = sub_tlv->board_type;
        
#if 0
       /*when init eth ports here, if the other board have not the information of this board
            an error will occurs*/
        if(board->online_removed == FALSE)
        {
            chasm_subboard_init_eth_ports(board, board, PORT_NORMAL);
        }
        board->online_removed = FALSE; 
#endif        
        npd_syslog_cslot_event("\n%% Remote board type is %s\r\n", 
            sub_tlv->board_short_name);
        if(localmoduleinfo->fix_spec_param->system_conn_init)
            (*localmoduleinfo->fix_spec_param->system_conn_init)(SYS_PRODUCT_TYPE,
                    sub_tlv->board_type, board->slot_index);
        if(snros_system_param->product_pp_feature_set)
            (*snros_system_param->product_pp_feature_set)();
        
        
    }
	
    {

		/* configure the board's ams param.		  */
        for(i = 0; i < MAX_ASIC_NUM_PER_BOARD; i++)
        {
            int j;
            if(CHASM_PDU_TYPE_AMS_REG != ams_tlv->type)
            {
				break;
            }
			
            for(j = ASIC_SWITCH_TYPE; j < ASIC_TYPE_MAX; j++)
            {
				ams_info_t *ams = NULL;
                if(ams_tlv->ams_type != j)
                    continue;
                ams = &board->ams_info[j][i];
                npd_syslog_dbg("ASIC type %d, ASIC id %d\r\n", j, i);
                strncpy((char*)ams->name, (char*)ams_tlv->name, 31);
                ams->index = i;
                ams->dev_id = ams_tlv->vender_id;
                ams->rev_id = ams_tlv->revid;
                ams_tlv++;
                 break;
            }
        }
/*
        for(i = 0; i < board->fix_spec_param->temper_fix_param->num; i++)
        {
            board->temperature[i].name 
                = board->fix_spec_param->temper_fix_param->name[i];
        }
*/
        memcpy(board->man_param.modname, sub_tlv->board_short_name, 32);
        memcpy(board->man_param.sn, sub_tlv->sn, 32);
    } 

    npd_syslog_cslot_event("\n%% Remote board %d is registered.\r\n", board->slot_index+1);
	npd_syslog_dbg("Remote board %d rmtstate %d move to RMT_BOARD_REGISTERED.\r\n", 
		board->slot_index+1, src_board->slot_index + 1);
    {
        char * new_pdu;
        int len;
		
	    board->runstate = RMT_BOARD_REGISTERED;
        board->rmtstate[src_board->slot_index] = RMT_BOARD_REGISTERED;
        chasm_assemble_reg_res_pdu(board, board, src_board, &new_pdu, &len);
        chasm_send_pdu(src_board, new_pdu, len);
        free(new_pdu);

        chasm_assemble_status_report_pdu(board, board, &new_pdu, &len);
        chasm_broadcast_pdu(new_pdu, len);
        free(new_pdu);

		/* 
        circle_register_timeout(CHASM_BOARD_ALLREADY_TIMEOUT, 0, chasm_timeout, 
            &chasm_circle_data[board->slot_index], NULL);
		*/

        return NPD_SUCCESS;
    }
#endif
    return NPD_SUCCESS;           
}

long rmt_board_registering_reg_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_subboard_reg_req(board, src_board, pdu);
}

long rmt_board_registering_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    /*we can not confirm that the board have done all things, so the better 
          way is reset the board*/
   	if (board->runstate >= RMT_BOARD_READY && 
		board->runstate < RMT_BOARD_REMOVING)
   	{
		board->online_removed = TRUE;
	}
    board->runstate = RMT_BOARD_NOEXIST;
#endif    
    return NPD_SUCCESS;
}


long rmt_board_registering_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registering_timer(board, src_board, TIMER, NULL);
}


state_event_func_t rmt_board_registering_funcs = 
{
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_registering_hw_remove,
    &rmt_board_registering_sw_remove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_registering_tipc_break,
    &rmt_board_registering_reg_request,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_registering_switchover_report,
    &board_state_event_error,
    &rmt_board_registering_reset,
    &board_state_event_error
};


long rmt_board_registered_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    board->runstate = RMT_BOARD_READY;
	npd_syslog_dbg("Remote board %d move to RMT_BOARD_READY\r\n", board->slot_index+1);
    chasm_assemble_status_report_pdu(board, src_board, &status_pdu, &len);
    chasm_broadcast_pdu(status_pdu, len);
    free(status_pdu);
    npd_syslog_cslot_event("\n%% Remote board %d expire registered timer.\r\n", board->slot_index+1);
    if(board->online_removed == FALSE)
        chasm_subboard_init_eth_ports(board, board, PORT_NORMAL);
    chasm_subboard_online_insert(board, board);
	board->online_removed = FALSE;
#endif
	return NPD_SUCCESS;
}

long rmt_board_registered_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	board->runstate = RMT_BOARD_NOEXIST;
	if(chassis_sys_reset_ext)
	    (*chassis_sys_reset_ext)(board->slot_index);
	return 0;
}

long rmt_board_registered_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;
    char *status_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
#if 0
    chasm_subboard_hwremove_tlv_t *tlv;
    board_param_t *sub_board;

    if(pdu != NULL)
    {
        int state;
        int evt;
        tlv = (chasm_subboard_hwremove_tlv_t *)pdu;
        sub_board = board->sub_board[tlv->subslot_id];
        if(NULL == sub_board)
        {
            npd_syslog_err("Receive sub board %d-%d remove message when it not exist.\r\n",
                 board->slot_index, sub_board->slot_index);
        }
        state = sub_board->runstate;
        evt = HW_REMOVE;
        (*((sub_board->state_function)[state].funcs)[evt])
            (sub_board, evt, NULL);
        
    }
    else
#endif
    {
        if(localmoduleinfo->fix_spec_param->system_conn_deinit)
            (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                    board->fix_param->board_type,
                    board->slot_index);
        if(snros_system_param->product_pp_feature_set)
            (*snros_system_param->product_pp_feature_set)();
        
        
        board->inserted = FALSE;
	   	if (board->runstate >= RMT_BOARD_READY && 
			board->runstate < RMT_BOARD_REMOVING)
	   	{
			board->online_removed = TRUE;
		}	
        board->led_status = 0;
        board->runstate = RMT_BOARD_REMOVING;
        npd_syslog_cslot_event("\n%% Remote board %d is removing...\r\n", board->slot_index+1);
        if(board->mother_board == board)
        {
            data = &(chasm_circle_data[board->slot_index]);
			npd_syslog_dbg("close the data sock %d.\r\n", data->sock);

            circle_unregister_sock(data->sock, EVENT_TYPE_READ);
            circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
            close(data->sock);
            data->sock = 0;		
        }
    
        chasm_subboard_online_remove(board, board);
        chasm_assemble_status_report_pdu(board->mother_board, board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);

		npd_syslog_official_event("\nRemote board %d is removed.\r\n", board->slot_index+1);
		board->runstate = RMT_BOARD_NOEXIST;
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_registered_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	/*   */
	if(board->online_removed)
	{
        if(board->mother_board == board)
        {
			struct chasm_circle_data_s* data = NULL;
			
            data = &(chasm_circle_data[board->slot_index]);
			npd_syslog_dbg("close the data sock %d.\r\n", data->sock);
			if (data && data->sock)
			{
	            circle_unregister_sock(data->sock, EVENT_TYPE_READ);
	            circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
	            close(data->sock);
	            data->sock = 0;	
			}
        }		
        return NPD_SUCCESS;
	}
	
	/* ye sljlk */
    //if(CHASSIS_SLOT_INSERTED(board->slot_index)) /* */
    if (0)
    {
        int state = board->runstate;
        int evt = RESET;
        (*(*(board->state_function)[state].funcs)[evt])
            (board, src_board, evt, NULL);
        board->runstate = RMT_BOARD_REMOVING;
        npd_syslog_cslot_event("\n%% Remote board %d is removing...\r\n", board->slot_index+1);
        return NPD_SUCCESS;
    }
    else
    {
        return rmt_board_registered_hwremove(board, src_board, HW_REMOVE, NULL);
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_registered_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_swremove(board, src_board, SW_REMOVE, NULL);
}

long rmt_board_registered_reg_req(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_reginfo_tlv_t *sub_tlv = chasm_board_reginfo_tlv_ntoh((chasm_board_reginfo_tlv_t *)pdu);
    
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(0 == sub_tlv->subslot_id)
    {
        npd_syslog_err("Error, the remote board %d subboard register on wrong state.\r\n", 
            sub_tlv->slotid, sub_tlv->subslot_id);
        return NPD_FAIL;
    }
#if 0
    sub_board = board->sub_board[sub_tlv->subslot_id];
    if(NULL == sub_board)
    {
        sub_board = malloc(sizeof(board_param_t));
        if(NULL == sub_board)
        {
            npd_syslog_err("Failed to alloc memory for chasm board % sub-board % management.\r\n",
                      board->slot_index, sub_tlv->subslot_id);
        }
        memset(sub_board, 0, sizeof(board_param_t));
        sub_board->slot_index = sub_tlv->subslot_id;
        sub_board->workmode = SUB_BOARD;
        sub_board->runstate = RMT_BOARD_REGISTERED;
        sub_board->state_function = rmt_board_state_desc;
        sub_board->mother_board = board;
    }

    if(sub_board->online_removed == TRUE)
    {
        if(sub_board->fix_param->board_type != sub_tlv->board_type)
        {
            npd_syslog_dbg("The board %d register request board type is not same as removed one.\n",
              board->slot_index);
            chasm_subboard_online_delete(board, sub_board);
            sub_board->online_removed = FALSE;
            sub_board->fix_param = module_basic_info[sub_tlv->board_type];
        }
    }
    
    {
        for(i = ASIC_SWITCH_TYPE; i < ASIC_TYPE_MAX; i++)
        {
            int j;
			if (NULL == board->fix_param->ams_param[i]) //for fix the bug that ams_tlv point out of boundary.
				continue;

			
            if(ams_tlv->ams_type != i)
                continue;
            for(j = 0; j < sub_board->fix_param->ams_param[i]->num; j++)
            {
                ams_info_t *ams = &sub_board->ams_info[i][j];
                strncpy((char*)ams->name, (char*)ams_tlv->name, 31);
                ams->index = j;
                ams->dev_id = ams_tlv->vender_id;
                ams->rev_id = ams_tlv->revid;
/*                ams->pci_id = ams_tlv->pci_id;*/
                ams_tlv++;
            }
        }

        for(i = 0; i < sub_board->fix_param->temper_fix_param->num; i++)
        {
            sub_board->temperature[i].name 
                = sub_board->fix_param->temper_fix_param->name[i];
        }
        
/*        memcpy(sub_board->man_param.modname, sub_tlv->subboard_full_name, 32);*/
/*        memcpy(sub_board->man_param.hw_version, sub_tlv->hw_version, 32);*/
        memcpy(sub_board->man_param.sn, sub_tlv->sn, 32);
    } 

	npd_syslog_dbg("board %d move to RMT_BOARD_REGISTERED",board->slot_index+1);

    board->runstate = RMT_BOARD_REGISTERED;
    board->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_READY;
    {
        char * new_pdu;
        int len;
        chasm_assemble_reg_res_pdu(board, board, src_board, &new_pdu, &len);
        chasm_send_pdu(board, new_pdu, len);
        free(new_pdu);

        chasm_assemble_status_report_pdu(board, board, &new_pdu, &len);
        chasm_broadcast_pdu(new_pdu, len);
        free(new_pdu);
		
        circle_register_timeout(CHASM_BOARD_ALLREADY_TIMEOUT, 0, chasm_timeout, 
            &chasm_circle_data[board->slot_index], NULL);
		npd_syslog_dbg("register_timeout CHASM_BOARD_ALLREADY_TIMEOUT to  board %d",
			board->slot_index+1);


        return NPD_SUCCESS;
    }
#endif
#endif
    return NPD_SUCCESS;
    
}

long rmt_board_registered_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return board_state_event_error(board, src_board, event, pdu);
    else
    {
        chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;

        npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
		npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);
        if((NULL == board->fix_param)
            || (board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
            )
        {
            npd_syslog_err("Receive error status for board %d.\r\n",
                 board_tlv->slotid);
            return NPD_FAIL;
        }
		
		npd_syslog_dbg("Remote board %d , src_board is %d.\r\n", board->slot_index+1, src_board->slot_index+1);

        if(board_tlv->runstate == RMT_BOARD_SW_VERERR)
        {
            board->runstate = RMT_BOARD_SW_VERERR;
            npd_syslog_cslot_event("\n%% Remote board software version is error and is upgrading... \r\n", board->slot_index+1);
            circle_register_timeout(CHASM_SWVERERR_TIMEOUT, 0, chasm_timeout, 
                &chasm_circle_data[board->slot_index], NULL);
			board->fix_param->os_upgrade(board->slot_index);
    		return NPD_SUCCESS;
        }
        
        {
			board->rmtstate[src_board->slot_index] = board_tlv->runstate;
			npd_syslog_dbg("board %d 's rmtstate %d is state %d.\r\n", board->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);
		}
           
        return NPD_SUCCESS;
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_registered_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    /*we can not confirm that the board have done all things, so the better 
          way is reset the board*/
   	if (board->runstate >= RMT_BOARD_READY && 
		board->runstate < RMT_BOARD_REMOVING)
   	{
		board->online_removed = TRUE;
	}          
    board->runstate = RMT_BOARD_NOEXIST;
    chasm_subboard_online_remove(board, board);    
	if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                           board->fix_param->board_type, board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
    
    chasm_subboard_online_remove(board, board);
    chassis_sys_reset_ext(board->slot_index);
#endif
    return NPD_SUCCESS;
}


long rmt_board_registered_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
    char *status_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	if (data->sock)
	{
		npd_syslog_dbg("close the sock fd %d\r\n", data->sock);
		circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    	circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    	close(data->sock);
		data->sock = 0;

	}
    memset(data, 0, sizeof(chasm_circle_data_t));
   	if (board->runstate >= RMT_BOARD_READY && 
		board->runstate < RMT_BOARD_REMOVING)
   	{
		board->online_removed = TRUE;
	}
    chasm_subboard_online_remove(board, board);
    rmt_board_swinserted_reset(board, src_board, RESET, NULL);

	npd_syslog_dbg("Remote board %d move to RMT_BOARD_REMOVING\r\n", board->slot_index+1);
	board->runstate = RMT_BOARD_REMOVING;

    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
    chasm_broadcast_pdu(status_pdu, len);
    free(status_pdu);
    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                           board->fix_param->board_type, board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
    
    board->runstate = RMT_BOARD_NOEXIST;
#endif	
	return NPD_SUCCESS;
}

state_event_func_t rmt_board_registered_funcs = 
{
    &rmt_board_registered_timer,
    &board_state_event_error,
    &rmt_board_registered_swinsert,
    &rmt_board_registered_hwremove,
    &rmt_board_registered_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_registered_tipc_break,
    &rmt_board_registering_reg_request,
    &board_state_event_error,
    &rmt_board_registered_status_report,
    &board_state_event_error,
    &rmt_board_registered_switchover_report,
    &board_state_event_error,
    &rmt_board_registered_reset,
    &board_state_event_error
};

long rmt_board_sw_vererr_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registering_timer(board, src_board, event, pdu);
}

long rmt_board_sw_vererr_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registering_hw_remove(board, src_board, event, pdu);
}

long rmt_board_sw_vererr_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registering_sw_remove(board, src_board, event, pdu);
}

long rmt_board_sw_vererr_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registering_reset(board, src_board, event, pdu);
}

state_event_func_t rmt_board_sw_vererr_funcs = 
{
    &rmt_board_sw_vererr_timer,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_sw_vererr_hwremove,
    &rmt_board_sw_vererr_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_sw_vererr_reset,
    &board_state_event_error
};

long rmt_board_ready_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return NPD_SUCCESS;
}

long rmt_board_ready_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if 0	
   	struct sockaddr_tipc server_addr;
    int fd;
    int ret;
    chasm_circle_data_t *data;


	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!board->online_removed)
    {
        return NPD_SUCCESS;
    }
    npd_syslog_dbg("Remote board %d is logically inserted.\r\n", board->slot_index+1);     
    data = &chasm_circle_data[board->slot_index];
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket!\r\n");
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAME;
    server_addr.addr.name.name.type = SERVER_CHASM_MANAGE;
    server_addr.addr.name.name.instance = board->slot_index+1;
    server_addr.addr.name.domain = 0;

    data->flag = fcntl(fd, F_GETFL, 0);
	/*
    fcntl(fd, F_SETFL, data->flag|O_NONBLOCK);
    */

    data->sock = fd;
    data->board = board;
    
    /* Make server available: */
    ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        if(errno != EINPROGRESS)
        {
            npd_syslog_err("Failed to connect to chasm server, ret is %d", errno);
            close(fd);
			data->sock = 0;
            return NPD_FAIL;
        }

        circle_register_sock(fd, EVENT_TYPE_READ, wait_chasm_slave_server, data, NULL);
        circle_register_sock(fd, EVENT_TYPE_WRITE, wait_chasm_slave_server, data, NULL);
        circle_register_timeout(MAX_CONNECT_CHASM_SLAVE_TIME, 0, 
                 chasm_timeout, data, NULL);
		npd_syslog_dbg("register_timeout MAX_CONNECT_CHASM_SLAVE_TIME to  board %d",
			data->board->slot_index+1);

        return NPD_SUCCESS;
        

    }
    fcntl(fd, F_SETFL, data->flag);
    board->inserted = TRUE;
    circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
#endif    
    return NPD_SUCCESS;
}

long rmt_board_ready_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_hwremove(board, src_board, event, pdu);
}

long rmt_board_ready_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_swremove(board, src_board, event, pdu);
}

long rmt_board_ready_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    /*in stanby master, the rmt line board have only two state, READY or NOEXIST,
    */
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        return NPD_SUCCESS;
    }
    return rmt_board_ready_swremove(board, src_board, SW_REMOVE, pdu);
}

long rmt_board_ready_req_reg(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(board->online_removed == FALSE)
        return NPD_SUCCESS;
#endif	
    return rmt_board_registering_reg_request(board, src_board, event, pdu);
}
long rmt_board_ready_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;

    chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;
    if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
        return NPD_SUCCESS;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
	npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);

	
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
		npd_syslog_dbg("Local board is not MASTERACTIVE\r\n");
		npd_syslog_dbg("board %d rmtstate %d is %d\r\n", board->slot_index+1,
			SYS_MASTER_ACTIVE_SLOT_INDEX+1, board_tlv->runstate);
        board->rmtstate[SYS_MASTER_ACTIVE_SLOT_INDEX] = board_tlv->runstate;
                  
        if((board_tlv->runstate == RMT_BOARD_REMOVING)           
            ||(board_tlv->runstate == RMT_BOARD_NOEXIST)
            ||(board_tlv->runstate == RMT_BOARD_HWINSERTED))
        {
            chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
            if(data->sock && (SYS_MASTER_ACTIVE_SLOT_INDEX !=board->slot_index))
            {
				/* active slot socket fd can't be closed here. It will lose the tipc break state transition */
				/* and that will make master switch error */
				circle_unregister_sock(data->sock, EVENT_TYPE_READ);
            	circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);

				npd_syslog_dbg("Close sock fd %d to Remote board %d.\r\n", data->sock, board->slot_index+1 );
				close(data->sock);
                data->sock = 0;
            }
			npd_syslog_dbg("Remote board %d state move to NOEXIST.\r\n", board->slot_index+1);
            board->runstate = RMT_BOARD_NOEXIST;
			board->online_removed = TRUE;
            if(localmoduleinfo->fix_spec_param->system_conn_deinit)
                (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                        board->fix_param->board_type,
                        board->slot_index);
            if(snros_system_param->product_pp_feature_set)
                (*snros_system_param->product_pp_feature_set)();
            
            chasm_assemble_status_report_pdu(board, board,&status_pdu, &len);
            chasm_send_pdu(src_board, status_pdu, len);
            free(status_pdu);
            if(board_tlv->online_removed == FALSE)
			{
			    board->online_removed = FALSE;
			    board->fix_param = NULL;
			    board->fix_spec_param = NULL;
		    }
			npd_board_info_notify(board->slot_index);
			
        }

		if (board_tlv->runstate == RMT_BOARD_RUNNING)
		{
			if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)
			{
	            int ret = 0;
				if (SYS_MASTER_ACTIVE_SLOT_INDEX != board->slot_index)
	            	ret = chassis_sbymaster_connectto_board(board);
	            if(0 == ret)
	            {
	    			board->runstate = RMT_BOARD_RUNNING;
	                npd_syslog_dbg("Remote board %d state move to RUNNING.\r\n", board->slot_index+1);

	    			chasm_assemble_status_report_pdu(board, board,&status_pdu, &len);
	                chasm_send_pdu(src_board, status_pdu, len);
	            }
			}
			else
			{
				board->runstate = RMT_BOARD_RUNNING;
	            npd_syslog_dbg("Remote board %d state move to RUNNING.\r\n", board->slot_index+1);				
			}
			npd_board_info_notify(board->slot_index);
			
		}
		return NPD_SUCCESS;
    }
    else
    {
        char *status_pdu;
        int len;

		npd_syslog_dbg("Local board is MASTERACTIVE\r\n");

        if(NULL == board->fix_param)
        {
            npd_syslog_cslot_err("\n%% Receive error status for board %d.\r\n",
                 board->slot_index);
            return NPD_FAIL;
        }
        if(board != src_board)
        {
			board->rmtstate[src_board->slot_index] = board_tlv->runstate;
			npd_syslog_dbg("board %d rmtstate %d is %d\r\n", board->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);
		}
        else
        {
            if(board_tlv->runstate == RMT_BOARD_RUNNING)
            {
				npd_syslog_dbg("board %d move to RMT_BOARD_RUNNING\r\n", board->slot_index+1);
				
                board->runstate = RMT_BOARD_RUNNING;
                chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
                chasm_broadcast_pdu(status_pdu, len);
                free(status_pdu);
				
				/* This pdu is to tell the sby master about active mod name */
				if (SYS_MODULE_ISMASTERSTANDBY(board->slot_index))
				{
					char *reg_pdu;
    				int len;
					
			        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
			        chasm_send_pdu(board, reg_pdu, len);
			        free(reg_pdu);
				}
				npd_key_database_lock();
				netif_notify_event(board->slot_index, NOTIFIER_SLOT_INSERT);
				netif_app_notify_event(board->slot_index, NOTIFIER_SLOT_INSERT, NULL, 0);	
				npd_key_database_unlock();
            }
            else
            {
				npd_syslog_dbg("board %d: board_tlv->runstate != RMT_BOARD_RUNNING\r\n", board->slot_index+1);
            }
        }
        
        return NPD_SUCCESS;
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_ready_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    /*we can not confirm that the board have done all things, so the better 
          way is reset the board*/
    board->runstate = RMT_BOARD_NOEXIST;
    board->online_removed = TRUE;
    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                           board->fix_param->board_type, board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
    
    chasm_subboard_online_remove(board, board);
    chassis_sys_reset_ext(board->slot_index);
#if 0
	if((data->sock == 0) && (!SYS_CHASSIS_ISMASTERSLOT(board->slot_index))
        ret = chassis_sbymaster_connectto_board(board);
    if(0 != ret)
    {
		npd_syslog_dbg("board %d connect fail. move to NOEXIST, but online_removed is true", i +1);	
        ret = chassis_sys_reset_ext(board->slot_index);
		chassis_slots[i]->runstate = RMT_BOARD_NOEXIST;
		chassis_slots[i]->online_removed= TRUE;
        return 0;
    }
    else
    {
		npd_syslog_dbg("board %d move to SWITCHOVERING", i +1);	
		chassis_slots[i]->runstate = RMT_BOARD_SWITCHOVERING;
	}
#endif 
#endif
    return NPD_SUCCESS;
}


long rmt_board_ready_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_reset(board, src_board, event, pdu);
}

state_event_func_t rmt_board_ready_funcs =
{
    &rmt_board_ready_timer,
    &board_state_event_error,
    &rmt_board_ready_swinsert,
    &rmt_board_ready_hwremove,
    &rmt_board_ready_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_ready_tipc_break,
    &rmt_board_ready_req_reg,
    &board_state_event_error,
    &rmt_board_ready_status_report,
    &board_state_event_error,
    &rmt_board_ready_switchover_report,
    &board_state_event_error,
    &rmt_board_ready_reset,
    &board_state_event_error
};

long rmt_board_error_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long rmt_board_error_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long rmt_board_error_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long rmt_board_error_req_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long rmt_board_error_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t rmt_board_error_funcs =
{
    &rmt_board_error_timer,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_error_hwremove,
    &rmt_board_error_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_error_reset,
    &board_state_event_error
};


long rmt_board_removing_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if 0
    char *status_pdu;
    int len;
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_event("\nRemote board %d expires removing timer.\r\n", board->slot_index+1);
    circle_cancel_timeout(chasm_timeout, data, NULL);
	
	npd_syslog_event("\nRemote board %d is removed.\r\n", board->slot_index+1);
    board->runstate = RMT_BOARD_NOEXIST;
    
    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
    chasm_broadcast_pdu(status_pdu, len);
	free(status_pdu);
#endif
	
	return NPD_SUCCESS;
}

long rmt_board_removing_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if 0
    chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
	npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);

    if((NULL == board->fix_param)
        || (board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
        )
    {
        npd_syslog_err("Receive error status for board %d, subboard %d.\r\n",
             board_tlv->slotid, board_tlv->subslot_id);
        return NPD_FAIL;
    }

    if(board != src_board)
    {
		board->rmtstate[src_board->slot_index] = board_tlv->runstate;
		npd_syslog_dbg("board %d rmtstate %d is %d\r\n", board->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);

	}
        

	/*
    if(chasm_board_isremoved(board))
    {
        board->runstate = RMT_BOARD_NOEXIST;
        npd_syslog_dbg("Remote board %d state move to NOEXIST.\r\n", board->slot_index);
        chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);
    }
    */
#endif    
	return NPD_SUCCESS;
}

long rmt_board_removing_sw_insert(board_param_t *board, board_param_t *src_board, state_event_e event, char* pdu)
{
#if 0    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return rmt_board_noexist_sw_insert(board, src_board, event, pdu);
#endif
    return NPD_SUCCESS;
}


state_event_func_t rmt_board_removing_funcs =
{
    &rmt_board_removing_timer,
    &board_state_event_error,
    &rmt_board_removing_sw_insert,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_removing_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};


long rmt_board_running_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_hwremove(board, src_board, event, pdu);
}

long rmt_board_running_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_swremove(board, src_board, event, pdu);
}

long rmt_board_running_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return NPD_SUCCESS;
}

long rmt_board_running_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;

    chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;
	
    if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
        return NPD_SUCCESS;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
	npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);

	
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
		npd_syslog_dbg("Local board is not MASTERACTIVE\r\n");
        board->rmtstate[SYS_MASTER_ACTIVE_SLOT_INDEX] = board_tlv->runstate;
                  
        if(board_tlv->runstate == RMT_BOARD_REMOVING)
        {
            chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
			
            if(data->sock && (SYS_MASTER_ACTIVE_SLOT_INDEX !=board->slot_index))
            {
				/* active slot socket fd can't be closed here. It will lose the tipc state transition */
				/* and that will make master switch error */
				npd_syslog_dbg("Close sock fd %d to Remote board %d.\r\n", data->sock, board->slot_index+1 );
				circle_unregister_sock(data->sock, EVENT_TYPE_READ);
            	circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);

                close(data->sock);
                data->sock = 0;
            }
            //chasm_subboard_online_delete(board, board);
			board->online_removed = TRUE;		// fix by wuhao 2010- keep refresh
            board->runstate = RMT_BOARD_NOEXIST;
            npd_syslog_dbg("Remote board %d state move to NOEXIST.\r\n", board->slot_index+1);
            if(localmoduleinfo->fix_spec_param->system_conn_deinit)
                (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                        board->fix_param->board_type,
                        board->slot_index);
            if(snros_system_param->product_pp_feature_set)
                (*snros_system_param->product_pp_feature_set)();
        
            chasm_assemble_status_report_pdu(board, board,
                        &status_pdu, &len);
            chasm_send_pdu(src_board, status_pdu, len);
            free(status_pdu);
            if(board_tlv->online_removed == FALSE)
            {
                board->online_removed = FALSE;
                board->fix_param = NULL;
            }
			npd_board_info_notify(board->slot_index);
        }		

		return NPD_SUCCESS;
    }
    else
    {
        char *status_pdu;
        int len;

		npd_syslog_dbg("Local board is MASTERACTIVE\r\n");

        if(NULL == board->fix_param)
        {
            npd_syslog_cslot_err("\n%% Receive error status for board %d.\r\n",
                 board->slot_index);
            return NPD_FAIL;
        }
        if(board != src_board)
        {
			board->rmtstate[src_board->slot_index] = board_tlv->runstate;
			npd_syslog_dbg("board %d rmtstate %d is %d\r\n", board->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);
		}
		
        if(board == src_board)
        {
            if(board_tlv->runstate == RMT_BOARD_RUNNING)
            {
				npd_syslog_dbg("board %d move to RMT_BOARD_RUNNING\r\n", board->slot_index+1);
				
                board->runstate = RMT_BOARD_RUNNING;
                chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
                chasm_broadcast_pdu(status_pdu, len);
                free(status_pdu);
            }
            else
            {
            }
        }
        
        return NPD_SUCCESS;
    }
#endif
    return NPD_SUCCESS;
}

long rmt_board_running_master_cmd(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long rmt_board_running_query(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long rmt_board_running_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_reset(board, src_board, event, pdu);
}

long rmt_board_running_reg_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    //return rmt_board_registering_reg_request(board, src_board, event, pdu);
	chasm_board_reginfo_tlv_t *sub_tlv = chasm_board_reginfo_tlv_ntoh((chasm_board_reginfo_tlv_t *)pdu);

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
 
    if(0 == sub_tlv->subslot_id)
    {
        npd_syslog_err("Error, the remote board %d subboard register on wrong state.\r\n", 
            sub_tlv->slotid, sub_tlv->subslot_id);
        return NPD_FAIL;
    }
#if SUBCARD_DEFINE
    sub_board = board->sub_board[sub_tlv->subslot_id];
    if(NULL == sub_board)
    {
        sub_board = malloc(sizeof(board_param_t));
        if(NULL == sub_board)
        {
            npd_syslog_err("Failed to alloc memory for chasm board % sub-board % management.\r\n",
                      board->slot_index, sub_tlv->subslot_id);
        }
        memset(sub_board, 0, sizeof(board_param_t));
        sub_board->slot_index = sub_tlv->subslot_id;
        sub_board->workmode = SUB_BOARD;
        sub_board->runstate = RMT_BOARD_REGISTERED;
        sub_board->state_function = rmt_board_state_desc;
        sub_board->mother_board = board;
    }

    if(sub_board->online_removed == TRUE)
    {
        if(sub_board->fix_param->board_type != sub_tlv->board_type)
        {
            npd_syslog_cslot_event("\n%% The board %d register request board type is not same as removed one.\r\n",
              board->slot_index);
            npd_syslog_cslot_event("\n%% Deleting all configuration related the preset board...\r\n");
            
            chasm_subboard_online_delete(board, sub_board);
            sub_board->online_removed = FALSE;
            sub_board->fix_param = module_basic_info[sub_tlv->board_type];
            sub_board->fix_spec_param = module_spec_info[sub_tlv->board_type];
        }
    }
    
    //if(FALSE == sub_board->online_removed)
    {
        for(i = ASIC_SWITCH_TYPE; i < ASIC_TYPE_MAX; i++)
        {
            int j;

			if (NULL == board->fix_spec_param->ams_param[i]) //for fix the bug that ams_tlv point out of boundary.
				continue;

            if(ams_tlv->ams_type != i)
                continue;
            for(j = 0; j < sub_board->fix_param->ams_param[i]->num; j++)
            {
                ams_info_t *ams = &sub_board->ams_info[i][j];
                strncpy((char*)ams->name, (char*)ams_tlv->name, 31);
                ams->index = j;
                ams->dev_id = ams_tlv->vender_id;
                ams->rev_id = ams_tlv->revid;
	/*                ams->pci_id = ams_tlv->pci_id;*/
                ams_tlv++;
            }
        }

        for(i = 0; i < sub_board->fix_spec_param->temper_fix_param->num; i++)
        {
            sub_board->temperature[i].name 
                = sub_board->fix_spec_param->temper_fix_param->name[i];
        }
        
/*        memcpy(sub_board->man_param.modname, sub_tlv->subboard_full_name, 32);*/
/*        memcpy(sub_board->man_param.hw_version, sub_tlv->hw_version, 32);*/
        memcpy(sub_board->man_param.sn, sub_tlv->sn, 32);
    } 

	npd_syslog_dbg("board %d register succeed",board->slot_index+1);

    {
        char * new_pdu;
        int len;
        chasm_assemble_reg_res_pdu(board, board, src_board, &new_pdu, &len);
        chasm_send_pdu(board, new_pdu, len);
        free(new_pdu);

    }
#endif
#endif
    return NPD_SUCCESS;


}



long rmt_board_running_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
		
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    return NPD_SUCCESS;
}

long rmt_board_running_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    /*in stanby master, the rmt line board have only two state, READY or NOEXIST,
    */
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        return NPD_SUCCESS;
    }
    return rmt_board_running_swremove(board, src_board, SW_REMOVE, pdu);
}

long rmt_board_running_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
    int ret = 0;

	if((data->sock == 0) && !SYS_CHASSIS_ISMASTERSLOT(board->slot_index))
        ret = chassis_sbymaster_connectto_board(board);
    if(0 != ret)
    {
		npd_syslog_dbg("board %d connect fail. move to NOEXIST, but online_removed is true", board->slot_index+1);	
        ret = chassis_sys_reset_ext(board->slot_index);
		board->runstate = RMT_BOARD_NOEXIST;
        if(localmoduleinfo->fix_spec_param->system_conn_deinit)
            (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                    board->fix_param->board_type,
                    board->slot_index);
        if(snros_system_param->product_pp_feature_set)
            (*snros_system_param->product_pp_feature_set)();
		board->online_removed= TRUE;
        return 0;
    }
    else
    {
		npd_syslog_dbg("board %d move to SWITCHOVERING", board->slot_index+1);	
		board->runstate = RMT_BOARD_SWITCHOVERING;
	}   
#endif	
    return NPD_SUCCESS;
}


state_event_func_t rmt_board_running_funcs =
{
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_running_swinsert,
    &rmt_board_running_hwremove,
    &rmt_board_running_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_running_tipc_connect,
    &rmt_board_running_tipc_break,
    &rmt_board_running_reg_request,
    &board_state_event_error,
    &rmt_board_running_status_report,
    &rmt_board_running_master_cmd,
    &rmt_board_running_switchover_report,
    &rmt_board_running_query,
    &rmt_board_running_reset,
    &board_state_event_error
};

long rmt_board_switchovering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    chassis_sys_reset_ext(board->slot_index);
    return rmt_board_registered_hwremove(board, src_board, event, pdu);
}

long rmt_board_switchovering_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_hwremove(board, src_board, event, pdu);
}

long rmt_board_switchovering_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_swremove(board, src_board, event, pdu);
}

long rmt_board_switchovering_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	
    chasm_board_statusinfo_tlv_t *board_tlv = (chasm_board_statusinfo_tlv_t *)pdu;
	
    if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
        return NPD_SUCCESS;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Remote board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
	npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);

	
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
		return NPD_SUCCESS;
    }
    else
    {
        char *status_pdu;
        int len;

		npd_syslog_dbg("Local board is MASTERACTIVE\r\n");

        if(NULL == board->fix_param)
        {
            npd_syslog_cslot_err("\n%% Receive error status for board %d.\r\n",
                 board->slot_index+1);
            return NPD_FAIL;
        }
        if(board != src_board)
        {
			board->rmtstate[src_board->slot_index] = board_tlv->runstate;
			npd_syslog_dbg("board %d rmtstate %d is %d\r\n", board->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);
		}
		
        if(board == src_board)
        {
            if(board_tlv->board_type == board->fix_param->board_type)
            {
                chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
                
				npd_syslog_dbg("board %d move to RMT_BOARD_RUNNING\r\n", board->slot_index+1);

                circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
                board->runstate = RMT_BOARD_RUNNING;
                chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
                chasm_broadcast_pdu(status_pdu, len);
                free(status_pdu);
            }
            else
            {
        		board->runstate = RMT_BOARD_NOEXIST;
                if(localmoduleinfo->fix_spec_param->system_conn_deinit)
                    (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                            board->fix_param->board_type,
                            board->slot_index);
                if(snros_system_param->product_pp_feature_set)
                    (*snros_system_param->product_pp_feature_set)();
        		board->online_removed= TRUE;
                chassis_sys_reset_ext(board->slot_index);
            }
        }
        
        return NPD_SUCCESS;
    }
#endif
    return NPD_SUCCESS;
}



long rmt_board_switchovering_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return rmt_board_registered_reset(board, src_board, event, pdu);
}

long rmt_board_switchovering_reg_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return NPD_SUCCESS;
}

long rmt_board_switchovering_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
   	struct sockaddr_tipc server_addr = {0};
    int fd;
    int ret;
    chasm_circle_data_t *data;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Remote board %d is inserted.\r\n", board->slot_index+1);     
    if(!SYS_CHASSIS_ISMASTERSLOT(board->slot_index))
    {
        return NPD_SUCCESS;
    }
    data = &chasm_circle_data[board->slot_index];
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);

    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket!\r\n");
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAME;
    server_addr.addr.name.name.type = SERVER_CHASM_MANAGE;
    server_addr.addr.name.name.instance = board->slot_index+1;
    server_addr.addr.name.domain = 0;

    data->sock = fd;
    data->board = board;
    
    /* Make server available: */
    ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_cslot_err("\n%% Failed to connect to chasm server, ret is %d(%s)", errno, strerror(errno));
        close(fd);
		data->sock = 0;
        chassis_sys_reset_ext(board->slot_index);
        board->runstate = RMT_BOARD_NOEXIST;
        board->online_removed = TRUE;
        board->inserted = FALSE;
        if(localmoduleinfo->fix_spec_param->system_conn_deinit)
            (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                    board->fix_param->board_type,
                    board->slot_index);
        if(snros_system_param->product_pp_feature_set)
            (*snros_system_param->product_pp_feature_set)();
        return NPD_FAIL;
    }
	
    circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);

    board->runstate = RMT_BOARD_RUNNING;
    board->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
    board->online_removed = FALSE;
#endif
    return NPD_SUCCESS;
}

long rmt_board_switchovering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    /*in stanby master, the rmt line board have only two state, READY or NOEXIST,
    */
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        return NPD_SUCCESS;
    }
    return rmt_board_running_swremove(board, src_board, SW_REMOVE, pdu);
}


state_event_func_t rmt_board_switchovering_funcs =
{
    &rmt_board_switchovering_timer,
    &board_state_event_error,
    &rmt_board_switchovering_swinsert,
    &rmt_board_switchovering_hwremove,
    &rmt_board_switchovering_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_switchovering_tipc_break,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_switchovering_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &rmt_board_switchovering_reset
};

state_desc_t rmt_board_state_desc[] = 
{
    {
        RMT_BOARD_NOEXIST, 
        "remote board empty", 
        &rmt_board_noexist_funcs
    },
    {
        RMT_BOARD_HWINSERTED, 
        "remote board hardware inserted", 
        &rmt_board_hwinserted_funcs
    },
    {
        RMT_BOARD_SWINSERTED,
        "remote board software inserted",
        &rmt_board_swinserted_funcs
    },
    {
        RMT_BOARD_REGISTERING,
        "remote board registering",
        &rmt_board_registering_funcs
    },
    {
        RMT_BOARD_SW_VERERR, 
        "remote board software version error", 
        &rmt_board_sw_vererr_funcs
    },    
    {
        RMT_BOARD_REGISTERED, 
        "remote board registered", 
        &rmt_board_registered_funcs
    },
    {
        RMT_BOARD_READY, 
        "remote board ready", 
        &rmt_board_ready_funcs
    },
    {
        RMT_BOARD_RUNNING, 
        "remote board running",
        &rmt_board_running_funcs
    },
    {
        RMT_BOARD_SWITCHOVERING, 
        "remote board switchovering",
        &rmt_board_switchovering_funcs
    },

    {
        RMT_BOARD_REMOVING, 
        "remote board removing",
        &rmt_board_removing_funcs
    },
    {
        RMT_BOARD_ERROR, 
        "remote board error",
        &rmt_board_error_funcs
    },
};

int local_wait_for_mcu_exist_server(struct tipc_name_seq* name,int wait)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    struct sockaddr_tipc topsrv = {0};
    struct tipc_subscr *subscr = malloc(sizeof(struct tipc_subscr));
    struct tipc_event event;
    int ret;
    int state;
    int evt;
    int mcu_exist = FALSE;

    memset(subscr, 0, sizeof(struct tipc_subscr));
    subscr->seq.type = name->type;
    subscr->seq.lower = name->lower;
    subscr->seq.upper = name->upper;
    subscr->timeout = wait*1000;
    subscr->filter = TIPC_SUB_SERVICE;
        
    int sd = socket (AF_TIPC, SOCK_SEQPACKET,0);

    memset(&topsrv,0,sizeof(topsrv));
    topsrv.family = AF_TIPC;
    topsrv.addrtype = TIPC_ADDR_NAME;
    topsrv.addr.name.name.type = TIPC_TOP_SRV;
    topsrv.addr.name.name.instance = TIPC_TOP_SRV;

    /* Connect to topology server: */
    if (0 > (ret = connect(sd,(struct sockaddr*)&topsrv,sizeof(topsrv)))){
        if(errno != EINPROGRESS)
        {
            npd_syslog_err("failed to connect to topology server");
            free(subscr);
            return -1;
        }
        return NPD_SUCCESS;
    }
    
   
    if (send(sd,subscr,sizeof(*subscr),0) != sizeof(*subscr)){
            npd_syslog_err("failed to send subscription");
            free(subscr);
            return -1;
    }
    free(subscr);
    
    if (recv(sd,&event,sizeof(event),0) != sizeof(event)){
            npd_syslog_dbg("Chasm tipc recv from topology service error\r\n");
            mcu_exist = FALSE;
    }
    else if (event.event == TIPC_PUBLISHED)
    {
        npd_syslog_dbg("Chasm mcu exist server published.\r\n");
        mcu_exist = TRUE;

    }
    else
    {
        mcu_exist = FALSE;
    }

    if(mcu_exist == TRUE)
    {
        chasm_circle_data_t *data;
        int fd;
        char buf[4] ={0};
    
        data = &chasm_circle_data[localmoduleinfo->slot_index];
        fd = data->sock;
    	sprintf(buf, "%d", SYS_LOCAL_MODULE_ISMASTERACTIVE); 
    	write_to_file(CHASSIS_MAN_ACTMASTER_STATE_FILE, buf, strlen(buf));
        
        localmoduleinfo->runstate = LOCAL_SLAVE_WAIT_CONNECT;
        npd_syslog_dbg("Local board state move to WAIT_CONNECT.\r\n");
    
    	npd_syslog_dbg("register fd %d to chasm_accept_tipc. \r\n", fd);
        circle_register_sock(fd, EVENT_TYPE_READ, chasm_accept_tipc, data, NULL);
    
    	npd_syslog_dbg("register_timeout CHASM_REG_TIMEOUT to  board %d\n",data->board->slot_index+1);
    	circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout, data, NULL);
        close(sd);
   
    	return NPD_SUCCESS;
    }
    else
    {
        npd_syslog_cslot_event("\n%% Local board expire the WAIT_CONNECT timer.\r\n");
        close(sd);
		
		/* don't do anything , just reset for  this need modify for better*/
		snros_local_board_spec->reset();

   /*由于6602 产测需要放在6603的三槽上面检测，此时没有主控板
若要取得正确的本板信息，需把下面的代码放开，板管理状态
才能正确
*/
#if 1
		npd_syslog_dbg("slave indpnet get is %d.\r\n", 1);	
    	localmoduleinfo->state_function = local_master_state_desc;
    
        evt = ACTIVE_MASTER_ENABLE;
        localmoduleinfo->runstate = LOCAL_MASTER_INIT;
        state = LOCAL_MASTER_INIT;
        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, localmoduleinfo, evt, NULL);
#endif    
    	return NPD_SUCCESS;
    }
#endif        
    return NPD_SUCCESS;
}


long local_slave_init_sw_insert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
   	struct sockaddr_tipc server_addr = {0};
    int fd;
    int ret;
    chasm_circle_data_t *data;

    struct tipc_name_seq name;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    data = &chasm_circle_data[localmoduleinfo->slot_index];
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_cslot_err("\n%% Can not create chassis management tipc socket for board %d!\r\n"
            , localmoduleinfo->slot_index+1);
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_CHASM_MANAGE;
	server_addr.addr.nameseq.lower = localmoduleinfo->slot_index+1;
	server_addr.addr.nameseq.upper = localmoduleinfo->slot_index+1;
	server_addr.scope = TIPC_ZONE_SCOPE;

    ret = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_cslot_err("\n%% Can not bind tipc socket addr for board %d\n",
            board->slot_index+1);
        perror("TIPC socket bind error.");
        exit(1);
    }

    ret = listen(fd, 1);
    if(ret < 0)
    {
        npd_syslog_cslot_err("\n%% Can not listen tipc socket addr for board %d\r\n",
            board->slot_index+1);
        perror("TIPC socket listen error.");
        exit(1);
    }

    data->sock = fd;
    data->board = localmoduleinfo;
    
    name.type = SERVER_MCU_EXIST;
    name.lower = SYS_CHASSIS_MASTER_SLOT_INDEX(0)+1;
    name.upper = SYS_CHASSIS_MASTER_SLOT_INDEX(SYS_CHASSIS_MASTER_SLOTNUM-1)+1;

	npd_syslog_event("\nLocal slave board %d wait active master board information...\r\n", board->slot_index+1);
	
    local_wait_for_mcu_exist_server(&name, CHASM_SBY_CONNECT_TIMEOUT);
#endif
	return NPD_SUCCESS;
}
state_event_func_t local_slave_init_funcs =
{
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_init_sw_insert,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_slave_wait_connect_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Local board expire the WAIT_CONNECT timer.\r\n");
    npd_syslog_dbg("slave indpnet get is %d.\r\n", 1);	
	localmoduleinfo->state_function = local_master_state_desc;

    event = ACTIVE_MASTER_ENABLE;
    localmoduleinfo->runstate = LOCAL_MASTER_INIT;
    state = LOCAL_MASTER_INIT;
    (*(*(localmoduleinfo->state_function)[state].funcs)[event])
        (localmoduleinfo, localmoduleinfo, event, NULL);
#endif
	return NPD_SUCCESS;
}

long local_slave_wait_connect_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;

    if(-1 == SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
    }

    {
        chasm_circle_data_t *local_data = &chasm_circle_data[localmoduleinfo->slot_index];
        npd_syslog_dbg("Local board connect to active master board %d.\r\n", src_board->slot_index +1);

        {
            char buf[4];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%d", 0);
            write_to_file(CHASSIS_MAN_SLAVE_RUNNING_INDPNT_FILE, buf, strlen(buf));
        }

        
        circle_cancel_timeout(chasm_timeout, local_data, circle_ALL_CTX);
		
		npd_syslog_dbg("Register CHASM_REG_TIMEOUT  board %d.\r\n", data->board->slot_index+1);
		circle_register_timeout(CHASM_REG_TIMEOUT, 0, chasm_timeout, data, NULL);

        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
        chasm_send_pdu(src_board, reg_pdu, len);
        free(reg_pdu);

		npd_syslog_dbg("register fd %d to chasm_pdu_recv. \r\n", fd);
		circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);

		npd_syslog_dbg("board %d move to RMT_BOARD_SWINSERTED\r\n", src_board->slot_index+1);
		src_board->workmode = MASTER_BOARD;
		src_board->redundancystate = MASTER_ACTIVE;
			
		npd_syslog_dbg("local board %d move to LOCAL_SLAVE_REGISTERING\r\n", localmoduleinfo->slot_index+1);
		localmoduleinfo->runstate = LOCAL_SLAVE_REGISTERING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_REGISTERING;

        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;       
    }
#endif	
    return NPD_SUCCESS;
}


long local_slave_noncompat_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
#ifdef HAVE_AC_BOARD	
	long product_type = PRODUCT_AX8603;
	/* AX8603 */

	chasm_circle_data_t * data = &chasm_circle_data[board->slot_index];

	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Local board receive the WAIT_CONNECT timer.\r\n");
	/* get the product code.  */
	if(SYS_PRODUCT_TYPE != product_type)
    {
		/* unregister timeout  */
		circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    	circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    	close(data->sock);
		
        npd_syslog_dbg("Local board product type is not same as MCU, reset product type to %s\r\n",
            device_product_type2name(product_type));
        device_product_reset(product_type);
        chassis_info_reinit();
		ENVIROMENT_MONITOR_ENABLE = 0;/* stop monitor */
		chasm_set_product_reset_state(HAS_RESET);
		npd_syslog_dbg("Local Module now is %s.\n", module_id_str(localmoduleinfo->fix_param->board_type));
		
	}
#endif
#endif
	return NPD_SUCCESS;
}

state_event_func_t local_slave_wait_connect_funcs =
{
    &local_slave_wait_connect_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_wait_connect_tipc_connect,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_noncompat_request,
};

long local_slave_registering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_err("\n%% Local board expire the REGISTERING timer\r\n");

    data = &chasm_circle_data[board->slot_index];
    fd = data->sock;
    circle_unregister_sock(fd, EVENT_TYPE_READ);
    close(fd);
    
    localmoduleinfo->runstate = LOCAL_SLAVE_WAIT_CONNECT;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_NOEXIST;
    board->redundancystate = MASTER_STANDBY;
    board->workmode = SLAVE_BOARD;
    board->runstate = RMT_BOARD_NOEXIST;
#endif
    return NPD_SUCCESS;
}

long local_slave_registering_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return board_state_event_error(board, src_board, event, pdu);
#if 0    
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;

    assert(src_board != NULL);

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;

    /*if there is not ACT MASTER*/
    if(-1 == SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        assert(0);
#if 0        
        chasm_circle_data_t *local_data = &chasm_circle_data[localmoduleinfo->slot_index];
        npd_syslog_dbg("Local board connect to active master board %d.\r\n", src_board->slot_index);

        circle_cancel_timeout(chasm_timeout, local_data, NULL);
        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
        chasm_send_pdu(src_board, reg_pdu, len);
        free(reg_pdu);
		
		circle_register_timeout(CHASM_REG_TIMEOUT, 0, &chasm_timeout, data, NULL);
		npd_syslog_dbg("register_timeout CHASM_REG_TIMEOUT to  board %d",
			data->board->slot_index+1);

        circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
		src_board->workmode = MASTER_BOARD;
        src_board->redundancystate = MASTER_ACTIVE;
        src_board->runstate = RMT_BOARD_SWINSERTED;

		
        localmoduleinfo->runstate = LOCAL_SLAVE_REGISTERING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_REGISTERING;
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;  
#endif        
    }
    /*the SBY MASTER*/
    else
    {
        /*sby connect only happen in running state*/
        npd_syslog_dbg("Local board connect to standby master.\r\n");
        circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
        src_board->redundancystate = MASTER_STANDBY;
        src_board->workmode = MASTER_BOARD;
        //src_board->runstate = RMT_BOARD_RUNNING;
    }
    return NPD_SUCCESS;
#endif    
}


long local_slave_registering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;


    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;
    circle_unregister_sock(fd, EVENT_TYPE_READ);
    close(fd);
    data->sock = 0;

    src_board->redundancystate = MASTER_STANDBY;
    src_board->runstate = RMT_BOARD_NOEXIST;
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;
 
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    localmoduleinfo->runstate = LOCAL_SLAVE_WAIT_CONNECT;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_NOEXIST;
    npd_syslog_dbg("Local board state move to WAIT_CONNECT.\r\n");

    data = &chasm_circle_data[localmoduleinfo->slot_index];
    fd = data->sock;

	npd_syslog_dbg("register_timeout CHASM_REG_TIMEOUT to  board %d",data->board->slot_index+1);
	circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout, data, NULL);
#endif	
    return NPD_SUCCESS;
    
}

long local_slave_registering_reg_response(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_product_info_tlv_t *regres_tlv;
    char *status_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    regres_tlv = chasm_product_info_tlv_ntoh((chasm_product_info_tlv_t *)pdu);

    npd_syslog_dbg("Local board received register response.\r\n");
	circle_cancel_timeout(chasm_timeout, &chasm_circle_data[src_board->slot_index], circle_ALL_CTX);

    if(SYS_PRODUCT_TYPE != regres_tlv->product_type)
    {
        npd_syslog_dbg("Local board product type is not same as MCU, reset product type to %s\n",
            device_product_type2name(regres_tlv->product_type));
        device_product_reset(regres_tlv->product_type);
        chassis_info_reinit();
        /*Need FIX , after chassis reset, board is point to wrong address*/
    }
    memcpy(SYS_PRODUCT_BASEMAC, regres_tlv->base_mac, 6);
    npd_syslog_dbg("\r\nRemote MAC address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
		regres_tlv->base_mac[0], regres_tlv->base_mac[1], regres_tlv->base_mac[2],
		regres_tlv->base_mac[3], regres_tlv->base_mac[4], regres_tlv->base_mac[5]);
    npd_syslog_dbg("\r\nRemote software version: %s\r\n", regres_tlv->sw_version);
    if(chasm_version_err((char*)regres_tlv->sw_version,regres_tlv->build_time))
    {
        npd_syslog_dbg("Local board software version error. state move to UPGRADING.\r\n");
        board->runstate = LOCAL_SLAVE_SW_UPGRADING;
        board->rmtstate[board->slot_index] = RMT_BOARD_SW_VERERR;
        /*board->fix_param->os_upgrade(board->slot_index);*/
        //return NPD_SUCCESS;
    }
    else
    {
        board->runstate = LOCAL_SLAVE_READY;
        board->rmtstate[board->slot_index] = RMT_BOARD_READY;
        npd_syslog_dbg("Local board state move to READY.\r\n");
    }
    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
    chasm_send_pdu(src_board, status_pdu, len);
    free(status_pdu);
#endif	
	return NPD_SUCCESS;
}

long local_slave_registering_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return board_state_event_error(board, src_board, event, pdu);
#if 0    
    chasm_circle_data_t *actmas_data
                      = &chasm_circle_data[SYS_MASTER_ACTIVE_SLOT_INDEX];
    
    char *reg_pdu;
    int len;

    /*reset self to avoid complicating process, this situcaiton should not happen*/
    snros_local_board->reset();
#if 0
    assert(src_board != NULL);
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("Local board received switchover report from board %d.\r\n", src_board->slot_index+1);

    circle_cancel_timeout(chasm_timeout, actmas_data, NULL);
    chasm_assemble_reg_req_pdu(board, board, &reg_pdu, &len);
    chasm_send_pdu(src_board, reg_pdu, len);
    free(reg_pdu);

	npd_syslog_dbg("Register CHASM_REG_TIMEOUT to board %d.\r\n", SYS_LOCAL_MODULE_SLOT_INDEX +1);
    circle_register_timeout(CHASM_REG_TIMEOUT, 0, chasm_timeout, 
       &chasm_circle_data[SYS_LOCAL_MODULE_SLOT_INDEX], NULL);
	
	//npd_syslog_dbg("Register EVENT_TYPE_READ to board %d.\r\n", data->board->slot_index+1);
    //circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
	
	npd_syslog_dbg("board %d is MASTER_ACTIVE, move to RMT_BOARD_READY.\r\n", src_board->slot_index+1);
    src_board->redundancystate = MASTER_ACTIVE;
    src_board->runstate = RMT_BOARD_READY;
    if(SYS_MASTER_ACTIVE_SLOT_INDEX != src_board->slot_index)
    {
		npd_syslog_dbg("update MASTER_ACTIVE_SLOT_INDEX to slot %d.\r\n", src_board->slot_index+1);
        chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX]->redundancystate = MASTER_STANDBY;
		chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX]->runstate = RMT_BOARD_NOEXIST;
		//chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX]->fix_param = NULL;
		chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX]->online_removed = TRUE;
			
		SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
    }

	npd_syslog_dbg("Local board move to REGISTERING.\r\n");
    board->runstate = LOCAL_SLAVE_REGISTERING;
    board->rmtstate[board->slot_index] = RMT_BOARD_REGISTERING;
    
#endif    
	return NPD_SUCCESS;
#endif
}

state_event_func_t local_slave_registering_funcs =
{
    &local_slave_registering_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_registering_tipc_break,
    &board_state_event_error,
    &local_slave_registering_reg_response,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_slave_sw_upgrading_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_slave_sw_upgrading_reg_response(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_slave_sw_upgrading_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_slave_sw_upgrading_swichover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    (*chassis_sys_reset)();
    
	return NPD_SUCCESS;
}
long local_slave_sw_upgrading_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t local_slave_sw_upgrading_funcs =
{
    &local_slave_sw_upgrading_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_registering_tipc_connect,
    &local_slave_registering_tipc_break,
    &board_state_event_error,
    &local_slave_sw_upgrading_reg_response,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_sw_upgrading_swichover_report,
    &board_state_event_error,
    &local_slave_sw_upgrading_reset,
    &board_state_event_error
};

long local_slave_ready_timeout(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    snros_local_board_spec->reset();
#endif
	return NPD_SUCCESS;
}

long local_slave_ready_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return board_state_event_error(board, src_board, event, pdu);
#if 0    
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;

    if(-1 == SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        /*should not happen in design*/
        assert(0);
#if 0        
        chasm_circle_data_t *local_data = &chasm_circle_data[board->slot_index];

        npd_syslog_dbg("Local board connect to active master\n");
        circle_cancel_timeout(chasm_timeout, local_data, NULL);
        chasm_assemble_reg_req_pdu(board, board, &reg_pdu, &len);
        chasm_send_pdu(src_board, reg_pdu, len);
        free(reg_pdu);

		npd_syslog_dbg("Register CHASM_REG_TIMEOUT to board %d\n", data->board->slot_index +1);
        circle_register_timeout(CHASM_REG_TIMEOUT, 0, &chasm_timeout, data, NULL);
        circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
		src_board->workmode = MASTER_BOARD;
		src_board->redundancystate = MASTER_ACTIVE;
        src_board->runstate = RMT_BOARD_SWINSERTED;
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index; 
#endif        
    }
    /*the SBY MASTER*/
    else
    {
        npd_syslog_dbg("Local board connect to stanby master board %d\n", src_board->slot_index+1);
        circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
        src_board->redundancystate = MASTER_STANDBY;
        src_board->workmode = MASTER_BOARD;
    }
    return NPD_SUCCESS;
#endif    
}


long local_slave_ready_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;

    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;
    circle_unregister_sock(fd, EVENT_TYPE_READ);
    close(fd);
	data->sock = 0;

    src_board->redundancystate = MASTER_STANDBY;
    src_board->rmtstate = RMT_BOARD_NOEXIST;
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;
    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                           src_board->fix_param->board_type, src_board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    localmoduleinfo->runstate = LOCAL_SLAVE_WAIT_CONNECT;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_NOEXIST;
    npd_syslog_dbg("Local board state move to WAIT_CONNECT.\r\n");

    data = &chasm_circle_data[localmoduleinfo->slot_index];
    fd = data->sock;

	npd_syslog_dbg("register_timeout CHASM_REG_TIMEOUT to  board %d",data->board->slot_index+1);
	circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout, data, NULL);
#endif
    return NPD_SUCCESS;
}

long local_slave_ready_reg_res(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char * status_pdu;
    int len;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	circle_cancel_timeout(chasm_timeout, &chasm_circle_data[board->slot_index], circle_ALL_CTX);
    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
    chasm_send_pdu(src_board, status_pdu, len);
    free(status_pdu);
#endif
    return NPD_SUCCESS;
}


long local_slave_ready_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu);
    int evt = STATUS_REPORT;
    board_param_t *dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;
	

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	npd_syslog_dbg("dest_board is %d, and currunt runstate is %d\r\n", board_tlv->slotid+1, dest_board->runstate);
    if(localmoduleinfo == dest_board)
    {
        if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
            return NPD_SUCCESS;

        localmoduleinfo->rmtstate[src_board->slot_index] 
                  = board_tlv->runstate;
		npd_syslog_dbg("localmoduleinfo == dest_board, and rmtstate from %d is %d\r\n",
			src_board->slot_index+1, board_tlv->runstate);
        {
            if(board_tlv->runstate == RMT_BOARD_READY)
            {
				char * status_pdu;
				int len;
                int i;

            	chasm_write_board_info();
                chasm_init_done();
                chasm_unlock();

                chasm_wait_startup_config_done();
                chasm_lock();
                for(i = 0; i < ASIC_TYPE_MAX; i++)
                {
                    int j;
					if (NULL == localmoduleinfo->fix_spec_param->ams_param[i])
						continue;
					
                    for(j = 0; j < localmoduleinfo->fix_spec_param->ams_param[i]->num; j++)
                    {
						if (NULL != (localmoduleinfo->fix_spec_param->ams_param[i]->ams_enable))
						{
                        	(*localmoduleinfo->fix_spec_param->ams_param[i]->ams_enable)(j);
						}
                    }
                }
                
                localmoduleinfo->runstate = LOCAL_SLAVE_RUNNING;
                localmoduleinfo->rmtstate[localmoduleinfo->slot_index] 
                    = RMT_BOARD_RUNNING;

				chasm_assemble_status_report_pdu(localmoduleinfo, localmoduleinfo, &status_pdu, &len);
                chasm_send_pdu(src_board, status_pdu, len);
                free(status_pdu);

                npd_syslog_cslot_event("\n%% Local board is normally running.\r\n");
				npd_chassis_board_info_notify(0);
            }			
        }
		return NPD_SUCCESS;
    }
    else
        return (*(*(dest_board->state_function)[state].funcs)[evt])
                (dest_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_slave_ready_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return board_state_event_error(board, src_board, event, pdu);
#if 0    
    board_param_t *act_board;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(-1 != SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        act_board = chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX];
    
        local_slave_act_break(board, act_board, event, pdu);
        act_board->redundancystate = MASTER_STANDBY;
        act_board->runstate = RMT_BOARD_READY;
    	act_board->online_removed = FALSE;
    }
    
	return NPD_SUCCESS;
#endif    
}

long local_slave_ready_master_cmd(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_slave_ready_query(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    /*assemble all board status report pdu and send to master*/
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_slave_ready_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    /*rebooting*/
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t local_slave_ready_funcs =
{
    &local_slave_ready_timeout,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_ready_tipc_break,
    &board_state_event_error,
    &local_slave_ready_reg_res,
    &local_slave_ready_status_report,
    &local_slave_ready_master_cmd,
    &board_state_event_error,
    &local_slave_ready_query,
    &local_slave_ready_reset,
    &board_state_event_error
};

long local_slave_running_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return local_slave_ready_timeout(board, src_board, event, pdu);
}

long local_slave_running_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;

    {
		npd_syslog_dbg("register fd %d to chasm_pdu_recv. \r\n", fd);
        circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
        src_board->redundancystate = MASTER_STANDBY;
        src_board->workmode = MASTER_BOARD;

		/* send register info to standy for sync the name */
		if(SYS_MODULE_ISMASTERSTANDBY(src_board->slot_index))
		{
			char * reg_pdu;
			int len;
			
	        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
	        chasm_send_pdu(src_board, reg_pdu, len);
	        free(reg_pdu);			
		}
        //src_board->runstate = RMT_BOARD_SWINSERTED;
    }
#endif
    return NPD_SUCCESS;
    
}

long local_slave_act_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];

    fd = data->sock;
    circle_unregister_sock(fd, EVENT_TYPE_READ);
    close(fd);
	npd_syslog_dbg("close: Current fd is %d.\r\n", data->sock);
	data->sock = 0;

	if (SYS_MASTER_ACTIVE_SLOT_INDEX != src_board->slot_index)
	{
		npd_syslog_cslot_err("ActMaster has from %d to %d.\r\n", 
			src_board->slot_index+1, SYS_MASTER_ACTIVE_SLOT_INDEX+1);	
		npd_board_info_notify(src_board->slot_index);
		return NPD_SUCCESS;
	}

	if (chasm_board_issbymaster_connected(localmoduleinfo))
	{
		npd_syslog_dbg("localmodule move to SWITCHOVERING\r\n");
        chasm_assemble_status_report_all(&reg_pdu, &len);
        chasm_send_pdu(chasm_get_sbymaster(), reg_pdu, len);
        free(reg_pdu);
        SYS_MASTER_ACTIVE_SLOT_INDEX = chasm_get_sbymaster()->slot_index;
        localmoduleinfo->runstate = LOCAL_SLAVE_RUNNING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
        chasm_write_board_info();
	}
	else
	{
		npd_syslog_dbg("localmodule move to WAIT_CONNECT\r\n");
    	circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout,
        	&chasm_circle_data[localmoduleinfo->slot_index], NULL);
		SYS_MASTER_ACTIVE_SLOT_INDEX = -1;
        localmoduleinfo->runstate = LOCAL_SLAVE_SWITCHOVERING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = LOCAL_SLAVE_SWITCHOVERING;

		/* don't do anything , just reset for */
		snros_local_board_spec->reset();
	}

	npd_syslog_dbg("localmodule move to state %d \r\n",localmoduleinfo->runstate);
#endif	
    return NPD_SUCCESS;
}

long local_slave_running_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    /*disable asic*/
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    src_board->redundancystate = MASTER_STANDBY;
    src_board->runstate = RMT_BOARD_NOEXIST;
    src_board->online_removed = TRUE;
    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                           src_board->fix_param->board_type, src_board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
    chasm_subboard_online_remove(src_board, src_board);
#endif
    return local_slave_act_break(board, src_board, event, pdu);
}


long local_slave_running_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu);
    board_param_t *dest_board;
    int state ;
    int evt = STATUS_REPORT;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
    if(board->slot_index == board_tlv->slotid)
    {
        return NPD_SUCCESS;
    }

    dest_board = chassis_slots[board_tlv->slotid];
	state = dest_board->runstate;

	npd_syslog_dbg("dest_board is %d, and currunt runstate is %d\r\n", board_tlv->slotid+1, dest_board->runstate);

    return (*(*(dest_board->state_function)[state].funcs)[evt])
            (dest_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_slave_running_master_cmd(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_slave_running_swichover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    char *reg_pdu;
	char *switchover_pdu;
    int len;
	board_param_t *prev_master = NULL;

	if((SYS_MASTER_ACTIVE_SLOT_INDEX != -1) && (src_board->slot_index != SYS_MASTER_ACTIVE_SLOT_INDEX))
	{
		prev_master = chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX];

        chasm_assemble_switchover_report_pdu(localmoduleinfo, &switchover_pdu, &len);
    	chasm_send_pdu(prev_master, switchover_pdu, len);
		free(switchover_pdu);
        chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX]->redundancystate = MASTER_STANDBY;
	}

    src_board->redundancystate = MASTER_ACTIVE;
    SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
    src_board->runstate = RMT_BOARD_RUNNING;
    src_board->rmtstate[src_board->slot_index] = RMT_BOARD_RUNNING;
    src_board->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
    localmoduleinfo->runstate = LOCAL_SLAVE_RUNNING;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
	if(localmoduleinfo->fix_spec_param->master_switch)
        (*localmoduleinfo->fix_spec_param->master_switch)(SYS_PRODUCT_TYPE, 
                 src_board->fix_param->board_type, src_board->slot_index);
	npd_chassis_board_info_notify(0);
    chasm_assemble_status_report_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
    chasm_send_pdu(src_board, reg_pdu, len);
    free(reg_pdu);
    chasm_write_board_info();
#endif    
    return NPD_SUCCESS;
}

long local_slave_running_query(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return NPD_SUCCESS;
}

long local_slave_running_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return NPD_SUCCESS;
}

state_event_func_t local_slave_running_funcs =
{
    &local_slave_running_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,    
    &board_state_event_error,
    &local_slave_running_tipc_connect,
    &local_slave_running_tipc_break,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_running_status_report,
    &local_slave_running_master_cmd,
    &local_slave_running_swichover_report,
    &local_slave_running_query,
    &local_slave_running_reset,
    &board_state_event_error
};

long local_slave_switchovering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    snros_local_board_spec->reset();
    return NPD_SUCCESS;
}

long local_slave_switchovering_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;
	int need_register = 0;
	

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;
    
    if(-1 == SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
		need_register = 1;
    }

    {
        chasm_circle_data_t *local_data = &chasm_circle_data[localmoduleinfo->slot_index];
        npd_syslog_dbg("Local board connect to active master board %d.\r\n", src_board->slot_index +1);

        circle_cancel_timeout(chasm_timeout, local_data, circle_ALL_CTX);
		
		npd_syslog_dbg("register fd %d to chasm_pdu_recv. \r\n", fd);
		circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);

		npd_syslog_dbg("board %d move to RMT_BOARD_SWINSERTED\r\n", src_board->slot_index+1);
		src_board->workmode = MASTER_BOARD;
		src_board->redundancystate = MASTER_ACTIVE;
        src_board->runstate = RMT_BOARD_RUNNING;
        src_board->rmtstate[src_board->slot_index] = RMT_BOARD_RUNNING;
        src_board->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;

		if (need_register)
		{
			npd_syslog_dbg("local board %d move to LOCAL_SLAVE_REGISTERING\r\n", localmoduleinfo->slot_index+1);
			localmoduleinfo->runstate = LOCAL_SLAVE_REGISTERING;
	        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_REGISTERING;

			npd_syslog_dbg("Register CHASM_REG_TIMEOUT  board %d.\r\n", data->board->slot_index+1);
			circle_register_timeout(CHASM_REG_TIMEOUT, 0, chasm_timeout, data, NULL);

	        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
	        chasm_send_pdu(src_board, reg_pdu, len);
	        free(reg_pdu);			
		}
		else
		{
			localmoduleinfo->runstate = LOCAL_SLAVE_RUNNING;
	        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;

			npd_chassis_board_info_notify(0);
	        chasm_assemble_status_report_all(&reg_pdu, &len);
	        chasm_send_pdu(src_board, reg_pdu, len);
	        chasm_write_board_info();
		}
    }
#endif	
	return NPD_SUCCESS;
}

long local_slave_switchovering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    
    snros_local_board_spec->reset();
	return NPD_SUCCESS;
}


state_event_func_t local_slave_switchovering_funcs =
{
    &local_slave_switchovering_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,    
    &board_state_event_error,
    &local_slave_switchovering_tipc_connect,
    &local_slave_switchovering_tipc_break,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};



long local_slave_error_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t local_slave_error_funcs =
{
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_slave_error_reset,
    &board_state_event_error
};


state_desc_t local_slave_state_desc[] = 
{
    {
        LOCAL_SLAVE_INIT, 
        "local board init",
        &local_slave_init_funcs
    },
    {
        LOCAL_SLAVE_WAIT_CONNECT, 
        "local board wait for connection",
        &local_slave_wait_connect_funcs
    },
    {
        LOCAL_SLAVE_REGISTERING, 
        "local board registering",
        &local_slave_registering_funcs
    },
    {
        LOCAL_SLAVE_SW_UPGRADING, 
        "local board software upgrading",
        &local_slave_sw_upgrading_funcs
    },
    {
        LOCAL_SLAVE_READY, 
        "local board ready",
        &local_slave_ready_funcs
    },
    {
        LOCAL_SLAVE_RUNNING, 
        "local board running",
        &local_slave_running_funcs
    },
    {
        LOCAL_SLAVE_SWITCHOVERING, 
        "local board switchovering",
        &local_slave_switchovering_funcs
    },

    {
        LOCAL_SLAVE_ERROR, 
        "local board error",
        &local_slave_error_funcs
    }
};

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
static int chasm_man_mon_sock = 0;
void  wait_server_handle(int sd, void *circle_data, void *user_data)
{
    struct tipc_event event[16];
    int i = 0;
	int len = 0;
	int max_board_count = 16;
	int recv_counter = 0;
	static int error_count = 0;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	npd_syslog_dbg("Receive tipc topo service event\r\n");
	len = recv(sd,&event,sizeof(struct tipc_event) * max_board_count,0);
	if (len < 0)
	{
		error_count++;
		if (error_count > 10)
		{
			circle_unregister_sock(sd, EVENT_TYPE_READ);
			close(sd);
			sd = -1;			
		}
		npd_syslog_dbg("receive the len is %d.\r\n", len);
		return ;				
	}
	if(len < sizeof(struct tipc_event))
	{
		npd_syslog_dbg("Chasm tipc recv from topology service error\r\n");
		return;
	}
	recv_counter = len/sizeof(struct tipc_event);
	
	npd_syslog_dbg("Receive %d service SERVER_CHASM_MANAGE event(s)\r\n", recv_counter);
	for(i = 0; i < recv_counter; i++)
	{
        if (event[i].event == TIPC_PUBLISHED){
    		if(event[i].s.seq.type != SERVER_CHASM_MANAGE)
    		{
                npd_syslog_err("TIPC topo service send error message\r\n");
    			continue;
    		}
    		{
    		    int slot_index = tipc_node(event[i].port.node) -1;
                board_param_t *board;  
    
                if(!SYS_SLOTINDEX_IS_REMOTE(slot_index))
    			{
    				npd_syslog_dbg("Receive local board wrong tipc event.\r\n");
    				return;
    			}
                board = chassis_slots[slot_index];
    		    int evt = SW_INSERT;
                npd_syslog_official_event("\nRemote board %d software connection is built.\r\n", board->slot_index+1);
    
    		    chasm_lock();
    			{
    				int state = localmoduleinfo->runstate;
    				npd_syslog_dbg("Current state is %d.\r\n", state);
    				
    			    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
    			        (localmoduleinfo, board, evt, NULL);
    
    			}
    			        
    		    chasm_unlock();
    		}
            
        }
        if(event[i].event == TIPC_WITHDRAWN){
            if(event[i].s.seq.type != SERVER_CHASM_MANAGE)
            {
                npd_syslog_err("TIPC topo service send error message\r\n");
    			continue;
            }
    		{
    		    int slot_index = tipc_node(event[i].port.node) -1;
    		    board_param_t *board;
                
                if(!SYS_SLOTINDEX_IS_REMOTE(slot_index))
                    return;
    
                board = chassis_slots[slot_index];
    		    int evt = SW_REMOVE;
    			if(slot_index == SYS_LOCAL_MODULE_SLOT_INDEX)
    			{
    				npd_syslog_dbg("Receive local board wrong tipc event.\r\n");
    				return;
    			}
    			npd_syslog_official_event("Remote board %d software connection is broken.\r\n", slot_index+1);
    
    		    chasm_lock();
    		    int state = localmoduleinfo->runstate;
    		    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
    		        (localmoduleinfo, board, evt, NULL);
    		    chasm_unlock();
				sleep(1);
    		}
        }
	}
    return;        
}

int wait_for_server(struct tipc_name_seq* name, int wait)
{
    struct sockaddr_tipc topsrv = {0};
    struct tipc_subscr *subscr = malloc(sizeof(struct tipc_subscr));
    int ret;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    memset(subscr, 0, sizeof(struct tipc_subscr));
    subscr->seq.type = name->type;
    subscr->seq.lower = name->lower;
    subscr->seq.upper = name->upper;
	if(-1 != wait)
        subscr->timeout = wait * 1000;
	else
		subscr->timeout = wait;
    subscr->filter = TIPC_SUB_SERVICE;
        
    int sd = socket (AF_TIPC, SOCK_SEQPACKET,0);

    memset(&topsrv,0,sizeof(topsrv));
    topsrv.family = AF_TIPC;
    topsrv.addrtype = TIPC_ADDR_NAME;
    topsrv.addr.name.name.type = TIPC_TOP_SRV;
    topsrv.addr.name.name.instance = TIPC_TOP_SRV;


    /* Connect to topology server: */
    if (0 > (ret = connect(sd,(struct sockaddr*)&topsrv,sizeof(topsrv)))){
        npd_syslog_err("failed to connect to topology server");
        free(subscr);
        return NPD_FAIL;
    }
    
   
    if (send(sd,subscr,sizeof(*subscr),0) != sizeof(*subscr)){
            npd_syslog_err("failed to send subscription");
            free(subscr);
            return NPD_FAIL;
    }
    free(subscr);

	npd_syslog_dbg("Register sock fd %d and wait_server_handle to READ.\r\n", sd);
    circle_register_sock(sd, EVENT_TYPE_READ, wait_server_handle, NULL, NULL);
    return sd;
}

int wait_for_mcu_exist_server(struct tipc_name_seq* name,int wait)
{
    struct sockaddr_tipc topsrv = {0};
    struct tipc_subscr *subscr = malloc(sizeof(struct tipc_subscr));
    struct tipc_event event;
    int ret;
    int state;
    int evt;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    memset(subscr, 0, sizeof(struct tipc_subscr));
    subscr->seq.type = name->type;
    subscr->seq.lower = name->lower;
    subscr->seq.upper = name->upper;
    subscr->timeout = wait*1000;
    subscr->filter = TIPC_SUB_SERVICE;
        
    int sd = socket (AF_TIPC, SOCK_SEQPACKET,0);

    memset(&topsrv,0,sizeof(topsrv));
    topsrv.family = AF_TIPC;
    topsrv.addrtype = TIPC_ADDR_NAME;
    topsrv.addr.name.name.type = TIPC_TOP_SRV;
    topsrv.addr.name.name.instance = TIPC_TOP_SRV;

    /* Connect to topology server: */
    if (0 > (ret = connect(sd,(struct sockaddr*)&topsrv,sizeof(topsrv)))){
        if(errno != EINPROGRESS)
        {
            npd_syslog_err("failed to connect to topology server");
            free(subscr);
            return -1;
        }
        return NPD_SUCCESS;
    }
    
   
    if (send(sd,subscr,sizeof(*subscr),0) != sizeof(*subscr)){
            npd_syslog_err("failed to send subscription");
            free(subscr);
            return -1;
    }
    free(subscr);
    
    if (recv(sd,&event,sizeof(event),0) != sizeof(event)){
            npd_syslog_dbg("Chasm tipc recv from topology service error\r\n");
            evt = ACTIVE_MASTER_ENABLE;
    }
    else if (event.event == TIPC_PUBLISHED)
    {
        npd_syslog_dbg("Chasm mcu exist server published.\r\n");
        evt = STANDBY_MASTER_ENABLE;

    }
    else if(chassis_manage_high_priority(SYS_LOCAL_MODULE_SLOT_INDEX)
            || wait > CHASM_MCU_EXIST_TIMEOUT)
    {
        evt = ACTIVE_MASTER_ENABLE;
        npd_syslog_dbg("Chasm local mcu enable as active mcu, slot %d.\r\n", 
            SYS_LOCAL_MODULE_SLOTNO);
    }
    else
    {
        evt = STANDBY_MASTER_ENABLE;
        npd_syslog_dbg("Chasm local mcu enable as standby mcu, slot %d.\r\n", 
            SYS_LOCAL_MODULE_SLOTNO);
    }
    if((evt == ACTIVE_MASTER_ENABLE) 
        || wait > CHASM_MCU_EXIST_TIMEOUT
        || event.event == TIPC_PUBLISHED)
    {
        state = localmoduleinfo->runstate;
        (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
            (localmoduleinfo, localmoduleinfo, evt, NULL);
    }
    else
    {
        wait_for_mcu_exist_server(name, CHASM_SBY_CONNECT_TIMEOUT);
    }
        
    close(sd);
    return NPD_SUCCESS;
}
#endif
long local_master_init_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_master_init_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    struct tipc_name_seq name;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    name.type = SERVER_MCU_EXIST;
    name.lower = SYS_CHASSIS_MASTER_SLOT_INDEX(0)+1;
    name.upper = SYS_CHASSIS_MASTER_SLOT_INDEX(SYS_CHASSIS_MASTER_SLOTNUM-1)+1;

	npd_syslog_event("\nLocal master board %d wait active master board information...\r\n", board->slot_index+1);
	
    wait_for_mcu_exist_server(&name, CHASM_MCU_EXIST_TIMEOUT);
#endif	
	return NPD_SUCCESS;
}

long local_master_init_act_enable(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    chasm_circle_data_t *data = &chasm_circle_data[localmoduleinfo->slot_index];
   	struct sockaddr_tipc server_addr;
    struct tipc_name_seq name;
    int fd;
	int subslot_id;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("\nLocal master init as active master board.\r\n");
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    if(1 != app_slave_indpnt_get())
    {
        /*firstly, build the MCU_EXIST server*/
        fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
        if(0 > fd)
        {
            npd_syslog_err("Can not create chassis management tipc socket!\r\n");
            return NPD_FAIL;
        }
    
        server_addr.family = AF_TIPC;
        server_addr.addrtype = TIPC_ADDR_NAMESEQ;
        server_addr.addr.nameseq.type = SERVER_MCU_EXIST;
        server_addr.addr.nameseq.lower = SYS_LOCAL_MODULE_SLOT_INDEX+1;
        server_addr.addr.nameseq.upper = SYS_LOCAL_MODULE_SLOT_INDEX+1;
        server_addr.scope = TIPC_ZONE_SCOPE;
    
        /* Make server available: */
    
        if (0 != bind (fd, (struct sockaddr*)&server_addr,sizeof(server_addr)))
        {
              npd_syslog_err ("Chassis manager server failed to bind port name\r\n");
              return NPD_FAIL;
        }
    
        if (0 != listen (fd, 0)) 
        {
              npd_syslog_err ("Chassis manager server: Failed to listen\r\n");
              return NPD_FAIL;
        }
    	
    	npd_syslog_dbg("Publish SERVER_MCU_EXIST server at board slot_no %d \r\n", localmoduleinfo->slot_index+1);
    
        /*then, wait for sw insert server*/
        name.type = SERVER_CHASM_MANAGE;
        name.lower = 1;
        name.upper = SYS_CHASSIS_SLOTNUM;
        chasm_man_mon_sock = wait_for_server(&name, -1);
    }
#endif	
    localmoduleinfo->redundancystate = MASTER_ACTIVE;
    localmoduleinfo->workmode = MASTER_BOARD;
    SYS_MASTER_ACTIVE_SLOT_INDEX = localmoduleinfo->slot_index;
	if(SYS_CHASSIS_MASTER_SET_FUNC)
	    (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
	chasm_write_board_info();
	chasm_init_done();
    npd_syslog_dbg("CHASM init done, wait startup config done\r\n");
    chasm_unlock();

    chasm_wait_startup_config_done();
    chasm_lock();

    if(localmoduleinfo->online_removed == TRUE)
    {
        localmoduleinfo->online_removed = FALSE;
    }
    else
    {
        chasm_subboard_init_eth_ports(localmoduleinfo, 
                              localmoduleinfo->sub_board[0], PORT_NORMAL);
    }
    {
        int slot_num;
        for(slot_num = 1; 
            slot_num < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; 
            slot_num ++)
        {
            if(localmoduleinfo->sub_board[slot_num] != NULL)
            {
				if((localmoduleinfo->sub_board[slot_num]->online_removed == TRUE)
					&& (localmoduleinfo->sub_board[slot_num]->inserted))
				{
					localmoduleinfo->sub_board[slot_num]->online_removed = FALSE;
				}
				else if((localmoduleinfo->sub_board[slot_num]->fix_param)
					&& (localmoduleinfo->sub_board[slot_num]->inserted))
				{
                    chasm_subboard_init_eth_ports(localmoduleinfo, 
                              localmoduleinfo->sub_board[slot_num], PORT_NORMAL);
				}
				
				if(localmoduleinfo->sub_board[slot_num]->inserted)
					chasm_subboard_online_insert(localmoduleinfo, 
                              localmoduleinfo->sub_board[slot_num]);
            }
        }
    }
    
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)	
	nam_thread_create("NpdDBSync",(void *)npd_dbtable_thread_main,NULL,NPD_TRUE,NPD_FALSE);
#endif
    localmoduleinfo->runstate = LOCAL_ACTMASTER_DISCOVERING;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_READY;
	for(subslot_id = 1; subslot_id < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum;
	                    subslot_id++)
    {
		if(localmoduleinfo->sub_board[subslot_id])
		{
			if(localmoduleinfo->sub_board[subslot_id]->inserted)
			    localmoduleinfo->sub_board[subslot_id]->runstate = RMT_BOARD_RUNNING;
		}
    }
	npd_syslog_cslot_event("\n%% Local board works as active master board and is discovering other boards.\r\n");
	npd_syslog_dbg("rmtstate of Local board index %d  move to state RMT_BOARD_READY\r\n", localmoduleinfo->slot_index+1);
	data->board = localmoduleinfo;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	data->sock = fd;

	if (chasm_board_is_onlyone() 
		|| SYS_CHASSIS_ISSLAVESLOT(localmoduleinfo->slot_index)) /* for only live*/
#endif		
	{
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
        npd_dbtable_sync_alldone();
        npd_startup_end = 1;
        npd_eth_port_startup_end_update();
		
		npd_syslog_dbg("Only One Board %d itself.\r\n", data->board->slot_index+1);
	}
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	else
	{
	    circle_register_timeout(CHASM_MANTEST_TIMEOUT, 0, chasm_timeout, data, NULL);
		npd_syslog_dbg("Register CHASM_DISCOVING_TIMEOUT to board %d.\r\n", data->board->slot_index+1);
	}
#endif
    return NPD_SUCCESS;
}

long local_master_init_sby_enable(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
   	struct sockaddr_tipc server_addr = {0};
    int fd;
    int ret;
    chasm_circle_data_t *data;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("Local board slot_no %d enter local_master_init_sby_enable !\r\n", board->slot_index+1);
            
    data = &chasm_circle_data[localmoduleinfo->slot_index];
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket for board %d!\r\n"
            , localmoduleinfo->slot_index+1);
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_CHASM_MANAGE;
	server_addr.addr.nameseq.lower = localmoduleinfo->slot_index+1;
	server_addr.addr.nameseq.upper = localmoduleinfo->slot_index+1;
	server_addr.scope = TIPC_ZONE_SCOPE;

    ret = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_err("Can not bind tipc socket addr for board %d\r\n",
            board->slot_index+1);
        perror("TIPC socket bind error.");
        exit(1);
    }

    ret = listen(fd, 1);
    if(ret < 0)
    {
        npd_syslog_err("Can not listen tipc socket addr for board %d\r\n",
            board->slot_index+1);
        perror("TIPC socket listen error.");
        exit(1);
    }
	npd_syslog_dbg("Local board publish server SERVER_CHASM_MANAGE \r\n");
	
    data->board = localmoduleinfo;
    data->sock = fd;
    localmoduleinfo->runstate = LOCAL_SBYMASTER_WAIT_CONNECT;
    localmoduleinfo->redundancystate = MASTER_STANDBY;
    localmoduleinfo->workmode = MASTER_BOARD;
	npd_syslog_dbg("Local board state move to LOCAL_SBYMASTER_WAIT_CONNECT\r\n");
	npd_syslog_dbg("local board is MASTER_STANDBY, workmode is MASTER_BOARD\r\n");

    ret = circle_register_sock(fd, EVENT_TYPE_READ, chasm_accept_tipc, data, NULL);
    if(ret < 0)
    {
        npd_syslog_err("Can not register circle read sock\r\n");
        exit(1);
    }
	
	npd_syslog_dbg("Register timeout CHASM_WAIT_CONNECT_TIMEOUT to board %d \r\n", data->board->slot_index +1);
    ret = circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout, data, NULL);
    if(ret < 0)
    {
        npd_syslog_err("Can not register circle read sock\r\n");
        exit(1);
        
    }
#endif	
	return NPD_SUCCESS;
    
}

state_event_func_t local_master_init_funcs =
{
    &local_master_init_timer,
    &board_state_event_error,
    &local_master_init_swinsert,
    &board_state_event_error,
    &board_state_event_error,
    &local_master_init_act_enable,
    &local_master_init_sby_enable,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_sbymaster_wait_connect_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state;
    int ent;
    chasm_circle_data_t * data = &chasm_circle_data[board->slot_index];
    
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_err("\n%% Local standby master board can not connect to active master.\r\n");
    npd_syslog_cslot_err("\n%% Local master board change to active master board.\r\n");
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);

    ent = ACTIVE_MASTER_ENABLE;
    localmoduleinfo->runstate = LOCAL_MASTER_INIT;
    state = LOCAL_MASTER_INIT;
    return (*(*(localmoduleinfo->state_function)[state].funcs)[ent])
        (board, src_board, ent, NULL);
#endif
    return NPD_SUCCESS;
}

long local_sbymaster_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;
    char* reg_pdu;
    int len;
    int fd;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    fd = data->sock;

    /*if the state of local board is LOCAL_SBYMASTER_WAIT_CONNECT, the connecting board is ACT MASTER*/
    if(localmoduleinfo->runstate == LOCAL_SBYMASTER_WAIT_CONNECT)
    {
        chasm_circle_data_t *local_data = &chasm_circle_data[localmoduleinfo->slot_index];

        {
            char buf[4];
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%d", 0);
            write_to_file(CHASSIS_MAN_SLAVE_RUNNING_INDPNT_FILE, buf, strlen(buf));
        }

        circle_cancel_timeout(chasm_timeout, local_data, circle_ALL_CTX);
        chasm_assemble_reg_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
        chasm_send_pdu(src_board, reg_pdu, len);
        free(reg_pdu);

		npd_syslog_dbg("Local board register CHASM_REG_TIMEOUT to board %d\r\n", data->board->slot_index+1);
		circle_register_timeout(CHASM_REG_TIMEOUT, 0, &chasm_timeout, data, NULL);
		npd_syslog_dbg("Local board register sock %d to chasm_pdu_recv form board %d\r\n", 
			data->sock, data->board->slot_index + 1);
		circle_register_sock(fd, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);

		npd_syslog_dbg("Remote board %d reduncystate is MASTER_ACTIVE\r\n", src_board->slot_index + 1);
		npd_syslog_dbg("Remote board %d state move to RMT_BOARD_SWINSERTED\r\n", board->slot_index + 1);
		src_board->workmode = MASTER_BOARD;
		src_board->redundancystate = MASTER_ACTIVE;	
		
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;

        npd_syslog_cslot_event("\n%% Local standby master board connected to active master board %d and is registering...\r\n", src_board->slot_index+1);
        localmoduleinfo->runstate = LOCAL_SBYMASTER_REGISTERING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_REGISTERING;
    }
    else
    {
    }
#endif
    return NPD_SUCCESS;
}


state_event_func_t local_sbymaster_wait_connect_funcs =
{
    &local_sbymaster_wait_connect_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_tipc_connect,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_sbymaster_registering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);
    data->sock = 0;
#endif    
    return local_sbymaster_wait_connect_timer(localmoduleinfo, localmoduleinfo, event, pdu);
}

long local_sbymaster_registering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;
 
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_dbg("TIPC break\r\n");

    /*clear internal path sock*/
    data = &chasm_circle_data[src_board->slot_index];
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);
    data->sock = 0;

    /*clear accept sock*/
    data = &chasm_circle_data[localmoduleinfo->slot_index];
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);
    data->sock = 0;
    
    {
        int evt = ACTIVE_MASTER_ENABLE;
        localmoduleinfo->runstate = LOCAL_MASTER_INIT;
        src_board->runstate = RMT_BOARD_NOEXIST;
        src_board->redundancystate = MASTER_STANDBY;
        src_board->workmode = SLAVE_BOARD;
		src_board->fix_param = NULL;
		src_board->fix_spec_param = NULL;
        npd_syslog_cslot_err("\n%% Local standby master board can not connect to active master.\r\n");
        npd_syslog_cslot_err("\n%% Local master board change to active master board.\r\n");
		
        (*(*(localmoduleinfo->state_function)[LOCAL_MASTER_INIT].funcs)[ACTIVE_MASTER_ENABLE])
            (localmoduleinfo, localmoduleinfo, evt, NULL); 
    }
#endif    
    return NPD_SUCCESS;
    
}


long local_sbymaster_registering_reg_response(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_product_info_tlv_t *regres_tlv;
    char *status_pdu;
    int len;
	chasm_circle_data_t *data;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    regres_tlv = chasm_product_info_tlv_ntoh((chasm_product_info_tlv_t *)pdu);
	data = &chasm_circle_data[src_board->slot_index];

    npd_syslog_dbg("Local board receive register response pdu\r\n");
	circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
	if(chasm_debug_pkt)
    {
		npd_syslog_dbg("The register response pdu length is %d\n", regres_tlv->length);
		chasm_pdu_print(pdu, regres_tlv->length);    
    }

    if(SYS_PRODUCT_TYPE != regres_tlv->product_type)
    {
        npd_syslog_dbg("Local board product type is not same as MCU, reset product type to %s\r\n",
            device_product_type2name(regres_tlv->product_type));
        device_product_reset(regres_tlv->product_type);
        chassis_info_reinit();
        /*Need FIX , after chassis reset, board is point to wrong address*/
    }
    memcpy(SYS_PRODUCT_BASEMAC, regres_tlv->base_mac, 6);
    if(chasm_version_err((char*)regres_tlv->sw_version,regres_tlv->build_time))
    {
        npd_syslog_dbg("Local board software version error. state move to UPGRADING.\r\n");
        board->runstate = LOCAL_SBYMASTER_SW_UPGRADING;
        board->rmtstate[board->slot_index] = RMT_BOARD_SW_VERERR;
        /*board->fix_param->os_upgrade(board->slot_index);*/
        //return NPD_SUCCESS;
    }
    else
    {
        board->runstate = LOCAL_SBYMASTER_READY;
        board->rmtstate[board->slot_index] = RMT_BOARD_READY;
        npd_syslog_dbg("Local board state move to READY.\r\n");
    }
	
    chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
    chasm_send_pdu(src_board, status_pdu, len);
    free(status_pdu);
#endif	
	return NPD_SUCCESS;
}


state_event_func_t local_sbymaster_registering_funcs =
{
    &local_sbymaster_registering_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_tipc_connect,
    &local_sbymaster_registering_tipc_break,
    &board_state_event_error,
    &local_sbymaster_registering_reg_response,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_sbymaster_sw_vererr_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_sbymaster_sw_vererr_reg_response(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_sbymaster_sw_vererr_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_sbymaster_sw_vererr_swichover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	return NPD_SUCCESS;
}
long local_sbymaster_sw_vererr_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t local_sbymaster_sw_vererr_funcs =
{
    &local_sbymaster_sw_vererr_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_sw_vererr_reset,
    &board_state_event_error
};

long local_sbymaster_act_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int i;
    chasm_circle_data_t *data;
   	struct sockaddr_tipc server_addr = {0};
    struct tipc_name_seq name;
    int fd;
    char *switchover_pdu;
    int switchover_pdu_len;
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    if(!SYS_MODULE_ISMASTERACTIVE(src_board->slot_index))
        return NPD_SUCCESS;

    /*clear internal path sock*/
    data = &chasm_circle_data[src_board->slot_index];
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);
    data->sock = 0;

    /*clear accept sock*/
    data = &chasm_circle_data[board->slot_index];
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    close(data->sock);
    data->sock = 0;

	npd_syslog_dbg(" build the MCU_EXIST server\r\n");
    /*firstly, build the MCU_EXIST server*/
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket!\r\n");
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
    server_addr.addr.nameseq.type = SERVER_MCU_EXIST;
    server_addr.addr.nameseq.lower = SYS_LOCAL_MODULE_SLOT_INDEX+1;
    server_addr.addr.nameseq.upper = SYS_LOCAL_MODULE_SLOT_INDEX+1;
    server_addr.scope = TIPC_ZONE_SCOPE;

    /* Make server available: */

    if (0 != bind (fd, (struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
          npd_syslog_err ("Chassis manager server failed to bind port name\r\n");
          return NPD_FAIL;
    }

    if (0 != listen (fd, 0)) 
    {
          npd_syslog_err ("Chassis manager server: Failed to listen\r\n");
          return NPD_FAIL;
    }

	npd_syslog_dbg("change running board state to SWITCH_OVERING.\r\n");
	for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
	{
        int evt = SWITCHOVER_REPORT;
        int state = chassis_slots[i]->runstate;

		if (SYS_LOCAL_MODULE_SLOT_INDEX == i)
			continue;

        if (i == src_board->slot_index)
        {
            continue;
        }

		(*(*(chassis_slots[i]->state_function)[state].funcs)[evt])
            (chassis_slots[i], chassis_slots[i], evt, NULL);
	}

    name.type = SERVER_CHASM_MANAGE;
    name.lower = 1;
    name.upper = SYS_CHASSIS_SLOTNUM;
    chasm_man_mon_sock = wait_for_server(&name, -1);
	npd_syslog_dbg("local board %d move to SWITCHOVERING.\r\n", board->slot_index+1);
    chasm_assemble_switchover_report_pdu(localmoduleinfo, &switchover_pdu, 
        &switchover_pdu_len);
    chasm_broadcast_pdu(switchover_pdu, switchover_pdu_len);

    board->runstate = LOCAL_SBYMASTER_SWITCHOVERING;
    board->redundancystate = MASTER_ACTIVE;
    board->state_function = local_master_state_desc;

	data = &chasm_circle_data[localmoduleinfo->slot_index];
	data->sock = fd;
	circle_register_timeout(CHASM_SWITCHOVER_TIMEOUT, 0, chasm_timeout, data, NULL);
	npd_syslog_dbg("Register CHASM_SWITCHOVER_TIMEOUT to board %d", data->board->slot_index+1);
#endif
    return NPD_SUCCESS;	
}
long local_sbymaster_ready_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int i;
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(SYS_LOCAL_MODULE_SLOT_INDEX == i)
        {
            continue;
        }
        else
        {
            chassis_slots[i]->redundancystate = MASTER_STANDBY;
		   	if (chassis_slots[i]->runstate >= RMT_BOARD_READY && 
				chassis_slots[i]->runstate < RMT_BOARD_REMOVING)
		   	{
				chassis_slots[i]->online_removed = TRUE;
			}		
            chassis_slots[i]->runstate = RMT_BOARD_NOEXIST;
			
            if(localmoduleinfo->fix_spec_param->system_conn_deinit && chassis_slots[i]->fix_param)
                (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                        chassis_slots[i]->fix_param->board_type,
                        chassis_slots[i]->slot_index);
            if(snros_system_param->product_pp_feature_set)
                (*snros_system_param->product_pp_feature_set)();
        }
    }
    local_sbymaster_registering_tipc_break(board, src_board, event, pdu);
#endif	
    return NPD_SUCCESS;
}

long local_sbymaster_ready_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu);
    //int state = board->runstate;
    board_param_t * dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;
    int evt = STATUS_REPORT;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("ready_status_report: the status report dest_board slot no is %d\r\n", board_tlv->slotid+1);
	npd_syslog_dbg("dest_board's runstate is %d\r\n", state);

	
	if(chasm_debug_pkt)
    {
		chasm_pdu_print(pdu, board_tlv->length);
		npd_syslog_dbg("\r\nReceive packet length is %d byte\r\n", board_tlv->length);
        npd_syslog_dbg("\r\n");
    }

    if(localmoduleinfo == dest_board)
    {
		npd_syslog_dbg("localmoduleinfo == dest_board\r\n");
        if(board_tlv->board_type == PPAL_BOARD_TYPE_NONE)
            return NPD_SUCCESS;

        localmoduleinfo->rmtstate[src_board->slot_index] = board_tlv->runstate;;                  
		npd_syslog_dbg("localmoduleinfo %d rmtstate %d runstate is %d\r\n",
			localmoduleinfo->slot_index+1, src_board->slot_index+1, board_tlv->runstate);
        {
            if(board_tlv->runstate == RMT_BOARD_READY)
            {
				npd_syslog_dbg("board_tlv->runstate == RMT_BOARD_READY\r\n");

                int i;
                char *status_pdu;
                int len;

                chasm_write_board_info();
                chasm_init_done();
            	chasm_unlock();

                chasm_wait_startup_config_done();
            	npd_syslog_dbg("Local board init done\r\n");
            	
                chasm_lock();

                for(i = 0; i < ASIC_TYPE_MAX; i++)
                {
                    int j;
					if (NULL == localmoduleinfo->fix_spec_param->ams_param[i])
						continue;

					
                    for(j = 0; j < localmoduleinfo->fix_spec_param->ams_param[i]->num; j++)
                    {
						if (NULL != (localmoduleinfo->fix_spec_param->ams_param[i]->ams_enable))
						{
                        	(*localmoduleinfo->fix_spec_param->ams_param[i]->ams_enable)(j);
						}
                    }
                }

				npd_syslog_dbg("local board move to LOCAL_SBYMASTER_RUNNING\r\n");

                localmoduleinfo->runstate = LOCAL_SBYMASTER_RUNNING;
				localmoduleinfo->rmtstate[localmoduleinfo->slot_index] 
                    = RMT_BOARD_RUNNING;

                npd_syslog_cslot_event("\n%% Local board normally running as standby master.\r\n");

                chasm_assemble_status_report_pdu(localmoduleinfo, localmoduleinfo, &status_pdu, &len);
                chasm_send_pdu(src_board, status_pdu, len);
                free(status_pdu);
            }
        }
		return NPD_SUCCESS;
    }
    else
        return (*(*(dest_board->state_function)[state].funcs)[evt])
                (dest_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_sbymaster_ready_master_cmd(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_sbymaster_ready_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return board_state_event_error(board, src_board, event, pdu);
}

long local_sbymaster_ready_query(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}
long local_sbymaster_ready_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

state_event_func_t local_sbymaster_ready_funcs =
{
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_ready_tipc_break,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_ready_status_report,
    &local_sbymaster_ready_master_cmd,
    &local_sbymaster_ready_switchover_report,
    &local_sbymaster_ready_query,
    &local_sbymaster_ready_reset,
    &board_state_event_error
};

long local_sbymaster_running_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    snros_system_param->product_reset();
	return NPD_SUCCESS;
}


long local_sbymaster_running_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu);
    int evt = STATUS_REPORT;
    board_param_t * dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	if(localmoduleinfo == dest_board)
	{
		npd_syslog_dbg("dest_board is localmodule\r\n");

		return NPD_SUCCESS;
	}
		
    return (*(*(dest_board->state_function)[state].funcs)[evt])
                (dest_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_sbymaster_running_master_cmd(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}


long local_sbymaster_running_query(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}

long local_sbymaster_running_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;
}



long local_sbymaster_running_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    int ret = NPD_SUCCESS;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int i;
    
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    if(localmoduleinfo->fix_spec_param->system_conn_deinit)
        (*localmoduleinfo->fix_spec_param->system_conn_deinit)(SYS_PRODUCT_TYPE,
                src_board->fix_param->board_type,
                src_board->slot_index);
    if(snros_system_param->product_pp_feature_set)
        (*snros_system_param->product_pp_feature_set)();
    if(!SYS_MODULE_ISMASTERACTIVE(src_board->slot_index))
        return 0;
    npd_syslog_cslot_event("\n%% Local standby master lost connection with active master.\r\n");
    npd_syslog_cslot_event("\n%% Local standby master switchovering to active master.\r\n");
    
    ret = local_sbymaster_act_break(board, src_board, event, pdu);
   	if (src_board->runstate >= RMT_BOARD_READY && 
		src_board->runstate < RMT_BOARD_REMOVING)
   	{
		src_board->online_removed = TRUE;
	}				
    src_board->runstate = RMT_BOARD_NOEXIST;
    src_board->redundancystate = MASTER_STANDBY;
    board->runstate = LOCAL_ACTMASTER_RUNNING;
	SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;
	if(SYS_CHASSIS_MASTER_SET_FUNC)
	    (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
    chasm_write_board_info();
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
        if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
            npd_dbtable_slot_sync_done(i);
    }

    npd_dbtable_init();
    chasm_switchover_event();
    if(SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index))
    {
        chasm_subboard_online_remove(src_board, src_board);
    }
#endif
    return ret;	
}

long local_sbymaster_running_reg_req(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_reginfo_tlv_t *sub_tlv = 
        (chasm_board_reginfo_tlv_t *)pdu;
	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	/* sync the src_board name */
    memcpy(src_board->man_param.modname, sub_tlv->board_short_name, 32);
    memcpy(src_board->man_param.sn, sub_tlv->sn, 32);
#endif
	return 0;
}

long local_sbymaster_running_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    long ret = NPD_SUCCESS;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_cslot_event("\n%% Local standby master receive switchover request from active master.\r\n");
    npd_syslog_cslot_event("\n%% Local standby master switchovering to active master.\r\n");
	ret =  local_sbymaster_act_break(board, src_board, event, pdu);
    src_board->runstate = RMT_BOARD_SWITCHOVERING;
    src_board->redundancystate = MASTER_STANDBY;
#endif	
    return ret;
}

state_event_func_t local_sbymaster_running_funcs =
{
    &local_sbymaster_running_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,    
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_running_tipc_break,
    &local_sbymaster_running_reg_req,
    &board_state_event_error,
    &local_sbymaster_running_status_report,
    &local_sbymaster_running_master_cmd,
    &local_sbymaster_running_switchover_report,
    &local_sbymaster_running_query,
    &local_sbymaster_running_reset,
    &board_state_event_error
};

long local_sbymaster_error_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return NPD_SUCCESS;
}

state_event_func_t local_sbymaster_error_funcs =
{
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_sbymaster_error_reset,
    &board_state_event_error
};


long local_actmaster_discovering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;
	int i;
    chasm_circle_data_t *data = &chasm_circle_data[localmoduleinfo->slot_index];
	static int mantest_timeout = 0;
	int ready = 0;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_official_event("\nLocal active master board expires discovering timer.\r\n");    
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
        else
        {
            if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
            {
                chasm_subboard_online_insert(chassis_slots[i], chassis_slots[i]);
				chassis_slots[i]->online_removed = FALSE;
            }
			else if((chassis_slots[i]->runstate > RMT_BOARD_HWINSERTED)
				     && (0 == mantest_timeout))
			{
				ready = 1;
				mantest_timeout = 1;
			}
        }
    }
    if(ready)
    {
		circle_register_timeout(CHASM_DISCOVERING_TIMEOUT, 0, chasm_timeout, data, (void*)0);
		return NPD_SUCCESS;
    }
	if(1 == mantest_timeout)
	{
        localmoduleinfo->runstate = LOCAL_ACTMASTER_DBSYNCING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_READY;
    
        circle_register_timeout(CHASM_DBSYNC_TIMEOUT, 0, chasm_timeout, data, (void*)0);
    
    	npd_syslog_dbg("Local board move to state LOCAL_ACTMASTER_RUNNING -- %s\r\n", __FUNCTION__);  
        npd_syslog_official_event("\nLocal active master board is syncing data.\r\n");
	}
	else
	{
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
        npd_dbtable_sync_alldone();
        npd_startup_end = 1;
        npd_eth_port_startup_end_update();
		npd_syslog_official_event("\nLocal active master board is normally running.\r\n");
	}
	chasm_assemble_status_report_pdu(localmoduleinfo, localmoduleinfo, &status_pdu, &len);
	chasm_broadcast_pdu(status_pdu, len);
	free(status_pdu);
#endif	
	return NPD_SUCCESS;
}

long local_actmaster_discovering_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_REMOVE;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    npd_syslog_official_event("\nRemote board %d is removed.\n", src_board->slot_index+1);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_discovering_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_REMOVE;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}
long local_actmaster_discovering_hwinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_INSERT;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_discovering_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_INSERT;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_discovering_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    if(!SYS_MODULE_SLOT_ISMCU(SYS_LOCAL_MODULE_SLOT_INDEX)
        && SYS_MODULE_SLOT_ISMCU(src_board->slot_index))
    {
        (*chassis_sys_reset)();
    }
#endif	
	return NPD_SUCCESS;
}

long local_actmaster_discovering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = TIPC_BREAK;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_discovering_req_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = REGISTER_REQUEST;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("src_board %d runstate is %d", src_board->slot_index+1, state);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}


long local_actmaster_discovering_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu); 
    board_param_t *dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;
    int evt = STATUS_REPORT;
    int i, ready = TRUE;
	char *status_pdu;
    int len;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	npd_syslog_dbg("dest_board %d current runstate is %d \r\n", 
			dest_board->slot_index +1, dest_board->runstate);

	if (dest_board == localmoduleinfo)
	{
		chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu);

        npd_syslog_dbg("local board %d status report received from source board %d.", 
			board->slot_index+1, src_board->slot_index+1);
		npd_syslog_dbg("Board_tlv  runstate is %d.\r\n", board_tlv->runstate);
		
		npd_syslog_dbg("Remote board %d , src_board is %d.\r\n", board->slot_index+1, src_board->slot_index+1);

		localmoduleinfo->rmtstate[src_board->slot_index] = board_tlv->runstate;
		npd_syslog_dbg("board %d 's rmtstate %d is state %d.\r\n", localmoduleinfo->slot_index +1,
				src_board->slot_index+1, board_tlv->runstate);
	}
	else
	{
		(*(*(dest_board->state_function)[state].funcs)[evt])
        	(dest_board, src_board, evt, pdu);
	}
	
    
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
		if (i == SYS_LOCAL_MODULE_SLOT_INDEX)
			continue;
        if(chassis_slots[i]->inserted == FALSE)
        {
			continue;
        }
		//if the board had been READY, don't check
		if (chassis_slots[i]->runstate >= RMT_BOARD_READY)
			continue;
		
        if(chassis_slots[i]->runstate >= RMT_BOARD_REGISTERED)
        {
            if(chasm_board_isready(chassis_slots[i]))
            {
				//wait_for_dbtable_sync();
				chassis_slots[i]->runstate = RMT_BOARD_READY;
    			circle_cancel_timeout(chasm_timeout, &chasm_circle_data[chassis_slots[i]->slot_index], circle_ALL_CTX);
            	npd_syslog_cslot_event("\n%% Remote board %d is ready.\r\n", chassis_slots[i]->slot_index+1);
				chasm_ready_check(chassis_slots[i]);
                {
                    int j;
                    for(j = 0; j < SYS_MODULE_SUBSLOT_NUM(i); j++)
                    {
                        if(chassis_slots[i]->sub_board[j])
                        {
                            if(chassis_slots[i]->sub_board[j]->online_removed == FALSE)
                                chasm_subboard_init_eth_ports(chassis_slots[i], 
                                      chassis_slots[i]->sub_board[j], PORT_NORMAL);
                            chassis_slots[i]->sub_board[j]->online_removed = FALSE; 
                        }
                    }
                }

				
            	chasm_assemble_status_report_pdu(chassis_slots[i], chassis_slots[i], &status_pdu, &len);
            	chasm_broadcast_pdu(status_pdu, len);
			
            	free(status_pdu);		            			
            }
			else
			{
				npd_syslog_dbg("chassis_slot %d runstate has RMT_BOARD_REGISTERED. But it is nor ready.\r\n", i+1);			
                ready = FALSE;
				break;	// if  there is one board not ready , master has't need find others

			}
        }
		else
		{
			npd_syslog_dbg("chassis_slot %d runstate is %d , less RMT_BOARD_READY. ready is FALSE.\r\n", 
				i+1, chassis_slots[i]->runstate);
			ready = FALSE;			
			continue;
		}
    }
    if(ready)
    {
        chasm_circle_data_t *data = &chasm_circle_data[localmoduleinfo->slot_index];
		npd_syslog_official_event("\nAll remote boards have been ready.\r\n");
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
                continue;
            else
            {
                if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
                {
                    chasm_subboard_online_insert(chassis_slots[i], chassis_slots[i]);
					chassis_slots[i]->online_removed = FALSE;
                }
            }
        }
		npd_syslog_official_event("\nLocal active master board is going to sync database.\r\n");
        board->runstate = LOCAL_ACTMASTER_DBSYNCING;
        board->rmtstate[board->slot_index] = RMT_BOARD_READY;

        circle_register_timeout(CHASM_DBSYNC_TIMEOUT, 0, chasm_timeout, data, (void*)0);

        chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);
    }
#endif
	return NPD_SUCCESS;
    
}


state_event_func_t local_actmaster_discovering_funcs =
{
    &local_actmaster_discovering_timer,
    &local_actmaster_discovering_hwinsert,
    &local_actmaster_discovering_swinsert,
    &local_actmaster_discovering_hwremove,
    &local_actmaster_discovering_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_discovering_tipc_connect,
    &local_actmaster_discovering_tipc_break,
    &local_actmaster_discovering_req_request,
    &board_state_event_error,
    &local_actmaster_discovering_status_report,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error
};

long local_actmaster_running_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    chasm_assemble_status_report_all(&status_pdu, &len);
    chasm_broadcast_pdu(status_pdu, len);
    free(status_pdu);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_hwinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_INSERT;
    npd_syslog_official_event("\nRemote board %d is inserted.\r\n", src_board->slot_index+1);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_INSERT;
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_REMOVE;
    npd_syslog_official_event("\nRemote board %d is removed.\r\n", src_board->slot_index+1);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_REMOVE;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    if(!SYS_MODULE_SLOT_ISMCU(SYS_LOCAL_MODULE_SLOT_INDEX)
        && SYS_MODULE_SLOT_ISMCU(src_board->slot_index))
    {
		(*chassis_sys_reset)();
	}
#endif	
    return NPD_SUCCESS;   
}


long local_actmaster_running_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = TIPC_BREAK;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_req_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = REGISTER_REQUEST;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, pdu);
#endif
    return NPD_SUCCESS;
}

long local_actmaster_running_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu); 
    board_param_t *dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;
    int evt = STATUS_REPORT;

	char * status_pdu;
	int len;
	int i;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    if(dest_board == board)
        return NPD_SUCCESS;

	
    (*(*(dest_board->state_function)[state].funcs)[evt])
        (dest_board, src_board, evt, pdu);

	for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
		if (i == SYS_LOCAL_MODULE_SLOT_INDEX)
			continue;

		//The board's runstate must be between  [RMT_BOARD_REGISTERED , RMT_BOARD_READY)
		if (chassis_slots[i]->runstate >= RMT_BOARD_READY
			|| chassis_slots[i]->runstate < RMT_BOARD_REGISTERED)
			continue;

        if(chasm_board_isready(chassis_slots[i]))
        {
			
			//wait_for_dbtable_sync();
			chassis_slots[i]->runstate = RMT_BOARD_READY;
        	npd_syslog_official_event("\nRemote board %d is ready to running.\r\n", chassis_slots[i]->slot_index+1);

			circle_cancel_timeout(chasm_timeout, &chasm_circle_data[chassis_slots[i]->slot_index], circle_ALL_CTX);
        	chasm_assemble_status_report_pdu(chassis_slots[i], chassis_slots[i], &status_pdu, &len);
        	chasm_broadcast_pdu(status_pdu, len);
			chasm_ready_check(chassis_slots[i]);
        	free(status_pdu);	
            {
                int j;
                for(j = 0; j < SYS_MODULE_SUBSLOT_NUM(i); j++)
                {
                    if(chassis_slots[i]->sub_board[j])
                    {
                        if(chassis_slots[i]->sub_board[j]->online_removed == FALSE)
                            chasm_subboard_init_eth_ports(chassis_slots[i], 
                                  chassis_slots[i]->sub_board[j], PORT_NORMAL);
		                chasm_subboard_online_insert(chassis_slots[i], chassis_slots[i]);
                            
                        chassis_slots[i]->sub_board[j]->online_removed = FALSE; 
                    }
                }
            }

    	}
    }

	/*
    if(dest_board->runstate == RMT_BOARD_REGISTERED 
		&& chasm_board_isready(dest_board))
    {
		char * status_pdu;
		int len;
		dest_board->runstate = RMT_BOARD_READY;
        npd_syslog_dbg("Remote board %d state move to READY.\r\n", dest_board->slot_index+1);

		circle_cancel_timeout(chasm_timeout, &chasm_circle_data[dest_board->slot_index], NULL);				
        chasm_assemble_status_report_pdu(dest_board, dest_board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);			
        free(status_pdu);		            			

		
        if(dest_board->online_removed == FALSE)
        {
			//chasm_subboard_init_eth_ports(dest_board, dest_board);
			//npd_syslog_dbg("Init ether ports");
		}
          
	
        chasm_subboard_online_insert(dest_board, dest_board);
    }
	*/
	
    if(dest_board->runstate == RMT_BOARD_REMOVING 
		&& chasm_board_isremoved(dest_board))
    {
		char * status_pdu;
		int 	len;
		
		dest_board->runstate = RMT_BOARD_NOEXIST;
        npd_syslog_cslot_event("\n%% Remote board %d is removed.\r\n", dest_board->slot_index+1);

		circle_cancel_timeout(chasm_timeout, &chasm_circle_data[dest_board->slot_index], circle_ALL_CTX);	
		chasm_assemble_status_report_pdu(dest_board, dest_board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);

        //chasm_subboard_online_remove(dest_board, dest_board);
    }
#endif	
	return NPD_SUCCESS;
    
}

long local_actmaster_running_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data = &chasm_circle_data[localmoduleinfo->slot_index];
    char *switch_pdu;
    int len;

    /*业务板上报switch over消息表示系统存在双主的情况*/
	if(!SYS_MODULE_SLOT_ISMCU(src_board->slot_index))
	{
		/*双主的情况下，板号小的抢占为主，对板号大的主控板复位，期待能够使系统恢复正常*/
		if(chassis_manage_high_priority(SYS_LOCAL_MODULE_SLOT_INDEX))
		{
			char *switchover_pdu;
			int switchover_pdu_len;

			/*发送switch over报文使得业务板把主控切换回来*/
			/*因为另一块板已经发送了相关报文，所以此时业务板的主控信息已经错误*/
			chasm_assemble_switchover_report_pdu(localmoduleinfo, &switchover_pdu, 
                &switchover_pdu_len);
            chasm_broadcast_pdu(switchover_pdu, switchover_pdu_len);
			free(switchover_pdu);

            /*CPLD的信息已经被改写，需要改回来*/
			if(SYS_CHASSIS_MASTER_SET_FUNC)
	            (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
	        chasm_write_board_info();


            /*以下为试图恢复主控之间的物理通路函数*/
            {
				int master_num;
				int i;
				int slot_index;
            	master_num = SYS_CHASSIS_MASTER_SLOTNUM;
            	for (i = 0; i < master_num; i++)
            	{
            		slot_index = SYS_CHASSIS_MASTER_SLOT_INDEX(i); 
            		if(SYS_MODULE_ISMASTERSTANDBY(slot_index))
            		{
						chasm_circle_data_t *sby_master = &chasm_circle_data[slot_index];

	                    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
						/*只针对连接中断的备用主控做处理*/
						if(sby_master->sock == 0)
						{
							if(chassis_sys_reset_ext)
							    (*chassis_sys_reset_ext)(slot_index);

            			    if(localmoduleinfo->fix_spec_param->dual_master_handle)
            				    (*localmoduleinfo->fix_spec_param->dual_master_handle)(slot_index);
						}
            		}
            	}
				
            }

		}
		/*对自身进行复位*/
		else
		{
	        npd_syslog_dbg("********Entering function %s: reset myself********\r\n", __FUNCTION__);
			snros_local_board_spec->reset();
		}
		return NPD_SUCCESS;
	}

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    chasm_assemble_switchover_report_pdu(localmoduleinfo, &switch_pdu, &len);

    chasm_send_pdu(src_board, switch_pdu, len);
    free(switch_pdu);
    board->runstate = LOCAL_ACTMASTER_SWITCHOVERING;
    board->rmtstate[board->slot_index] = RMT_BOARD_SWITCHOVERING;
    src_board->runstate = RMT_BOARD_SWITCHOVERING;
    src_board->rmtstate[board->slot_index] = RMT_BOARD_SWITCHOVERING;
	npd_syslog_official_event("\nLocal active master is switchovering to standby master.\r\n");
    
	npd_syslog_dbg("Registering CHASM_SWITCHOVER_TIMEOUT to board %d\r\n", data->board->slot_index +1);
    circle_register_timeout(CHASM_SWITCHOVER_TIMEOUT, 0, chasm_timeout, data, NULL);
    data = &chasm_circle_data[src_board->slot_index];
    circle_register_timeout(CHASM_SWITCHOVER_TIMEOUT, 0, chasm_timeout, data, NULL);
#endif	
    return NPD_SUCCESS;
}


long local_actmaster_running_reset(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	return NPD_SUCCESS;    
}

state_event_func_t local_actmaster_running_funcs =
{
    &local_actmaster_running_timer,
    &local_actmaster_running_hwinsert,
    &local_actmaster_running_swinsert,
    &local_actmaster_running_hwremove,
    &local_actmaster_running_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_running_tipc_connect,
    &local_actmaster_running_tipc_break,
    &local_actmaster_running_req_request,
    &board_state_event_error,
    &local_actmaster_running_status_report,
    &board_state_event_error,
    &local_actmaster_running_switchover_report,
    &board_state_event_error,
    &local_actmaster_running_reset,
    &board_state_event_error
};


long local_actmaster_dbsyncing_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    char *status_pdu;
    int len;
    int i;
    int sync_done = FALSE;
    int times = (int)pdu;
    chasm_circle_data_t *data = &chasm_circle_data[localmoduleinfo->slot_index];
    
    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;

        else if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
        {
            sync_done = app_npd_sync_done_state_get(i);
            if(!sync_done)
            {
                break;
            }
        }
    }  
    times++;
    if(!sync_done) 
    {
        if(times >= NPD_DBSYNC_WAIT_MAX_TIMES)
        {
            localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
            localmoduleinfo->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
            npd_dbtable_sync_alldone();
            npd_startup_end = 1;
            npd_eth_port_startup_end_update();
            npd_syslog_official_event("\nDatabase syncing have been finished.\r\n");
			npd_syslog_official_event("\nLocal active master board is normally running.\r\n");
        }
        else
        {
            circle_register_timeout(CHASM_DBSYNC_TIMEOUT, 0, chasm_timeout, data, (void*)times);
        }
    }
    else
    {
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
        npd_dbtable_sync_alldone();
        npd_startup_end = 1;
        npd_eth_port_startup_end_update();
		npd_syslog_official_event("\nLocal active master board is normally running.\r\n");
    }
    
    chasm_assemble_status_report_all(&status_pdu, &len);
    chasm_broadcast_pdu(status_pdu, len);
    free(status_pdu);
#endif	
	return NPD_SUCCESS;
}

state_event_func_t local_actmaster_dbsyncing_funcs =
{
    &local_actmaster_dbsyncing_timer,
    &local_actmaster_running_hwinsert,
    &local_actmaster_running_swinsert,
    &local_actmaster_running_hwremove,
    &local_actmaster_running_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_running_tipc_break,
    &local_actmaster_running_req_request,
    &board_state_event_error,
    &local_actmaster_running_status_report,
    &board_state_event_error,
    &local_actmaster_running_switchover_report,
    &board_state_event_error,
    &local_actmaster_running_reset,
    &board_state_event_error
};

long local_sbymaster_switchovering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    //localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
    {
		int i;
		int origin_state = RMT_BOARD_READY;

		char * status_pdu;
		int len;
		
		npd_syslog_official_event("\nAll remote boards have been ready.\r\n");
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
			//
			if (SYS_MASTER_ACTIVE_SLOT_INDEX == i
				|| SYS_LOCAL_MODULE_SLOT_INDEX == i)
				continue;
            else
            {
				if(chassis_slots[i]->runstate > RMT_BOARD_READY)
				{
					npd_syslog_dbg("Notice the board %d enable ams.\r\n", i + 1);
					//send origin state is running board enable ams
					origin_state = chassis_slots[i]->runstate;
					chassis_slots[i]->runstate = RMT_BOARD_READY;

					chasm_assemble_status_report_pdu(chassis_slots[i], chassis_slots[i], &status_pdu, &len);
	            	chasm_broadcast_pdu(status_pdu, len);
	            	free(status_pdu);		            			

					chassis_slots[i]->runstate = origin_state;
				}

            }
		}

		SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;
		//first update dbtable
		npd_dbtable_init();

		npd_syslog_official_event("\nLocal active master is normally running.\r\n");
        board->runstate = LOCAL_ACTMASTER_RUNNING;
        board->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;

	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
		circle_cancel_timeout(chasm_timeout, &chasm_circle_data[localmoduleinfo->slot_index], circle_ALL_CTX);
        
        chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);
	}
#endif
    return NPD_SUCCESS;
}

long local_sbymaster_switchovering_hwinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_INSERT;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
#endif
    return NPD_SUCCESS;
}

long local_sbymaster_switchovering_swinsert(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_INSERT;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
    if(SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index))
    {
	    int i;   
        src_board->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
		SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;

	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
        chasm_write_board_info();
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
                continue;
            if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
                npd_dbtable_slot_sync_done(i);
        }

        npd_dbtable_init();
        chasm_switchover_event();
    }
#endif	
    return NPD_SUCCESS;    
}

long local_sbymaster_switchovering_hwremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = HW_REMOVE;
    int i;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);
    if(SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index))
    {
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
		SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;
	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);

        chasm_write_board_info();
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
                continue;
            if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
                npd_dbtable_slot_sync_done(i);
        }

        npd_dbtable_init();
        chasm_switchover_event();
    }
#endif	
    return NPD_SUCCESS;
}

long local_sbymaster_switchovering_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int state = src_board->runstate;
    int evt = SW_REMOVE;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    (*(*(src_board->state_function)[state].funcs)[evt])
        (src_board, src_board, evt, NULL);

    if(SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index))
    {
        int i;  
		localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
		SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;
	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);

        chasm_write_board_info();
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
                continue;
            if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
                npd_dbtable_slot_sync_done(i);
        }

        npd_dbtable_init();
        chasm_switchover_event();
    }
#endif	
    return NPD_SUCCESS;
}

long local_sbymaster_switchovering_switchover_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    return NPD_SUCCESS;
}

long local_sbymaster_switchovering_status_report(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	chasm_board_statusinfo_tlv_t *board_tlv = chasm_board_statusnfo_tlv_ntoh((chasm_board_statusinfo_tlv_t *)pdu); 
    board_param_t *dest_board = chassis_slots[board_tlv->slotid];
    int state = dest_board->runstate;
    int evt = STATUS_REPORT;

	char * status_pdu;
	int len;
	int i, ready = TRUE;

	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    if(dest_board == board)
        return NPD_SUCCESS;

    (*(*(dest_board->state_function)[state].funcs)[evt])
        (dest_board, src_board, evt, pdu);

    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
		if (i == SYS_LOCAL_MODULE_SLOT_INDEX
			|| SYS_MASTER_ACTIVE_SLOT_INDEX == i)
			continue;
		if (chassis_slots[i]->inserted == FALSE)
			continue;
	    if (chassis_slots[i]->runstate == RMT_BOARD_SWITCHOVERING)
		    ready = FALSE;
    }
    if(ready)
    {
	    int i;
		SYS_MASTER_ACTIVE_SLOT_INDEX = SYS_LOCAL_MODULE_SLOT_INDEX;
		chasm_write_board_info();
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
                continue;
            if(chassis_slots[i]->runstate >= RMT_BOARD_READY)
                npd_dbtable_slot_sync_done(i);
        }

        
		//first update dbtable
		npd_dbtable_init();
		npd_syslog_official_event("\nLocal active master board is normally running.\r\n");
        chasm_switchover_event();
        board->runstate = LOCAL_ACTMASTER_RUNNING;
        board->rmtstate[board->slot_index] = RMT_BOARD_RUNNING;
	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);

		circle_cancel_timeout(chasm_timeout, &chasm_circle_data[localmoduleinfo->slot_index], circle_ALL_CTX);
        
        chasm_assemble_status_report_pdu(board, board, &status_pdu, &len);
        chasm_broadcast_pdu(status_pdu, len);
        free(status_pdu);
    }
#endif	
	return NPD_SUCCESS;
}

state_event_func_t local_sbymaster_switchovering_funcs =
{
    &local_sbymaster_switchovering_timer,
    &local_sbymaster_switchovering_hwinsert,
    &local_sbymaster_switchovering_swinsert,
    &local_sbymaster_switchovering_hwremove,
    &local_sbymaster_switchovering_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_running_tipc_break,
    &local_actmaster_running_req_request,
    &board_state_event_error,
    &local_sbymaster_switchovering_status_report,
    &board_state_event_error,
    &local_sbymaster_switchovering_switchover_report,
    &board_state_event_error,
    &local_actmaster_running_reset,
    &board_state_event_error
};

long local_actmaster_switchovering_timer(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    chasm_circle_data_t *data = &chasm_circle_data[board->slot_index];
    circle_cancel_timeout(chasm_timeout, (void*)data, circle_ALL_CTX);
    if(SYS_MASTER_ACTIVE_SLOT_INDEX == localmoduleinfo->slot_index)
    {
        localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
        localmoduleinfo->runstate = LOCAL_ACTMASTER_RUNNING;
	    if(SYS_CHASSIS_MASTER_SET_FUNC)
	        (*SYS_CHASSIS_MASTER_SET_FUNC)(TRUE);
    }
    else
        snros_system_param->product_reset();
#endif	
    return NPD_SUCCESS;
}
long local_actmaster_switchovering_swremove(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    int fd = 0;
    int ret;
    chasm_circle_data_t *data = &chasm_circle_data[src_board->slot_index];
   	struct sockaddr_tipc server_addr = {0};
    int state = src_board->runstate;
    int evt = SW_REMOVE;

    if(!SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index)) 
    {
        return (*(*(src_board->state_function)[state].funcs)[evt])
            (src_board, src_board, evt, NULL);
    }
    if(SYS_MASTER_ACTIVE_SLOT_INDEX == -1)
        return NPD_SUCCESS;
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    if(data->sock)
    {
        close(data->sock);
        data->sock = 0;
    }
    /*close MCU_EXIST server*/
    data = &chasm_circle_data[localmoduleinfo->slot_index];
    circle_unregister_sock(data->sock, EVENT_TYPE_READ);
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    if(data->sock)
    {
        close(data->sock);
        data->sock = 0;
    }

    /*close CHASM_MANAGE monitor sock*/
    circle_unregister_sock(chasm_man_mon_sock, EVENT_TYPE_READ);
    close(chasm_man_mon_sock);
    chasm_man_mon_sock = 0;
    fd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
    if(0 > fd)
    {
        npd_syslog_err("Can not create chassis management tipc socket for board %d!\r\n"
            , localmoduleinfo->slot_index+1);
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_CHASM_MANAGE;
	server_addr.addr.nameseq.lower = localmoduleinfo->slot_index+1;
	server_addr.addr.nameseq.upper = localmoduleinfo->slot_index+1;
	server_addr.scope = TIPC_ZONE_SCOPE;

    ret = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        npd_syslog_err("Can not bind tipc socket addr for board %d\r\n",
            board->slot_index+1);
        perror("TIPC socket bind error.");
        exit(1);
    }

    ret = listen(fd, 1);
    if(ret < 0)
    {
        npd_syslog_err("Can not listen tipc socket addr for board %d\r\n",
            board->slot_index+1);
        perror("TIPC socket listen error.");
        exit(1);
    }
	npd_syslog_dbg("Local board publish server SERVER_CHASM_MANAGE \r\n");

    data->sock = fd;

    ret = circle_register_sock(fd, EVENT_TYPE_READ, chasm_accept_tipc, data, NULL);
    if(ret < 0)
    {
        npd_syslog_err("Can not register circle read sock\r\n");
        exit(1);
    }
	
	npd_syslog_dbg("Register timeout CHASM_WAIT_CONNECT_TIMEOUT to board %d \r\n", data->board->slot_index +1);
    ret = circle_register_timeout(CHASM_WAIT_CONNECT_TIMEOUT, 0, chasm_timeout, data, NULL);
    if(ret < 0)
    {
        npd_syslog_err("Can not register circle read sock\r\n");
        exit(1);
        
    }
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;
#endif	
    return NPD_SUCCESS;
}

long local_actmaster_switchovering_tipc_connect(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    chasm_circle_data_t *data;
    char* reg_pdu;
    int len;
    chasm_circle_data_t *local_data = &chasm_circle_data[localmoduleinfo->slot_index];

    if(!SYS_CHASSIS_ISMASTERSLOT(src_board->slot_index)) 
    {
        return NPD_SUCCESS;
    }

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    data = &chasm_circle_data[src_board->slot_index];
    circle_cancel_timeout(chasm_timeout, data, circle_ALL_CTX);
    circle_register_sock(data->sock, EVENT_TYPE_READ, chasm_pdu_recv, data, NULL);
    
    npd_syslog_official_event("\nLocal standby mcu connect to active master board %d\r\n", src_board->slot_index+1);
    circle_cancel_timeout(chasm_timeout, local_data, circle_ALL_CTX);
  
	src_board->workmode = MASTER_BOARD;
	src_board->redundancystate = MASTER_ACTIVE;	
    src_board->runstate = RMT_BOARD_RUNNING;
    src_board->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;
    src_board->rmtstate[src_board->slot_index] = RMT_BOARD_RUNNING;

    localmoduleinfo->redundancystate = MASTER_STANDBY;
    localmoduleinfo->runstate = LOCAL_SBYMASTER_RUNNING;
    localmoduleinfo->rmtstate[localmoduleinfo->slot_index] = RMT_BOARD_RUNNING;

    SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
    chasm_write_board_info();
    npd_dbtable_init();
    chasm_switchover_event();

	npd_syslog_official_event("\nLocal board normally running as standby master.\r\n");
    chasm_assemble_status_report_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
    chasm_send_pdu(src_board, reg_pdu, len);
    free(reg_pdu);
#endif    
    return NPD_SUCCESS;
    
}


long local_actmaster_switchovering_tipc_break(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
   	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    return local_actmaster_switchovering_swremove(board, src_board, event, pdu);
}

state_event_func_t local_actmaster_switchovering_funcs =
{
    &local_actmaster_switchovering_timer,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_switchovering_swremove,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_switchovering_tipc_connect,
    &local_actmaster_switchovering_tipc_break,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &board_state_event_error,
    &local_actmaster_running_reset,
    &board_state_event_error
};

state_desc_t local_master_state_desc[] =
{
    {
        LOCAL_MASTER_INIT, 
        "master init",
        &local_master_init_funcs
    },
    {
        LOCAL_SBYMASTER_WAIT_CONNECT, 
        "local standby master wait for connection",
        &local_sbymaster_wait_connect_funcs
    },
    {
        LOCAL_SBYMASTER_REGISTERING, 
        "local standby master registering",
        &local_sbymaster_registering_funcs
    },
    {
        LOCAL_SBYMASTER_SW_UPGRADING, 
        "local standby master registering",
        &local_sbymaster_sw_vererr_funcs
    },
    {
        LOCAL_SBYMASTER_READY, 
        "local standby master ready",
        &local_sbymaster_ready_funcs
    },
    {
        LOCAL_SBYMASTER_RUNNING, 
        "local standby master ready",
        &local_sbymaster_running_funcs
    },
    {
        LOCAL_SBYMASTER_SWITCHOVERING,
        "local standby master switchovering",
        &local_sbymaster_switchovering_funcs
    },

    {
        LOCAL_SBYMASTER_ERROR, 
        "local standby master error",
        &local_sbymaster_error_funcs
    },
    {
        LOCAL_ACTMASTER_DISCOVERING, 
        "local active master discovering",
        &local_actmaster_discovering_funcs
    },
    {
        LOCAL_ACTMASTER_DBSYNCING, 
        "local active master running",
        &local_actmaster_dbsyncing_funcs
    },
    {
        LOCAL_ACTMASTER_RUNNING, 
        "local active master running",
        &local_actmaster_running_funcs
    },
    {
        LOCAL_ACTMASTER_SWITCHOVERING,
        "local active master switchovering",
        &local_actmaster_switchovering_funcs
    }
        
};


#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)	

void chasm_board_oir_handle(int sock, void *circle_data, void *user_data)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	if(SYS_CHASSIS_INTERRUPT_HANDER_FUNC)
	    (*SYS_CHASSIS_INTERRUPT_HANDER_FUNC)();
}
#endif
extern int g_bm_fd;
int chassis_manage_board_check_init()
{
    int i;
    int insert;


    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        
        if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
            continue;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)        
        /*for dbtable sync flag, for avoid switchover complex logic*/
        npd_dbtable_slot_sync_done(i);
        
        insert = CHASSIS_SLOT_INSERTED(i);
        if(insert)
        {
            int state, event;

            chasm_lock();
            state = chassis_slots[i]->runstate;
            event = HW_INSERT;
            (*(*(chassis_slots[i]->state_function)[state].funcs)[event])
                (chassis_slots[i], chassis_slots[i], event, NULL);
            chasm_unlock();
        }
#endif		
    }

	if (SYS_LOCAL_MODULE_IS_MCU)
	{
		npd_syslog_dbg("system power and fan info init \n");
		npd_pne_info_init();		
	}
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)	
    circle_register_sock(g_bm_fd, EVENT_TYPE_READ, chasm_board_oir_handle, NULL, NULL);
    circle_register_sock(g_bm_fd, EVENT_TYPE_EXCEPTION, chasm_board_oir_handle, NULL, NULL);
#endif
	return NPD_SUCCESS;    
}

#ifdef HAVE_AC_BOARD /* This is for  AC compatile code*/


/* for adapter ac board*/
#if 1

#define NOT_RESET		0
#define HAS_RESET		!NOT_RESET

int g_reset_product_state = NOT_RESET;

int chasm_set_product_reset_state(int state)
{
	g_reset_product_state = state;
	return NPD_SUCCESS;
}

int chasm_get_product_reset_state(int * state)
{
	if (state == NULL)
		return NPD_FAIL;
	
	*state = g_reset_product_state ;

	return NPD_SUCCESS;
}
#endif

enum
{
	SEM_CMD_PDU_STBY_ACTIVE_LINK_TEST_REQUEST,			
	SEM_CMD_PDU_STBY_ACTIVE_LINK_TEST_RESPONSE,			
	SEM_FORCE_STBY,										
	SEM_NON_ACTIVE_BOARD_REGISTER,						
	SEM_ACTIVE_MASTER_BOARD_SYN_PRODUCT_INFO,			
	SEM_BOARD_INFO_SYN,									
	SEM_SOFTWARE_VERSION_SYN_REQUEST,					
	SEM_SOFTWARE_VERSION_SYN_RESPONSE,					
	SEM_SOFTWARE_VERSION_SYNING,						
	SEM_SOFTWARE_VERSION_SYN_FINISH,
	SEM_SOFTWARE_VERSION_SYNC_SUCCESS,
	SEM_FILE_SYNING,									
	SEM_FILE_SYN_FINISH,								
	SEM_ETHPORT_INFO_SYN,								
	SEM_NETLINK_MSG,									
	SEM_CONNECT_CONFIRM,								
	SEM_CONNECT_REQUEST,								
	SEM_HARDWARE_RESET,									
	SEM_HARDWARE_RESET_PREPARE,							
	SEM_RESET_READY,									
	SEM_MCB_ACTIVE_STANDBY_SWITCH_NOTIFICATION,
	SEM_SET_SYSTEM_IMG,
	SEM_SET_SYSTEM_IMG_REPLY,
	SEM_DISABLE_KEEP_ALIVE_TEMPORARILY,
	SEM_EXECUTE_SYSTEM_COMMAND,
	SEM_COMPATIBLE_SYNC_PRODUCT_INFO, /* 25 */
	SEM_COMPATIBLE_BOARD_REGIST,		/* 26 */
	SEM_CMD_PDU_MAX
};

enum board_function_type{
	UNKNOWN_BOARD = 0,
	NONCOMPAT_MASTER_BOARD = 0x1,		/*master board*/
	AC_BOARD = 0x2,			/*wireless business board*/
	SWITCH_BOARD = 0x4,		/*switch board*/
	BASE_BOARD = 0x8,		/*Bas board*/
	NAT_BOARD = 0x10		/*NAT board*/
};


 enum board_state
{
	BOARD_INSERTED_AND_REMOVED, /*to describe the slot were insert a board,and the board is removed for some reasons.*/
	BOARD_EMPTY_NONCOMPAT,
	BOARD_REMOVED_NONCOMPAT,
	BOARD_INSERTED_NONCOMPAT,
	BOARD_INITIALIZING,
	BOARD_READY,
	BOARD_RUNNING
};
#define IS_DEAD			0
#define IS_ALIVING		!IS_DEAD

#define TIPC_SEM_TYPE_LOW	0x1000
#define TIPC_SEM_TYPE_HIGH	0x1fff

#define MAX_SLOT_NUM			20

#define SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE	1121
#define SEM_TIPC_INSTANCE_BASE_NON_ACTIVE_TO_ACTIVE	SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE+MAX_SLOT_NUM

#define MAX_BOARD_NAME_LEN			32	


typedef struct sem_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned char body[0];
}sem_tlv_t;


typedef struct sem_pdu_head_s
{
    unsigned long  version;
    unsigned long  length;
	int slot_id;
    struct sem_tlv_s tlv[0];
}sem_pdu_head_t;

typedef char bool;

typedef struct
{
	int board_code;
	int board_type;
	int slot_id;
	char name[MAX_BOARD_NAME_LEN];
	bool is_master;
	bool is_active_master;
	bool is_use_default_master;
	unsigned int function_type;
	int board_state;
	unsigned int asic_start_no;
	unsigned int asic_port_num;
}board_info_syn_t;


typedef struct sem_link_test_s
{
	unsigned short type;
	unsigned long length;
}sem_link_test_t;

struct chasm_circle_data_s *chasm_circle_noncompat;
int g_active_mcb_state = IS_DEAD;

/*
 *
 *return val:0 link on;1 link failed;-1 error
 */
int chasm_active_link_test(int test_count, int master_slot)
{	

#define SEM_TIPC_RECV_BUF_LEN		1024	
#define SEM_TIPC_SEND_BUF_LEN		SEM_TIPC_RECV_BUF_LEN

	int ret ;
	char *link_request_pdu, *link_echo_pdu;
	int len;
	int flag = 0;
	int temp_test_count;
	fd_set rdfds;
	unsigned int time_out = 5;
	sem_pdu_head_t * head;
	sem_tlv_t *tlv_head;
	
	board_info_syn_t *board_info_head;
	struct timeval wait_time;
	wait_time.tv_sec = 0;
	wait_time.tv_usec = 1000000;
	int temp_sd;
	
    char send_buf[SEM_TIPC_SEND_BUF_LEN] = {0};
	char recv_buf[SEM_TIPC_RECV_BUF_LEN];


    int send_sd = (&chasm_circle_noncompat[master_slot])->sock;;
    int recv_sd = (&chasm_circle_noncompat[localmoduleinfo->slot_index])->sock;;
	struct sockaddr_tipc send_server_addr;
	/* this send instance is used in AC */
	unsigned int send_instance = 
		SEM_TIPC_INSTANCE_BASE_NON_ACTIVE_TO_ACTIVE+localmoduleinfo->slot_index;
	
	unsigned int recv_instance = 
		SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE+localmoduleinfo->slot_index;
	struct sockaddr_tipc recv_server_addr;
	
 	send_server_addr.family = AF_TIPC;
	send_server_addr.addrtype = TIPC_ADDR_NAME;
	send_server_addr.addr.name.name.type = TIPC_SEM_TYPE_LOW;
	send_server_addr.addr.name.name.instance = send_instance;
	send_server_addr.addr.name.domain = 0;	

	recv_server_addr.family = AF_TIPC;
	recv_server_addr.addrtype = TIPC_ADDR_NAME;
	recv_server_addr.addr.name.name.type = TIPC_SEM_TYPE_LOW;
	recv_server_addr.addr.name.name.instance = send_instance;
	recv_server_addr.addr.name.domain = 0;

	head = (sem_pdu_head_t *)send_buf;
	head->slot_id = localmoduleinfo->slot_index;
	head->version = 11;
	head->length = 22;

	tlv_head = (sem_tlv_t *)(send_buf + sizeof(sem_pdu_head_t));
	tlv_head->type = SEM_CONNECT_REQUEST;
	tlv_head->length = 33;

	int addr_len;

	while (test_count)
	{
		if (sendto(send_sd, send_buf, sizeof(sem_pdu_head_t)+sizeof(sem_link_test_t), 0, 
			(struct sockaddr*)&send_server_addr, sizeof(struct sockaddr_tipc)) < 0)
		{
			npd_syslog_dbg("	link request:send failed\n");
			continue;
		}
		else
		{
			npd_syslog_dbg("	link request:send succeed\n");
			while (time_out)
			{
				FD_ZERO(&rdfds);
				FD_SET(recv_sd, &rdfds);

				ret = select((recv_sd)+1, &rdfds, NULL, NULL, &wait_time);

				if (ret < 0)
				{
				}
				else if (ret == 0)
				{
				}
				else
				{
					if (recvfrom(recv_sd, recv_buf, sizeof(recv_buf), 0, 
						(struct sockaddr *)&recv_server_addr, &addr_len) <= 0)
					{
						npd_syslog_warning("recv wrong mesg\n");
						break;
					}
					else
					{
						head = (sem_pdu_head_t *)recv_buf;
						tlv_head = (sem_tlv_t *)(recv_buf + sizeof(sem_pdu_head_t));

						switch (tlv_head->type)
						{
							case SEM_CONNECT_CONFIRM:
								npd_syslog_dbg("	recv connnect confirm msg\n");
								flag = 1;
								break;
							default :
								npd_syslog_dbg("	recv message type %d\n", tlv_head->type);
								flag = 0;
								break;
						}
					}
				}
				
				if (flag)
					break;
				
				time_out--;
				//sleep(1);
			}
		}

		if (flag)
		{
			break;	
		}
		test_count--;
	}
	
	if (test_count == 0)
	{
		npd_syslog_warning("	master is not exist\n");
		return 1;
	}
	else
	{
		g_active_mcb_state = IS_ALIVING;
		return 0;
	}
}


long chasm_send_noncompat_pdu(board_param_t *dst_board, char *pdu, int len)
{
	struct sockaddr_tipc server_addr;
	/* this send instance is used in AC */
	unsigned int send_instance = 
		SEM_TIPC_INSTANCE_BASE_NON_ACTIVE_TO_ACTIVE+localmoduleinfo->slot_index;
    chasm_circle_data_t *data = &chasm_circle_noncompat[dst_board->slot_index];
    int sock = data->sock;
    int ret;
	
    if(0 == sock)
        return 0;

	//npd_syslog_dbg("Send packet to board %d:\r\n", dst_board->slot_index+1);
    if(chasm_debug_pkt)
    {
		chasm_pdu_print(pdu, len);
		npd_syslog_dbg("the length is %d byte\r\n", len);
        npd_syslog_dbg("\r\n");
    }

 	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = TIPC_SEM_TYPE_LOW;
	server_addr.addr.name.name.instance = send_instance;
	server_addr.addr.name.domain = 0;	

	ret = sendto(sock, pdu, len, 0,
		(struct sockaddr*)&server_addr,
		 sizeof(struct sockaddr_tipc));
    if(ret < 0)
    {
        npd_syslog_err("TIPC socket send error, errno 0x%x.\r\n", errno);      
        return NPD_FAIL;
    }
	return NPD_SUCCESS;
}

long chasm_assemble_noncompat_req_pdu(board_param_t *board, board_param_t *sub_board, char**pdu, int *len)
{
    sem_pdu_head_t *head;
    sem_tlv_t *tlv;
	board_info_syn_t *board_info_head;

    npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
    *len = sizeof(sem_pdu_head_t) + sizeof(sem_tlv_t);
	*len += sizeof(board_info_syn_t);

    (*pdu) = malloc(*len);
    if(NULL == *pdu)
    {
        npd_syslog_err("Can not get memory for chasm packets.\r\n");
        *len = 0;
        return NPD_FAIL; 
    }
    memset(*pdu, 0, *len);

    head = (sem_pdu_head_t*)(*pdu);
	head->slot_id = localmoduleinfo->slot_index;
    head->version = chasm_protocol_version();
    head->length = *len;
	
    tlv  = (sem_tlv_t *)(head + 1);
	tlv->type = SEM_COMPATIBLE_BOARD_REGIST;
	tlv->length = *len - sizeof(sem_pdu_head_t);

	board_info_head = (board_info_syn_t *)(tlv+1);
	board_info_head->board_code = board->fix_param->board_code;
	board_info_head->board_type = 0;
	board_info_head->slot_id = board->slot_index;
	memcpy(board_info_head->name, board->man_param.modname, 
		strlen(board->man_param.modname)+1);
	board_info_head->function_type= SWITCH_BOARD;
	board_info_head->board_state = BOARD_READY;
	board_info_head->asic_port_num = board->fix_param->panel_portnum;

    if(chasm_debug_pkt)
    {
		chasm_pdu_print(*pdu, *len);
		npd_syslog_dbg("\r\nThe noncompat req pdu length is %d\n", *len);
        npd_syslog_dbg("\r\n");
    }
	return NPD_SUCCESS;
}

long chasm_assemble_noncompat_connectreq_pdu(board_param_t *board, board_param_t *sub_board, char**pdu, int *len)
{
	sem_pdu_head_t *head;
	sem_tlv_t *tlv_head;

	*len = sizeof(sem_pdu_head_t) + sizeof(sem_tlv_t);
	(*pdu) = malloc(*len);
	if ((*pdu) == NULL)
	{
		exit(1);
	}
	head = (sem_pdu_head_t *)(*pdu);
	head->slot_id = localmoduleinfo->slot_index;

	tlv_head = (sem_tlv_t *)((*pdu) + sizeof(sem_pdu_head_t));
	tlv_head->type = SEM_CONNECT_REQUEST;
		
	return NPD_SUCCESS;
}

long chasm_assemble_noncompat_linktest_pdu(board_param_t *board, board_param_t *sub_board, char**pdu, int *len)
{
	sem_pdu_head_t *head;
	sem_tlv_t *tlv_head;

	*len = sizeof(sem_pdu_head_t) + sizeof(sem_tlv_t);
	(*pdu) = malloc(*len);
	if ((*pdu) == NULL)
	{
		exit(1);
	}
	head = (sem_pdu_head_t *)(*pdu);
	head->slot_id = localmoduleinfo->slot_index;

	tlv_head = (sem_tlv_t *)((*pdu) + sizeof(sem_pdu_head_t));
	tlv_head->type = SEM_CMD_PDU_STBY_ACTIVE_LINK_TEST_REQUEST;
		
	return NPD_SUCCESS;	
}


void chasm_noncompat_reg_request(board_param_t *board, board_param_t *src_board, state_event_e event, char *pdu)
{
    int fd;
    chasm_circle_data_t *data;
    char *reg_pdu;
    int len;
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

	/* create socket to send master. */
	data = &chasm_circle_noncompat[src_board->slot_index];
	fd = socket(AF_TIPC, SOCK_RDM, 0);
	if(0 > fd)
	{
	    npd_syslog_cslot_err("\n%% Can not create chassis management tipc socket for board %d!\r\n"
	        , localmoduleinfo->slot_index+1);
	    return ;
	}
	data->sock = fd;
	

    if(-1 == SYS_MASTER_ACTIVE_SLOT_INDEX)
    {
        SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;
    }
	
    {
		npd_syslog_dbg("Test to board %d Link up \r\n", src_board->slot_index+1);
		int ret = 0;
		ret =chasm_active_link_test(10, src_board->slot_index);
		if (ret != 0)
		{
			npd_syslog_dbg("Can't Connect to Master board %d.\r\n", src_board->slot_index+1);
			//snros_system_param->product_reset();
			//return 0;
		}
		
		npd_syslog_dbg("send noncompat regist req to board %d\r\n", src_board->slot_index+1);
        chasm_assemble_noncompat_req_pdu(localmoduleinfo, localmoduleinfo, &reg_pdu, &len);
        chasm_send_noncompat_pdu(src_board, reg_pdu, len);
        free(reg_pdu);

		npd_syslog_dbg("board %d move to RMT_BOARD_READY\r\n", src_board->slot_index+1);
		src_board->workmode = MASTER_BOARD;
		src_board->redundancystate = MASTER_ACTIVE;
		src_board->runstate = RMT_BOARD_READY;
			
       SYS_MASTER_ACTIVE_SLOT_INDEX = src_board->slot_index;       
    }
	
    return ;
}

int npd_noncompat_keep_active_thread
(
    void
)
{
	char *link_pdu;
	int link_pdu_len;
	char * connect_pdu;
	int connect_pdu_len;
	board_param_t* master_board = 
		chassis_slots[SYS_MASTER_ACTIVE_SLOT_INDEX];
	
	npd_init_tell_whoami("noncomKeepalive", (unsigned char)0);
	
	/* data = &chasm_circle_data[SYS_MASTER_ACTIVE_SLOT_INDEX]; */
	chasm_assemble_noncompat_linktest_pdu(localmoduleinfo,
				localmoduleinfo, &link_pdu, &link_pdu_len);
	chasm_assemble_noncompat_connectreq_pdu(localmoduleinfo,
				localmoduleinfo, &connect_pdu, &connect_pdu_len);	
	
	
	while(1)
	{

		/* tell the Noncompat board, I'm alive */
		chasm_send_noncompat_pdu(master_board, link_pdu, link_pdu_len);
		
		sleep(1);
		
		if (g_active_mcb_state != IS_ALIVING)
		{

			int test_count = 10;
			while(test_count--)
			{				
				chasm_send_noncompat_pdu(
					master_board, connect_pdu, connect_pdu_len);
				sleep(10);
				if (g_active_mcb_state == IS_ALIVING)
					break;
			}
			/* wait the 100s can't link the master board , reset itself */
			if (g_active_mcb_state != IS_ALIVING)
			{
				snros_system_param->product_reset();
			}			
		}
		g_active_mcb_state = IS_DEAD;
	}
}

void chasm_noncompat_pdu_recv(int sock, void *circle_data, void *user_data)
{
    chasm_circle_data_t *data = (chasm_circle_data_t*)circle_data;
    board_param_t *board = data->board;
    char *pdu;
    int len;
    sem_pdu_head_t *head;
    sem_tlv_t *tlv;
    char *buf;
    int state;
    int evt;
	unsigned int recv_instance = 
		SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE+localmoduleinfo->slot_index;
	struct sockaddr_tipc server_addr;
	int addr_len;
 
	//npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	//npd_syslog_dbg("from sock fd %d\r\n", sock);
    pdu = malloc(2048);
    
    if(NULL == pdu)
    {
        npd_syslog_err("Can not alloc memory for chasm packet receiving.\r\n");
        return;
    }

	server_addr.family = AF_TIPC;
   	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = TIPC_SEM_TYPE_LOW;
    server_addr.addr.nameseq.lower = recv_instance;
    server_addr.addr.nameseq.upper = recv_instance;
	server_addr.scope = TIPC_CLUSTER_SCOPE;	
	
    len = recvfrom(sock, pdu, 2048, 0,
		(struct sockaddr *)&server_addr, &addr_len);    
    if(len < 0)
    {
        npd_syslog_err("Failed to receive chasm packet.\r\n");
        free(pdu);
        return;
    }
	
	if(chasm_debug_pkt)
    {
			
        npd_syslog_dbg("chasm_pdu_recv<header>: Receive packet from board %d:\r\n", board->slot_index+1);		
		chasm_pdu_print(pdu, len);
		npd_syslog_dbg("\r\n Receive packet length is %ld byte\r\n", len);
        npd_syslog_dbg("\r\n");
		
    }

    if(len < sizeof(sem_pdu_head_t))
    {
        npd_syslog_err("TIPC packets error, lenght is %d\r\n", len);
        free(pdu);
        return;
    }
	
	head = (sem_pdu_head_t *)pdu;
    if(head->length > CHASM_PACKET_MAX_LENGTH)
    {
        npd_syslog_err("The packet length %u is longer than allowed length.\r\n",
            head->length);
        free(pdu);
        return;
    }
	/* 
    if(len < head->length-sizeof(sem_pdu_head_t))
    {
        npd_syslog_err("TIPC packets error, length is %d, should be %d\r\n", len, 
            head->length-sizeof(sem_pdu_head_t));
        free(pdu);
        return;
    }
    */
	g_active_mcb_state=IS_ALIVING;
    {
		buf = pdu + sizeof(sem_pdu_head_t);
    
        tlv = (sem_tlv_t *)buf;

		if(chasm_debug_pkt)
		{
			chasm_pdu_print(pdu, tlv->length);

			npd_syslog_dbg("chasm_pdu_recv<header>: Receive packet from board %d:\r\n", board->slot_index+1);		
			chasm_pdu_print(pdu, tlv->length);
			npd_syslog_dbg("\r\n process packet length is %d byte\r\n", tlv->length);
	        npd_syslog_dbg("\r\n");
		}
		
        switch(tlv->type)
        {
            case SEM_COMPATIBLE_SYNC_PRODUCT_INFO:
                if(SYS_CHASSIS_ISMASTERSLOT(head->slot_id) 
					&& g_reset_product_state==NOT_RESET)
                {
					npd_syslog_dbg("BEGIN:tlv->type == SEM_COMPATIBLE_SYNC_PRODUCT_INFO BEGIN.\r\n");

					/* reset the product type */
                    chasm_lock();
                    state = localmoduleinfo->runstate;
                    evt = NONCOMPAT_REQUEST;
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, board, evt, buf);
                    chasm_unlock();
						
					/* switch the boad type  */
                    chasm_lock();
					
					localmoduleinfo->state_function = local_master_state_desc;
					state = LOCAL_MASTER_INIT;
					evt = ACTIVE_MASTER_ENABLE;
                    localmoduleinfo->runstate = LOCAL_MASTER_INIT;
                    
                    (*(*(localmoduleinfo->state_function)[state].funcs)[evt])
                        (localmoduleinfo, localmoduleinfo, evt, buf);
                    chasm_unlock();

					board = chassis_slots[head->slot_id];
					
					chasm_noncompat_reg_request(localmoduleinfo, board, 0, NULL);
					/* creat a compatile thread to  Keepalive   */
					nam_thread_create("KeepAlive",(void *)npd_noncompat_keep_active_thread,NULL,NPD_TRUE,NPD_FALSE);
					
					npd_syslog_dbg("END: tlv->type == SEM_COMPATIBLE_SYNC_PRODUCT_INFO END.\r\n");

					npd_syslog_dbg("srcboard %d workmode is MASTER %d , reduncy is ACTIVE %d, \r\n",
						(board->workmode == MASTER_BOARD), (board->redundancystate == MASTER_ACTIVE));

                }
                break;
			
            default:
				//npd_syslog_dbg("Receive pdu type is %d.\n", tlv->type);
                /* npd_syslog_err("Error chasm packets.\r\n"); */
                break;
        }
    }
	//npd_syslog_dbg("free pdu .\r\n");
    free(pdu);
	//npd_syslog_dbg("END: chasm_pdu_recv .\r\n");
    return;
}



int chassis_manage_board_noncompat_init()
{
	struct sockaddr_tipc server_addr;	
	int fd;
	int ret;
	chasm_circle_data_t *data;
	unsigned long slave_recv_base = SEM_TIPC_INSTANCE_BASE_ACTIVE_TO_NON_ACTIVE;
	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);

    chasm_circle_noncompat = malloc(sizeof(chasm_circle_data_t)*SYS_CHASSIS_SLOTNUM);
    memset(chasm_circle_noncompat, 0, sizeof(chasm_circle_data_t)*SYS_CHASSIS_SLOTNUM);

	/* create non compatible socket for other department */
    data = &chasm_circle_noncompat[localmoduleinfo->slot_index];	
    fd = socket(AF_TIPC, SOCK_RDM, 0);
    if(0 > fd)
    {
        npd_syslog_cslot_err("\n%% Can not create chassis management non compat tipc socket for board %d!\r\n"
            , localmoduleinfo->slot_index+1);
        return NPD_FAIL;
    }

    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = TIPC_SEM_TYPE_LOW;
	server_addr.addr.nameseq.lower = slave_recv_base + localmoduleinfo->slot_index;
	server_addr.addr.nameseq.upper = slave_recv_base + localmoduleinfo->slot_index;
	server_addr.scope = TIPC_CLUSTER_SCOPE;

	ret = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        perror("TIPC socket bind error.");
        exit(1);
    }

	/*
    ret = listen(fd, 1);
    if(ret < 0)
    {
        perror("TIPC socket listen error.");
        exit(1);
    }
    */
	
	/* save fd to chasm_circle_noncompatble */
    data->board = localmoduleinfo;
    data->sock = fd;

	npd_syslog_dbg("register fd %d to chasm_noncmpat_pdu_recv. \r\n", fd);
	circle_register_sock(fd, EVENT_TYPE_READ, chasm_noncompat_pdu_recv, data, NULL);
	/* register to chasm_pdu_recv_non_compat */	

	return NPD_SUCCESS;
}
#endif


int chasm_clear_resource()
{
    int i;
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(chasm_circle_data[i].sock)
        {
            circle_unregister_sock(chasm_circle_data[i].sock, EVENT_TYPE_READ);
			circle_unregister_sock(chasm_circle_data[i].sock, EVENT_TYPE_WRITE);
			circle_unregister_sock(chasm_circle_data[i].sock, EVENT_TYPE_EXCEPTION);
			circle_cancel_timeout(chasm_timeout, &chasm_circle_data[i], circle_ALL_CTX);
			close(chasm_circle_data[i].sock);
        }
    }
#endif	
	return NPD_SUCCESS;
}


static void handle_term(int sig, void *circle_ctx, void *signal_ctx)
{
	npd_syslog_official_event("Signal %d received - terminating\r\n", sig);
	//circle_terminate();
}

#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
int npd_chassis_board_info_notify(unsigned int chassisId)
{
	int i = 0, ret = 0;
	int slot_index = 0;
	int type= 0, slotnum = 0, actmaster = -1, sbymaster = -1, local = -1, state = 0;

	{
		type = SYS_PRODUCT_TYPE;
		slotnum = SYS_CHASSIS_SLOTNUM;
		actmaster = SYS_MASTER_ACTIVE_SLOT_INDEX;

		for (i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
		{
			slot_index = SYS_CHASSIS_MASTER_SLOT_INDEX(i); 
			if(SYS_MODULE_ISMASTERSTANDBY(slot_index))
			{
				sbymaster = slot_index;
			}
		}
		local = SYS_LOCAL_MODULE_SLOTNO;
		state = CHASSIS_RUNNING;
		nam_chassis_info_notify(type, slotnum, actmaster, sbymaster, local, state);
	}

	for(slot_index = 0; slot_index < SYS_CHASSIS_SLOTNUM; slot_index++)
	{
		npd_board_info_notify(slot_index);			
	}

	return ret;
}

int npd_board_info_notify(unsigned int slot_index)
{
	int ret = 0;
	int board_type = 0, runstate = 0, workmode = 0, online = 0;

	npd_syslog_dbg("npd board info notify: slot %d", slot_index+1);	

	if(SYS_LOCAL_MODULE_SLOT_INDEX == slot_index)
		runstate = SYS_LOCAL_MODULE_REMOTE_RUNSTATE;
	else
		runstate = SYS_MODULE_RUNNINGSTATE(slot_index);
	board_type = MODULE_TYPE_ON_SLOT_INDEX(slot_index);
	if (board_type != 0)
	{
		online = MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(slot_index);
		workmode = SYS_MODULE_WORKMODE_INDEX(slot_index);	
	}
	if(runstate == RMT_BOARD_NOEXIST)
    {
        if(online == FALSE)
        {
            board_type = PPAL_BOARD_TYPE_NONE;
        }
    }
	ret = nam_board_info_notify(slot_index, board_type, runstate, workmode, online);		
	
	return ret;
}
#endif
int npd_chassis_manage
(
    void
)
{
    int event;
    int state;

	if (circle_init(NULL)) {
		npd_syslog_err("Failed to initialize event loop.\r\n");
		return -1;
	}
	npd_init_tell_whoami("chassMan", (unsigned char)0);
/*	
	circle_register_signal(SIGHUP, handle_reload, NULL);
	circle_register_signal(SIGUSR1, handle_dump_state, NULL);
*/	
	
	circle_register_signal_terminate(handle_term, NULL);

    chasm_circle_data = malloc(sizeof(chasm_circle_data_t)*SYS_CHASSIS_SLOTNUM);
    memset(chasm_circle_data, 0, sizeof(chasm_circle_data_t)*SYS_CHASSIS_SLOTNUM);

    npd_syslog_dbg("Finish circle init\r\n");
	chasm_write_board_type_string();
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	if (1 == app_slave_indpnt_get())
	{
		//jump to master active routine

		npd_syslog_dbg("slave indpnet get is %d.\r\n", 1);	
		chasm_close_ctrlintf(localmoduleinfo);
		localmoduleinfo->state_function = local_master_state_desc;

	    event = ACTIVE_MASTER_ENABLE;
	    localmoduleinfo->runstate = LOCAL_MASTER_INIT;
	    state = LOCAL_MASTER_INIT;
	}
	else
#endif		
	{
	    if(SYS_LOCAL_MODULE_IS_MCU)
	    {
	        npd_syslog_dbg("Local module is mcu\r\n");
	        chassis_manage_board_check_init();
	        
	        if(chassis_only_one_mcu())
	        {
	            npd_syslog_dbg("Only one MCU\r\n");
	            event = ACTIVE_MASTER_ENABLE;
	        }
	        else 
	        {
	            event = SW_INSERT;
	        }
	    }
	    else
	    {
	        event = SW_INSERT;
	    }
	}
	    
    chasm_lock();
    state = localmoduleinfo->runstate;
    (*(*(localmoduleinfo->state_function)[state].funcs)[event])
        (localmoduleinfo, localmoduleinfo, event, NULL);

#if 0	
	chassis_manage_board_noncompat_init();
#endif
	
    chasm_unlock();
	circle_run();
	chasm_clear_resource();
		
    chasm_init_done();
	return NPD_SUCCESS;

}

#ifdef __cplusplus
}
#endif

