
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_DHCPV6_SERVER
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include "npd_database.h"
#include "man_dhcpv6.h"
#include "man_dhcp_relay.h"
#include "man_intf.h"

extern DBusConnection *config_dbus_connection;

unsigned int man_dhcpv6_reserve_ip(const unsigned char* p_ipv6_address)
{
    struct in6_addr ipv6_address;

    if ((NULL == p_ipv6_address)
        || (strlen(p_ipv6_address) > sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")))
    {
        return -1;
    }

    memset(&ipv6_address, 0, sizeof(ipv6_address));

    inet_pton(AF_INET6, p_ipv6_address, &ipv6_address);

    if (IN6_IS_ADDR_UNSPECIFIED(&ipv6_address)
        || IN6_IS_ADDR_LOOPBACK(&ipv6_address)
        || IN6_IS_ADDR_MULTICAST(&ipv6_address)
        || IN6_IS_ADDR_V4MAPPED(&ipv6_address)
        || IN6_IS_ADDR_V4COMPAT(&ipv6_address))
    {
        return -1;
    }

    /* XXX: Link-Local unicast ??? */
    
    return 0;
}

unsigned int man_dhcpv6_range6_check
(
    const unsigned char* range_low,
    const unsigned char* range_high,
    const unsigned char* prefix
)
{
    int ni = 0;
    int length = 0;
    struct in6_addr low_address;
    struct in6_addr high_address;
    struct in6_addr prefix_address;
    struct in6_addr inner_prefix_address;
    char prefix_buf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128")];
    char* p_prefix = NULL;

    if ((NULL == range_low)
        || (NULL == range_high)
        || (NULL == prefix)
        || (strlen(range_low) > sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"))
        || (strlen(range_high) > sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"))
        || (strlen(prefix) > sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128")))
    {
        return -1;
    }

    memset(&low_address, 0, sizeof(low_address));
    memset(&high_address, 0, sizeof(high_address));
    memset(&prefix_address, 0, sizeof(prefix_address));
    memset(&inner_prefix_address, 0, sizeof(inner_prefix_address));
    memset(prefix_buf, 0, sizeof(prefix_buf));

    strncpy(prefix_buf, prefix, \
        (strlen(prefix) > sizeof(prefix_buf)) ? sizeof(prefix_buf) : strlen(prefix));

    p_prefix = strchr(prefix_buf, '/');
    p_prefix[0] = '\0';
    p_prefix++;
    length = (int)strtoul(p_prefix, NULL, 10);
    if (length >= 128)
    {
        return -1;
    }

    inet_pton(AF_INET6, range_low, &low_address);
    inet_pton(AF_INET6, range_high, &high_address);
    inet_pton(AF_INET6, prefix_buf, &prefix_address);

    if (IN6_IS_ADDR_UNSPECIFIED(&low_address)
        || IN6_IS_ADDR_LOOPBACK(&low_address)
        || IN6_IS_ADDR_MULTICAST(&low_address)
        || IN6_IS_ADDR_V4MAPPED(&low_address)
        || IN6_IS_ADDR_V4COMPAT(&low_address))
    {
        return -1;
    }

    if (IN6_IS_ADDR_UNSPECIFIED(&high_address)
        || IN6_IS_ADDR_LOOPBACK(&high_address)
        || IN6_IS_ADDR_MULTICAST(&high_address)
        || IN6_IS_ADDR_V4MAPPED(&high_address)
        || IN6_IS_ADDR_V4COMPAT(&high_address))
    {
        return -1;
    }

    memcpy(&inner_prefix_address, &prefix_address, sizeof(struct in6_addr));
    for (ni = length; ni < 128; ni++)
    {
        inner_prefix_address.s6_addr32[ni / 32] &= ~(0x1 << (31 - ni % 32));
    }

    if (!IN6_ARE_ADDR_EQUAL(&inner_prefix_address, &prefix_address))
    {
        return -1;
    }

    IN6_ARE_ADDR_AND(&low_address, &prefix_address);
    IN6_ARE_ADDR_AND(&high_address, &prefix_address);

    if (!IN6_ARE_ADDR_EQUAL(&low_address, &high_address))
    {
        return -1;
    }

    /* XXX: Link-Local unicast ??? */
    
    return 0;
}

unsigned int man_dhcpv6_lifetime_exchange
(
    const char* day_time,
    const char* hour_time,
    const char* minite_time,
    unsigned int* lifetime
)
{    
    int n_days = 0;
    int n_hours = 0;
    int n_minutes = 0;
	unsigned int un_time = 0;

    if (0 != day_time[0])
    {
        n_days = (int)strtoul(day_time, NULL, 10);

        if (0 != hour_time[0])
        {
            n_hours = (int)strtoul(hour_time, NULL, 10);
            if (0 != minite_time[0])
            {
                n_minutes = (int)strtoul(minite_time, NULL, 10);
            }
        }
    }

	un_time = n_days * 24 * 60 * 60 + n_hours * 60 * 60 + n_minutes * 60;
    
    *lifetime = un_time;
    
	return 0;            
}

int man_dhcpv6_hex_check(const unsigned char* hex)
{
    int length = 0;
    char* pri = NULL;
    char* endptr = NULL;
    int value = 0;

    if (NULL != hex)
    {
        pri = (char*)hex;
        length = strlen(hex);
        do
        {
            value = (int)strtoul(pri, &endptr, 16);
            if ((endptr - pri) != 2)
            {
                return -1;
            }
            else if ('\0' == endptr[0])
            {
                return 0;
            }
            else if (':' != endptr[0])
            {
                return -1;
            }
            length -= 3;
            endptr++;
            pri = endptr;

            } while (length >= 0);
        }
        else
        {
            return -1;
        }

    return 0;
}

#define MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method)           \
do {                                                            \
	query = dbus_message_new_method_call(                       \
                                	DHCPDV6_DBUS_BUSNAME,       \
                                	DHCPDV6_DBUS_OBJPATH,       \
                                	DHCPDV6_DBUS_INTERFACE,     \
                                	p_method);                  \
} while (0)

#define MAN_DHCPDV6_DBUS_FOR_ERROR(err)             \
	printf("failed get reply.\n");                  \
	if (dbus_error_is_set(&err))                    \
    {                                               \
    	printf("%s raised: %s\n", err.name, err.message);       \
    	dbus_error_free(&err);                      \
    }                                               \
	return -1

#define MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret)    \
do                                          \
{                                           \
    DBusMessage* reply = NULL;              \
	DBusError err;                          \
                                            \
    dbus_error_init(&err);                  \
    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);    \
                                            \
	dbus_message_unref(query);              \
	if (NULL == reply)                      \
    {                                       \
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);    \
    }                                       \
                                            \    
	if (!dbus_message_get_args(reply, &err, \
                        	DBUS_TYPE_UINT32, &op_ret,          \
                        	DBUS_TYPE_INVALID))                 \
    {                                       \
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);    \
    }                                       \
                                            \
} while (0)

unsigned int man_dhcpv6_dbus_u32(unsigned int value, const unsigned char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value,
                         DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_dbus_string(const unsigned char* p_string, const unsigned char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_string,
                                DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_dbus_u32_string
(
    unsigned int value,
    const unsigned char* p_string,
    const unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_UINT32, &value,
                                DBUS_TYPE_STRING, &p_string,
                                DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_dbus_2_string
(
    const unsigned char* p_string_1,
    const unsigned char* p_string_2,
    const unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_string_1,
                                DBUS_TYPE_STRING, &p_string_2,
                                DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_dbus_u32_2_string
(
    unsigned int value,
    const unsigned char* p_string_1,
    const unsigned char* p_string_2,
    const unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_UINT32, &value,
                                DBUS_TYPE_STRING, &p_string_1,
                                DBUS_TYPE_STRING, &p_string_2,
                                DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_dbus_4_string
(
    const unsigned char* p_string_1,
    const unsigned char* p_string_2,
    const unsigned char* p_string_3,
    const unsigned char* p_string_4,
    const unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_string_1,
                                DBUS_TYPE_STRING, &p_string_2,
                                DBUS_TYPE_STRING, &p_string_3,
                                DBUS_TYPE_STRING, &p_string_4,
                                DBUS_TYPE_INVALID);

    MAN_DHCPV6_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_dhcpv6_service_set(unsigned int is_enable)
{
    return man_dhcpv6_dbus_u32(is_enable, DHCPDV6_DBUS_INTERFACE_METHOD_GLOBAL_SET);
}

unsigned int man_dhcpv6_pool(const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_string(p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL);
}

unsigned int man_dhcpv6_pool_delete(const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_string(p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_DELETE);
}

unsigned int man_dhcpv6_pool_domain(const unsigned char* p_domain, const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_2_string(p_domain, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_DOMAIN);
}

unsigned int man_dhcpv6_pool_dns
(
    const unsigned char* p_dns_1,
    const unsigned char* p_dns_2,
    const unsigned char* p_dns_3,
    const unsigned char* p_pool_name
)
{
        return man_dhcpv6_dbus_4_string(
            p_dns_1,
            p_dns_2,
            p_dns_3,
            p_pool_name,
            DHCPDV6_DBUS_INTERFACE_METHOD_POOL_DNS);

}

unsigned int man_dhcpv6_pool_range6
(
    const unsigned char* prefix,
    const unsigned char* range_low,
    const unsigned char* range_high,
    const unsigned char* p_pool_name
)
{
        return man_dhcpv6_dbus_4_string(
            prefix,
            range_low,
            range_high,
            p_pool_name,
            DHCPDV6_DBUS_INTERFACE_METHOD_POOL_RANGE6);
}

unsigned int man_dhcpv6_pool_preferred_lifetime(unsigned int preferred_time, const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_u32_string(preferred_time, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_PREFERRED_TIME);
}

unsigned int man_dhcpv6_pool_valid_lifetime(unsigned int valid_time, const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_u32_string(valid_time, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_VALID_TIME);
}

unsigned int man_dhcpv6_pool_preference(unsigned int preference, const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_u32_string(preference, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_PREFERENCE);
}

unsigned int man_dhcpv6_pool_rapid(unsigned int is_enable, const unsigned char* p_pool_name)
{
    return man_dhcpv6_dbus_u32_string(is_enable, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_RAPID_COMMIT);
}

unsigned int man_dhcpv6_if_set(unsigned int is_enable, const unsigned char* p_if_name)
{
    return man_dhcpv6_dbus_u32_string(is_enable, p_if_name, DHCPDV6_DBUS_INTERFACE_METHOD_INTERFACE_SET);
}


unsigned int man_dhcpv6_pool_vendor
(
    unsigned int enterprise_number,
    const unsigned char* vendor_data,
    const unsigned char* p_pool_name
)
{
    return man_dhcpv6_dbus_u32_2_string(enterprise_number, vendor_data, p_pool_name, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_VENDOR);
}

unsigned int man_dhcpv6_host(const unsigned char* ipv6_addr, const unsigned char* duid)
{
    return man_dhcpv6_dbus_2_string(ipv6_addr, duid, DHCPDV6_DBUS_INTERFACE_METHOD_HOST_ADD);
}

unsigned int man_dhcpv6_host_remove(const unsigned char* ipv6_addr)
{
    return man_dhcpv6_dbus_string(ipv6_addr, DHCPDV6_DBUS_INTERFACE_METHOD_HOST_DELETE);
}

unsigned int man_dhcpv6_get_u32(unsigned int* value, const unsigned char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);

    dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_UINT32, value,
                        	DBUS_TYPE_INVALID))
    {
        MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

	dbus_message_unref(reply);
    
	return op_ret;
}

unsigned int man_dhcpv6_global_get(struct man_dhcpdv6_service_s* entry)
{
    return man_dhcpv6_get_u32(&entry->is_enable, DHCPDV6_DBUS_INTERFACE_METHOD_GLOBAL_GET);
}

unsigned int man_dhcpv6_ia_count_get(unsigned int* ia_count)
{
    return man_dhcpv6_get_u32(ia_count, DHCPDV6_DBUS_INTERFACE_METHOD_IA_COUNT_GET);
}

unsigned int man_dhcpv6_host_get_next(struct man_dhcpdv6_host_s* entry)
{
    unsigned int op_ret = 0;
    char* p_ipv6_address = entry->ipv6_address;
    char* p_duid = NULL;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, DHCPDV6_DBUS_INTERFACE_METHOD_HOST_NEXT_GET);

    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_ipv6_address,
                                DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_STRING, &p_ipv6_address,
                        	DBUS_TYPE_STRING, &p_duid,
                        	DBUS_TYPE_INVALID))
    {
        MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

    if (DHCPV6_SERVER_RETURN_CODE_SUCCESS == op_ret)
    {
        strncpy(entry->ipv6_address, p_ipv6_address, sizeof(entry->ipv6_address));
        strncpy(entry->duid, p_duid, sizeof(entry->duid));
    }

	dbus_message_unref(reply);
    
	return op_ret;
}

unsigned int man_dhcpv6_lease_get_next(struct man_dhcpdv6_iasubopt_s* entry)
{
    unsigned int op_ret = 0;
    unsigned int length = 0;
    unsigned char* p_array = NULL;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, DHCPDV6_DBUS_INTERFACE_METHOD_NEXT_IA_GET);

    dbus_message_append_args(query,
                                DBUS_TYPE_UINT32, &entry->ia.iaid,
                                DBUS_TYPE_UINT32, &entry->addr.s6_addr32[0],
                                DBUS_TYPE_UINT32, &entry->addr.s6_addr32[1],
                                DBUS_TYPE_UINT32, &entry->addr.s6_addr32[2],
                                DBUS_TYPE_UINT32, &entry->addr.s6_addr32[3],
                                DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                DBUS_TYPE_UINT32, &op_ret,
                DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &p_array, &length,
                DBUS_TYPE_INVALID))
    {
        if (dbus_error_is_set(&err)) 
        {
            printf("%% %s raised: %s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        dbus_message_unref(reply);
        return -1;
    }

    if (DHCPV6_SERVER_RETURN_CODE_SUCCESS == op_ret)
    {
        memcpy(entry, p_array, length);
    }

    return op_ret;
}

unsigned int man_dhcpv6_pool_get(struct man_dhcpdv6_pool_s* entry, const char* p_method)
{
    unsigned int op_ret = 0;
    char* p_pool_name = entry->pool_name;
    char* p_domain = NULL;
    char* p_dns_server_1 = NULL;
    char* p_dns_server_2 = NULL;
    char* p_dns_server_3 = NULL;
    char* p_prefix = NULL;
    char* p_range_low = NULL;
    char* p_range_high = NULL;
    char* p_vendor_data = NULL;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
    DBusMessageIter iter;
	DBusError err;

    MAN_DHCPV6_DBUS_MACRO_METHOD(query, p_method);

    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_pool_name,
                                DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &op_ret);

    if (DHCPV6_SERVER_RETURN_CODE_SUCCESS == op_ret)
    {
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_pool_name);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_domain);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_dns_server_1);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_dns_server_2);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_dns_server_3);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_prefix);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_range_low);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_range_high);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &entry->preferred_lifetime);
        
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &entry->valid_lifetime);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &entry->preference);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &entry->rapid_commit);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &entry->enterprise_number);
        
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_vendor_data);

        strncpy(entry->pool_name, p_pool_name, sizeof(entry->pool_name));
        strncpy(entry->domain, p_domain, sizeof(entry->domain));
        strncpy(entry->dns_server[0], p_dns_server_1, sizeof(entry->domain));   /* XXX: I see that it's domain */
        strncpy(entry->dns_server[1], p_dns_server_2, sizeof(entry->domain));
        strncpy(entry->dns_server[2], p_dns_server_3, sizeof(entry->domain));
        strncpy(entry->prefix, p_prefix, sizeof(entry->prefix));
        strncpy(entry->range_low, p_range_low, sizeof(entry->range_low));
        strncpy(entry->range_high, p_range_high, sizeof(entry->range_high));
        strncpy(entry->vendor_data, p_vendor_data, sizeof(entry->vendor_data));
    }

	dbus_message_unref(reply);
    
	return op_ret;
}

