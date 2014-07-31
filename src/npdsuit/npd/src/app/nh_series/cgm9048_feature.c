#ifdef __cplusplus
extern "C"
{
#endif
#include "ctc_api.h"
#include "glb_if_define.h"
long cgm9048_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long cgm9048_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long cgm9048_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long cgm9048_slotno_get()
{
	return nbm_slotno_get();
}

long cgm9048_local_reset()
{
	return nbm_local_reset();
}

long cgm9048_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long cgm9048_led_lighting(unsigned long status)
{
    npd_syslog_err("NH G96-48 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long cgm9048_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH G96-48 led code loading.\n");
	return 0;
}

long cgm9048_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long cgm9048_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_CGM9048;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}
extern void lcm_gchip_id_preconfig(int gchip_id);
void cgm9048_sal_config_init_defaults(void)
{
    lcm_gchip_id_preconfig(UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                         SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0));
}


long cgm9048_local_conn_init(int product_type)
{
	int ret = 0;
	int i = 0;
    unsigned char modNum;
    unsigned char portNum;
    unsigned short gport;

	modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                         SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
    ret = ctc_sgmac_set_trunk_en(0);
    if (ret < 0)
    {
        
    }
	
	for(i = 0; i < 4; i++)
	{
		portNum = 48 + i;
		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(modNum, portNum);
		ret = ctc_port_set_mac_en(gport, 0);
		if(ret != 0)
		{
		}
		
		ret = ctc_port_set_receive_en(gport, 0);
		if(ret != 0)
		{
			
		}
		
		ret = ctc_port_set_transmit_en(gport, 0);
		if(ret != 0)
		{
		}
		
		ret = ctc_port_set_bridge_en(gport, 0);
		if(ret != 0)
		{
		}
		
	}
    return 0;
}

long cgm9048_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int ret = 0;
	int i = 0;
	int panel_port_num = 0;
	unsigned char unit_no = 0;
	unsigned char mod_no = 0;
	unsigned char lport = 0;
	unsigned short gport = 0;
	
	if(SYS_MODULE_RUNNINGSTATE(insert_slotid) == RMT_BOARD_NOEXIST)
	{
		return 0;
	}
	panel_port_num = SYS_MODULE_PORT_NUM(insert_board_type);
	for(i = 1; i <= panel_port_num; i++)
	{
		unsigned char modport = 0;
		unit_no = PPAL_PANEL_2_PHY_UNIT(insert_board_type, i);
		lport = PPAL_PANEL_2_PHY_PORT(insert_board_type, i);
		mod_no = UNIT_2_MODULE(insert_board_type, insert_slotid, unit_no, lport);
		modport = UNIT_PORT_2_MODULE_PORT(insert_board_type, unit_no, lport);
		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(mod_no, lport);
		ret = ctc_l2_create_ucast_nh(gport, 0xf);
	}

 
    if((CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
        &&SYS_MODULE_ISMASTERACTIVE(insert_slotid)
        )
    {
        unsigned char modNum;
        unsigned char portNum;
    	modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                             SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);

		/*switch the HIGH-SPEED-SWITCH to inserted slot*/
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
#if 0 
        ret = ctc_sgmac_set_trunk_en(1);
        if (ret < 0)
        {
            
        }
    	for(i = 0; i < 4; i++)
    	{
    		portNum = 48 + i;
    		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(modNum, portNum);
    		ret = ctc_port_set_receive_en(gport, 1);
    		if(ret != 0)
    		{
    			
    		}
    		
    		ret = ctc_port_set_transmit_en(gport, 1);
    		if(ret != 0)
    		{
    		}
			
    		ret = ctc_port_set_bridge_en(gport, 1);
    		if(ret != 0)
    		{
    		}
		
    		ret = ctc_port_set_mac_en(gport, 1);
    		if(ret != 0)
    		{
    		}
    		ret = ctc_sgmac_add_trunk_member(0, portNum);
    		if(ret != 0)
    		{
    		}
    		ret = ctc_port_set_dot1q_type(gport, CTC_DOT1Q_TYPE_BOTH);
    		if(ret != 0)
    		{
    		}
            ret = ctc_set_max_frame_size(1, 13000);
            ret = ctc_port_set_max_frame(gport, 1);
    	}
#endif    
		
    }
    return 0;
}

