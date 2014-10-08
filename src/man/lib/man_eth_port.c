
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
#include <dbus/dbus.h>
#include "lib/osinc.h"

#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "man_str_parse.h"
#include "man_eth_port.h"
#include "npd/npd_eth_port.h"


extern DBusConnection *dcli_dbus_connection;

int man_set_eth_port_rate_poll(int rate_poll_enable)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_RATE_POLL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&rate_poll_enable,
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
		DBUS_TYPE_UINT32,&op_ret,
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

int man_get_eth_port_rate_poll(int *rate_poll_enable)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RATE_POLL);
	
	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32, rate_poll_enable,
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
	return ETHPORT_RETURN_CODE_ERR_NONE;
}

int dcli_eth_port_get_next_g_index(unsigned int eth_g_index, unsigned int *out_eth_g_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int dbus_ret = 0;
	query = dbus_message_new_method_call(
                                    NPD_DBUS_BUSNAME,                \
                                    NPD_DBUS_ETHPORTS_OBJPATH,          \
                                    NPD_DBUS_ETHPORTS_INTERFACE,        \
                                    NPD_DBUS_ETHPORTS_INTERFACE_METHOD_NEXT_GINDEX);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    						 DBUS_TYPE_UINT32,&eth_g_index,
    						 DBUS_TYPE_INVALID);
   	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
        
    dbus_message_unref(query);
    if (NULL == reply) {
	    if (dbus_error_is_set(&err))
	    {
	    	dbus_error_free(&err);
	    }
        return ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    dbus_ret = dbus_message_get_args(reply, &err,
        DBUS_TYPE_UINT32, &op_ret,
        DBUS_TYPE_UINT32, out_eth_g_index,
        DBUS_TYPE_INVALID);

    dbus_message_unref(reply);
    if(dbus_ret == FALSE)
    {
    	if (dbus_error_is_set(&err)) {
    		dbus_error_free(&err);
    	}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
	return op_ret;
}

int dcli_get_eth_port_attr(unsigned int eth_g_index, struct eth_port_s *port_info, unsigned char *lacp_sta, unsigned char *gvrp_sta)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	char *desc = NULL;
#ifdef HAVE_LACP
	unsigned char tmp_lacp_sta = 0;
#endif
	query = dbus_message_new_method_call(
										 NPD_DBUS_BUSNAME,	   \
										 NPD_DBUS_RELAY_OBJPATH,	   \
									     NPD_DBUS_RELAY_INTERFACE,	   \
									     NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR);
		   
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_INVALID);
		   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		   
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
	    DBUS_TYPE_UINT32,&(port_info->port_type),
		DBUS_TYPE_UINT32,&(port_info->state),
		DBUS_TYPE_UINT32,&(port_info->attr_bitmap),
		DBUS_TYPE_UINT32,&(port_info->mtu),
		DBUS_TYPE_UINT32,&(port_info->lastLinkChange),	  
		DBUS_TYPE_UINT32,&(port_info->eee),
#ifdef HAVE_LACP		
		DBUS_TYPE_BYTE, &tmp_lacp_sta,
#endif		
		DBUS_TYPE_STRING, &desc,
		DBUS_TYPE_INT32, &(port_info->forward_mode),
		DBUS_TYPE_UINT32, &port_info->loopback,
		DBUS_TYPE_UINT32, &port_info->bandwidth[0],
		DBUS_TYPE_UINT32, &port_info->bandwidth[1],
	    DBUS_TYPE_INVALID))) 
	{
	    if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;  
	} 
#ifdef HAVE_LACP
    *lacp_sta = tmp_lacp_sta;
#endif
	memcpy(port_info->desc, desc, 64);
    dbus_message_unref(reply);	  
	return op_ret;
}


int dcli_get_eth_port_ipg(unsigned int eth_g_index, unsigned char *port_ipg)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,	\
								NPD_DBUS_ETHPORTS_INTERFACE,	\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_IPG);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_BYTE,  port_ipg,
		DBUS_TYPE_INVALID))) {
			if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		    }
		    dbus_message_unref(reply);
			return ETHPORT_RETURN_CODE_ERR_GENERAL;
	   }
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_eth_port_get_one_port_index(unsigned int port_index, unsigned int* netif_index) 
{
	DBusMessage *query, *reply;
    
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int eth_g_index = port_index;
    unsigned int route;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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
					DBUS_TYPE_UINT32, netif_index,
					DBUS_TYPE_UINT32, &route,
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


int dcli_get_switchport(unsigned int netif_index, struct switch_port_db_s *port, char *untag_vlan_list, char *tag_vlan_list)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	char *get_untag_vlan_list = NULL;
	char *get_tag_vlan_list = NULL;

	
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH ,	\
							    NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_SWITCHPORT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&netif_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->state));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->vlan_access_mode));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->vlan_private_mode));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->fdb_limit));
        
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->pvid));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->stp_flag));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(port->fdb_learning_mode));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &get_untag_vlan_list);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &get_tag_vlan_list);
		strncpy(untag_vlan_list, get_untag_vlan_list, 256);
		strncpy(tag_vlan_list, get_tag_vlan_list, 256);  
	}

	dbus_message_unref(reply);

	return op_ret;
    
}
#ifdef HAVE_CHASSIS_SUPPORT
int dcli_clear_stack_stat_by_slotno_and_portno(unsigned char slot_no, unsigned char port_no)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
										NPD_DBUS_RELAY_OBJPATH, \
										NPD_DBUS_RELAY_INTERFACE, \
										NPD_DBUS_ETHPORTS_METHOD_CLEAR_STACK_STAT_BY_SLOTNO_AND_PORTNO);
	dbus_error_init(&error);
	dbus_message_append_args(query, 
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &port_no,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (reply == NULL)
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
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
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_get_port_stat_by_slotno_and_portno
(
	unsigned char slot_no, 
	unsigned char port_no, 
	struct eth_port_counter_s *ptr,
	unsigned int *linkupcount, 
	unsigned int *linkdowncount,
	unsigned int *link_state,
	unsigned char *mod,
	unsigned char *dev,
	unsigned char *port
	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	DBusMessageIter iter;
	DBusMessageIter iter_struct, iter_array;
	unsigned int op_ret = 0, i = 0;
	unsigned long long tmp = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
										NPD_DBUS_RELAY_OBJPATH, \
										NPD_DBUS_RELAY_INTERFACE, \
										NPD_DBUS_ETHPORTS_METHOD_SHOW_PORT_STAT_BY_SLOTNO_AND_PORTNO);
	dbus_error_init(&error);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &port_no,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	dbus_message_unref(query);
	if (reply == NULL)
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 26 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_next(&iter_array);
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,linkupcount);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,linkdowncount);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,link_state);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,mod);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,dev);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,port);
									
	}

	dbus_message_unref(reply);

	return op_ret;
}

int dcli_get_next_stack_port
(
	unsigned char slot_no, 
	unsigned char port_no, 
	unsigned char *out_slot_no,
	unsigned char *out_port_no,
	int *devNum,
	int *devPort,
	int *module_id
)
{
	unsigned char tmp_in_slot_no = slot_no;
	unsigned char tmp_in_port_no = port_no;
	unsigned int op_ret = 0;
	do
	{
		DBusMessage *query = NULL, *reply = NULL;
		DBusError error;
		DBusMessageIter iter;
		unsigned char tmp_slot_no = 0;
		unsigned char tmp_port_no = 0;
		int tmp_dev_num = 0;
		int tmp_dev_port = 0;
		int tmp_module_id = 0;
		
		op_ret = 0;
		query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_METHOD_GET_NEXT_STACK_PORT);
	
		dbus_error_init(&error);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE, &tmp_in_slot_no,
								DBUS_TYPE_BYTE, &tmp_in_port_no,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &error);
	
		dbus_message_unref(query);
	
		if (NULL == reply)
		{
			if (dbus_error_is_set(&error))
			{
				dbus_error_free(&error);
			}
			return ETHPORT_RETURN_CODE_ERR_GENERAL;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&op_ret);
		if (op_ret == BOARD_RETURN_CODE_NO_SUCH_SLOT)
		{
			dbus_message_unref(reply);
			return op_ret;
		}
		else if (op_ret == ETHPORT_RETURN_CODE_NO_SUCH_PORT)
		{
			tmp_in_slot_no += 1;
			tmp_in_port_no = 0;
		}
		else if (op_ret == ETHPORT_RETURN_CODE_ERR_NONE)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&tmp_slot_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&tmp_port_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&tmp_dev_num);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&tmp_dev_port);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&tmp_module_id);
			*out_slot_no = tmp_slot_no;
			*out_port_no = tmp_port_no;
			*devNum = tmp_dev_num;
			*devPort = tmp_dev_port;
			*module_id = tmp_module_id;
			dbus_message_unref(reply);
			return op_ret;
		}
		dbus_message_unref(reply);
	}while(op_ret != 0);
	return op_ret;
}

#endif


int dcli_get_eth_port_stat(unsigned int eth_g_index, eth_port_stats_t *ptr, unsigned int *linkupcount, unsigned int *linkdowncount)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0, i = 0;
	unsigned long long tmp = 0;


	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 64 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_next(&iter_array);
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,linkupcount);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,linkdowncount);
					
	}

	dbus_message_unref(reply);

	return op_ret;
}

int dcli_get_eth_port_rate(
    unsigned int eth_g_index, 
    unsigned int *inbandwidth,
    unsigned int *outbandwidth)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;


	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_RATE);
	

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,inbandwidth);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,outbandwidth);
					
	}

	dbus_message_unref(reply);

	return op_ret;
					
}




int dcli_get_eth_port_sfp_atrr(unsigned int eth_g_index, eth_port_sfp *sfpInfo)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int op_ret;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,	\
								NPD_DBUS_RELAY_INTERFACE,	\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_SFP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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
		DBUS_TYPE_INT32, &(sfpInfo->presense),		
		DBUS_TYPE_INT32, &(sfpInfo->laser),
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

int clear_eth_port_stat(unsigned int eth_g_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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


int dcli_config_eth_port(unsigned int eth_g_index, unsigned int *route, unsigned int *out_eth_g_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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
					DBUS_TYPE_UINT32, out_eth_g_index,
					DBUS_TYPE_UINT32, route,
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


int dcli_config_eth_port_descr(unsigned int eth_g_index, char *descr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORTDESC);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_STRING, &descr,
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

int dcli_port_ratelimit_set(unsigned int eth_g_index, int flow_dir, unsigned int bandwidth, unsigned int burstsize)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = INTERFACE_RETURN_CODE_SUCCESS;
    
        
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_RATELIMIT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_UINT32,&flow_dir,
							 	DBUS_TYPE_UINT32,&bandwidth,
							 	DBUS_TYPE_UINT32,&burstsize,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return op_ret;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			
	}
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;    
}


int dcli_config_eth_port_mode(unsigned int eth_g_index, unsigned int mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = INTERFACE_RETURN_CODE_SUCCESS;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
			
	}
	dbus_message_unref(reply);
	return op_ret;
}



