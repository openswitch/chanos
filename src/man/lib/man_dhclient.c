#ifdef HAVE_DHCP_CLIENT
#ifdef __cplusplus
extern "C"
{
#endif
	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include "man_dhclient.h"
#include "man_dhcp.h"
#include "man_dhcp_relay.h"

extern DBusConnection *dcli_dbus_connection;

int man_dhclient_raw(unsigned char* ifname, unsigned char* string, unsigned int* op_ret, const char* dhclient_dbus_interface_method)
{
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										DHCLIENT_DBUS_BUSNAME,
										DHCLIENT_DBUS_OBJPATH,
										DHCLIENT_DBUS_INTERFACE,
										dhclient_dbus_interface_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_STRING, &ifname,
						 DBUS_TYPE_STRING, &string,
						 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return -1;
	}
    
    return 0;
}

int man_dhclient_32(unsigned char* ifname, unsigned int value, unsigned int* op_ret, const char* dhclient_dbus_interface_method)
{
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										DHCLIENT_DBUS_BUSNAME,
										DHCLIENT_DBUS_OBJPATH,
										DHCLIENT_DBUS_INTERFACE,
										dhclient_dbus_interface_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_STRING, &ifname,
						 DBUS_TYPE_UINT32, &value,
						 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
        return -1;
	}
    
    return 0;
}


int man_dhclient_show(unsigned char* ifname, struct man_dhclient_interface_s* dhclient_cfg, unsigned int* op_ret, const char* dhclient_dbus_interface_method)
{
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
    DBusMessageIter	 iter;
	DBusError err;

    char* pstring = NULL;
    int ni = 0;

	query = dbus_message_new_method_call(
										DHCLIENT_DBUS_BUSNAME,
										DHCLIENT_DBUS_OBJPATH,
										DHCLIENT_DBUS_INTERFACE,
										dhclient_dbus_interface_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_STRING, &ifname,
						 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	
	dbus_message_iter_init(reply, &iter);
		
	dbus_message_iter_get_basic(&iter, op_ret);
	dbus_message_iter_next(&iter);

    dbus_message_iter_get_basic(&iter, &pstring);
	dbus_message_iter_next(&iter);
    strncpy(dhclient_cfg->ifname, pstring, MAN_DHCLIENT_IFNAME_LENGTH_MAX);

    dbus_message_iter_get_basic(&iter, &pstring);
	dbus_message_iter_next(&iter);
    strncpy(dhclient_cfg->client_id, pstring, MAN_DHCLIENT_STRING_LENGTH_MAX);

    dbus_message_iter_get_basic(&iter, &pstring);
	dbus_message_iter_next(&iter);
    strncpy(dhclient_cfg->class_id, pstring, MAN_DHCLIENT_STRING_LENGTH_MAX);

    dbus_message_iter_get_basic(&iter, &(dhclient_cfg->default_route_flag));
	dbus_message_iter_next(&iter);
    
    dbus_message_iter_get_basic(&iter, &(dhclient_cfg->request_time));
	dbus_message_iter_next(&iter);

    dbus_message_iter_get_basic(&iter, &(dhclient_cfg->interface_enable));
	dbus_message_iter_next(&iter);

    for (ni = 0; ni < (256 / sizeof(unsigned int)); ni++)
    {
        dbus_message_iter_get_basic(&iter, &(dhclient_cfg->option_request_bit[ni]));
    	dbus_message_iter_next(&iter);
    }
    
    dbus_message_iter_get_basic(&iter, &pstring);
	dbus_message_iter_next(&iter);
    strncpy(dhclient_cfg->ipaddr, pstring, MAN_DHCLIENT_IFNAME_LENGTH_MAX);
    
    dbus_message_iter_get_basic(&iter, &(dhclient_cfg->expiry));
	dbus_message_iter_next(&iter);
    
    return 0;
}



int man_dhclient_client_id(unsigned char* ifname, unsigned char* string, unsigned int* op_ret)
{
    return man_dhclient_raw(ifname, string, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_CLIENT_ID);
}

int man_dhclient_class_id(unsigned char* ifname, unsigned char* string, unsigned int* op_ret)
{
    return man_dhclient_raw(ifname, string, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_CLASS_ID);
}

int man_dhclient_option_default(unsigned char* ifname, unsigned char* string, unsigned int* op_ret)
{
    return man_dhclient_raw(ifname, string, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_OPTION_DEFAULT);
}

int man_dhclient_option_add(unsigned char* ifname, unsigned char* string, unsigned int* op_ret)
{
    return man_dhclient_raw(ifname, string, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_OPTION_ADD);
}

int man_dhclient_option_remove(unsigned char* ifname, unsigned char* string, unsigned int* op_ret)
{
    return man_dhclient_raw(ifname, string, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_OPTION_REMOVE);
}

int man_dhclient_lease(unsigned char* ifname, unsigned int value, unsigned int* op_ret)
{
    return man_dhclient_32(ifname, value, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_LEASE);
}

int man_dhclient_default_route(unsigned char* ifname, unsigned int value, unsigned int* op_ret)
{
    return man_dhclient_32(ifname, value, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_DEFAULT_ROUTE);
}

int man_dhclient_interface(unsigned char* ifname, unsigned int value, unsigned int* op_ret)
{
    return man_dhclient_32(ifname, value, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_INTERFACE);
}

int man_dhclient_action(unsigned char* ifname, unsigned int value, unsigned int* op_ret)
{
    return man_dhclient_32(ifname, value, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_ACTION);
}

int man_dhclient_show_one(unsigned char* ifname, struct man_dhclient_interface_s* dhclient_cfg, unsigned int* op_ret)
{
    return man_dhclient_show(ifname, dhclient_cfg, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_SHOW_ONE);
}

int man_dhclient_show_next(unsigned char* ifname, struct man_dhclient_interface_s* dhclient_cfg, unsigned int* op_ret)
{
    return man_dhclient_show(ifname, dhclient_cfg, op_ret, DHCLIENT_DBUS_INTERFACE_METHOD_SHOW_NEXT);
}

int man_dhclient_mutex_staus(unsigned char* ifname, unsigned int* status)
{
    unsigned int count = 0;
#ifdef HAVE_DHCP
    struct dcli_dhcpd_intf service_intf_cfg;
#endif
#ifdef HAVE_DHCP_RELAY
    struct man_dhcp_relay_helper_info_s helper_info;
#endif
#ifdef HAVE_DHCP
    memset(&service_intf_cfg, 0, sizeof(struct dcli_dhcpd_intf));
#endif
#ifdef HAVE_DHCP_RELAY
    memset(&helper_info, 0, sizeof(struct man_dhcp_relay_helper_info_s));
#endif

#ifdef HAVE_DHCP
    strncpy(service_intf_cfg.ifName, ifname, DHCPD_NAME_LEN);
    if (DHCP_SERVER_RETURN_CODE_SUCCESS == dhcpd_intf_show(&service_intf_cfg, NULL, &count))
    {
        if (TRUE == service_intf_cfg.enDis)
        {
            *status |= DHCLINET_MUTEX_SERVER;
            return 0;
        }
    }
#endif
#ifdef HAVE_DHCP_RELAY
    strncpy(helper_info.ifName, ifname, sizeof(helper_info.ifName));
    if (0 == show_dhcpr_helper(NULL, &helper_info, &count))
    {
        if (helper_info.enDis)
        {
            *status |= DHCLINET_MUTEX_RELAY;
            return 0;
        }
    }
#endif

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif

