
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_SMART_LINK
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "dbus/smartlink/smart_link_dbus_def.h"
#include "dbus/npd/npd_dbus_def.h"
#include "man_smart_link.h"

extern DBusConnection* config_dbus_connection;


void man_smart_link_make_vlan_list_string(int length, short* list, char* buf)
{
    int ni = 0;
    int start = 0;
    int end = 0;
    int temp = 0;
    int current_len = 0;
    int real_length = 0;

    for (ni = 0; ni < length; ni++)
    {
        if (0 != list[ni])
        {
            real_length++;
        }
        else
        {
            break;
        }
    }

    for (ni = 0; ni < real_length; ni++)
    {
        start = list[ni];
        if (((ni + 1) < real_length) && ((start + 1) == list[ni + 1]))
        {
            temp = start;
            while (((ni + 1) < real_length) && ((temp + 1) == list[ni + 1]))
            {
                end = list[ni + 1];
                temp = end;
                ni++;
            }

            if ((ni + 1) < real_length)
            {
                current_len += sprintf(buf + current_len, "%d-%d,", start, end);
            }
            else
            {
                current_len += sprintf(buf + current_len, "%d-%d", start, end);
            }
        }
        else
        {
            if ((ni + 1) < real_length)
            {
                current_len += sprintf(buf + current_len, "%d,", start);
            }
            else
            {
                current_len += sprintf(buf + current_len, "%d", start);
            }
        }
    }

    return ;
}


#define MAN_SMART_LINK_DBUS_MACRO_METHOD(query, p_method)       \
do {                                                            \
	query = dbus_message_new_method_call(                       \
                                	SMART_LINK_DBUS_BUSNAME,    \
                                	SMART_LINK_DBUS_OBJPATH,    \
                                	SMART_LINK_DBUS_INTERFACE,  \
                                	p_method);                  \
} while (0)

#define MAN_SMART_LINK_DBUS_FOR_ERROR(err)             \
	printf("failed get reply.\n");                  \
	if (dbus_error_is_set(&err))                    \
    {                                               \
    	printf("%s raised: %s\n", err.name, err.message);       \
    	dbus_error_free(&err);                      \
    }                                               \
	return -1

#define MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret)    \
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
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);    \
    }                                       \
                                            \    
	if (!dbus_message_get_args(reply, &err, \
                        	DBUS_TYPE_UINT32, &op_ret,          \
                        	DBUS_TYPE_INVALID))                 \
    {                                       \
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);    \
    }                                       \
                                            \
} while (0)

unsigned int man_smart_link_dbus_u32(unsigned int value, unsigned char* p_method)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_SMART_LINK_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value,
                         DBUS_TYPE_INVALID);

    MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_smart_link_dbus_2_u32
(
    unsigned int value_1,
    unsigned int value_2,
    unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_SMART_LINK_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value_1,
                         DBUS_TYPE_UINT32, &value_2,
                         DBUS_TYPE_INVALID);

    MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_smart_link_dbus_5_u32
(
    unsigned int value_1,
    unsigned int value_2,
    unsigned int value_3,
    unsigned int value_4,
    unsigned int value_5,
    unsigned char* p_method
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_SMART_LINK_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &value_1,
                         DBUS_TYPE_UINT32, &value_2,
                         DBUS_TYPE_UINT32, &value_3,
                         DBUS_TYPE_UINT32, &value_4,
                         DBUS_TYPE_UINT32, &value_5,
                         DBUS_TYPE_INVALID);

    MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_smart_link_msti
(
    unsigned int id,
    unsigned int instance,
    unsigned int length,
    unsigned char* vlan_bitmap
)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    MAN_SMART_LINK_DBUS_MACRO_METHOD(query, SMART_LINK_DBUS_METHOD_MSTI);
	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &id,
                         DBUS_TYPE_UINT32, &instance,
                         DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &vlan_bitmap, length,
                         DBUS_TYPE_INVALID);

    MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_smart_link_group_get(struct man_smart_link_request_s* entry)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;

    dbus_error_init(&err);

    MAN_SMART_LINK_DBUS_MACRO_METHOD(query, SMART_LINK_DBUS_METHOD_GROUP_GET);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &entry->id,
                         DBUS_TYPE_INVALID);


    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_UINT32, &entry->id,
                        	DBUS_TYPE_UINT32, &entry->is_enable,
                        	DBUS_TYPE_UINT32, &entry->is_preempt,
                        	DBUS_TYPE_UINT32, &entry->instance,
                        	DBUS_TYPE_UINT32, &entry->master_port,
                        	DBUS_TYPE_UINT32, &entry->master_index,
                        	DBUS_TYPE_UINT32, &entry->master_status,
                        	DBUS_TYPE_UINT32, &entry->slave_index,
                        	DBUS_TYPE_UINT32, &entry->slave_status,
                        	DBUS_TYPE_UINT32, &entry->advertise_vlan,
                        	DBUS_TYPE_INVALID))
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    } 

    return op_ret;
}

unsigned int man_smart_link_group(unsigned int level)
{
    return man_smart_link_dbus_u32(level, SMART_LINK_DBUS_METHOD_GROUP);
}

unsigned int man_smart_link_no_group(unsigned int level)
{
    return man_smart_link_dbus_u32(level, SMART_LINK_DBUS_METHOD_NO_GROUP);
}

