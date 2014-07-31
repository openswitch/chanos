#ifndef _ASD_DBUS_DEF_H
#define _ASD_DBUS_DEF_H

extern char ASD_DBUS_BUSNAME[];
extern char ASD_DBUS_OBJPATH[];
extern char ASD_DBUS_INTERFACE[];
extern char ASD_DBUS_STA_OBJPATH[];
extern char ASD_DBUS_STA_INTERFACE[];

#define ASD_DBUS_STA_METHOD_SHOWSTA	"show_sta"
#define ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA	"extend_show_sta"
#define ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC "show_sta_by_mac"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID	"show_info_bywtpid"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID_BSS	"show_info_bywtpid_bss"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID	"show_info_bywlanid"

/*showting  information for mib nl*/
/*----------------------------------------------------------------------------------------------*/
#define ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP	"show_asd_collect_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_OF_ALL_WTP	"show_asd_stats_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_stats_info_of_wlan_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_ssid_stats_info_of_wlan_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP	"show_asd_terminal_info_of_all_wtp"	/*lzh add 20100512*/
#define ASD_DBUS_STA_METHOD_SHOW_STA_INFO_OF_ALL_WTP		"show_asd_sta_info_of_all_wtp" /*liuzhenhua append 2010-05-28*/
#define ASD_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION "show_asd_ssid_config_information_of_all_wtp" /*liuzhenhua append 2010-05-21*/

#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_BASIC_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_wapi_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_USER_LINK_INFO_OF_ALL_WTP		"show_user_link_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_UNICAST_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_unicast_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_STATS_INFO_OF_ALL_WLAN	"show_asd_wapi_stats_performance_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_EXTEND_CONFIG_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_wapi_extend_config_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_BSS_INFO_OF_ALL_WLAN	"show_asd_wapi_bss_performance_info_of_all_wlan"

#define ASD_DBUS_STA_METHOD_SHOW_RADIO_WIRELESS_INFO_BYWTPID	"show_radio_wireless_info_bywtpid"	/*nl add 20100502*/
#define ASD_DBUS_STA_METHOD_SHOW_RADIO_NEW_WIRELESS_INFO_BYWTPID	"show_radio_new_wireless_info_bywtpid"/*nl add 20100607*/
#define ASD_DBUS_STA_METHOD_SHOW_SECURITY_INFO_BYWTPID	"show_security_info_bywtpid"/*nl add 20100607*/
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_INFORMATION	"show_all_wtp_information"/*nl add 20100622*/


/*----------------------------------------------------------------------------------------------*/




#define ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID	"show_radio_info_bywtpid"	/*ht add 090428*/
#define ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID	"show_wapi_info_bywtpid"	/*ht add 090622*/
#define ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME	"show_channel_access_time"	/*ht add 090513*/

#define ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID	"show_mib_info_byradioid"	/*	xm0616*/
#define ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID	"show_wapi_mib_info_byradioid"	/*	xm0623*/

#define ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX	"show_traffic_limit_by_bssindex"	/*	xm0723*/
#define ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO	"show_traffic_limit_by_radioid"	/*	xm0723*/


#define ASD_DBUS_STA_METHOD_SET_AC_FLOW	 "set_ac_flow"  /*xm0714*/
#define ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE	 "set_extern_balance"  /*xm0714*/



#define ASD_DBUS_STA_METHOD_KICKSTA	"kick_sta"

#define ASD_DBUS_STA_METHOD_ADD_MAC_FILTER_LIST				"add_MAC_filter_list"
#define ASD_DBUS_STA_METHOD_FILTER_LIST_ADD_MAC				"filter_list_add_MAC"
#define ASD_DBUS_STA_METHOD_FILTER_LIST_DEL_MAC				"filter_list_del_MAC"
#define ASD_DBUS_STA_METHOD_DEL_MAC_FILTER_LIST				"del_MAC_filter_list"
#define ASD_DBUS_STA_METHOD_WLAN_UNUSE_MAC_LIST				"wlan_unuse_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_SHOW_MAC_FILTER_LIST	 		"show_MAC_filter_list"

