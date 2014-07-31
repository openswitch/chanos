#ifdef __cplusplus
extern "C"
{
#endif

#include "npd/nbm/npd_cplddef.h"

long asx6602_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long asx6602_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long asx6602_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long asx6602_slotno_get()
{
	return nbm_slotno_get();
}

long asx6602_local_reset()
{
	return nbm_local_reset();
}

long asx6602_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long asx6602_led_lighting(unsigned long status)
{
    npd_syslog_err("NH ASX6602 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long asx6602_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH ASX6602 led code loading.\n");
	return 0;
}

long asx6602_pne_mon_start()
{
    npd_syslog_err("NH ASX6602 p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long asx6602_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_ASX6602;
	strncpy(info->sn, "1000", strlen("1000"));

	return as6600_board_mnparam_get(info);
}


void asx6602_sal_config_init_defaults(void)
{
	int i = 0;
	npd_syslog_dbg("init portNum2phyId.\n");
	portNum2phyId[0] = 2;
	portNum2phyId[1] = 3;
	npd_syslog_dbg("NH ASX6602 asic config init doing nothing.\n");
}


long asx6602_local_conn_init(int product_type)
{
    return 0;
}

long asx6602_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int ret = 0;
	
	/*switch the HIGH-SPEED-SWITCH to inserted slot*/
	if (SYS_MODULE_ISMASTERACTIVE(insert_slotid))
	{
		cpld_mux_args cpld_mux_param;
		int masteractive_slotid;
		
		masteractive_slotid = nbm_master_slotno_get();
		if (masteractive_slotid < 0)
		{
			npd_syslog_err("MASTERACTIVE slotno get error.\n");
			return -1;
		}

		cpld_mux_param.master_slot = masteractive_slotid;

		ret = nbm_xaui_switch(&cpld_mux_param);
		if (ret < 0)
		{
			npd_syslog_err("XAUI channel switch failed.\n");
			return -1;
		}
	}
	
    return 0; 
}

long asx6602_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	int ret = 0;
	
	/*switch the HIGH-SPEED-SWITCH to master_standby slot*/
	int masteractive_slotid;
	cpld_mux_args cpld_mux_param;

	masteractive_slotid = nbm_master_slotno_get();
	if (masteractive_slotid < 0)
	{
		npd_syslog_err("MASTERACTIVE slotno get error.\n");
		return -1;
	}

	cpld_mux_param.master_slot = masteractive_slotid;

	ret = nbm_xaui_switch(&cpld_mux_param);
	if (ret < 0)
	{
		npd_syslog_err("XAUI channel switch failed.\n");
		return -1;
	}
	
    return 0;
}

fiber_module_fix_param_t asx6602_sfp_param =
{
    .fiber_module_inserted = &asx6602_sfp_online,
    .fiber_module_insert_detect = &asx6602_sfp_detect_start,
    .fiber_module_info_get = &asx6602_sfp_info_get,
};

ams_fix_param_t asx6602_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 2,
    .ams_pre_init = NULL,
    .ams_driver_init =  (long (*)(int ))&nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &asx6602_asic_led_proc,
};

board_spec_fix_param_t asx6602_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_ASX6602,
    .fiber_module_fix_param = &asx6602_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &asx6602_asic_switch,
                 },
    .slotno_get = &asx6602_slotno_get,
    .reset = &asx6602_local_reset,
    .get_reset_type = &asx6602_reset_type,
    .sys_led_lighting = &asx6602_led_lighting,
    .pne_monitor_start = &asx6602_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &asx6602_local_conn_init,
    .system_conn_init = &asx6602_system_conn_init,
    .system_conn_deinit = &asx6602_system_conn_deinit,
    .asic_config_init = &asx6602_sal_config_init_defaults
};


#ifdef __cplusplus
}
#endif

