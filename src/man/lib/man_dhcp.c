
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_dhcp.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		11/26/2009
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef HAVE_DHCP
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include "npd_database.h"
#include "man_dhcp.h"
#include "man_dhcp_relay.h"
#include "man_intf.h"

extern DBusConnection *config_dbus_connection;

int dcli_dhcp_port2name(int portValue, char *portName)
{
	if( portName == NULL)
		return -1;
	
	switch(portValue)
	{
		case 67:
			sprintf(portName,"dhcp");
			break;
		default:
			sprintf(portName,"other");
			break;
	}

	return 0;
}

int man_dhcp_option_43_check(int argc, char* argv[])
{
    int ni = 0;
    int length = 0;
    
    if (0 == strncmp(argv[0], "ascii", strlen(argv[0])))
    {
        if (NULL != argv[1])
        {
            length = strlen(argv[1]);
            if (length > 64)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return -1;
        }
    }
    else if (0 == strncmp(argv[0], "hex", strlen(argv[0])))
    {
        if (NULL != argv[1])
        {
            length = strlen(argv[1]);
            if (length > 64)
            {
                return -1;
            }
            else
            {
                for (ni = 0; ni < length; ni++)
                {
                    if (!isxdigit(argv[1][ni]))
                    {
                        return -1;
                    }
                }

                if (length & 0x00000001)
                {
                    return -1;
                }
                
                return 0;
            }
        }
        else
        {
            return -1;
        }
    }
    else    /* ip-address don't check. */
    {
        return 0;
    }

    return 0;
}

int man_dhcp_option_code_format_check(int argc, char* argv[])
{
    int code = 0;

    code = (int)strtoul(argv[0], 0, 10);
    switch (code)
    {
        case 43:
        {
            return man_dhcp_option_43_check(argc - 1, &argv[1]);
        }
        default :
        {
            return -1;
        }
    }

    return 0;
}

int man_dhcp_illegal_mask_check(unsigned int mask)
{
    int ni = 0;
    static unsigned int legal_mask[32] =
        {
            0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
            0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
            0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
            0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
            0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
            0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
            0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
            0xf0000000, 0xe0000000, 0xc0000000, 0x80000000
        };

    for (ni = 0; ni < 32; ni++)
    {
        if (ntohl(mask) == legal_mask[ni])
        {
            return 0;
        }
    }

    return -1;
}

int man_dhcp_illegal_cidr_check(unsigned int low, unsigned int high, unsigned int mask)
{
    if (((low & (~mask)) == (~mask))
        || ((high & (~mask)) == (~mask)))
    {
        return -1;
    }

    if (((low & (~mask)) == 0)
        || ((high & (~mask)) == 0))
    {
        return -1;
    }

    return 0;
}

int man_dhcp_netbios_type_interpret_to_code(char* str, unsigned int* code)
{
    if (strlen(str) != strlen("x-node"))
    {
        *code = 0;
        return 0;
    }

    if (0 == strcmp(str, "b-node"))
    {
        *code = 0x1;
        return 0;
    }
    else if (0 == strcmp(str, "h-node"))
    {
        *code = 0x8;
        return 0;
    }
    else if (0 == strcmp(str, "m-node"))
    {
        *code = 0x4;
        return 0;
    }
    else if (0 == strcmp(str, "p-node"))
    {
        *code = 0x2;
        return 0;
    }
    else
    {
        *code = 0;
    }

    return 0;
}

int man_dhcp_service_mutex(unsigned char* interface_name)
{
#ifdef HAVE_DHCP_RELAY
    unsigned int count = 0;
    struct man_dhcp_relay_helper_info_s helper_info;

    memset(&helper_info, 0, sizeof(struct man_dhcp_relay_helper_info_s));


    strncpy(helper_info.ifName, interface_name, sizeof(helper_info.ifName));
    if (0 == show_dhcpr_helper(NULL, &helper_info, &count))
    {
        return helper_info.enDis;
    }
#endif

    return 0;
}

unsigned int man_dhcp_server_show_lease_spec_info_count
(
	unsigned char mode,
	struct lease_query_data* quary,
	unsigned int *count
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	unsigned int ret = 0;
    
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_LEASE_SPEC_INFO_COUNT);
    dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &mode,	
							 DBUS_TYPE_UINT32, &quary->ipmin,
							 DBUS_TYPE_UINT32, &quary->ipmax,
							 DBUS_TYPE_UINT32, &quary->ipmask,
							 DBUS_TYPE_BYTE, &quary->hostMac[0],
							 DBUS_TYPE_BYTE, &quary->hostMac[1],
							 DBUS_TYPE_BYTE, &quary->hostMac[2],
							 DBUS_TYPE_BYTE, &quary->hostMac[3],
							 DBUS_TYPE_BYTE, &quary->hostMac[4],
							 DBUS_TYPE_BYTE, &quary->hostMac[5],
							 DBUS_TYPE_UINT32, &quary->type,
							 DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);	
    
	dbus_message_iter_get_basic(&iter, &ret);
    dbus_message_iter_next(&iter);
    
	dbus_message_iter_get_basic(&iter, count);
	dbus_message_iter_next(&iter);

    return ret;
}

unsigned int dcli_dhcp_server_show_lease_spec_info
(
	unsigned char mode,
	struct lease_query_data* quary,
	struct dcli_dhcpd_lease* lease
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	unsigned int ret = 0,j=0;
    int ni = 0;
    unsigned char* p_mac = lease->hostMac;
    unsigned char* p_uid = lease->client_id;
    
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_LEASE_SPEC_INFO);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &mode,	
							 DBUS_TYPE_UINT32, &quary->ipmin,
							 DBUS_TYPE_UINT32, &quary->ipmax,
							 DBUS_TYPE_UINT32, &quary->ipmask,
							 DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &p_mac, 6,
							 DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &p_uid, DHCPD_NAME_LEN,
							 DBUS_TYPE_UINT32, &quary->type,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);	
	dbus_message_iter_get_basic(&iter,&ret);
	
	if (DHCP_SERVER_RETURN_CODE_SUCCESS == ret)
	{
		memset(lease, 0, sizeof(*lease));		
		for (j = 0; j< 6; j++) 
		{
			dbus_message_iter_next(&iter);					
			dbus_message_iter_get_basic(&iter,&(lease->hostMac[j]));
		}
		dbus_message_iter_next(&iter);	
        
		dbus_message_iter_get_basic(&iter,&lease->hostip);
		dbus_message_iter_next(&iter);	
        
		dbus_message_iter_get_basic(&iter,&lease->start);
		dbus_message_iter_next(&iter);
        
		dbus_message_iter_get_basic(&iter,&lease->end);
		dbus_message_iter_next(&iter);	

		dbus_message_iter_get_basic(&iter,&lease->type);
		dbus_message_iter_next(&iter);

   		dbus_message_iter_get_basic(&iter,&lease->uid_len);
		dbus_message_iter_next(&iter);

        for (ni = 0; ni < DHCPD_NAME_LEN; ni++)
        {
		    dbus_message_iter_get_basic(&iter, &lease->client_id[ni]);
            dbus_message_iter_next(&iter);
        }
	}
	
	dbus_message_unref(reply);
	
	return ret;
}

int dcli_dhcp_server_intf_config
(
	unsigned char *ifName,
	unsigned char isEnable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_INTF_SERVICE_ENABLE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return 1;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

int man_dhcp_server_fast_check
(
	unsigned char *ifName,
	unsigned int isEnable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_INTF_SIGNAL_SERVER);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return 1;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_leasetime_count
(
	unsigned int day,
	unsigned int hour,
	unsigned int minite,
	unsigned int *leasetime
)
{	
	unsigned int lease = 0;

    if (NULL == leasetime)
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }
	
	if (day > 365)
	{
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (hour > 23)
	{
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (minite> 59)
	{
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	lease = day*24*60*60 + hour*60*60 + minite*60;
	
	*leasetime = lease;
	
	if( lease == 0 )		
		*leasetime = ~0UL;
	
	return DHCP_SNP_RETURN_CODE_OK;			
}
unsigned int dcli_dhcp_leasetime_analysis
(
	unsigned int *day,
	unsigned int *hour,
	unsigned int *minite,
	unsigned int leasetime
)
{	
	*day = leasetime/(86400);
	*hour = (leasetime - (*day * 86400))/(3600);
	*minite = (leasetime - (*day * 86400) - (*hour * 3600))/(60);
	return DHCP_SNP_RETURN_CODE_OK;			
}

unsigned int dhcpd_service_status_set
(
	unsigned char endis
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVICE_ENABLE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	return ret;
}

unsigned int dhcpd_pool_create
(
	unsigned char *poolName
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_CREATE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");

		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}


unsigned int dhcpd_pool_delete
(
	unsigned char *poolName
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DELETE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dhcpd_pool_add_range
(
	unsigned char *poolName,
	unsigned int  netmask_addr,
	unsigned int  iprange_min_addr,
	unsigned int  iprange_max_addr
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_ADD_RANGE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &iprange_min_addr,
							DBUS_TYPE_UINT32, &iprange_max_addr,
							DBUS_TYPE_UINT32, &netmask_addr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_pool_del_range
(
	unsigned char *poolName,
	unsigned int  netmask_addr,
	unsigned int  iprange_min_addr,
	unsigned int  iprange_max_addr
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DEL_RANGE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &iprange_min_addr,
							DBUS_TYPE_UINT32, &iprange_max_addr,
							DBUS_TYPE_UINT32, &netmask_addr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}


unsigned int man_dhcpd_pool_add_forbidden_ip
(
	unsigned char *poolName,
	unsigned int  forbidden_ip_min_addr,
	unsigned int  forbidden_ip_max_addr,
	unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_ADD_FORBIDDEN);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &forbidden_ip_min_addr,
							DBUS_TYPE_UINT32, &forbidden_ip_max_addr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int man_dhcpd_pool_del_forbidden_ip
(
	unsigned char *poolName,
	unsigned int  forbidden_ip_min_addr,
	unsigned int  forbidden_ip_max_addr,
	unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DEL_FORBIDDEN);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &forbidden_ip_min_addr,
							DBUS_TYPE_UINT32, &forbidden_ip_max_addr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int man_dhcpd_pool_option43
(
    unsigned int is_add,
    unsigned char* poolName,
    char* option43,
    unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_OPTION_43_60);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &is_add,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_STRING, &option43,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int man_dhcpd_pool_option_code
(
    unsigned char* poolName,
    unsigned int code,
    char* type,
    char* value,
    unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_OPTION_CODE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &code,
							DBUS_TYPE_STRING, &type,
							DBUS_TYPE_STRING, &value,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int man_dhcpd_pool_no_option_code
(
    unsigned char* poolName,
    unsigned int code,
    unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_NO_OPTION_CODE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &code,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int man_dhcpd_pool_netbios_node_type
(
    unsigned char* poolName,
    unsigned int netbios_node_type,
    unsigned int* op_ret
)
{ 
	DBusMessage* query = NULL;
	DBusMessage* reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_NETBIOS_NODE_TYPE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &netbios_node_type,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, op_ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
           	dbus_message_unref(reply);
            return -1;
		}
	}

	dbus_message_unref(reply);
	
	return 0;
}

unsigned int dhcpd_pool_dns_config
(
	unsigned char *poolName,
	unsigned int  dns_addrA,
	unsigned int  dns_addrB,
	unsigned int  dns_addrC,
	unsigned int  endis 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DNS_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &dns_addrA,
							DBUS_TYPE_UINT32, &dns_addrB,
							DBUS_TYPE_UINT32, &dns_addrC,
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);	
	return ret;
}

unsigned int dhcpd_pool_wins_config
(
	unsigned char *poolName,
	unsigned int wins_addr,
	unsigned int endis 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_WINS_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &wins_addr,
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_pool_gateway_config
(
	unsigned char *poolName,
	unsigned int gateway_addr,
	unsigned int endis 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_GATEWAY_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &gateway_addr,
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_pool_lease_default_config
(
	unsigned char *poolName,
	unsigned int default_lease
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DEFAULT_LEASE_TIME);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &default_lease,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}


unsigned int dhcpd_pool_lease_max_config
(
	unsigned char *poolName,
	unsigned int max_lease
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_MAX_LEASE_TIME);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_UINT32, &max_lease,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_pool_domain_config
(
	unsigned char *poolName,
	unsigned char *domain ,
	unsigned int endis 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;


	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_POOL_DOMAIN_NAME);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_STRING, &domain,
							DBUS_TYPE_UINT32, &endis,
						DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dhcpd_pool_config_for_web
(
	struct dcli_dhcpd_pool *pool_cfg
)
{
	unsigned int ret = 0;
	unsigned int count = 0;
	struct dcli_dhcpd_pool pre_pool_cfg;

	
	memset(&pre_pool_cfg, 0, sizeof(struct dcli_dhcpd_pool));
	strncpy(pre_pool_cfg.poolName, pool_cfg->poolName, strlen(pool_cfg->poolName));
	
	ret = dhcpd_get_curr_pool_info(&pre_pool_cfg, &count);
	if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret )
	{
		ret = dhcpd_pool_create(pool_cfg->poolName);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}

	if(pool_cfg->iprange_min_addr != pre_pool_cfg.iprange_min_addr ||
		pool_cfg->iprange_max_addr != pre_pool_cfg.iprange_max_addr ||
		pool_cfg->netmask_addr != pre_pool_cfg.netmask_addr)
	{
		if(pre_pool_cfg.iprange_min_addr != 0 && pre_pool_cfg.iprange_max_addr != 0 )
		{
			ret = dhcpd_pool_del_range(pool_cfg->poolName,pre_pool_cfg.netmask_addr,pre_pool_cfg.iprange_min_addr,pre_pool_cfg.iprange_max_addr);
			if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
			{
				return ret ;
			}	
		}
		
		if(pool_cfg->iprange_min_addr != 0 && pool_cfg->iprange_max_addr != 0 )
		{
			ret = dhcpd_pool_add_range(pool_cfg->poolName,pool_cfg->netmask_addr,pool_cfg->iprange_min_addr,pool_cfg->iprange_max_addr);
			if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
			{
				return ret ;
			}	
		}
	}

	if(memcmp((char *)pool_cfg->dns_addr, (char *)pre_pool_cfg.dns_addr, sizeof(unsigned int)*DHCPD_DNS_NUM))
	{
		ret = dhcpd_pool_dns_config(pool_cfg->poolName,pool_cfg->dns_addr[0],pool_cfg->dns_addr[1],pool_cfg->dns_addr[2],TRUE);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}

	if(memcmp((char *)pool_cfg->wins_addr, (char *)pre_pool_cfg.wins_addr, sizeof(unsigned int)*DHCPD_WINS_NUM))
	{
		ret = dhcpd_pool_wins_config(pool_cfg->poolName,pool_cfg->wins_addr[0],TRUE);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}

	if(memcmp((char *)pool_cfg->gateway_addr, (char *)pre_pool_cfg.gateway_addr, sizeof(unsigned int)*DHCPD_GATEWAY_NUM))
	{
		ret = dhcpd_pool_gateway_config(pool_cfg->poolName,pool_cfg->gateway_addr[0],TRUE);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}

	if(memcmp(pool_cfg->domain_name,pre_pool_cfg.domain_name, DHCPD_NAME_LEN))
	{
		ret = dhcpd_pool_domain_config(pool_cfg->poolName,pool_cfg->domain_name,TRUE);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}

	
	if(pool_cfg->max_lease_time != pre_pool_cfg.max_lease_time )
	{
		ret = dhcpd_pool_lease_max_config(pool_cfg->poolName,pool_cfg->max_lease_time);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}	
	
	if(pool_cfg->default_lease_time != pre_pool_cfg.default_lease_time )
	{
		ret = dhcpd_pool_lease_default_config(pool_cfg->poolName,pool_cfg->default_lease_time);
		if(DHCP_SERVER_RETURN_CODE_SUCCESS != ret) 
		{
			return ret ;
		}
	}
	
	
	return ret ;

}

unsigned int dhcpd_service_domain_config
(
	unsigned int endis,
	unsigned char *domain 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_DOMAIN_NAME);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &domain,
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) 
		{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;
}

unsigned int dhcpd_service_dns_config
(
	unsigned int dns_addrA,
	unsigned int dns_addrB,
	unsigned int dns_addrC,
	unsigned int enDis
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_DNS_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &dns_addrA,
							DBUS_TYPE_UINT32, &dns_addrB,
							DBUS_TYPE_UINT32, &dns_addrC,
							DBUS_TYPE_UINT32, &enDis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int  dhcpd_service_wins_config
(
	unsigned int *wins_addr,
	unsigned int endis 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_WINS_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &wins_addr[0],
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
			printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dhcpd_service_gateway_config
(
	unsigned int *gateway_addr,
	unsigned int endis
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_GATEWAY_ADDR);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &gateway_addr[0],
							DBUS_TYPE_UINT32, &endis,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)){
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;

}

unsigned int dhcpd_service_default_lease_config
(
	unsigned int default_lease
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret =0;
	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_DEFAULT_LEASE_TIME);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &default_lease,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;

}

unsigned int dhcpd_service_max_lease_config
(
	unsigned int max_lease 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SERVER_MAX_LEASE_TIME);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &max_lease,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpd_server_one_parameter(unsigned int parameter, const char* dbus_method)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
    
	query = dbus_message_new_method_call(DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										dbus_method);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &parameter,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpd_service_ping_check(unsigned int ping_check)
{
    return man_dhcpd_server_one_parameter(ping_check, DHCPD_DBUS_INTERFACE_METHOD_SERVER_PING_CHECK);
}

unsigned int dhcpd_service_show
(
	struct dcli_dhcpd_service_cfg *serviceCfg
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char *domainName = NULL;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_SERVICE_INFO);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
		
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	

	if( DHCP_SERVER_RETURN_CODE_SUCCESS == ret )
	{
		dbus_message_iter_get_basic(&iter,&serviceCfg->enDis);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->default_lease_time);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->max_lease_time);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&domainName);
		dbus_message_iter_next(&iter);	
		memcpy(serviceCfg->domain_name,domainName,sizeof(serviceCfg->domain_name));

		dbus_message_iter_get_basic(&iter,&serviceCfg->dns_addr[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->dns_addr[1]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->dns_addr[2]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->gateway_addr[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&serviceCfg->wins_addr[0]);
		dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&serviceCfg->ping_check);
		dbus_message_iter_next(&iter);	
	}
	else
	{
		printf( "Failed get args.\n");
	}
	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_pool_show
