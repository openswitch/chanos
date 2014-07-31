
#ifdef __cplusplus
extern "C"
{
#endif

extern uint tipc_get_own_node();
extern unsigned char *base_mac;
long as2k_3200_board_online(unsigned long slot_index)
{
    return TRUE;
}

long as2k_3200_load_backinfo(product_man_param_t *param)
{
    npd_syslog_dbg("Get product sys info\n --%s", __FUNCTION__);
	
    param->basemac = (char*)base_mac;
    param->sn = "1001";
    param->name = "AS3224";
    param->sw_name = "AuteWare OS";
    param->enterprise_name = "Autelan";
    param->enterprise_snmp_oid = "1001";
	
/*    param->built_in_admin_name = "netgear";
    param->built_in_admin_passwd = "";
    
*/
    nbm_read_backplane_sysinfo(param);
    return NPD_SUCCESS;
}

long as2k_3200_slotno_get()
{
	int tipc_node_temp = tipc_get_own_node();
	printf("(%s)Local board slot no.: %d\r\n", __func__, ((tipc_node_temp&0x0F) - 1));
    return (tipc_node_temp&0x0F) - 1;
}

void as2k_3200_npd_master_set()
{
	return;
}

board_manage_fix_param_t as2k_3200_boardmn_fix_param =
{
    .slotnum = 1,
    .master_slotnum = 1,
    .master_slot_id = {0},
    .can_distr = TRUE,
    .can_master_service = TRUE,
    .topo = FULL_MESH,
    .slot_map = {
        {0,0}
    },
    .board_inserted = &as2k_3200_board_online,
    .board_insert_detect = &as2k_board_detect_start,
    .master_board_inserted = &as2k_master_board_online,
    .board_reset = &as2k_board_reset,
    .board_poweroff = &as2k_board_poweroff
};

pne_fix_param_t as2k_3200_pne_fix_param = 
{
    .power_num = 2,
    .fan_num = 1,
    .pne_monitor_start = NULL,
    .pne_fanspeed_adjust = NULL
};


product_feature_t as2k_3200_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 1024,
    .max_macapp = 512,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 64,
    .max_protobased_vlan = 16,
    .max_ipsubnet_vlan = 16,
    .max_stpg = 128,
    .max_trunk = 8,
    .max_port_per_trunk = 8,
    .max_fabric_trunk_extra = 0,
    .max_vlanswitch_ingress = 256,
    .max_vlanswitch_egress = 256,
    .max_qinq = 256,
    .max_range_qinq = 128, /*a range of vlan map in one q tag id*/
    .max_sec_per_range_qinq = 16, /*a range can contain section number*/
    .max_mirror_des_port = 1,
    
    .max_l2mc = 1024,
    .max_vr = 1,
    .max_ip_intf = 4,
    .max_ip_route = 14,
    .max_ip_host_extra = 14,
    .max_ip_ecmp = 0,
    .max_ip_route_per_ecmp = 1,
    .max_ipmc_extra = 0,
    .max_ipv6_route = 0,
    .max_ipv6_host_extra = 0,
    .max_ipv6_ecmp = 0,
    .max_ipv6_route_per_ecmp = 0,
    .max_ipv6mc_extra = 0,
    .max_tunnel = 0,
    .max_dscp_map = 32,
    .max_ele_per_dscp_map = 32,
    .max_mpls_virtual_port = 0,
    .max_mpls_vfi = 0,
    .max_mpls_lsp = 0,
    .max_mpls_pushtagnum = 0,
    .max_mpls_handle_tagnum = 0,
    .max_vlan_acl_std = 0,
    .max_ingress_acl_std = 512,
    .max_egress_acl_std = 0,
    .max_qos_meter = 0,
    .max_qos_counter = 0,
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
    .route_urpf = TRUE,

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

    .wred = TRUE,
    .queue_wdrr = TRUE,
    .queue_wrr = TRUE,
    .queue_rr = TRUE,

    .stack = TRUE,
    .rspan = TRUE,
    .sflow = TRUE,
    .support_acl_based_vlan = TRUE
    
};
product_fix_param_t as2k_3200_fix_param =
{
    .product_code = PPAL_PRODUCT_HWCODE_NH3052,
    .product_type = PRODUCT_NH_3052,
    .product_short_name = "AS3200",
    .product_name = "",
    .product_pp_feature = &as2k_3200_feature,
    .serial_no = "1001",
    .board_manage = &as2k_3200_boardmn_fix_param,
    .pne_fix_param_t = &as2k_3200_pne_fix_param,
    .product_man_param_get = &as2k_3200_load_backinfo,
    .product_reset = &as2k_sys_reset,
    .product_show_chassis = &as2k_chassis_show,
    .slotno_get = &as2k_3200_slotno_get,
    .master_set = &as2k_3200_npd_master_set
};



#ifdef __cplusplus
}
#endif

