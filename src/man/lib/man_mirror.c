
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_mirror.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for MIRROR module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.33 $	
*******************************************************************************/
#ifdef HAVE_MIRROR
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include <dbus/npd/npd_dbus_def.h>
#include "npd/nam/npd_amapi.h"
#include "npd/npd_fdb.h"
#include "man_mirror.h"

extern DBusConnection *dcli_dbus_connection;
unsigned int dcli_mirror_config_profile(unsigned int profile, unsigned char add)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
    unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_MIRROR);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_BYTE, &add,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
    {
        printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}

	if(!dbus_message_get_args (reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &profile,
		DBUS_TYPE_INVALID))
    {		
		if(dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;    
}

unsigned int dcli_mirror_delete_profile(unsigned int profile, unsigned char add)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
    unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_NO_MIRROR);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_BYTE, &add,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &profile,
		DBUS_TYPE_INVALID))
	{	
		if(dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_add_destination_port(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int g_eth_index = 0;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_DEST_PORT_CREATE);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &direct,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
        {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &g_eth_index,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return op_ret;

}

unsigned int dcli_mirror_delete_destination_port(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_DEST_PORT_DEL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &direct,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
    {
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
        {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return op_ret;   
}

unsigned int dcli_mirror_add_policy_source(unsigned int profile, unsigned int ruleIndex)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &ruleIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
        {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_delete_policy_source(unsigned int profile, unsigned int ruleIndex)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &ruleIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
	    {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{					
		if(dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_add_port_source(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											 NPD_DBUS_MIRROR_OBJPATH,
											 NPD_DBUS_MIRROR_INTERFACE,
											 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_CREATE);
	dbus_error_init(&err);
		
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_UINT32, &direct,
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
	    printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
		
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_delete_port_source(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
    unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_DEL);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &direct,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}

	
	if(dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_add_vlan_source(unsigned int profile, unsigned short vid)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT16, &vid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_delete_vlan_source(unsigned int profile, unsigned short vid)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT16, &vid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);	
	return op_ret;
}

unsigned int dcli_mirror_add_fdb_source(unsigned int profile, unsigned short vlanId, unsigned int eth_g_index, ETHERADDR macAddr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
    unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_FDB);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_BYTE, &(macAddr.arEther[0]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[1]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[2]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[3]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[4]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[5]),
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{			
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_delete_fdb_source(unsigned int profile, unsigned short vlanId, unsigned int eth_g_index, ETHERADDR macAddr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
    unsigned int op_ret;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_FDB);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_BYTE, &(macAddr.arEther[0]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[1]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[2]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[3]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[4]),
							 DBUS_TYPE_BYTE, &(macAddr.arEther[5]),
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{	
        if(dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	    return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_add_remote_vlan(
	unsigned int profile, 
	unsigned short vid,
	unsigned int direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_REMOTE_VLAN_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT16, &vid,
							 DBUS_TYPE_UINT32, &direct,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int dcli_mirror_delete_remote_vlan(
	unsigned int profile, 
	unsigned short vid,
	unsigned int direct)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_REMOTE_VLAN_DEL);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT16, &vid,
							 DBUS_TYPE_UINT32, &direct,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return MIRROR_RETURN_CODE_DBUS_ERR;
	}
	if(dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);	
	return op_ret;
}


unsigned int dcli_mirror_get_by_profile(
	unsigned int profile,
	struct npd_mirror_item_s *dbItem    
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
    
    unsigned int *ptr_a =NULL, *ptr_b = NULL, *ptr_c = NULL, *ptr_d = NULL, *ptr_e = NULL;
	unsigned int  ret = 0;
    unsigned int  op_rc = MIRROR_RETURN_CODE_SUCCESS;
    unsigned int  length;
	unsigned int  port_count = 0;
	unsigned int  count = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_SHOW);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("get profile %d failed get reply.\n", profile);
		if(dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

   	if(!dbus_message_get_args(reply, &err,
	DBUS_TYPE_UINT32, &op_rc,
	DBUS_TYPE_BYTE,   &dbItem->profileId,
	DBUS_TYPE_UINT32, &dbItem->in_eth_index,
	DBUS_TYPE_UINT32, &dbItem->eg_eth_index,
	DBUS_TYPE_UINT32, &dbItem->in_remote_vid,
	DBUS_TYPE_UINT32, &dbItem->eg_remote_vid,	
	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_a, &length,
	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_b, &length,
	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_c, &length,
	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_d, &length,
	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_e, &length,
	DBUS_TYPE_INVALID))
    {		
	    if (dbus_error_is_set(&err)) 
	    {
		    printf("%s raised: %s", err.name, err.message);
		    dbus_error_free(&err);
	    }
    }

	memcpy(dbItem->in_eth_mbr.nbits, ptr_a, length);
	memcpy(dbItem->eg_eth_mbr.nbits, ptr_b, length);
	memcpy(dbItem->bi_eth_mbr.nbits, ptr_c, length);
	memcpy(dbItem->vlan_mbr.nbits, ptr_d, length);
	memcpy(dbItem->acl_mbr.nbits, ptr_e, length);
	
	dbus_message_unref(reply);					
	return op_rc;
}

unsigned int dcli_mirror_get_fdb_by_profile(
	unsigned int profile,
	unsigned int *fdb_count,
	struct fdb_entry_item_s **static_mirror_array
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int i = 0;
    unsigned int op_rc = MIRROR_RETURN_CODE_SUCCESS;
	unsigned char interfaceName[ALIAS_NAME_SIZE];

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_SHOW_FDB);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("get profile %d failed get reply.\n", profile);
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter, fdb_count);

    *static_mirror_array = malloc((*fdb_count) * sizeof(struct fdb_entry_item_s));

	if((*fdb_count) > 0)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter, &iter_array);
		for(i = 0; i < (*fdb_count); i++)
		{		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].vlanid));
			dbus_message_iter_next(&iter_struct);
			
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[0]));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[1]));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[2]));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[3]));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[4]));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mac[5]));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].blockMode));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].isStatic));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].isBlock));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].isAuthen));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].isMirror));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&((*static_mirror_array)[i].mirrorProfile));
			dbus_message_iter_next(&iter_struct);
		    dbus_message_iter_get_basic(&iter_struct, &((*static_mirror_array)[i].ifIndex));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_next(&iter_array);
		}	
	}
	dbus_message_unref(reply);
	return op_rc;
}

#ifdef __cplusplus
}
#endif
#endif

