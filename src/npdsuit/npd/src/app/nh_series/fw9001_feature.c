#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>


#define MODULE_NAME_FILE 	"/proc/sysinfo/module_name"
#define MODULE_SN_FILE 		"/proc/sysinfo/module_sn"
#define OEM_MODULE_NAME_FILE 	"/devinfo/module_name"


long fw9001_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long fw9001_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long fw9001_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long fw9001_slotno_get()
{
	return nbm_slotno_get();
}

long fw9001_local_reset()
{
	char * path = "/data/npd_reboot_off";
	int ret = 0;
	npd_syslog_dbg("Enter fw9001_local_reset.\n");
	if ((ret=access(path, F_OK)) == 0)
	{
		/* if exist , not reboot myself */
		npd_syslog_dbg("npd_reboot off exist, so npd exit not reboot.\n");
		exit(0);
	}
	else
	{
		npd_syslog_dbg("npd_reboot off exist, so npd exit not reboot.\n");	
	    nbm_local_reset();
		system("reboot");
	}
	return NPD_SUCCESS;
}

long fw9001_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long fw9001_led_lighting(unsigned long status)
{
    npd_syslog_err("NH fw9001 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long fw9001_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH fw9001 led code loading.\n");
	return NPD_SUCCESS;
}

long fw9001_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long fw9001_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_FW9001;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

void fw9001_sal_config_init_defaults(void)
{
}


long fw9001_local_conn_init(int product_type)
{
    return NPD_OK;
}

long fw9001_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return NPD_OK; 
}

long fw9001_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	return NPD_OK; 
}


int fw9001_read_sysinfo_file(char * filename, char * buf, int len)
{
	int fd;
	int size = 0;

	fd = open(filename,O_RDONLY);
	if (fd <= 0)
	{
		npd_syslog_err("open file error.\n", filename);
		return NPD_FAIL;
	}
	size = read(fd,buf,len);
	if (size < 0)
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	if (NULL != strstr(buf, "error"))
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	if(strncmp(buf, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	close(fd);
	return NPD_SUCCESS;	
}



int fw9001_board_mnparam_get(board_man_param_t *info)
{
	char mod_name[32];
	char mod_sn [32];
	char * ptr;
	int ret = 0;
	int hw_code;
	
	info->id = PPAL_BOARD_HWCODE_FW9001;
	strncpy(info->sn, "1000", strlen("1000"));

	ret = fw9001_read_sysinfo_file(OEM_MODULE_NAME_FILE,mod_name, 32);
	if (ret != NPD_SUCCESS)
	{
		npd_syslog_err("get oem module name error.\n");
		ret = fw9001_read_sysinfo_file(MODULE_NAME_FILE,mod_name, 32);
		if (ret != NPD_SUCCESS)
		{
			npd_syslog_err("get module name error.\n");
			return ret;
		}		
		//return ret;
	}	
	ptr = (char *)memchr(mod_name, '\n', 32);
	if(ptr)
	{
	    *ptr = '\0';
	}
	strncpy(info->modname, mod_name, strlen(mod_name));
	info->modname[strlen(mod_name)] = '\0';
	npd_syslog_dbg("modname is %s.\n",info->modname);
	
	return NPD_SUCCESS;
}

fiber_module_fix_param_t fw9001_sfp_param =
{
    .fiber_module_inserted = &fw9001_sfp_online,
    .fiber_module_insert_detect = &fw9001_sfp_detect_start,
    .fiber_module_info_get = &fw9001_sfp_info_get,
};

ams_fix_param_t fw9001_asic_switch =
{
    .type = ASIC_FPGA_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &fw9001_asic_led_proc,
};

board_spec_fix_param_t fw9001_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_FW9001,
    .fiber_module_fix_param = &fw9001_sfp_param,
    .ams_param = {
                    [0] = &fw9001_asic_switch,
                 },
    .slotno_get = &fw9001_slotno_get,
    .reset = &fw9001_local_reset,
    .get_reset_type = &fw9001_reset_type,
    .sys_led_lighting = &fw9001_led_lighting,
    .pne_monitor_start = &fw9001_pne_mon_start,
    .board_man_param_get = &fw9001_board_mnparam_get,
    .local_conn_init = &fw9001_local_conn_init,
    .system_conn_init = &fw9001_system_conn_init,
    .system_conn_deinit = &fw9001_system_conn_deinit,
    .asic_config_init = &fw9001_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

