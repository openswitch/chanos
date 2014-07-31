#ifdef __cplusplus
extern "C"
{
#endif

#include "as2k_3200_24gc_led_code.c"

long as2k_3200_24gc_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long as2k_3200_24gc_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_slotno_get()
{
    return 0;
}

long as2k_3200_24gc_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long as2k_3200_24gc_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long as2k_3200_24gc_led_lighting(unsigned long status)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long as2k_3200_24gc_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_AS2K_3200;
    info->sn = "1001";
    return NPD_SUCCESS;	
}
#include <sal/appl/config.h>
long as2k_3200_sal_config_init_defaults(void)
{
	sal_config_set("os", "unix");
	/* phy address can use broadcom default */
	sal_config_set("phy_fiber_pref_ge0.0", "0");
	sal_config_set("phy_fiber_pref_ge1.0", "0");
	sal_config_set("phy_fiber_pref_ge2.0", "0");
	sal_config_set("phy_fiber_pref_ge3.0", "0");
	sal_config_set("phy_fiber_pref_ge4.0", "0");
	sal_config_set("phy_fiber_pref_ge5.0", "0");
	sal_config_set("phy_fiber_pref_ge6.0", "0");
	sal_config_set("phy_fiber_pref_ge7.0", "0");	
	
	sal_config_set("phy_fiber_pref_ge14.0", "1");	
	sal_config_set("phy_fiber_pref_ge15.0", "1");	
	
	/* Integrated phy port, automedium will unstable */	
	
	sal_config_set("phy_automedium_ge0.0","0");	
	sal_config_set("phy_automedium_ge1.0","0");	
	sal_config_set("phy_automedium_ge2.0","0");	
	sal_config_set("phy_automedium_ge3.0","0");	
	sal_config_set("phy_automedium_ge4.0","0");	
	sal_config_set("phy_automedium_ge5.0","0");	
	sal_config_set("phy_automedium_ge6.0","0");	
	sal_config_set("phy_automedium_ge7.0","0");	
	
	/* combo port need open automedium */
	sal_config_set("phy_automedium_ge14.0","1");	
	sal_config_set("phy_automedium_ge15.0","1");	
	
	sal_config_set("trunk_extend.0","1");

	return NPD_SUCCESS;
}

ipp_fix_param_t  as2k_3200_24gc_ipp_param =
{
    .ipp_portnum = 1,
    .ipp_phyport_map = {"eth0-1"},
    .ipp_board_map = {-2}
};

sub_board_fix_param_t as2k_3200_24gc_subboard_param = 
{
    .sub_slotnum = 1,     // more than 1
    .sub_slot_portnum = 0
};
poe_module_fix_param_t as2k_3200_24gc_poe_param = 
{
	.poe_ports = {0, 0}
};
fiber_module_fix_param_t as2k_3200_24gc_sfp_param =
{
    .fiber_module_inserted = &as2k_3200_24gc_sfp_online,
    .fiber_module_insert_detect = &as2k_3200_24gc_sfp_detect_start,
    .fiber_module_info_get = &as2k_3200_24gc_sfp_info_get,
};

long nh_null_asic_init(int unit)
{
    npd_syslog_dbg("only for debug chasm\n");
    systemInitialized = 1;
    return NPD_SUCCESS;
}

long as2k_3200_asic_led_proc(int unit)
{
    npd_syslog_dbg("AS3224 led code for unit %d loading.\n", unit);
	return soc_ledproc_config(unit, ledproc_u3200, sizeof(ledproc_u3200));
}

ams_fix_param_t as2k_3200_24gc_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = (long (*)(int ))&nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &as2k_3200_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t as2k_3200_24gc_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

temper_fix_param_t as2k_3200_24gc_temper_param =
{
    .num = 0,
};

board_feature_t as2k_3200_24gc_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 8192,
    .max_macapp = 512,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 16,
    .max_protobased_vlan = 16,
    .max_ipsubnet_vlan = 16,
    .max_stpg = 64,
    .max_trunk = 8,
    .max_port_per_trunk = 8,
    .max_fabric_trunk_extra = 0,
    .max_vlanswitch_ingress = 4,
    .max_vlanswitch_egress = 4,
    .max_qinq = 8,
    .max_range_qinq = 8, /*a range of vlan map in one q tag id*/
    .max_sec_per_range_qinq = 16, /*a range can contain section number*/
    .max_mirror_des_port = 1,
    
    .max_l2mc = 256,
    .max_vr = 32,
    .max_ip_intf = 16,
    .max_ip_route = 16,
    .max_ip_host_extra = 64,
    .max_ip_ecmp = 8,
    .max_ip_route_per_ecmp = 1,
    .max_ipmc_extra = 16,
    .max_ipv6_route = 0,
    .max_ipv6_host_extra = 0,
    .max_ipv6_ecmp = 8,
    .max_ipv6_route_per_ecmp = 1,
    .max_ipv6mc_extra = 16,
    .max_tunnel = 512,
    .max_dscp_map = 32,
    .max_ele_per_dscp_map = 32,
    .max_mpls_virtual_port = 0,
    .max_mpls_vfi = 0,
    .max_mpls_lsp = 0,
    .max_mpls_pushtagnum = 0,
    .max_mpls_handle_tagnum = 0,
    .max_vlan_acl_std = 4096,
    .max_ingress_acl_std = 512,
    .max_egress_acl_std = 256,
    .vlan_acl_portlist_len = 8,
    .ing_acl_portlist_len = -1,
    .egr_acl_portlist_len = 8,    
    .max_qos_meter = 64,
    .max_qos_counter = 64,
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
    
    .preingress_filter = FALSE,
    .ingress_filter = TRUE,
    .egress_filter = TRUE,

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
    .sflow = TRUE
};

int as2k_3200_24gc_support_product[] =
{
    PRODUCT_AS2K_3200,
    0
};

board_fix_param_t as2k_3200_24gc_param =
{
    .board_code = PPAL_BOARD_HWCODE_AS2K_3200,
    .board_type = PPAL_BOARD_TYPE_AS2K_3200,
    .full_name = "AS3224 Smart Switch",
    .short_name = "AS3200-24GC",
    .have_pp = TRUE,
    .master_flag = FALSE,
    .service_flag = TRUE,
    .panel_portnum = 24,

    .subboard_fix_param = &as2k_3200_24gc_subboard_param,
    .ipp_fix_param = &as2k_3200_24gc_ipp_param,
    .temper_fix_param = &as2k_3200_24gc_temper_param,
    .feature = &as2k_3200_24gc_feature,
    .sdk_type = SDK_BCM,
    .board_support_product = (int*)as2k_3200_24gc_support_product
};

struct board_spec_fix_param_s as2k_3200_24gc_spec_param = 
{
    .board_type = PPAL_BOARD_TYPE_AS2K_3200,

    .fiber_module_fix_param = &as2k_3200_24gc_sfp_param,
	.poe_module_fix_param = &as2k_3200_24gc_poe_param,
    .ams_param = {
                    [0] = &as2k_3200_24gc_asic_switch,
                 },

    .slotno_get = &as2k_3200_24gc_slotno_get,
    .reset = &as2k_3200_24gc_local_reset,
    .get_reset_type = &as2k_3200_24gc_reset_type,
    .sys_led_lighting = NULL,
    .pne_monitor_start = NULL,
    .board_man_param_get = &as2k_3200_24gc_mnparam_get,
    .local_conn_init = &as2k_3200_local_conn_init,
    .system_conn_init = &as2k_3200_system_conn_init,
    .asic_config_init = &as2k_3200_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