#define ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST				"wlan_add_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST				"wtp_add_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST				"bss_add_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST				"wlan_del_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST				"wtp_del_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST				"bss_del_MAC_list_sta"

#define ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST				"wlan_use_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST				"wtp_use_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST				"bss_use_MAC_list_sta"
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST	 		"show_all_wlan_MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST	 		"show_all_wtp_MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST	 		"show_all_bss_MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST	 			"show_wlan_MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_BLACK_MAC_LIST        "show_wlan_black__MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WHITE_MAC_LIST        "show_wlan_white__MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST	 			"show_wtp_MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WTP_BLACK_MAC_LIST        "show_wtp_black__MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_WTP_WHITE_MAC_LIST        "show_wtp_white__MAC_list"
#define ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST	 			"show_bss_MAC_list"
#define ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG 	"wlan_list_show_running_config" 
#define ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG 	"wtp_list_show_running_config" 
#define ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG 	"bss_list_show_running_config" 
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST	 		"show_wlan_wids_MAC_list"
#define ASD_DBUS_STA_METHOD_STALIST							"show_sta_list"
#define ASD_DBUS_STA_METHOD_WIRELESS_STALIST				"show_wireless_sta_list"
#define ASD_DBUS_STA_METHOD_WIRELESS_STALIST_BY_WTP			"show_wireless_sta_list_by_wtp"
#define ASD_DBUS_STA_METHOD_WIRELESS_STALIST_BY_WLAN		"show_wireless_sta_list_by_wlan"
#define ASD_DBUS_STA_METHOD_WIREd_STALIST					"show_wired_sta_list"
#define ASD_DBUS_STA_METHOD_GET_STA_INFO					"get_sta_info" 

#define ASD_DBUS_STA_METHOD_STA_SUMMARY	"show_sta_summary"
#define ASD_DBUS_STA_METHOD_WLAN_STA_SUMMARY	 "show_wlan_sta_summary"
#define ASD_DBUS_STA_METHOD_WTP_STA_SUMMARY	 "show_wtp_sta_summary"

#define ASD_DBUS_STA_METHOD_WLAN_STALIST	"show_wlan_sta_list"

#define ASD_DBUS_STA_METHOD_WTP_STALIST	"show_wtp_sta_list"
#define ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST	"extend_show_wtp_sta_list"
#define ASD_DBUS_STA_METHOD_EXTEND_SHOW_BSS_STA_STALIST "extend_show_bss_sta_list"

#define ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT	"set_sta_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL	"cancel_sta_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT	"set_sta_send_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL	"cancel_sta_send_traffic_limit"

#define ASD_DBUS_SET_STA_MAC_VLANID			"set_sta_mac_vlanid" /*ht add 091028*/
#define ASD_DBUS_CHECK_STA_BYMAC			"check_sta_bymac" /*ht add 100113*/
#define ASD_DBUS_SET_STA_STATIC_INFO		"set_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_DEL_STA_STATIC_INFO		"del_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_SHOW_STA_STATIC_INFO_BYMAC	"show_sta_static_info_bymac" /*ht add 100113*/
#define ASD_DBUS_SHOW_STA_STATIC_INFO		"show_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_STA_METHOD_SHOW_STATIC_STA_RUNNING_CONFIG	"show_static_sta_running_config"

#define ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN	 "set_asd_sta_arp_listen"
#define ASD_DBUS_STA_METHOD_SET_STA_ARP		"set_sta_arp"
#define ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP	 "set_asd_sta_static_arp"
#define ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP_IF_GROUP	 "set_asd_sta_static_arp_if_group"



extern char ASD_DBUS_SECURITY_OBJPATH[];
extern char ASD_DBUS_SECURITY_INTERFACE[];


#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS		"show_radius"