unsigned int man_dhcpv6_pool_get_one(struct man_dhcpdv6_pool_s* entry)
{
    return man_dhcpv6_pool_get(entry, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_GET_ONE);
}

unsigned int man_dhcpv6_pool_get_next(struct man_dhcpdv6_pool_s* entry)
{
    return man_dhcpv6_pool_get(entry, DHCPDV6_DBUS_INTERFACE_METHOD_POOL_GET_NEXT);
}

unsigned int man_dhcpv6_if_get_next(struct man_dhcpdv6_if_s* entry)
{
    unsigned int op_ret = 0;
    char* p_if_name = entry->if_name;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
    
    MAN_DHCPV6_DBUS_MACRO_METHOD(query, DHCPDV6_DBUS_INTERFACE_METHOD_IF_GET_NEXT);

    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &p_if_name,
                                DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_STRING, &p_if_name,
                        	DBUS_TYPE_UINT32, &entry->is_enable,
                        	DBUS_TYPE_INVALID))
    {
        MAN_DHCPDV6_DBUS_FOR_ERROR(err);
    }

    if (DHCPV6_SERVER_RETURN_CODE_SUCCESS == op_ret)
    {
        strncpy(entry->if_name, p_if_name, sizeof(entry->if_name));
    }

	dbus_message_unref(reply);
    
	return op_ret;
}

#ifdef __cplusplus
}
#endif
#endif

