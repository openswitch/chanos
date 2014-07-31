
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#include <dbus/dbus.h>
#include "npd_dbus_def.h"
#include <stdio.h>
#include "sysdef/returncode.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nbm/npd_cplddef.h"
#include "man_dbus.h"
#include "man_product.h"

extern DBusConnection *config_dbus_connection;
extern DBusConnection *dcli_dbus_connection;

int man_board_type_preconfig(unsigned int slot, char *board_type)
{
	int dbus_ret;
	int op_ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int slotno = slot;
	char * type = board_type;
    if(board_type == NULL)
    {
		return -1;
    }
    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,                \
                                 NPD_DBUS_BOARDMNG_OBJPATH,          \
                                 NPD_DBUS_BOARDMNG_INTERFACE,        \
                                 NPD_DBUS_BOARDMNG_CONFIG_SLOT_TYPE);
                                
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&slotno,
						 	DBUS_TYPE_STRING,&type,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply) {
        return -1;	
	}
	
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_INVALID);

    if(dbus_ret == FALSE)
    {
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;	
    }
	return op_ret;

}
int man_board_attr_get(unsigned int slot, struct board_mng_s* board_attr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
    unsigned int slotno = slot;
	unsigned int op_ret = 0;
	char *sn = NULL;
	char *modname = NULL;
	char *full_name = NULL;
	char *short_name = NULL;
	unsigned int sw_version;
	char *sw_type = NULL;
	char tmp_sw_type[16] = {0};
	char *ptr = NULL;

	

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_BOARDMNG_OBJPATH,	\
								NPD_DBUS_BOARDMNG_INTERFACE,	\
								NPD_DBUS_BOARDMNG_SHOW_SLOT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slotno,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INT32, &board_attr->run_state,
		DBUS_TYPE_UINT32, &board_attr->inserted,
		DBUS_TYPE_UINT32, &board_attr->online_removed,
		DBUS_TYPE_INT32, &board_attr->work_mode,
		DBUS_TYPE_INT32, &board_attr->redundancy_state,
		DBUS_TYPE_BYTE, &board_attr->hw_ver,
		DBUS_TYPE_STRING, &sn,
		DBUS_TYPE_STRING, &modname,
		DBUS_TYPE_INT64, &board_attr->configure_type,
		DBUS_TYPE_INT64, &board_attr->led_status,
		DBUS_TYPE_INT32, &board_attr->board_code,
		DBUS_TYPE_INT32, &board_attr->board_type,
		DBUS_TYPE_INT32, &board_attr->ams_type,
		DBUS_TYPE_STRING, &full_name,
		DBUS_TYPE_STRING, &short_name,
		DBUS_TYPE_UINT32, &sw_version,
		DBUS_TYPE_STRING, &sw_type,
		DBUS_TYPE_INVALID))
		{
			if (BOARD_RETURN_CODE_ERR_NONE == op_ret )
			{
                strncpy(board_attr->sn, sn, 24);
                strncpy(board_attr->modname, modname, 24);
                strncpy(board_attr->fullname, full_name, 64);
                strncpy(board_attr->shortname, short_name, 32);
				/* board run state >= READY */
				if (board_attr->run_state >= 5)
				{
					strncpy(tmp_sw_type, sw_type, 16);
				    if ((ptr = strchr(tmp_sw_type, '\n')) != NULL)
					{
						*ptr = '\0';
					}
					else if ((ptr = strchr(tmp_sw_type, '\r')) != NULL)
					{
						*ptr = '\0';
					}
					sprintf(board_attr->sw_ver, "%d.%d.%d.%0.4d.%s", 
						SW_MAJOR_VER(sw_version),
					    SW_MINOR_VER(sw_version),
					    SW_COMPATIBLE_VER(sw_version),
					    SW_BUILD_VER(sw_version),
					    tmp_sw_type);
				}
				else
				{
					strcpy(board_attr->sw_ver, "-");
				}
			}
	} 
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		op_ret = -1;
	}


	dbus_message_unref(reply);

	return op_ret;
}

int man_board_attr_get_next(unsigned int *slot, struct board_mng_s* board_attr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int op_ret = 0;
	int dbus_ret = 0;
	unsigned int slotno = *slot;
	if(board_attr == NULL)
	{
		return -1;
	}
    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,                \
                                 NPD_DBUS_BOARDMNG_OBJPATH,          \
                                 NPD_DBUS_BOARDMNG_INTERFACE,        \
                                 NPD_DBUS_BOARDMNG_NEXT_SLOT);
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&slotno,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply)
	{
        return -1;
	}
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT32, &slotno,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;
    }

    if (BOARD_RETURN_CODE_ERR_NONE != op_ret )
    {
        return op_ret;
    }
    *slot = slotno;
	return man_board_attr_get(slotno, board_attr);
}

int show_product_info(struct product_info *info)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int product_id;
	unsigned int sw_version;
	unsigned int hw_version;
	
	char *product_name = NULL;
	char *base_mac = NULL;
	char *serial_no = NULL;
	char *swname = NULL;
	char *boot_vername = NULL;
   
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_OBJPATH,	\
										NPD_DBUS_INTERFACE,	\
										NPD_DBUS_INTERFACE_METHOD_VER);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_UINT32,&sw_version,
		DBUS_TYPE_UINT32,&hw_version,		
		DBUS_TYPE_STRING,&product_name,
		DBUS_TYPE_STRING,&base_mac,
		DBUS_TYPE_STRING,&serial_no,
		DBUS_TYPE_STRING,&swname,
		DBUS_TYPE_STRING,&boot_vername, 
		DBUS_TYPE_INVALID)) {
		memcpy(info->name,product_name,strlen(product_name));
        memcpy(info->base_mac,base_mac,strlen(base_mac));
        memcpy(info->bootrom_version,boot_vername,strlen(boot_vername));
        info->hw_version=hw_version;
        info->sw_version=sw_version;
        info->product_id = product_id;
        memcpy(info->serial_no,serial_no,strlen(serial_no));
        memcpy(info->sw_name,swname,strlen(swname));
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);

	return 0;

}


int get_fan_atrr(unsigned int fan_no,struct fan_info *faninfo)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int op_ret = 0;
    int ret =-1;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_DEVICE_OBJPATH,	\
								NPD_DBUS_DEVICE_INTERFACE,	\
								NPD_DBUS_DEVICE_SHOW_FAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&fan_no,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &faninfo->inserted,
		DBUS_TYPE_INT32, &faninfo->state,
		DBUS_TYPE_INT32, &faninfo->speed,
		DBUS_TYPE_INVALID)) {	
		
            ret=op_ret;	
	} 
	else 
    {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
        {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;
}




int fan_get_next(int *fanno,struct fan_info *faninfo)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int i = 0;
    int dbus_ret;	
	int op_ret;
    int ret=-1;
    int fan_no=*fanno;
	query = dbus_message_new_method_call(
                             NPD_DBUS_BUSNAME,                \
                             NPD_DBUS_DEVICE_OBJPATH,          \
                             NPD_DBUS_DEVICE_INTERFACE,        \
                             NPD_DBUS_DEVICE_NEXT_FAN);
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&fan_no,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply) {
        printf("%% Can not get dbus reply\n");
        if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
        return ret;
	}
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT32, &fan_no,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
		printf("%% Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        return ret;
    }

    if (DEVICE_RETURN_CODE_ERR_NONE != op_ret )
    {
		printf("%%Get fan %d error.\n", fan_no);
        return ret;
    }
    *fanno=fan_no;
	dbus_ret = get_fan_atrr(fan_no,faninfo); 
	if(dbus_ret == -1)
	{
		printf("%%Show fan information error\n");
	}	
	 return dbus_ret;
}

int temperature_get(int slotno,struct temperature_info *tempre_info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	int slot_no = slotno;
    int ret=-1;
    
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE,		\
								NPD_DBUS_DEVICE_METHOD_SHOW_TEMPER);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_no,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INT32, &tempre_info->ready,
		DBUS_TYPE_INT32, &tempre_info->temp_state,
		DBUS_TYPE_INT32, &tempre_info->core_temp,
		DBUS_TYPE_INT32, &tempre_info->core_upper_limit,
		DBUS_TYPE_INT32, &tempre_info->core_lower_limit,
		DBUS_TYPE_INT32, &tempre_info->surface_temp,
		DBUS_TYPE_INT32, &tempre_info->surface_upper_limit,
		DBUS_TYPE_INT32, &tempre_info->surface_lower_limit,
		DBUS_TYPE_INVALID)) {
		ret=op_ret;
		if (DEVICE_RETURN_CODE_SLOT_NOT_LOCAL == op_ret)
		{
			//printf(" %%Device return code slot not local\n");
		}
		else if (BOARD_RETURN_CODE_NO_SUCH_SLOT == op_ret) 
		{
    		printf("%% Error:Illegal slot %d,No such slot.\n", slotno);
		}
		else if (DEVICE_RETURN_CODE_ERR_GENERAL== op_ret ) 
		{
			printf("%% Execute command failed.\n"); 
		}
	} 
	else 
   {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
        {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return 0;
}


int temperature_get_next(int *slotno,struct temperature_info *tempre_info)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int i = 0;
    int dbus_ret;
	int op_ret;
    int slot_no=*slotno;
	query = dbus_message_new_method_call(
                             NPD_DBUS_BUSNAME,                \
                             NPD_DBUS_BOARDMNG_OBJPATH,          \
                             NPD_DBUS_BOARDMNG_INTERFACE,        \
                             NPD_DBUS_BOARDMNG_NEXT_SLOT);
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&slot_no,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply) {
    	printf( "%% Can not get dbus reply\n");
        if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
        return -1;

	}
	dbus_ret = dbus_message_get_args(reply, &err,
    		DBUS_TYPE_UINT32, &op_ret,
    		DBUS_TYPE_UINT32, &slot_no,
    		DBUS_TYPE_INVALID);

	dbus_message_unref(reply);
	if(dbus_ret == FALSE)
	{
		printf("%% Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        return -1;
	}

    if (DEVICE_RETURN_CODE_ERR_NONE != op_ret )
    {
        return -1;
    }

    *slotno=slot_no;
	dbus_ret = temperature_get(slot_no,tempre_info);  
	if(dbus_ret == -1)
	{
		printf("%% show slot attrs error\n");
	}
	return dbus_ret;
}



