
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */

#ifdef HAVE_DHCP_SNP
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
//#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include "dbus/npd/npd_dbus_def.h"
#include "sysdef/returncode.h"
#include "man_dhcp_snp.h"
#include "man_intf.h"

extern DBusConnection *config_dbus_connection;

/**********************************************************************************
 * dcli_dhcp_snp_remoteid_string_legal_check
 * 
 *	INPUT:
 *		str	- remote-id string user entered from vtysh. 
 *		len	- Length of remote-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_remoteid_string_legal_check
(
	char *str,
	unsigned int len
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = 0;
	char c = 0;

	if ((NULL == str) || (len == 0)) {
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if(len >= DCLI_DHCP_SNP_REMOTEID_STR_LEN){
		return 1;
	}

	c = str[0];
	if(	(c == '_')||
		(c <= 'z' && c >= 'a') ||
		(c <= 'Z' && c >= 'A')
	  ){
		ret = DHCP_SNP_RETURN_CODE_OK;
	}else {
		return 2;
	}

	for (i = 1; i <= len - 1; i++) {
		c = str[i];
		if ((c >= '0' && c <= '9')||
			(c <= 'z' && c >= 'a')||
		    (c <= 'Z' && c >= 'A')||
		    (c == '_')
		    ){
			continue;
		}else {
			ret = 3;
			break;
		}
	}

	return ret;
}

unsigned int man_dhcp_ip_address_pair[3][2] =
{
    {0x01000001, 0x7efffffe},   /* XXX:A 1.0.0.1--126.255.255.254 */
    {0x80000001, 0xbffffffe},   /* XXX:B 128.0.0.1--191.255.255.254 */
    {0xc0000001, 0xdffffffe}   /* XXX:C 192.0.0.1--223.255.255.254 */
};

unsigned int man_dhcp_reserve_ip_address[3] = 
{
    0x00ffffff, 0x0000ffff, 0x000000ff
};

#define MAN_DHCP_IP_PAIR_NUM    (sizeof(man_dhcp_ip_address_pair) / (sizeof(unsigned int) * 2))

/*
 * return value non-zero means reserve ip or error
 * return value zero means legal ip
 */
