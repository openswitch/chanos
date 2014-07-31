
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
#include "dbus/pimd/mrt_dbus_def.h"
#include "man_pim.h"

extern DBusConnection *dcli_dbus_connection;

#ifdef HAVE_PIM
int get_pim_mode(unsigned int *pim_mode)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    unsigned int op_ret = 0;
    int dm_isrunning;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
                                     				    PIMD_DBUS_OBJPATH, 
                                     				    PIMD_DBUS_INTERFACE, 
                                     				    PIMD_DBUS_INTERFACE_METHOD_SHOW_MODE);
    dbus_error_init(&err);
    dbus_message_append_args(query,DBUS_TYPE_INVALID);
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
    DBUS_TYPE_UINT32, &dm_isrunning,
	DBUS_TYPE_INVALID))
    {
        if(dm_isrunning)
	      *pim_mode = PIM_M_DM;
        else 
          *pim_mode = PIM_M_SM;
    }
	else 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

int get_pim_next_mrt(unsigned char flag, struct pim_mrt_info *info)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    unsigned int op_ret = 0;
    DBusMessageIter iter;
    u_int32 wc_count;
    u_int32 sg_count;
    u_int32 pmbr_count;
    u_int32 source;
    u_int32 group;
    u_int32  rp;
    u_int32  upstream;
    u_int32  entry_flag;
    u_int16	  timer;
    u_int16	  jp_timer;
    u_int16  rs_timer;
    u_int	  assert_timer;
    u_int16     oifs_num;
    char       *vifi_name;
    char        vifi_flag;
    u_int16  vifi_timer;
    int mode;
    int i;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
                                     					PIMD_DBUS_OBJPATH, 
                                     					PIMD_DBUS_INTERFACE, 
                                     					PIMD_DBUS_INTERFACE_METHOD_SHOW_MRT);
    dbus_error_init(&err);
    dbus_message_append_args(query,
					            DBUS_TYPE_BYTE, &flag,
					            DBUS_TYPE_UINT32, &info->source,
                                                         DBUS_TYPE_UINT32, &info->group,
                                                         DBUS_TYPE_UINT32, &info->mask,
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

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    dbus_message_iter_next(&iter);	
    dbus_message_iter_get_basic(&iter, &mode);
    dbus_message_iter_next(&iter);
    if (0 == op_ret){
        memset(info, 0, sizeof(struct pim_mrt_info) + sizeof(vifbitmap_t)*16);
      
        if (mode)
        {
            dbus_message_iter_get_basic(&iter, &source);
            info->source = source;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &group);
            info->group = group;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &upstream);
            info->upstream = upstream;
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &sg_count);
            info->sg_count = sg_count;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &vifi_name);
            strncpy(info->incoming.name, vifi_name, IFNAMSIZ);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &oifs_num);
            info->oifs_num = oifs_num;
            for (i = 0; i < oifs_num; i++){
                dbus_message_iter_next(&iter);	
                dbus_message_iter_get_basic(&iter, &vifi_name);
                strncpy(info->oif[i].name, vifi_name, IFNAMSIZ);
            }
        }
        else
        {
            dbus_message_iter_get_basic(&iter, &source);
            info->source = source;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &group);
            info->group = group;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &rp);
            info->rp = rp;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &upstream);
            info->upstream = upstream;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &entry_flag);
            info->entry_flag = entry_flag;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &wc_count);
            info->wc_count = wc_count;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &sg_count);
            info->sg_count = sg_count;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &pmbr_count);
            info->pmbr_count = pmbr_count;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &vifi_name);
            strncpy(info->incoming.name, vifi_name, IFNAMSIZ);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &timer);
            info->timer = timer;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &jp_timer);
            info->jp_timer = jp_timer;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &rs_timer);
            info->rs_timer = rs_timer;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &assert_timer);
            info->assert_timer = assert_timer;
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter, &oifs_num);
            info->oifs_num = oifs_num;
            
            for (i = 0; i < oifs_num; i++){
                dbus_message_iter_next(&iter);	
                dbus_message_iter_get_basic(&iter, &vifi_name);
                strncpy(info->oif[i].name, vifi_name, IFNAMSIZ);
                dbus_message_iter_next(&iter);	
                dbus_message_iter_get_basic(&iter, &vifi_flag);
                info->oif[i].flag = vifi_flag;
                dbus_message_iter_next(&iter);	
                dbus_message_iter_get_basic(&iter, &vifi_timer);
                info->oif[i].timer = vifi_timer;
            }
        }
    }
    
    dbus_message_unref(reply);
    return op_ret;
}

u_int32 set_igmp_version(char *ifname, u_int32 version)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_IGMPVER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &version, 
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

u_int32 set_pim_ssm_range_cofig(u_int32 flag, u_int32 addr, u_int32 mask)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_PIM_SSM_RANGE_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
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

u_int32 set_bidir_pim_capability(u_int32 flag)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMd_DBUG_INTERFACE_METHOD_BIDIR_PIM_ENABLE);
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
    
u_int32 set_pim_spt_switch_threshold(u_int32 flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SPT_SWITCH_THRESHOLD);
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

u_int32 get_pim_mrt_state()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPMRT_GETSTATE);
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


u_int32 set_pim_mrt_route(u_int32 flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPMRT_ENABLE);
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

u_int32  mode_selete(u_int32 flag)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
								PIMD_DBUS_OBJPATH, 
								PIMD_DBUS_INTERFACE, 
								PIMD_DBUS_INTERFACE_METHOD_IPPIM_MODE_SELETE);
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
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return 1;
	}
	
	return op_ret;

}

u_int32  set_vif_pim(u_int32 flag, char *ifname, char *addr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE);
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

unsigned int set_vif_pim_hello_option
(
	unsigned char flag,
	unsigned int addr, 
	unsigned int mask, 	
	unsigned int option,
	unsigned int value
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPPIM_HELLO_OPTION);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag, 
							 DBUS_TYPE_UINT32, &addr,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_UINT32, &option,
							 DBUS_TYPE_UINT32, &value,
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


u_int32  set_vif_pim_byaddr(unsigned char mode, unsigned char flag, u_int32 addr, u_int32 mask)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE_BYADDR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_BYTE, &mode,	
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

u_int32 set_pim_dm_state_refresh_capability(u_int32 flag)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_PIMDM_STATE_REFRESH_ENABLE);

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
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return 1;
	}
	
	return op_ret;
    
}


u_int32 set_state_refresh_interval_value(u_int32 flag, u_int32 time)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_PIMDM_STATE_REFRESH_INTERVAL);

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

u_int32 set_pim_bsr_candidate(u_int32 flag, char *addr, u_int32 hmasklen, u_int32 priority)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_BSR_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag, 
							DBUS_TYPE_STRING,&addr,							
							DBUS_TYPE_UINT32,&hmasklen,
							DBUS_TYPE_UINT32,&priority,							
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
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;

}

u_int32 no_pim_bsr_candidate(void)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 enable = 0;
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_NO_BSR_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
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
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;

}

u_int32 set_pimd_rp_candidate(u_int32 flag, char *addr, u_int32 priority, u_int32 time)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 enable = 1;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_STRING, &addr,
							DBUS_TYPE_UINT32,&priority,
							DBUS_TYPE_UINT32,&time,
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
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	} 

	dbus_message_unref(reply);
	return op_ret;

}

u_int32 set_pim_rp_candidate_grp(u_int32 flag, u_int32 rp_addr, u_int32 grp_addr, u_int32 mask)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE_GRP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag, 
							DBUS_TYPE_UINT32, &rp_addr, 
							DBUS_TYPE_UINT32, &grp_addr, 
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
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

u_int32 set_pim_rp_address(u_int32 flag, u_int32 rp_addr, u_int32 grp_addr, u_int32 mask)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SET_RP_STATIC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag, 
							DBUS_TYPE_UINT32, &rp_addr, 
							DBUS_TYPE_UINT32, &grp_addr, 
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
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

u_int32 set_pim_query_interval(u_int32 flag, char *ifname, u_int32 interval)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32  op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_QUERY_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,	
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &interval,
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

u_int32 set_pim_message_interval(u_int32 flag, u_int32 interval)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_MESSAGE_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,	
							 DBUS_TYPE_UINT32, &interval,
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

u_int32 set_pim_mroute_static(u_int32 flag, char *source, char *group, char *vifi_name)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_IPMRT_STATIC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,	
							 DBUS_TYPE_STRING, &source,
                                                DBUS_TYPE_STRING, &group,
                                                DBUS_TYPE_STRING, &vifi_name,
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

u_int32 clear_pim_mroute(char *addr)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_CLEAR_IPMRT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
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

