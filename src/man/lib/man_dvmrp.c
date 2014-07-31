
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */

#include <stdio.h>
#include <string.h> 
#include <ctype.h>
#include <zebra.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include "vtysh/vtysh.h"
#include "command.h"
#include "dbus/dvmrp/dvmrp_dbus_def.h"
#include "man_dvmrp.h"

#ifdef HAVE_DVMRP
extern DBusConnection *dcli_dbus_connection;

unsigned int get_dvmrp_mrt_state()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
                              			DVMRP_DBUS_OBJPATH, 
                              			DVMRP_DBUS_INTERFACE, 
                              			DVMRP_DBUS_INTERFACE_METHOD_IPMRT_GETSTATE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
                  printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else 
        {       
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return 1;
	}
	
	return op_ret;
}

unsigned int set_dvmrp_mrt_route(unsigned int flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int  op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_INTERFACE_METHOD_IPMRT_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag, 
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else 
	{       
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;
}

#if 0
unsigned int  set_inf_dvmrp(unsigned int flag, char *ifname, char *addr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_INTERFACE_METHOD_IPDVMRP_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_STRING, &ifname,
			DBUS_TYPE_STRING, &addr,
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;

}
#endif

unsigned int set_inf_metric(unsigned int flag, char *ifname, unsigned int metric)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_INTERFACE_METRIC_SET);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_STRING, &ifname,
			DBUS_TYPE_UINT32, &metric,	
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;

}

unsigned int set_vif_dvmrp_byaddr(unsigned char flag, unsigned int addr, unsigned int mask)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
									DVMRP_DBUS_OBJPATH, 
									DVMRP_DBUS_INTERFACE, 
									DVMRP_DBUS_INTERFACE_METHOD_IPDVMRP_ENABLE_BYADDR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag,	
							 DBUS_TYPE_UINT32, &addr,
                             DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {		
                  printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}
	
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &flag,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return 1;
	}
	
	return op_ret;
}

unsigned int set_nbr_timeout_value(unsigned int flag, unsigned int time)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_NBR_TIMEOUT_SET);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_UINT32, &time,	
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;

}


unsigned int set_probe_interval_value(unsigned int flag, unsigned int time)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_PROBE_INTERVAL_SET);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_UINT32, &time,	
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;

}

unsigned int set_report_interval_val(unsigned int flag,unsigned int time)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_REPORT_INTERVAL_SET);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_UINT32, &time,	
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}

	return op_ret;
}

unsigned int set_route_timeout_val(unsigned int flag, unsigned int time)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME, 
			DVMRP_DBUS_OBJPATH, 
			DVMRP_DBUS_INTERFACE, 
			DVMRP_DBUS_ROUTE_TIMEOUT_SET);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &flag,	
			DBUS_TYPE_UINT32, &time,	
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 1;
	}

	if (dbus_message_get_args(reply, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return 1;
	}  	
	return op_ret;
}

unsigned int get_dvmrp_next_mrt(unsigned char flag, struct dvmrp_mrt_info *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	DBusMessageIter iter;
	u_int32  sg_count;
	u_int32 source;
	u_int32 group;
	u_int16  oifs_num;
	char    *vifi_name;
	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME,
			DVMRP_DBUS_OBJPATH,
			DVMRP_DBUS_INTERFACE,
			DVMRP_DBUS_INTERFACE_METHOD_SHOW_MRT);

	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_BYTE, &flag,
			DBUS_TYPE_UINT32, &info->source,
			DBUS_TYPE_UINT32, &info->group,
			DBUS_TYPE_UINT32, &info->mask,
			DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
			dbus_error_free(&err);

		return 1;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if (0 == op_ret)
	{
		memset(info, 0, sizeof(struct dvmrp_mrt_info) + sizeof(vifbitmap_t)*16);
		dbus_message_iter_get_basic(&iter, &source);
		info->source = source;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &group);
		info->group = group;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &sg_count);
		info->sg_count = sg_count;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &vifi_name);
		strncpy(info->incoming.name, vifi_name, IFNAMSIZ);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &oifs_num);
		info->oifs_num = oifs_num;
		int i;
		for (i = 0; i < oifs_num; i++){
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter, &vifi_name);
			strncpy(info->oif[i].name, vifi_name, IFNAMSIZ);
		}
	}

	dbus_message_unref(reply);
	return op_ret;
}