unsigned int man_smart_link_log_level_set(unsigned int level)
{
    return man_smart_link_dbus_u32(level, SMART_LINK_DBUS_METHOD_LOG_TYPE_SET);
}

unsigned int man_smart_link_log_level_unset(unsigned int level)
{
    return man_smart_link_dbus_u32(level, SMART_LINK_DBUS_METHOD_LOG_TYPE_UNSET);
}

unsigned int man_smart_link_enable(unsigned int id, unsigned int is_enable)
{
    return man_smart_link_dbus_2_u32(id, is_enable, SMART_LINK_DBUS_METHOD_ENABLE);
}

unsigned int man_smart_link_preempt(unsigned int id, unsigned int is_enable)
{
    return man_smart_link_dbus_2_u32(id, is_enable, SMART_LINK_DBUS_METHOD_PREEMPT);
}

unsigned int man_smart_link_advertise_vlan(unsigned int id, unsigned int advertise_vlan)
{
    return man_smart_link_dbus_2_u32(id, advertise_vlan, SMART_LINK_DBUS_METHOD_ADVERTISE_VLAN);
}

unsigned int man_smart_link_port
(
    unsigned int id,
    unsigned int netif_m_idx,
    unsigned int netif_m_status,
    unsigned int netif_s_idx,
    unsigned int netif_s_status
)
{
    return man_smart_link_dbus_5_u32
        (id, netif_m_idx, netif_m_status, netif_s_idx, netif_s_status, SMART_LINK_DBUS_METHOD_PORT);
}

int man_smart_link_get_netif_index(char* arg)
{
    unsigned int index = 0;
    char* endptr = NULL;

    index = (unsigned int)strtoul(arg, &endptr, 10);
    if ('\0' == *endptr)
    {
        index = npd_netif_trunk_get_index(index);
    }
    else
    {
        index = parse_str_to_eth_index(arg);
    }

    return index;
}

int man_smart_link_npd_u32(unsigned int netif_index, char* p_method)
{
    unsigned int ret = 0;
    DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
    DBusError err;  


	query = dbus_message_new_method_call(
                                	NPD_DBUS_BUSNAME,
                                	NPD_DBUS_SMART_LINK_OBJPATH,
                                	NPD_DBUS_SMART_LINK_INTERFACE,
                                	p_method);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &netif_index,
                         DBUS_TYPE_INVALID);

    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                            DBUS_TYPE_UINT32, &ret,
                        	DBUS_TYPE_INVALID))
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    } 

    return ret;
}

int man_smart_link_get_netif_status(unsigned int netif_index, unsigned int* netif_status)
{
    int ret = 0;
    
    ret =  man_smart_link_npd_u32(netif_index, NPD_DBUS_SMART_LINK_INFORMATION);

    if (ret < 0)
    {
        return -1;
    }
    else
    {
        *netif_status = ret ? SL_PORT_UP : SL_PORT_DOWN;
    }
    
    return 0;
}

int man_smart_link_port_adv_vlan(unsigned int netif_index, short* adv_vlan_list, int length)
{
    unsigned int op_ret = 0;
	DBusMessage* query = NULL;

    query = dbus_message_new_method_call(
                                	NPD_DBUS_BUSNAME,
                                	NPD_DBUS_SMART_LINK_OBJPATH,
                                	NPD_DBUS_SMART_LINK_INTERFACE,
                                	NPD_DBUS_SMART_LINK_ADV_VLAN_LIST);
	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &netif_index,
                         DBUS_TYPE_ARRAY, DBUS_TYPE_UINT16, &adv_vlan_list, length,
                         DBUS_TYPE_INVALID);

    MAN_SMART_LINK_DBUS_MACRO_RAW(query, op_ret);

    return op_ret;
}

unsigned int man_smart_link_port_adv_vlan_get(struct man_smart_link_port_list_s* entry)
{
    unsigned int op_ret = 0;
    int length = 0;
	DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err;
    short* p_adv_vlan_list = NULL;

    dbus_error_init(&err);

    query = dbus_message_new_method_call(
                            	NPD_DBUS_BUSNAME,
                            	NPD_DBUS_SMART_LINK_OBJPATH,
                            	NPD_DBUS_SMART_LINK_INTERFACE,
                            	NPD_DBUS_SMART_LINK_ADV_VLAN_LIST_GET);

	dbus_message_append_args(query,
                         DBUS_TYPE_UINT32, &entry->netif_index,
                         DBUS_TYPE_INVALID);


    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    }

	if (!dbus_message_get_args(reply, &err,
                        	DBUS_TYPE_UINT32, &op_ret,
                        	DBUS_TYPE_UINT32, &entry->netif_index,
                        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT16, &p_adv_vlan_list, &length,
                        	DBUS_TYPE_INVALID))
    {
    	MAN_SMART_LINK_DBUS_FOR_ERROR(err);
    }

    if (0 == op_ret)
    {
        length = length > MAN_ADV_VLAN_LIST_MAX ? MAN_ADV_VLAN_LIST_MAX : length;
        memcpy(entry->adv_vlan_list, p_adv_vlan_list, length * sizeof(short));
    }

    return op_ret;
}


#endif
