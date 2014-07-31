
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

#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/npd_intf.h"
#include "npd/npd_vlan.h"
#include "man_str_parse.h"
#include "man_vlan.h"
#include "man_intf.h"

extern DBusConnection *dcli_dbus_connection;

int dcli_vlan_member_get
(
	unsigned short vid,
	unsigned char isTagged,
	unsigned char type,
	npd_pbmp_t *mbr
)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = VLAN_RETURN_CODE_ERR_NONE;
	int i = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_GET_VLAN_MEMBERS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_BYTE, &isTagged,
							DBUS_TYPE_BYTE, &type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("vlan member get: failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	if( op_ret == VLAN_RETURN_CODE_ERR_NONE ) {
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
    	{
        	dbus_message_iter_next(&iter);
        	dbus_message_iter_get_basic(&iter, (unsigned int *)mbr+i);
    	}
	}
		
	dbus_message_unref(reply);
	
	return op_ret;

}


int dcli_vlan_get_vname_base_info
(
	char *name,
	struct vlan_s *vlanInfo
)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	char *vlanName = NULL;
	unsigned int  op_ret = IGMPSNP_RETURN_CODE_OK;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_GET_VLAN_VNAME_BASE_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	if( VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->vid);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanName);
		memcpy(vlanInfo->name, vlanName, ALIAS_NAME_SIZE);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->link_status);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->mtu);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->fdb_learning_mode);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->fdb_limit);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->isAutoCreated);
/*
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->igmpSnpEnDis);
*/
	}
	
	dbus_message_unref(reply);
	return op_ret;

}


int dcli_vlan_get_base_info
(
	unsigned short vid,
	struct vlan_s *vlanInfo
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	int  op_ret = VLAN_RETURN_CODE_ERR_NONE;
	char *name = NULL;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_GET_VLAN_BASE_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("vlan get base info: failed get args reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	if( VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->vid);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &name);
		memcpy(vlanInfo->name, name, ALIAS_NAME_SIZE);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->link_status);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->mtu);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->fdb_learning_mode);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->fdb_limit);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->isAutoCreated);
/*
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->igmpSnpEnDis);
*/
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->pvlan_type);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vlanInfo->isStatic);
		
	}
	
	dbus_message_unref(reply);
	
	return op_ret;

}


int dcli_vlan_add_del_port
(
	boolean addel,
	unsigned short vlanid,
	unsigned int netif_index,
	boolean tagged,
	unsigned int pvid_set
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&addel,
								DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_BYTE,&tagged,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_UINT32,&pvid_set,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		} 
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;		
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_vlan_port_pvid
(
	boolean addel,
	unsigned short vlanid,
	unsigned int eth_g_index,
	boolean tagged
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_PVID);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&addel,
								DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_BYTE,&tagged,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
	return op_ret;
}


int dcli_create_vlan_byid(unsigned long vid)
{
    unsigned short vlanId = (unsigned short)vid;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	char* vlanName = NULL;
	unsigned int op_ret = -1;

	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

    sprintf(vlanName, "VLAN%.4d", vlanId);

	/*once bad param,it'll NOT sed message to NPD*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		free(vlanName);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
        if(VLAN_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Vlan exist,then enter vlan_config_node CMD.*/
		{
			dbus_message_unref(reply);
		       free(vlanName);
			return op_ret;
		}
	} 

    /*vlan not exist, create it*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_STRING,&vlanName, 
							 DBUS_TYPE_INVALID);

	/*printf("build query for vlanId %d vlan name %s\n",vlanId,vlanName);*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	    free(vlanName);
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		free(vlanName);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 	

	dbus_message_unref(reply);
	free(vlanName);
    return op_ret;
}


int dcli_delete_vlan_byid(unsigned int vid)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	vlanId = (unsigned short)vid;
	int ret = 0;
	unsigned int op_ret = 0;
	if (1 == vlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	if (4095 == vlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else {

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
       return op_ret;
	    
}


int dcli_create_vlan_by_id_and_name(unsigned short vid, char *vlan_name)
{

       DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int ret = 0;

	if ((vid == 1) || (vid >= 4095))
	{
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
       ret = vlan_name_legal_check(vlan_name, strlen(vlan_name));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return ALIAS_NAME_LEN_ERROR;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return ALIAS_NAME_HEAD_ERROR;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return ALIAS_NAME_BODY_ERROR;
	}
      else
      {
		if(ALIAS_NAME_RESD_ERROR == ret)
		{
			char auto_name[30];
			sprintf(auto_name, "VLAN%.4d", vid);
			if (0 != strcmp(vlan_name, auto_name))
			{
        			return ALIAS_NAME_RESD_ERROR;		
			}
		}
       	 /*vlan not exist, create it*/
        	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );
        
       	 dbus_error_init(&err);
        	dbus_message_append_args(query,
        						 DBUS_TYPE_UINT16,&vid,
        						 DBUS_TYPE_STRING,&vlan_name, 
        						 DBUS_TYPE_INVALID);
        
        	/*printf("build query for vlanId %d vlan name %s\n",vlanId,vlanName);*/
        	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

       	dbus_message_unref(query);
	
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return VLAN_RETURN_CODE_ERR_GENERAL;
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
			return VLAN_RETURN_CODE_ERR_GENERAL;
		} 
      	}
	
	dbus_message_unref(reply);
       return op_ret;
}


int dcli_config_vlan_name(unsigned short vid, char *vlan_name)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0;
	unsigned int nameSize = 0;
	unsigned int op_ret = 0, nodeSave = 0;


	ret = vlan_name_legal_check(vlan_name, strlen(vlan_name));
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
			sprintf(auto_name, "VLAN%.4d", vid);
			if (0 != strcmp(vlan_name, auto_name))
			{
        		return ALIAS_NAME_RESD_ERROR;		
			}
		}
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_METHOD_UPDATE_NAME );
        
        dbus_error_init(&err);
        dbus_message_append_args(query,
        						 DBUS_TYPE_UINT16,&vid,
        						 DBUS_TYPE_STRING,&vlan_name, 
        						 DBUS_TYPE_INVALID);
        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    }

    dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
       return op_ret;
}


int dcli_config_layer2_vlan_entity(char *vlan_name, unsigned short *vlan_id)
{
       DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short vlanId = 0;
	unsigned int nameSize = 0, nodeSave = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	ret = vlan_name_legal_check(vlan_name, strlen(vlan_name));
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
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_VIA_VLANNAME );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&vlan_name,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	
			
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	else 
	{
		*vlan_id = vlanId;
		dbus_message_unref(reply);
		return op_ret;
	}
}

int dcli_netif_port_protect(unsigned int port_index, int protect)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(	NPD_DBUS_BUSNAME,		\
										NPD_DBUS_ETHPORTS_OBJPATH ,	\
										NPD_DBUS_ETHPORTS_INTERFACE ,		\
										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_PROTECT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &port_index,
								DBUS_TYPE_UINT32, &protect,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32, &op_ret,
								DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;	
}


int dcli_netif_port_access_mode(unsigned int netif_index, int mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);	
		}	
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
	return op_ret;
}


int dcli_vlan_private( unsigned int vid, int is_enabled)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_VLAN_PRIVATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&vid,
							 	DBUS_TYPE_UINT32,&is_enabled,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
       return op_ret;
}


int dcli_netif_private(unsigned int netif_index,  int mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_PRIVATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}	
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_netif_isolate(unsigned int netif_index,  int isolated)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ISOLATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&isolated,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}	
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_vlan_assoc_mac(unsigned int vid,  ETHERADDR *macAddr, int isAdd)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_ASSOC_MAC);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32, &isAdd,
								DBUS_TYPE_UINT32,&vid,
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[0]),
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[1]),
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[2]),
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[3]),
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[4]),
							 	DBUS_TYPE_BYTE,&(macAddr->arEther[5]),
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	} 

	dbus_message_unref(reply);
	return op_ret;
}

