
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_IGMP_SNP
#ifdef __cplusplus
extern "C"
{
#endif

/*include header files begin */
/*kernel part */
#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <dbus/igmpsnp/igmp_snp_dbus.h>
#include "netif_index.h"
#include "npd_bitop.h"
#include "man_igmpsnp.h"
#include "sysdef/returncode.h"


/*include header files begin */

/*MACRO definition begin */
/*MACRO definition end */

/*local variables definition begin */
extern DBusConnection *dcli_dbus_connection;

 int dcli_igmp_snp_fastleave_check_status(unsigned char* stats)
{
    DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = 0;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,			\
								IGMP_DBUS_OBJPATH,		\
								IGMP_DBUS_INTERFACE,		\
								IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_FASTLEAVE_STATUS);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*stats = status;
		}
	} 
	else {
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

/* 	
 *  by slot and local_port get global port index
 */
int dcli_igmp_snp_check_status(unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = 0;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,			\
								IGMP_DBUS_OBJPATH,		\
								IGMP_DBUS_INTERFACE,		\
								IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*stats = status;
		}
	} 
	else 
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

int dcli_igmp_snp_port_filter_check_status(unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status;
	int op_ret;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,			\
								IGMP_DBUS_OBJPATH,		\
								IGMP_DBUS_INTERFACE,		\
								IGMP_SNP_DBUS_METHOD_CHECK_IGMP_SNP_PORT_FILTER);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*stats = status;
		}
	} 
	else 
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

int dcli_igmp_snp_check_max_join_group(unsigned int *groupcount)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int groupCnt;
	int op_ret;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,			\
								IGMP_DBUS_OBJPATH,		\
								IGMP_DBUS_INTERFACE,		\
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_CHECK_MAX_JOIN_GROUP);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&groupCnt,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*groupcount = groupCnt;
		}
	} 
	else 
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
#if 0
int dcli_igmp_snp_querier_source_get(unsigned long *querier_source)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned long querier_ip;
	int op_ret;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,			\
								IGMP_DBUS_OBJPATH,		\
								IGMP_DBUS_INTERFACE,		\
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_GET_QUERIER_SOURCE);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&querier_ip,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*querier_source = querier_ip;
		}
	} 
	else 
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
#endif
int dcli_vlan_igmp_snp_status_get(unsigned short vid,unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = IGMPSNP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_IGMPSNP_OBJPATH,		\
								NPD_DBUS_IGMPSNP_INTERFACE,	\
								NPD_DBUS_IGMPSNP_METHOD_CHECK_VLAN_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			*stats = status;
		}
	} 
	else {
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

int dcli_enable_disable_igmp_one_vlan
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
								NPD_DBUS_IGMPSNP_OBJPATH,		\
								NPD_DBUS_IGMPSNP_INTERFACE,		\
								NPD_DBUS_IGMPSNP_METHOD_CONFIG_IGMP_SNP_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,180*1000, &err);
	
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
/*
 *  by slot and local_port get global port index
 */
int dcli_enable_disable_igmp_one_port
(
	unsigned short vlanId,
 	unsigned int netif_index,
	unsigned char enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char isEnable = enable;
	int op_ret;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_IGMPSNP_OBJPATH,	\
								NPD_DBUS_IGMPSNP_INTERFACE,	\
								NPD_DBUS_IGMPSNP_METHOD_CONFIG_ETHPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_UINT32,&netif_index,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		op_ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_igmp_snp_max_join_group(unsigned int groupcount)
{
	DBusMessage *query, *reply; 
	DBusError err;
	int ret;
	unsigned int GrpCount = groupcount;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,		\
										IGMP_DBUS_OBJPATH,		\
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_CONFIG_MAX_JOIN_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query, 
							DBUS_TYPE_UINT32,&GrpCount,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,				
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
	    ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
	
}

int dcli_igmp_snp_port_max_join_group(unsigned int netif_index, unsigned int groupcount)
{
	DBusMessage *query, *reply; 
	DBusError err;
	int ret;
	unsigned int GrpCount = groupcount;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,		\
										IGMP_DBUS_OBJPATH,		\
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_CONFIG_PORT_MAX_JOIN_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query, 
							DBUS_TYPE_UINT32,&GrpCount,
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,				
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
	    ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
	
}


int dcli_enable_disable_igmp_mcrouter_port
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned char enDis
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = vlanId;
	unsigned char isEnable = enDis;
	int ret = 0;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,			\
										IGMP_DBUS_OBJPATH,		\
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_UINT32,&eth_g_index,
							DBUS_TYPE_BYTE,	&isEnable,
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

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,				
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
	    ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
	
}

int dcli_igmp_snp_mcgroup_get
(
    int first,
    IGMP_SNP_GRP_STC *groupInfo
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned int 	ret = IGMPSNP_RETURN_CODE_OK;
    npd_pbmp_t    mbrBmp;
	int i = 0;

	memset(&mbrBmp,0,sizeof(npd_pbmp_t));	

	dbus_error_init(&err);	

		
	query = dbus_message_new_method_call(\
								IGMP_DBUS_BUSNAME,	\
								IGMP_DBUS_OBJPATH ,	\
								IGMP_DBUS_INTERFACE ,	\
								IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_SPEC_PORT_MEMBERS );

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&first,     
							 DBUS_TYPE_UINT16,&(groupInfo->vid),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_sadd),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_ipadd),
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
	
	if(IGMPSNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->mgroup_id));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_sadd));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_ipadd));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->state));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(groupInfo->ver_flag));
        
		for(i = 0; i < sizeof(npd_pbmp_t)/4; i++)
	    {		        
			dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter, (unsigned int *)&mbrBmp+i);
	    }
		
		groupInfo->portmbr = mbrBmp;
	}
	
	dbus_message_unref(reply);
	return ret;
}


int dcli_igmp_snp_mcgroup_get_next
(
    IGMP_SNP_GRP_STC *groupInfo
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned int 	ret = IGMPSNP_RETURN_CODE_OK;
    npd_pbmp_t    mbrBmp;
	int i = 0;

	memset(&mbrBmp,0,sizeof(npd_pbmp_t));	

	dbus_error_init(&err);	

		
	query = dbus_message_new_method_call(\
								IGMP_DBUS_BUSNAME,	\
								IGMP_DBUS_OBJPATH ,	\
								IGMP_DBUS_INTERFACE ,	\
								IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST_PORT_MEMBERS );

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&(groupInfo->vid),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_sadd),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_ipadd),
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
	
	if(IGMPSNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->mgroup_id));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_sadd));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_ipadd));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->state));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(groupInfo->ver_flag));

		for(i = 0; i < sizeof(npd_pbmp_t)/4; i++)
	    {		        
			dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter, (unsigned int *)&mbrBmp+i);
	    }
		
		groupInfo->portmbr = mbrBmp;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_cfg_port_get_next(IGMP_SNP_PORT_CFG_STC * igmpsnpCfgPort)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned int 	ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int ifindex = igmpsnpCfgPort->ifindex;
	unsigned int max_join_group = 0;

	dbus_error_init(&err);	

	query = dbus_message_new_method_call(\
								IGMP_DBUS_BUSNAME,	\
								IGMP_DBUS_OBJPATH ,	\
								IGMP_DBUS_INTERFACE ,	\
								IGMP_SNP_DBUS_METHOD_GET_NEXT_CONFIG_PORT);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ifindex,
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
	
	if(IGMPSNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifindex);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&max_join_group);	

#ifdef HAVE_MVLAN
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(igmpsnpCfgPort->mvlanId));	
#endif
		igmpsnpCfgPort->ifindex = ifindex;
		igmpsnpCfgPort->max_join_group = max_join_group;
	}
	
	dbus_message_unref(reply);
	return ret;
}

/*************************************************************************** 
 * function : Show all layer 2 multicast group's count in the vlan
 *     name : dcli_show_igmp_snp_mcgroup_count
 *     input :
 *			struct vty       *vty
 *			unsigned short vlanId
 *   output :
 *			null
 *  re_value: void
 ***************************************************************************/
