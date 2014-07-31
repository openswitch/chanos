#ifdef __cplusplus
extern "C"
{
#endif

long ax63ge48_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long ax63ge48_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long ax63ge48_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long ax63ge48_slotno_get()
{
	return nbm_slotno_get();
}

long ax63ge48_local_reset()
{
	return nbm_local_reset();
}

long ax63ge48_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge48_led_lighting(unsigned long status)
{
    npd_syslog_err("NH ASG6648 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge48_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH ASG6648 led code loading.\n");
	return 0;
}

long ax63ge48_pne_mon_start()
{
    npd_syslog_err("NH ASG6648 p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long ax63ge48_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_AX63GE48;
	strncpy(info->sn, "1000", strlen("1000"));

	return as6600_board_mnparam_get(info);
}
extern unsigned short phy_addr_map[128][64];
extern unsigned short phy_info_map[128][64];

void ax63ge48_sal_config_init_defaults(void)
{
	unsigned char devNum = 0, portNum = 0; 
	memset(phy_addr_map, 0xff, sizeof(phy_addr_map));
	memset(phy_info_map, 0x0, sizeof(phy_info_map));
	
	for(devNum = 0; devNum < 2; devNum++)
	{
		for(portNum = 0; portNum < 28; portNum++)
		{
			printf("NPD: PORT %d:%d: PHY address: %d.\r\n", devNum, portNum, NPD_PORT_PHY_ADDR(devNum, portNum));
			if(NPD_PORT_PHY_ADDR(devNum, portNum) != -1)
			{
				phy_addr_map[devNum][portNum] = NPD_PORT_PHY_ADDR(devNum, portNum);
			}
		}
	}	
}


long ax63ge48_local_conn_init(int product_type)
{
    return as_series_linecard_local_conn_init(product_type);
}

long ax63ge48_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return as_series_linecard_system_conn_init(product_type, 
        insert_board_type, insert_slotid);
    return 0; 
}

long ax63ge48_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    return as_series_linecard_system_conn_deinit(product_type, 
        del_board_type, del_slotid);
}

fiber_module_fix_param_t ax63ge48_sfp_param =
{
    .fiber_module_inserted = &ax63ge48_sfp_online,
    .fiber_module_insert_detect = &ax63ge48_sfp_detect_start,
    .fiber_module_info_get = &ax63ge48_sfp_info_get,
};

ams_fix_param_t ax63ge48_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 2,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &ax63ge48_asic_led_proc,
};

board_spec_fix_param_t ax63ge48_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_AX63GE48,
    .fiber_module_fix_param = &ax63ge48_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &ax63ge48_asic_switch,
                 },
    .slotno_get = &ax63ge48_slotno_get,
    .reset = &ax63ge48_local_reset,
    .get_reset_type = &ax63ge48_reset_type,
    .sys_led_lighting = &ax63ge48_led_lighting,
    .pne_monitor_start = &ax63ge48_pne_mon_start,
    .board_man_param_get = &as6600_board_mnparam_get,
    .local_conn_init = &ax63ge48_local_conn_init,
    .system_conn_init = &ax63ge48_system_conn_init,
    .system_conn_deinit = &ax63ge48_system_conn_deinit,
    .asic_config_init = &ax63ge48_sal_config_init_defaults
};


#ifdef __cplusplus
}
#endif

