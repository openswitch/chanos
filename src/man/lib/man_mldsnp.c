
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_MLD_SNP
#ifdef __cplusplus
extern "C"
{
#endif

#include <netinet/ip6.h>
#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "netif_index.h"
#include "npd_bitop.h"
#include "man_mldsnp.h"
#include "sysdef/returncode.h"
#include "npd/ipv6.h"

extern DBusConnection *dcli_dbus_connection;

int man_mld_snp_set_status(char status)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
    char isEnable = status;
    query = dbus_message_new_method_call(
										MLD_DBUS_BUSNAME,   \
										MLD_DBUS_OBJPATH,    \
										MLD_DBUS_INTERFACE,   \
										MLD_SNP_DBUS_METHOD_SET_ENABLE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
	    if (dbus_error_is_set(&err)) 
		{
		    printf("%s raised: %s",err.name,err.message);
		    dbus_error_free(&err);
	    }
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args (reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
	    printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	} 
	
	dbus_message_unref(reply);
	return ret;
}

int man_mld_snp_set_cfg(unsigned char type, unsigned int value)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
    query = dbus_message_new_method_call(
										MLD_DBUS_BUSNAME,   \
										MLD_DBUS_OBJPATH,    \
										MLD_DBUS_INTERFACE,   \
										MLD_SNP_DBUS_METHOD_SET_CFG);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
	    if (dbus_error_is_set(&err)) 
		{
		    printf("%s raised: %s",err.name,err.message);
		    dbus_error_free(&err);
	    }
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args (reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
	    printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	} 
	
	dbus_message_unref(reply);
	return ret;
}


int man_mld_snp_set_vlan_enable
(
	unsigned short vlanId,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = vlanId;
	unsigned int isEnable = enable;
	int op_ret =0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_MLDSNP_OBJPATH,		\
								NPD_DBUS_MLDSNP_INTERFACE,		\
								NPD_DBUS_MLDSNP_METHOD_SET_VLAN_ENABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_mld_snp_set_vlan_querier(unsigned short vid, struct in6_addr *queryAddr)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_SET_VLAN_QUERIER);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&queryAddr->s6_addr32[0],
							 DBUS_TYPE_UINT32,&queryAddr->s6_addr32[1],
							 DBUS_TYPE_UINT32,&queryAddr->s6_addr32[2],
							 DBUS_TYPE_UINT32,&queryAddr->s6_addr32[3],
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_mld_snp_set_vlan_report(unsigned short vid, struct in6_addr *report)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_SET_VLAN_REPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&report->s6_addr32[0],
							 DBUS_TYPE_UINT32,&report->s6_addr32[1],
							 DBUS_TYPE_UINT32,&report->s6_addr32[2],
							 DBUS_TYPE_UINT32,&report->s6_addr32[3],
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_mld_snp_set_static_group
(
	unsigned char isAdd,
	unsigned short vid,
	unsigned int netif_index,
	struct in6_addr *group,
	struct in6_addr *src
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_SET_STATIC_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isAdd,	
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32, &netif_index,
							 DBUS_TYPE_UINT32,&group->s6_addr32[0],
							 DBUS_TYPE_UINT32,&group->s6_addr32[1],
							 DBUS_TYPE_UINT32,&group->s6_addr32[2],
							 DBUS_TYPE_UINT32,&group->s6_addr32[3],
							 DBUS_TYPE_UINT32,&src->s6_addr32[0],
							 DBUS_TYPE_UINT32,&src->s6_addr32[1],
							 DBUS_TYPE_UINT32,&src->s6_addr32[2],
							 DBUS_TYPE_UINT32,&src->s6_addr32[3],
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_mld_snp_set_static_rtport
(
	unsigned char isAdd,
	unsigned short vid,
	unsigned int netif_index
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_SET_STATIC_RTPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isAdd,	
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32, &netif_index,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int man_mld_snp_clear_rtport(unsigned short vlanId, unsigned int netif_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_CLR_RTPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_UINT32,&netif_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}	

int man_mld_snp_clear_group
(
	unsigned short vlanId,
	struct prefix_ip6_addr *grpAddr, 
	struct in6_addr *srcAddr
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret =0;

	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_CLR_MCGROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_UINT32,&grpAddr->prefix.u6_addr32[0],
							 DBUS_TYPE_UINT32,&grpAddr->prefix.u6_addr32[0],
							 DBUS_TYPE_UINT32,&grpAddr->prefix.u6_addr32[0],
							 DBUS_TYPE_UINT32,&grpAddr->prefix.u6_addr32[0],
							 DBUS_TYPE_BYTE,&grpAddr->prefixlen,
							 DBUS_TYPE_UINT32,&srcAddr->s6_addr32[0],
							 DBUS_TYPE_UINT32,&srcAddr->s6_addr32[1],
							 DBUS_TYPE_UINT32,&srcAddr->s6_addr32[2],
							 DBUS_TYPE_UINT32,&srcAddr->s6_addr32[3],
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}


	
int man_mld_snp_get_cfg
(
	struct man_mldsnp_info *mldsnp_cfg
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int	ret = 0;
	int i = 0;

	dbus_error_init(&err);	

		
	query = dbus_message_new_method_call(\
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_GET_CFG);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);		
	
	if(0 == ret)
	{	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_enable));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_robust_val));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_resp_int));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_host_aging_time));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_router_aging_time));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_last_member_int));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_last_member_count));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_fastleave_enable));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_cfg->mldsnp_max_join_group));	
		
	}
	
	dbus_message_unref(reply);
	return ret;
}

