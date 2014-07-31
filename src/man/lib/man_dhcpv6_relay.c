
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#if defined(HAVE_NPD_IPV6) && defined(HAVE_DHCPV6_RELAY)
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include <netinet/ip6.h>
#include "man_dhcpv6_relay.h"

extern DBusConnection *config_dbus_connection;

unsigned int man_dhcp6r_global(unsigned int is_enable)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_GLOBAL);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &is_enable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcp6r_interface(unsigned int is_enable, unsigned char* p_if_name)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &is_enable,
							DBUS_TYPE_STRING, &p_if_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}
    
unsigned int man_dhcp6r_server
(
    struct in6_addr* ipv6_address,
    unsigned int is_fwd_interface,
    unsigned char* p_if_name,
    unsigned char* p_if_name_fwd
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_SERVER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[0],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[1],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[2],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[3],
							DBUS_TYPE_UINT32, &is_fwd_interface,
							DBUS_TYPE_STRING, &p_if_name,
							DBUS_TYPE_STRING, &p_if_name_fwd,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}
    
unsigned int man_dhcp6r_no_server
(
    struct in6_addr* ipv6_address,
    unsigned char* p_if_name
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_NO_SERVER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[0],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[1],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[2],
							DBUS_TYPE_UINT32, &ipv6_address->s6_addr32[3],
							DBUS_TYPE_STRING, &p_if_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpv6_relay_global_show(unsigned int* is_enable)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_GLOBAL_SHOW);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_UINT32, is_enable,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpv6_relay_interface_show(struct man_dhcpv6_relay_s* entry)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE_SHOW);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &entry->netif_index,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_UINT32, &entry->is_enable,
							DBUS_TYPE_UINT32, &entry->netif_index,
							DBUS_TYPE_UINT32, &entry->md6_server_fwd,
							DBUS_TYPE_UINT32, &entry->md6_server_fwd_ifidx,
							DBUS_TYPE_UINT32, &entry->md6_server_in32[0],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[1],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[2],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[3],
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpv6_relay_interface_show_one(struct man_dhcpv6_relay_s* entry, char* p_if_name)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCPV6_RELAY_OBJPATH,
										NPD_DBUS_DHCPV6_RELAY_INTERFACE,
										NPD_DBUS_DHCP6R_METHOD_INTERFACE_SHOW_ONE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &p_if_name,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_UINT32, &entry->is_enable,
							DBUS_TYPE_UINT32, &entry->netif_index,
							DBUS_TYPE_UINT32, &entry->md6_server_fwd,
							DBUS_TYPE_UINT32, &entry->md6_server_fwd_ifidx,
							DBUS_TYPE_UINT32, &entry->md6_server_in32[0],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[1],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[2],
							DBUS_TYPE_UINT32, &entry->md6_server_in32[3],
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);
	
	return ret;
}


#ifdef __cplusplus
}
#endif
#endif
