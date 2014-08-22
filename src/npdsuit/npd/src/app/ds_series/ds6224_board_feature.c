#ifdef __cplusplus
extern "C"
{
#endif

long ds6224_board_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long ds6224_board_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long ds6224_board_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long ds6224_board_slotno_get()
{
	return 0;
    int tipc_node_temp = tipc_get_own_node();
	return (tipc_node_temp&0x0F) - 1;
}

long ds6224_board_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long ds6224_board_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long ds6224_board_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");	
    return NPD_SUCCESS;
}

long ds6224_board_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long ds6224_board_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_DS6224;
    info->sn = "1001";
    return NPD_SUCCESS;
}

extern unsigned short phy_addr_map[128][256];
extern unsigned short phy_info_map[128][256];
long ds6224_board_sal_config_init_defaults(void)
{
	memset(phy_addr_map, 0xff, sizeof(phy_addr_map));
	memset(phy_info_map, 0x0, sizeof(phy_info_map));

	appDemoDbEntryAdd("boardIdx", 27);/*LION2 4 cores board*/
	appDemoDbEntryAdd("boardRevId", 12);
    return 0;
}

extern long ds_series_linecard_local_conn_init(int product_type);
long ds6224_board_local_conn_init(int product_type)
{
    ds_series_linecard_local_conn_init(product_type);
    return 0;  
}

long ds6224_board_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}

fiber_module_fix_param_t ds6224_board_sfp_param =
{
    .fiber_module_inserted = &ds6224_board_sfp_online,
    .fiber_module_insert_detect = &ds6224_board_sfp_detect_start,
    .fiber_module_info_get = &ds6224_board_sfp_info_get,
};

long ds6224_board_asic_led_proc(int unit)
{
#if 0
	CPSS_LED_CONF_STC				ledStreamConf;		/* led stream interface configuration structure */	
	GT_STATUS						rc; 			/* return code */	
	GT_U32							ledInterfaceNum=0;	  /* led stream interface number */ 
	GT_U32	  portGroupId = 0x3;		 /*Led stream configuration */	  

	printf("Enter %s %s %d ",__FILE__, __FUNCTION__,__LINE__);
	ledStreamConf.ledOrganize = CPSS_LED_ORDER_MODE_BY_CLASS_E;    
	ledStreamConf.disableOnLinkDown = GT_TRUE;	  
	ledStreamConf.blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;	  
	ledStreamConf.blink0Duration  = CPSS_LED_BLINK_DURATION_6_E;	
	ledStreamConf.blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;	  
	ledStreamConf.blink1Duration  = CPSS_LED_BLINK_DURATION_5_E;	
	ledStreamConf.pulseStretch	= CPSS_LED_PULSE_STRETCH_5_E;	 
	ledStreamConf.ledStart	= 0x53;    
	ledStreamConf.ledEnd	= 0x6b;    
	ledStreamConf.clkInvert = GT_TRUE;	  
	ledStreamConf.class5select	= CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;	
	ledStreamConf.class13select = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
	
	rc = cpssDxChLedStreamPortGroupConfigSet(0, portGroupId, ledInterfaceNum, &ledStreamConf);		
	printf("Leave  %s %s %d rc=%d ",__FILE__, __FUNCTION__, __LINE__, rc);
	if(rc != GT_OK)    
	{		  
		return rc;	  
	}		
#endif
	return NPD_SUCCESS; 
}




ams_fix_param_t ds6224_board_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &ds6224_board_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t ds6224_board_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

board_spec_fix_param_t ds6224_board_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_DS6224,
    .fiber_module_fix_param = &ds6224_board_sfp_param,
    .ams_param = {
                    [0] = &ds6224_board_asic_switch
                 },
    .slotno_get = &ds6224_board_slotno_get,
    .reset = &ds6224_board_local_reset,
    .get_reset_type = &ds6224_board_reset_type,
    .sys_led_lighting = &ds6224_board_led_lighting,
    .pne_monitor_start = &ds6224_board_pne_mon_start,
    .board_man_param_get = &ds6224_board_mnparam_get,
    .local_conn_init = &ds6224_board_local_conn_init,
    .system_conn_init = &ds6224_board_system_conn_init,
    .asic_config_init = &ds6224_board_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif


