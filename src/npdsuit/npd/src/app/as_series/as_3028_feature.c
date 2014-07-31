
#ifdef __cplusplus
extern "C"
{
#endif

extern unsigned char *base_mac;

long as_3028_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long as_3028_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long as_3028_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long as_3028_slotno_get()
{
	int tipc_node_temp = tipc_get_own_node();
	printf("(%s)Local board slot no.: %d\r\n", __func__, ((tipc_node_temp&0x0F) - 1));
    return (tipc_node_temp&0x0F) - 1;
}

long as_3028_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long as_3028_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long as_3028_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long as_3028_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long as_3028_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_HWCODE_NH_3052;
    info->sn = "1001";
    return NPD_SUCCESS;
}

long as_3028_npd_os_upgrade(unsigned int slot_index)
{
	return as_os_upgrade(slot_index);
}

extern unsigned short phy_addr_map[128][64];
void as_3028_sal_config_init_defaults(void)
{
	unsigned char devNum = 0, portNum = 0; 
	memset(phy_addr_map, 0xff, sizeof(phy_addr_map));
	for(devNum = 0; devNum < 1; devNum++)
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

fiber_module_fix_param_t as_3028_sfp_param =
{
    .fiber_module_inserted = &as_3028_sfp_online,
    .fiber_module_insert_detect = &as_3028_sfp_detect_start,
    .fiber_module_info_get = &as_3028_sfp_info_get,
};

extern unsigned long systemInitialized;

ams_fix_param_t as_3028_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_enable = NULL,
};
ams_fix_param_t as_3028_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

ipp_fix_param_t  as_3028_ipp_param =
{
    .ipp_portnum = 1,
    .ipp_phyport_map = {"eth0-1"},
    .ipp_board_map = {-2}
};

sub_board_fix_param_t as_3028_subboard_param = 
{
    .sub_slotnum = 1,
    .sub_slot_portnum = 0
};

temper_fix_param_t as_3028_temper_param =
{
    .num = 2,
    .name = {"OCTEON TEMP", "BCM 56820 TEMP"}
};

board_feature_t as_3028_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 32000,
    .max_macapp = 512,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 4096,
    .max_protobased_vlan = 4096,
    .max_ipsubnet_vlan = 4096,
    .max_stpg = 128,
    .max_trunk = 128,
    .max_port_per_trunk = 32,
    .max_fabric_trunk_extra = 8,
    .max_vlanswitch_ingress = 8192,
    .max_vlanswitch_egress = 8192,
    .max_qinq = 8192,
    .max_range_qinq = 2048, /*a range of vlan map in one q tag id*/
    .max_sec_per_range_qinq = 16, /*a range can contain section number*/
    .max_mirror_des_port = 1,
    
    .max_l2mc = 1024,
    .max_vr = 32,
    .max_ip_intf = 4096,
    .max_ip_route = 16384,
    .max_ip_host_extra = 16384,
    .max_ip_ecmp = 16384,
    .max_ip_route_per_ecmp = 32,
    .max_ipmc_extra = 4096,
    .max_ipv6_route = 4096,
    .max_ipv6_host_extra = 4096,
    .max_ipv6_ecmp = 4096,
    .max_ipv6_route_per_ecmp = 32,
    .max_ipv6mc_extra = 1024,
    .max_tunnel = 512,
    .max_dscp_map = 32,
    .max_ele_per_dscp_map = 32,
    .max_mpls_virtual_port = 0,
    .max_mpls_vfi = 0,
    .max_mpls_lsp = 0,
    .max_mpls_pushtagnum = 0,
    .max_mpls_handle_tagnum = 0,
    .max_vlan_acl_std = 4096,
    .max_ingress_acl_std = 8192,
    .max_egress_acl_std = 2048,
    .max_qos_meter = 8192,
    .max_qos_counter = 8192,
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
    
	.ctrl_num = 0,
	.ctrl_switch = FALSE 
};
extern long as_3028_local_conn_init(int product_type);
extern long as_3028_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    );
board_fix_param_t as_3028_param =
{
    .board_code = PPAL_HWCODE_NH_3028,
    .board_type = PPAL_BOARD_TYPE_NH_3028,
    .full_name = "AS3028 BOARD",
    .short_name = "AS3028",
    .have_pp = TRUE,
    .master_flag = FALSE,
    .service_flag = TRUE,
    .panel_portnum = 28,
    .fiber_module_fix_param = &as_3028_sfp_param,
    .ams_param = {
                    [0] = &as_3028_asic_switch
                 },
    .slotno_get = &as_3028_slotno_get,
    .reset = &as_3028_local_reset,
    .get_reset_type = &as_3028_reset_type,
    .temper_fix_param = &as_3028_temper_param,
    .sys_led_lighting = &as_3028_led_lighting,
    .pne_monitor_start = &as_3028_pne_mon_start,
    .board_man_param_get = &as_3028_mnparam_get,
    .local_conn_init = &as_3028_local_conn_init,
    .system_conn_init = &as_3028_system_conn_init,
    .asic_config_init = &as_3028_sal_config_init_defaults,
    .os_upgrade = &as_3028_npd_os_upgrade,    
    .feature = &as_3028_feature
};


long as_3028_board_online(int slot_index)
{
    return TRUE;
}

long as3028_load_backinfo(product_man_param_t *param)
{
    npd_syslog_dbg("Get product sys info\n --%s", __FUNCTION__);
	
    param->basemac = (char*)base_mac;
    param->sn = "1001";
    param->name = "AS6603";
    param->sw_name = "AuteWare OS";
    param->enterprise_name = "AUTELAN";
    param->enterprise_snmp_oid = "1001";
	
/*    param->built_in_admin_name = "netgear";
    param->built_in_admin_passwd = "";
    
*/
    nbm_read_backplane_sysinfo(param);
    return NPD_SUCCESS;
}

long as3028_slotno_get()
{
	int tipc_node_temp = tipc_get_own_node();
	printf("(%s)Local board slot no.: %d\r\n", __func__, ((tipc_node_temp&0x0F) - 1));
    return (tipc_node_temp&0x0F) - 1;
}

void as3028_npd_master_set()
{
	return;
}

board_manage_fix_param_t as3028_boardmn_fix_param =
{
    .slotnum = 3,
    .master_slotnum = 2,
    .master_slot_id = {0, 1},
    .can_distr = TRUE,
    .can_master_service = TRUE,
    .topo = FULL_MESH,
    .slot_map = {
        {0,0}, {1, 1}, {2, 2}
    },
    .board_inserted = &as_3028_board_online,
    .board_insert_detect = &as_board_detect_start,
    .master_board_inserted = &as_master_board_online,
    .board_reset = &as_board_reset,
    .board_poweroff = &as_board_poweroff
};

pne_fix_param_t as3028_pne_fix_param = 
{
    .power_num = 2,
    .fan_num = 1,
    .pne_monitor_start = &as_pne_monitor_start,
    .pne_fanspeed_adjust = &as_fan_speed_adjust
};

product_fix_param_t as3028_fix_param =
{
    .product_code = PPAL_PRODUCT_HWCODE_NH3028,
    .product_type = PRODUCT_NH_3028,
    .product_short_name = NH3028_SHORT_NAME,
    .product_name = "",
    .serial_no = "1001",
    .board_manage = &as3028_boardmn_fix_param,
    .pne_fix_param_t = &as3028_pne_fix_param,
    .product_man_param_get = &as3028_load_backinfo,
    .product_reset = &as_sys_reset,
    .product_show_chassis = &as_chassis_show,
    .slotno_get = &as3028_slotno_get,
    .master_set = &as3028_npd_master_set
};

#ifdef __cplusplus
}
#endif

