
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dbus.c
*
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		dbus message main routine for NPD module.
*
* DATE:
*		12/21/2007	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.166 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"
#include "dbus/npd/npd_dbus_def.h"

//#include "npd_dhcp_snp_com.h"

 DBusConnection *npd_dbus_connection = NULL;

static DBusHandlerResult npd_dbus_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessage		*reply = NULL;
		
	if (strcmp(dbus_message_get_path(message),NPD_DBUS_OBJPATH) == 0) {
    	/*syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_OBJPATH"\r\n");*/
		if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_VER)) {
			reply = npd_dbus_interface_show_ver(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_SHUTDOWN_STATE)) {
			reply = npd_system_shut_down_enable(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE)) {
			reply = npd_system_debug_enable(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_METHOD_ASIC_SYSLOG_DEBUG)) {
			reply = npd_dbus_asic_syslog_debug(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_METHOD_ASIC_SYSLOG_NO_DEBUG)) {
			reply = npd_dbus_asic_syslog_no_debug(connection,message,user_data);
		}

	
#ifdef HAVE_DIAG
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_SYSTEM_WATCHDOG_CONTROL)) {
			reply = npd_dbus_diagnosis_hw_watchdog_control(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_SYSTEM_WATCHDOG_TIMEOUT)) {
			reply = npd_dbus_diagnosis_hw_watchdog_timeout_op(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_SYSTEM_ENVMONITOR_CONTROL)) {
			reply = npd_dbus_diagnosis_env_monitor_control(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_SYSTEM_RESET_BOARD)) {
			reply = npd_dbus_diagnosis_reset_board(connection, message, user_data);
		}		
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_DBTABLE_SHOW)) {
			reply = npd_dbus_dbtable_show(connection, message, user_data);
		}	
#endif
#ifdef HAVE_IP_TUNNEL
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_IP_TUNNEL_ADD)) {
			reply = npd_dbus_config_ip_tunnel_add(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_IP_TUNNEL_DELETE)) {
			reply = npd_dbus_config_ip_tunnel_delete(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_IP_TUNNEL_HOST_ADD)) {
			reply = npd_dbus_config_ip_tunnel_host_add(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_IP_TUNNEL_HOST_DELETE)) {
			reply = npd_dbus_config_ip_tunnel_host_delete(connection,message,user_data);

		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTERFACE, NPD_DBUS_IP_TUNNEL_SHOW_TAB)){
            		reply = npd_dbus_ip_tunnel_show_tab(connection,message,user_data);
		}	
#endif
#ifdef HAVE_M4_TUNNEL
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_IP_TUNNEL))
        {
            reply = npd_dbus_ip_tunnel(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_NO_IP_TUNNEL))
        {
            reply = npd_dbus_no_ip_tunnel(connection,message,user_data);
        }
#if 0
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_IPV6_ADDRESS))
        {
            reply = npd_dbus_tunnel_in6(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_NO_IPV6_ADDRESS))
        {
            reply = npd_dbus_tunnel_no_in6(connection,message,user_data);
        }
#endif
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_SOURCE))
        {
            reply = npd_dbus_tunnel_source(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_DESTINATION))
        {
            reply = npd_dbus_tunnel_destination(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_MODE))
        {
            reply = npd_dbus_tunnel_mode(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_SHOW_ID))
        {
            reply = npd_dbus_show_interface_tunnel_id(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_SHOW_NEXT))
        {
            reply = npd_dbus_show_interface_tunnel_next(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_TUNNEL_SHOW_RUN))
        {
            reply = npd_dbus_tunnel_show_running(connection,message,user_data);
        }
#if 0
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_INTERFACE_TUNNEL))
        {
            reply = npd_dbus_interface_tunnel(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_INTERFACE, NPD_DBUS_METHOD_NO_INTERFACE_TUNNEL))
        {
            reply = npd_dbus_no_interface_tunnel(connection,message,user_data);
        }
#endif
#endif
#ifdef HAVE_VRRP
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTERFACE,NPD_DBUS_FDB_METHOD_CREATE_VRRP_BY_IFNAME))
		{
			reply = npd_dbus_create_vrrp_by_ifname(connection,message,user_data);
		}
#endif
	} 
	else if (strcmp(dbus_message_get_path(message),NPD_DBUS_DEVICE_OBJPATH) == 0)  {
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_DEVICE_OBJPATH);
#ifdef HAVE_POWER_MONITOR
		if(dbus_message_is_method_call(message,NPD_DBUS_DEVICE_INTERFACE,NPD_DBUS_DEVICE_SHOW_POWER_SUPPLY))
		{
			reply = npd_dbus_device_show_power_supply(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_DEVICE_INTERFACE,NPD_DBUS_DEVICE_NEXT_PSUNIT))
		{
			reply = npd_dbus_device_get_next_psunit(connection,message,user_data);
		}
#endif
#ifdef HAVE_FAN_MONITOR
		else if (dbus_message_is_method_call(message,NPD_DBUS_DEVICE_INTERFACE,NPD_DBUS_DEVICE_SHOW_FAN))
		{
			reply = npd_dbus_device_show_fan(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,NPD_DBUS_DEVICE_INTERFACE,NPD_DBUS_DEVICE_NEXT_FAN))
		{
			reply = npd_dbus_device_get_next_fan(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_DEVICE_INTERFACE,NPD_DBUS_DEVICE_CONFIG_FANSPEED))
		{
			reply = npd_dbus_device_config_fan_speed(connection,message,user_data);
		}
#endif
	} 
	else if (strcmp(dbus_message_get_path(message),NPD_DBUS_BOARDMNG_OBJPATH) == 0)  {
		syslog_ax_dbus_dbg("npd obj path %s\r\n", NPD_DBUS_BOARDMNG_OBJPATH);
	    if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_SHOW_SLOT_ATTR))
		{
			reply = npd_dbus_boardmng_show_board_attr(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_NEXT_SLOT))
		{
			reply = npd_dbus_boardmng_get_next_slot(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_CONFIG_SLOT_TYPE))
		{
			reply = npd_dbus_boardmng_config_slot_type(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_NO_CONFIG_SLOT))
		{
			reply = npd_dbus_boardmng_no_config_slot(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_SHOW_RUN))
		{
			reply = npd_dbus_boardmng_show_running(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_BOARD_RANGE))
		{
			reply = npd_dbus_boardmng_board_range(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_BOARD_SWITCHOVER))
		{
			reply = npd_dbus_boardmng_board_switchover(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_BOARD_ISSTANDBY))
		{
			reply = npd_dbus_boardmng_board_isstandby(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_BOARDMNG_INTERFACE,NPD_DBUS_BOARDMNG_BOARD_REBOOT))
		{
			reply = npd_dbus_boardmng_board_reboot(connection,message,user_data);
		}

	} 
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_VLAN_OBJPATH) == 0) {
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_VLAN_OBJPATH);

		if (dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,\
			                            NPD_DBUS_VLAN_METHOD_CHECK_VLAN_EXIST)){
			reply = npd_dbus_check_vlan_exist(connection, message, user_data);
		}/**check vlan exists******/
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE)) {
			reply = npd_dbus_vlan_create_vlan_entry_one(connection,message,user_data);
		}/*create_vlan_interface_node in SW&HW.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE)) {

			reply = npd_dbus_vlan_config_layer2(connection,message,user_data);
		}/*enter_vlan_config_node.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_VIA_VLANNAME)) {

			reply = npd_dbus_vlan_config_layer2_vname(connection,message,user_data);
		}/*enter_vlan_config_node.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_UPDATE_NAME)) {

			reply = npd_dbus_vlan_update_name(connection,message,user_data);
		}/*enter_vlan_config_node.*/

        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,   \
										NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_PVID)) {

			reply = npd_dbus_vlan_config_netif_pvid(connection,message,user_data);
		}/*private_vlan_config.*/
#ifdef HAVE_QINQ		
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,   \
										NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_INNER_PVID)) {

			reply = npd_dbus_vlan_config_netif_inner_pvid(connection,message,user_data);
		}/*private_vlan_config.*/	
#endif		
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,   \
										NPD_DBUS_VLAN_METHOD_CONFIG_VLAN_PRIVATE)) {

			reply = npd_dbus_vlan_config_private(connection,message,user_data);
		}/*private_vlan_config.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_PRIVATE)) {

			reply = npd_dbus_netif_config_private(connection,message,user_data);
		}/*private_vlan_netif_config.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_ASSOC_MAC)) {

			reply = npd_dbus_vlan_config_assoc_mac(connection,message,user_data);
		}/*mac_base_vlan_config.*/
#ifdef HAVE_PORT_ISOLATE		
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ISOLATE)) {

			reply = npd_dbus_netif_config_isolate(connection,message,user_data);
		}/*port isolate config.*/
#endif				
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_SHOW_ASSOC_MAC)) {
			reply = npd_dbus_vlan_show_assoc_mac(connection,message,user_data);
		}/*mac_base_vlan_show.*/
		else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_ASSOC_SUBNET)) {

			reply = npd_dbus_vlan_config_assoc_subnet(connection,message,user_data);
		}/*subnet_base_vlan_config.*/
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_SHOW_ASSOC_SUBNET)) {

			reply = npd_dbus_vlan_show_assoc_subnet(connection,message,user_data);
		}/*subnet_base_vlan_show.*/
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_SUBNET)) {
		    reply = npd_dbus_vlan_config_netif_subnet(connection,message,user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_MAC)) {
			reply = npd_dbus_vlan_config_netif_mac(connection, message, user_data);
        }
#ifdef HAVE_QINQ		
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_MODE)) {
			reply = npd_dbus_vlan_config_netif_qinq(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_MISS_DROP)) {
			reply = npd_dbus_vlan_config_netif_qinq_miss(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_MISS_DROP)) {
			reply = npd_dbus_vlan_config_xlate(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_TPID)) {
			reply = npd_dbus_vlan_config_netif_tpid(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_INNER_TPID)) {
			reply = npd_dbus_vlan_config_netif_inner_tpid(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_QINQ_GLOBAL_TPID)) {
			reply = npd_dbus_vlan_config_qinq_global_tpid(connection, message, user_data);
        }		
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_XLATE)) {
			reply = npd_dbus_vlan_config_xlate(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_ELINE)) {
			reply = npd_dbus_vlan_config_eline(connection, message, user_data);
        }
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_ELINE)) {
			reply = npd_dbus_vlan_config_netif_eline(connection, message, user_data);
        }
#endif		
        else if(dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,   \
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MODE)) {

			reply = npd_dbus_vlan_config_switchport_mode(connection,message,user_data);
		}/*subnet_base_vlan_config.*/
        
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL)) {
			reply = npd_dbus_vlan_config_port_add_del(connection, message, user_data);
		}/*add or del vlan port member */
		else if (dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_GET_VLAN_BASE_INFO))
		{
			reply = npd_dbus_vlan_get_base_info(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_GET_VLAN_VNAME_BASE_INFO)) {
			reply = npd_dbus_vlan_get_vname_base_info(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY)) {
			reply = npd_dbus_vlan_delete_vlan_entry_one(connection,message,user_data);
		}/*delete vlan */
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_DELETE_VLAN_ENTRY_VIA_NAME)) {
			reply = npd_dbus_vlan_delete_vlan_entry_vname(connection,message,user_data);
		}/*delete vlan by vlanname*/
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_SHOW_ONE_PORT_PVID)) {
			reply = npd_dbus_vlan_show_one_port_pvid(connection,message,user_data);
		}/*show one port pvid */
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_SHOW_RUNNING_CONFIG)) {
			reply = npd_dbus_vlan_show_running_config(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_EGRESS_FILTER_SHOW_RUNNING_CONFIG)) {
			reply = npd_dbus_vlan_egress_filter_show_running_config(connection,message,user_data);
		}		
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_ETHER_ADD_DEL)) {
			reply = npd_dbus_vlan_config_protovlan(connection,message,user_data);
		} 
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_PORT_APPLY)) {
			reply = npd_dbus_vlan_config_protovlan_port(connection,message,user_data);
		} 
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_CONFIG_EGRESS_FILTER)) {
			reply = npd_dbus_vlan_config_vlan_egress_filter(connection,message,user_data);
		}		
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,  \
										NPD_DBUS_VLAN_METHOD_CONFIG_FILTER)) {
			reply = npd_dbus_vlan_config_vlan_filter(connection,message,user_data);
		} 
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_GET_VLAN_MEMBERS)) {
			reply = npd_dbus_vlan_get_member(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_NEXT_VLAN_INDEX)) {
			reply = npd_dbus_vlan_get_next_vid(connection, message, user_data);
		}		
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
									NPD_DBUS_VLAN_METHOD_GET_PRIVLAN_MEMBERS)) {
			reply = npd_dbus_privlan_get_member(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_NEXT_PRIVLAN_INDEX)) {
			reply = npd_dbus_privlan_get_next_vid(connection, message, user_data);
		}	
#ifdef HAVE_PORT_ISOLATE		
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
									NPD_DBUS_VLAN_METHOD_GET_ISOLATE_GROPU_MEMBERS)) {
			reply = npd_dbus_isolate_group_get_member(connection, message, user_data);
		}			
#endif			
#ifdef HAVE_DOT1X
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_CHECK_PORT_VLAN)) {
			reply = npd_dbus_vlan_check_port_vlan(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_SET_PORT_AUTH_FAIL_VLAN)) {
			reply = npd_dbus_vlan_set_port_auth_fail_vlan(connection, message, user_data);
		}
#endif

#ifdef HAVE_PRIVATE_VLAN
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_CONFIG_PVLAN_PRIMARY)){
			reply = npd_dbus_pvlan_primary_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_CONFIG_PVLAN_ASSOC)){
			reply = npd_dbus_pvlan_primary_associate(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_CONFIG_PVLAN_PROMICUOUS)){
			reply = npd_dbus_pvlan_promis_port_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_CONFIG_PVLAN_ISOLATE)){
			reply = npd_dbus_pvlan_isolate_port_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_GET_NEXT_PVLAN_INFO)){
			reply = npd_dbus_pvlan_get_next_info(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_GET_PVLAN_MEMBER)){
			reply = npd_dbus_pvlan_get_member(connection, message, user_data);
		}		
		else if (dbus_message_is_method_call(message,NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_GET_NEXT_PVLAN_MAP)){
			reply = npd_dbus_pvlan_get_next_map(connection, message, user_data);
		}
		
#endif
	}  
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_TRUNK_OBJPATH) == 0){
			syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_TRUNK_OBJPATH);
		if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE)) {
			reply = npd_dbus_trunk_create_one(connection,message,user_data);
		}/*create trunk one*/
		else if (dbus_message_is_method_call(message, NPD_DBUS_TRUNK_INTERFACE, \
			                    NPD_DBUS_TRUNK_METHOD_CHECK_TRUNK_EXIST)){
			reply = npd_dbus_check_trunk_exist(connection, message, user_data);
		}
#ifdef HAVE_LACP
        else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_AGGREGATOR_METHOD_MODE_CHANGE)) {
			reply = npd_dbus_aggregator_mode_change(connection,message,user_data);
		}/*change  LAG  mode*/
		else if (dbus_message_is_method_call(message, NPD_DBUS_TRUNK_INTERFACE, \
								NPD_DBUS_METHOD_SHOW_LACP_RUNNING_CONFIG)){
			reply = npd_dbus_show_lacp_running_config(connection,message,user_data);
		}
        else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_AGGREGATOR_GET_BY_TRUNKID)) {
			reply = npd_dbus_aggregator_show_by_trunkId(connection,message,user_data);
		}/*get agg information*/
#endif
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_CONFIG_ONE)) {
			reply = npd_dbus_trunk_config_one(connection,message,user_data);
		}/*config trunk Id*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME)) {
			reply = npd_dbus_trunk_config_by_name(connection,message,user_data);
		}/*config trunk name*/
        else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
			     NPD_DBUS_TRUNK_METHOD_CONFIG_UPDATE_TRUNKNAME)) {
			reply = npd_dbus_trunk_config_update_name(connection,message,user_data);
								
        }
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL)) {
			reply = npd_dbus_trunk_add_delete_port_member(connection,message,user_data);
		}/*trunk add delete port*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG)) {
			reply = npd_dbus_trunk_master_port_set(connection,message,user_data);
		}/*master port config */
#if 0        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE)) {
			reply = npd_dbus_trunk_port_endis(connection,message,user_data);
		}/*rtunk port enable/disable */
#endif        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE)) {
			reply = npd_dbus_trunk_load_banlc_config(connection,message,user_data);
		}/*rtunk port enable/disable */ 
#if 0        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST)) {
			reply = npd_dbus_trunk_allow_refuse_vlan(connection,message,user_data);
		}/*allow refuse vlan*/
#endif        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY)) {
			reply = npd_dbus_trunk_delete_one(connection,message,user_data);
		}/*delete trunk Id*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME)) {
			reply = npd_dbus_trunk_delete_by_name(connection,message,user_data);
		} /*delete trunk name*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1)) {
			reply = npd_dbus_trunk_show_one_v1(connection,message,user_data);
		} /*show trunk one*/
#if 0        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME)) {
			reply = npd_dbus_trunk_show_by_name(connection,message,user_data);
		} /*show trunk by name*/
#endif        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME_V1)) {
			reply = npd_dbus_trunk_show_by_name_v1(connection,message,user_data);
		} /*show trunk by name*/
        else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1)) {
			reply = npd_dbus_trunk_show_trunklist_port_member_v1(connection,message,user_data);
		}
#if 0        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS)) {
			reply = npd_dbus_trunk_show_trunklist_port_member(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1)) {
			reply = npd_dbus_trunk_show_trunklist_port_member_v1(connection,message,user_data);
		}
       
        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,	\
								NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION)) {
			reply = npd_dbus_trunk_show_vlanlist(connection,message,user_data);
		}
#endif         
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,  \
										NPD_DBUS_TRUNK_METHOD_SHOW_RUNNING_CONFIG)) {
			reply = npd_dbus_trunk_show_running_config(connection,message,user_data);
		} /*show trunk list*/
#if 0        
		else if(dbus_message_is_method_call(message,NPD_DBUS_TRUNK_INTERFACE,  \
										NPD_DBUS_TRUNK_METHOD_CLEAR_TRUNK_ARP)) {
			reply = npd_dbus_trunk_clear_arp(connection,message,user_data);
		}
#endif        
	}

	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_FDB_OBJPATH) == 0) {
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_FDB_OBJPATH"\n");

		if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME)) {
			reply = npd_dbus_fdb_config_agingtime(connection,message,user_data);
		}/*config fdb agingtime*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN )) {
			reply = npd_dbus_fdb_delete_fdb_with_vlan(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT)) {
			reply = npd_dbus_fdb_delete_fdb_with_port(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN )) {
			reply = npd_dbus_fdb_delete_static_fdb_with_vlan(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT)) {
			reply = npd_dbus_fdb_delete_static_fdb_with_port(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK)) {
			reply = npd_dbus_fdb_delete_fdb_with_trunk(connection,message,user_data);
		}
            else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_TRUNK)) {
			reply = npd_dbus_fdb_static_delete_fdb_with_trunk(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME)) {
			reply = npd_dbus_fdb_no_config_agingtime(connection,message,user_data);
		}/*config fdb no agingtime*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP)) {
			reply = npd_dbus_fdb_delete_blacklist(connection,message,user_data);
		}/*config fdb mac_vlan_port*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME)) {
			reply = npd_dbus_fdb_delete_blacklist_with_name(connection,message,user_data);
		}/*conf*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP)) {
			reply = npd_dbus_fdb_create_blacklist(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME)) {
			reply = npd_dbus_fdb_create_blacklist_with_vlanname(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME)) {
			reply = npd_dbus_fdb_show_agingtime(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC)) {
			reply = npd_dbus_fdb_config_static_fdb_item(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC)) {
			reply = npd_dbus_fdb_config_static_fdb_trunk_item(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_WITH_NAME)) {
			reply = npd_dbus_fdb_config_static_fdb_with_name(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_TRUNK_WITH_NAME)) {
			reply = npd_dbus_fdb_config_static_fdb_trunk_with_name(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC)) {
			reply =  npd_dbus_fdb_delete_static_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC_WITH_NAME)) {
			reply = npd_dbus_fdb_delete_static_fdb_with_name(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE)) {
			reply = npd_dbus_fdb_show_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_SINGLE_UNIT_TABLE)) {
			reply = npd_dbus_fdb_show_single_unit_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE)) {
			reply = npd_dbus_fdb_show_dynamic_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE)) {
			reply = npd_dbus_fdb_show_static_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE)){
			reply = npd_dbus_fdb_show_blacklist_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT)) {
			reply = npd_dbus_fdb_show_fdb_port(connection,message,user_data);
			}	
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN)) {
			reply = npd_dbus_fdb_show_fdb_vlan(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME)) {
			reply = npd_dbus_fdb_show_fdb_vlan_with_name(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE)) {
			reply = npd_dbus_fdb_show_fdb_one(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC)) {
			reply = npd_dbus_fdb_show_fdb_mac(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT)) {
			reply = npd_dbus_fdb_show_fdb_count(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,	\
								NPD_DBUS_STATIC_FDB_METHOD_SHOW_RUNNING_CONFIG)) {
			reply = npd_dbus_fdb_show_running_config(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_PORT)){
			reply = npd_dbus_fdb_config_port_number(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN)){
			reply = npd_dbus_fdb_config_vlan_number(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN_PORT)){
			reply = npd_dbus_fdb_config_vlan_port_number(connection,message,user_data);
		}
	
	}
#ifdef HAVE_IGMP_SNP
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_IGMPSNP_OBJPATH) == 0) {
		if (dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_CHECK_VLAN_IGMP_SNP_STATUS)) {																			
			reply = npd_dbus_igmpsnp_check_vlan_igmp_snoop(connection,message,user_data);
		} 
		else if (dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_IGMP_SNP_VLAN_COUNT)) {																		
			reply = npd_dbus_igmpsnp_vlan_count(connection,message,user_data);
		} 
		else if (dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_CONFIG_IGMP_SNP_VLAN)) {																			
			reply = npd_dbus_igmpsnp_config_vlan(connection,message,user_data);
		} 
		else if (dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_IGMP_SNP_VLAN_LIST_SHOW)) {										
			reply = npd_dbus_igmpsnp_vlan_list_show(connection,message,user_data);
		} 
		else if (dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_CONFIG_ETHPORT)) {
			reply = npd_dbus_igmpsnp_portmbr_config(connection,message,user_data);
		} 
				
		else if(dbus_message_is_method_call(message,NPD_DBUS_IGMPSNP_INTERFACE,  \
										NPD_DBUS_IGMPSNP_METHOD_SHOW_VLAN_RUNNING_CONFIG)) {
			reply = npd_dbus_igmpsnp_show_vlan_running_config(connection,message,user_data);
		}
	}
#endif
#ifdef HAVE_MLD_SNP
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_MLDSNP_OBJPATH) == 0) {
		if (dbus_message_is_method_call(message,NPD_DBUS_MLDSNP_INTERFACE,  \
										NPD_DBUS_MLDSNP_METHOD_SET_VLAN_ENABLE)) {																			
			reply = npd_dbus_mldsnp_config_vlan(connection,message,user_data);
		} 
    }
#endif
#ifdef HAVE_MIRROR
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_MIRROR_OBJPATH) == 0) {	/*mirror*/
		if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_CONFIG_MIRROR)) {
			reply = npd_dbus_mirror_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_CONFIG_NO_MIRROR)) {
			reply = npd_dbus_mirror_config(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_MIRROR_DEST_PORT_CREATE)) {
			reply = npd_dbus_mirror_create_dest_port(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_MIRROR_DEST_PORT_DEL)) {
			reply = npd_dbus_mirror_del_dest_port(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL)) {
			reply = npd_dbus_mirror_create_source_acl(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_ACL)) {
			reply = npd_dbus_mirror_delete_source_acl(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_CREATE)) {
			reply = npd_dbus_mirror_create_source_port(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_DEL)) {
			reply = npd_dbus_mirror_del_source_port(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE)) {
			reply = npd_dbus_mirror_create_source_vlan(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL)) {
			reply = npd_dbus_mirror_del_source_vlan(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_BASE_FDB)) {
			reply = npd_dbus_mirror_fdb_mac_vlanid_port_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_FDB)) {
			reply = npd_dbus_mirror_fdb_mac_vlanid_port_cancel(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_REMOTE_VLAN_CREATE)) {
			reply = npd_dbus_mirror_create_remote_vlan(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_APPEND_MIRROR_REMOTE_VLAN_DEL)) {
			reply = npd_dbus_mirror_del_remote_vlan(connection,message,user_data);
		}		
		else if (dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_MIRROR_SHOW)) {
			reply = npd_dbus_mirror_show(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_MIRROR_SHOW_FDB)) {
			reply = npd_dbus_mirror_show_fdb(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_MIRROR_INTERFACE,NPD_DBUS_METHOD_SHOW_RUNNING_CGF)) {
			reply = npd_dbus_mirror_show_running_cfg(connection,message,user_data);
		}
	}
#endif
#ifdef HAVE_QOS
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_QOS_OBJPATH) == 0){
		/*QOS*/
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_QOS_OBJPATH);
		if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_QOS_PROFILE)) {
			reply = npd_dbus_config_qos_profile(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_QOS_PROFILE_ATTRIBUTE)) {
			reply =  npd_dbus_qos_profile_attributes(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DEFAULT_PROFILE_TABLE)) {
			reply =  npd_dbus_defaule_profile_to_qos_profile_table(connection,message,user_data);
		}
	    else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_DEFAULT_PROFILE_TABLE)) {
			reply =  npd_dbus_delete_default_qos_profile_table(connection,message,user_data);
		}
	    else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_PORT_QOS_REMARK)) {
			reply =  npd_dbus_qos_port_eg_remark(connection,message,user_data);
		}
	    else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_DSCP_PROFILE_TABLE)) {
			reply =  npd_dbus_dscp_to_qos_profile_table(connection,message,user_data);
		}
	   else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_DSCP_DSCP_TABLE)) {
			reply =  npd_dbus_dscp_to_dscp_table(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_UP_PROFILE_TABLE)) {
			reply =  npd_dbus_up_to_qos_profile(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_PROFILE_PROFILE_TABLE)) {
			reply =  npd_dbus_qos_profile_to_qos_profile(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_DSCP_PROFILE_TABLE)) {
			reply =  npd_dbus_delete_dscp_to_qos_profile_table(connection,message,user_data);
		}
#if 0
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_ACL_PROFILE_TABLE)) {
			reply =  npd_dbus_delete_acl_to_qos_profile_table(connection,message,user_data);
		}
#endif
	   else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_DSCP_DSCP_TABLE)) {
			reply =  npd_dbus_delete_dscp_to_dscp_table(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_UP_PROFILE_TABLE)) {
			reply =  npd_dbus_delete_up_to_qos_profile(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_POLICY_MAP)) {
			reply =  npd_dbus_config_policy_map(connection,message,user_data);
		}
		#if 0
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_DEFAULT_UP)) {
			reply =  npd_dbus_policy_default_up(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_DEFAULT_QOS_PROFILE)) {
			reply =  npd_dbus_policy_default_qos_profile(connection,message,user_data);
		}
		#endif
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_PORT_TRUST_L2_MODE)) 
        {
			reply =  npd_dbus_policy_trust_mode_l2_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_PORT_TRUST_L3DSCP_MODE)) 
        {
			reply =  npd_dbus_policy_trust_mode_l3dscp_set(connection,message,user_data);
		}
#if 0
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_PORT_TRUST_L2_L3_MODE)) {
			reply =  npd_dbus_policy_trust_mode_l2_l3_set(connection,message,user_data);
		}
#endif
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE, NPD_DBUS_QOS_CONFIG_FLOW_CONTROL)) {
			reply =  npd_dbus_flow_control_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE, NPD_DBUS_QOS_SHOW_FLOW_CONTROL)) {
			reply =  npd_dbus_show_flow_control(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_MODIFY_MARK_QOS)) {
			reply =  npd_dbus_policy_modify_qos(connection,message,user_data);
		}
#if 0
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_QOS_INGRESS_POLICY_BASE_ON_ACL))
		{
			reply = npd_dbus_config_qos_base_acl_ingress(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SET_QOS_EGRESS_POLICY_BASE_ON_ACL))
		{
			reply = npd_dbus_config_qos_base_acl_egress(connection,message,user_data);
		}
#endif
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_POLICY_MAP))
		{
			reply = npd_dbus_show_policy_map(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_QOS_PROFILE))
		{
			reply = npd_dbus_show_qos_profile(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_POLICER))
		{
			reply = npd_dbus_policer_set(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_POLICER_RANGE))
		{
			reply = npd_dbus_policer_set_range(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_CIR_CBS))
		{
			reply = npd_dbus_policer_cir_cbs(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_CONFIG_PACKETCOLOR))
		{
			reply = npd_dbus_policer_packetcolor(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_OUT_PROFILE_DROP_KEEP))
		{
			reply = npd_dbus_policer_out_profile_cmd_keep_drop(connection,message,user_data);
		}
#if 0
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_POLICER_ENABLE))
		{
			reply = npd_dbus_policer_enable(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_GLOBAL_METER_MODE))
		{
			reply = npd_dbus_global_policer_meter_mode(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_GLOBAL_PACKET_SIZE))
		{
			reply = npd_dbus_global_policing_mode(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_POLICER_COUNTER))
		{
			reply = npd_dbus_policer_counter(connection,message,user_data);
		}
#endif
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_POLICER_ENABLE))
		{
			reply = npd_dbus_policer_counter(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_POLICER))
		{
			reply = npd_dbus_show_policer(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_GET_POLICER))
		{
			reply = npd_dbus_get_policer(connection,message,user_data);
		}
#if 0
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_POLICER_COLOR))
		{
			reply = npd_dbus_policer_color(connection,message,user_data);
		}
#endif
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_QOS_PROFILE))
		{
			reply = npd_dbus_delete_qos_profile(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_POLICY_MAP))
		{
			reply = npd_dbus_delete_policy_map(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_POLICER))
		{
			reply = npd_dbus_delete_policer(connection,message,user_data);
		}
#if 0
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_DELETE_POLICER_RANGE))
		{
			reply = npd_dbus_delete_policer_range(connection,message,user_data);
		}
#endif
		else if(dbus_message_is_method_call(message, NPD_DBUS_QOS_INTERFACE, NPD_DBUS_METHOD_SHOW_QOS_RUNNIG_CONFIG))
		{
			reply = npd_dbus_qos_show_running_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_QUEQUE_SCH))
		{
			reply = npd_dbus_queue_scheduler(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_QUEQUE_WRR_GROUP))
		{
			reply = npd_dbus_queue_sche_wrr_group(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_QUEUE_DROP))
		{
			reply = npd_dbus_queue_drop(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_QOS_INTERFACE, NPD_DBUS_METHOD_DEFAULT_QUEQUE))
		{
			reply = npd_dbus_queue_def_sche(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_QUEUE))
		{
			reply = npd_dbus_show_queue_scheduler(connection,message,user_data);
		}
#if 0
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_APPEND_QOS_MARK_BASE_ACL))
		{
			reply = npd_dbus_qos_acl_apend(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_SHOW_APPEND_QOS_MARK_BASE_ACL))
		{
			reply = npd_dbus_show_qos_acl_apend(connection,message,user_data);
		}
#endif
	}
#endif
#if defined(HAVE_VRRP) || defined(HAVE_SMARTLINK)
    else if (strcmp(dbus_message_get_path(message), NPD_DBUS_TRACK_OBJPATH) == 0)
    {
		/* TRACK */
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_TRACK_OBJPATH);
		if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP))
		{
			reply = npd_dbus_tracking_group(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_DELETE))
		{
			reply = npd_dbus_tracking_group_delete(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_ADD_OBJECT))
		{
			reply = npd_dbus_tracking_group_add_object(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_REMOVE_OBJECT))
		{
			reply = npd_dbus_tracking_group_remove_object(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_ACTION_DOWN))
		{
			reply = npd_dbus_tracking_group_action_down(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_MODE))
		{
			reply = npd_dbus_tracking_group_mode(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_OBJECT_L3EXIST))
		{
			reply = npd_dbus_tracking_object_l3exist(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_OBJECT_L2EXIST))
		{
			reply = npd_dbus_tracking_object_l2exist(connection, message, user_data);
		}

		else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_GET))
		{
			reply = npd_dbus_tracking_group_query(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_GET_NEXT))
		{
			reply = npd_dbus_tracking_group_query_next(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_SHOW_RUNNING))
		{
			reply = npd_dbus_tracking_group_show_running(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_TRACK_INTERFACE, NPD_DBUS_METHOD_TRACK_GROUP_STATUS))
		{
			reply = npd_dbus_tracking_group_status(connection, message, user_data);
		}
    }
#ifdef HAVE_SMART_LINK
    else if (strcmp(dbus_message_get_path(message), NPD_DBUS_SMART_LINK_OBJPATH) == 0)
    {
        if (dbus_message_is_method_call(message, NPD_DBUS_SMART_LINK_INTERFACE, NPD_DBUS_SMART_LINK_INFORMATION))
		{
			reply = npd_dbus_smart_link_netif_status_get(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_SMART_LINK_INTERFACE, NPD_DBUS_SMART_LINK_ADV_VLAN_LIST))
		{
			reply = npd_dbus_smart_link_adv_vlan_list(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_SMART_LINK_INTERFACE, NPD_DBUS_SMART_LINK_ADV_VLAN_LIST_GET))
		{
			reply = npd_dbus_smart_link_adv_vlan_list_get(connection, message, user_data);
		}
    }
#endif
#endif
	else if (strcmp(dbus_message_get_path(message),NPD_DBUS_ACL_OBJPATH) == 0) 
    {
		if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_ACL_RULE_SHOW_CONFIG)) 
		{
			reply =  npd_dbus_acl_rule_show_config(connection,message,user_data);
		}
		/*time*/
#if 0
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_RULE_IP_RANGE)) {
			reply = npd_dbus_config_ip_range(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_SET_TIME_RANGE)) 
		{
			reply =  npd_dbus_time_range_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_SET_ABSOLUTE)) 
		{
			reply =  npd_dbus_time_absolute_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_SET_PERIODIC)) 
		{
			reply =  npd_dbus_time_periodic_set(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_SHOW_TIME_RANGE)) 
		{
			reply =  npd_dbus_time_range_show(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_TIME_RANGE)) 
		{
			reply =  npd_dbus_time_range_acl(connection,message,user_data);
		}
#endif
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_CONFIG)) 
		{
			reply =  npd_dbus_create_acl_group(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_NO_ACL_GROUP_CONFIG)) 
		{
			reply =  npd_dbus_delete_acl_group(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_ADD_DESP)) 
		{
			reply =  npd_dbus_acl_group_add_desp(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_DELETE_DESP)) 
		{
			reply =  npd_dbus_acl_group_delete_desp(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_ADD_RULE)) 
		{
			reply =  npd_dbus_acl_group_add_rule(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_DELETE_RULE)) 
		{
			reply =  npd_dbus_acl_group_delete_rule(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_DEPLOY)) 
		{
			reply =  npd_dbus_acl_group_deploy(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_UNDEPLOY)) 
		{
			reply =  npd_dbus_acl_group_undeploy(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_UNDEPLOY_ALL)) 
		{
			reply =  npd_dbus_acl_group_undeploy_all(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_SHOW_DETAIL)) 
		{
			reply =  npd_dbus_acl_group_show_detail(connection,message,user_data);
		}        
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_INTF_SHOW)) 
		{
			reply =  npd_dbus_acl_group_intf_show(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_ACL_GROUP_SHOW_ALL)) 
		{
			reply =  npd_dbus_acl_group_show_all(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_CLASS_MAP_MATCH_ALL)) 
		{
			reply =  npd_dbus_create_class_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_NO_CLASS_MAP)) 
		{
			reply =  npd_dbus_no_class_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_CLASS_MAP_RENAME)) 
		{
			reply =  npd_dbus_rename_class_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_ACL_INTERFACE, NPD_DBUS_ACL_METHOD_MATCH_ADD)) 
		{
			reply =  npd_dbus_class_map_add_match(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message, NPD_DBUS_ACL_INTERFACE, NPD_DBUS_ACL_METHOD_MATCH_L4PORT_RANGE))
        {
            reply = npd_dbus_match_l4port_range(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message, NPD_DBUS_ACL_INTERFACE, NPD_DBUS_ACL_POLICY_ROUTE_SUPPORT))
        {
            reply = npd_dbus_check_policy_route_support(connection,message,user_data);
        }
		else if (dbus_message_is_method_call(message, NPD_DBUS_ACL_INTERFACE, NPD_DBUS_ACL_METHOD_MATCH_DELETE)) 
		{
			reply =  npd_dbus_class_map_delete_match(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_POLICY_MAP_IN)) 
		{
			reply =  npd_dbus_create_policy_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_NO_POLICY_MAP)) 
		{
			reply =  npd_dbus_no_policy_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_POLICY_MAP_RENAME)) 
		{
			reply =  npd_dbus_rename_policy_map(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_POLICY_ACTION)) 
		{
			reply =  npd_dbus_policy_map_add_action(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_ACL_METHOD_DELETE_POLICY_ACTION)) 
		{
			reply =  npd_dbus_policy_map_delete_action(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_CONFIG_CLASS)) 
		{
			reply =  npd_dbus_policy_map_class(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_CONFIG_NO_CLASS)) 
		{
			reply =  npd_dbus_policy_map_no_class(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_SERVICE_POLICY_IN)) 
		{
			reply =  npd_dbus_service_policy_in(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_METHOD_NO_SERVICE_POLICY_IN)) 
		{
			reply =  npd_dbus_service_no_policy(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_VMAP_NAME)) 
		{
			reply =  npd_dbus_acl_show_vmap_name(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_CMAP_LIST)) 
		{
			reply =  npd_dbus_acl_show_cmap_list(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_PMAP_LIST)) 
		{
			reply =  npd_dbus_acl_show_pmap_list(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_CMAP_NAME)) 
		{
			reply =  npd_dbus_acl_show_cmap_name(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_PMAP_NAME)) 
		{
			reply =  npd_dbus_acl_show_pmap_name(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_SERVICE_POLICY)) 
		{
			reply =  npd_dbus_acl_show_service_policy(connection,message,user_data);
		}
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_TIME_RANGE_NAME))
        {
            reply =  npd_dbus_time_range_info_search_name(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_TIME_RANGE_ARG))
        {
            reply =  npd_dbus_time_range_info_create(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_ASSOCIATE_TIME_RANGE))
        {
            reply =  npd_dbus_is_acl_associate_time_range_info(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_NO_TIME_RANGE_NAME))
        {
            reply =  npd_dbus_no_time_range_name(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_NO_ACL_TIME_RANGE_ASSOCIATE))
        {
            reply =  npd_dbus_no_acl_time_range_associate(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_TIME_RANGE))
        {
            reply =  npd_dbus_acl_show_time_range(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_TIME_RANGE_NAME))
        {
            reply =  npd_dbus_acl_show_time_range_name(connection,message,user_data);
        }
        else if (dbus_message_is_method_call(message,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_ACL_SHOW_TIME_RANGE_BIND))
        {
            reply = npd_dbus_acl_show_time_range_bind(connection,message,user_data);
        }
	}
	else if(strcmp(dbus_message_get_path(message),NPD_DBUS_INTF_OBJPATH) == 0) {
		/*config interface*/
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_INTF_OBJPATH"\n");
		if (dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_CREATE_INTF)) 
		{
			reply = npd_dbus_create_l3intf(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_DELETE_INTF))
		{
			reply = npd_dbus_delete_l3intf(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_SHUTDOWN_SET))
		{
			reply = npd_dbus_shutdown_set(connection,message,user_data);
		}
#ifndef HAVE_ZEBRA
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_ADD_IPV4_ADDR))
		{
			reply = npd_dbus_add_ipv4_addr(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_DEL_IPV4_ADDR))
		{
			reply = npd_dbus_del_ipv4_addr(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_INTF_SHOW_RUN))
		{
			reply = npd_dbus_intf_showrun(connection,message,user_data);
		}
#endif
#ifdef HAVE_PORTAL
        else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_SET_PORTAL_SERVER))
		{
			reply = npd_dbus_set_portal_server(connection,message,user_data);
		}
        else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_SHOW_PORTAL_INTERFACE))
		{
			reply = npd_dbus_show_portal_interface(connection,message,user_data);
		}        
        else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_PORTAL_SHOW_RUN))
		{
			reply = npd_dbus_intf_portal_server_showrun(connection,message,user_data);
		}
#endif
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_VLAN_INTF_CHECK))
		{
			reply = npd_dbus_vlan_intf_check(connection, message, user_data);
		}
#ifdef HAVE_ROUTE        
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_INTF_URPF_SET))
		{
			reply = npd_dbus_intf_urpf_set(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_CONF_URPF_SET))
		{
			reply = config_ucrpf_enable(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_CONF_URPF_GET))
		{
			reply = show_ucrpf_enable(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_CONF_IPMC_SET))
		{
			reply = npd_dbus_intf_ipmc_enable(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_URPF_METHOD_SHOW_RUNNING_CONFIG))
		{
            reply = npd_dbus_urpf_show_running(connection,message,user_data);
		}
#endif 
#ifdef HAVE_ZEBRA
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_INTF_PROXY_ARP_SET))
		{
			reply = npd_dbus_intf_proxy_arp_set(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_PROXY_ARP_METHOD_SHOW_RUNNING_CONFIG))
		{
			reply = npd_dbus_intf_proxy_arp_show_running(connection,message,user_data);
		}
#endif        
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_L3INTF_GET_NEXT))
		{
			reply = npd_dbus_l3intf_get_next(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_L3INTF_GET))
		{
			reply = npd_dbus_l3intf_get(connection, message, user_data);
		}

		if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_IP_STATIC_ARP))
		{
            reply = npd_dbus_ip_static_arp(connection,message,user_data);
		}	
#ifdef HAVE_DHCP_SNP   
        if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_SOUCE_GUARD_SERVICE))
		{
			reply = npd_dbus_interface_sg_service(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_SOUCE_GUARD_ENTRY))
		{
            reply = npd_dbus_source_guard_entry(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_SOUCE_GUARD_SHOW_PORT))
		{
            reply = npd_dbus_ip_sg_port_show_enable(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_SOUCE_GUARD_SHOW_ALL))
		{
            reply = npd_dbus_source_guard_entry_show_all(connection,message,user_data);
		}
        else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_SOUCE_GUARD_SHOWRUNNING))
		{
            reply = npd_dbus_source_guard_entry_showrunning(connection,message,user_data);
		}	
#endif        
#ifdef HAVE_NPD_IPV6
		if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_IPV6_STATIC_NEIGH))
		{
            reply = npd_dbus_ipv6_static_neigh(connection,message,user_data);
		}		
#endif //HAVE_NPD_IPV6		
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP))
		{
            reply = npd_dbus_no_ip_static_arp(connection,message,user_data);
		}
#ifdef HAVE_NPD_IPV6	
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTERFACE_NO_IP_STATIC_NEIGH))
		{
            reply = npd_dbus_no_ip_static_neigh(connection,message,user_data);
		}
#endif //HAVE_NPD_IPV6		
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_SHOW_ARP_SPECIFY))
		{
			reply = npd_dbus_show_ip_arp_info(connection, message, user_data);
		}
#ifdef HAVE_NPD_IPV6
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_SHOW_NEIGH_SPECIFY))
		{
			reply = npd_dbus_show_ip_neigh_info(connection, message, user_data);
		}
#endif //HAVE_NPD_IPV6		
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_CLEAR_ARP_FOR_IFINDEX))
		{
			reply = npd_dbus_clear_arp(connection, message, user_data);
		}
#ifdef HAVE_NPD_IPV6
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_CLEAR_NEIGHBOUR_FOR_IFINDEX))
		{
			reply = npd_dbus_clear_neigh(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_NEIGH_METHOD_SHOW_RUNNING_CONFIG))
		{
			reply = npd_dbus_static_ndisc_show_running(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_SET_NDP_AGETIME))
		{
			reply = npd_dbus_ip_set_ndisc_agetime(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, NPD_DBUS_INTF_INTERFACE, NPD_DBUS_INTF_METHOD_SHOW_NDP_AGETIME))
		{
			reply = npd_dbus_ip_show_ndisc_agetime(connection, message, user_data);
		}
#endif //HAVE_NPD_IPV6		
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_DYNTOSTATIC_ARP))
		{
            reply = npd_dbus_ip_dyntostatic_arp(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_SET_ARP_AGETIME))
		{
            reply = npd_dbus_ip_set_arp_agetime(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_SHOW_ARP_AGETIME))
		{
            reply = npd_dbus_ip_show_arp_agetime(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_SET_ARP_DROP))
		{
            reply = npd_dbus_ip_set_arp_drop(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_SHOW_ARP_DROP))
		{
            reply = npd_dbus_ip_show_arp_drop(connection,message,user_data);
		}
		
		/*
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_VLAN_ETH_PORT_INTERFACE_ADVANCED_ROUTING_SHOW))
		{
            reply = npd_dbus_vlan_eth_port_interface_advanced_routing_show(connection,message,user_data);
		}
		*/
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_STATIC_ARP_METHOD_SHOW_RUNNING_CONFIG))
		{
            reply = npd_dbus_static_arp_show_running(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_ARP_INSPECTION_METHOD_SHOW_RUNNING_CONFIG))
		{
            reply = npd_dbus_arp_inspection_show_running(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_VLAN_ARP_INSPECTION))
		{
			reply = npd_dbus_arp_inspection_vlan_enable(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION))
		{
			reply = npd_dbus_arp_inspection_enable(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_TRUST))
		{
			reply = npd_dbus_arp_inspection_trust_set(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_VALIDATE_TYPE))
		{
			reply = npd_dbus_arp_inspection_validate_set(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_GLOBAL))
		{
			reply = npd_dbus_arp_inspection_check_global(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_VLAN))
		{
			reply = npd_dbus_arp_inspection_check_vlan(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_VLAN_BY_VID))
		{
			reply = npd_dbus_arp_inspection_check_vlan_by_vid(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_TRUST))
		{
			reply = npd_dbus_arp_inspection_check_trust(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_STATISTICS))
		{
			reply = npd_dbus_arp_inspection_satistics(connection,message,user_data);
		}
        else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ARP_INSPECTION_CLEAR))
		{
			reply = npd_dbus_arp_inspection_clear_statistics(connection,message,user_data);
		}

		/*
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG))
		{
            reply = npd_dbus_interface_advanced_routing_show_running(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_CONFIG_ADVANCED_ROUTING_DEFAULT_VID))
		{
			reply = npd_dbus_interface_config_advanced_routing_default_vid(connection,message,user_data);
		}
		else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_SHOW_ADVANCED_ROUTING_DEFAULT_VID))
		{
			reply = npd_dbus_interface_show_advanced_routing_default_vid(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_SUBIF_SET_QINQ_TYPE))
		{
			reply = npd_dbus_subif_set_qinq_type(connection,message,user_data);
		}else if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_SET_QINQ_TYPE_SAVE_CFG))
		{
			reply = npd_dbus_subif_qinq_type_save_cfg(connection,message,user_data);
		}
		*/
#if 0
		if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_CREATE_PORT_INTF))
		{

			reply = npd_dbus_create_intf_by_port_index(connection,message,user_data);
		}
		if(dbus_message_is_method_call(message,NPD_DBUS_INTF_INTERFACE,NPD_DBUS_INTF_METHOD_DEL_PORT_INTF))
		{
			reply = npd_dbus_del_intf_by_port_index(connection,message,user_data);
		}
#endif
	}
#ifdef HAVE_ROUTE
	/*added by scx*/
	else if(strcmp(dbus_message_get_path(message),RTDRV_DBUS_OBJPATH)== 0) {
		syslog_ax_dbus_dbg("npd obj path"RTDRV_DBUS_OBJPATH);
		syslog_ax_dbus_dbg("get dbus msg for drv rt\n");
		if (dbus_message_is_method_call(message,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_ALL)) {
			syslog_ax_dbus_dbg("get dbus msg for drv rt RTDRV_DBUS_METHOD_SHOW_ALL\n");
			reply =  show_rtdrv_all(connection,message,user_data);
		}
		if (dbus_message_is_method_call(message,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_ENTRY)) {
			syslog_ax_dbus_dbg("get dbus msg for drv rt RTDRV_DBUS_METHOD_SHOW_ENTRY\n");
			reply =  show_rtdrv_entry(connection,message,user_data);
		}
		if (dbus_message_is_method_call(message,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_CONFIG_RPF)) {
			syslog_ax_dbus_dbg("get dbus msg for drv rt RTDRV_DBUS_METHOD_CONFIG_RPF\n");
			reply =  config_ucrpf_enable(connection,message,user_data);
		}
		if (dbus_message_is_method_call(message,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_RPF)) {
			syslog_ax_dbus_dbg("get dbus msg for drv rt RTDRV_DBUS_METHOD_SHOW_RPF\n");
			reply =  show_ucrpf_enable(connection,message,user_data);
		}
		/*
		if (dbus_message_is_method_call(message,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_STATUES)) {
			syslog_ax_dbus_dbg("get dbus msg for drv rt RTDRV_DBUS_METHOD_SHOW_STATUES\n");
			reply =  show_route_status(connection,message,user_data);
		}
		*/
	}
#endif
#ifdef HAVE_DLDP
	else if(strcmp(dbus_message_get_path(message), NPD_DBUS_DLDP_OBJPATH) == 0) {	/* DLDP */
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_DLDP_OBJPATH"\n");

		if (dbus_message_is_method_call(message,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_CHECK_GLOBAL_STATUS))
		{
			reply = npd_dbus_dldp_check_global_status(connection, message, user_data);
		}
		#if 0
		// base port
		else if (dbus_message_is_method_call(message,
												NPD_DBUS_DLDP_INTERFACE,
												NPD_DBUS_DLDP_METHOD_CONFIG_ETHPORT))
		{
			reply = npd_dbus_dldp_config_ethport(connection, message, user_data);
		} 
		#endif
		/* base vlan*/
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_DLDP_INTERFACE,
											NPD_DBUS_DLDP_METHOD_CONFIG_VLAN))
		{
			reply = npd_dbus_dldp_config_vlan(connection, message, user_data);
		} 
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_DLDP_INTERFACE,
											NPD_DBUS_DLDP_METHOD_GET_VLAN_COUNT))
		{
			reply = npd_dbus_dldp_get_vlan_count(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_DLDP_INTERFACE,
											NPD_DBUS_DLDP_METHOD_GET_PRODUCT_ID))
		{
			reply = npd_dbus_dldp_get_product_id(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_DLDP_INTERFACE,
											NPD_DBUS_DLDP_METHOD_EXCHANGE_IFINDEX_TO_SLOTPORT))
		{
			reply = npd_dbus_dldp_exchange_ifindex_to_slotport(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_DLDP_INTERFACE,
											NPD_DBUS_DLDP_METHOD_VLAN_SHOW_RUNNING_CONFIG))
		{
			reply = npd_dbus_dldp_vlan_show_running_config(connection, message, user_data);
		}
	}
#endif
#ifdef HAVE_DHCP_RELAY
	else if(strcmp(dbus_message_get_path(message), NPD_DBUS_DHCP_RELAY_OBJPATH) == 0 ) {
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_DHCP_RELAY_OBJPATH"\n");
		if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_ADD_IP_HELPER))
		{
			reply = npd_dbus_dhcp_relay_add_iphelper(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_DEL_IP_HELPER))
		{
			reply = npd_dbus_dhcp_relay_del_iphelper(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_SHOW_IP_HELPER))
		{
			reply = npd_dbus_dhcp_relay_show_iphelper(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_GLOBAL_ENABLE))
		{
			reply = npd_dbus_dhcp_relay_enable_global_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_INTF_ENABLE))
		{
			reply = npd_dbus_dhcp_relay_enable_intf_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCP_RELAY_METHOD_SHOW_RUNNING_GLOBAL_CONFIG))
		{
			reply = npd_dbus_dhcp_relay_show_running_global_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_CONFIG_IP_DHCP_RELAY_SHOW_GLOBAL))
		{
			reply = npd_dbus_ip_dhcp_relay_check_global(connection,message,user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCP_RELAY_METHOD_OPT82_ENABLE))
		{
			reply = npd_dbus_dhcp_relay_enable_opt82(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCP_RELAY_METHOD_SHOW_RUNNING_INTF_CONFIG))
		{
			reply = npd_dbus_dhcp_relay_show_running_intf_config(connection, message, user_data);
		}
	}
#endif
#ifdef HAVE_DHCPV6_RELAY
    else if (strcmp(dbus_message_get_path(message), NPD_DBUS_DHCPV6_RELAY_OBJPATH) == 0)
    {
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_DHCPV6_RELAY_OBJPATH"\n");
		if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_GLOBAL))
		{
			reply = npd_dbus_dhcpv6_relay_global(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE))
		{
			reply = npd_dbus_dhcpv6_relay_interface(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_SERVER))
		{
			reply = npd_dbus_dhcpv6_relay_server(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_NO_SERVER))
		{
			reply = npd_dbus_dhcpv6_relay_no_server(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_GLOBAL_SHOW))
		{
			reply = npd_dbus_dhcpv6_relay_global_show(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE_SHOW))
		{
			reply = npd_dbus_dhcpv6_relay_interface_show(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE_SHOW_ONE))
		{
			reply = npd_dbus_dhcpv6_relay_interface_show_one(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_SHOW_RUNNING))
		{
			reply = npd_dbus_dhcpv6_relay_show_running_config(connection, message, user_data);
		}
    }
#endif
#ifdef HAVE_DHCP_SNP
	else if(strcmp(dbus_message_get_path(message), NPD_DBUS_DHCP_SNP_OBJPATH) == 0) {	/* DHCP_Snooping */
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_DHCP_SNP_OBJPATH"\n");
#if 0
		if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_GLOBAL_STATUS))
		{
			reply = npd_dbus_dhcp_snp_check_global_status(connection, message, user_data);
		}
		else 
#endif
        if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_VLAN_SHOW))
		{
			reply = npd_dbus_dhcp_snp_check_vlan_status(connection, message, user_data);
		}

		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_INFORMATION_GLOBAL_GET))
		{
			reply = npd_dbus_dhcp_snp_check_op82_status(connection, message, user_data);
		}

        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_GLOBAL_STATUS_GET))
		{
			reply = npd_dbus_dhcp_snp_check_op82_global_status_get(connection, message, user_data);
		}

		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_INTERFACE_SHOW))
		{
			reply = npd_dbus_dhcp_snp_check_op82_interface_status(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_INFORMATION_PORT_GET))
		{
			reply = npd_dbus_dhcp_snp_op82_port_status_get(connection, message, user_data);
		}


		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_VLAN_SHOW_BY_VID))
		{
			reply = npd_dbus_dhcp_snp_check_vlan_status_by_vid(connection, message, user_data);
		}

		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE))
		{
			reply = npd_dbus_dhcp_snp_enable_global_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE))
		{
			reply = npd_dbus_dhcp_snp_enable_vlan_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE))
		{
			reply = npd_dbus_dhcp_snp_config_port(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE))
		{
			reply = npd_dbus_dhcp_snp_show_bind_table(connection, message, user_data);
		}
#if 0
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE))
		{
			reply = npd_dbus_dhcp_snp_show_static_bind_table(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_VLAN))
		{
			reply = npd_dbus_dhcp_snp_show_static_bind_table_by_vlan(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_ETHPORT))
		{
			reply = npd_dbus_dhcp_snp_show_static_bind_table_by_ethport(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_TRUST_PORTS))
		{
			reply = npd_dbus_dhcp_snp_show_trust_ports(connection, message, user_data);
		}
#endif
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE))
		{
			reply = npd_dbus_dhcp_snp_enable_opt82(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FORMAT_TYPE))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_format_type(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_fill_format_type(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_remoteid_content(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_port_strategy(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_port_circuitid_content(connection, message, user_data);
		}
#if 0
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT))
		{
			reply = npd_dbus_dhcp_snp_set_opt82_port_remoteid_content(connection, message, user_data);
		}
#endif
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_BINDING))
		{
			reply = npd_dbus_dhcp_snp_add_del_binding(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG))
		{
			reply = npd_dbus_dhcp_snp_show_running_global_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_VLAN_CONFIG))
		{
			reply = npd_dbus_dhcp_snp_show_running_vlan_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_SAVE_BIND_TABLE))
		{
			reply = npd_dbus_dhcp_snp_show_running_save_bind_table(connection, message, user_data);
		}
#if 0
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_DHCP_SNP_INTERFACE,  
										NPD_DBUS_DHCP_SNP_METHOD_CONFIG_ETHPORT)) 
		{
			reply = npd_dbus_dhcp_snp_portmbr_config(connection,message,user_data);
		} 
#endif
	}
#endif
#ifdef HAVE_POE
	else if(strcmp(dbus_message_get_path(message), NPD_DBUS_POE_OBJPATH) == 0) {	/* DLDP */
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_POE_OBJPATH"\n");
		if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_GLOBAL_POE_ENDIS)){
			reply = npd_dbus_poe_config_global_endis(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_CREATE_TIME_RANGE)){
			reply = npd_dbus_poe_create_time_range(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_DELETE_TIME_RANGE)){
			reply = npd_dbus_poe_delete_time_range(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_ADD_TIME_RANGE)){
			reply = npd_dbus_poe_add_time_range(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_NO_TIME_INFO)){
			reply = npd_dbus_no_poe_time_info(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_ON_TIME_DEPLOY)){
			reply = npd_dbus_poe_on_time_deploy(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_SHOW_POE_TIME_RANGE)){
			reply = npd_dbus_poe_show_time_range(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_SHOW_POE_TIME_INFO)){
			reply = npd_dbus_poe_show_time_info(connection, message, user_data);
		}
        else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_SHOW_POE_TIME_BIND_INFO)){
			reply = npd_dbus_poe_show_time_bind_info(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_ENDIS)){
			reply = npd_dbus_poe_config_port_endis(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_MAX_POWER)){
			reply = npd_dbus_poe_config_port_max_power(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_PRIORITY)){
			reply = npd_dbus_poe_config_port_priority(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_POWER_MANAGE)){
			reply = npd_dbus_poe_config_power_manage_mode(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_MODE)){
			reply = npd_dbus_poe_config_poe_interface_mode(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_LEGACY_CHECK)){
			reply = npd_dbus_poe_config_port_poe_legacy_check(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_GET_POE_INTERFACE)){
			reply = npd_dbus_poe_get_poe_interface(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_POE_GET_NEXT_INTERFACE)){
			reply = npd_dbus_poe_get_next_poe_interface(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_POE_GET_PSE)){
			reply = npd_dbus_poe_get_pse_info(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_SHOW_POE_RUNNING_CONFIG)){
			reply = npd_dbus_show_poe_running_config(connection, message, user_data);
    	}
		else if (dbus_message_is_method_call(message, 
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_POWER_MODE)){
			reply = npd_dbus_poe_config_power_mode(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message, 
										NPD_DBUS_POE_INTERFACE,
										NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_POWER_UP_MODE)){
			reply = npd_dbus_poe_config_power_up_mode(connection, message, user_data);
		}	
	}
#endif
#ifdef HAVE_SFLOW
	else if(strcmp(dbus_message_get_path(message), NPD_DBUS_SFLOW_OBJPATH) == 0) {	
		syslog_ax_dbus_dbg("npd obj path"NPD_DBUS_SFLOW_OBJPATH"\n");
		if (dbus_message_is_method_call(message,
										NPD_DBUS_SFLOW_INTERFACE,
										NPD_DBUS_SFLOW_METHOD_CONFIG_PORT_SFLOW_ENDIS)){
			reply = npd_dbus_sflow_config_port_endis(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_SFLOW_INTERFACE,
										NPD_DBUS_SFLOW_METHOD_CONFIG_SAMPLRATE)){
			reply = npd_dbus_sflow_config_samplrate(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_SFLOW_INTERFACE,
										NPD_DBUS_SFLOW_METHOD_CONFIG_AGENT)){
			reply = npd_dbus_sflow_config_agent(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										NPD_DBUS_SFLOW_INTERFACE,
										NPD_DBUS_SFLOW_METHOD_CONFIG_COLLECTOR)){
			reply = npd_dbus_sflow_config_collector(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, 
											NPD_DBUS_SFLOW_INTERFACE, 
											NPD_DBUS_METHOD_SHOW_SFLOW_RUNNIG_CONFIG)){
			reply = npd_dbus_sflow_show_running_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_SFLOW_METHOD_NO_AGENT)){
			reply = npd_dbus_sflow_no_agent(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_SFLOW_METHOD_NO_COLLECTOR)){
			reply = npd_dbus_sflow_no_collector(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_SFLOW_METHOD_NO_PORT)){
			reply = npd_dbus_sflow_no_port(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_METHOD_SHOW_SFLOW_AGT_CONFIG)){
			reply = npd_sflow_agent_show_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_METHOD_SHOW_SFLOW_CLT_CONFIG)){
			reply = npd_sflow_collector_show_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
											NPD_DBUS_SFLOW_INTERFACE,
											NPD_DBUS_METHOD_SHOW_SFLOW_PORT)){
			reply = npd_sflow_global_show_port(connection, message, user_data);
		}
	}
#endif

#ifdef HAVE_PORTAL
    else if(strcmp(dbus_message_get_path(message), NPD_DBUS_ASD_OBJPATH) == 0) {
        syslog_ax_dbus_dbg("npd asd path"NPD_DBUS_ASD_OBJPATH"\n");
		if (dbus_message_is_method_call(message,
										NPD_DBUS_ASD_INTERFACE,
										NPD_DBUS_ASD_SET_PORTAL_CLIENT_BYPASS))
		{
			reply = npd_dbus_asd_set_portal_client_bypass(connection, message, user_data);
		}
        else if(dbus_message_is_method_call(message,
                                            NPD_DBUS_ASD_INTERFACE,
                                            NPD_DBUS_ASD_SHOW_PORTAL_CLIENT_BYPASS))
		{
			reply = npd_dbus_asd_show_portal_bypass(connection,message,user_data);
		}
        else if(dbus_message_is_method_call(message,
                                            NPD_DBUS_ASD_INTERFACE,
                                            NPD_DBUS_ASD_PORTAL_SHOWRUNNING))
		{
			reply = npd_dbus_asd_portal_showrunning(connection,message,user_data);
		}
    }

#endif
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO    Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
	return DBUS_HANDLER_RESULT_HANDLED ;
}
extern DBusMessage *dbus_relay_to_slave(DBusConnection *conn, DBusMessage *msg, void *user_data);;
static DBusHandlerResult npd_dbus_ethports_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessage		*reply = NULL;
	if(reply == NULL)
	{
	    if (strcmp(dbus_message_get_path(message),NPD_DBUS_ETHPORTS_OBJPATH) == 0) 
    	{
    		npd_syslog_dbg("npd obj path "NPD_DBUS_ETHPORTS_OBJPATH);
            
    		if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SWITCHPORT_EXIST)) 
    		{
    			reply = npd_dbus_switchport_exist(connection,message,user_data);
    		} 
    		else if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_SWITCHPORT)) 
    		{
    			reply = npd_dbus_show_switchport(connection,message,user_data);
    		} 
#ifdef HAVE_CHASSIS_SUPPORT
			else if (dbus_message_is_method_call(message, NPD_DBUS_ETHPORTS_INTERFACE, NPD_DBUS_ETHPORTS_METHOD_GET_NEXT_STACK_PORT))
			{
				reply = npd_dbus_get_next_stack_port(connection, message, user_data);
			}
#endif
            else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_NEXT_GINDEX))
    		{
    			reply = npd_dbus_get_next_portindex(connection,message,user_data);
    		}
    		else if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE)) 
    		{
    			reply = npd_dbus_config_port_mode(connection,message,user_data);
    		}
#ifdef HAVE_LACP
            else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ALL_PORT_LACP)) 
    		{
    			reply = npd_dbus_config_all_port_lacp(connection,message,user_data);
    		} 
            else if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_LACP)) 
    		{
    			reply = npd_dbus_config_port_lacp(connection,message,user_data);
    		}
#endif
    		else if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE)) 
    		{
    			reply = npd_dbus_config_port_interface_mode(connection,message,user_data);
    		} 
    		/* add by yinlm@autelan.com for queue wrr and sp */
    		else if (dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_IPG)) 
    		{
    			reply = npd_dbus_show_ethport_ipg(connection,message,user_data);
    		} 
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT))
    		{
    			reply = npd_dbus_config_ethport_one(connection,message,user_data);
    		}
            else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORTDESC))
            {
                reply = npd_dbus_config_ethport_desc(connection, message, user_data);
            }
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG))
    		{
    			reply = npd_dbus_config_ethport_ipg(connection,message,user_data);
    		}    		
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_PROTECT))
    		{
    			reply = npd_dbus_config_switchport_protect(connection,message,user_data);
    		}    		
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR))
    		{
    			reply = npd_dbus_config_eth_port_attr(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_EEE))
    		{
    			reply = npd_dbus_config_eth_port_eee(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_LOOPBACK))
    		{
    			reply = npd_dbus_config_eth_port_loopback(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_RATELIMIT))
    		{
    			reply = npd_dbus_config_eth_port_ratelimit(connection,message,user_data);
    		}
            else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_VCT))
    		{
    			reply = npd_dbus_config_ports_vct(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA))
    		{
    			reply = npd_dbus_config_eth_port_attr(connection,message,user_data);
    		}
#ifdef HAVE_BRIDGE_STP
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP))
    		{
    			reply = npd_dbus_ethports_interface_config_stp(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP))
    		{
    			reply = npd_dbus_stp_all_vlans_bind_to_stpid(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_CONFIG_NONSTP))
    		{
    			reply = npd_dbus_stp_set_port_nonstp(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_CONFIG_P2P))
    		{
    			reply = npd_dbus_stp_set_port_p2p(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_CONFIG_EDGE))
    		{
    			reply = npd_dbus_stp_set_port_edge(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,MSTP_DBUS_METHOD_CFG_VLAN_ON_MST))
    		{
    			reply = npd_dbus_stp_set_stpid_for_vlan(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_CONFIG_PORTPRIO))
    		{
    			reply = npd_dbus_stp_set_port_prio(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST))
    		{
    			reply = npd_dbus_stp_set_port_pathcost(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG))
    		{
    			reply = npd_dbus_stp_show_running_cfg(connection,message,user_data);
    		}
#endif
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_STORM_CONTROL))
    		{
    			reply = npd_dbus_config_storm_control(connection,message,user_data);
    		}			
#ifdef HAVE_QOS
    		/*QOS*/
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_BIND_POLICY_MAP))
    		{
    			reply = npd_dbus_ethport_bind_policy_map(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORT_METHOD_SHOW_POLICY_MAP))
    		{
    			reply = npd_dbus_ethport_show_policy_map(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_UNBIND_POLICY_MAP))
    		{
    			reply = npd_dbus_ethport_unbind_policy_map(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_METHOD_TRAFFIC_SHAPE))
    		{
    			reply = npd_dbus_traffic_shape(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_METHOD_SHOW_TRAFFIC))
    		{
    			reply = npd_dbus_show_traffic_shape(connection,message,user_data);
    		}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_METHOD_DELETE_TRAFFIC))
    		{
    			reply = npd_dbus_delete_traffic_shape(connection,message,user_data);
    		}
#endif
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,	\
    										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RUNNING_CONFIG))
    		{
    			reply = npd_dbus_ethports_show_running_config(connection,message,user_data);
    		}	
#ifdef HAVE_QINQ			
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_SHOW_NETIF_QINQ_XLATE_RUNNING))
    		{
    			reply = npd_dbus_netif_xlate_showrunning(connection,message,user_data);
    		}		
#endif			
    		else if(dbus_message_is_method_call(message,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_SYSTEM_CONFIG_PORT_LINK)) {
    			reply = npd_dbus_eth_link_state_config (connection,message,user_data);
    		}
    		else 
    		{
    			syslog_ax_dbus_err("unknow method call %s",dbus_message_get_member(message));
    		}
    		
    	}
	    else
		{
		    syslog_ax_dbus_err("unknow obj path %s",dbus_message_get_path(message));
	    }
    }

	if (reply) 
	{
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO    Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
	return DBUS_HANDLER_RESULT_HANDLED ;
}

static DBusHandlerResult npd_dbus_relay_message_handler (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessage		*reply = NULL;
	#ifdef HAVE_DBUS_RELAY
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
        reply = dbus_relay_to_slave(connection, message, user_data);/*massage from active master to all active slave*/
	}
	#endif
	if(reply == NULL)
	{
	    if (strcmp(dbus_message_get_path(message),NPD_DBUS_RELAY_OBJPATH) == 0) 
    	{
    		syslog_ax_dbus_dbg("npd obj path "NPD_DBUS_RELAY_OBJPATH);

            if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_READ_PORT_VCT))
    		{
    			reply = npd_dbus_get_ethport_vct(connection,message,user_data);
    		}
            else if (dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR)) 
    		{
    			reply = npd_dbus_show_ethport_attr(connection,message,user_data);
    		} 
			else if (dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_EEE)) 
			{
    			reply = npd_dbus_get_eth_port_eee(connection,message,user_data);
			} 
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT))
    		{
    			reply = npd_dbus_show_ethport_stat(connection,message,user_data);
    		}
			else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_RATE))
    		{
    			reply = npd_dbus_show_ethport_rate(connection,message,user_data);
    		}
#ifdef HAVE_CHASSIS_SUPPORT
			else if (dbus_message_is_method_call(message, NPD_DBUS_RELAY_INTERFACE, NPD_DBUS_ETHPORTS_METHOD_SHOW_PORT_STAT_BY_SLOTNO_AND_PORTNO))
			{
				reply = npd_dbus_show_port_stat_by_slotno_and_portno(connection, message, user_data);
			}	
			else if (dbus_message_is_method_call(message, NPD_DBUS_RELAY_INTERFACE, NPD_DBUS_ETHPORTS_METHOD_CLEAR_STACK_STAT_BY_SLOTNO_AND_PORTNO))
			{
				reply = npd_dbus_clear_stack_stat_by_slotno_and_portno(connection, message, user_data);
			}
#endif
			else if (dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_SFP))
			{
				 reply = npd_dbus_show_ethport_sfp(connection,message,user_data);
			}
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT))
    		{
    			reply = npd_dbus_clear_ethport_stat(connection,message,user_data);
    		}
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_CPU_STATS))
			{
				reply = npd_dbus_clear_cpu_stats(connection,message,user_data);
			}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_CPU_STATS))
    		{
    			reply = npd_dbus_show_cpu_stats(connection,message,user_data);
    		}
#ifdef HAVE_TEMPERATURE_MONITOR
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_DEVICE_METHOD_SHOW_TEMPER))
			{
				reply = npd_dbus_show_temperature(connection,message,user_data);
			}
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_DEVICE_METHOD_TEMPER_THRESHOLD))
			{
				reply = npd_dbus_config_temper_threshold(connection,message,user_data);
			}
#endif
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_MANUFACTURE_TEST_BOARD))
			{
				reply = npd_dbus_manufacture_board(connection,message,user_data);
			}
            else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_PRE_MANU_TEST_BOARD))
			{
				reply = npd_dbus_pre_manufacture_board(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_MANUFACTURE_TEST_SUBBOARD))
			{
				reply = npd_dbus_manufacture_subboard(connection,message,user_data);
			}
    		else if(dbus_message_is_method_call(message,NPD_DBUS_RELAY_INTERFACE,NPD_DBUS_METHOD_GET_COUNTER))
    		{
    			reply = npd_dbus_read_counter(connection,message,user_data);
    		}
            else 
    		{
    			syslog_ax_dbus_err("unknow method call %s",dbus_message_get_member(message));
    		}
    		
    	}
	    else
		{
		    syslog_ax_dbus_err("unknow obj path %s",dbus_message_get_path(message));
	    }
    }

	if (reply) 
	{
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO    Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
	return DBUS_HANDLER_RESULT_HANDLED ;
}
/** Message handler for Signals
 *  or normally there should be no signals except dbus-daemon related.
 *
 *  @param  connection          D-BUS connection
 *  @param  message             Message
 *  @param  user_data           User data
 *  @return                     What to do with the message
 */
DBusHandlerResult
npd_dbus_filter_function (DBusConnection * connection,
					   DBusMessage * message, void *user_data)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */
		syslog_ax_dbus_dbg ("Got disconnected from the system message bus; "
				"retrying to reconnect every 1000 ms");

		dbus_connection_unref (npd_dbus_connection);
		npd_dbus_connection = NULL;
		sleep(1);
        while(npd_dbus_init() == FALSE)
        {
			sleep(1);
        }
		/*g_timeout_add (3000, reinit_dbus, NULL);*/

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

		/*if (services_with_locks != NULL)  service_deleted (message);*/
	} else
		return TRUE;
		/*return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);*/

	return DBUS_HANDLER_RESULT_HANDLED;
}


int npd_dbus_init(void)
{
	DBusError dbus_error;
	DBusObjectPathVTable	npd_vtable = {NULL, &npd_dbus_message_handler, NULL, NULL, NULL, NULL};
	DBusObjectPathVTable	ports_vtable = {NULL, &npd_dbus_ethports_message_handler, NULL, NULL, NULL, NULL};
    DBusObjectPathVTable	relay_vtable = {NULL, &npd_dbus_relay_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	npd_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (npd_dbus_connection == NULL) {
		syslog_ax_dbus_err ("dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

    dbus_connection_set_exit_on_disconnect(npd_dbus_connection,FALSE);
	/* Use npd to handle subsection of NPD_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (npd_dbus_connection, NPD_DBUS_OBJPATH, &npd_vtable, NULL)) {
		syslog_ax_dbus_err("can't register D-BUS handlers (fallback NPD). cannot continue.");
		return FALSE;
		
	}
	
    /* use port handler to handle exact port related functions.*/
	if (!dbus_connection_register_object_path (npd_dbus_connection, NPD_DBUS_ETHPORTS_OBJPATH, &ports_vtable, NULL)) {
        	syslog_ax_dbus_err("can't register D-BUS handlers (obj PORTS)). cannot continue.");
		return FALSE;
		
	}

    if (!dbus_connection_register_object_path (npd_dbus_connection, NPD_DBUS_RELAY_OBJPATH, &relay_vtable, NULL)) {
        	syslog_ax_dbus_err("can't register D-BUS handlers (obj relay)). cannot continue.");
		return FALSE;
		
	}
        
	dbus_bus_request_name (npd_dbus_connection, NPD_DBUS_BUSNAME,
			       0, &dbus_error);
	
	
	if (dbus_error_is_set (&dbus_error)) {
		syslog_ax_dbus_err ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return FALSE;
	}

/*

	dbus_connection_add_filter (npd_dbus_connection, npd_dbus_filter_function, NULL, NULL);
	dbus_bus_add_match (npd_dbus_connection,
			    "type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);
*/
	return TRUE;
  
}

extern int dbus_relay_client_init();
void * npd_dbus_thread_main(void *arg)
{
	/* tell my thread id*/
	npd_init_tell_whoami("NpdDBUS",0);

	/* tell about my initialization process done*/
	npd_init_tell_stage_end();
#ifdef HAVE_DBUS_RELAY	
	dbus_relay_client_init();
#endif

#ifdef HAVE_DBUS_NONE	
	/* For running on the dbus dameon not on this system */
	while (1)
	{
		sleep(100);
	}
	
#endif


	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	for(;;)
	{
    	while (dbus_connection_read_write_dispatch(npd_dbus_connection,-1)) {
    		;
    	}
    
        dbus_connection_unref(npd_dbus_connection);
        npd_dbus_connection = NULL;
        npd_dbus_init();
    }

    
	return NULL;
}
#ifdef __cplusplus
}
#endif