int board_info_get(unsigned int slot, struct board_mng_s* board_attr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
    unsigned int slotno = slot;
	unsigned int op_ret = 0;
	char *sn = NULL;
	char *modname = NULL;
	char *full_name = NULL;
	char *short_name = NULL;

	

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_BOARDMNG_OBJPATH,	\
								NPD_DBUS_BOARDMNG_INTERFACE,	\
								NPD_DBUS_BOARDMNG_SHOW_SLOT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slotno,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INT32, &board_attr->run_state,
		DBUS_TYPE_UINT32, &board_attr->inserted,
		DBUS_TYPE_UINT32, &board_attr->online_removed,
		DBUS_TYPE_INT32, &board_attr->work_mode,
		DBUS_TYPE_INT32, &board_attr->redundancy_state,
		DBUS_TYPE_BYTE, &board_attr->hw_ver,
		DBUS_TYPE_STRING, &sn,
		DBUS_TYPE_STRING, &modname,
		DBUS_TYPE_INT64, &board_attr->configure_type,
		DBUS_TYPE_INT64, &board_attr->led_status,
		DBUS_TYPE_INT32, &board_attr->board_code,
		DBUS_TYPE_INT32, &board_attr->board_type,
		DBUS_TYPE_INT32, &board_attr->ams_type,
		DBUS_TYPE_STRING, &full_name,
		DBUS_TYPE_STRING, &short_name,
		DBUS_TYPE_INVALID))
		{
			if (BOARD_RETURN_CODE_ERR_NONE == op_ret )
			{
                strncpy(board_attr->sn, sn, 24);
                strncpy(board_attr->modname, modname, 24);
                strncpy(board_attr->fullname, full_name, 64);
                strncpy(board_attr->shortname, short_name, 32);
			}
	} 
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		op_ret = -1;
	}


	dbus_message_unref(reply);

	return op_ret;
}

int board_info_get_next(unsigned int *slot, struct board_mng_s* board_attr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int op_ret = 0;
	int dbus_ret = 0;
	unsigned int slotno = *slot;
	if(board_attr == NULL)
	{
		return -1;
	}
    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,                \
                                 NPD_DBUS_BOARDMNG_OBJPATH,          \
                                 NPD_DBUS_BOARDMNG_INTERFACE,        \
                                 NPD_DBUS_BOARDMNG_NEXT_SLOT);
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&slotno,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply)
	{
        return -1;
	}
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT32, &slotno,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return -1;
    }

    if (BOARD_RETURN_CODE_ERR_NONE != op_ret )
    {
        return op_ret;
    }
    *slot = slotno;
	return board_info_get(slotno, board_attr);
}

int get_power_supply_atrr(unsigned int psno,struct power_info *ps_info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int op_ret = 0;
    char *ps_type=NULL;
    int ps_no=psno;
    int ret=-1;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_DEVICE_OBJPATH,	\
								NPD_DBUS_DEVICE_INTERFACE,	\
								NPD_DBUS_DEVICE_SHOW_POWER_SUPPLY);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ps_no,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &ps_info->inserted,
		DBUS_TYPE_STRING, &ps_type,		
		DBUS_TYPE_INT32, &ps_info->ps_state,
		DBUS_TYPE_INVALID)) {
       memcpy(ps_info->ps_type,ps_type,strlen(ps_type));
			ret=op_ret;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;
}


int power_get_next(int *power_no, struct power_info *ps_info)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int i = 0;
    int dbus_ret;
	unsigned int ps_no = *power_no;	
	int op_ret;
    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,                \
                                 NPD_DBUS_DEVICE_OBJPATH,          \
                                 NPD_DBUS_DEVICE_INTERFACE,        \
                                 NPD_DBUS_DEVICE_NEXT_PSUNIT);
	dbus_error_init(&err);

   	dbus_message_append_args(query,
						 	DBUS_TYPE_UINT32,&ps_no,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply) {
        printf("%% Can not get dbus reply\n");
        return -1;
	}
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT32, &ps_no,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
		printf("% Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
    }

    if (DEVICE_RETURN_CODE_ERR_NONE != op_ret )
    {
		printf("get ps unit %d error.\n", ps_no);
        return -1;
    }
    
    *power_no = ps_no;
	dbus_ret = get_power_supply_atrr(ps_no,ps_info); 
	if(dbus_ret == -1)
	{
		printf("show power supply information error\n");
	}
    return dbus_ret;
	   
    
}