int dcli_eth_port_admin_set(unsigned int eth_g_index, int mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int op_ret = 0;
	unsigned int isEnable = 1;
	unsigned int type = ADMIN;
	unsigned int port_index = 0;
    isEnable = mode;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&isEnable,
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
					DBUS_TYPE_UINT32, &port_index,
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




int dcli_config_ethport_ipg(unsigned int port_index, unsigned char port_ipg)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(	NPD_DBUS_BUSNAME,		\
										NPD_DBUS_ETHPORTS_OBJPATH ,	\
										NPD_DBUS_ETHPORTS_INTERFACE ,		\
										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &port_index,
								DBUS_TYPE_BYTE, &port_ipg,
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

int dcli_config_ethport_attr(unsigned int type, unsigned int port_index, unsigned int value, unsigned int *out_port_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int  op_ret = 0;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						   DBUS_TYPE_UINT32,&type,
						   DBUS_TYPE_UINT32,&port_index,
						   DBUS_TYPE_UINT32,&value,
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
					DBUS_TYPE_UINT32, out_port_index,
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



int dcli_config_eth_port_link_state(unsigned int port_index, unsigned int isEnable, unsigned int *out_port_index)
{
	DBusMessage *query = NULL , *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_SYSTEM_CONFIG_PORT_LINK);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
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
					DBUS_TYPE_UINT32, out_port_index,
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


int dcli_ethport_mtu_set(unsigned int port_index, int mtu, unsigned int *out_port_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int op_ret=0;
	unsigned int type = CFGMTU;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mtu,
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
					DBUS_TYPE_UINT32, out_port_index,
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

int dcli_get_eth_port_eee_avail(unsigned int port_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
 
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,  \
										NPD_DBUS_RELAY_OBJPATH,  \
										NPD_DBUS_RELAY_INTERFACE,  \
										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_EEE);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&port_index,
							 	DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);  
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
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

int dcli_config_eth_port_eee(unsigned int port_index, unsigned int isEnable)
{
 	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
 
	unsigned int 	op_ret = 0;
	int ret = 0;

	ret = dcli_get_eth_port_eee_avail(port_index);
	
	if (ret != ETHPORT_RETURN_CODE_ERR_NONE)
		return ETHPORT_RETURN_CODE_UNSUPPORT;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_EEE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&port_index,
							 	DBUS_TYPE_UINT32,&isEnable,
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

int dcli_config_eth_port_loopback(unsigned int port_index, unsigned int isEnable)
{
 	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
 
	unsigned int 	op_ret = 0;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_LOOPBACK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&port_index,
							 	DBUS_TYPE_UINT32,&isEnable,
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

int dcli_eth_port_get_running_config(unsigned int eth_g_index, char *showStr) 
{
	DBusMessage *query, *reply;
	DBusError err;

	char *get_show_str = NULL;
		
    query = dbus_message_new_method_call(
    						NPD_DBUS_BUSNAME,		\
    						NPD_DBUS_ETHPORTS_OBJPATH , \
    						NPD_DBUS_ETHPORTS_INTERFACE ,		\
    						NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RUNNING_CONFIG);
    dbus_message_append_args(query, 
                      DBUS_TYPE_UINT32, &eth_g_index,
                      DBUS_TYPE_INVALID);
    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

    dbus_message_unref(query);
    if (NULL == reply) {
    	if (dbus_error_is_set(&err)) {
    		dbus_error_free(&err);
    	}
    	return ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    if (!(dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_STRING, &get_show_str,
    				DBUS_TYPE_INVALID))) 
    {
		if (dbus_error_is_set(&err)) 
    	{
    		dbus_error_free(&err);
    	}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
    } 
	
	strncpy(showStr, get_show_str, NPD_ETHPORT_SHOWRUN_CFG_SIZE);
    dbus_message_unref(reply);   
	return ETHPORT_RETURN_CODE_ERR_NONE;
}


unsigned int dcli_eth_port_config_sc_common(unsigned char modeType, unsigned int g_index, unsigned int scMode, unsigned int scType, unsigned int scValue, unsigned int* ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH , \
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_STORM_CONTROL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&scMode,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_UINT32,&scValue,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {	
		return DCLI_ERROR_DBUS;
	}

	if (!dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	    return DCLI_ERROR_DBUS;
	}
	*ret = op_ret;
	dbus_message_unref(reply);
	return DCLI_ERROR_NONE;

}

unsigned int dcli_global_config_sc_common(unsigned int scType, unsigned int* ret)
{
    DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_STORM_CONTROL_GLOBAL_MODEL);


	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   *ret = 0;
       return DCLI_ERROR_DBUS;
	}

	if (! dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		*ret = 0;
		return DCLI_ERROR_DBUS;

	} 
    *ret = op_ret;
	dbus_message_unref(reply);
    return DCLI_ERROR_NONE;
}


int dcli_eth_port_config_vct(unsigned int eth_g_index, unsigned int mode, unsigned int *out_eth_g_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_VCT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&mode,
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
					DBUS_TYPE_UINT32, out_eth_g_index,
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

int dcli_eth_port_get_vct_info(unsigned int port_index, unsigned int *Isable, unsigned short *state, unsigned int *len, unsigned int *result, unsigned int *out_port_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_RELAY_OBJPATH ,	\
							NPD_DBUS_RELAY_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_READ_PORT_VCT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,100000, &err);
	
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
					DBUS_TYPE_UINT32, out_port_index,
					DBUS_TYPE_UINT32, Isable,
					DBUS_TYPE_UINT16, state,
					DBUS_TYPE_UINT32, len,
					DBUS_TYPE_UINT32, result,
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

#define NPD_PACKET_TYPE_OTHER 31
int man_get_eth_port_cpu_stat(unsigned int eth_g_index, unsigned int *rx_type_stats, unsigned int *rx_type_pps)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0, i = 0;
	unsigned int tmp = 0;


	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_CPU_STATS);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
	{
		dbus_message_iter_next(&iter);	

		for(i = 0; i < NPD_PACKET_TYPE_OTHER ; i++)
		{
			dbus_message_iter_get_basic(&iter,&tmp);
			rx_type_stats[i] = tmp;
			dbus_message_iter_next(&iter);
			tmp = 0;
		}
		for(i = 0; i < NPD_PACKET_TYPE_OTHER ; i++)
		{
			dbus_message_iter_get_basic(&iter,&tmp);
			rx_type_pps[i] = tmp;
			dbus_message_iter_next(&iter);
			tmp = 0;
		}
	}

	dbus_message_unref(reply);

	return op_ret;
}

int man_clear_eth_port_cpu_stats(unsigned int eth_g_index)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_RELAY_OBJPATH,		\
								NPD_DBUS_RELAY_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_CPU_STATS);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
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


unsigned int dcli_get_ethport_number(unsigned int eth_g_index)
{
	unsigned int ethport_number  = 0;
	while(1)
    {
    	struct eth_port_s port_info;
		memset(&port_info, 0, sizeof(struct eth_port_s));
		int ret = 0;
		

		ret = dcli_eth_port_get_next_g_index(eth_g_index, &eth_g_index);      
        if (ETHPORT_RETURN_CODE_ERR_NONE != ret )
        {
            break;
        }
		ethport_number++;
	}
	return ethport_number;
}



 int get_one_port_index
(
	unsigned int port_index,
	unsigned int* netif_index
) 
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = port_index;
    unsigned int route;

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, netif_index,
					DBUS_TYPE_UINT32, &route,
					DBUS_TYPE_INVALID)) 
	{
        char if_name[50];
        npd_netif_index_to_user_name(port_index, if_name);
		if (INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST == op_ret) 
		{
			dbus_message_unref(reply);
            return 1;
		}
		else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			dbus_message_unref(reply);
            return 1;
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)   
		{
			dbus_message_unref(reply);
			return 0;
		}
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}
#ifdef HAVE_STACKING
int man_set_eth_port_stacking(unsigned int port_index, int enable, int remote_unit)
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = port_index;

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_STACKING);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_UINT32,&remote_unit,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		return op_ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}
#endif


