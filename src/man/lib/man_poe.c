
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
#ifdef HAVE_POE
#include "lib/osinc.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/npd_intf.h"
#include "npd/npd_poe.h"
#include "man_str_parse.h"
#include "man_poe.h"

extern DBusConnection *dcli_dbus_connection;
unsigned int dcli_create_poe_time_range(unsigned int index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
    unsigned int op_ret;
	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_POE_CREATE_TIME_RANGE);

	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

    if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}

    if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;	
}

unsigned int dcli_delete_poe_time_range(unsigned int index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
    unsigned int op_ret;
	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_POE_DELETE_TIME_RANGE);

	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

    if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}

    if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;	
}

int dcli_add_poe_time_range(unsigned int index,char *start_time,char *end_time,unsigned int flag,unsigned int *op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_POE_ADD_TIME_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_STRING, &start_time,
								DBUS_TYPE_STRING, &end_time,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}

	dbus_message_unref(reply);
	return TRUE;

}
int dcli_no_poe_time_info(unsigned int index, unsigned int flag, unsigned int *op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_POE_NO_TIME_INFO);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}

	dbus_message_unref(reply);
	return TRUE;
}
int dcli_port_poe_time_endis(unsigned int port_index, unsigned int index)
{
	unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_POE_ON_TIME_DEPLOY);
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_show_poe_time_range(char* show_str)
{
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    char*           tmp = NULL;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_SHOW_POE_TIME_RANGE);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, POE_SHOW_TIME_SIZE);
    
	dbus_message_unref(reply);

	return TRUE; 
}
int dcli_show_poe_time_range_info(unsigned int index, char** time_range_info,unsigned int* op_ret, unsigned int *count)
{
	char*			temp_info;
	unsigned int	uni = 0;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
							    NPD_DBUS_POE_METHOD_SHOW_POE_TIME_INFO);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, op_ret);

    if(*op_ret == 0)
    	for (uni = 0; uni < 8; uni++)
    	{
    		dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter, &temp_info);
    		strcpy(time_range_info[uni], temp_info);
    	}

	dbus_message_unref(reply);
	return TRUE; 
}
int dcli_show_poe_time_range_bind(unsigned int index, char* acl_bind_show_str)
{
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    char *          tmp = NULL;
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
							    NPD_DBUS_POE_METHOD_SHOW_POE_TIME_BIND_INFO);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(acl_bind_show_str, tmp, POE_SHOW_TIME_SIZE);
	dbus_message_unref(reply);
	return TRUE; 
}
int dcli_global_poe_endis(unsigned char isEnable)
{
	unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_GLOBAL_POE_ENDIS);
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;	
}

int dcli_port_poe_endis(unsigned int port_index, unsigned char isEnable)
{
	unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_ENDIS);
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_BYTE,   &isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_max_power_config(unsigned int port_index, unsigned int max_power)
{
	unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_POE_OBJPATH,\
									NPD_DBUS_POE_INTERFACE,\
									NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_MAX_POWER);
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_UINT32, &max_power,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_priority_config(unsigned int port_index, unsigned char port_poe_priority)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_PRIORITY);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_BYTE,   &port_poe_priority,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_power_manage_mode_config(unsigned char poe_power_manage)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_POE_POWER_MANAGE);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,   &poe_power_manage,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_interface_mode_config(unsigned int port_index, unsigned char poe_mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_MODE);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_BYTE,   &poe_mode,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_interface_power_mode_config(unsigned int port_index, unsigned char power_mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_POWER_MODE);								
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_BYTE,   &power_mode,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_poe_interface_power_up_mode_config(unsigned int port_index, unsigned char power_up_mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_POE_INTERFACE_POWER_UP_MODE);								
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_BYTE,   &power_up_mode,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_port_poe_config_legacy_check(unsigned int netif_index, unsigned char is_enable)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_CONFIG_PORT_POE_LEGACY_CHECK);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &netif_index, 
						DBUS_TYPE_BYTE,   &is_enable,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_get_poe_interface(unsigned int port_index, poe_intf_db_t *poe_intf)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_GET_POE_INTERFACE);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,   &port_index,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);     
	if (op_ret == POE_RETURN_CODE_ERR_NONE)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->poe_id)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->poe_mode)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->admin)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->detect_status));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->operate_status));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->pd_type)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->pd_class));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->priority));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->max_power)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &poe_intf->power_user); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->current)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->voltage)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->temperature)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->legacy_dect)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->power_up_mode)); 
	}
	poe_intf->netif_index = port_index;
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_poe_get_next_interface(unsigned int port_index, poe_intf_db_t *poe_intf)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_POE_GET_NEXT_INTERFACE);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,   &port_index,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);   
	if (op_ret == POE_RETURN_CODE_ERR_NONE)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->netif_index)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->poe_id)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->poe_mode)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->admin)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->detect_status));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->operate_status));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->pd_type)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->pd_class));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->priority));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->max_power)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &poe_intf->power_user); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->current)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->voltage)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->temperature)); 
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter, &(poe_intf->legacy_dect)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_intf->power_up_mode)); 
	}
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_get_poe_pse(unsigned int pse_id, poe_pse_db_t *poe_pse)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,\
								NPD_DBUS_POE_OBJPATH,\
								NPD_DBUS_POE_INTERFACE,\
								NPD_DBUS_POE_METHOD_POE_GET_PSE);
	dbus_error_init(&error);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &pse_id,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return POE_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);   
	if (op_ret == POE_RETURN_CODE_ERR_NONE)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->slot_no)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->admin)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->pse_type)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->max_power)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->available)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->current)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->voltage)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->mode)); 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(poe_pse->guardBand)); 
	}
	poe_pse->pse_id = pse_id;
	dbus_message_unref(reply);
    return op_ret;
}

#endif
#ifdef __cplusplus
}
#endif

