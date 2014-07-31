#ifdef __cplusplus
extern "C"
{
#endif

#include "npd/nbm/npd_cplddef.h"

long asx9604l_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long asx9604l_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long asx9604l_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long asx9604l_slotno_get()
{
	return nbm_slotno_get();
}

long asx9604l_local_reset()
{
	return nbm_local_reset();
}

long asx9604l_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long asx9604l_led_lighting(unsigned long status)
{
    npd_syslog_err("NH ASX9604L led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long asx9604l_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH ASX9604L led code loading.\n");
	return 0;
}

long asx9604l_pne_mon_start()
{
    npd_syslog_err("NH ASX9604L p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}


void asx9604l_sal_config_init_defaults(void)
{
	int i = 0;
	npd_syslog_dbg("init portNum2phyId.\n");
	portNum2phyId[0] = 4;
	portNum2phyId[1] = 5;
	portNum2phyId[2] = 2;
	portNum2phyId[3] = 3;
	npd_syslog_dbg("NH ASX9604L asic config init doing nothing.\n");
}


long asx9604l_local_conn_init(int product_type)
{
    return 0;
}

long asx9604l_system_conn_init(
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

long asx9604l_system_conn_deinit(
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

fiber_module_fix_param_t asx9604l_sfp_param =
{
    .fiber_module_inserted = &asx9604l_sfp_online,
    .fiber_module_insert_detect = &asx9604l_sfp_detect_start,
    .fiber_module_info_get = &asx9604l_sfp_info_get,
};


ams_fix_param_t asx9604l_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 4,
    .ams_pre_init = NULL,
    .ams_driver_init =  (long (*)(int ))&nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &asx9604l_asic_led_proc,
};

board_spec_fix_param_t asx9604l_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_ASX9604L,
    .fiber_module_fix_param = &asx9604l_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &asx9604l_asic_switch,
                 },
    .slotno_get = &asx9604l_slotno_get,
    .reset = &asx9604l_local_reset,
    .get_reset_type = &asx9604l_reset_type,
    .sys_led_lighting = &asx9604l_led_lighting,
    .pne_monitor_start = &asx9604l_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &asx9604l_local_conn_init,
    .system_conn_init = &asx9604l_system_conn_init,
    .system_conn_deinit = &asx9604l_system_conn_deinit,
    .asic_config_init = &asx9604l_sal_config_init_defaults
};


#ifdef __cplusplus
}
#endif