int dcli_show_igmp_snp_mcgroup_count
(
	unsigned short vlanId,
	unsigned int *group_Cont
)
{
	DBusMessage     *query, *reply;
	DBusError        err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int	 count = 0;
	unsigned int 	 ret = 0;

	query = dbus_message_new_method_call(\
						IGMP_DBUS_BUSNAME,	\
						IGMP_DBUS_OBJPATH ,	\
						IGMP_DBUS_INTERFACE ,	\
						IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST_COUNT );

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &count);
		*group_Cont = count;
	}
	else 
	{
		*group_Cont = 0;
	}

	dbus_message_unref(reply);
	return ret;
}

/*************************************************************************** 
 * function : show the user profile route-port in the vlan
 *	name : igmp_snp_show_running_routeport
 *	input :
 *		struct vty 	 *vty
 *		unsigned short vlanId		-lan id
 *
 *	output :
 *		PORT_MEMBER_BMP *routeBmp	-route Bmp
 *
 *  re_value: ret
 *			CMD_WARNING	-dbus err
 *			CMD_SUCCESS	-success
 ***************************************************************************/
int dcli_igmp_snp_show_running_routeport
(
	unsigned short	vlanId,
	IGMP_SNP_PORT_STC *portArray
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
								IGMP_DBUS_BUSNAME,    \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,  \
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT);

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

	if (IGMPSNP_RETURN_CODE_OK == ret)
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
			dbus_message_iter_get_basic(&iter_struct, &(portArray[i].state));
			
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}
int dcli_igmp_snp_set_status(char status)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
    char isEnable = status;
    query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE);
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
	if (!dbus_message_get_args ( reply, &err,
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

int	dcli_igmp_snp_fasleave_set(unsigned char status)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	char isEnable = status; 

    query = dbus_message_new_method_call(
							IGMP_DBUS_BUSNAME,  \
							IGMP_DBUS_OBJPATH,   \
							IGMP_DBUS_INTERFACE,  \
							IGMP_SNP_DBUS_METHOD_IGMP_SNP_FASTLEAVE_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			    DBUS_TYPE_BYTE,&isEnable,
				DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    if (NULL == reply) 
	{
	    if (dbus_error_is_set(&err)) 
		{
		    printf("%s raised: %s",err.name,err.message);
	        dbus_error_free(&err);
        }
	     return DBUS_RETURN_CODE_NULL_REPLY;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		    DBUS_TYPE_UINT32,&ret,
		    DBUS_TYPE_INVALID)) 
    {
    	printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
	    }
		ret = DBUS_RETURN_CODE_PARM_ERROR;		
	} 

	dbus_message_unref(query);
	return ret;
}