#define ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO	"show_security_wapi_info" 

#define ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF	"show_wlan_security_wapi_conf"


#define ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT	"asd_update_wtp_count"

#define ASD_DBUS_SECURITY_METHOD_SET_ACCT	"set_acct"

#define ASD_DBUS_SECURITY_METHOD_SET_AUTH	"set_auth"


#define ASD_DBUS_SECURITY_METHOD_DEL_RADIUS_SERVER_BY_IP	"del_radius_server_by_ip"
#define ASD_DBUS_SECURITY_METHOD_DEL_RADIUS_SERVER_BY_NAME	"del_radius_server_by_name"
#define ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_BY_IP "update_radius_server_by_ip"
#define ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_BY_NAME "update_radius_server_by_name"
#define ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_NAME_BY_IP "update_radius_server_name_by_ip"

#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH	"set_wpai_auth"
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH	"set_wpai_certification_path"
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT "set_wpai_multi_cert"

#define ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD	"eap_reauth_period" /*xm 08/09/02*/
#define ASD_DBUS_SECURITY_METHOD_WEP_INDEX_PERIOD	"security_index_period" /*nl 10/03/15*/
#define ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL	"acct_interim_interval" /*xm 08/09/02*/

#define ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD	"security_set_quiet_period" /*ht 090727*/
#define ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP "security_host_ip"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE	"security_type"

#define ASD_DBUS_SECURITY_METHOD_CONFIGURE_VLAN_ENABLE  "configure_vlan_enable"/*jianchao add*/
#define ASD_DBUS_SECURITY_METHOD_GUEST_VLAN_PERIOD  "security_set_guest_vlan_period"/*jianchao add*/
#define ASD_DBUS_SECURITY_METHOD_TX_PERIOD  "security_set_tx_period"/*jianchao add*/
#define ASD_DBUS_SECURITY_METHOD_SUPP_TIMEOUT  "security_set_supp_timeout"/*jianchao add*/
#define ASD_DBUS_SECURITY_METHOD_SERV_TIMEOUT  "security_set_serv_timeout"/*jianchao add*/
#define ASD_DBUS_SECURITY_METHOD_MAC_AUTH_BYPASS   "security_set_mac_auth_bypass"/*jianchao add*/


#define ASD_DBUS_SECURITY_METHOD_WPA_GROUP_REKEY_PERIOD	"wpa_group_rekey_period"
#define ASD_DBUS_SECURITY_METHOD_WPA_KEYUPDATE_TIMEOUT_PERIOD	"wpa_keyupdate_timeout_period"

#define ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD	"wapi_ucast_rekey_method"	/*	xm0701*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA	"wapi_rekey_para"	/*	xm0701*/


#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE	"wapi_sub_attr_sertificateupdate" /*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE	"wapi_sub_attr_multicastupdate" /*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE	"wapi_sub_attr_unicastupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE	"wapi_sub_attr_bklifetimeupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE	"wapi_sub_attr_bkreauth_threasholdupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE	"wapi_sub_attr_satimeoutupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_CIPHER 	"wapi_sub_attr_multicast_cipher"/*ht 100112*/


#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE	"wapi_sub_attr_wapipreauth"

#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MUTICASEREKEYSTRICT_UPDATE	"wapi_sub_attr_muticaserekeystrict"
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICASTCIPHERENABLED_UPDATE	"wapi_sub_attr_unicastcipherenabled"
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_AUTHENTICATIONSUITEENABLE_UPDATE	"wapi_sub_attr_authenticationsuteenable"






#define ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE	"encryption_type"
#define ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH	"extensible_auth"
#define ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT	"RADIUS_SERVER_SELECT"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK		"security_wlan_check"

#define ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_SYSTEM_CONTROL "security_show_system_auth_control"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG	"security_show_running_config"
#define ASD_DBUS_SECURITY_METHOD_HANSI_SECURITY_SHOW_RUNNING_CONFIG	"hansi_security_show_running_config"
#define ASD_DBUS_SECURITY_METHOD_RADIUS_SHOW_RUNNING_CONFIG	"radius_show_running_config"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_KEY	"security_key"