int  dcli_vlan_show_associate_mac(unsigned int vlanId,  ETHERADDR *macAddr)
{
    unsigned int	i = 0;
    unsigned int	index = 0;
    int   op_ret = 0;
    unsigned int nodesave;

    DBusMessage *query, *reply;
    DBusMessageIter iter;
    DBusError err;
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_SHOW_ASSOC_MAC);
    	
    dbus_error_init(&err);
    dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&vlanId,
    							DBUS_TYPE_BYTE,&(macAddr->arEther[0]),
    							DBUS_TYPE_BYTE,&(macAddr->arEther[1]),
    							DBUS_TYPE_BYTE,&(macAddr->arEther[2]),
    							DBUS_TYPE_BYTE,&(macAddr->arEther[3]),
    							DBUS_TYPE_BYTE,&(macAddr->arEther[4]),
    							DBUS_TYPE_BYTE,&(macAddr->arEther[5]),
    							DBUS_TYPE_INVALID);
    	
    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    dbus_message_unref(query);
    if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return VLAN_RETURN_CODE_ERR_GENERAL;
    	}
    
    if (!(dbus_message_get_args(reply, &err,
		                  DBUS_TYPE_UINT32, &op_ret,
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[0]),
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[1]),
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[2]),
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[3]),
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[4]),
		                  DBUS_TYPE_BYTE,   &(macAddr->arEther[5]),
		                  DBUS_TYPE_INVALID)))
    {
       	if (dbus_error_is_set(&err)) {
    		dbus_error_free(&err);
    	}
		dbus_message_unref(reply);
    	return VLAN_RETURN_CODE_ERR_GENERAL;
   }
   dbus_message_unref(reply);
   return op_ret;
}


int dcli_vlan_assoc_subnet(unsigned int vid, unsigned long ipno, unsigned long ipmask, int isAdd)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_ASSOC_SUBNET);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32, &isAdd,
								DBUS_TYPE_UINT32,&vid,
								DBUS_TYPE_UINT32,&ipno,
							 	DBUS_TYPE_UINT32,&ipmask,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_switchport_cfg_subnetvlan(unsigned int netif_index, int isEnable, int preferred)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_SUBNET);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&preferred,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;		
	} 

	dbus_message_unref(reply);
	return op_ret;
}


int dcli_switchport_cfg_macvlan(unsigned int netif_index, int isEnable, int preferred)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_MAC);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&preferred,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	} 
	dbus_message_unref(reply);
	return op_ret;
}

#ifdef HAVE_QINQ
int dcli_netif_qinq_mode_set(unsigned int isEnable, unsigned int netif_index)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isEnable,
                                DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_netif_qinq_miss_drop(int isEnable, unsigned int netif_index)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_MISS_DROP);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isEnable,
                                DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	} 
	
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_vlan_xlate_cfg(unsigned int netif_index, unsigned int isEgress, unsigned int isAdd, vlan_xlate_db_entry_t *entry)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

    entry->netif_index = netif_index;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_QINQ_XLATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isAdd,
								DBUS_TYPE_UINT32,&isEgress,
                                DBUS_TYPE_UINT32,&entry->netif_index,
								DBUS_TYPE_UINT32,&entry->xlate_type,
								DBUS_TYPE_UINT32,&entry->ingress_outer_start_vid,
								DBUS_TYPE_UINT32,&entry->ingress_outer_vid_num,
								DBUS_TYPE_UINT32,&entry->ingress_inner_start_vid,
								DBUS_TYPE_UINT32,&entry->ingress_inner_vid_num,	
								DBUS_TYPE_UINT32,&entry->priority,
								DBUS_TYPE_UINT32,&entry->egress_outer_vid,
								DBUS_TYPE_UINT32,&entry->egress_inner_vid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;			
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_outer_tpid_add(unsigned int netif_index, int isSet, unsigned short outer_tpid)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_TPID);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isSet,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_UINT16, &outer_tpid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;	
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_inner_tpid_add(
    unsigned int netif_index,
    int isSet,
    unsigned short inner_tpid
    )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_INNER_TPID);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isSet,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_UINT16, &inner_tpid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}	
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_global_tpid_set(
    unsigned int isSet,
    unsigned int isOuter,
    unsigned short tpid
    )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_QINQ_GLOBAL_TPID);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isSet,
                                DBUS_TYPE_UINT32,&isOuter,
                                DBUS_TYPE_UINT16, &tpid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;

    
}