int dcli_igmp_snp_vlan_query_saddr(unsigned short vid, unsigned long ipAddr, unsigned short flag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned short vlan_id = vid;
	unsigned long source_ip = ipAddr;
    unsigned short enable = flag;

    query = dbus_message_new_method_call(
							IGMP_DBUS_BUSNAME,  \
							IGMP_DBUS_OBJPATH,   \
							IGMP_DBUS_INTERFACE,  \
							IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_QURRY_SIP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
			    DBUS_TYPE_UINT16,&vlan_id,
				DBUS_TYPE_UINT32,&source_ip,
				DBUS_TYPE_UINT16,&enable,
				DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    if (NULL == reply) 
	{
	    if (dbus_error_is_set(&err)) 
		{
		    printf("%s raised: %s",err.name,err.message);
	        dbus_error_free(&err);
        }
	     return DBUS_RETURN_CODE_NULL_REPLY;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		    DBUS_TYPE_UINT32,&ret,
		    DBUS_TYPE_INVALID)) 
    {
    	printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
	    }
		ret = DBUS_RETURN_CODE_PARM_ERROR;		
	} 

	dbus_message_unref(query);
	return ret;
}
#if 0
int dcli_igmp_snp_global_query_saddr(unsigned long ipAddr)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned long source_ip = ipAddr;

    query = dbus_message_new_method_call(
							IGMP_DBUS_BUSNAME,  \
							IGMP_DBUS_OBJPATH,   \
							IGMP_DBUS_INTERFACE,  \
							IGMP_SNP_DBUS_METHOD_IGMP_SNP_GLOBAL_QURRY_SIP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
				DBUS_TYPE_UINT32,&source_ip,
				DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    if (NULL == reply) 
	{
	    if (dbus_error_is_set(&err)) 
		{
		    printf("%s raised: %s",err.name,err.message);
	        dbus_error_free(&err);
        }
	     return DBUS_RETURN_CODE_NULL_REPLY;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		    DBUS_TYPE_UINT32,&ret,
		    DBUS_TYPE_INVALID)) 
    {
    	printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
	    }
		ret = DBUS_RETURN_CODE_PARM_ERROR;		
	} 

	dbus_message_unref(query);
	return ret;
}
#endif
int dcli_igmp_snp_port_filter_set(unsigned char status)
{
	DBusMessage *query, *reply;
	DBusError error;
	int ret;
	unsigned char isEnable = status; 

	query = dbus_message_new_method_call(
							IGMP_DBUS_BUSNAME,
							IGMP_DBUS_OBJPATH, 
							IGMP_DBUS_INTERFACE,
							IGMP_SNP_DBUS_METHOD_IGMP_SNP_PORT_FILTER_ENABLE);

	dbus_error_init(&error);
	dbus_message_append_args(query,
			    DBUS_TYPE_BYTE,&isEnable,
				DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &error);
    if (NULL == reply) 
	{
	    if (dbus_error_is_set(&error)) 
		{
		    printf("%s raised: %s",error.name,error.message);
	        dbus_error_free(&error);
        }
	     return DBUS_RETURN_CODE_NULL_REPLY;
	}
	
	if (!dbus_message_get_args ( reply, &error,
		    DBUS_TYPE_UINT32,&ret,
		    DBUS_TYPE_INVALID)) 
    {
    	printf("Failed get args.\n");
		if (dbus_error_is_set(&error)) 
		{
			printf("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
	    }
		ret = DBUS_RETURN_CODE_PARM_ERROR;		
	} 

	dbus_message_unref(query);
	return ret;
	
}

int dcli_igmp_snp_static_group_del_port(unsigned short vid,int ipAddr, unsigned int netif_index)
{
    DBusMessage *query, *reply;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,	  \
										IGMP_DBUS_OBJPATH,	  \
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_STATIC_MCGROUP_DEL_PORT);
	dbus_error_init(&err);

    dbus_message_append_args(query,
					DBUS_TYPE_UINT16,&vid,
					DBUS_TYPE_UINT32,&ipAddr,
					DBUS_TYPE_UINT32,&netif_index,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
	    }
		return DBUS_RETURN_CODE_NULL_REPLY;
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
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_static_group_add_port(unsigned short vid,unsigned long ipAddr,unsigned long SipAddr,unsigned int netif_index)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
							  IGMP_DBUS_BUSNAME,	  \
							  IGMP_DBUS_OBJPATH,	  \
							  IGMP_DBUS_INTERFACE,	\
							  IGMP_SNP_DBUS_METHOD_IGMP_SNP_STATIC_MCGROUP_ADD_PORT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_UINT16,&vid,
					DBUS_TYPE_UINT32,&ipAddr,
					DBUS_TYPE_UINT32,&SipAddr,
					DBUS_TYPE_UINT32,&netif_index,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,180*1000, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
			
	if (!dbus_message_get_args ( reply, &err,
			          DBUS_TYPE_UINT32,&ret,
					  DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_dbus_config_igmp_snp_timer(int timer_type,int timeout)
{
    
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,    \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,    \
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&timer_type,
						DBUS_TYPE_UINT32,&timeout,
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

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
	
}

int dcli_igmp_snp_dbus_mcgroup_del(unsigned short vid,unsigned long ipAddr)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
				              IGMP_DBUS_BUSNAME,	  \
				              IGMP_DBUS_OBJPATH,	  \
				              IGMP_DBUS_INTERFACE,	\
				              IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_UINT16,&vid,
					DBUS_TYPE_UINT32,&ipAddr,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return DBUS_RETURN_CODE_NULL_REPLY;
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
		dbus_message_unref(reply);
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_dbus_del_one_mcgroup_vlan(unsigned short vid)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
							IGMP_DBUS_BUSNAME,	  \
							IGMP_DBUS_OBJPATH,	  \
							IGMP_DBUS_INTERFACE,	\
							IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
					DBUS_TYPE_UINT16,&vid,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
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
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}	
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_dbus_mcgroup_vlan_del_all()
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,	  \
										IGMP_DBUS_OBJPATH,	  \
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
			
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_dbus_mcroute_port_del(unsigned short vid, unsigned int netif_index)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,	  \
										IGMP_DBUS_OBJPATH,	  \
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT16,&vid,
						DBUS_TYPE_UINT32,&netif_index,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
			
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}


int dcli_igmp_snp_dbus_mcroute_port_del_vlan(unsigned short vid)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,	  \
										IGMP_DBUS_OBJPATH,	  \
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT_VLAN);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT16,&vid,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
			
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_dbus_mcroute_port_del_all()
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,	  \
										IGMP_DBUS_OBJPATH,	  \
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCROUTE_PORT_ALL);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
			
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);
	return ret;
}


