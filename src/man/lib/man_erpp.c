
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_ERPP
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include <erpp/erpp_dbus.h>

extern DBusConnection* config_dbus_connection;

void man_erpp_make_vlan_list_string(int length, short* list, char* buf)
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


#define MAN_ERPP_DBUS_MACRO_METHOD(query, p_method)       \
do {															\
	query = dbus_message_new_method_call(						\
									ERPP_DBUS_BUSNAME,	\
									ERPP_DBUS_OBJPATH,	\
									ERPP_DBUS_INTERFACE,	\
									p_method);					\
} while (0)

#define MAN_ERPP_DBUS_FOR_ERROR(err)             \
	printf("failed get reply.\n");					\
	if (dbus_error_is_set(&err))					\
	{												\
		printf("%s raised: %s\n", err.name, err.message);		\
		dbus_error_free(&err);						\
	}												\
	return -1

#define MAN_ERPP_DBUS_MACRO_RAW(query, op_ret)    \
do											\
{											\
	DBusMessage* reply = NULL;				\
	DBusError err;							\
											\
	dbus_error_init(&err);					\
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);	\
											\
	dbus_message_unref(query);				\
	if (NULL == reply)						\
	{										\
		MAN_ERPP_DBUS_FOR_ERROR(err);    \
	}										\
											\	 
	if (!dbus_message_get_args(reply, &err, \
							DBUS_TYPE_UINT32, &op_ret,			\
							DBUS_TYPE_INVALID)) 				\
	{										\
		MAN_ERPP_DBUS_FOR_ERROR(err);    \
	}										\
											\
} while (0)

unsigned int man_erpp_dbus_domain_iscreate(unsigned int domain, unsigned int iscreate, char* p_method)
{
	unsigned int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &domain,
						 DBUS_TYPE_UINT32, &iscreate,
						 DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);
	
	return op_ret;
}

unsigned int man_erpp_dbus_domain_binding_control_vlan(unsigned int domain, unsigned short vid, char* p_method)
{
	unsigned int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &domain,
						 DBUS_TYPE_UINT16, &vid,
						 DBUS_TYPE_INVALID);

	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}

unsigned int man_erpp_dbus_domain_binding_instance(unsigned int domain, unsigned int instance, char* p_method)
{
	unsigned int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &domain,
						 DBUS_TYPE_UINT32, &instance,
						 DBUS_TYPE_INVALID);

	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}

unsigned int man_erpp_dbus_ring_configure
(
	unsigned int domain_id,
	unsigned int ring_id,
    unsigned char node_mode, 
    unsigned int primary_port, 
    unsigned int secondary_port, 
    unsigned int level,
    char *p_method
)
{
	unsigned int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);
	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &domain_id,
						 DBUS_TYPE_UINT32, &ring_id,
						 DBUS_TYPE_BYTE,   &node_mode,
						 DBUS_TYPE_UINT32, &primary_port,
						 DBUS_TYPE_UINT32, &secondary_port,
                         DBUS_TYPE_UINT32, &level,
                         DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}

unsigned int man_erpp_dbus_ring_isenable(unsigned int domain, unsigned int ring_id, unsigned int isenable, char* p_method)
{
	int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &domain,
						 DBUS_TYPE_UINT32, &ring_id,
		                 DBUS_TYPE_UINT32, &isenable,
						 DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);


	return op_ret;
}

unsigned int man_erpp_dbus_enable(unsigned int isenable, char* p_method)
{
	int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
		                 DBUS_TYPE_UINT32, &isenable,
						 DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}
unsigned int man_erpp_dbus_timer_set(unsigned int domain_id, unsigned int hello_value, unsigned int fail_value, char* p_method)
{
	int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
		                 DBUS_TYPE_UINT32, &domain_id,
						 DBUS_TYPE_UINT32, &hello_value,
						 DBUS_TYPE_UINT32, &fail_value,
						 DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}