int dcli_netif_eline_set(
    unsigned int isEnable,
    unsigned int eline_id,
    unsigned int netif_index
    )
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_NETIF_ELINE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&isEnable,
                                DBUS_TYPE_UINT32,&eline_id,    
                                DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;


}


int dcli_vlan_eline_add_del(
    vlan_eline_db_entry_t * entry,
    unsigned int added
    )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_VLAN_OBJPATH,\
										NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_CONFIG_ELINE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
                                DBUS_TYPE_UINT32,&added,
                                DBUS_TYPE_UINT32,&entry->eline_id,
                                DBUS_TYPE_UINT32,&entry->eline_type,
                                DBUS_TYPE_UINT32,&entry->outer_vid,
                                DBUS_TYPE_UINT32,&entry->inner_vid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}	
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 

	dbus_message_unref(reply);
	return op_ret;
	
}


int dcli_vlan_port_inner_pvid
(
	boolean addel,	
	unsigned short vlanid,
	unsigned int eth_g_index
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_VLAN_OBJPATH,\
										NPD_DBUS_VLAN_INTERFACE,\
										NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_INNER_PVID);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&addel,
								DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_NONE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		} 
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;		
	} 
	dbus_message_unref(reply);
	return op_ret;
}
#endif

int dcli_get_next_vlanid(unsigned short in_vid, unsigned short *out_vid)
{
    int op_ret;
	int dbus_ret;
    unsigned short vlanId = in_vid; 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

    query = dbus_message_new_method_call(
                                 NPD_DBUS_BUSNAME,
                                 NPD_DBUS_VLAN_OBJPATH,
                                 NPD_DBUS_VLAN_INTERFACE,
                                 NPD_DBUS_VLAN_METHOD_NEXT_VLAN_INDEX);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    
	dbus_message_unref(query);
	if (NULL == reply) {
        return DBUS_VLAN_ERROR;
	}
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT16, &vlanId,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
        return DBUS_VLAN_ERROR;
    }  
    if (VLAN_RETURN_CODE_ERR_NONE != op_ret )
    {
        return DCLI_GET_NEXTVLAN_END;
    }
    *out_vid = vlanId;
    return DCLI_GET_NEXTVLAN_SUCCESS;
    
}



int dcli_config_add_del_prot_ethertype(unsigned char isAdd, unsigned int frame_type, unsigned int groupid, unsigned short etherType)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_ETHER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_UINT32,&frame_type,
							 	DBUS_TYPE_UINT32,&groupid,
							 	DBUS_TYPE_UINT16,&etherType,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
		
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_vlan_add_del_prot_ethertype(unsigned int isAdd, unsigned int netif_index, unsigned int groupid, unsigned short vlanId)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_PORT_APPLY);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&isAdd,
							 	DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&groupid,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_vlan_active_protvlan(unsigned short protvlanId, unsigned short *out_protvlan_id)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;

	if (4095 == protvlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if (1 > protvlanId)
	{
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_VLAN_ONE );
		
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&protvlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, out_protvlan_id,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
		
	} 
	dbus_message_unref(reply);	
	return op_ret;
}