int dcli_igmp_snp_dbus_igmp_snp_debug(int flag, char off)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	
	query = dbus_message_new_method_call(
			                IGMP_DBUS_BUSNAME,    \
			                IGMP_DBUS_OBJPATH,    \
			                IGMP_DBUS_INTERFACE,  \
			                IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
							DBUS_TYPE_BYTE, &off,
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
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int dcli_igmp_snp_timeout_get(IGMP_SNP_CFG_STC *igmpCfgPtr)
{
    DBusMessage *query, *reply;
	DBusError err;
    int ret;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_vlan_lifetime),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_grp_lifetime),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_robust_val),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_query_int),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_resp_int),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_last_member_int),
								DBUS_TYPE_UINT32,&(igmpCfgPtr->igmpsnp_last_member_count),
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_get_group_total_count(int *groupcount)
{
    DBusMessage *query, *reply;
	DBusError err;
	unsigned int ret = 0;
	
    query = dbus_message_new_method_call(   
				                IGMP_DBUS_BUSNAME,    \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,   
								IGMP_SNP_DBUS_METHOD_SHOW_MCGROUP_TOTAL_COUNT);
						
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) 
	{
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,groupcount,
								DBUS_TYPE_INVALID)) 
	{
	    printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);	
            ret = DBUS_RETURN_CODE_PARM_ERROR;
		}
	} 

	dbus_message_unref(reply);   
	return ret;

}

int  dcli_npd_dbus_igmpsnp_vlan_count(int *vlan_Count)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_IGMPSNP_OBJPATH,
										NPD_DBUS_IGMPSNP_INTERFACE,
										NPD_DBUS_IGMPSNP_METHOD_IGMP_SNP_VLAN_COUNT);

	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if (!dbus_message_get_args( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_UINT32,vlan_Count,
							DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);	
            ret = DBUS_RETURN_CODE_PARM_ERROR;
		}
	}

	dbus_message_unref(reply);
	return ret ;
}