int man_mld_snp_get_vlan_cfg(
	struct man_mldsnp_vlan_info *mldsnp_vlan_info,
	unsigned short *nextvid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int	ret = 0;
	int i = 0;

	dbus_error_init(&err);	

		
	query = dbus_message_new_method_call(\
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_GET_VLAN_CFG);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &mldsnp_vlan_info->vid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);		
	
	if(0 == ret)
	{	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->vid));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->mldsnp_enable));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->mldCnt));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->rtpCnt));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->queryAddr.s6_addr32[0]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->queryAddr.s6_addr32[1]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->queryAddr.s6_addr32[2]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->queryAddr.s6_addr32[3]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->report.s6_addr32[0])); 

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->report.s6_addr32[1])); 

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->report.s6_addr32[2])); 

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(mldsnp_vlan_info->report.s6_addr32[3])); 

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,nextvid);	
	}
	
	dbus_message_unref(reply);
	return ret;
}


int man_mld_snp_get_vlan_routeport
(
	unsigned short	vlanId,
	struct man_mldsnp_rtport_info *portArray
)
{
	DBusMessage	*query;
	DBusMessage	*reply;
	DBusError	err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int ret = 0;
	unsigned int i = 0;
	unsigned int count = 0;

	/* show the user profile route-port */
	query = dbus_message_new_method_call(
								MLD_DBUS_BUSNAME,    \
								MLD_DBUS_OBJPATH,    \
								MLD_DBUS_INTERFACE,  \
								MLD_SNP_DBUS_METHOD_GET_VLAN_RTPORT);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (0 == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		for (i = 0; i < count; i++)
		{
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(portArray[i].ifindex));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(portArray[i].version));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &(portArray[i].flags));
            
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}

int man_mld_snp_get_mcgroup_next
(
    struct man_mldsnp_group_info *groupInfo
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int 	ret = 0;
	int i = 0;

	dbus_error_init(&err);	

		
	query = dbus_message_new_method_call(\
								MLD_DBUS_BUSNAME,	\
								MLD_DBUS_OBJPATH ,	\
								MLD_DBUS_INTERFACE ,	\
								MLD_SNP_DBUS_METHOD_GET_VLAN_MCGROUP);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&(groupInfo->vlanId),
							 DBUS_TYPE_UINT32,&(groupInfo->srcaddr.s6_addr32[0]),
							 DBUS_TYPE_UINT32,&(groupInfo->srcaddr.s6_addr32[1]),
							 DBUS_TYPE_UINT32,&(groupInfo->srcaddr.s6_addr32[2]),
							 DBUS_TYPE_UINT32,&(groupInfo->srcaddr.s6_addr32[3]),
							 DBUS_TYPE_UINT32,&(groupInfo->grpaddr.s6_addr32[0]),
							 DBUS_TYPE_UINT32,&(groupInfo->grpaddr.s6_addr32[1]),
							 DBUS_TYPE_UINT32,&(groupInfo->grpaddr.s6_addr32[2]),
							 DBUS_TYPE_UINT32,&(groupInfo->grpaddr.s6_addr32[3]),
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);		
	
	if(0 == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->srcaddr.s6_addr32[0]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->srcaddr.s6_addr32[1]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->srcaddr.s6_addr32[2]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->srcaddr.s6_addr32[3]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->grpaddr.s6_addr32[0]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->grpaddr.s6_addr32[1]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->grpaddr.s6_addr32[2]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->grpaddr.s6_addr32[3]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->version));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(groupInfo->flags));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(groupInfo->portnum));

		for(i = 0; i < groupInfo->portnum; i++)
	    {		        
			dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter, &(groupInfo->ifindex[i]));
			dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter, &(groupInfo->ifmode[i]));
	    }
	}
	
	dbus_message_unref(reply);
	return ret;
}

#ifdef __cplusplus
}
#endif
#endif

