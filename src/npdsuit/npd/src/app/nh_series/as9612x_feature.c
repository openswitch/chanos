#ifdef __cplusplus
extern "C"
{
#endif
long as9612x_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long as9612x_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long as9612x_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long as9612x_slotno_get()
{
	return nbm_slotno_get();
}

long as9612x_local_reset()
{
	return nbm_local_reset();
}

long as9612x_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long as9612x_led_lighting(unsigned long status)
{
    npd_syslog_err("NH AS9612X led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long as9612x_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH AS9612X led code loading.\n");
	return 0;
}

long as9612x_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long as9612x_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_AS9612X;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

extern unsigned short phy_addr_map[128][64];
extern unsigned short phy_info_map[128][64];

void as9612x_sal_config_init_defaults(void)
{
	unsigned char devNum = 0, portNum = 0; 
	memset(phy_addr_map, 0xff, sizeof(phy_addr_map));
	memset(phy_info_map, 0x0, sizeof(phy_info_map));

	for(devNum = 0; devNum < 1; devNum++)
	{
		for(portNum = 0; portNum < 64; portNum++)
		{
			if(NPD_PORT_PHY_ADDR(devNum, portNum) != -1)
			{
				phy_addr_map[devNum][portNum] = NPD_PORT_PHY_ADDR(devNum, portNum);
			}
			else
			{
				phy_addr_map[devNum][portNum] = 0xff;				
			}
		}
	}
}


long as9612x_local_conn_init(int product_type)
{
	return nh_lion_linecard_local_conn_init(product_type);
}

long as9612x_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	return nh_lion_linecard_system_conn_init(product_type, insert_board_type, insert_slotid);
}

long as9612x_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	return nh_lion_linecard_system_conn_deinit(product_type, del_board_type, del_slotid);
}

fiber_module_fix_param_t as9612x_sfp_param =
{
    .fiber_module_inserted = &as9612x_sfp_online,
    .fiber_module_insert_detect = &as9612x_sfp_detect_start,
    .fiber_module_info_get = &as9612x_sfp_info_get,
};


ams_fix_param_t as9612x_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &as9612x_asic_led_proc,
};

board_spec_fix_param_t as9612x_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_AS9612X,

    .fiber_module_fix_param = &as9612x_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &as9612x_asic_switch,
                 },
    .slotno_get = &as9612x_slotno_get,
    .reset = &as9612x_local_reset,
    .get_reset_type = &as9612x_reset_type,
    .sys_led_lighting = &as9612x_led_lighting,
    .pne_monitor_start = &as9612x_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &as9612x_local_conn_init,
    .system_conn_init = &as9612x_system_conn_init,
    .system_conn_deinit = &as9612x_system_conn_deinit,
    .asic_config_init = &as9612x_sal_config_init_defaults
};


#ifdef __cplusplus
}
#endif

