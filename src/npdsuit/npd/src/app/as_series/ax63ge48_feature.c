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

board_feature_t ax63ge48_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 16384,
    .max_macapp = 0,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 1024,
    .max_protobased_vlan = 12,
    .max_ipsubnet_vlan = 1024,
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
    .max_ip_ecmp = 0,
    .max_ip_route_per_ecmp = 0,
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
    .max_vlan_acl_std = 1024,
    .max_ingress_acl_std = 1024,
    .max_egress_acl_std = 0,
    .vlan_acl_portlist_len = 10,
    .ing_acl_portlist_len = 10,
    .egr_acl_portlist_len = 10,
    .max_qos_meter = 256,
    .max_qos_counter = 2048,
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

    .ipv4_tunnel = FALSE,
    .ipv6_tunnel = FALSE,
    .gre_tunnel = FALSE,
    .mpls_tunnel = FALSE,

    .mpls_vpls = FALSE,
    .mpls_h_vpls = FALSE,
    .mpls_vpn = FALSE,
    .mpls_te = FALSE,
    .mpls_frr = FALSE,
    .mpls_diffserv = FALSE,

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
    .acl_as_route = TRUE,
    .sys_rsvd_end_id = 15,
    .ip_route_start_id = 16,
    .ip_route_end_id = 767,
    .acl_start_id = 768,
    .acl_end_id = 1024,
	
	.ctrl_num = 1,
	.ctrl_switch = TRUE 
};

board_fix_param_t ax63ge48_param =
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