(
	struct dcli_dhcpd_pool *poolshow,
	char* next_pool_name,
	unsigned int *count
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
    DBusMessageIter	 iter_array;
    int ni = 0;
	unsigned int ret = 0;
    unsigned int* next_pool_name_ptr = NULL;
	unsigned char *poolName = NULL;
	unsigned char *domainName = NULL;
    unsigned char* p_buffer = NULL;

	poolName = poolshow->poolName;
    
	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_POOL_INFO);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
    dbus_message_iter_next(&iter);
   	dbus_message_iter_get_basic(&iter, count);

    if (ret == DHCP_SERVER_RETURN_CODE_SUCCESS)
    {
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &next_pool_name_ptr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolName);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->iprange_min_addr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->iprange_max_addr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->netmask_addr);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &domainName);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->default_lease_time);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->max_lease_time);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->dns_addr[0]);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->dns_addr[1]);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->dns_addr[2]);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->gateway_addr[0]);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->wins_addr[0]);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->netbios_node_type);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->forbidden_ip_count);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->option43_count);

        if (poolshow->forbidden_ip_count > 0)
        {
            DBusMessageIter iter_struct;
            dbus_message_iter_next(&iter);
            dbus_message_iter_recurse(&iter, &iter_array);
            
            for (ni = 0; ni < poolshow->forbidden_ip_count; ni++)
            {
                dbus_message_iter_recurse(&iter_array, &iter_struct);

                dbus_message_iter_get_basic(&iter_struct, &poolshow->forbidden_ip_min[ni]);
        		dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct, &poolshow->forbidden_ip_max[ni]);
        		dbus_message_iter_next(&iter_struct);
                
                dbus_message_iter_next(&iter_array);
            }
        }

        if (poolshow->option43_count > 0)
        {
            for (ni = 0; ni < poolshow->option43_count; ni++)
            {
                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &poolshow->option43[ni]);
            }
        }

        strncpy(next_pool_name, next_pool_name_ptr, DHCPD_NAME_LEN - 1);
        strncpy(poolshow->poolName, poolName, DHCPD_NAME_LEN-1);
        strncpy(poolshow->domain_name, domainName, DHCPD_NAME_LEN-1);

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->option_43_value_type);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &p_buffer);
        strncpy(poolshow->buffer_43, p_buffer, sizeof(poolshow->buffer_43));
        
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->tot_lease_cnt);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &poolshow->valid_lease_cnt);

    }
    dbus_message_unref(reply);
	
	return ret;
}


