#ifdef __cplusplus
extern "C"
{
#endif

long ax63ge24i_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long ax63ge24i_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long ax63ge24i_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long ax63ge24i_slotno_get()
{
	return nbm_slotno_get();
}

long ax63ge24i_local_reset()
{
	return nbm_local_reset();
}

long ax63ge24i_reset_type()
{
    npd_syslog_err("NH ASG6624CE reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge24i_led_lighting(unsigned long status)
{
    npd_syslog_err("NH AX63GE24I led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge24i_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH ASG6624CE led code loading.\n");
	return 0;
}

long ax63ge24i_pne_mon_start()
{
    npd_syslog_err("NH ASG6624CE p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge24i_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_AX63GE24I;
	strncpy(info->sn, "1000", strlen("1000"));

	return as6600_board_mnparam_get(info);
}

void ax63ge24i_sal_config_init_defaults(void)
{
}


fiber_module_fix_param_t ax63ge24i_sfp_param =
{
    .fiber_module_inserted = &ax63ge24i_sfp_online,
    .fiber_module_insert_detect = &ax63ge24i_sfp_detect_start,
    .fiber_module_info_get = &ax63ge24i_sfp_info_get,
};

ams_fix_param_t ax63ge24i_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &ax63ge24i_asic_led_proc,
};

board_fix_param_t ax63ge24i_param =
{
    .board_type = PPAL_BOARD_TYPE_AX63GE24I,
    .fiber_module_fix_param = &ax63ge24i_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &ax63ge24i_asic_switch,
                 },
    .slotno_get = &ax63ge24i_slotno_get,
    .reset = &ax63ge24i_local_reset,
    .get_reset_type = &ax63ge24i_reset_type,
    .sys_led_lighting = &ax63ge24i_led_lighting,
    .pne_monitor_start = &ax63ge24i_pne_mon_start,
    .board_man_param_get = &as6600_board_mnparam_get,
    .local_conn_init = &ax63ge24i_local_conn_init,
    .system_conn_init = &ax63ge24i_system_conn_init,
    .system_conn_deinit = &ax63ge24i_system_conn_deinit,
    .asic_config_init = &ax63ge24i_sal_config_init_defaults
};


#ifdef __cplusplus
}
#endif

