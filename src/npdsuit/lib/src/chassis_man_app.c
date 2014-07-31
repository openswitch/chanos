
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
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		APIs used for chassis manage in app other than NPD.
*
* DATE:
*		10/27/2010	
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "lib/chassis_man_app.h"


int write_to_file(char * filename, char * buffer, int len)
{
	FILE * outfile;
	int write_mem;
	int ret = 0;
	
	outfile = fopen(filename, "w+b" );
	
	if( outfile == NULL  )
	{	
	    return -1;
	}
	write_mem = fwrite(buffer, sizeof(unsigned char), len, outfile);
	
	fclose(outfile);

	if (len != write_mem)
	{
		return -1;
	}
	chmod(filename, 0777);
	
	return ret;
}


int state_file_read(char *filename)
{
	int state_fd = -1;
	char buf[4] = {0};
	int ret = -1;
    int value = 0;

    state_fd = open(filename,O_RDONLY);
    if(state_fd < 0) 
    {
        return -1;
    }

    ret = read(state_fd,buf,2);
    if(ret > 0)
    {
        buf[ret] = 0;
        value = atoi(buf);
        close(state_fd);
        state_fd = -1;
        return value;
    }
    else
    {
        close(state_fd);
        state_fd = -1;
        return -1;
    }
	return ret;
}

int property_file_read(char *filename,char *property)
{
	int property_fd = -1;
	int ret = -1;
    property_fd = open(filename,O_RDONLY);
    if(property_fd < 0) 
    {
        return -1;
    }
    ret = read(property_fd,property,PROPERTY_MAX_LEN);
    close(property_fd);
    property_fd = -1;
    if(ret > 0)
    {       
        return 0;
    }
    else
    {
        return -1;
    }
}
int app_slot_num_get()
{
    return state_file_read(CHASSIS_MAN_SLOT_NUM_FILE);
}

int app_enterprise_name_get(char *enterprise)
{
    return property_file_read(CHASSIS_MAN_ENTERPRISE_NAME_FILE,enterprise);
}

