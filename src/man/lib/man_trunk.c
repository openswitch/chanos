
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
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"

#include "dbus/npd/npd_dbus_def.h"
#include "npd/npd_trunk.h"
#include "npd/npd_lacp.h"
#include "man_str_parse.h"

#include "man_trunk.h"

extern DBusConnection *dcli_dbus_connection;


int dcli_create_trunk_by_id_name(unsigned short trunkId, char *trunkName)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0;
	unsigned int nameSize = 0;
	unsigned int op_ret = 0, nodesave = 0;

    if(NULL == trunkName){
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}	
	ret = trunk_name_legal_check(trunkName, strlen(trunkName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
	else{

        if(ALIAS_NAME_RESD_ERROR == ret)
		{
			char auto_name[30];
			sprintf(auto_name, "lag%d", trunkId);
			if (0 != strcmp(trunkName, auto_name))
			{
				return ALIAS_NAME_RESD_ERROR;		
			}
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
								 DBUS_TYPE_STRING,&trunkName,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) 
	{	
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_create_trunk_by_name(char *trunkName, unsigned short *trunkId, unsigned int *route)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int nameSize = 0,nodesave = 0;
	int ret = 0;
	unsigned int op_ret = 0;


	if(NULL == trunkName){
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}	
	ret = trunk_name_legal_check(trunkName, strlen(trunkName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
    else if(ALIAS_NAME_RESD_ERROR == ret){
		return ALIAS_NAME_RESD_ERROR;	
    }
	else{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&trunkName,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, trunkId,
					DBUS_TYPE_UINT32, route,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_create_trunk_by_id(unsigned short trunkId, unsigned int *route)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	int ret = 0;
	unsigned int op_ret = 0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_CONFIG_ONE );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, route,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_add_del_trunk_member(unsigned char isAdd, unsigned int eth_g_index, unsigned short trunkId)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	if(eth_g_index == 0)
	{
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_delete_trunk_by_id(unsigned short trunkId)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										 NPD_DBUS_TRUNK_OBJPATH ,	\
										 NPD_DBUS_TRUNK_INTERFACE ,	\
										 NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY );
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;
}

int dcli_delete_trunk_by_name(char *trunkName)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	
	if(NULL == trunkName){
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}	
	ret = trunk_name_legal_check(trunkName, strlen(trunkName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
	else {
        if(ret == ALIAS_NAME_RESD_ERROR)
        {
            ret = NPD_SUCCESS;
        }
	}
	if (NPD_SUCCESS != ret) {
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	else {

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME );
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING,&trunkName,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_get_aggregator_by_trunk_id
(
	unsigned short trunk_id, 
	unsigned int *lacp_enable_state, 
	unsigned char en_actor_status[], 
	unsigned char dis_actor_status[],
	unsigned short key_port_manual[],
	struct lacpport_actor en_trunk_actor[], 
	struct lacpport_actor dis_trunk_actor[], 
	struct lacpport_partner trunk_partner[], 
	struct aggregator *trunk_aggregator
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int op_ret = 0;
	int i = 0, j = 0;
	unsigned char aggregator_mode = 0;
    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,                \
                                 NPD_DBUS_TRUNK_OBJPATH,          \
                                 NPD_DBUS_TRUNK_INTERFACE,        \
                                 NPD_DBUS_AGGREGATOR_GET_BY_TRUNKID);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunk_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
        return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, lacp_enable_state);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &(trunk_aggregator->aggregator_identifier));
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &aggregator_mode);
	trunk_aggregator->aggregator_mode = aggregator_mode;
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(en_actor_status[i]));
     	dbus_message_iter_next(&iter);
     	dbus_message_iter_get_basic(&iter, &(en_trunk_actor[i].actor_port_priority));
    	dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(en_trunk_actor[i].actor_oper_port_key));
	    dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(trunk_partner[i].partner_oper_key));
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(en_trunk_actor[i].actor_oper_port_state));
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(dis_actor_status[i]));
     	dbus_message_iter_next(&iter);
     	dbus_message_iter_get_basic(&iter, &(dis_trunk_actor[i].actor_port_priority));
    	dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(dis_trunk_actor[i].actor_oper_port_key));
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(dis_trunk_actor[i].actor_oper_port_state));
 		dbus_message_iter_next(&iter);
 		dbus_message_iter_get_basic(&iter, &(trunk_partner[i].partner_oper_port_number));   
		dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(trunk_partner[i].partner_oper_system_priority));  
		dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(trunk_partner[i].partner_oper_port_state));   
		dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &(key_port_manual[i]));
    }
    for(i = 0; i < 8; i++)
    {
    	for (j = 0; j < 6; j++)
    	{
        	dbus_message_iter_next(&iter);
        	dbus_message_iter_get_basic(&iter, &(trunk_partner[i].partner_oper_system.mac_addr_value[j]));
    	}
    } 
	dbus_message_unref(reply);   
    return op_ret;
}

