
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef __cplusplus
extern "C"
{
#endif
#if defined (HAVE_VRRP) || defined (HAVE_SMART_LINK)
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "dbus/npd/npd_dbus_def.h"
#include "man_tracking.h"

extern DBusConnection *config_dbus_connection;

int man_tracking_u32(unsigned int value, unsigned int* op_ret, char* p_method)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_TRACK_OBJPATH,
                                         NPD_DBUS_TRACK_INTERFACE,
                                         p_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
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
	dbus_message_unref(reply);
	
	return 0;
}

int man_tracking_2_u32
(
    unsigned int value_1,
    unsigned int value_2,
    unsigned int* op_ret,
    char* p_method
)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_TRACK_OBJPATH,
                                         NPD_DBUS_TRACK_INTERFACE,
                                         p_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value_1,
							 DBUS_TYPE_UINT32, &value_2,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
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
	dbus_message_unref(reply);
	
	return 0;
}

int man_tracking_3_u32
(
    unsigned int value_1,
    unsigned int value_2,
    unsigned int value_3,
    unsigned int* op_ret,
    char* p_method
)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_TRACK_OBJPATH,
                                         NPD_DBUS_TRACK_INTERFACE,
                                         p_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value_1,
							 DBUS_TYPE_UINT32, &value_2,
							 DBUS_TYPE_UINT32, &value_3,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
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
	dbus_message_unref(reply);
	
	return 0;
}

int man_tracking_interface_is_exist(unsigned int netif_index, unsigned int* op_ret)
{
    return man_tracking_u32(netif_index, op_ret, NPD_DBUS_METHOD_TRACK_GROUP_OBJECT_L3EXIST);
}

int man_tracking_netif_is_exist(unsigned int netif_index, unsigned int* op_ret)
{
    return man_tracking_u32(netif_index, op_ret, NPD_DBUS_METHOD_TRACK_GROUP_OBJECT_L2EXIST);
}

int man_tracking_group(unsigned int tracking_group, unsigned int* op_ret)
{
    return man_tracking_u32(tracking_group, op_ret, NPD_DBUS_METHOD_TRACK_GROUP);
}

int man_tracking_group_delete(unsigned int tracking_group, unsigned int* op_ret)
{
    return man_tracking_u32(tracking_group, op_ret, NPD_DBUS_METHOD_TRACK_GROUP_DELETE);
}

int man_tracking_object(unsigned int tracking_group, unsigned int tracking_object, unsigned int tracking_object_layer, unsigned int* op_ret)
{
    return man_tracking_3_u32(tracking_group,
                                tracking_object,
                                tracking_object_layer,
                                op_ret,
                                NPD_DBUS_METHOD_TRACK_GROUP_ADD_OBJECT
                                );
}

int man_tracking_object_delete(unsigned int tracking_group, unsigned int tracking_object, unsigned int tracking_object_layer, unsigned int* op_ret)
{
    return man_tracking_3_u32(tracking_group,
                                tracking_object,
                                tracking_object_layer,
                                op_ret,
                                NPD_DBUS_METHOD_TRACK_GROUP_REMOVE_OBJECT
                                );
}

int man_tracking_action_down(unsigned int tracking_group, unsigned int netif_index, unsigned int* op_ret)
{
    return man_tracking_2_u32(tracking_group,
                                netif_index,
                                op_ret,
                                NPD_DBUS_METHOD_TRACK_GROUP_ACTION_DOWN
                                );
}

int man_tracking_group_mode(unsigned int tracking_group, unsigned int tracking_mode, unsigned int* op_ret)
{
    return man_tracking_2_u32(tracking_group,
                                tracking_mode,
                                op_ret,
                                NPD_DBUS_METHOD_TRACK_GROUP_MODE
                                );
}

int man_tracking_group_get
(
    unsigned int tracking_group,
    struct track_group_query* tracking_query,
    unsigned int* op_ret,
    char* p_method
)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    int ni = 0;
    unsigned int    length_1 = 0;
    unsigned int    length_2 = 0;
    unsigned int    length_3 = 0;
    unsigned int*   ptr_netif_index = NULL;
    unsigned int*   ptr_netif_index_layer = NULL;
    unsigned int*   ptr_status = NULL;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_TRACK_OBJPATH,
                                         NPD_DBUS_TRACK_INTERFACE,
                                         p_method);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &tracking_group,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
        	DBUS_TYPE_UINT32, op_ret,
        	DBUS_TYPE_UINT32, &tracking_query->tracking_group_id,
        	DBUS_TYPE_UINT32, &tracking_query->tracking_mode,
        	DBUS_TYPE_UINT32, &tracking_query->tracking_count,
        	DBUS_TYPE_UINT32, &tracking_query->tracking_up_count,
        	DBUS_TYPE_UINT32, &tracking_query->tracking_event,
        	DBUS_TYPE_UINT32, &tracking_query->dl_index,	
        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_netif_index, &length_1,
            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_netif_index_layer, &length_2,
        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_status, &length_3,
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

    for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
    {
        tracking_query->tracking_object[ni].netif_index = ptr_netif_index[ni];
        tracking_query->tracking_object[ni].netif_index_layer= ptr_netif_index_layer[ni];
        tracking_query->tracking_object[ni].status = ptr_status[ni];
    }
    
	dbus_message_unref(reply);
	
	return 0;
}

int man_tracking_group_query(unsigned int tracking_group, struct track_group_query* tracking_query, unsigned int* op_ret)
{
    return man_tracking_group_get(tracking_group, tracking_query, op_ret, NPD_DBUS_METHOD_TRACK_GROUP_GET);
}

int man_tracking_group_query_next(unsigned int tracking_group, struct track_group_query* tracking_query, unsigned int* op_ret)
{
    return man_tracking_group_get(tracking_group, tracking_query, op_ret, NPD_DBUS_METHOD_TRACK_GROUP_GET_NEXT);
}

int man_tracking_group_status(unsigned int tracking_group, unsigned int* tracking_query, unsigned int* op_ret)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_TRACK_OBJPATH,
                                         NPD_DBUS_TRACK_INTERFACE,
                                         NPD_DBUS_METHOD_TRACK_GROUP_STATUS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &tracking_group,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_UINT32, tracking_query,
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
	dbus_message_unref(reply);
	
	return 0;
}
#endif
#ifdef __cplusplus
}
#endif