int dcli_igmp_snp_vlan_get( unsigned short *vid ,npd_pbmp_t  *tagBmp,npd_pbmp_t	*untagBmp)
{
    DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	int ret,i;
	unsigned short  vlanId = *vid;
	npd_pbmp_t  untagTrunkBmp, tagTrunkBmp;
    query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_IGMPSNP_OBJPATH,		\
								NPD_DBUS_IGMPSNP_INTERFACE, 	\
								NPD_DBUS_IGMPSNP_METHOD_IGMP_SNP_VLAN_LIST_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&vlanId);
	*vid = vlanId;

	if(IGMPSNP_RETURN_CODE_OK == ret){							
		memset(untagBmp,0,sizeof(npd_pbmp_t));
		memset(tagBmp,0,sizeof(npd_pbmp_t));
		memset(&untagTrunkBmp,0,sizeof(npd_pbmp_t));
		memset(&tagTrunkBmp,0,sizeof(npd_pbmp_t));

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,(unsigned int *)(tagBmp)+i);
			dbus_message_iter_next(&iter_array);
		}

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,(unsigned int *)(untagBmp)+i);
			dbus_message_iter_next(&iter_array);
		}		
	}

	dbus_message_unref(reply);
	return ret;
}

int dcli_igmp_snp_igmp_vlan_get( IGMP_SNP_VLAN_STC  *vlanInfo )
{
	int ipAddr;
	DBusMessage *query, *reply; 
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret,i;
	unsigned short vlan_id = vlanInfo->vid;
	unsigned int   querier_addr = 0;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,		\
										IGMP_DBUS_OBJPATH,		\
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_GET_VLAN);
	dbus_error_init(&err);

	dbus_message_append_args(query, 
							DBUS_TYPE_UINT16,&vlan_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&vlan_id);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&querier_addr);
#ifdef HAVE_MVLAN
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(vlanInfo->ismvlan));
#endif
	vlanInfo->vid = vlan_id;
	vlanInfo->saddr = querier_addr;
	if(ret == IGMPSNP_RETURN_CODE_OK)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,(unsigned int *)(&(vlanInfo->member))+i);
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	return ret;
}

#ifdef HAVE_MVLAN
int dcli_enable_disable_mvlan_on_vlan
(
	unsigned short vlanId,
	unsigned char enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = vlanId;
	int op_ret =0;

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,   \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,	\
								IGMP_SNP_DBUS_METHOD_CONFIG_MVLAN_ON_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_BYTE,&enable,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,180*1000, &err);
	
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

int dcli_mvlan_port_set_vlanid(unsigned int netif_index, unsigned short vlanid)
{
	DBusMessage *query, *reply; 
	DBusError err;
	int ret;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,		\
										IGMP_DBUS_OBJPATH,		\
										IGMP_DBUS_INTERFACE,	\
										IGMP_SNP_DBUS_METHOD_MVLAN_PORT_SET_VLANID);
	dbus_error_init(&err);

	dbus_message_append_args(query, 
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_UINT16,&vlanid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if(NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}
	if (!dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,				
								DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
	    ret = DBUS_RETURN_CODE_PARM_ERROR;
	}

	dbus_message_unref(reply);
	return ret;
	
}

#endif

int man_pim_snp_group_get_next
(
	unsigned char flag,
    IGMP_SNP_GRP_STC *groupInfo,
    unsigned int *count
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned int 	ret = IGMPSNP_RETURN_CODE_OK;
	npd_pbmp_t    mbrBmp;
	int i = 0;

	memset(&mbrBmp,0,sizeof(npd_pbmp_t));	

	dbus_error_init(&err);	
	query = dbus_message_new_method_call(\
								IGMP_DBUS_BUSNAME,	\
								IGMP_DBUS_OBJPATH ,	\
								IGMP_DBUS_INTERFACE ,	\
								PIM_SNP_DBUS_METHOD_SHOW_MCGROUP_LIST );

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag,
							 DBUS_TYPE_UINT16,&(groupInfo->vid),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_sadd),
							 DBUS_TYPE_UINT32,&(groupInfo->MC_ipadd),
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
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,count);	
	
	if(IGMPSNP_RETURN_CODE_OK == ret)
	{
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->vid));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_sadd));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->MC_ipadd));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(groupInfo->state));	

		for(i = 0; i < sizeof(npd_pbmp_t)/4; i++)
	    {		        
			dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter, (unsigned int *)&mbrBmp+i);
	    }
		
		groupInfo->portmbr = mbrBmp;
	}
	
	dbus_message_unref(reply);
	return ret;
}


#ifdef __cplusplus
}
#endif
#endif

