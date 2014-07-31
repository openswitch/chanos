#ifdef __cplusplus
extern "C"
{
#endif

long dummy_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long dummy_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long dummy_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long dummy_slotno_get()
{
	return NPD_SUCCESS;
}

long dummy_local_reset()
{
    return NPD_SUCCESS;
}

long dummy_reset_type()
{
    return NPD_SUCCESS;
}

long dummy_led_lighting(unsigned long status)
{
    return NPD_SUCCESS;
}

long dummy_pne_mon_start()
{
    return NPD_SUCCESS;
}

long dummy_mnparam_get(board_man_param_t *info)
{
	return NPD_SUCCESS;
}

long dummy_local_conn_init(int product_type)
{
    return NPD_SUCCESS;
}

long dummy_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return NPD_SUCCESS; 
}

long dummy_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	return NPD_SUCCESS; 
}

int dummy_board_mnparam_get(board_man_param_t *info)
{
	info->id = PPAL_BOARD_HWCODE_DUMMY;
	strncpy(info->sn, "1000", strlen("1000"));
	
	return t9000_board_mnparam_get(info);
}

fiber_module_fix_param_t dummy_sfp_param =
{
    .fiber_module_inserted = &dummy_sfp_online,
    .fiber_module_insert_detect = &dummy_sfp_detect_start,
    .fiber_module_info_get = &dummy_sfp_info_get,
};

ams_fix_param_t dummy_asic_switch =
{
    .type = 0,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
    .ams_led_proc = NULL,
};

board_spec_fix_param_t dummy_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_DUMMY,
    .fiber_module_fix_param = &dummy_sfp_param,
    .ams_param = {
                    [0] = &dummy_asic_switch,
                 },
    .slotno_get = &dummy_slotno_get,
    .reset = &dummy_local_reset,
    .get_reset_type = &dummy_reset_type,
    .sys_led_lighting = &dummy_led_lighting,
    .pne_monitor_start = &dummy_pne_mon_start,
    .board_man_param_get = &dummy_board_mnparam_get,
    .local_conn_init = &dummy_local_conn_init,
    .system_conn_init = &dummy_system_conn_init,
    .system_conn_deinit = &dummy_system_conn_deinit,
    .asic_config_init = NULL
};


#ifdef __cplusplus
extern "C"
}
#endif