#define ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE	"configure_port_enable"/*xm 08/09/01*/
#define ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT	    "configure_port"/*xm 08/09/01*/

#define ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_SECURITY	    "set_port_vlan_append_security"/*sz20080825*/ 
#define ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_ENABLE	    "set_port_vlan_append_enable"/*sz20080825 */
#define ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY       "set_vlan_list_append_security"/*sz20080825*/
#define ASD_DBUS_SECURITY_METHOD_SET_VLAN_LIST_APPEND_ENABLE    "set_vlan_list_append_enable"/*sz20080825*/
#define ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG    	"set_asd_daemonlog_debug"/*ht 08.12.01*/
#define ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG	"set_asd_logger_printflag"/*ht 08.12.04*/
#define ASD_DBUS_SECURITY_METHOD_SET_HOSTAPD_LOGGER_PRINTFLAG	"set_hostapd_logger_printflag"/*ht 08.12.04*/

#define ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION		"pre_authentication"
#define ASD_DBUS_MOBILE_OPEN	"mobile_open"
#define ASD_DBUS_NOTICE_STA_INFO_TO_PORTAL_OPEN	"asd_notice_sta_info_to_portal"
#define ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN	"asd_notice_sta_info_to_proto"

#define ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON		"accounting_on"
#define ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR		"radius_extend_attr"


/*jianchao add 10/05/28*/
#define ASD_DBUS_SECURITY_METHOD_VLAN_CONTROL  "set_dot1x_vlan_control_mode"
#define ASD_DBUS_SECURITY_METHOD_PORT_VLAN_CONTROL  "set_dot1x_port_vlan_control"
#define ASD_DBUS_SECURITY_METHOD_CLEAR_DOT1X_STATISTICS "clear_dot1x_statistics"

#define ASD_DBUS_SECURITY_METHOD_SET_AUTHENTICATION_MODE "set_dot1x_authentication_mode"
#define ASD_DBUS_SECURITY_METHOD_SHOW_AUTHENTICATION_MODE "show_dot1x_authentication_mode"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_SUMMARY "show_dot1x_summary"


#define ASD_DBUS_SECURITY_METHOD_CLEAR_RADIUS_STATISTICS "clear_radius_statistics"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_GUEST_VLAN "set_asd_dot1x_guest_vlan"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_MAX_REQUEST "set_asd_dot1x_max_request"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_MAX_USER "set_asd_dot1x_max_user"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_CONTROL "set_dot1x_port_control_mode"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_CONTROL_ALL "set_dot1x_port_control_mode_all"
#define ASD_DBUS_SECURITY_METHOD_REAUTHENTICATE_PORT "set_dot1x_port_reauthenticate_port"
#define ASD_DBUS_SECURITY_METHOD_SET_PORT_REAUTHENTICATION "set_dot1x_port_reauthentication"

#define ASD_DBUS_SECURITY_METHOD_SET_WLAN_ISOLATION_STATUS "set_wlan_isolation_configuration"
#define ASD_DBUS_SECURITY_METHOD_GET_WLAN_ISOLATION_CONFIGURATION		"get_wlan_isolation_configuration"