unsigned int dhcpd_get_curr_pool_info
(
	struct dcli_dhcpd_pool *poolshow,
	unsigned int *count
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret = 0;
	unsigned char *poolName = NULL;
	unsigned char *domainName = NULL;

	
	poolName = poolshow->poolName;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_GET_CURR_POOL_INFO);
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if( DHCP_SERVER_RETURN_CODE_SUCCESS == ret )
	{	
		memset(poolshow, 0, sizeof(*poolshow));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolName);
		memcpy(poolshow->poolName, poolName,strlen(poolName));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->iprange_min_addr);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->iprange_max_addr);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->netmask_addr);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&domainName);
		memcpy(poolshow->domain_name,domainName,strlen(domainName));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->default_lease_time);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->max_lease_time);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->dns_addr[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->dns_addr[1]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->dns_addr[2]);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->gateway_addr[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolshow->wins_addr[0]);
	}
	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_intf_show
(
	struct dcli_dhcpd_intf *intfcfg,
	char* next_ifname,
	unsigned int *count
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char *poolName= NULL;
	unsigned char *ifName = NULL;
	unsigned int ret = 0;
	unsigned int counttemp = 0;
	ifName = intfcfg->ifName;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_INTF_INFO);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_INVALID);
	
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
		
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&counttemp);
	dbus_message_iter_next(&iter);	
	*count = counttemp;
	if( DHCP_SERVER_RETURN_CODE_SUCCESS == ret )
	{
		memset(intfcfg, 0, sizeof(*intfcfg));
		dbus_message_iter_get_basic(&iter,&ifName);
		dbus_message_iter_next(&iter);
        if (NULL != next_ifname)
        {
		    strncpy(next_ifname, ifName, DHCPD_NAME_LEN-1);
        }
        dbus_message_iter_get_basic(&iter,&ifName);
		dbus_message_iter_next(&iter);
        strncpy(intfcfg->ifName, ifName, DHCPD_NAME_LEN-1);
		
		dbus_message_iter_get_basic(&iter,&(intfcfg->enDis));
		dbus_message_iter_next(&iter);	
		
		dbus_message_iter_get_basic(&iter,&intfcfg->ifIndex);
		dbus_message_iter_next(&iter);	
	
		dbus_message_iter_get_basic(&iter,&poolName);
		dbus_message_iter_next(&iter);	
		strncpy(intfcfg->bindpoolname, poolName,DHCPD_NAME_LEN-1);

        dbus_message_iter_get_basic(&iter,&intfcfg->have_more_server);
		dbus_message_iter_next(&iter);
	}
	
	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_get_curr_intf_info