long cgm9048_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	int ret = 0;
	int i = 0;
	int panel_port_num = 0;
	unsigned char unit_no = 0;
	unsigned char mod_no = 0;
	unsigned char lport = 0;
	unsigned short gport = 0;
	
	panel_port_num = SYS_MODULE_PORT_NUM(del_board_type);
	for(i = 1; i <= panel_port_num; i++)
	{
		unsigned char modport = 0;
		unit_no = PPAL_PANEL_2_PHY_UNIT(del_board_type, i);
		lport = PPAL_PANEL_2_PHY_PORT(del_board_type, i);
		mod_no = UNIT_2_MODULE(del_board_type, del_slotid, unit_no, lport);
		modport = UNIT_PORT_2_MODULE_PORT(del_board_type, unit_no, lport);
		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(mod_no, lport);
		ret = ctc_l2_delete_ucast_nh(gport);
	}

	if((CENTRAL_FABRIC == SYS_PRODUCT_TOPO))
	{
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
		
	}

    if((CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
        &&SYS_MODULE_ISMASTERACTIVE(del_slotid)
        )
    {
    	mod_no = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                             SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
    	for(i = 0; i < 4; i++)
    	{
    		lport = 48 + i;
    		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(mod_no, lport);
    		ret = ctc_port_set_mac_en(gport, 0);
    		if(ret != 0)
    		{
    		}
			
    		ret = ctc_port_set_receive_en(gport, 0);
    		if(ret != 0)
    		{
    			
    		}
    		
    		ret = ctc_port_set_transmit_en(gport, 0);
    		if(ret != 0)
    		{
    		}
			
    		ret = ctc_port_set_bridge_en(gport, 0);
    		if(ret != 0)
    		{
    		}
			
    		ret = ctc_sgmac_remove_trunk_member(0, lport);
    		if(ret != 0)
    		{
    		}
    		
    	}
		
        ret = ctc_sgmac_set_trunk_en(0);
        if (ret < 0)
        {
            
        }
    }
	return 0;
}

long cgm9048_system_master_switch(
    int product_type, 
    int switch_board_type, 
    int switch_slotid    
    )
{
	int ret = 0;
 
    if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
    {
		/*switch the HIGH-SPEED-SWITCH to inserted slot*/
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
    return ret;
}



fiber_module_fix_param_t cgm9048_sfp_param =
{
    .fiber_module_inserted = &cgm9048_sfp_online,
    .fiber_module_insert_detect = &cgm9048_sfp_detect_start,
    .fiber_module_info_get = &cgm9048_sfp_info_get,
};
long cgm9048_nam_asic_enable()
{
	int ret = 0;
	int i = 0;
	unsigned char unit_no = 0;
	unsigned char mod_no = 0;
	unsigned char lport = 0;
	unsigned short gport = 0;

	if((CENTRAL_FABRIC == SYS_PRODUCT_TOPO))
	{
		/*switch the HIGH-SPEED-SWITCH to master_standby slot*/
        unsigned char modNum;
        unsigned char portNum;
		/*switch the HIGH-SPEED-SWITCH to inserted slot*/
		cpld_mux_args cpld_mux_param;
		int masteractive_slotid;
        
    	modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                             SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);

		
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

        ret = ctc_sgmac_set_trunk_en(1);
        if (ret < 0)
        {
            
        }
    	for(i = 0; i < 4; i++)
    	{
    		portNum = 48 + i;
    		gport = GLB_TRANS_PORTID_TO_GPORT_COMPAT(modNum, portNum);
    		ret = ctc_port_set_receive_en(gport, 1);
    		if(ret != 0)
    		{
    			
    		}
    		
    		ret = ctc_port_set_transmit_en(gport, 1);
    		if(ret != 0)
    		{
    		}
			
    		ret = ctc_port_set_bridge_en(gport, 1);
    		if(ret != 0)
    		{
    		}
		
    		ret = ctc_port_set_mac_en(gport, 1);
    		if(ret != 0)
    		{
    		}
    		ret = ctc_sgmac_add_trunk_member(0, portNum);
    		if(ret != 0)
    		{
    		}
    		ret = ctc_port_set_dot1q_type(gport, CTC_DOT1Q_TYPE_BOTH);
    		if(ret != 0)
    		{
    		}
            ret = ctc_set_max_frame_size(1, 13000);
            ret = ctc_port_set_max_frame(gport, 1);
    	}
    }    
}

ams_fix_param_t cgm9048_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &cgm9048_asic_led_proc,
};

board_spec_fix_param_t cgm9048_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_CGM9048,

    .fiber_module_fix_param = &cgm9048_sfp_param,
    .ams_param = {
                    [ASIC_SWITCH_TYPE] = &cgm9048_asic_switch,
                 },
    .slotno_get = &cgm9048_slotno_get,
    .reset = &cgm9048_local_reset,
    .get_reset_type = &cgm9048_reset_type,
    .sys_led_lighting = &cgm9048_led_lighting,
    .pne_monitor_start = &cgm9048_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &cgm9048_local_conn_init,
    .system_conn_init = &cgm9048_system_conn_init,
    .system_conn_deinit = &cgm9048_system_conn_deinit,
    .asic_config_init = &cgm9048_sal_config_init_defaults,
    .asic_after_data_sync_conf = &cgm9048_nam_asic_enable,
    .master_switch = &cgm9048_system_master_switch
};


#ifdef __cplusplus
}
#endif

