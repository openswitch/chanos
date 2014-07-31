#ifdef __cplusplus
extern "C"
{
#endif

long as6612c_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long as6612c_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long as6612c_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long as6612c_slotno_get()
{
	return nbm_slotno_get();
}

long as6612c_local_reset()
{
    nbm_local_reset();
	system("reboot");
	return NPD_SUCCESS;
}

long as6612c_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long as6612c_led_lighting(unsigned long status)
{
    npd_syslog_err("NH fw9001 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long as6612c_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH fw9001 led code loading.\n");
	return NPD_SUCCESS;
}

long as6612c_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long as6612c_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_FW9001;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

void as6612c_sal_config_init_defaults(void)
{
}


long as6612c_local_conn_init(int product_type)
{
    return NPD_OK;
}

long as6612c_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return NPD_OK; 
}

long as6612c_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	return NPD_OK; 
}

int as6612c_board_mnparam_get(board_man_param_t *info)
{
	info->id = PPAL_BOARD_HWCODE_FW9001;
	strncpy(info->sn, "1000", strlen("1000"));

	return NPD_SUCCESS;
}

fiber_module_fix_param_t as6612c_sfp_param =
{
    .fiber_module_inserted = &as6612c_sfp_online,
    .fiber_module_insert_detect = &as6612c_sfp_detect_start,
    .fiber_module_info_get = &as6612c_sfp_info_get,
};
extern long nam_asic_info_get(int unit, struct ams_info_s *info);
extern long nam_asic_enable(int index);

ams_fix_param_t as6612c_asic_switch =
{
    .type = ASIC_FPGA_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &as6612c_asic_led_proc,
};

board_spec_fix_param_t as6612c_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_AX6612C,
    .fiber_module_fix_param = &as6612c_sfp_param,
    .ams_param = {
                    [0] = &as6612c_asic_switch,
                 },
    .slotno_get = &as6612c_slotno_get,
    .reset = &as6612c_local_reset,
    .get_reset_type = &as6612c_reset_type,
    .sys_led_lighting = &as6612c_led_lighting,
    .pne_monitor_start = &as6612c_pne_mon_start,
    .board_man_param_get = &as6612c_board_mnparam_get,
    .local_conn_init = &as6612c_local_conn_init,
    .system_conn_init = &as6612c_system_conn_init,
    .system_conn_deinit = &as6612c_system_conn_deinit,
    .asic_config_init = &as6612c_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