int app_support_url_get(char *pname)
{
	int ret = 0;
    ret = property_file_read(CHASSIS_MAN_SUPPORT_URL_FILE,pname);
	if(ret != 0)
	{
		sprintf(pname, "www.open-switch.org");
		return 0;
	}
	if(strncmp(pname, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
		sprintf(pname, "www.open-switch.org");
		return 0;
	}
	return 0;
}

int app_enterprise_snmp_oid_get(char *pname)
{
	int ret = 0;
	char enterprise_oid[64];
	int name_len = 0;
	
	memset(enterprise_oid, 0, 64);
    strcpy(pname,"1.3.6.1.4.1.");
    ret = property_file_read(CHASSIS_MAN_ENTERPRISE_OID_FILE,enterprise_oid);
	if(ret != 0)
	{
		strcat(pname, "31656");
		return 0;
	}
	if(strncmp(enterprise_oid, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
		strcat(pname, "31656");
		return 0;
	}
	name_len = strlen(enterprise_oid);
	if(name_len == 0)
	{
		strcat(pname, "31656");
		return 0;
	}
	if (enterprise_oid[name_len-1] == '\n')
	{
		enterprise_oid[name_len-1] = '\0';
	}
	strcat(pname, enterprise_oid);
	return 0;
}

int app_snmp_system_oid_get(char *pname)
{
	int ret = 0;
	char enterprise_oid[64];
	int name_len = 0;
	
	memset(enterprise_oid, 0, 64);
    strcpy(pname,"1.3.6.1.4.1.");
    ret = property_file_read(CHASSIS_MAN_ENTERPRISE_OID_FILE,enterprise_oid);
	if(ret != 0)
	{
		strcat(pname, "31656");
		goto do_system_oid;
	}
	if(strncmp(enterprise_oid, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
		strcat(pname, "31656");
		goto do_system_oid;
	}
	name_len = strlen(enterprise_oid);
	if(name_len == 0)
	{
		strcat(pname, "31656");
		goto do_system_oid;
	}
	if (enterprise_oid[name_len-1] == '\n')
	{
		enterprise_oid[name_len-1] = '\0';
	}	
	strcat(pname, enterprise_oid);
do_system_oid:
	memset(enterprise_oid, 0, 64);
	ret = property_file_read(CHASSIS_MAN_SYSTEM_OID_FILE, enterprise_oid);
	if(ret != 0)
	{
		return 0;
	}
	if(strncmp(enterprise_oid, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
		return 0;
	}
	if(strlen(enterprise_oid) == 0)
	{
		return 0;
	}
	strcat(pname, ".");
	strcat(pname, enterprise_oid);
	return 0;
}

int app_product_name_get(char *pname)
{
    return property_file_read(CHASSIS_MAN_PRODUCT_NAME_FILE,pname);
}

int app_module_name_get(char *mname)
{
    return property_file_read(CHASSIS_MAN_MODULE_NAME_FILE, mname);
}

int app_product_type_get()
{
	int ret;
	ret = state_file_read(CHASSIS_MAN_PRODUCT_TYPE_FILE);
	if(-1 == ret)
		return FALSE;
	else
		return ret;
}

int app_board_type_get()
{
	int ret;
	ret = state_file_read(CHASSIS_MAN_BOARD_TYPE_FILE);
	if(-1 == ret)
		return FALSE;
	else
		return ret;
}


int app_act_master_running()
{
    return state_file_read(CHASSIS_MAN_ACTMASTER_STATE_FILE);
}

int  app_local_slot_get()
{
    return state_file_read(CHASSIS_MAN_SLOT_NO_FILE);
}

int app_actmaster_slot_get()
{
    return state_file_read(CHASSIS_MAN_ACTMASTER_SLOT_FILE);
}

int app_sbymaster_slot_get()
{
    return state_file_read(CHASSIS_MAN_SBYMASTER_SLOT_FILE);
}

int app_slot_work_mode_get()
{
    return state_file_read(CHASSIS_MAN_MASTER_STATE_FILE);
}

int app_box_state_get()
{
	int ret;
    ret = state_file_read(CHASSIS_MAN_BOX_STATE_FILE);
	if(-1 == ret)
		return FALSE;
	else
		return ret;
}

int app_npd_sync_done_state_get(int slot_index)
{
    char filename[32];
    int ret;

    sprintf(filename, NPD_DBSYNC_DONE_STATE_FILE, slot_index);
    ret = state_file_read(filename);
    if(-1 == ret)
        return 0;
    return ret;
}

int app_npd_sync_alldone_state_get()
{
    char filename[32];
    int ret;

    sprintf(filename, NPD_DBSYNC_ALLDONE_STATE_FILE);
    ret = state_file_read(filename);
    if(-1 == ret)
        return 0;
    return ret;
}

int app_slave_indpnt_get()
{
    int ret;
    ret = state_file_read(CHASSIS_MAN_SLAVE_INDPNT_FILE);
    return ret;
}

int app_slave_indpnt_runget()
{
    int ret;
    ret = state_file_read(CHASSIS_MAN_SLAVE_RUNNING_INDPNT_FILE);
    if(-1 == ret)
        ret = state_file_read(CHASSIS_MAN_SLAVE_INDPNT_FILE);
    return ret;
}

int app_zebra_state_get()
{
    return state_file_read(ZEBRA_RUNNING_STATE);
}

int app_module_inst_init()
{
    int ret = -1;
    FILE *fp = NULL;
    
    fp = fopen(SYS_MODULE_INST_PATH, "w+");
    if(fp == NULL )
    {
	    return ret;
    }
    
    ret = 0;
    fclose(fp);
    return ret;
}

int app_module_inst_set(char *mod_name, unsigned int mod_pid)
{
    int ret = -1;
    FILE *fp = NULL;
    char pidBuf[64] = {0};

	if(0 == app_module_inst_check(mod_name))
	{
		return ret;	
	}

    fp = fopen(SYS_MODULE_INST_PATH, "a");
    if(fp == NULL )
    {
		return ret;
    }
    
    snprintf(pidBuf, 63, "%5d(tid: %d) %s %s\n", mod_pid, (int)syscall(SYS_gettid), DELIMITER, mod_name);
    ret = fwrite(pidBuf, sizeof(unsigned char), strlen(pidBuf), fp);
    if(ret < 0)
    {
	     fclose(fp);
		 return ret;
    }

	ret = 0;
    fclose(fp);
    return ret;
}

int app_module_inst_get(char *mod_name, unsigned int *mod_pid)
{
	int ret = -1;
	FILE *fp = NULL;
	char pidBuf[64] = {0};
	char st_pid[5];
	char *st_name = NULL;

	if(mod_name == NULL || mod_pid == NULL)
		return ret;
	
	fp = fopen(SYS_MODULE_INST_PATH, "r");
	if(!fp)
	{
		return ret;
	}

	while(fgets(pidBuf, 63,fp))
	{
		st_name = strstr(pidBuf, mod_name);
		if(st_name)
    	{
    		ret = 0;
    		memcpy(st_pid, pidBuf, 5);
			*mod_pid = strtoul(st_pid, NULL, 10);
    		break;
    	}
	}

	fclose(fp);
	return ret;	
}

int app_module_inst_check(char *mod_name)
{
	int ret = -1;
	FILE *fp = NULL;
	char pidBuf[64] = {0};	
	char *st_name = NULL;

	if( mod_name == NULL )
	    return ret;

	fp = fopen(SYS_MODULE_INST_PATH, "r");
	if(!fp)
	{
		return ret;
	}

	while(fgets(pidBuf, 63,fp))
	{
		st_name = strstr(pidBuf, mod_name);
		if( st_name )
    	{
    		ret = 0;
			break;
		}	
	}

	fclose(fp);
	return ret;	
}


#ifdef __cplusplus
}
#endif

