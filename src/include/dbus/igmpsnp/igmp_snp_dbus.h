#ifndef _IGMP_SNP_DBUS_H
#define _IGMP_SNP_DBUS_H

/*
 *IGMP snooping
 *
 */
#define IGMP_DBUS_BUSNAME	"aw.igmp"
#define IGMP_DBUS_OBJPATH	"/aw/igmp"
#define IGMP_DBUS_INTERFACE	"aw.igmp"

#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG		"igmp_snp_debug"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE	"igmp_snp_en"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_FASTLEAVE_ENABLE    "igmp_snp_fastleave_en"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_QURRY_SIP      "igmp_snp_query_source_ip" 
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GLOBAL_QURRY_SIP    "igmp_snp_global_query_source_ip" 
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_PORT_FILTER_ENABLE	"igmp_snp_port_filter_en"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER	"igmp_snp_config_timer"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP	"igmp_snp_del_group"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE	"igmp_snp_del_gvlan_one"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL	"igmp_snp_del_all_gvlan"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT          "igmp_snp_del_mcroute_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT_VLAN     "igmp_snp_del_mcroute_port_vlan"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT_ALL      "igmp_snp_del_mcroute_port_all"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER		"igmp_snp_show_timer"
#define IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_STATUS	"check_igmp_status"
#define IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_PORT_FILTER		"check_igmp_snp_port_filter"
#define IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_TOTAL_COUNT	"show_group_total_count"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT	"add_del_mcroute_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_EN_DIS	"igmp_snp_vlan_add_delete"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ETH_PORT_EN_DIS	"igmp_snp_port_endis"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW		"igmp_snp_show_vlan_cnt"
//#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW		"igmp_snp_show_group_cnt"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW	"show_igmp_total_group_count"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_CONFIG_PARAMETER_GET		"igmp_snp_config_parameter_get"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT			"igmp_snp_show_route_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ROUTE_PORT_SHOW_RUNNING   "igmp_snp_route_port_show_running"
#define IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST_COUNT	"show_group_list_count"
#define IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_FASTLEAVE_STATUS "check_igmp_snp_fastleave_state"
#define IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_SPEC_PORT_MEMBERS	"show_group_spec_mbr"
#define IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST_PORT_MEMBERS	"show_group_list_mbr"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_STATIC_MCGROUP_ADD_PORT  "igmp_snp_static_group_add_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_STATIC_MCGROUP_DEL_PORT  "igmp_snp_static_group_del_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_CONFIG_MAX_JOIN_GROUP "igmp_snp_config_max_join_group"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_CHECK_MAX_JOIN_GROUP "check_igmp_snp_max_join_group" 
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GET_QUERIER_SOURCE  "igmp_snp_get_global_querier_source"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GET_VLAN "igmp_snp_get_vlan"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_CONFIG_PORT_MAX_JOIN_GROUP "igmp_snp_config_port_max_join_group"
#ifdef HAVE_MVLAN
#define IGMP_SNP_DBUS_METHOD_CONFIG_MVLAN_ON_VLAN       "igmp_snp_config_mvlan_on_vlan"
#define IGMP_SNP_DBUS_METHOD_MVLAN_PORT_SET_VLANID   "igmp_snp_config_mvlan_port_vlanid"
#endif
#define IGMP_SNP_DBUS_METHOD_GET_NEXT_CONFIG_PORT			  "igmp_snp_get_next_cfg_port"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_PORT_CONFIG_SHOW_RUNNING "igmp_snp_config_port_show_running"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_CONFIG_SHOW_RUNNING "igmp_snp_config_vlan_show_running"

#define PIM_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST          "pim_snp_show_mcgroup_list"

#define IGMP_DBUS_WRITE 1
#define IGMP_DBUS_READ  2

struct dbus_ctx {     
	DBusWatch *watch;
	int fd;
	int event;
	struct dbus_ctx *next;
	struct dbus_ctx *prev;
	};
extern struct dbus_ctx watch_event_head;
#endif