u_int32 clear_pim_mfib(void)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 enable = 1;

	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_CLEAR_MFIB);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable,	
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
#if 0
static struct debugname {
    char *name;
    int	 level;
    int	 nchars;
} debugnames[] = {
    {   "igmp_proto",	    DEBUG_IGMP_PROTO,     6	    },
    {   "igmp_timers",	    DEBUG_IGMP_TIMER,     6	    },
    {   "igmp_members",	    DEBUG_IGMP_MEMBER,    6	    },
    {   "groups",	    DEBUG_MEMBER,         1	    },
    {   "igmp",	            DEBUG_IGMP, 	  1	    },
    {   "trace",	    DEBUG_TRACE,          2	    },
    {   "timeout",	    DEBUG_TIMEOUT,        2	    },
    {   "packets",	    DEBUG_PKT,  	  2	    },
    {   "interfaces",       DEBUG_IF,   	  2	    },
    {   "kernel",           DEBUG_KERN,           2	    },
    {   "cache",            DEBUG_MFC,   	  1	    },
    {   "pim_detail",       DEBUG_PIM_DETAIL,     5	    },
    {   "pim_hello",        DEBUG_PIM_HELLO,      5	    },
    {   "pim_register",     DEBUG_PIM_REGISTER,   5	    },
    {   "pim_join_prune",   DEBUG_PIM_JOIN_PRUNE, 5	    },
    {   "pim_bootstrap",    DEBUG_PIM_BOOTSTRAP,  5	    },
    {   "pim_asserts",      DEBUG_PIM_ASSERT,     5	    },
    {   "pim_cand_rp",      DEBUG_PIM_CAND_RP,    5	    },
    {   "pim_routing",      DEBUG_PIM_MRT,        6	    },
    {   "pim_timers",       DEBUG_PIM_TIMER,      5	    },
    {   "pim_rpf",          DEBUG_PIM_RPF,        6	    },
    {   "rpf",              DEBUG_RPF,            3	    },
    {   "pim",              DEBUG_PIM,  	  1	    },
    {   "routes",	    DEBUG_MRT,            1	    },
    {   "routers",          DEBUG_NEIGHBORS,      6	    },
    {   "timers",           DEBUG_TIMER,          1	    },
    {   "asserts",          DEBUG_ASSERT,         1	    },
    {   "all",              DEBUG_ALL,            2         },
};
#endif
static struct debugname {
	char *name;
	int	 level;
	int	 nchars;
} debugnames[] = {
    { "packet", DEBUG_PACKET, 6 },
    { "event",  DEBUG_EVENT,  5 },
    { "timer",  DEBUG_TIMER,  5 },
    { "kernel", DEBUG_KERNEL, 6 },
    { "all",    DEBUG_ALL,    3 }
};

u_int32 set_debug_pim_func(u_int32 flag, char *str)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 debug;
	int detect = 0,i=0;
	struct debugname *d;

	
	for (i = 0, d = debugnames;
	 i < sizeof(debugnames) / sizeof(debugnames[0]);
	 i++, d++)
	 {
		if(strncmp(d->name,str,strlen(str))==0)
			break;
	 }
	 if(i>=sizeof(debugnames) / sizeof(debugnames[0])){
	 	printf("Error debug name %s\n",str);
		return 1;
	 }
	 
	 debug = d->level;
	 
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_DEBUG_MRT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &flag, 
								DBUS_TYPE_UINT32, &debug, 
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
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &flag,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;
}

u_int32 man_pim_info_get(struct pim_info *info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 state = 0, mode = 0, grp = 0, masklen = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_PIM_INFO);
	dbus_error_init(&err);
	dbus_message_append_args(query,
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
	
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    dbus_message_iter_next(&iter);	
    memset(info, 0, sizeof(struct pim_info));
    if (0 == op_ret){
        dbus_message_iter_get_basic(&iter,&info->state);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->mode);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->ssm_range[0].group);
		dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->ssm_range[0].masklen);
        dbus_message_iter_next(&iter);
    }
	
	dbus_message_unref(reply);
	return op_ret;
}


u_int32 get_pim_rp_candidate(struct pim_rp_candidate *info)
{
         u_int32    addr;
         u_int8      priority;
         u_int16    time;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	u_int32 enable = 1;
    
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable, 
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
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &addr,	
		DBUS_TYPE_BYTE, &priority,
		DBUS_TYPE_UINT16, &time,
		DBUS_TYPE_INVALID)) 
	{
                  info->addr = addr;
                  info->priority = priority;
                  info->time = time;
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

u_int32 get_pim_rp_candidate_group(struct pim_rp_set *info)
{
    u_int32	addr = info->grp_addr;
    u_int8 	masklen = (u_int8)info->masklen;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	u_int32 op_ret = 0;
	
	query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE_GRP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &addr, 
							DBUS_TYPE_BYTE, &masklen,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &addr,	
					DBUS_TYPE_BYTE, &masklen,
					DBUS_TYPE_INVALID)) 
	{
		info->grp_addr = addr;
		info->masklen = masklen;
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}


u_int32 get_pim_bsr_router(struct pim_bsr_router *info)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    u_int32 op_ret = 0;
    u_int32 enable = 1;
    DBusMessageIter iter;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
    								PIMD_DBUS_OBJPATH, 
    								PIMD_DBUS_INTERFACE, 
    								PIMD_DBUS_INTERFACE_METHOD_SHOW_BSR);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    						DBUS_TYPE_UINT32, &enable, 
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

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    dbus_message_iter_next(&iter);	
    memset(info, 0, sizeof(struct pim_bsr_router));
    if (2 == op_ret){
        dbus_message_iter_get_basic(&iter,&info->my_addr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->my_masklen);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->my_priority);
        dbus_message_iter_next(&iter);
    }
    if(0==op_ret || 2==op_ret){
        dbus_message_iter_get_basic(&iter,&info->curr_addr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->curr_masklen);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->curr_priority);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->curr_fragment_tag);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&info->curr_timer);
    }

    dbus_message_unref(reply);
    return op_ret;
}

u_int32 get_pim_next_rp(struct pim_rp_set *info)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    u_int32 op_ret = 0;
    char *ifname;
    DBusMessageIter iter;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
    							PIMD_DBUS_OBJPATH, 
    							PIMD_DBUS_INTERFACE, 
    							PIMD_DBUS_INTERFACE_METHOD_SHOW_RP);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32, &info->rp_addr,
    					DBUS_TYPE_UINT32, &info->grp_addr,
    					DBUS_TYPE_UINT32, &info->masklen,
    					DBUS_TYPE_BYTE, &info->priority,
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
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    if (0==op_ret || 4==op_ret){
        if(0==op_ret){
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->rp_addr);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&ifname);
            strcpy(info->iif_name, ifname);
        }
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->grp_addr);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->masklen);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->priority);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->holdtime);
    }

    dbus_message_unref(reply);
    return op_ret;
}

u_int32 get_pim_next_rp_static(struct pim_rp_set *info)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    u_int32 op_ret = 0;
    char *ifname;
    DBusMessageIter iter;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
    							PIMD_DBUS_OBJPATH, 
    							PIMD_DBUS_INTERFACE, 
    							PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_STATIC);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32, &info->rp_addr,
    					DBUS_TYPE_UINT32, &info->grp_addr,
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
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    if (0==op_ret || 4==op_ret){
        if(0==op_ret){
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->rp_addr);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&ifname);
            strcpy(info->iif_name, ifname);
        }
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->grp_addr);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->masklen);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->priority);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->holdtime);
    }

    dbus_message_unref(reply);
    return op_ret;
}

u_int32 get_vif_info(u_int32 *vif_num, struct pim_vif_info *info)
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    unsigned int op_ret = 0;
    unsigned int enable = 1;
    DBusMessageIter iter;
    char  *net;
    u_int32 vif_count;
    u_int32 neigh_count;
    char *vif_name;

    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
    								PIMD_DBUS_OBJPATH, 
    								PIMD_DBUS_INTERFACE, 
    								PIMD_DBUS_INTERFACE_METHOD_SHOW_IF);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    						DBUS_TYPE_UINT32, &enable, 
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
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    if(0==op_ret){
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&vif_count);
        *vif_num = vif_count;
        int i,j;
        for(i=0; i<vif_count; i++){
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&vif_name);
            strcpy(info->name, vif_name);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->addr);
            dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&info->holdtime);
            dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&info->priority);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->flag);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&net);
            strcpy(info->netname, net);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->threshold);
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->neigh_num);
            neigh_count = MIN(info->neigh_num, 10);
            for(j=0; j<neigh_count;j++){
                dbus_message_iter_next(&iter);	
                dbus_message_iter_get_basic(&iter,&info->neigh_addr[j]);
            }
            info = (char *)info+ sizeof(struct pim_vif_info) + neigh_count * 4;
        }
    }
    dbus_message_unref(reply);
    return op_ret;

}

u_int32 get_vif_info_byname(char *ifname, struct pim_vif_info *info)
{
    DBusMessageIter iter;
    char  *net;
    u_int32 neigh_count;
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    unsigned int op_ret = 0;
    unsigned int enable = 1;
	
    query = dbus_message_new_method_call(PIMD_DBUS_BUSNAME, 
									PIMD_DBUS_OBJPATH, 
									PIMD_DBUS_INTERFACE, 
									PIMD_DBUS_INTERFACE_METHOD_SHOW_IF1);
    dbus_error_init(&err);
    dbus_message_append_args(query,
    						DBUS_TYPE_UINT32, &enable, 
    						DBUS_TYPE_STRING,&ifname,							
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
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    if(0==op_ret){
        strcpy(info->name, ifname);
        int j;
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->addr);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->flag);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&net);
        strcpy(info->netname, net);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->threshold);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&info->neigh_num);
        neigh_count = MIN(info->neigh_num, 10);
        for(j=0; j<info->neigh_num;j++){
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&info->neigh_addr[j]);
        }
    }
    dbus_message_unref(reply);
    return op_ret;
}
#endif