int dcli_get_trunk_by_id(unsigned short trunk_id, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i = 0;
	unsigned int op_ret = 0;
	char *trunkName = NULL;
	unsigned short trunkId = 0;
	
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunk_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);     
	if (op_ret == TRUNK_RETURN_CODE_ERR_NONE)
	{
		dbus_message_iter_next(&iter);
    	dbus_message_iter_get_basic(&iter, &trunkId);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &trunkName);
		dbus_message_iter_next(&iter);
    	dbus_message_iter_get_basic(&iter, masterFlag);
		dbus_message_iter_next(&iter);
    	dbus_message_iter_get_basic(&iter, &(trunk_info->g_index));
    	for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    	{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(enport_index[i]));
    	}
    	for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    	{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(disport_index[i]));
    	}
		dbus_message_iter_next(&iter);
    	dbus_message_iter_get_basic(&iter,&(trunk_info->load_balance_mode));
		dbus_message_iter_next(&iter);
    	dbus_message_iter_get_basic(&iter,&(trunk_info->linkstate));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(trunk_info->forward_mode));
		trunk_info->trunk_id = trunkId;
		memcpy(trunk_info->name, trunkName, ALIAS_NAME_SIZE - 1);
	}
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_get_trunk_by_name(char *trunkName, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i = 0;
	int	ret = 0;
	unsigned int op_ret = 0;
	char *trunk_name = NULL;
	unsigned short trunkId = 0;
	
	if(NULL == trunkName){
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}	
	ret = trunk_name_legal_check(trunkName, strlen(trunkName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
	else {
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME_V1 );
	}
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&trunkName,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);     
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &trunkId);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &trunk_name);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, masterFlag);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &(trunk_info->g_index));
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&enport_index[i]);
    }
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&disport_index[i]);
    }
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(trunk_info->load_balance_mode));
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(trunk_info->linkstate));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(trunk_info->forward_mode));
	trunk_info->trunk_id = trunkId;
	memcpy(trunk_info->name, trunk_name, ALIAS_NAME_SIZE - 1);
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_config_trunk_load_balanc(unsigned short trunkId, unsigned int loadBalanMode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_UINT32,&loadBalanMode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) 
	{	
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
	return op_ret;
}

int dcli_get_trunklist(unsigned short trunk_id, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
    DBusError err;

	unsigned int op_ret = 0;
	int i = 0;
	char *trunk_name = NULL;
	unsigned short trunkId = 0;
	
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    									 NPD_DBUS_TRUNK_OBJPATH ,	\
    									 NPD_DBUS_TRUNK_INTERFACE ,	\
    									 NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    						 DBUS_TYPE_UINT16,&trunk_id,
    						 DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);     
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &trunkId);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &trunk_name);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, masterFlag);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &(trunk_info->g_index));
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&enport_index[i]);
    }
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&disport_index[i]);
    }
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(trunk_info->load_balance_mode));
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(trunk_info->linkstate));
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(trunk_info->forward_mode));
	trunk_info->trunk_id = trunkId;
	memcpy(trunk_info->name, trunk_name, ALIAS_NAME_SIZE - 1);
	dbus_message_unref(reply);
    return op_ret;
}

int dcli_config_trunk_update_name(unsigned int netif_index, char *trunkName)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int nodesave = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	unsigned short trunk_id = 0;

    if(NULL == trunkName){
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}	
	ret = trunk_name_legal_check(trunkName, strlen(trunkName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
	else{
		if(ALIAS_NAME_RESD_ERROR == ret)
		{
			trunk_id = npd_netif_trunk_get_tid(netif_index);
			char auto_name[30];
			sprintf(auto_name, "lag%d", trunk_id);
			if (0 != strcmp(trunkName, auto_name))
			{
				return ALIAS_NAME_RESD_ERROR;		
			}
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_CONFIG_UPDATE_TRUNKNAME );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&netif_index,
								 DBUS_TYPE_STRING,&trunkName,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) 
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_check_trunk_exist(unsigned short trunk_id)
{
	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
										NPD_DBUS_TRUNK_OBJPATH, \
										NPD_DBUS_TRUNK_INTERFACE, \
										NPD_DBUS_TRUNK_METHOD_CHECK_TRUNK_EXIST);
	dbus_error_init(&error);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &trunk_id,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&error)) {
			dbus_error_free(&error);
		}
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args(reply, &error,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		dbus_message_unref(reply);
		return TRUNK_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_config_trunk_update_name_by_id(unsigned short trunk_id, char *trunk_name)
{
	unsigned int trunk_netif_index = 0;
	int ret = 0;

	trunk_netif_index = npd_netif_trunk_index(trunk_id);
	ret = dcli_config_trunk_update_name(trunk_netif_index, trunk_name);
	return ret;
}

int dcli_create_trunk_intf(unsigned short trunk_id)
{
	int ret = 0;
	unsigned int trunk_netif_index = 0;
	unsigned int trunk_if_index = 0;

	trunk_netif_index = npd_netif_trunk_index(trunk_id);
	if (trunk_netif_index == 0)
		return VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	ret = dcli_create_intf(trunk_netif_index, &trunk_if_index);
	return ret;
}

int dcli_delete_trunk_intf(unsigned short trunk_id)
{
	int ret = 0;
	unsigned int trunk_netif_index = 0;

	trunk_netif_index = npd_netif_trunk_index(trunk_id);
	ret = dcli_delete_intf(trunk_netif_index);
	return ret;
}

#ifdef __cplusplus
}
#endif