#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_SYSTEM_AUTH_CONTROL "set_dot1x_port_system_auth_control"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_GUEST_VLAN_PERIOD "set_dot1x_timeout_guest_vlan_period"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_REAUTH_PERIOD "set_dot1x_timeout_reauth_period"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_QUIET_PERIOD "set_dot1x_timeout_quiet_period"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_TX_PERIOD "set_dot1x_timeout_tx_period"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_SUPP_TIMEOUT "set_dot1x_timeout_supp_timeout"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_TIMEOUT_SERVER_TIMEOUT "set_dot1x_timeout_server_timeout"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_GUEST_VLAN "set_dot1x_port_guest_vlan"
#define ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_AUTH_FAIL_VLAN "set_dot1x_port_auth_fail_vlan"
#define ASD_DBUS_SECURITY_METHOD_SHOW_AUTHENTICATION_METHODS "show_dot1x_authentication_methods"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_INFORMATION "show_dot1x_information"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_TIMER_INFORMATION "show_dot1x_timer_information"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_SUMMARY_INFORMATION "show_dot1x_port_summary_information"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_DETAIL_INFORMATION "show_dot1x_port_detail_information"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_STATISTICS_INFORMATION "show_dot1x_port_statistics_information"
#define ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_CLIENTS_INFORMATION "show_dot1x_port_clients_information"

#define ASD_DBUS_SECURITY_METHOD_DOT1X_GET_NEXT_IFINDEX    "dot1x_get_next_ifindex"
#define ASD_DBUS_SECURITY_METHOD_SET_AUTHORIZATION_NETWORK_RADIUS "set_authorization_network_radius"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_ATTR_4 "set_radius_server_attr_4"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_ATTR_4_IP_ADDR "set_radius_server_attr_4_ip_addr"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_HOST_IP_ADDR "set_radius_server_host_ip_addr"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_HOST_IP_ADDR_NAME "set_radius_server_host_ip_addr_name"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_HOST_IP_ADDR_PORT "set_radius_server_host_ip_addr_port"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_HOST_IP_ADDR_NAME_PORT "set_radius_server_host_ip_addr_name_port"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_KEY "set_radius_server_key"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_MSGAUTH "set_radius_server_msgauth"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_PRIMARY_BY_IP "set_radius_server_primary"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_PRIMARY_BY_NAME "set_radius_server_primary_by_name"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_RETRANSMIT "set_radius_server_retransmit"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_TIMEOUT "set_radius_server_timeout"
#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_CONFIGURATION "show_radius_conguration"
#define ASD_DBUS_SECURITY_METHOD_SHOW_CURR_RADIUS_SERVER_CONF "show_curr_radius_server_conf"
#define ASD_DBUS_SECURITY_METHOD_DOT1X_GET_NEXT_RADIUS_SERVER "dot1x_get_next_radius_server"
#define ASD_DBUS_SECURITY_METHOD_SHOW_ALL_RADIUS_ACCOUNTING_NAME "show_all_radius_accounting_name"
#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_ACCOUNTING_NAME "show_radius_accounting_name"
#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_STATISTICS_BY_IP "show_radius_statistics_by_ip"
#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_STATISTICS_BY_NAME "show_radius_statistics_by_name"
#define ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_ATTR_DEF "set_radius_server_attr_DEF"


#define ASD_DBUS_SECURITY_METHOD_ADD_LOCAL_USER "add_local_uesr"
#define ASD_DBUS_SECURITY_METHOD_DELETE_LOCAL_USER "delete_local_uesr"

#define ASD_DBUS_SECURITY_SHOW_LOCAL_USER "show_dot1x_local_uesr"

#define ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE	"set_asd_get_sta_info_able"	//ht add 091111
#define ASD_DBUS_SET_ASD_GET_STA_INFO_TIME	"set_asd_get_sta_info_time"
#define ASD_DBUS_SET_ASD_PROCESS_80211N_ABLE	"set_asd_process_80211n_able"	//ht add 091111

/*xm add 09/02/13*/
#define ASD_DBUS_SIG_STA_LEAVE	"signal_sta_leave"
#define ASD_DBUS_SIG_STA_COME   "signal_sta_come"
#define ASD_DBUS_SIG_WTP_DENY_STA   "signal_wtp_deny"
#define ASD_DBUS_SIG_DE_WTP_DENY_STA "signal_wtp_de_deny"

