
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_M4_TUNNEL 
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "lib/npd_bitop.h"
#include "man_tunnel.h"

extern DBusConnection* config_dbus_connection;

#define MAN_IP_TUNNEL_DBUS_MACRO_METHOD(query, p_method) \
do {                                                     \
	query = dbus_message_new_method_call(                \
                                	NPD_DBUS_BUSNAME,    \
                                	NPD_DBUS_OBJPATH,    \
                                	NPD_DBUS_INTERFACE,  \
                                	p_method);           \
} while (0)

#define MAN_IP_TUNNEL_DBUS_FOR_ERROR(err)           \
do {                                                \
	printf("failed get reply.\n");                  \
	if (dbus_error_is_set(&err))                    \
    {                                               \
    	printf("%s raised: %s\n", err.name, err.message);       \
    	dbus_error_free(&err);                      \
    }                                               \
	return -1;                                      \
}                                                   \
while (0)

#define MAN_IP_TUNNEL_DBUS_MACRO_RAW(query, op_ret)  \
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
    	MAN_IP_TUNNEL_DBUS_FOR_ERROR(err);  \
    }                                       \
	if (!dbus_message_get_args(reply, &err, \
                        	DBUS_TYPE_UINT32, &op_ret,          \
                        	DBUS_TYPE_INVALID))                 \
    {                                       \
    	MAN_IP_TUNNEL_DBUS_FOR_ERROR(err);  \
    }                                       \
                                            \
} while (0)

unsigned int man_dbus_ip_tunnel_u32(unsigned int value, const char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_IP_TUNNEL_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value,
                         DBUS_TYPE_INVALID);

    MAN_IP_TUNNEL_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

#if 0
unsigned int man_dbus_ip_tunnel_u32_in6(unsigned int value, struct in6_addr* in6, const char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_IP_TUNNEL_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                        DBUS_TYPE_UINT32, &value,
                        DBUS_TYPE_UINT32, &in6->s6_addr32[0],
                        DBUS_TYPE_UINT32, &in6->s6_addr32[1],
                        DBUS_TYPE_UINT32, &in6->s6_addr32[2],
                        DBUS_TYPE_UINT32, &in6->s6_addr32[3],
                        DBUS_TYPE_INVALID);

    MAN_IP_TUNNEL_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}
#endif

unsigned int man_dbus_ip_tunnel_2_u32(unsigned int value_1, unsigned int value_2, const char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_IP_TUNNEL_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value_1,
                         DBUS_TYPE_UINT32, &value_2,
                         DBUS_TYPE_INVALID);

    MAN_IP_TUNNEL_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_interface_tunnel(unsigned int id)
{
    return man_dbus_ip_tunnel_u32(id, NPD_DBUS_METHOD_INTERFACE_TUNNEL);
}

unsigned int man_no_interface_tunnel(unsigned int id)
{
    return man_dbus_ip_tunnel_u32(id, NPD_DBUS_METHOD_NO_INTERFACE_TUNNEL);
}

unsigned int man_ip_tunnel(unsigned int id)
{
    return man_dbus_ip_tunnel_u32(id, NPD_DBUS_METHOD_IP_TUNNEL);
}

unsigned int man_no_ip_tunnel(unsigned int id)
{
    return man_dbus_ip_tunnel_u32(id, NPD_DBUS_METHOD_NO_IP_TUNNEL);
}

#if 0
unsigned int man_tunnel_in6(unsigned int id, struct in6_addr* in6)
{
    return man_dbus_ip_tunnel_u32_in6(id, in6, NPD_DBUS_METHOD_TUNNEL_IPV6_ADDRESS);
}

unsigned int man_tunnel_no_in6(unsigned int id, struct in6_addr* in6)
{
    return man_dbus_ip_tunnel_u32_in6(id, in6, NPD_DBUS_METHOD_TUNNEL_NO_IPV6_ADDRESS);
}
#endif

unsigned int man_tunnel_source(unsigned int id, unsigned int in4)
{
    return man_dbus_ip_tunnel_2_u32(id, in4, NPD_DBUS_METHOD_TUNNEL_SOURCE);
}

unsigned int man_tunnel_destination(unsigned int id, unsigned int in4)
{
    return man_dbus_ip_tunnel_2_u32(id, in4, NPD_DBUS_METHOD_TUNNEL_DESTINATION);
}

unsigned int man_tunnel_mode(unsigned int id, unsigned int mode)
{
    return man_dbus_ip_tunnel_2_u32(id, mode, NPD_DBUS_METHOD_TUNNEL_MODE);
}

unsigned int man_show_interface_tunnel(unsigned int id, struct man_ip_tunnel_s* entry, const char* p_method)
{
    int length = 0;
    unsigned int op_ret = 0;
    unsigned char* p_string = NULL;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
    DBusError err;

    MAN_IP_TUNNEL_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &id,
                         DBUS_TYPE_INVALID);

    dbus_error_init(&err); 
    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_IP_TUNNEL_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &p_string, &length,
                        	DBUS_TYPE_INVALID))
    {
    	MAN_IP_TUNNEL_DBUS_FOR_ERROR(err);
    }

    if (0 == op_ret)
    {
        length = length > sizeof(struct man_ip_tunnel_s) ? sizeof(struct man_ip_tunnel_s) : length;
        memcpy(entry, p_string, length);
    }

    return op_ret;
}

unsigned int man_show_interface_tunnel_id(unsigned int id, struct man_ip_tunnel_s* entry)
{
    return man_show_interface_tunnel(id, entry, NPD_DBUS_METHOD_TUNNEL_SHOW_ID);
}

unsigned int man_show_interface_tunnel_next(struct man_ip_tunnel_s* entry)
{
    return man_show_interface_tunnel(entry->id, entry, NPD_DBUS_METHOD_TUNNEL_SHOW_NEXT);
}


#endif

