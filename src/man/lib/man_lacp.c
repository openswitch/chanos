
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_lacp.c
*
*
* CREATOR:
*		anph@autelan.com
*
* DESCRIPTION:
*		CLI definition for LACP module.
*
* DATE:
*		11/08/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/
#ifdef HAVE_LACP
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "dbus/npd/npd_dbus_def.h"
#include "man_lacp.h"

extern DBusConnection *dcli_dbus_connection;
char *lacp_port_type[] = {
    "NULL",
    "NULL",
    "ETH_PORT",
    "NULL",
    "TRUNK"
};

int dcli_config_port_lacp_endis(unsigned int eth_g_index, unsigned int enDis)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_LACP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&enDis,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err); 
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
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

int dcli_config_all_port_lacp_enDis(unsigned int enDis)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
    
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ALL_PORT_LACP);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&enDis,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);  
    
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
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
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_aggregation_mode(unsigned short trunk_id, unsigned int enDis)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_TRUNK_OBJPATH ,	\
							NPD_DBUS_TRUNK_INTERFACE ,		\
							NPD_DBUS_AGGREGATOR_METHOD_MODE_CHANGE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&trunk_id,
							 	DBUS_TYPE_UINT32,&enDis,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);  
    
	dbus_message_unref(query);
    
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
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
#ifdef __cplusplus
}
#endif
#endif