#ifdef HAVE_SERDES_CONFIG

int man_set_eth_port_serdes(unsigned int port_index, int amplitude, int amp_adj, int emp_en, int emp_level)
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = port_index;

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_SERDES);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&amplitude,
							 DBUS_TYPE_UINT32,&amp_adj,
							 DBUS_TYPE_UINT32,&emp_en,
							 DBUS_TYPE_UINT32,&emp_level,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		return op_ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}

int man_get_eth_port_serdes(unsigned int port_index, int *amplitude, int *amp_adj, int *emp_en, int *emp_level)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int op_ret;

	query = dbus_message_new_method_call(
							    NPD_DBUS_BUSNAME,		\
							    NPD_DBUS_ETHPORTS_OBJPATH ,	\
							    NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_SERDES);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
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
		DBUS_TYPE_INT32, amplitude,		
		DBUS_TYPE_INT32, amp_adj,
		DBUS_TYPE_INT32, emp_en),		
		DBUS_TYPE_INT32, emp_level,
		DBUS_TYPE_INVALID)) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

#endif

#ifdef HAVE_FORWARD_MODE_CONFIG

int man_set_eth_port_forward_mode(unsigned int port_index, int mode)
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = port_index;
	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_CUT_THROUGH);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		return op_ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}

int man_get_eth_port_forward_mode(unsigned int port_index, int *mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int op_ret;

	query = dbus_message_new_method_call(
							    NPD_DBUS_BUSNAME,		\
							    NPD_DBUS_ETHPORTS_OBJPATH ,	\
							    NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_CUT_THROUGH);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
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
		DBUS_TYPE_INT32, mode),
		DBUS_TYPE_INVALID)) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return ETHPORT_RETURN_CODE_ERR_GENERAL;
	} 
	dbus_message_unref(reply);
	return op_ret;
}

#endif

#ifdef HAVE_PORT_BUFFER_CONFIG
int man_set_eth_port_buffer_mode(int mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_BUFFER_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mode,
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
		DBUS_TYPE_UINT32,&op_ret,
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
int man_get_eth_port_buffer_mode(int *mode)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_BUFFER_MODE);
	
	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32, mode,
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
	return ETHPORT_RETURN_CODE_ERR_NONE;
}
#endif

#ifdef HAVE_STORM_CONTROL_GLB
int dcli_eth_port_config_sc_enable(unsigned int netif_index, int enable)
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = netif_index;
	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_SC_GLOBAL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		return op_ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}

int dcli_eth_port_config_sc_global(int sc_type, int kbps)
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_SC_GLOBAL_KBPS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&sc_type,
							 DBUS_TYPE_UINT32,&kbps,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		return op_ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
    	dbus_message_unref(reply);
    	return 1;
	}
    return 1;
}
#endif
#ifdef __cplusplus
}
#endif