unsigned int man_erpp_dbus_info_get(struct erpp_domain_s *erpp_info, unsigned int ring_id, char* p_method)
{
	unsigned int op_ret = 0;
	DBusMessage* reply = NULL;	
	DBusMessage* query = NULL;
	DBusError err;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);
		
	dbus_message_append_args(query,		                 
		                 DBUS_TYPE_UINT32, &erpp_info->domain_id,
						 DBUS_TYPE_UINT32, &ring_id,
						 DBUS_TYPE_INVALID);
	dbus_error_init(&err);	
    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
    {
    	MAN_ERPP_DBUS_FOR_ERROR(err);
    }

    if (!dbus_message_get_args(reply, &err,
                            DBUS_TYPE_UINT32, &op_ret,
                            DBUS_TYPE_UINT32, &erpp_info->domain_id,
                            DBUS_TYPE_UINT32, &erpp_info->protect_instance_id,
                            DBUS_TYPE_UINT16, &erpp_info->control_vlan_id[0],
                            DBUS_TYPE_UINT32, &erpp_info->hello_timer,
                            DBUS_TYPE_UINT32, &erpp_info->fail_timer,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.erpp_node_role,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[0].netif_index,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[1].netif_index,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[0].erpp_port_role,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[1].erpp_port_role,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[0].erpp_port_status,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].node.port[1].erpp_port_status,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].is_enable,
                            DBUS_TYPE_UINT32, &erpp_info->ring[ring_id-1].level,
							DBUS_TYPE_INVALID))
    {
    	MAN_ERPP_DBUS_FOR_ERROR(err);
    } 


	return op_ret;
}

unsigned int man_erpp_dbus_log_level_set(unsigned char log_level, char* p_method)
{
	unsigned int op_ret = 0;
	DBusMessage* query = NULL;

	MAN_ERPP_DBUS_MACRO_METHOD(query, p_method);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,  &log_level,
						 DBUS_TYPE_INVALID);
	
	MAN_ERPP_DBUS_MACRO_RAW(query, op_ret);

	return op_ret;
}

unsigned int man_erpp_domain_create(unsigned int domain, unsigned int iscreate)
{
	return man_erpp_dbus_domain_iscreate(domain, iscreate, ERPP_DBUS_METHOD_DOMAIN);
}

unsigned int man_erpp_domain_binding_control_vid(unsigned int domain, unsigned short vid)
{
	return man_erpp_dbus_domain_binding_control_vlan(domain, vid, ERPP_DBUS_METHOD_BIND_CONTRIL_VLAN);
}

unsigned int man_erpp_domain_binding_instance(unsigned int domain, unsigned int instance)
{
	return man_erpp_dbus_domain_binding_instance(domain, instance, ERPP_DBUS_METHOD_BIND_INSTANCE);
}

unsigned int man_erpp_ring_configure
(
    unsigned int domain_id, 
    unsigned int ring_id, 
    unsigned char node_mode, 
    unsigned int primary_port, 
    unsigned int secondary_port, 
    unsigned int level
)
{
	return man_erpp_dbus_ring_configure(domain_id, ring_id, node_mode, primary_port, secondary_port, level, ERPP_DBUS_METHOD_RING_CONFIGURE);
}

unsigned int man_erpp_ring_isenable
(
	unsigned int domain_id,
	unsigned int ring_id,
	unsigned int isenable
)
{
	return man_erpp_dbus_ring_isenable(domain_id, ring_id, isenable, ERPP_DBUS_METHOD_RING_ENABLE);
}

unsigned int man_erpp_isenable(unsigned int is_enable)
{
	return man_erpp_dbus_enable(is_enable, ERPP_DBUS_METHOD_ENABLE);
}

unsigned int man_erpp_timer_set(unsigned int domain_id, unsigned int hello_value, unsigned int fail_value)
{
	return man_erpp_dbus_timer_set(domain_id, hello_value, fail_value, ERPP_DBUS_METHOD_TIMER_SET);
}

unsigned int man_erpp_info_get(struct erpp_domain_s *erpp_info, unsigned int ring_id)
{
	return man_erpp_dbus_info_get(erpp_info, ring_id, ERPP_DBUS_METHOD_INFO_GET);
}

int man_erpp_set_log_level(unsigned char log_level)
{
	return man_erpp_dbus_log_level_set(log_level, ERPP_DBUS_METHOD_LOG_SET);
}

#endif

