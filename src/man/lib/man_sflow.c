
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

#ifdef HAVE_SFLOW
#include "lib/osinc.h"

#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/npd_intf.h"
#include "npd/npd_sflow.h"
#include "man_str_parse.h"
#include "man_sflow.h"
extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *sfl_dbus_connection;
extern DBusConnection *config_dbus_connection;

int man_sflow_show_agt(unsigned long *agt_ip)
{
	int ret = -1;	
	unsigned long agent_ip;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_SFLOW_OBJPATH,\
										 NPD_DBUS_SFLOW_INTERFACE,\
										 NPD_DBUS_METHOD_SHOW_SFLOW_AGT_CONFIG);
		
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
		 
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			return 1;
	}
		 
	if (dbus_message_get_args( reply, &err, \
							   DBUS_TYPE_UINT32, &agent_ip, \
							   DBUS_TYPE_INVALID)) {
		*agt_ip = agent_ip;
		ret = 0;
	} 
	else {
		if(dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			ret = -1;
	}
		 
	dbus_message_unref(reply);
	return ret; 
}

int man_sflow_show_clt(sfl_clt_t *sfl_clt, unsigned int index)
{
	int ret = -1;	
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_SFLOW_OBJPATH,\
										 NPD_DBUS_SFLOW_INTERFACE,\
										 NPD_DBUS_METHOD_SHOW_SFLOW_CLT_CONFIG);
			
	dbus_error_init(&err);
	dbus_message_append_args(query, \
							 DBUS_TYPE_UINT32, &index, \
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	 
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			return 1;
	}
			 
	if (dbus_message_get_args( reply, &err, \
							   DBUS_TYPE_UINT32, &sfl_clt->index, \
							   DBUS_TYPE_UINT32, &sfl_clt->clt_ip, \
							   DBUS_TYPE_UINT32, &sfl_clt->clt_port, \
							   DBUS_TYPE_INT32,  &ret, \
							   DBUS_TYPE_INVALID)) {
	} 
	else {
		if(dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			ret = 1;
	}	 
	dbus_message_unref(reply);
	return ret; 
}

int man_sflow_show_port(sflport_info_t *port_info, unsigned int ifindex, int num)
{
	int ret = -1;	
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_SFLOW_OBJPATH,\
										 NPD_DBUS_SFLOW_INTERFACE,\
										 NPD_DBUS_METHOD_SHOW_SFLOW_PORT);
			
	dbus_error_init(&err);
	dbus_message_append_args(query, \
					 		 DBUS_TYPE_UINT32, &num, \
							 DBUS_TYPE_UINT32, &ifindex, \
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);	 
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			return 1;
	}
			 
	if (dbus_message_get_args( reply, &err, \
							   DBUS_TYPE_UINT32, &port_info->netif_index, \
							   DBUS_TYPE_UINT32, &port_info->is_enable, \
							   DBUS_TYPE_UINT32, &port_info->sampling_rate, \
							   DBUS_TYPE_INT32,  &ret, \
							   DBUS_TYPE_INVALID)) {
	} 
	else {
		if(dbus_error_is_set(&err)) 
			dbus_error_free(&err);
			ret = 1;
	}
		 
	dbus_message_unref(reply);
	return ret; 
}

int man_no_sflow_agtip_protocol(void)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	unsigned int del_flag = 1;
	
	query = dbus_message_new_method_call( SFLOW_DBUS_BUSNAME,\
										  SFLOW_DBUS_OBJPATH,\
										  SFLOW_DBUS_INTERFACE,\
										  SFLOW_DBUS_NO_AGTIP);
	
	dbus_error_init(&error);
	dbus_message_append_args(query, \
							 DBUS_TYPE_UINT32, &del_flag, \
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return SFLOW_RETURN_CODE_ERR_GENERAL;
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
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;	
}

int man_no_sflow_cltip_protocol(unsigned int clt_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
									SFLOW_DBUS_BUSNAME,\
									SFLOW_DBUS_OBJPATH,\
									SFLOW_DBUS_INTERFACE,\
									SFLOW_DBUS_NO_CLTIP);
	
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &clt_index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return SFLOW_RETURN_CODE_ERR_GENERAL;
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
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;

}

