#ifndef __NPD_DBUS_H__
#define __NPD_DBUS_H__

#include "dbus/npd/npd_dbus_def.h"

int npd_dbus_init(void);
void * npd_dbus_thread_main(void *arg);

/***********************************************************************************
 *		NPD dbus operation
 *
 ***********************************************************************************/
#ifdef HAVE_VRRP
DBusMessage * npd_dbus_create_vrrp_by_ifname(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif
#ifdef HAVE_SERVICE_BOARD
DBusMessage * npd_dbus_service_board_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_traffic_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_traffic_recover_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_traffic_backup_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_board_netif_index(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_board_load_banlc_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_service_board_show(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_service_board_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data);
#ifdef HAVE_STACK_INTF
DBusMessage *npd_dbus_get_next_stk_portindex(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_stkport_attr(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

#ifdef HAVE_FW_GROUP
DBusMessage *npd_dbus_fw_group_show(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif


#endif //HAVE_SERVICE_BOARD

DBusMessage * npd_dbus_boardmng_get_next_slot(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_boardmng_show_board_attr(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_boardmng_get_next_slot(DBusConnection *conn, DBusMessage *msg, void *user_data);


DBusMessage * npd_dbus_boardmng_config_slot_type(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_boardmng_no_config_slot(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_boardmng_board_switchover(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_boardmng_board_isstandby(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_boardmng_board_reboot(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_boardmng_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_boardmng_board_range(DBusConnection *conn, DBusMessage *msg, void *user_data) ;


DBusMessage * npd_dbus_check_vlan_exist
(
	DBusConnection *conn, 
 	DBusMessage *msg, 
 	void *user_data
);
DBusMessage * npd_dbus_vlan_create_vlan_entry_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2_vname
(	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_vlan_update_name
(	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_vlan_config_port_add_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage *npd_dbus_vlan_config_netif_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
#ifdef HAVE_QINQ
DBusMessage *npd_dbus_vlan_config_netif_inner_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
#endif
DBusMessage * npd_dbus_vlan_get_vname_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_next_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_delete_vlan_entry_one(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_vlan_delete_vlan_entry_vname(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_switchport_protect(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_switchport_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_assoc_mac(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_vlan_show_assoc_mac(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_vlan_config_assoc_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_show_assoc_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_privlan_get_next_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_privlan_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_vlan_config_private(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_netif_config_private(DBusConnection *conn, DBusMessage *msg, void *user_data);

#ifdef HAVE_PORT_ISOLATE
DBusMessage * npd_dbus_netif_config_isolate(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_isolate_group_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
#endif


#ifdef HAVE_PRIVATE_VLAN


DBusMessage * npd_dbus_pvlan_primary_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_pvlan_primary_associate(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_pvlan_promis_port_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_pvlan_isolate_port_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_pvlan_get_next_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_pvlan_get_member(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_pvlan_get_next_map(DBusConnection *conn, DBusMessage *msg, void *user_data);

#endif

DBusMessage * npd_dbus_vlan_config_protovlan(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_vlan_config_protovlan_port(DBusConnection *conn, DBusMessage *msg, void *user_data);


#ifdef HAVE_QINQ

DBusMessage * npd_dbus_vlan_config_xlate(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_vlan_show_xlate(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_netif_xlate_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_netif_qinq(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

DBusMessage * npd_dbus_vlan_config_netif_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_netif_mac(DBusConnection *conn, DBusMessage *msg, void *user_data);
#ifdef HAVE_QINQ
DBusMessage * npd_dbus_vlan_config_netif_qinq_miss(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_netif_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_netif_inner_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_config_qinq_global_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif



DBusMessage * npd_dbus_vlan_egress_filter_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

#ifdef HAVE_QINQ

DBusMessage * npd_dbus_vlan_config_eline(DBusConnection *conn, DBusMessage *msg, void *user_data);


DBusMessage * npd_dbus_vlan_config_netif_eline(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

DBusMessage * npd_dbus_vlan_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_vlan_check_port_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_set_port_auth_fail_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

#ifdef HAVE_LACP
DBusMessage* npd_dbus_config_all_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data); 
DBusMessage * npd_dbus_aggregator_mode_change
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage *npd_dbus_show_lacp_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_aggregator_show_by_trunkId(DBusConnection *conn, DBusMessage *msg, void *user_data);

#endif

DBusMessage* npd_dbus_show_gvrp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_gvrp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_gvrp_timer(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage* npd_dbus_config_port_gvrp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_port_gvrp_reg_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_show_gvrp_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_config_agingtime
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_no_config_agingtime
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_blacklist
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_blacklist_with_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_create_blacklist
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_create_blacklist_with_vlanname
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_agingtime
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_config_static_fdb_item
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_config_static_fdb_with_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *  npd_dbus_fdb_delete_static_fdb
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_static_fdb_with_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_mac
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_single_unit_fdb
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_fdb_with_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_fdb_with_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_static_fdb
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_vlan
(
DBusConnection *conn, 
DBusMessage *msg, 
void *user_data
);

DBusMessage * npd_dbus_fdb_show_fdb_vlan_with_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_config_system_mac
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);



DBusMessage * npd_dbus_fdb_config_port_number
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
); 

DBusMessage * npd_dbus_fdb_config_vlan_number
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
); 

DBusMessage * npd_dbus_fdb_show_port_number
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
); 
DBusMessage * npd_system_debug_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_create_intf_by_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_del_intf_by_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_create_sub_intf
	(	
		DBusConnection *conn,
		DBusMessage *msg,
		void *user_data
	);

DBusMessage * npd_dbus_del_sub_intf
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_advanced_route
	(
		DBusConnection *conn,
		DBusMessage *msg, 
		void *user_data
	);

DBusMessage * npd_dbus_create_intf_by_vlan_ifname
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
);
DBusMessage * npd_dbus_vlan_interface_advanced_routing_enable
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
);
DBusMessage *npd_dbus_vlan_intf_check
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_ip_static_arp
( 
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
);
DBusMessage * npd_dbus_no_ip_static_arp
(   DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
);

DBusMessage *npd_dbus_show_ip_arp_info(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_clear_arp(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_static_arp_show_running
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
);

DBusMessage *npd_dbus_ip_dyntostatic_arp(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_ip_set_arp_agetime(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_ip_show_arp_agetime(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_ip_set_arp_drop(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_ip_show_arp_drop(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_static_arp_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_arp_inspection_clear_statistics(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_arp_inspection_check_trust(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_arp_inspection_satistics(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_arp_inspection_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_arp_inspection_enable(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_arp_inspection_vlan_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);   

DBusMessage * npd_dbus_arp_inspection_trust_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_arp_inspection_validate_set(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_arp_inspection_check_global(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_arp_inspection_check_vlan_by_vid(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_arp_inspection_check_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_arp_inspection_check_trust(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_config_port_mode
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_ethports_show_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);


DBusMessage* npd_dbus_eth_link_state_config
(
    DBusConnection *conn,
    DBusMessage *msg, 
    void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_enable_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_check_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_enable_vlan_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_config_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_show_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_show_static_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_enable_opt82
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
); 

DBusMessage *npd_dbus_dhcp_snp_set_opt82_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_set_opt82_fill_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_set_opt82_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
); 

DBusMessage *npd_dbus_dhcp_snp_set_opt82_port_strategy
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_set_opt82_port_circuitid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_dhcp_snp_add_del_static_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);


DBusMessage *npd_dbus_dhcp_snp_add_del_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);


DBusMessage *npd_dbus_dhcp_snp_show_running_global_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);


DBusMessage *npd_dbus_dhcp_snp_show_running_vlan_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);



DBusMessage *npd_dbus_dldp_check_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);



DBusMessage *npd_dbus_dldp_config_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_portmbr_check_igmp_snoop
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_vlan_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_config_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage* npd_dbus_igmpsnp_portmbr_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_show_vlan_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_global_endis
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_port_endis
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_port_max_power
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_port_priority
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_power_manage_mode
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_poe_interface_mode
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_config_port_poe_legacy_check
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_get_poe_interface
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_get_next_poe_interface
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_poe_get_pse_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_interface_show_ver
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_interface_show_hwconf
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);


DBusMessage * npd_dbus_config_system_temperature
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_config_system_alarm_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);


DBusMessage * show_system_environment_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_system_shut_down_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_system_arp_smac_check_enable
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
);

DBusMessage * npd_dbus_sys_global_cfg_save
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_system_arp_smac_check_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * nbm_dbus_interface_syslog_no_debug
( 	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * nam_dbus_interface_syslog_no_debug
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage* npd_dbus_asic_syslog_debug
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage* npd_dbus_asic_syslog_no_debug
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_asic_bcast_rate_limiter
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpufc_config
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_cpufc_set_queue_quota
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpufc_get_queue_quota
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpufc_set_queue_shaper
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpufc_set_port_shaper
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpu_show_sdma_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpu_show_sdma_mib
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_cpu_clear_sdma_mib
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_show_vlan_trunk_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_show_vlan_trunk_member_vname
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_show_vlanlist_trunk_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_show_vlanlist_trunk_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_slotport_by_ethportindex
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_config_vlan_mtu
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_config_vlan_egress_filter
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_config_vlan_filter
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_vlan_igmp_snp_show_running_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_member
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_trunk_load_banlc_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_allow_refuse_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_show_vlanlist
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
); 

DBusMessage * npd_dbus_trunk_show_running_config
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_show_running_config
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_static_fdb_with_vlan
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_static_fdb_with_port
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_fdb_delete_fdb_with_trunk
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_fdb_static_delete_fdb_with_trunk(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_fdb_config_static_fdb_trunk_item
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_fdb_config_static_fdb_trunk_with_name
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
); 

DBusMessage * npd_dbus_fdb_show_dynamic_fdb
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_fdb_show_blacklist_fdb
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_fdb_show_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage *npd_dbus_fdb_config_vlan_port_number
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_check_vlan_igmp_snoop
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_igmpsnp_vlan_list_show
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_create_dest_port
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_del_dest_port
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_create_source_acl
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_delete_source_acl
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_create_source_port
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_del_source_port
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_create_source_vlan
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_pvlan_show_running_cfg
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_set
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_cancel
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_mirror_show
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_delete
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_show_running_cfg
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_mirror_create_remote_vlan(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);


DBusMessage * npd_dbus_mirror_del_remote_vlan(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);


DBusMessage * npd_dbus_mirror_show_fdb(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);


DBusMessage * npd_dbus_mirror_del_source_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_show_remap_table
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_delete_qos_profile
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_delete_policy_map
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_delete_policer
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_qos_counter_show_running_config
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_qos_acl_apend
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_show_qos_acl_apend
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_time_range_acl
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * show_rtdrv_all
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * show_rtdrv_entry
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * config_ucrpf_enable
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);
DBusMessage *npd_dbus_urpf_show_running
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * show_ucrpf_enable
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * show_route_status
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_config_buffer_mode
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_config_buffer_mode
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_ethports_ipg
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_get_broad_id
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_get_port_speed
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_set_stpid_for_vlan
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_show_buffer_mode
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_get_slot_port_by_portindex
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_arp_aging_destmac_broadcast
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
); 
DBusMessage * npd_dbus_promis_port_add2_vlan_intf
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_sys_host_ingress_counter
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage * npd_dbus_sys_host_smac_ingress_counter
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);
DBusMessage * npd_dbus_sys_host_dmac_ingress_counter
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_sys_show_ingress_counter


(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);
DBusMessage *npd_dbus_sys_show_egress_counter


(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);
DBusMessage *  npd_dbus_vlan_egress_drop_statistic
(
	  
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_sys_egress_drop_statistic
(
 
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data

);
DBusMessage * npd_dbus_sys_global_counter_drop_statistic
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_sys_global_config_counter_drop_statistic
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_device_show_power_supply
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_device_get_next_psunit
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_show_temperature
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_config_temper_threshold
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_device_get_next_fan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_device_show_fan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_device_config_fan_speed
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_pvlan_create
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_pvlan_add_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_delete_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_cpu_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_config_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_config_spi_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_config_trunk
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_show_pvlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_port_delete
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_pvlan_config_control
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_ethports_interface_config_stp
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_all_vlans_bind_to_stpid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_get_port_link_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_rstp_slot_port_get
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_rstp_get_all_port_index
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_rstp_get_all_port_index_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_set_port_pathcost
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_set_port_prio
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_stp_set_port_nonstp
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_set_port_p2p
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_stp_set_port_edge
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_stp_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_create_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*enter trunk configure node*/
DBusMessage * npd_dbus_trunk_config_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*enter trunk configure node*/
DBusMessage * npd_dbus_trunk_config_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_trunk_config_update_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_add_delete_port_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_delete_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_master_port_set
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_trunk_delete_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);


/*show a special trunk member slot_port*/
DBusMessage * npd_dbus_trunk_show_one_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*show a special trunk member slot_port*/
DBusMessage * npd_dbus_trunk_show_by_name_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*original*/
DBusMessage *npd_dbus_trunk_show_trunklist_port_member_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
DBusMessage * npd_dbus_check_trunk_exist
(
   DBusConnection *conn,
   DBusMessage *msg,
   void *user_data
);


DBusMessage * npd_dbus_config_ip_tunnel_add
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_config_ip_tunnel_delete
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_config_ip_tunnel_host_add
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_config_ip_tunnel_host_delete
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_create_vlan_entry_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2_vname
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_config_port_add_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);



/*add | delete trunk to(from) vlan*/
DBusMessage * npd_dbus_vlan_config_trunk_add_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;

DBusMessage * npd_dbus_vlan_config_trunk_untag_tag_add_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_delete_vlan_entry_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_delete_vlan_entry_vname
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*npd_dbus_vlan_set_one_port_pvid*/
DBusMessage * npd_dbus_vlan_set_one_port_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

/*npd_dbus_vlan_show_one_port_pvid*/
DBusMessage * npd_dbus_vlan_show_one_port_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) ;


/*npd_dbus_vlan_show_ports_pvid*/
DBusMessage * npd_dbus_vlan_show_ports_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_show_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_check_igmp_snoop
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_vlan_get_vname_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);	
DBusMessage * npd_dbus_check_vlan_exist
(
	DBusConnection *conn, 
 	DBusMessage *msg, 
 	void *user_data
);

DBusMessage * npd_dbus_show_ethport_sfp
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
);

DBusMessage * npd_dbus_show_ethport_transceiver
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
);

DBusMessage *npd_dbus_clear_stack_stat_by_slotno_and_portno
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);
DBusMessage * npd_dbus_show_port_stat_by_slotno_and_portno
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);

DBusMessage *npd_dbus_clear_stack_stat_by_slotno_and_portno
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
);
DBusMessage * npd_dbus_show_ethport_rate
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
);

DBusMessage * npd_dbus_show_ethport_stat
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
);
DBusMessage *npd_dbus_config_storm_control(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_eth_port_attr(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_ports_vct(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_eth_port_ratelimit(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_eth_port_loopback(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_eth_port_eee(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_switchport_protect(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_ethport_ipg(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_config_ethport_desc(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_config_ethport_one(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_ethport_ipg(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_config_port_interface_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage* npd_dbus_config_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_config_all_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_get_next_portindex(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_get_next_stack_port
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
);
DBusMessage *  npd_dbus_show_switchport(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *  npd_dbus_switchport_exist(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_asd_portal_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_asd_show_portal_bypass(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_asd_set_portal_client_bypass(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_sflow_global_show_port(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage* npd_sflow_collector_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage* npd_sflow_agent_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data) ;
DBusMessage * npd_dbus_sflow_no_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_no_collector(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_config_port_endis(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_config_samplrate(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_config_agent(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_config_collector(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_sflow_no_agent(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_global_endis(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_create_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_delete_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_add_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_no_poe_time_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_on_time_deploy(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_show_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_show_time_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_show_time_bind_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_port_endis(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_port_max_power(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_port_priority(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_power_manage_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_poe_interface_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_port_poe_legacy_check(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_get_poe_interface(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_get_next_poe_interface(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_poe_get_pse_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_poe_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_power_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_poe_config_power_up_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_check_vlan_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_check_op82_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_check_op82_global_status_get(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_check_op82_interface_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_op82_port_status_get(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_check_vlan_status_by_vid(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_enable_global_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_enable_vlan_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_config_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_show_bind_table(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_dhcp_snp_enable_opt82(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_set_opt82_format_type(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_set_opt82_fill_format_type(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_set_opt82_remoteid_content(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_set_opt82_port_strategy(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_set_opt82_port_circuitid_content(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_add_del_binding(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_show_running_global_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_show_running_vlan_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_snp_show_running_save_bind_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_global(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_interface(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_server(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_no_server(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_global_show(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_interface_show(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_interface_show_one(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcpv6_relay_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_add_iphelper(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_del_iphelper(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_show_iphelper(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_enable_global_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_enable_intf_status(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_show_running_global_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_ip_dhcp_relay_check_global(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_enable_opt82(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dhcp_relay_show_running_intf_config(DBusConnection *conn, DBusMessage *msg, void *user_data);


DBusMessage* npd_dbus_get_eth_port_eee(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_ethport_attr(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_get_ethport_vct(DBusConnection *conn, DBusMessage *msg, void *user_data);



DBusMessage * npd_dbus_show_cpu_stats(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_clear_cpu_stats(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_clear_ethport_stat(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_config_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_qos_profile_attributes(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dscp_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_dscp_to_dscp_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_up_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_qos_profile_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_dscp_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_dscp_to_dscp_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_up_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_config_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_modify_qos(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_defaule_profile_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_default_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_qos_port_eg_remark(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_trust_mode_l2_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_trust_mode_l3dscp_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_trust_mode_l3remap_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
#if 0
DBusMessage * npd_dbus_policy_trust_mode_l2_l3_set(DBusConnection *conn, DBusMessage *msg, void *user_data);	*/
#endif
DBusMessage * npd_dbus_ethport_bind_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_ethport_show_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_cir_cbs(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_packetcolor(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_set_out_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_out_profile_cmd_keep_drop(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_out_profile_cmd_remap(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_enable(DBusConnection *conn, DBusMessage *msg, void *user_data);
#if 0
DBusMessage * npd_dbus_global_policer_meter_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_global_policing_mode(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif
DBusMessage * npd_dbus_set_counter(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_counter(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_policer_counter(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_read_counter(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policer_color(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_policer(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_get_policer(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_ethport_unbind_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_queue_sche_wrr_group(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_queue_scheduler(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_queue_scheduler(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_queue_drop(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage* npd_dbus_queue_def_sche(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_qos_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_delete_policer(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_delete_policer_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_policer_set_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_delete_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_delete_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_flow_control_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_flow_control(DBusConnection *conn, DBusMessage *msg, void *user_data);
#ifdef HAVE_VRRP
DBusMessage* npd_dbus_tracking_group(DBusConnection* conn, DBusMessage* msg, void* user_data);

DBusMessage* npd_dbus_tracking_group_delete(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_add_object(DBusConnection* conn, DBusMessage* msg, void* user_data);

DBusMessage* npd_dbus_tracking_group_remove_object(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_action_down(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_mode(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_object_l3exist(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_object_l2exist(DBusConnection* conn, DBusMessage* msg, void* user_data);

DBusMessage* npd_dbus_tracking_group_query(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_query_next(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_show_running(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_tracking_group_status(DBusConnection* conn, DBusMessage* msg, void* user_data);
#endif
DBusMessage* npd_dbus_smart_link_netif_status_get(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage* npd_dbus_smart_link_adv_vlan_list(DBusConnection* conn, DBusMessage* msg, void* user_data);

DBusMessage* npd_dbus_smart_link_adv_vlan_list_get(DBusConnection* conn, DBusMessage* msg, void* user_data);

DBusMessage* npd_dbus_acl_rule_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_create_acl_group(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_acl_group(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_delete_rule(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_add_rule(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_add_desp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_delete_desp(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_deploy(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_undeploy(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_undeploy_all(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_group_show_detail(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_acl_group_intf_show(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_acl_group_show_all(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_create_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_no_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_rename_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_class_map_add_match(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_class_map_delete_match(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_create_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_no_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_rename_policy_map(DBusConnection* conn, DBusMessage* msg, void* user_data);
DBusMessage * npd_dbus_policy_map_add_action(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_map_delete_action(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_map_class(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_policy_map_no_class(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_service_policy_in(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_no_policy(DBusConnection *conn, DBusMessage *msg, void *user_data);

#ifdef HAVE_SERVICE_BOARD
DBusMessage * npd_dbus_service_policy_all(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_service_no_policy_all(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage* npd_dbus_acl_ipv6_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data);

#endif

DBusMessage * npd_dbus_acl_show_cmap_list(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_acl_show_pmap_list(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_cmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_acl_show_pmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_service_policy(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_time_range_info_search_name(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_time_range_info_create(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_is_acl_associate_time_range_info(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_no_time_range_name(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_no_acl_time_range_associate(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_time_range_name(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_time_range_bind(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_acl_show_vmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data);
#ifdef HAVE_SERVICE_BOARD
DBusMessage * npd_dbus_acl_show_srv_board(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

DBusMessage * npd_dbus_match_l4port_range(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_check_policy_route_support(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_create_l3intf(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_delete_l3intf(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_shutdown_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
#ifndef HAVE_ZEBRA

DBusMessage * npd_dbus_add_ipv4_addr(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_del_ipv4_addr(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_intf_showrun(DBusConnection *conn, DBusMessage *msg, void *user_data);

#endif

#ifdef HAVE_PORTAL

DBusMessage * npd_dbus_set_portal_server(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_show_portal_interface(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_intf_portal_server_showrun(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

#ifdef HAVE_ROUTE
DBusMessage *npd_dbus_intf_urpf_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *config_intf_ipmc_enable(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif

#ifdef HAVE_ZEBRA
DBusMessage * npd_dbus_intf_proxy_arp_set(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_intf_proxy_arp_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data);
#endif
DBusMessage *npd_dbus_vlan_intf_check(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_l3intf_get_next(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_l3intf_get(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_interface_sg_service(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_source_guard_entry(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_ip_sg_port_show_enable(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_source_guard_entry_show_all(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_source_guard_entry_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_clear_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage *npd_dbus_show_ip_neigh_info(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage * npd_dbus_no_ip_static_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_ipv6_static_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage *npd_dbus_ip_set_ndisc_agetime(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_ip_show_ndisc_agetime(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *npd_dbus_static_ndisc_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data);


DBusMessage * npd_dbus_boardmng_runstate_get
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_boardmng_show_board_attr(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
); 

DBusMessage * npd_dbus_manufacture_subboard(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_pre_manufacture_board(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_manufacture_board(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_diagnosis_hw_watchdog_control(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_diagnosis_hw_watchdog_timeout_op(DBusConnection *conn, DBusMessage *msg, void *user_data) ;


DBusMessage * npd_dbus_diagnosis_env_monitor_control(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_diagnosis_reset_board(DBusConnection *conn, DBusMessage *msg, void *user_data) ;

DBusMessage * npd_dbus_dbtable_show(DBusConnection *conn, DBusMessage *msg, void *user_data);

DBusMessage * npd_dbus_mirror_config(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_create_dest_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_del_dest_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_create_source_acl(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_delete_source_acl(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_create_source_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_del_source_port(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_create_source_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_del_source_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_set(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_cancel(DBusConnection *conn, DBusMessage *msg, void *user_data);
DBusMessage * npd_dbus_sflow_config_port_endis
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_config_samplrate
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_config_agent
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_config_collector
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_show_agent
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_show_collector
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
);

DBusMessage * npd_dbus_sflow_show_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 		
	void *user_data
);

DBusMessage * npd_dbus_sflow_show_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

DBusMessage * npd_dbus_sflow_no_agent
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

DBusMessage * npd_dbus_sflow_no_collector
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

DBusMessage * npd_dbus_sflow_no_port
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

DBusMessage * npd_sflow_agent_show_config
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

DBusMessage * npd_sflow_collector_show_config
(
	DBusConnection *conn, 
	DBusMessage *msg,		
	void *user_data
);

#endif