#ifdef HAVE_SUPER_VLAN
int dcli_add_del_subvlan(unsigned char isAdd, unsigned char isTagged, unsigned short subvlanId, unsigned short vlanId)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	op_ret = 0;

	if (4095 == subvlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if (1 > subvlanId)
	{
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SUBVLAN_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_BYTE,&subvlanId,
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
        if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
		
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_vlan_active_supervlan(unsigned short supervid, unsigned short *out_supervid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;

	if (4095 == supervid){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if (1 > supervid)
	{
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SUPER_VLAN_ONE );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&supervid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, out_supervid,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_show_supervlan(unsigned short vlanId, unsigned short *out_vlan_id)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;

	if (4095 == vlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if (1 > vlanId)
	{
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SHOW_SUPERVLAN_ENTRY );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, out_vlan_id,
					DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
#endif

int dcli_set_port_vlan_ingress_filter(unsigned int eth_g_index, unsigned char enDis)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SET_PORT_VLAN_INGRES_FLITER );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&eth_g_index,
							DBUS_TYPE_BYTE,&enDis,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
	
}

int dcli_show_port_pvid(unsigned int eth_g_index, unsigned short *pvid)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int	op_ret = 0;

	if (0 == eth_g_index) {
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SHOW_ONE_PORT_PVID );
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, pvid,
					DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);

	return op_ret;

}

int dcli_show_port_list_pvid(DBusMessageIter	 *iter)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter_array;
	query = dbus_message_new_method_call(
						NPD_DBUS_BUSNAME,  \
						NPD_DBUS_VLAN_OBJPATH ,  \
						NPD_DBUS_VLAN_INTERFACE ,  \
						NPD_DBUS_VLAN_METHOD_SHOW_PORTS_LIST_PVID );
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply,iter);

	dbus_message_unref(reply);
	
	return VLAN_RETURN_CODE_ERR_NONE;

}

int dcli_del_vlan_by_id(unsigned short vlanId)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	if (1 >= vlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if (4095 <= vlanId){
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_INVALID);
		/*printf("build query for vlanId %d.\n",vlanId);*/
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	
	return op_ret;
}

int dcli_del_vlan_by_name(char *vlanName)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int op_ret = 0;

	ret = vlan_name_legal_check(vlanName, strlen(vlanName));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}

	if (NPD_SUCCESS != ret) {
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else {

		query = dbus_message_new_method_call(
											NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_DELETE_VLAN_ENTRY_VIA_NAME );
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING,&vlanName,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_vlan_filter(unsigned short vlanId, unsigned int vlanfilterType, unsigned int en_dis)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_FILTER );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_UINT32,&vlanfilterType,
							DBUS_TYPE_UINT32,&en_dis,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_privlan_member_get(unsigned short vid, unsigned char pri_mode, unsigned char type, npd_pbmp_t *mbr)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = VLAN_RETURN_CODE_ERR_NONE;
	int i = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_GET_PRIVLAN_MEMBERS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_BYTE, &pri_mode,
							DBUS_TYPE_BYTE, &type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_NONE;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	if( op_ret == VLAN_RETURN_CODE_ERR_NONE ) {
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
    	{
        	dbus_message_iter_next(&iter);
        	dbus_message_iter_get_basic(&iter, (unsigned int *)mbr+i);
    	}
	}
		
	dbus_message_unref(reply);
	
	return op_ret;

}

int dcli_isolate_group_member_get(unsigned short groupId, unsigned char type, npd_pbmp_t *mbr)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = VLAN_RETURN_CODE_ERR_NONE;
	int i = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_GET_ISOLATE_GROPU_MEMBERS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&groupId,
							DBUS_TYPE_BYTE, &type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_NONE;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	if( op_ret == VLAN_RETURN_CODE_ERR_NONE ) {
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
    	{
        	dbus_message_iter_next(&iter);
        	dbus_message_iter_get_basic(&iter, (unsigned int *)mbr+i);
    	}
	}
		
	dbus_message_unref(reply);
	
	return op_ret;

}

