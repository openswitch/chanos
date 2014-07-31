
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* man_dhcp.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		CLI definition for VRRP module.
*
* DATE:
*		11/26/2011
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#ifdef HAVE_VRRP	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include "sysdef/returncode.h"
#include "dbus/had/vrrp_dbus.h"
#include "man_vrrp.h"

extern DBusConnection *dcli_dbus_connection;

int man_delete_ip_vrrp_by_vrid(unsigned int vrrp_id, unsigned char* ifname)
{
    unsigned int op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_DELETE_ONE_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &vrrp_id,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}

	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        dbus_message_unref(reply);
		return VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}
int man_vrrp_interface_set
(
	unsigned int profile,
	unsigned int flag,
	unsigned char* ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_ENABLE_BY_VRID);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}

	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int vrrp_config_virtul_ip
(
	unsigned int profile,
	char        *ifname,
	unsigned int virtual_ip,
	unsigned int add
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int mask = 32;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &virtual_ip,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_UINT32, &add,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	else if (!dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int vrrp_config_virtul_ip_track
(
	unsigned int profile,
	char *ifname,
	char *track_ifname,
	unsigned int reducePriority,
	int addFlag
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;

    dcli_netif_name_convert_to_interface_name(track_ifname, track_ifname);

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_TRACK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
		                     DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_STRING, &track_ifname,
							 DBUS_TYPE_UINT32, &reducePriority,
							 DBUS_TYPE_UINT32, &addFlag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	else if (!dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int vrrp_config_virtul_ip_tracking_group
(
	unsigned int vrrp_id,
	unsigned int tracking_group,
    unsigned int tracking_group_status,
	unsigned int reduce_priority,
	char *ifname,
	unsigned int* op_ret
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
    int ret = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_TRACK_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &vrrp_id,
		                     DBUS_TYPE_UINT32, &tracking_group,
							 DBUS_TYPE_UINT32, &tracking_group_status,
							 DBUS_TYPE_UINT32, &reduce_priority,
		                     DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = -1;
	}

    if (!dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, op_ret,
									DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = -1;
	}
	
	dbus_message_unref(reply);
    
	return ret;
}

int config_vrrp_service
(
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&enable,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	else if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int config_vrrp_priority
(
	unsigned int profile,
	unsigned int priority,
	unsigned char* ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PROFILE_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &priority,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int config_vrrp_preempt
(
	unsigned int profile,
	unsigned int preempt,
	unsigned int delay,
	unsigned char* ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PREEMPT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &preempt,
							 DBUS_TYPE_UINT32, &delay,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int config_vrrp_advertisement_timer
(
	unsigned int profile,
	unsigned int advert,
	unsigned char* ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_ADVERT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &advert,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int config_vrrp_virtrul_mac
(
	unsigned int profile,
	unsigned int no_vmac,
	unsigned char* ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	
	
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &no_vmac,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	    return -1;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_vrrp_set_sm_down_count(int count)
{
    DBusMessage* query = NULL;
    DBusMessage* reply = NULL;
	DBusError err = {0};
    unsigned int op_ret = 0;
    
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
									 VRRP_DBUS_OBJPATH,
									 VRRP_DBUS_INTERFACE,
									 VRRP_DBUS_METHOD_SET_MS_D_COUNT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &count,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int vrrp_get_global
(
	struct vrrp_global_info *global
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_GET_GLOBAL);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
		dbus_message_unref(reply);
	    return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(global->enable));
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&(global->ms_d_count));

	dbus_message_unref(reply);
	return VRRP_RETURN_CODE_OK;
}


int man_vrrp_get_next
(
	struct vrrp_info *vrrp,
	unsigned int specify_vrrp_id_flag
)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0, vrid = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0;
	unsigned int priority_actual;
    char *uplink_ifname = NULL;
	char *track_ifname = NULL; 
	unsigned int reducePrio;
	unsigned int uplink_ipaddr;
	unsigned int uplink_set_flg = 0;
	unsigned int delay_int = 0;
	int i = 0;
	int uplink_naddr = 0;


	vrid = vrrp->vrid;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&specify_vrrp_id_flag,	
		                     DBUS_TYPE_UINT32,&vrid,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	memset(vrrp, 0, sizeof(struct vrrp_info));
	vrrp->vrid = vrid;
	
	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
		dbus_message_unref(reply);
	    return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
   	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vrrp->admin_enable);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vrrp->vrid);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);
	vrrp->state = state;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	vrrp->priority = priority;

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority_actual);
	vrrp->priority_actual= priority_actual;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	vrrp->advert = advert;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	vrrp->preempt = preempt;

	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &track_ifname);
    if (0 != track_ifname[0])
    {
	    strncpy(vrrp->track_ifname, track_ifname,strlen(track_ifname));
        dcli_interface_name_convert_to_netif_name(vrrp->track_ifname, vrrp->track_ifname);
    }

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &vrrp->tracking_group);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &reducePrio);
	vrrp->reduce_priority = reducePrio;

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&delay_int);
	vrrp->delay_int = delay_int;

    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &vrrp->virtual_mac_flag);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_set_flg);

	if (1 == uplink_set_flg)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &uplink_ifname);
        if (0 != uplink_ifname[0])
        {
		    strncpy(vrrp->ifname,uplink_ifname,strlen(uplink_ifname));
            dcli_interface_name_convert_to_netif_name(vrrp->ifname, vrrp->ifname);
        }
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_naddr);
		vrrp->naddr = uplink_naddr;
		
		if(uplink_naddr > 0)
		{
			for(i = 0; i < uplink_naddr; i++)
			{   
			    dbus_message_iter_next(&iter);
	            dbus_message_iter_get_basic(&iter,&uplink_ipaddr);
				vrrp->ipAddr[i] = uplink_ipaddr;
    		}
		}
	}
	
	dbus_message_unref(reply);
	return VRRP_RETURN_CODE_OK;
}

int config_vrrp_debug_value
(
	unsigned int flag
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;

	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										VRRP_DBUS_OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_BRG_DBUG_VRRP);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		printf("failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{		
		printf("Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return ret;
}
	
int config_vrrp_no_debug_value
(
	unsigned int flag
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		printf("failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return VRRP_RETURN_CODE_ERR;
	}
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{		
		printf("Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = VRRP_RETURN_CODE_ERR;
	}
	
	dbus_message_unref(reply);
	return ret;
}
#endif
#ifdef __cplusplus
}
#endif


