
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_fdb.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		CLI definition for FDB module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.63 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include <dbus/npd/npd_dbus_def.h>
#include "npd/nbm/npd_bmapi.h"
#include "netif_index.h"
#include "man_fdb.h"

extern DBusConnection *dcli_dbus_connection;
//dcli api 
int dcli_fdb_set_agingtime(unsigned int agingtime)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime=agingtime;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_FDB_OBJPATH,\
										NPD_DBUS_FDB_INTERFACE,\
										NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&testAgingtime,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
		if(FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
		}
	}			
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_get_agingtime(unsigned int *agingtime)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = 0;
	unsigned int 	op_ret = 0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME);
	
	dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
	 	DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
		if (FDB_RETURN_CODE_SUCCESS != op_ret ) {
			return op_ret;
		}
		else{
			*agingtime = testAgingtime;
		}
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}
int dcli_fdb_set_default_agingtime(unsigned int *agingtime)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = *agingtime;
	unsigned int 	ret = 0;
	/*agingtime = testagingtime;*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&testAgingtime,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
	    if(FDB_RETURN_CODE_SUCCESS == ret){
			*agingtime = testAgingtime;
	    }
		else {
			return ret;
		}
			
	}			
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_set_static(
	unsigned short vid, 
	ETHERADDR * pMacAddr, 
	unsigned int in_eth_index)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = vid;
	unsigned int eth_g_index = in_eth_index; 
	DBusError err;
	unsigned int 	op_ret = 0;	

	if (pMacAddr == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}
	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_FDB_OBJPATH,\
										NPD_DBUS_FDB_INTERFACE,\
										NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
		if( FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
	    }
    }
	else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_set_trunk_static(
	unsigned short vid, 
	unsigned short tid, 
	ETHERADDR * pMacAddr)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = vid;
	unsigned short  trunkId = tid;
	DBusError err;
	unsigned int 	op_ret = 0;	
	
	if (pMacAddr == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}
	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
					            DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
		if( FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
	    }
	}

	else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	

}

int dcli_fdb_set_blacklist_delete(
	unsigned int set_flag, 
	unsigned short vid,
	ETHERADDR * pMacAddr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	unsigned short	vlanId = vid;
	int 	op_ret = 0,len = 0;
    unsigned int   flag = set_flag;
	
	if (pMacAddr == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}
	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
						NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&flag,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
	   	if( FDB_RETURN_CODE_SUCCESS != op_ret){
	   		return op_ret;
	    }
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
	
}
int dcli_fdb_set_blacklist_add(
	unsigned int set_flag, 
	unsigned short vid,
	ETHERADDR * pMacAddr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	unsigned short	vlanId = vid;
	int 	op_ret = 0,len = 0;
    unsigned int   flag = set_flag;
	
	if (pMacAddr == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}
	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
						NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&flag,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
	   	if( FDB_RETURN_CODE_SUCCESS != op_ret){
	   		return op_ret;
	    }
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
	
}

int dcli_fdb_set_nostatic(
	unsigned short vid, 
	ETHERADDR * pMacAddr)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = vid;
	DBusError err;
	unsigned int 	op_ret = 0;	

	if (pMacAddr == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}
	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_FDB_OBJPATH,\
										NPD_DBUS_FDB_INTERFACE,\
										NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			if( FDB_RETURN_CODE_SUCCESS != op_ret){
				return op_ret;
		    }
    }
	else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_set_delete_vlan(unsigned short vid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = vid;
	unsigned int 	op_ret = 0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN );
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,120000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}		
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
	    if(FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
		}
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}

int dcli_fdb_set_delete_port(unsigned int in_eth_g_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int eth_g_index = in_eth_g_index;
	unsigned int 	op_ret = 0;


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,120000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
       	if(FDB_RETURN_CODE_SUCCESS != op_ret){
	   		return op_ret;
        }
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
	
}

int dcli_fdb_set_static_delete_vlan(unsigned short vid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = vid;
	unsigned int 	op_ret = 0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN );
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
	   	if( FDB_RETURN_CODE_SUCCESS != op_ret){
	   		return op_ret;
	    }
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_set_static_delete_port(unsigned int in_eth_g_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int eth_g_index = in_eth_g_index;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT);
	
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
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
	   if( FDB_RETURN_CODE_SUCCESS != op_ret){
	   		return op_ret;
	    }
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}

int dcli_fdb_set_delete_trunk(unsigned short tid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short trunk_no = tid;
	unsigned int 	op_ret;
	if (NPD_FAIL == op_ret) {
		return 1;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&trunk_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,120000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
	   if( FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
	   }
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}


int dcli_fdb_set_static_delete_trunk(unsigned short tid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short trunk_no = tid;
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_TRUNK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&trunk_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	} 
	else {
	   if( FDB_RETURN_CODE_SUCCESS != op_ret){
			return op_ret;
	   }
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}

int dcli_fdb_get_one(
	unsigned short vid, 
	ETHERADDR * pMacAddr,
	unsigned int *num,
	NPD_FDB* item)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned char  show_mac[6] ={0};
	unsigned short vlanid = vid;
	unsigned int netif_index = 0;
	unsigned int netif_type = 0;
	char netif_name[20] = {0};
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	int	op_ret = 0;
	
	if (pMacAddr == NULL || item == NULL)
	{
		return FDB_RETURN_CODE_GENERAL;
	}

	memset(&macAddr,0,sizeof(ETHERADDR));	
	memcpy(&macAddr,pMacAddr,sizeof(ETHERADDR));
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							DBUS_TYPE_UINT16,&vlanId,
						 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[5],						 	
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(FDB_RETURN_CODE_SUCCESS != op_ret){
	    dbus_message_unref(reply);
        return op_ret;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		*num = dnumber;
		return FDB_RETURN_CODE_SUCCESS;
	}

	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->vlanid));
	dbus_message_iter_next(&iter);		
	dbus_message_iter_get_basic(&iter,&(item->value));

	dbus_message_iter_next(&iter);		
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[0]));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[1]));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[2]));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[3]));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[4]));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(item->ether_mac[5]));
	
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}

int dcli_fdb_get_count(unsigned int * num)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int dnumber = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&dnumber,
		DBUS_TYPE_INVALID)) {
		*num = dnumber;
	} 
	else {			
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;
}

int dcli_fdb_get_all(
	unsigned int startIndex,
	unsigned int getNum,
	unsigned int *count,
	NPD_FDB **item_arr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int type_flag = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int netif_index = 0;
	unsigned int netif_type = 0;
    char port_name[20]={0};
    unsigned int start_index = startIndex;
	unsigned int get_num = getNum;
    int ret;
	NPD_FDB *item;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							DBUS_TYPE_UINT32,&start_index,
							DBUS_TYPE_UINT32,&get_num,
						 	DBUS_TYPE_INVALID);
	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
    
	dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter, &ret);
    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	*count = dnumber;
	if (dnumber == 0){
		
		return FDB_RETURN_CODE_SUCCESS;
	}
	*item_arr = malloc(sizeof(NPD_FDB)*dnumber);
	if (NULL == *item_arr)
	{
		return FDB_RETURN_CODE_MALLOC;
	}
	
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
		item = &((*item_arr)[i]);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->vlanid));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->value));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->type_flag));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[0]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[1]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[2]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[3]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[4]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[5]));

		dbus_message_iter_next(&iter_array);		
	}
	
	dbus_message_unref(reply);

	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_get_single_unit(
	unsigned int startIndex,
	unsigned int getNum,
	unsigned int *count,
	NPD_FDB **item_arr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int type_flag = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int netif_index = 0;
	unsigned int netif_type = 0;
    char port_name[20]={0};
    unsigned int unit = startIndex;
	unsigned int get_num = getNum;
    int ret;
	NPD_FDB *item;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_SINGLE_UNIT_TABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							DBUS_TYPE_UINT32,&unit,
							DBUS_TYPE_UINT32,&get_num,
						 	DBUS_TYPE_INVALID);
	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
    
	dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter, &ret);
    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	*count = dnumber;
	if (dnumber == 0){
		
		return FDB_RETURN_CODE_SUCCESS;
	}
	*item_arr = malloc(sizeof(NPD_FDB)*dnumber);
	if (NULL == *item_arr)
	{
		return FDB_RETURN_CODE_MALLOC;
	}
	
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
		item = &((*item_arr)[i]);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->vlanid));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->value));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->type_flag));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[0]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[1]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[2]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[3]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[4]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[5]));

		dbus_message_iter_next(&iter_array);		
	}
	
	dbus_message_unref(reply);

	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_get_all_dynamic(
	unsigned int startIndex,
	unsigned int getNum,
	unsigned int *count,
	NPD_FDB **item_arr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int type_flag = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int netif_index = 0;
	unsigned int netif_type = 0;
    char port_name[20]={0};
    unsigned int start_index = startIndex;
	unsigned int get_num = getNum;
    int ret;
	NPD_FDB *item;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_FDB_OBJPATH,\
										NPD_DBUS_FDB_INTERFACE,\
										NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							DBUS_TYPE_UINT32,&start_index,
							DBUS_TYPE_UINT32,&get_num,
						 	DBUS_TYPE_INVALID);
	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
    
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	*count = dnumber;
	if (dnumber == 0){
		
		return FDB_RETURN_CODE_SUCCESS;
	}
	*item_arr = malloc(sizeof(NPD_FDB)*dnumber);
	if (NULL == *item_arr)
	{
		return FDB_RETURN_CODE_MALLOC;
	}
	
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
		item = &((*item_arr)[i]);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->vlanid));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->value));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->type_flag));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[0]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[1]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[2]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[3]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[4]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[5]));

		dbus_message_iter_next(&iter_array);		
	}
	
	dbus_message_unref(reply);

	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_get_all_static(
	unsigned int *count,
	NPD_FDB **item_arr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int type_flag = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int netif_index = 0;
	unsigned int netif_type = 0;
    char port_name[20]={0};
    int ret;
	NPD_FDB *item;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_FDB_OBJPATH,\
										NPD_DBUS_FDB_INTERFACE,\
										NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE);
	
	dbus_error_init(&err);
	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
    
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	*count = dnumber;
	if (dnumber == 0){
		
		return FDB_RETURN_CODE_SUCCESS;
	}
	*item_arr = malloc(sizeof(NPD_FDB)*dnumber);
	if (NULL == *item_arr)
	{
		return FDB_RETURN_CODE_MALLOC;
	}
	
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
		unsigned char flag;
		item = &((*item_arr)[i]);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->vlanid));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->value));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[0]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[1]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[2]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[3]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[4]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[5]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&flag);
		item->type_flag = (unsigned int)flag;

		dbus_message_iter_next(&iter_array);		
	}
	
	dbus_message_unref(reply);

	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_get_all_blacklist(
	unsigned int *num,
	unsigned char **dmac_arr,
	unsigned char **smac_arr,
	NPD_FDB **item_arr)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
    unsigned char dmac = 0;
	unsigned char smac = 0;
	
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	NPD_FDB * item = NULL;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == FDB_RETURN_CODE_MAX){
		return FDB_RETURN_CODE_MAX;
	}
	*num = dnumber;
	if (dnumber == 0){		
		return FDB_RETURN_CODE_SUCCESS;
	}
	*dmac_arr = (unsigned char *)malloc(sizeof(unsigned char)*dnumber);
	*smac_arr = (unsigned char *)malloc(sizeof(unsigned char)*dnumber);
	*item_arr = (NPD_FDB *)malloc(sizeof(NPD_FDB)*dnumber);
	if ((NULL == *dmac_arr) 
		|| (NULL == *dmac_arr)
		|| (NULL == *item_arr))
	{
		return FDB_RETURN_CODE_MALLOC;
	}

	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
		item = &((*item_arr)[i]);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&((*dmac_arr)[i]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&((*smac_arr)[i]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->vlanid));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[0]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[1]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[2]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[3]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[4]));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(item->ether_mac[5]));
						
		dbus_message_iter_next(&iter_array);	
	}
	
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;	
}

int dcli_fdb_set_number(
	unsigned int netif_index,
	int number)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	op_ret = 0;
	unsigned int fdblimit = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	else{
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_UINT32,&fdblimit,
			DBUS_TYPE_INVALID)) {
			if(FDB_RETURN_CODE_SUCCESS != op_ret){
				return op_ret;
			}
		} 
		else {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return FDB_RETURN_CODE_ERR_DBUS;
		}
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;    
}

// cmd line		 
int dcli_fdb_set_vlan_limit(
    unsigned short vlanid,
    int number)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return FDB_RETURN_CODE_ERR_DBUS;
	}
	else{
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&ret,
			DBUS_TYPE_UINT32,&number,
			DBUS_TYPE_INVALID)) {
			if(FDB_RETURN_CODE_SUCCESS != ret){
				return ret;
			}
		}
		 else {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return FDB_RETURN_CODE_ERR_DBUS;
		}
	}
	dbus_message_unref(reply);
	return FDB_RETURN_CODE_SUCCESS;    
}         

#ifdef __cplusplus
}
#endif