int man_dhcp_reserve_ip(unsigned int ip_address)
{
    int ni = 0;
    int reserve_flag = 0;
    int host_byte_older_ip = 0;

    host_byte_older_ip = ntohl(ip_address);
    
    for (ni = 0; ni < MAN_DHCP_IP_PAIR_NUM; ni++)
    {
        if (host_byte_older_ip >= man_dhcp_ip_address_pair[ni][0]
            && host_byte_older_ip <= man_dhcp_ip_address_pair[ni][1])
        {
            reserve_flag = 0xff;
            break;
        }
    }

    if (reserve_flag)
    {
        if (((host_byte_older_ip & man_dhcp_reserve_ip_address[ni]) == 0)    /* zero-all host-ip */
            || ((host_byte_older_ip & man_dhcp_reserve_ip_address[ni]) == man_dhcp_reserve_ip_address[ni])) /* broadcast host-ip */
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}


/**********************************************************************************
 * dcli_dhcp_snp_circuitid_string_legal_check
 * 
 *	INPUT:
 *		str	- circuit-id string user entered from vtysh. 
 *		len	- Length of circuit-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long or too short
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_circuitid_string_legal_check
(
	char *str,
	unsigned int len
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = 0;
	char c = 0;

	if ((NULL == str) || (len == 0)) {
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if(len >= DCLI_DHCP_SNP_REMOTEID_STR_LEN || len < 3) {
		return 1;
	}

	c = str[0];
	if(	(c == '_')||
		(c <= 'z' && c >= 'a') ||
		(c <= 'Z' && c >= 'A')
	  ){
		ret = DHCP_SNP_RETURN_CODE_OK;
	}else {
		return 2;
	}

	for (i = 1; i <= len - 1; i++) {
		c = str[i];
		if ((c >= '0' && c <= '9')||
			(c <= 'z' && c >= 'a')||
		    (c <= 'Z' && c >= 'A')||
		    (c == '_')
		    ){
			continue;
		}else {
			ret = 3;
			break;
		}
	}

	return ret;
}

int man_dhcp_snp_lease_time_get(const char* arg, unsigned int* lease_time)
{
    unsigned int time = 0;
    char* endptr = NULL;

    /* 30758400 == 365 * 24 * 60 * 60 */
    if (strlen(arg) > strlen("30758400"))
    {
        return -1;
    }

    time = (unsigned int)strtoul(arg, &endptr, 10);
    if (0 != endptr[0])
    {
        return 0;
    }

    if (time > 30758400)
    {
        return -1;
    }

    *lease_time = time;
        
    return 0;
}
/**********************************************************************************
 * dcli_dhcp_snp_show_bind_table()
 *	DESCRIPTION:
 *		show DHCP-Snooping bind table
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_get_bind_table
(
	struct DCLI_DHCP_SNP_BINDTABLE *bind,
	unsigned int *count
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	int j = 0;
	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE);

	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&bind->chaddr[0],
							DBUS_TYPE_BYTE,&bind->chaddr[1],
							DBUS_TYPE_BYTE,&bind->chaddr[2],
							DBUS_TYPE_BYTE,&bind->chaddr[3],
							DBUS_TYPE_BYTE,&bind->chaddr[4],
							DBUS_TYPE_BYTE,&bind->chaddr[5],
							DBUS_TYPE_UINT16,&bind->vlanId,
 							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
		
	if (DHCP_SNP_RETURN_CODE_OK == ret) {

		memset(bind, 0, sizeof(*bind));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, count);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(bind->bind_type));

		for(j = 0; j< 6; j++) 			
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(bind->chaddr[j]));
		}
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(bind->ip_addr));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(bind->vlanId));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(bind->ifindex));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(bind->lease_time));
	}
	
	dbus_message_unref(reply);
	return ret;
}

/**********************************************************************************
 * dcli_dhcp_snp_config_port()
 *	DESCRIPTION:
 *		config DHCP-Snooping port's trust mode 
 *
 *	INPUTS:
 *	 	unsigned short vlanId,			- vlan id
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,	- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_config_port
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned int trust_mode
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
								NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_UINT32, &eth_g_index,
							 DBUS_TYPE_UINT32, &trust_mode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_snp_config
(
	unsigned char isEnable
)
{ 
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE);
	dbus_error_init(&err);
	
    dbus_message_append_args(query,
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
		return DHCP_SNP_RETURN_CODE_ERROR;
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

unsigned int dcli_dhcp_snp_vlan_config
(
	unsigned short vlanId,
	unsigned char isEnable 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	
	/*get vlanid */

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
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
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_snooping_vlan_show
(
	unsigned int *vlanid
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
    unsigned int length = 0;
    unsigned int* ptr_status = NULL;

	query = dbus_message_new_method_call(
						 	    NPD_DBUS_BUSNAME,	   \
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
							    NPD_DBUS_DHCP_SNP_VLAN_SHOW);
	dbus_error_init(&err); 
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
        	DBUS_TYPE_UINT32, &ret,
        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_status, &length,
        	DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return -1;
	}

    if (NULL != vlanid)
    {
	    memcpy(vlanid, ptr_status, length * sizeof(unsigned int));
    }
    
	dbus_message_unref(reply);	
	return ret;

}

unsigned int dcli_dhcp_snooping_show_vlan_by_id
(
	unsigned short vlanid,
	unsigned int *flag 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;	
	unsigned int temp = 0;
	query = dbus_message_new_method_call(
						 	    NPD_DBUS_BUSNAME,	   
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
							    NPD_DBUS_DHCP_SNP_VLAN_SHOW_BY_VID);
	dbus_error_init(&err); 
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanid,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_UINT32, &temp,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	*flag = temp;
	dbus_message_unref(reply);	
	return ret;
}

unsigned int dcli_dhcp_snooping_global_status_get
(
    struct dcli_dhcp_snooping_global_status_s* dhcp_snp_global_status
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	unsigned int ret = 0;
    
	query = dbus_message_new_method_call(
						 	    NPD_DBUS_BUSNAME,
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
							    NPD_DBUS_DHCP_SNP_OP82_GLOBAL_STATUS_GET);
	dbus_error_init(&err); 
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
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (DHCP_SNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dhcp_snp_global_status->dhcp_snp_enable));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dhcp_snp_global_status->dhcp_snp_opt82_enable));
	}
	
	dbus_message_unref(reply);
    
	return 0;
}

