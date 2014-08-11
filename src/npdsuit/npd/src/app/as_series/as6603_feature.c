
#ifdef __cplusplus
extern "C"
{
#endif


// as6603 int process

void as6603_int_linecard_handler(unsigned char int_value, unsigned char add_value)
{
	//print
	npd_int_data_ctrl linecard_int_data_array[] = 
	{
		{0x01, 0x01},
		{0x02, 0x02},
		{0x04, 0x04},
		{0x08, 0x08},				
	};
	npd_int_data_ctrl* int_data_ctl = NULL ;
	int linecard_slot_index = 0;
	int index = 0;
	int state = 0;
	int event = 0;

	npd_syslog_dbg("Enter as6603_int_linecard_handler.\n");
	
	for (index = 0; index < LENGTH(linecard_int_data_array); index++)
	{
		int_data_ctl = &linecard_int_data_array[index];
		if (int_value & int_data_ctl->int_mask) //unit remove
		{
			if (index < 2)
			{
				linecard_slot_index = index;
			}
			else
			{
				linecard_slot_index = index + 2;
			}
			
			if (add_value & int_data_ctl->add_mask) // unit removed
			{
	            state = chassis_slots[linecard_slot_index]->runstate;
	            event = HW_REMOVE;
	            (*(*(chassis_slots[linecard_slot_index]->state_function)[state].funcs)[event])
	                (chassis_slots[linecard_slot_index], chassis_slots[linecard_slot_index], event, NULL);					
			}
			else		      // unit inserted
			{
	            state = chassis_slots[linecard_slot_index]->runstate;
	            event = HW_INSERT;
	            (*(*(chassis_slots[linecard_slot_index]->state_function)[state].funcs)[event])
	                (chassis_slots[linecard_slot_index], chassis_slots[linecard_slot_index], event, NULL);					
			}		
		}
	}	
	
		
}

void as6603_int_chasis_state_handler(unsigned char int_value, unsigned char add_value)
{
	if (int_value & 0x01)   //mate_card int
	{
		int master_slot = 0;
		int mate_card_index = 0;
		int i;
		int state;
		int event;
		
	    for(i = 0; i < SYS_CHASSIS_MASTER_SLOTNUM; i++)
	    {
	        master_slot = SYS_CHASSIS_MASTER_SLOT_INDEX(i);
	        if(master_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
	        {
				mate_card_index = master_slot;
				break;
			}
	    }
		
		if (add_value & 0x01) // unit removed
		{
		    state = chassis_slots[mate_card_index]->runstate;
		    event = HW_REMOVE;
		    (*(*(chassis_slots[mate_card_index]->state_function)[state].funcs)[event])
		        (chassis_slots[mate_card_index], chassis_slots[i], event, NULL);					
		}
		else		      // unit inserted
		{
		    state = chassis_slots[mate_card_index]->runstate;
		    event = HW_INSERT;
		    (*(*(chassis_slots[mate_card_index]->state_function)[state].funcs)[event])
		        (chassis_slots[mate_card_index], chassis_slots[mate_card_index], event, NULL);					
		}		
		
	}
	
	if (int_value & 0x02)	// fan int
	{
	
	}
	
	if (int_value & 0x04)	// fan alarm int
	{
	
	}
}

void as6603_int_switch_master_handler(unsigned char int_value, unsigned char add_value)
{
	if (int_value & 0x08) //master active switch
	{
		npd_syslog_warning("switch_master occured.\n", index+1);
	}
}

npd_int_ctrl as6603_npd_int_proc_arr[] =
{
	{0, 8, 	as6603_int_linecard_handler},
	{1, 9, 	as6603_int_chasis_state_handler},
	{2, 10, as6600_int_power_handler},		
	{1, 11, as6603_int_switch_master_handler}, /* this int is special.  */
};


void as6603_npd_interrupt_handler(void)
{
	int ret = 0;
	unsigned char int_reg_value = 0;
	unsigned char add_reg_value = 0;
	npd_int_ctrl *npd_int_ctrl = NULL;
	char  int_data_buf[MAX_INT_DATA_OFFSET];
	int index = 0;

	npd_syslog_dbg("as6603 npd interrupt handler.\n");
	ret = nbm_get_int_info(int_data_buf, MAX_INT_DATA_OFFSET);
	if (ret != 0)
	{
		npd_syslog_err("nbm get interrupt info error.\n");
		return;
	}
	
	for(index = 0; index < LENGTH(as6603_npd_int_proc_arr); index++)
	{
		npd_int_ctrl = &as6603_npd_int_proc_arr[index];
		if (npd_int_ctrl == NULL)
			continue;
		
		int_reg_value = int_data_buf[npd_int_ctrl->int_offset];		
		add_reg_value = int_data_buf[npd_int_ctrl->add_offset];
		
		if (npd_int_ctrl->interrupt_handler != NULL)
		{
			npd_int_ctrl->interrupt_handler(int_reg_value, add_reg_value);
		}
	}
	return ;
}


board_manage_fix_param_t as6603_boardmn_fix_param =
{
    .slotnum = 3,
    .master_slotnum = 2,
    .master_slot_id = {0, 1},
    .can_distr = TRUE,
    .can_master_service = TRUE,
    .topo = FULL_MESH,
    .slot_map = {
        {0,0}, {1,1}, {2,2}
     },
    .board_inserted = &as_board_online,
    .board_insert_detect = &as_board_detect_start,
    .master_board_inserted = &as_master_board_online,
    .board_reset = &as_board_reset,
    .board_poweroff = &as_board_poweroff
};

pne_fix_param_t as6603_pne_fix_param = 
{
    .power_num = 3,   		
    .fan_num = 1,
    .pne_monitor_start = &as6600_pne_monitor_start,
    .pne_fanspeed_adjust = &as_fan_speed_adjust,
    .power_man_param_init = &as6600_power_man_param_init,
    .fan_man_param_init = &as6600_fan_param_init,    
};

product_feature_t as6603_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 16384,
    .max_macapp = 0,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 192,
    .max_protobased_vlan = 12,
    .max_ipsubnet_vlan = 64,
    .max_stpg = 128,
    .max_trunk = 128,
    .max_port_per_trunk = 8,
    .max_fabric_trunk_extra = 0,
    .max_vlanswitch_ingress = 1024,
    .max_vlanswitch_egress = 1024,
    .max_qinq = 1024,
    .max_range_qinq = 1024, /*a range of vlan map in one q tag id*/
    .max_sec_per_range_qinq = 16, /*a range can contain section number*/
    .max_mirror_des_port = 1,
    
    .max_l2mc = 1024,
    .max_vr = 1,
    .max_ip_intf = 256,
    .max_ip_route = 2048,
    .max_ip_host_extra = 4096,  
    .max_ip_ecmp = 4096,  
    .max_ip_route_per_ecmp = 8,
    .max_ipmc_extra = 1024,
    .max_ipv6_route = 512,
    .max_ipv6_host_extra = 1024,
    .max_ipv6_ecmp = 0,
    .max_ipv6_route_per_ecmp = 0,
    .max_ipv6mc_extra = 0,
    .max_tunnel = 128,
    .max_dscp_map = 128,
    .max_ele_per_dscp_map = 32,
    .max_mpls_virtual_port = 0,
    .max_mpls_vfi = 0,
    .max_mpls_lsp = 0,
    .max_mpls_pushtagnum = 0,
    .max_mpls_handle_tagnum = 0,
    .max_vlan_acl_std = 3072,
    .max_ingress_acl_std = 3072,
    .max_egress_acl_std = 3072,
    .vlan_acl_portlist_len = 10,
    .ing_acl_portlist_len = 10,
    .egr_acl_portlist_len = 10,
    .max_qos_meter = 2048,
    .max_qos_counter = 4096,
    .bandwidth_adjust_size = 1, /*unit, kbps*/  

    .max_vr_ext = 0,
    .max_ip_intf_ext = 0,
    .max_ip_route_ext = 0,
    .max_ip_host_extra_ext = 0,
    .max_ip_ecmp_ext = 0,
    .max_ip_route_per_ecmp_ext = 0,
    .max_ipmc_extra_ext = 0,
    .max_ipv6_route_ext = 0,
    .max_ipv6_host_extra_ext = 0,
    .max_ipv6_ecmp_ext = 0,
    .max_ipv6_route_per_ecmp_ext = 0,
    .max_ipv6mc_extra_ext = 0,
    .max_tunnel_ext = 0,
    .max_mpls_vfi_ext = 0,
    .max_mpls_lsp_ext = 0,
    .max_vlan_acl_std_ext = 0,
    .max_ingress_acl_std_ext = 0,
    .max_egress_acl_std_ext = 0,
    .max_qos_meter_ext = 0,
    .max_qos_counter_ext = 0,

    .max_txqueue_system = 0,
    .max_txqueue_perport = 8,
    .max_rxqueue_system = 0,
    .max_rxqueue_perport = 1,
    .max_shaper_system = 0,
    .max_shaper_perport = 8,
        
    .ipv4_route_ele_acl_rule_size = 0,
    .ipv6_route_ele_acl_rule_size = 0,
    .mpls_lsp_ele_acl_rule_size = 0,

    .ext_tcam = FALSE,
    .mac_pending_learn = TRUE,
    .ext_mac_auto_learn = FALSE,
    .prohibit_sta_move = TRUE,
    .class_sta_move = TRUE,
    .mac_limit_system = TRUE,
    .mac_limit_per_port = TRUE,
    .mac_limit_per_lag = TRUE,
    .mac_limit_per_vlan = TRUE,
    .mac_limit_per_vpls = FALSE,
    .mac_delete_per_port = TRUE,
    .mac_delete_per_lag = TRUE,
    .mac_delete_per_vlan = TRUE,
    .mac_delete_perport_pervlan = TRUE,
    .mac_delete_perlag_pervlan = TRUE,
    .mac_learn_svl = TRUE,
    .private_vlan = TRUE,
    .flow_vlan = TRUE,
    .vlan_xlate = TRUE,
    .mac_based_app = TRUE,
    .mac_based_mpls = FALSE,
    .same_port_bridge = TRUE,
    
    .vrf = FALSE,
    .route_urpf = FALSE,

    .ipv4_tunnel = TRUE,
    .ipv6_tunnel = TRUE,
    .gre_tunnel = TRUE,
    .mpls_tunnel = TRUE,

    .mpls_vpls = FALSE,
    .mpls_h_vpls = FALSE,
    .mpls_vpn = FALSE,
    .mpls_te = FALSE,
    .mpls_frr = FALSE,
    .mpls_diffserv = TRUE,
	
	.preingress_filter = TRUE,
    .ingress_filter = TRUE,
    .egress_filter = TRUE,

    .wred = TRUE,
    .queue_wdrr = TRUE,
    .queue_wrr = TRUE,
    .queue_rr = TRUE,

    .stack = TRUE,
    .rspan = TRUE,
    .sflow = TRUE,
    .acl_as_route = FALSE,
    .sys_rsvd_end_id = 15,
    .ip_route_start_id = 16,
    .ip_route_end_id = 144,    
    .acl_start_id = 768,
    .acl_end_id = 1024,
    .macbased_vlan_start_id=0,
    .macbased_vlan_end_id=191,    
    .ipsubnet_vlan_start_id=192,
    .ipsubnet_vlan_end_id=255,   
};

product_fix_param_t as6603_fix_param =
{
    .product_code = PPAL_PRODUCT_HWCODE_AS6603,
    .product_type = PRODUCT_AS6603,
    .product_short_name = AS6603_SHORT_NAME,
    .product_name = AS6603_FULL_NAME,
    .product_pp_feature = &as6603_feature,
    .serial_no = "6600",
    .board_manage = &as6603_boardmn_fix_param,
    .pne_fix_param_t = &as6603_pne_fix_param,
    .product_man_param_get = &as_load_backinfo,
    .product_reset = &as_sys_reset,
    .product_show_chassis = &as_chassis_show,
    .slotno_get = &as_slotno_get,
    .interrupt_handler = &as6603_npd_interrupt_handler,
    .master_set = &as6600_npd_master_set,
    
};

product_fix_param_t t9003_fix_param =
{
    .product_code = PPAL_PRODUCT_HWCODE_T9003,
    .product_type = PRODUCT_T9003,
    .product_short_name = T9003_SHORT_NAME,
    .product_name = T9003_FULL_NAME,
    .product_pp_feature = &as6603_feature,
    .serial_no = "6600",
    .board_manage = &as6603_boardmn_fix_param,
    .pne_fix_param_t = &as6603_pne_fix_param,
    .product_man_param_get = &as_load_backinfo,
    .product_reset = &as_sys_reset,
    .product_show_chassis = &as_chassis_show,
    .slotno_get = &as_slotno_get,
    .interrupt_handler = &as6603_npd_interrupt_handler,
    .master_set = &as6600_npd_master_set,
    
};

product_fix_param_t ax8603_fix_param =
{
    .product_code = PPAL_PRODUCT_HWCODE_AX8603,
    .product_type = PRODUCT_AX8603,
    .product_short_name = AX8603_SHORT_NAME,
    .product_name = AX8603_FULL_NAME,
    .product_pp_feature = &as6603_feature,
    .serial_no = "8600",
    .board_manage = &as6603_boardmn_fix_param,
    .pne_fix_param_t = &as6603_pne_fix_param,
    .product_man_param_get = &as_load_backinfo,
    .product_reset = &as_sys_reset,
    .product_show_chassis = &as_chassis_show,
    .slotno_get = &as_slotno_get,
    .interrupt_handler = &as6603_npd_interrupt_handler,
    .master_set = &as6600_npd_master_set,
    
};

#ifdef __cplusplus
}
#endif

