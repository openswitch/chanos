#ifdef __cplusplus
extern "C"
{
#endif

long us3000_board_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long us3000_board_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long us3000_board_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long us3000_board_slotno_get()
{
    int tipc_node_temp = tipc_get_own_node();
	return (tipc_node_temp&0x0F) - 1;
}

long us3000_board_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long us3000_board_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long us3000_board_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");	
    return NPD_SUCCESS;
}

long us3000_board_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long us3000_board_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_HWCODE_US_3000;
    info->sn = "1001";
    return NPD_SUCCESS;
}

void us3000_board_sal_config_init_defaults(void)
{

}


long us3000_board_local_conn_init(int product_type)
{

    return 0;  
}

long us3000_board_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}

fiber_module_fix_param_t us3000_board_sfp_param =
{
    .fiber_module_inserted = &us3000_board_sfp_online,
    .fiber_module_insert_detect = &us3000_board_sfp_detect_start,
    .fiber_module_info_get = &us3000_board_sfp_info_get,
};
extern long nam_asic_info_get(int unit, struct ams_info_s *info);
extern long nam_asic_enable(int index);

extern unsigned long systemInitialized;
long nh_null_asic_init(int unit)
{
    npd_syslog_dbg("only for debug chasm\n");
    systemInitialized = 1;
    return NPD_SUCCESS;
}

long us3000_board_asic_led_proc(int unit)
{
  npd_syslog_dbg("AU3200 led code loading.\n");
    return NPD_SUCCESS;
}



ams_fix_param_t us3000_board_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &us3000_board_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t us3000_board_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

board_spec_fix_param_t us3000_board_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_US_3000,
    .fiber_module_fix_param = &us3000_board_sfp_param,
    .ams_param = {
                    [0] = &us3000_board_asic_switch
                 },
    .slotno_get = &us3000_board_slotno_get,
    .reset = &us3000_board_local_reset,
    .get_reset_type = &us3000_board_reset_type,
    .sys_led_lighting = &us3000_board_led_lighting,
    .pne_monitor_start = &us3000_board_pne_mon_start,
    .board_man_param_get = &us3000_board_mnparam_get,
    .local_conn_init = &us3000_board_local_conn_init,
    .system_conn_init = &us3000_board_system_conn_init,
    .asic_config_init = &us3000_board_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif


