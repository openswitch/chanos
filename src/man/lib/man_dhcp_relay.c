
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_DHCP_RELAY
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include "npd_database.h"
#include "npd/npd_dhcp_relay.h"
#include "man_dhcp_relay.h"
#include "man_dhcp.h"
#include "man_intf.h"

extern DBusConnection *config_dbus_connection;

int man_dhcp_relay_service_mutex(char* interafce_name)
{
#ifdef HAVE_DHCP
    unsigned int count = 0;
    struct dcli_dhcpd_intf service_intf_cfg;

    memset(&service_intf_cfg, 0, sizeof(struct dcli_dhcpd_intf));

    strncpy(service_intf_cfg.ifName, interafce_name, DHCPD_NAME_LEN);
    if (DHCP_SERVER_RETURN_CODE_SUCCESS == dhcpd_intf_show(&service_intf_cfg, NULL, &count))
    {
        if (TRUE == service_intf_cfg.enDis)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
#endif

    return 0;
}

unsigned int dhcpr_global_enable_config
(
	unsigned char isEnable 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_RELAY_OBJPATH,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_GLOBAL_ENABLE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpr_intf_enable_config
(
	unsigned char isEnable ,
	unsigned char *ifName
)
{
	unsigned int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_RELAY_OBJPATH,
										NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCPR_METHOD_INTF_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return 0;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

unsigned int dhcpr_helper_add
(
	unsigned char *ifName,
	unsigned int ipAddr,
	unsigned int port
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, 
									NPD_DBUS_DHCP_RELAY_OBJPATH, 
									NPD_DBUS_DHCP_RELAY_INTERFACE, 
									NPD_DBUS_DHCPR_METHOD_ADD_IP_HELPER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ifName,
							 DBUS_TYPE_UINT32, &ipAddr,
							 DBUS_TYPE_UINT32, &port,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))  
	 {		
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);

	return op_ret;
}

unsigned int dhcpr_helper_del
(
	unsigned char *ifName,
	unsigned int ipAddr
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, 
									NPD_DBUS_DHCP_RELAY_OBJPATH, 
									NPD_DBUS_DHCP_RELAY_INTERFACE, 
									NPD_DBUS_DHCPR_METHOD_DEL_IP_HELPER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ifName,	
							 DBUS_TYPE_UINT32, &ipAddr,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;
}

unsigned int dcli_dhcp_relay_information_config
(
	unsigned char isEnable
)
{
    unsigned int    ret = 0;
	DBusMessage*    query = NULL;
	DBusMessage*    reply = NULL;
	DBusError       err;



	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_RELAY_OBJPATH, 
									    NPD_DBUS_DHCP_RELAY_INTERFACE,
										NPD_DBUS_DHCP_RELAY_METHOD_OPT82_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
    
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int show_dhcpr_helper
(
    char* next_ifname,
    struct man_dhcp_relay_helper_info_s* helper_info,
	unsigned int *serverCount
)
{
	int j;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_detail_array;		
	unsigned int op_ret = 0;
    unsigned char* pifname = NULL;
    unsigned char* pnext_ifname = NULL;

    pifname = helper_info->ifName;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, 
									NPD_DBUS_DHCP_RELAY_OBJPATH, 
									NPD_DBUS_DHCP_RELAY_INTERFACE, 
									NPD_DBUS_DHCPR_METHOD_SHOW_IP_HELPER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &pifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter, serverCount);
	dbus_message_iter_next(&iter);	

	if(DHCP_RELAY_RETURN_CODE_SUCCESS == op_ret)
	{
        dbus_message_iter_get_basic(&iter, &pifname);
		dbus_message_iter_next(&iter);
        strncpy((char*)(helper_info->ifName), (char*)pifname, sizeof(helper_info->ifName));
        
        dbus_message_iter_get_basic(&iter, &pnext_ifname);
		dbus_message_iter_next(&iter);
        if (next_ifname)
        {
            strncpy(next_ifname, (char*)pnext_ifname, sizeof(helper_info->ifName));
        }
        
		dbus_message_iter_recurse(&iter,&iter_array);
		DBusMessageIter	 iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(helper_info->netifIndex));
		dbus_message_iter_next(&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&(helper_info->enDis));				
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_recurse(&iter_struct,&iter_detail_array);
		for(j=0;j<MAX_DHCPR_IP_NUM;j++)
		{
			DBusMessageIter  iter_detail_struct;
			
			dbus_message_iter_recurse(&iter_detail_array,&iter_detail_struct);
			dbus_message_iter_get_basic(&iter_detail_struct,&(helper_info->ip_addr[j]));
			dbus_message_iter_next(&iter_detail_struct);					
			
			dbus_message_iter_get_basic(&iter_detail_struct,&(helper_info->port[j]));
			dbus_message_iter_next(&iter_detail_struct);
			
			dbus_message_iter_next(&iter_detail_array);
		}			
		dbus_message_iter_next(&iter_array);				
	}

	dbus_message_unref(reply);
	return op_ret;
}


unsigned int show_dhcpr_status
(
    struct dcli_dhcpr_query* dhcpr_query
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, 
										NPD_DBUS_DHCP_RELAY_OBJPATH, 
										NPD_DBUS_DHCP_RELAY_INTERFACE, 
										NPD_DBUS_CONFIG_IP_DHCP_RELAY_SHOW_GLOBAL);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return 1;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_BYTE, &(dhcpr_query->dhcp_relay_endis),
							DBUS_TYPE_BYTE, &(dhcpr_query->dhcp_relay_opt82_enable),
							DBUS_TYPE_INVALID))
	{
		printf( "failed get args!\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return 1;
	}
	dbus_message_unref(reply);
    
	return 0;
}


#ifdef __cplusplus
}
#endif
#endif