unsigned int get_dvmrp_next_route(unsigned char flag, struct dvmrp_route_info  *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	DBusMessageIter iter;
	u_int16  oifs_num;
	char    *vifi_name;
	u_int32  origin;
	u_int32  mask;
	u_int32  gateway;
	u_char     metric;
	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME,
			DVMRP_DBUS_OBJPATH,
			DVMRP_DBUS_INTERFACE,
			DVMRP_DBUS_INTERFACE_METHOD_SHOW_ROUTE);

	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_BYTE, &flag,
			DBUS_TYPE_UINT32, &info->origin,
			DBUS_TYPE_UINT32, &info->originmask,
			DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
			dbus_error_free(&err);

		return 1;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if (0 == op_ret)
	{
		memset(info, 0, sizeof(struct dvmrp_route_info) + sizeof(vifbitmap_t)*16);
		dbus_message_iter_get_basic(&iter, &origin);
		info->origin = origin;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &mask);
		info->originmask = mask;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &gateway);
		info->gateway = gateway;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &metric); 
		info->metric = metric;
	}

	dbus_message_unref(reply);
	return op_ret;

}

unsigned int get_dvmrp_next_nbr(unsigned char flag, struct dvmrp_nbr_info *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	DBusMessageIter iter;

	u_int32 nbr_addr;
	char *vifi_name;
	u_char  pv, mv;
	u_int32 nbr_num;



	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME,
			DVMRP_DBUS_OBJPATH,
			DVMRP_DBUS_INTERFACE,
			DVMRP_DBUS_INTERFACE_METHOD_SHOW_NBR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_BYTE, &flag,
			DBUS_TYPE_UINT32, &info->nbr_addr,
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
			dbus_error_free(&err);

		return 1;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if (0 == op_ret)
	{
		memset(info, 0 ,sizeof(struct dvmrp_nbr_info));
		dbus_message_iter_get_basic(&iter,&nbr_addr);
		info->nbr_addr = nbr_addr;
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vifi_name);
		strncpy(info->incoming.name, vifi_name, IFNAMSIZ);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&pv);
		info->pv = pv;
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mv);
		info->mv = mv; 
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&nbr_num);
		info->nbr_count= nbr_num;
	}
	dbus_message_unref(reply);
	return op_ret;

}

unsigned int get_dvmrp_next_vif(unsigned char flag, struct dvmrp_vif_info *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	DBusMessageIter iter;

	u_int32 vif_addr;
	char *vifi_name;
	u_char threshold;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME,
			DVMRP_DBUS_OBJPATH,
			DVMRP_DBUS_INTERFACE,
			DVMRP_DBUS_INTERFACE_METHOD_SHOW_VIF);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_BYTE, &flag,
			DBUS_TYPE_UINT32, &info->addr,
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
			dbus_error_free(&err);

		return 1;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if (0 == op_ret)
	{
		memset(info, 0 ,sizeof(struct dvmrp_nbr_info));
		dbus_message_iter_get_basic(&iter,&vifi_name);
		strncpy(info->name, vifi_name, IFNAMSIZ);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vif_addr);
		info->addr = vif_addr;
		dbus_message_iter_next(&iter);    
		dbus_message_iter_get_basic(&iter,&threshold);
		info->threshold = threshold;
	}
	dbus_message_unref(reply);
	return op_ret;
}

unsigned int get_dvmrp_info(struct dvmrp_info *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	DBusMessageIter iter;

	u_int32  probe_interval;
	u_int32  nbr_timeout;
	u_int32  report_interval;
	u_int32  route_timeout;

	query = dbus_message_new_method_call(DVMRP_DBUS_BUSNAME,
			DVMRP_DBUS_OBJPATH,
			DVMRP_DBUS_INTERFACE,
			DVMRP_DBUS_CONFIG_SHOW);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
			dbus_error_free(&err);

		return 1;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if (0 == op_ret)
	{
		memset(info, 0 ,sizeof(struct dvmrp_info));
		dbus_message_iter_get_basic(&iter,&probe_interval);
		info->probe_interval = probe_interval;
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&nbr_timeout);
		info->nbr_timeout = nbr_timeout;
		dbus_message_iter_next(&iter);    
		dbus_message_iter_get_basic(&iter,&report_interval);
		info->report_interval = report_interval;
		dbus_message_iter_next(&iter);    
		dbus_message_iter_get_basic(&iter,&route_timeout);
		info->route_timeout = route_timeout;
	}
	dbus_message_unref(reply);
	return op_ret;
}
#endif