/************create vlan intf*******************************/
int dcli_create_vlan_intf(unsigned short vlan_id)
{
	int ret = 0;
	unsigned int vlan_netif_index = 0;
	unsigned int vlan_if_index = 0;

	vlan_netif_index = npd_netif_vlan_index(vlan_id);
	if (vlan_netif_index == 0)
		return VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	ret = dcli_create_intf(vlan_netif_index, &vlan_if_index);
	return ret;
}
/************delete vlan intf*******************************/
int dcli_delete_vlan_intf(unsigned short vlan_id)
{
	int ret = 0;
	unsigned int vlan_netif_index = 0;

	vlan_netif_index = npd_netif_vlan_index(vlan_id);
	ret = dcli_delete_intf(vlan_netif_index);
	return ret;
}


/***********check vlan exist*****************************/
int dcli_check_vlan_exist(unsigned short vlan_id)
{
	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
										NPD_DBUS_VLAN_OBJPATH, \
										NPD_DBUS_VLAN_INTERFACE, \
										NPD_DBUS_VLAN_METHOD_CHECK_VLAN_EXIST);
	dbus_error_init(&error);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlan_id,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&error)) {
			dbus_error_free(&error);
		}
		dbus_message_unref(reply);
		return VLAN_RETURN_CODE_ERR_GENERAL;
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
		return VLAN_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

/*****************show vlan member name for web*********************************
member_type : 0 show port , 1 show trunk
show_type : 0 show only one member, 1 show zhe list of memeber like 1/3/1-1/3/5 (only for ports)

****************************************************************************/
int wp_show_vlan_member_name
(
	unsigned char member_type, 
	unsigned char show_type,
	npd_pbmp_t member, 
	char member_name[][64],
	unsigned int *member_counter
)
{
	unsigned char port_name[64] = {0};
	unsigned char port_string[64] = {0};
	unsigned int eth_g_index = 0;
    int start_port = 0;
    int end_port = 0;
    int end_array_id = 0;
	unsigned int counter = 0;
	unsigned int array_index = 0;
	
	switch (member_type)
	{
		case 0 :
		{
			switch (show_type)
			{
				case 0 :
				{
					NPD_PBMP_ITER(member, array_index)
					{
						eth_g_index = dcli_eth_port_array_index_to_ifindex(array_index);
		    			parse_eth_index_to_name(eth_g_index, port_string);              
            			memcpy(member_name[counter++], port_string, 64);
						memset(port_string, 0, 64);
					}
					*member_counter = counter;
					return 1;
				}
				case 1 :
				{
					NPD_PBMP_ITER(member, array_index)
					{
						eth_g_index = dcli_eth_port_array_index_to_ifindex(array_index);
       					 if(0 == start_port)
        				{
            				start_port = end_port = eth_g_index;
            				end_array_id = array_index;
            				continue;
        				}
        				if(end_array_id == array_index-1)
        				{
            				int slot1 = eth_port_get_slot_by_ifindex(end_port);
            				int slot2 = eth_port_get_slot_by_ifindex(eth_g_index);
            				if(slot1 == slot2)
           					{
                				end_port = eth_g_index;
                				end_array_id = array_index;
                				continue;
            				}
        				}
        				if(start_port == end_port)
       				 	{
		    				parse_eth_index_to_name(start_port, port_name);
            				sprintf(port_string,"%s%s", port_string, port_name);
        				}
        				else
        				{
		    				parse_eth_index_to_name(start_port, port_name);
            				sprintf(port_string,"%s%s-",port_string, port_name);
            				parse_eth_index_to_name(end_port, port_name);
            				sprintf(port_string,"%s%s",port_string, port_name+4);
        				}   
            			memcpy(member_name[counter++], port_string, 64);
						memset(port_string, 0, 64);
						memset(port_name, 0, 64);
						start_port = end_port = eth_g_index;
        				end_array_id = array_index;
					}
    				if(start_port)
    				{
        				if(start_port == end_port)
        				{
    	   	 				parse_eth_index_to_name(start_port, port_name);
            				sprintf(port_string,"%s%s", port_string, port_name);
        				}
        				else
        				{
    	    				parse_eth_index_to_name(start_port, port_name);
            				sprintf(port_string,"%s%s-",port_string, port_name);
            				parse_eth_index_to_name(end_port, port_name);
            				sprintf(port_string,"%s%s",port_string, port_name+4);
        				}   
       					memcpy(member_name[counter++], port_string, 64);
						memset(port_string, 0, 64);
						memset(port_name, 0, 64);
						*member_counter = counter;
						return 1;
    				}
					return 0;
    			}
				default : 
				{
					return -1;
				}	
			}
		}
	    case 1 :
		{
    		NPD_PBMP_ITER(member, array_index)
    		{
				sprintf(port_string, "%d", array_index);
        		memcpy(member_name[counter++], port_string, 64);
				memset(port_string, 0, 64);
    		}
			*member_counter = counter;
			return 1;
	    }
		default :
		{
			return -1;
		}
	}
    return 0;
}