//#define ASD_DBUS_SIG_DENY_STA_NUM   "signal_deny_sta_num"
//ht add 090216
#define ASD_DBUS_SIG_WAPI_TRAP	"signal_wapi_trap"
#define ASD_DBUS_SIG_STA_VERIFY	"signal_sta_verify"
#define ASD_DBUS_SIG_STA_VERIFY_FAILED   "signal_sta_verify_failed"
#define ASD_DBUS_SIG_RADIUS_CONNECT_FAILED   "signal_radius_connect_failed"
#define ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN   "signal_radius_connect_failed_clean"
#define ASD_DBUS_SIG_STA_JIANQUAN_FAILED				"signal_jianquan_fail"
#define ASD_DBUS_SIG_STA_ASSOC_FAILED				"signal_assoc_fail"



#define ASD_DBUS_SIG_KEY_CONFLICT				"signal_key_conflict"
#define ASD_DBUS_SIG_DE_KEY_CONFLICT				"signal_de_key_conflict"

#define ASD_DBUS_SECURITY_METHOD_TRAP_OPEN 		"asd_trap_open"
#define ASD_DBUS_SET_TRAP_ABLE 			"asd_set_trap_able"
#define ASD_DBUS_SHOW_TRAP_STATE 		"show_asd_trap_state"
#define ASD_DBUS_SHOW_DBUS_COUNT 		"show_asd_dbus_count"
#define ASD_DBUS_SET_DBUS_COUNT 		"set_asd_dbus_count"

#define ASD_DBUS_SECURITY_METHOD_FORCE_LOGOFF_STA     "asd_force_logoff_sta"

#ifdef HAVE_PORTAL
#define ASD_DBUS_SECURITY_METHOD_SET_PORTAL     "asd_set_portal"
#define ASD_DBUS_SECURITY_METHOD_GET_PORTAL     "asd_get_portal"
#define ASD_DBUS_SECURITY_METHOD_GET_PORTAL_STA "asd_get_portal_sta"
#endif

extern char ASD_DBUS_AC_GROUP_OBJPATH[];
extern char ASD_DBUS_AC_GROUP_INTERFACE[];

#define ASD_DBUS_AC_GROUP_METHOD_CREATE_GROUP "create_ac_group"
#define ASD_DBUS_AC_GROUP_METHOD_DELETE_GROUP  "delete_ac_group"
#define ASD_DBUS_AC_GROUP_METHOD_ADD_GROUP_MEMBER  "add_ac_group_member"
#define ASD_DBUS_AC_GROUP_METHOD_DEL_GROUP_MEMBER  "del_ac_group_member"
#define ASD_DBUS_AC_GROUP_METHOD_CONFIG  "config_ac_group"
#define ASD_DBUS_AC_GROUP_METHOD_HOST_IP  "set_host_ip"
#define ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_LIST  "show_ac_group_list"
#define ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP  "show_ac_group"
#define ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_ACM_ONE "show_ac_group_ac_acm_one"
#define ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_RUNNING_CONFIG  "show_ac_group_running_config"


enum asd_security_type {
	OPEN,
	SHARED,
	IEEE8021X,
	WPA_P,
	WPA2_P,
	WPA_E,
	WPA2_E,
	MD5,
	WAPI_PSK,
	WAPI_AUTH  
};

enum asd_encryption_type {
	NONE,
	WEP,
	AES,
	TKIP,
	SMS4
};
enum auth_state { AUTH_REQUEST, 
	   AUTH_RESPONSE, 
	   AUTH_SUCCESS,
	   AUTH_FAIL,
	   AUTH_TIMEOUT,
	   AUTH_IDLE,
	   AUTH_INITIALIZE,
	   AUTH_IGNORE
};

enum pae_state { PAE_INITIALIZE,
	   PAE_DISCONNECTED,
	   PAE_CONNECTING,
	   PAE_AUTHENTICATING,
	   PAE_AUTHENTICATED,
	   PAE_ABORTING,
	   PAE_HELD,
	   PAE_FORCE_AUTH,
	   PAE_FORCE_UNAUTH,
	   PAE_RESTART
};

#endif