(
	struct dcli_dhcpd_intf *intfcfg
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char *poolName= NULL;
	unsigned char *ifName = NULL;
	unsigned int ret = 0;

	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_GET_CURR_INTF_INFO);
	
	ifName = intfcfg->ifName;
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_INVALID);
	
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
		
	dbus_message_iter_get_basic(&iter,&ret);
	if( DHCP_SERVER_RETURN_CODE_SUCCESS == ret )
	{
		memset(intfcfg, 0, sizeof(*intfcfg));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifName);
		strncpy(intfcfg->ifName,ifName,DHCPD_NAME_LEN-1);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(intfcfg->enDis));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&intfcfg->ifIndex);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&poolName);
		strncpy(intfcfg->bindpoolname, poolName,DHCPD_NAME_LEN-1);

	}
	
	dbus_message_unref(reply);
	
	return ret;
}


unsigned int dhcpd_service_static_add
(
	ETHERADDR macAddr,
	unsigned int ipaddr 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_HOST_STATIC_ADD);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_BYTE,&macAddr.arEther[0],
							DBUS_TYPE_BYTE,&macAddr.arEther[1],
							DBUS_TYPE_BYTE,&macAddr.arEther[2],
							DBUS_TYPE_BYTE,&macAddr.arEther[3],
							DBUS_TYPE_BYTE,&macAddr.arEther[4],
							DBUS_TYPE_BYTE,&macAddr.arEther[5],
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_service_static_del
(
	ETHERADDR macAddr,
	unsigned int ipaddr 
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_HOST_STATIC_DEL);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_BYTE,&macAddr.arEther[0],
							DBUS_TYPE_BYTE,&macAddr.arEther[1],
							DBUS_TYPE_BYTE,&macAddr.arEther[2],
							DBUS_TYPE_BYTE,&macAddr.arEther[3],
							DBUS_TYPE_BYTE,&macAddr.arEther[4],
							DBUS_TYPE_BYTE,&macAddr.arEther[5],
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dhcpd_host_show
(
	struct dcli_dhcpd_host *hostcfg,
	unsigned int *count
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	int  j = 0; 
	unsigned char *hostName;
	unsigned int hostip;
	ETHERADDR hostmac;
	unsigned int ret = 0;
	unsigned int counttemp = 0;
	memcpy(hostmac.arEther,hostcfg->hardware.hbuf+1,6);
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_HOST_INFO);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&hostmac.arEther[0],
							DBUS_TYPE_BYTE,&hostmac.arEther[1],
							DBUS_TYPE_BYTE,&hostmac.arEther[2],
							DBUS_TYPE_BYTE,&hostmac.arEther[3],
							DBUS_TYPE_BYTE,&hostmac.arEther[4],
							DBUS_TYPE_BYTE,&hostmac.arEther[5],
 							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
		
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&counttemp);
	dbus_message_iter_next(&iter);					
	*count = counttemp;
	if( DHCP_SERVER_RETURN_CODE_SUCCESS == ret )
	{
		for(j = 1; j< 7; j++) 
		{
			dbus_message_iter_get_basic(&iter,&(hostcfg->hardware.hbuf[j]));
			dbus_message_iter_next(&iter);
		}
		dbus_message_iter_get_basic(&iter,&hostip);
		dbus_message_iter_next(&iter);	
		memcpy(hostcfg->hostip.iabuf,&hostip,4);
		
		dbus_message_iter_get_basic(&iter,&hostName);
		dbus_message_iter_next(&iter);	
		memcpy(hostcfg->client_hostname,hostName,strlen(hostName));
	}	
	dbus_message_unref(reply);	
	return ret;
}

unsigned int dhcpd_pool_bind_config
(
	unsigned char *ifName ,
	unsigned char *poolName
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_INTF_BIND_POOL);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dhcpd_pool_unbind_config
(
	unsigned char *ifName ,
	unsigned char *poolName
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_INTF_UNBIND_POOL);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifName,
							DBUS_TYPE_STRING, &poolName,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpd_debug_flag_set
(
	unsigned int isSet,
	unsigned char *flagstr
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_DEBUG_FLAG_SET);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isSet,
							DBUS_TYPE_STRING, &flagstr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int man_dhcpd_show_debug_flag
(
	unsigned char *buf,
	unsigned int maxlen
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned char *showstr = NULL;
	query = dbus_message_new_method_call(
										DHCPD_DBUS_BUSNAME,
										DHCPD_DBUS_OBJPATH,
										DHCPD_DBUS_INTERFACE,
										DHCPD_DBUS_INTERFACE_METHOD_SHOW_DEBUG_FLAG);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SERVER_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_STRING, &showstr,
							DBUS_TYPE_INVALID)) {
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	strncpy(buf, showstr, maxlen);

	dbus_message_unref(reply);
	
	return ret;
}

#ifdef __cplusplus
}
#endif
#endif