unsigned short *wp_parse_vlan_trunk_member(char *trunk_memeber)
{
	unsigned short *vlan_trunk_member = NULL;
	vlan_trunk_member = (unsigned short *)malloc(128*sizeof(unsigned short));
	memset(vlan_trunk_member, 0, 128*sizeof(unsigned short));
	char cToken = 0;
	char member[128 * 2] = {0};
	char int_tmp[5] = {0};
	unsigned int tmp_int = 0;
	unsigned int member_count = 0;
	strncpy(member, trunk_memeber, strlen(trunk_memeber));
	member[strlen(trunk_memeber) + 1] = '\0';
	unsigned int token_count  = 0;
	unsigned short member_temp = 0;
	unsigned char is_number = FALSE;
	cToken = member[0];
	while(cToken != 0)
	{
		if (isdigit( cToken ))
		{
			is_number = TRUE;
			if (tmp_int < 4)
			{
				int_tmp[tmp_int++] = cToken;
			}
			else 
				return NULL;	
		}
		else if (cToken == ',')
		{
			if (is_number == FALSE)
				return NULL;
			else
			{
				member_temp = (unsigned short)atoi(int_tmp);
				if (member_temp > 127)
					return NULL;
				if (member_count < 127)
				{
					vlan_trunk_member[member_count++] = member_temp;
					memset(int_tmp, 0, sizeof(int_tmp));
					tmp_int = 0;
				}
				is_number = FALSE;
			}
		}
		else
		{
			return NULL;
		}
		token_count++;
		cToken = member[token_count];
	}
	if (tmp_int != 0)
	{
		member_temp = (unsigned short)atoi(int_tmp);
		if (member_temp > 127)
			return NULL;
		if (member_count < 127)
		{
			vlan_trunk_member[member_count++] = member_temp;
			memset(int_tmp, 0, sizeof(int_tmp));
			tmp_int = 0;
		}
	}
	return vlan_trunk_member;
		
}

int dcli_get_mac_based_vlan_number(void)
{
	unsigned short vlanId = 0;
	unsigned int vlan_base_mac_count = 0;
	while (1)
	{
		int ret = 0;
        ret = dcli_get_next_vlanid(vlanId, &vlanId);

		if(DBUS_VLAN_ERROR == ret)
        {
            break;
        }
        if(DCLI_GET_NEXTVLAN_END == ret)
            break;
		
		if(vlanId != 4095)
		{	    
			ETHERADDR		macAddr;
			memset(&macAddr,0,sizeof(ETHERADDR));
			ret =  dcli_vlan_show_associate_mac(vlanId,  &macAddr);
			while(VLAN_RETURN_CODE_MACBASE_EXIST == ret)
			{
				ret =  dcli_vlan_show_associate_mac(vlanId,  &macAddr);
				vlan_base_mac_count++;
			}
		}
	}
	return vlan_base_mac_count;
}



#ifdef __cplusplus
}
#endif