unsigned int dcli_dhcp_information_global_get
(
    struct dcli_dhcp_information_global_status_s* dcli_dhcp_info_global_status
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	unsigned int ret = 0;
	unsigned char *remoteStr = NULL;
    
	query = dbus_message_new_method_call(
						 	    NPD_DBUS_BUSNAME,
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
							    NPD_DBUS_DHCP_SNP_OP82_INFORMATION_GLOBAL_GET);
	dbus_error_init(&err); 
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
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (DHCP_SNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_info_global_status->dhcp_snp_opt82_fill_format_type));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_info_global_status->dhcp_snp_opt82_format_type));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_info_global_status->dhcp_snp_opt82_remoteid_type));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &remoteStr);
        
		strncpy(&(dcli_dhcp_info_global_status->dhcp_snp_opt82_remoteid_str[0]), remoteStr, strlen(remoteStr));
        dcli_dhcp_info_global_status->dhcp_snp_opt82_remoteid_str[strlen(remoteStr)] = 0;
	}
	
	dbus_message_unref(reply);	
    
	return ret;
}

unsigned int dcli_dhcp_snp_show_interface_get
(
    struct dcli_dhcp_snp_port_s* dcli_dhcp_snp_port,
	unsigned int index
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_INTERFACE_SHOW);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &(dcli_dhcp_snp_port->global_port_ifindex),					
							DBUS_TYPE_UINT32, &index,					
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	dbus_message_iter_next(&iter);	

	if (DHCP_SNP_RETURN_CODE_OK == ret) 
	{
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_snp_port->global_port_ifindex));
		dbus_message_iter_next(&iter);	
		
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_snp_port->trust_mode));
		dbus_message_iter_next(&iter);
	}
	dbus_message_unref(reply);

	return ret;
}

unsigned int dcli_dhcp_snp_information_port_get
(
    struct dcli_dhcp_snp_information_port_s* dcli_dhcp_snp_info_port,
    unsigned int index
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char *circuit_temp = NULL;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_OP82_INFORMATION_PORT_GET);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &(dcli_dhcp_snp_info_port->global_port_ifindex),
							DBUS_TYPE_UINT32, &index,					
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	dbus_message_iter_next(&iter);	

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
			
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_snp_info_port->global_port_ifindex));
		dbus_message_iter_next(&iter);	
		
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_snp_info_port->opt82_strategy));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &(dcli_dhcp_snp_info_port->opt82_circuitid));
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &circuit_temp);
		dbus_message_iter_next(&iter);
		strncpy(&(dcli_dhcp_snp_info_port->opt82_circuitid_str[0]), circuit_temp, strlen(circuit_temp));
        dcli_dhcp_snp_info_port->opt82_circuitid_str[strlen(circuit_temp)] = 0;
	}
	dbus_message_unref(reply);

	return ret;
}

unsigned int dcli_dhcp_snooping_information_config
(
	unsigned char isEnable
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
		
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
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
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	
	return ret;
}

unsigned int dcli_dhcp_snooping_opt82_fotmat_config
(
	unsigned char format_type
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FORMAT_TYPE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &format_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

unsigned int dcli_dhcp_snooping_opt82_fill_format_config
(
	unsigned char fill_type
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &fill_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

unsigned int dcli_dhcp_snooping_opt82_remoteid_config
(
	unsigned char remoteid_type,
	unsigned char *remoteid_str
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &remoteid_type,
							DBUS_TYPE_STRING, &remoteid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_snooping_opt82_strategy_config
(	
	unsigned int eth_g_index,
	unsigned char strategy_mode 
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_BYTE, &strategy_mode,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_snooping_opt82_circuit_config
(
	unsigned int eth_g_index,
	unsigned char circuitid_type ,
	unsigned char *circuitid_str 
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_BYTE, &circuitid_type,
							DBUS_TYPE_STRING, &circuitid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;
}

unsigned int dcli_dhcp_snooping_binding_config
(
	struct DCLI_DHCP_SNP_BINDTABLE bind,
	unsigned int opt_type
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_BINDING);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &bind.chaddr[0],
							DBUS_TYPE_BYTE, &bind.chaddr[1],
							DBUS_TYPE_BYTE, &bind.chaddr[2],
							DBUS_TYPE_BYTE, &bind.chaddr[3],
							DBUS_TYPE_BYTE, &bind.chaddr[4],
							DBUS_TYPE_BYTE, &bind.chaddr[5],
							DBUS_TYPE_UINT32, &bind.ip_addr,
							DBUS_TYPE_UINT16, &bind.vlanId,
							DBUS_TYPE_UINT32, &bind.ifindex,
							DBUS_TYPE_UINT32, &opt_type,
							DBUS_TYPE_UINT32, &bind.lease_time,
							DBUS_TYPE_UINT32, &bind.bind_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf( "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf( "%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf( "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;	
}



#ifdef __cplusplus
}
#endif
#endif