int man_no_sflow_agtip(void)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	unsigned int del_flag = 1; //use for conmunicate with npd
	
	query = dbus_message_new_method_call( NPD_DBUS_BUSNAME,\
										  NPD_DBUS_SFLOW_OBJPATH,\
										  NPD_DBUS_SFLOW_INTERFACE,\
										  NPD_DBUS_SFLOW_METHOD_NO_AGENT);
		
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &del_flag,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
		}
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	}
	
	if (!(dbus_message_get_args ( reply, &error,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		dbus_message_unref(reply);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	
	dbus_message_unref(reply);
	return op_ret;	
}

int man_no_sflow_cltip(unsigned int clt_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
		
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,\
										NPD_DBUS_SFLOW_OBJPATH,\
										NPD_DBUS_SFLOW_INTERFACE,\
										NPD_DBUS_SFLOW_METHOD_NO_COLLECTOR);
			
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &clt_index,
							DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	}
		
	if (!(dbus_message_get_args ( reply, &error,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		dbus_message_unref(reply);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
		
	dbus_message_unref(reply);
	return op_ret;	
}

int man_sflow_agtip_protocol(unsigned long agt_ip)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
									SFLOW_DBUS_BUSNAME,\
									SFLOW_DBUS_OBJPATH,\
									SFLOW_DBUS_INTERFACE,\
									SFLOW_DBUS_AGTIP_SET);
	
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &agt_ip,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
	    }
        return SFLOW_RETURN_CODE_ERR_GENERAL;
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
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;	
}

int man_sflow_agtip(unsigned long agt_ip)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,\
										NPD_DBUS_SFLOW_OBJPATH,\
										NPD_DBUS_SFLOW_INTERFACE,\
										NPD_DBUS_SFLOW_METHOD_CONFIG_AGENT);
		
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &agt_ip,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
		}
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	}
	
	if (!(dbus_message_get_args ( reply, &error,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		dbus_message_unref(reply);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	
	dbus_message_unref(reply);
	return op_ret;	
}

int man_sflow_cltip(unsigned long clt_ip, unsigned int port, unsigned int clt_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,\
										NPD_DBUS_SFLOW_OBJPATH,\
										NPD_DBUS_SFLOW_INTERFACE,\
										NPD_DBUS_SFLOW_METHOD_CONFIG_COLLECTOR);
		
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &clt_ip, \
							DBUS_TYPE_UINT32, &port, \
							DBUS_TYPE_UINT32, &clt_index, \
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	}
	
	if (!(dbus_message_get_args ( reply, &error,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		dbus_message_unref(reply);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	
	dbus_message_unref(reply);
	return op_ret;	
}

int man_sflow_cltip_protocol(unsigned long clt_ip, unsigned int port, unsigned int clt_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
									SFLOW_DBUS_BUSNAME,\
									SFLOW_DBUS_OBJPATH,\
									SFLOW_DBUS_INTERFACE,\
									SFLOW_DBUS_CLTIP_SET);
	
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &clt_ip, \
							DBUS_TYPE_UINT32, &port, \
							DBUS_TYPE_UINT32, &clt_index, \
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s\n",error.name, error.message);
	    }
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)))
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s\n",error.name, error.message);
	    }
		dbus_message_unref(reply);
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;

}

int man_sflow_endis(unsigned int netif_index, unsigned int is_enable)
{
	unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;

	query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,\
									NPD_DBUS_SFLOW_OBJPATH,\
									NPD_DBUS_SFLOW_INTERFACE,\
									NPD_DBUS_SFLOW_METHOD_CONFIG_PORT_SFLOW_ENDIS);
	dbus_error_init(&error);
	dbus_message_append_args(query, \
							DBUS_TYPE_UINT32, &netif_index, \
							DBUS_TYPE_UINT32, &is_enable, \
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);

        return SFLOW_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &error,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s\n",error.name, error.message);
		
		dbus_message_unref(reply);
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	
	dbus_message_unref(reply);
	return op_ret;	
}

int man_sflow_samplingrate_config(unsigned int netif_index, unsigned int rate)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call( NPD_DBUS_BUSNAME,\
										  NPD_DBUS_SFLOW_OBJPATH,\
										  NPD_DBUS_SFLOW_INTERFACE,\
										  NPD_DBUS_SFLOW_METHOD_CONFIG_SAMPLRATE);
	
	dbus_error_init(&error);
	dbus_message_append_args( query, \
							  DBUS_TYPE_UINT32, &netif_index, \
							  DBUS_TYPE_UINT32, &rate, \
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
        return SFLOW_RETURN_CODE_ERR_GENERAL;
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
        return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;	
}

int man_sflow_samplingrate_to_protocol(unsigned int netif_index, unsigned int rate)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;
		
	query = dbus_message_new_method_call(
										SFLOW_DBUS_BUSNAME,\
										SFLOW_DBUS_OBJPATH,\
										SFLOW_DBUS_INTERFACE,\
										SFLOW_DBUS_SAMPLER_SET);
		
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_UINT32, &rate,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name, error.message);
		}
			return SFLOW_RETURN_CODE_ERR_GENERAL;
	}
	
	if (!(dbus_message_get_args (   reply, &error,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&error)) 
			printf("%s raised: %s",error.name, error.message);
		dbus_message_unref(reply);
		return SFLOW_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
		
	return op_ret;	
}
#endif
#ifdef __cplusplus
}
#endif

