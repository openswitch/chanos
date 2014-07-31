
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_qos.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		CLI definition for QOS module.
*
* DATE:
*		05/30/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.56 $	
*******************************************************************************/
#ifdef HAVE_QOS
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "dbus/npd/npd_dbus_def.h"
#include "man_acl.h"
#include "man_qos.h"

extern DBusConnection *config_dbus_connection;

#if 0
int dcli_qos_show_running_config()
{	 
	 char*	showStr = NULL;
	 int	ret = 1;	 
	 DBusMessage*	query = NULL;
	 DBusMessage*	reply = NULL;
	 DBusError		err;
	 
	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
										 	NPD_DBUS_QOS_OBJPATH, \
										 	NPD_DBUS_QOS_INTERFACE, \
										 	NPD_DBUS_METHOD_SHOW_QOS_RUNNIG_CONFIG);
 	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
 
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return 1;
	}
 
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "QOS SHOW RUNNING");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 0;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = 1;
	}
 
	dbus_message_unref(reply);
	
	return ret; 
}
#endif

int dcli_qos_qosprofile_create(unsigned int index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_CONFIG_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	
	return QOS_TRUE;
}

int dcli_qos_qosprofile_attribute_config(unsigned int index, unsigned int param0, unsigned int att_flag, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_QOS_PROFILE_ATTRIBUTE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &index,     
							 DBUS_TYPE_UINT32, &param0,
						     DBUS_TYPE_UINT32, &att_flag,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_no_qosprofile(unsigned int index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	
	return QOS_TRUE;
}

int dcli_qos_profile_show(unsigned int index, QOS_PROFILE_STC* qos_profile, unsigned int* next_index)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	 
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 	NPD_DBUS_QOS_OBJPATH,
										 	NPD_DBUS_QOS_INTERFACE,
										 	NPD_DBUS_METHOD_SHOW_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, next_index,
		DBUS_TYPE_UINT32, &qos_profile->index,
		DBUS_TYPE_UINT32, &qos_profile->dropPrecedence,
		DBUS_TYPE_UINT32, &qos_profile->userPriority,
		DBUS_TYPE_UINT32, &qos_profile->trafficClass,
		DBUS_TYPE_UINT32, &qos_profile->dscp,
		DBUS_TYPE_UINT32, &qos_profile->exp,
		DBUS_TYPE_UINT32, &qos_profile->swPortNum,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_default_qos_profile(unsigned int remark, unsigned int eth_g_index, unsigned int qos_proflie, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DEFAULT_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32, &qos_proflie,
						     DBUS_TYPE_UINT32, &eth_g_index,
						     DBUS_TYPE_UINT32, &remark,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}
int dcli_no_default_qos_profile(unsigned int index, unsigned int eth_g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DEFAULT_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32, &index,
						     DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}
int dcli_qos_remark(unsigned int eth_g_index, int dir, int remark, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_PORT_QOS_REMARK);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32, &remark,
						     DBUS_TYPE_UINT32, &dir,
						     DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_no_dscp_to_profile(unsigned int index, unsigned int dscp, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32, &dscp,
						     DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_dscp_to_profile(unsigned int ingress_dscp, unsigned int egress_dscp, unsigned int profile_index, unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32, &ingress_dscp,
						     DBUS_TYPE_UINT32, &egress_dscp,
						     DBUS_TYPE_UINT32, &profile_index, 
						     DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_no_dscp_to_dscp(unsigned int policymap_index, unsigned int dscp, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policymap_index,
                             DBUS_TYPE_UINT32, &dscp,                              
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_dscp_to_dscp(unsigned int policymap_index, unsigned int old_dscp, unsigned int new_dscp, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policymap_index,
                             DBUS_TYPE_UINT32, &old_dscp,     						
						     DBUS_TYPE_UINT32, &new_dscp,						     
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
		return QOS_FALSE;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_no_up_to_profile(unsigned int policymap_index, unsigned int up, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &up,    
                             DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_up_to_profile(unsigned int ingress_up, unsigned egress_up, unsigned int profile_index, unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &ingress_up,
                             DBUS_TYPE_UINT32, &egress_up,
						     DBUS_TYPE_UINT32, &profile_index,
						     DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_profile_to_profile(unsigned int profile_index, unsigned remarkParamValue, unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PROFILE_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &profile_index,
                             DBUS_TYPE_UINT32, &remarkParamValue,
						     DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_no_policymap(unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	
	return QOS_TRUE;
}

int dcli_qos_policymap_create(unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_QOS_OBJPATH,
										NPD_DBUS_QOS_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	
	return QOS_TRUE;
}

int dcli_qos_policymap_qosmarker_enable(unsigned int policymap_index, unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_MODIFY_MARK_QOS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &is_enable,  
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_trust_model2(unsigned int policymap_index, unsigned int eth_g_index, unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &is_enable,  
                             DBUS_TYPE_UINT32, &policymap_index,
                             DBUS_TYPE_UINT32, &eth_g_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_trust_l3dscp_mode(unsigned int policymap_index, unsigned int eth_g_index, unsigned int dscp_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L3DSCP_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &dscp_enable,
                             DBUS_TYPE_UINT32, &eth_g_index,
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_trust_l3remap_mode(unsigned int policymap_index, unsigned int remap_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L3REMAP_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &remap_enable,
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_trust_model2l3(unsigned int policymap_index, unsigned int up_enable, unsigned int dscp_enable, unsigned int remap_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_L3_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &up_enable,
                             DBUS_TYPE_UINT32, &dscp_enable,  
                             DBUS_TYPE_UINT32, &remap_enable,
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_eth_bind_policymap(unsigned int eth_g_index, unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_ETHPORTS_OBJPATH,
                                         NPD_DBUS_ETHPORTS_INTERFACE,
                                         NPD_DBUS_ETHPORTS_METHOD_BIND_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32, &eth_g_index,
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_eth_unbind_policymap(unsigned int eth_g_index, unsigned int policymap_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_ETHPORTS_OBJPATH,
										 NPD_DBUS_ETHPORTS_INTERFACE,
										 NPD_DBUS_ETHPORTS_METHOD_UNBIND_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32, &eth_g_index,
                             DBUS_TYPE_UINT32, &policymap_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_eth_policymap_show(unsigned int g_index, struct qos_port_show_s* show, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ETHPORTS_OBJPATH,
										NPD_DBUS_ETHPORTS_INTERFACE,
										NPD_DBUS_ETHPORT_METHOD_SHOW_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
					     DBUS_TYPE_UINT32, &g_index,
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
		return QOS_FALSE;
	}
    
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_UINT32, &show->egressMark,
		DBUS_TYPE_UINT32, &show->ingressMark,
		DBUS_TYPE_UINT32, &show->qosProfileIndex,
		DBUS_TYPE_UINT32, &show->poMapId,
		DBUS_TYPE_UINT32, &show->trust,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policymap_show(unsigned int policymap_index, unsigned int* next_index, struct qos_policy_show_s* policymap)
{
    unsigned int    unlen = 0;
    unsigned int*   ppbmp = NULL;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    DBusMessageIter iter;
    int i;
	 
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
									 	NPD_DBUS_QOS_OBJPATH,
									 	NPD_DBUS_QOS_INTERFACE,
									 	NPD_DBUS_METHOD_SHOW_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policymap_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}


	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, next_index);
    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &policymap->index);
    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &policymap->domain);
    
	for (i = 0; i < 8; i++)
	{
        dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &policymap->in_up_map[i].profile_id);
	}
    /*
	for (i = 0; i < 8; i++)
	{
        dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &policymap->out_up_map[i].profile_id);
	}
	*/
	for (i = 0; i < 64; i++)
	{
        dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &policymap->in_dscp_map[i].profile_id);
	}
    /*
	for (i = 0; i < 64; i++)
	{
        dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &policymap->out_dscp_map[i].profile_id);
	}
	*/
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_create(unsigned int policer_index, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policer_index,
							 DBUS_TYPE_UINT32, &dir,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_delete(unsigned int policer_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policer_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_cir_cbs_set(QOS_POLICER_STC policer, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											 NPD_DBUS_QOS_OBJPATH,
											 NPD_DBUS_QOS_INTERFACE,
											 NPD_DBUS_METHOD_CONFIG_CIR_CBS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32, &policer.index,
		 					 DBUS_TYPE_UINT32, &dir,
							 DBUS_TYPE_UINT32, &policer.cir,
							 DBUS_TYPE_UINT32, &policer.cbs,
							 DBUS_TYPE_UINT32, &policer.pir,
							 DBUS_TYPE_UINT32, &policer.pbs,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_packetcolor_action_set(unsigned int policer_index, int packetcolor,unsigned int profileId)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

    int op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											 NPD_DBUS_QOS_OBJPATH,
											 NPD_DBUS_QOS_INTERFACE,
											 NPD_DBUS_METHOD_CONFIG_PACKETCOLOR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
	                         DBUS_TYPE_UINT32, &policer_index,
		 					 DBUS_TYPE_UINT32, &packetcolor,
							 DBUS_TYPE_UINT32, &profileId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;

}

int dcli_qos_policer_keep_drop(unsigned int policer_index, int color, unsigned int action, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_OUT_PROFILE_DROP_KEEP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policer_index,
							 DBUS_TYPE_UINT32, &color,
							 DBUS_TYPE_UINT32, &action,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_counter_set(unsigned int policer_index, unsigned int counter_index, unsigned is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32, &policer_index,
							 DBUS_TYPE_UINT32, &counter_index,
							 DBUS_TYPE_UINT32, &is_enable,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_counter_config(unsigned int counter_index, unsigned int inrange, unsigned int outrange, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query,						
							 DBUS_TYPE_UINT32, &counter_index,
							 DBUS_TYPE_UINT32, &inrange,
							 DBUS_TYPE_UINT32, &outrange,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_counter_show(unsigned int policerId, struct policer_stats_s *stats, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_RELAY_OBJPATH,
										 NPD_DBUS_RELAY_INTERFACE,
										 NPD_DBUS_METHOD_GET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policerId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_UINT64, &stats->conform_bytes,
		DBUS_TYPE_UINT64, &stats->conform_pkts,
		DBUS_TYPE_UINT64, &stats->exceed_bytes,
		DBUS_TYPE_UINT64, &stats->exceed_pkts,
		DBUS_TYPE_UINT64, &stats->violate_bytes,
		DBUS_TYPE_UINT64, &stats->violate_pkts,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_get(unsigned int policer_index, unsigned int* op_ret, QOS_POLICER_STC* policer)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    int ret;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GET_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &policer_index,
						 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_UINT32, &policer->index,
		DBUS_TYPE_UINT32, &policer->policerEnable,
		DBUS_TYPE_UINT32, &policer->cir,
		DBUS_TYPE_UINT32, &policer->cbs,
        DBUS_TYPE_UINT32, &policer->pir,
        DBUS_TYPE_UINT32, &policer->pbs,
        DBUS_TYPE_UINT32, &policer->gPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->yPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->rPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->yPktParam.cmd,
        DBUS_TYPE_UINT32, &policer->rPktParam.cmd,
        DBUS_TYPE_UINT32, &policer->counterEnable,
        DBUS_TYPE_UINT32, &policer->counterSetIndex,        
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_policer_show(unsigned int policer_index, unsigned int* next_index, QOS_POLICER_STC* policer)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32, &policer_index,
						 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return QOS_FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, next_index,
		DBUS_TYPE_UINT32, &policer->index,
		DBUS_TYPE_UINT32, &policer->policerEnable,
		DBUS_TYPE_UINT32, &policer->cir,
		DBUS_TYPE_UINT32, &policer->cbs,
        DBUS_TYPE_UINT32, &policer->pir,
        DBUS_TYPE_UINT32, &policer->pbs,
        DBUS_TYPE_UINT32, &policer->gPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->yPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->rPktParam.qosProfileID,
        DBUS_TYPE_UINT32, &policer->yPktParam.cmd,
        DBUS_TYPE_UINT32, &policer->rPktParam.cmd,
        DBUS_TYPE_UINT32, &policer->counterEnable,
        DBUS_TYPE_UINT32, &policer->counterSetIndex,        
        DBUS_TYPE_INVALID))
    {
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_queue_scheduler_mode(unsigned int port_mode, unsigned int eth_g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEQUE_SCH);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32, &port_mode,
							 DBUS_TYPE_UINT32, &eth_g_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_queue_drop_mode(unsigned int port_mode, unsigned int eth_g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEUE_DROP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32, &port_mode,
							 DBUS_TYPE_UINT32, &eth_g_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}



int dcli_qos_queue_weight_set(unsigned int tc, unsigned int eth_g_index, unsigned int weight, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_QOS_OBJPATH,
											NPD_DBUS_QOS_INTERFACE,
											NPD_DBUS_METHOD_QUEQUE_WRR_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &tc,
							 DBUS_TYPE_UINT32, &weight,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_queue_scheduler_default(unsigned int port_mode, unsigned int eth_g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DEFAULT_QUEQUE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32, &port_mode,
							 DBUS_TYPE_UINT32, &eth_g_index,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_queue_schduler_show(unsigned int eth_g_index, QOS_PORT_COS_CFG_STC* squeue_sch)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;
	unsigned int	uni = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_QUEUE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&eth_g_index,
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
		return QOS_FALSE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter, &squeue_sch->queue_type);

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);
	for (uni = 0; uni < MAX_COS_QUEUE_NUM; uni++) 
	{
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct, &squeue_sch->queue[uni].groupFlag);
		dbus_message_iter_next(&iter_struct); 
		dbus_message_iter_get_basic(&iter_struct, &squeue_sch->queue[uni].weight);
		dbus_message_iter_next(&iter_array);
	}
	
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_shape_port_queue(unsigned int eth_g_index, unsigned int alg_flag, unsigned int queue_id, unsigned int maxrate, unsigned int kmstate, unsigned burst, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_ETHPORTS_OBJPATH,
                                         NPD_DBUS_ETHPORTS_INTERFACE,
										 NPD_DBUS_METHOD_TRAFFIC_SHAPE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &alg_flag,
							 DBUS_TYPE_UINT32, &queue_id,
							 DBUS_TYPE_UINT32, &maxrate,
							 DBUS_TYPE_UINT32, &kmstate,
							 DBUS_TYPE_UINT32, &burst,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_shape_no_port_queue(unsigned int eth_g_index, unsigned int alg_flag, unsigned int queue_id, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
										 NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_DELETE_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &alg_flag,
							 DBUS_TYPE_UINT32, &queue_id,
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
		return QOS_FALSE;
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
		return QOS_FALSE;
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int dcli_qos_shape_port_show(unsigned int eth_g_index, unsigned int* op_ret, QOS_PORT_TC_CFG_STC* sshape_que)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;
	unsigned int	uni = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_ETHPORTS_OBJPATH,
                                         NPD_DBUS_ETHPORTS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &eth_g_index,
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
		return QOS_FALSE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter, op_ret);
	
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter, &sshape_que->portEnable);
	 dbus_message_iter_next(&iter); 
	 dbus_message_iter_get_basic(&iter, &sshape_que->burstSize);
	 dbus_message_iter_next(&iter); 
	 dbus_message_iter_get_basic(&iter, &sshape_que->Maxrate);
	 dbus_message_iter_next(&iter);
	 dbus_message_iter_get_basic(&iter, &sshape_que->kmstate);
	 dbus_message_iter_next(&iter); 
	
	 dbus_message_iter_recurse(&iter,&iter_array);
	 
	 for (uni = 0; uni < MAX_COS_QUEUE_NUM; uni++)
	 {
		 DBusMessageIter iter_struct;
		 dbus_message_iter_recurse(&iter_array,&iter_struct);
		 dbus_message_iter_get_basic(&iter_struct,&sshape_que->queue[uni].queueEnable);
		 dbus_message_iter_next(&iter_struct); 
		 dbus_message_iter_get_basic(&iter_struct,&sshape_que->queue[uni].burstSize);
		 dbus_message_iter_next(&iter_struct); 
		 dbus_message_iter_get_basic(&iter_struct,&sshape_que->queue[uni].Maxrate);
		 dbus_message_iter_next(&iter_struct); 
		 dbus_message_iter_get_basic(&iter_struct,&sshape_que->queue[uni].kmstate);
		 dbus_message_iter_next(&iter_array); 
	}
	dbus_message_unref(reply);
	return QOS_TRUE;
}

int man_qos_flow_control(char *protocol, unsigned int bandwith, int priority)
{
	unsigned int op_ret = 0;
	char *name = protocol;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
                                         NPD_DBUS_QOS_OBJPATH, \
										 NPD_DBUS_QOS_INTERFACE,\
                                         NPD_DBUS_QOS_CONFIG_FLOW_CONTROL);
	dbus_error_init(&error);
	dbus_message_append_args(query, \
							DBUS_TYPE_STRING, &name, \
							DBUS_TYPE_UINT32, &bandwith, \
							DBUS_TYPE_INT32, &priority, \
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);

        return QOS_FALSE;
	}

	if (!(dbus_message_get_args ( reply, &error, \
								   DBUS_TYPE_UINT32, &op_ret, \
								   DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s\n",error.name, error.message);
		
		dbus_message_unref(reply);
        return QOS_FALSE;
	} 
	
	dbus_message_unref(reply);
	return op_ret;	
}

int man_qos_show_flow_control(cpu_flow_control_t *cfc, int num)
{
	int ret = -1;	
	int temp = -1;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError err;

	char *protocol = cfc->protocol;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
                                         NPD_DBUS_QOS_OBJPATH, \
										 NPD_DBUS_QOS_INTERFACE,\
                                         NPD_DBUS_QOS_SHOW_FLOW_CONTROL);

	dbus_error_init(&err);
	dbus_message_append_args(query, \
					 		 DBUS_TYPE_STRING, &protocol, \
					 		 DBUS_TYPE_INT32, &num, \
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);	 
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			return 1;
	}
			 
	if (dbus_message_get_args( reply, &err, \
							   DBUS_TYPE_STRING, &protocol, \
							   DBUS_TYPE_UINT32, &cfc->bandwith, \
							   DBUS_TYPE_UINT32, &cfc->priority, \
							   DBUS_TYPE_INT32,  &ret, \
							   DBUS_TYPE_INVALID)) {
		bzero(cfc->protocol, sizeof(cfc->protocol));
		strcpy(cfc->protocol, protocol);
	} 
	else {
		if(dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			ret = -1;
	}
		 
	dbus_message_unref(reply);
	return ret; 
}


#ifdef __cplusplus
}
#endif
#endif

