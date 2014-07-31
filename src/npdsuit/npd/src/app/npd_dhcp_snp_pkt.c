
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dhcp_snp_pkt.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		dhcp snooping packet for NPD module.
*
* DATE:
*		06/04/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef HAVE_DHCP_SNP
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_dhcp_snp_pkt.h"


/**********************************************************************************
 *npd_dhcp_snp_get_item_from_pkt()
 *
 *	DESCRIPTION:
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex
 *		NPD_DHCP_MESSAGE_T *packet
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_get_item_from_pkt
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *packet,
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{
    unsigned char *temp = NULL;

	syslog_ax_dhcp_snp_dbg("get user item from packet, ifindex %d.\n", ifindex);
    if ((packet == NULL) || (user == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp get item from pkt error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
    
    user->vlanId    = vlanid;
    user->haddr_len = packet->hlen;

    memcpy(user->chaddr, packet->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);

    /* 为了支持从RELAY到本地SERVER申请IP地址，同时在本地支持SNOOPING的处理
     * 添加对没有通过用户MAC+VLAN定位到用户接口时从数据包取接口的处理，同时
     * 支持对响应数据包不带接口的处理
     */
    user->ifindex = ifindex;
	syslog_ax_dhcp_snp_dbg("packet ->op = %d\n",packet->op);
    if (packet->op == NPD_DHCP_BOOTREPLY)
    {	
   		user->ifindex = 0;
    	syslog_ax_dhcp_snp_dbg("(packet->op == NPD_DHCP_BOOTREPLY)\n");
		user->ip_addr = packet->yiaddr;
		syslog_ax_dhcp_snp_dbg("IP %d.%d.%d.%d\n",(user->ip_addr >> 24) & 0xFF, (user->ip_addr >> 16) & 0xFF, (user->ip_addr >> 8) & 0xFF, user->ip_addr & 0xFF);
		
		temp = (unsigned char *)npd_dhcp_snp_get_option(packet, DHCP_LEASE_TIME);
		if (temp)
		{
			memcpy(&(user->lease_time), temp, 4);
			user->lease_time = ntohl(user->lease_time);
			syslog_ax_dhcp_snp_dbg("lease_time %d\n",user->lease_time);
		}
    }else {
/*		user->ip_addr    = 0;   */
/*		user->lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;    */
    }
/*	user->sys_escape = time(0); */

	syslog_ax_dhcp_snp_dbg("get user item from packet success.\n");
    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_discovery_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int npd_dhcp_snp_discovery_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_USER_ITEM_T temp;

	syslog_ax_dhcp_snp_dbg("receive discovery packet fromvlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp discovery process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = npd_dhcp_snp_get_item_from_pkt(vlanid, ifindex, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);		
		return ret;
	}
	memcpy(&temp,&user,sizeof(user));
	if (DHCP_SNP_RETURN_CODE_ERROR == npd_dhcp_snp_tbl_item_find(&temp)) {
		syslog_ax_dhcp_snp_dbg("no found item from dhcp snooping hash table, then insert a new.\n");
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
        user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
        user.relay_ip = dhcp->giaddr;
        user.xid = dhcp->xid;
		if (npd_dhcp_snp_tbl_item_insert(&user))
		{
			syslog_ax_dhcp_snp_err("insert item to table error\n");			
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_err("insert new item success ! \n");
		return DHCP_SNP_RETURN_CODE_OK;

	}else {
		if(temp.bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC)
		{
            user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
            user.relay_ip = dhcp->giaddr;
            user.xid = dhcp->xid;
            if (0 == user.lease_time)
            {
                if (0 == temp.lease_time)
                {
                    user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
                }
                else
                {
                    user.lease_time =  temp.lease_time;
                }
            }
                
			if(!npd_dhcp_snp_tbl_refresh_bind(&temp,&user))
			{
				syslog_ax_dhcp_snp_err("Refresh the binding table success ! \n");
				return DHCP_SNP_RETURN_CODE_OK;
			}else{
				syslog_ax_dhcp_snp_err("Refresh the binding table faile ! \n");
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
		}
		syslog_ax_dhcp_snp_dbg("found item from dhcp snooping hash table.\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_offer_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 ***********************************************************************************/
unsigned int npd_dhcp_snp_offer_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	syslog_ax_dhcp_snp_dbg("receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_request_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int npd_dhcp_snp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_USER_ITEM_T temp;

	syslog_ax_dhcp_snp_dbg("receive request packet fromvlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp request process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = npd_dhcp_snp_get_item_from_pkt(vlanid, ifindex, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	memcpy(&temp,&user,sizeof(user));
	if (DHCP_SNP_RETURN_CODE_ERROR == npd_dhcp_snp_tbl_item_find(&temp)) {
		syslog_ax_dhcp_snp_dbg("request: no found item from dhcp snooping hash table, then insert a new.\n");
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
        user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT_NAK;
        user.relay_ip = dhcp->giaddr;
        user.xid = dhcp->xid;
		if (npd_dhcp_snp_tbl_item_insert(&user))
		{
			syslog_ax_dhcp_snp_err("insert item to table error\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_err("insert new item success ! \n");
		return DHCP_SNP_RETURN_CODE_OK;

	}else {
		if (temp.bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC)
		{
            if (/* NPD_DHCP_SNP_BIND_STATE_BOUND != temp.state
                || */ temp.ifindex != ifindex
                || temp.vlanId != vlanid 
				|| temp.relay_ip != dhcp->giaddr)
            {
                user.xid = dhcp->xid;
                if (temp.relay_ip != dhcp->giaddr)
                {
                    user.relay_ip = dhcp->giaddr;
                }
                
                user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
                user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT_NAK;
#if 0
                if (0 == user.lease_time)
                {
                    if (0 == temp.lease_time)
                    {
                        user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
                    }
                    else
                    {
                        user.lease_time =  temp.lease_time;
                    }
                }
#endif
                
    			if(!npd_dhcp_snp_tbl_refresh_bind(&temp,&user))
    			{
    				syslog_ax_dhcp_snp_err("Refresh the binding table success ! \n");
    				return DHCP_SNP_RETURN_CODE_OK;
    			}
                else
                {
    				syslog_ax_dhcp_snp_err("Refresh the binding table faile ! \n");
    				return DHCP_SNP_RETURN_CODE_ERROR;
    			}
            }
		}
	}
    
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_ack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int npd_dhcp_snp_ack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_USER_ITEM_T temp;
    char ip_str[32];
	memset(&temp,0,sizeof(temp));
	syslog_ax_dhcp_snp_dbg("receive ack packet from vlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp ack process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = npd_dhcp_snp_get_item_from_pkt(vlanid, ifindex, dhcp, &user);

	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}
    
    if ((0 == user.ip_addr)
        || (0 == user.lease_time))
    {   /* XXX: maybe dhcpack for dhcpinformation reply. */
        return DHCP_SNP_RETURN_CODE_OK;
    }
	memcpy(&temp,&user,sizeof(user));
	
	if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_tbl_item_find(&temp)) 
    {
		if (temp.bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
			/* maybe here need add some codes */
			syslog_ax_dhcp_snp_err("It has been static binding!\n");
			return DHCP_SNP_RETURN_CODE_OK;
    	}        
	}
    else
    {
        return DHCP_SNP_RETURN_CODE_OK;
    }
	lib_get_string_from_ip(ip_str, temp.ip_addr);
	syslog_ax_dhcp_snp_dbg("TEMPA vid %d,ifindex %d,ip IP %s lease time %d MAC %02X:%02X:%02X:%02X:%02X:%02X state %d bindtype %d\n",
			temp.vlanId,temp.ifindex,
			ip_str
			,temp.lease_time,temp.chaddr[0],temp.chaddr[1]
			,temp.chaddr[2],temp.chaddr[3],temp.chaddr[4],temp.chaddr[5],temp.state,temp.bind_type);    
	
	temp.ip_addr = user.ip_addr;
	temp.lease_time = user.lease_time;
/*	temp.sys_escape = time(0);  */
	temp.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
    memcpy(&temp.server_id, (char*)npd_dhcp_snp_get_option(dhcp, DHCP_SERVER_ID), 4);
	ret = npd_dhcp_snp_tbl_refresh_bind(&temp, &temp);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("refresh bind table item value error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_nack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int npd_dhcp_snp_nack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;

	syslog_ax_dhcp_snp_dbg("receive nack packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp nack process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	ret = npd_dhcp_snp_get_item_from_pkt(vlanid, ifindex, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	if (npd_dhcp_snp_tbl_item_find(&user)) {
		return DHCP_SNP_RETURN_CODE_OK;
	}	
	
	if (user.bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		syslog_ax_dhcp_snp_err("It has been static binding!\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}

    if (0 != memcmp(&user.server_id, (char*)npd_dhcp_snp_get_option(dhcp, DHCP_SERVER_ID), 4))
    {
        unsigned int other = 0;
        
        memcpy(&other, (char*)npd_dhcp_snp_get_option(dhcp, DHCP_SERVER_ID), 4);
        syslog_ax_dhcp_snp_err("XXX:  pid = %d (server-id = %x) (nak server-id = %x)\n",
            getpid(), user.server_id, other);
        return DHCP_SNP_RETURN_CODE_OK;
    }

	if (user.bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC) {
		ret = npd_dhcp_snp_tbl_item_delete(&user);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("delete item from bind table error, ret %x\n", ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_release_process()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int npd_dhcp_snp_release_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_USER_ITEM_T temp;

    memset(&user,0,sizeof(user));
	memset(&temp,0,sizeof(temp));
	syslog_ax_dhcp_snp_dbg("receive release packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp release process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	ret = npd_dhcp_snp_get_item_from_pkt(vlanid, ifindex, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	memcpy(&temp,&user,sizeof(user));
	if (npd_dhcp_snp_tbl_item_find(&temp)) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}	
	
	if (temp.bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	ret = npd_dhcp_snp_tbl_identity_item(&temp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("identity user fail for relase msg\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}

	if (temp.bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC) {
		ret = npd_dhcp_snp_tbl_item_delete(&temp);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("delete item from bind table error, ret %x\n", ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

unsigned short npd_dhcp_snp_checksum
(
	void *addr,
	unsigned int count
)
{
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	register int sum = 0;
	unsigned short *source = (unsigned short *)addr;

	while (count > 1)  
	{
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) 
	{
		/* Make sure that the left-over byte is added correctly both
		* with little and big endian hosts */
		unsigned short tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return ~sum;
}

unsigned int npd_dhcp_packet_record_user_info
(
	unsigned short vid,
	unsigned int netif_index,
	NPD_DHCP_MESSAGE_T *data,
	unsigned char message_type,
	unsigned int trustMode
)	
{
	int ret = 0;

	if(data == NULL )
		return DHCP_SNP_RETURN_CODE_ERROR;

	switch (message_type)
	{
		case NPD_DHCP_DISCOVER :
			if(trustMode == NPD_DHCP_SNP_PORT_MODE_NOBIND)
			{
				syslog_ax_dhcp_snp_dbg("It is not necessary to make the record!\n");
				return DHCP_SNP_RETURN_CODE_OK;
			}
			ret = npd_dhcp_snp_discovery_process(vid, netif_index, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_OFFER :
			if (NPD_DHCP_SNP_PORT_MODE_NOTRUST != trustMode) {
				ret = npd_dhcp_snp_offer_process(vid, netif_index, data);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
					return DHCP_SNP_RETURN_CODE_PKT_DROP;
				}
			}
			break;
		case NPD_DHCP_REQUEST :
			if(trustMode == NPD_DHCP_SNP_PORT_MODE_NOBIND)
			{
				syslog_ax_dhcp_snp_dbg("It is not necessary to make the record!\n");
				return DHCP_SNP_RETURN_CODE_OK;
			}
			ret = npd_dhcp_snp_request_process(vid, netif_index, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_ACK :
			ret = npd_dhcp_snp_ack_process(vid, netif_index, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_NAK :
			ret = npd_dhcp_snp_nack_process(vid, netif_index, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_RELEASE :
        case NPD_DHCP_DECLINE :
			ret = npd_dhcp_snp_release_process(vid, netif_index, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("receive dhcp 0x%x packet error, ret %x\n", message_type, ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_INFORM :
			break;
		default :
			syslog_ax_dhcp_snp_dbg("receive dhcp packet unknow message\n");
			break;
	}	
	
	return DHCP_SNP_RETURN_CODE_OK;
}

#ifdef __cplusplus
}
#endif
#endif



