/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
* CREATOR:
*       zhengzw@autelan.com
*
* DESCRIPTION:
*       APIs used in NPD for L3 interface process.
*
* DATE:
*       02/21/2010
*UPDATE:
*05/10/2010              zhengzw@autelan.com          Using DB.
*06/11/2010              zhengzw@autelan.com          L3 interface supported.
*09/14/2010              zhengzw@autelan.com          Add IP in L3 interface struct.
*  FILE REVISION NUMBER:
*       $Revision: 1.128 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_intf.h"
#include "npd/protocol/intf_api.h"

char g_l3intf_name[16] = {'\0'};

extern int adptVirRxFd;

db_table_t *l3intf_table = NULL;
hash_table_index_t *l3intf_netif_index = NULL;
hash_table_index_t *l3intf_ifindex = NULL;
hash_table_index_t *l3local_ifindex = NULL;
hash_table_index_t *l3local_netif_index = NULL;
hash_table_index_t *l3intf_addr_index = NULL;
hash_table_index_t *l3intf_addr_if_index = NULL;

#ifdef HAVE_NPD_IPV6 

hash_table_index_t *v6l3intf_addr_index = NULL;
hash_table_index_t *v6l3intf_addr_if_index = NULL;

#endif //HAVE_NPD_IPV6



unsigned int npd_l3_intf_cmp_ifindex(void *if_struct_a, void *if_struct_b);
unsigned int npd_l3_intf_cmp_ipaddr(void *if_struct_a, void *if_struct_b);
unsigned int npd_l3_intf_cmp_netif_index(void *if_struct_a, void *if_struct_b);

int npd_intf_vlan_add_eth_hw_handler(unsigned short vid, unsigned int eth_g_index);
int npd_intf_vlan_del_eth_hw_handler(unsigned short vid, unsigned int eth_g_index);
int npd_intf_vlan_add_trunk_hw_handler(unsigned short vid, unsigned int tid);
int npd_intf_vlan_del_trunk_hw_handler(unsigned short vid, unsigned int tid);
int npd_intf_trunk_add_eth_hw_handler(unsigned short tid, unsigned int eth_g_index);
int npd_intf_trunk_del_eth_hw_handler(unsigned short tid, unsigned int eth_g_index);
int npd_intf_ip_check
(
	unsigned int ipAddr
)
{
	int retVal = INTERFACE_RETURN_CODE_SUCCESS;
	
	/* legal ip address must not be broadcast,0,or multicast,*/
	/* or 169.254.x.x or 127.0.0.1*/
	if((0==ipAddr) ||
		(~0UI == ipAddr)||
		(0x7F000001 == ipAddr ) ||
		(0xA9FE == ((ipAddr >> 16) & 0xffff)))
	{
		retVal = INTERFACE_RETURN_CODE_IP_CHECK_ERROR;	
	}
	return retVal;
}

void interface_name_convert_to_netif_name(char *ifname, char *netif_name)
{
	int i = 0;
	if(ifname && netif_name)
	{
        if(0 == strncmp("lo", ifname, 2))
        {
            sprintf(netif_name, "loopback0");
            return;
        }
        if(0 == strncmp("vlan", ifname, 4))
        {
            sprintf(netif_name, "vlan %d", atoi(&ifname[4]));
            return;
        }
	    while(ifname[i])
	    {
			if(ifname[i] == '_')
			{
				netif_name[i] = '/';
			}
			else
			{
				netif_name[i] = ifname[i];
			}
			i++;
	    }
		netif_name[i] = '\0';
	}
}

int npd_netif_check_exist(unsigned int netif_index)
{
	unsigned int ret = 0;
	int netif_type = npd_netif_type_get(netif_index);

	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
            ret = npd_check_ethport_exist(netif_index);
			if(ret)
			{
			    return INTERFACE_RETURN_CODE_SUCCESS;
			}
		    return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
            
		case NPD_NETIF_VLAN_TYPE:
			ret = npd_check_vlan_real_exist(npd_netif_vlan_get_vid(netif_index));
			if(ret)
			{
				return INTERFACE_RETURN_CODE_SUCCESS;
			}
			return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		case NPD_NETIF_TRUNK_TYPE:
			ret = npd_check_trunk_exist(npd_netif_trunk_get_tid(netif_index));
			if(ret == TRUNK_RETURN_CODE_TRUNK_EXISTS)
			{
				return INTERFACE_RETURN_CODE_SUCCESS;
			}
			return ret;
#ifdef HAVE_M4_TUNNEL
        case NPD_NETIF_TUNNEL_TYPE:
            {
                return INTERFACE_RETURN_CODE_SUCCESS;
            }
#endif
		default:
			return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
}

int npd_netif_get_status(unsigned int netif_index)
{
	int netif_type = npd_netif_type_get(netif_index);

	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
			return npd_check_eth_port_status(netif_index);
		case NPD_NETIF_VLAN_TYPE:
			return npd_check_vlan_status(npd_netif_vlan_get_vid(netif_index));
		case NPD_NETIF_TRUNK_TYPE:
			return npd_check_trunk_status(npd_netif_trunk_get_tid(netif_index));
#ifdef HAVE_M4_TUNNEL
        case NPD_NETIF_TUNNEL_TYPE:
            {
                return 1;
            }
#endif
		default:
			return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
}

KAP_DEV_TYPE npd_get_netif_kap_dev_type(unsigned int netif_index)
{
	int netif_type = npd_netif_type_get(netif_index);
	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
			return KAP_INTERFACE_PORT_E;
		case NPD_NETIF_VLAN_TYPE:
			return KAP_INTERFACE_VID_E;
		case NPD_NETIF_TRUNK_TYPE:
			return KAP_INTERFACE_TRUNK_E;
		default:
			return KAP_INTERFACE_UNKNOWN_E;
	}
	return KAP_INTERFACE_UNKNOWN_E;
}
/**********************************************************************************
 *  kap_create_intf
 *
 *  DESCRIPTION:
 *      this routine create kernel virtual dev
 *
 *  INPUT:
 *      pif : struct if_cfg_struct* --  param of createing dev
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  - create successed
 *
 **********************************************************************************/
static int kap_create_intf
(
    struct if_cfg_struct* pif
)
{
    if (NULL == pif)
    {
        syslog_ax_intf_err("add l3 interface to kap error parameter\n");
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("add l3 interface to kap error fd %d\n", adptVirRxFd);
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
        if (0 > ioctl(adptVirRxFd, KAPADDIFF, pif))
        {
            syslog_ax_intf_err("add l3 interface to kap ioctl fd %d cmd %#x error\n", \
                               adptVirRxFd, KAPADDIFF);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("add l3 interface to kap success\n");
            return INTERFACE_RETURN_CODE_SUCCESS;
        }
    }
}

/**********************************************************************************
 *  kap_del_intf
 *
 *  DESCRIPTION:
 *      this routine delete kernel virtual dev
 *
 *  INPUT:
 *      pif : struct if_cfg_struct* --  param of deleting dev
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  - operation successed
 *
 **********************************************************************************/
static int kap_del_intf
(
    struct if_cfg_struct* pif
)
{
    if (NULL == pif)
    {
        syslog_ax_intf_err("delete l3 interface from kap error parameter\n");
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("delete l3 interface from kap error fd %d\n", adptVirRxFd);
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
        if (0 > ioctl(adptVirRxFd, KAPDELIFF, pif))
        {
            syslog_ax_intf_err("delete l3 interface from kap ioctl fd %d cmd %#x error\n", \
                               adptVirRxFd, KAPDELIFF);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("delete l3 interface from kap success\n");
            return INTERFACE_RETURN_CODE_SUCCESS;
        }
    }
}

/**********************************************************************************
 *  kap_set_status
 *
 *  DESCRIPTION:
 *      this routine set kernel virtual dev status
 *
 *  INPUT:
 *      pif : struct if_cfg_struct* --  param of deleting dev
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  - operation successed
 *
 **********************************************************************************/
static int kap_set_status
(
    struct if_cfg_struct* pif
)
{
    if (NULL == pif)
    {
        syslog_ax_intf_err("set l3 interface status to kap error parameter\n");
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("set l3 interface status to kap error fd %d\n", adptVirRxFd);
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
        int ret;
        if (0 > (ret = ioctl(adptVirRxFd, KAPSETLINK, pif)))
        {
            syslog_ax_intf_err("set l3 interface status to kap ioctl fd %d cmd %#x error, errno %d\n", \
                               adptVirRxFd, KAPSETLINK, ret);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("set l3 interface status to kap success\n");
            return INTERFACE_RETURN_CODE_SUCCESS;
        }
    }
}

/**********************************************************************************
 *  npd_intf_set_status_kap_handler
 *
 *  DESCRIPTION:
 *      this routine set kap interface status
 *
 *  INPUT:
 *      netif_index  --  l2 netif index
 *      event -- up or down
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR   -  error occured,set failed
 *      INTERFACE_RETURN_CODE_SUCCESS  - set success
 *
 *
 **********************************************************************************/
int npd_intf_set_status_kap_handler
(
    unsigned int ifindex,
    unsigned int event
)
{
    struct if_cfg_struct cmd;
	memset(&cmd, 0, sizeof(cmd));
    cmd.dev_state = event;

    syslog_ax_intf_dbg("l3 interface %d link %s\n", \
                        ifindex, (INTF_CTRL_STATE_UP == event) ? "UP" : "DOWN");
    cmd.l3_index = ifindex;

    if (!kap_set_status(&cmd))
    {
        return INTERFACE_RETURN_CODE_SUCCESS;
    }
    else
    {
        syslog_ax_intf_err("l3 interface %d interface link %s tell kmod fail\n", \
                           ifindex, (INTF_CTRL_STATE_UP == event) ? "UP" : "DOWN");
        return INTERFACE_RETURN_CODE_ERROR;
    }
    
}

/**********************************************************************************
 *  kap_get_l3intf_info
 *
 *  DESCRIPTION:
 *      this routine get kernel virtual dev some infor
 *
  * INPUT:
 *      pif : struct if_cfg_struct* --  param of kap ioctl
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  - operation successed
 *
 **********************************************************************************/
static int kap_get_l3intf_info
(
    struct if_cfg_struct* pif
)
{
	int ret;
	
    if (NULL == pif)
    {
        syslog_ax_intf_err("get l3 interface info from kap error parameter\n");
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("get l3 interface info from kap error fd %d\n", adptVirRxFd);
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
    	ret = ioctl(adptVirRxFd, KAPGETDEVINFO, pif);
        if (0 > ret )
        {
            syslog_ax_intf_err("get l3 interface info from kap ioctl fd %d cmd %#x error %d\n", \
                               adptVirRxFd, KAPGETDEVINFO, ret);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("get l3 interface info from kap success\n");
            return INTERFACE_RETURN_CODE_SUCCESS;
        }
    }
}

/**********************************************************************************
 *  kap_set_l3intf_mac_address
 *
 *  DESCRIPTION:
 *      this routine set unique kap interface mac address
 *
  * INPUT:
 *      pif : struct if_cfg_struct* --  param of kap ioctl
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  - operation successed
 *
 **********************************************************************************/
static int kap_set_l3intf_mac_address
(
    struct if_cfg_struct* pif
)
{
    if (NULL == pif)
    {
        syslog_ax_intf_err("set l3 interface mac to kap error parameter\n");
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("set l3 interface mac to kap error fd %d\n", adptVirRxFd);
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
        if (0 > ioctl(adptVirRxFd, KAPSETMACADDR, pif))
        {
            syslog_ax_intf_err("set l3 interface %d mac %02x:%02x:%02x:%02x:%02x:%02x ioctl fd %d cmd %#x error\n", \
                               pif->l3_index, pif->mac_addr[0], pif->mac_addr[1], pif->mac_addr[2], \
                               pif->mac_addr[3], pif->mac_addr[4], pif->mac_addr[5], \
                               adptVirRxFd, KAPGETDEVINFO);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("set l3 interface %d mac %02x:%02x:%02x:%02x:%02x:%02x success\n", \
                               pif->l3_index, pif->mac_addr[0], pif->mac_addr[1], pif->mac_addr[2], \
                               pif->mac_addr[3], pif->mac_addr[4], pif->mac_addr[5]);
            return INTERFACE_RETURN_CODE_SUCCESS;
        }
    }
}

/****************************************************
 *
 *RETURN:
 *      INTERFACE_RETURN_CODE_ERROR -error occured
 *      INTERFACE_RETURN_CODE_SUCCESS - ifindex get successed
 *
 ****************************************************/
unsigned int npd_intf_ifindex_get_by_ifname
(
    unsigned char *ifName,
    unsigned int *ifIndex
)
{
	unsigned int db_ifindex = 0;
	struct if_cfg_struct cmd;
	int ret = INTERFACE_RETURN_CODE_SUCCESS;

	if ((NULL == ifName) || (NULL == ifIndex))
	{
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	memset(&cmd, 0, sizeof(struct if_cfg_struct));
	strcpy(cmd.if_name, (char *)ifName);
	ret = kap_get_l3intf_info(&cmd);

	if (INTERFACE_RETURN_CODE_SUCCESS != ret)
	{
		syslog_ax_intf_err("get interface %s info from kmod error %d\n", \
						   cmd.if_name, ret);
		return INTERFACE_RETURN_CODE_ERROR;
	}
	else
	{
		ret = npd_intf_exist_check(cmd.l2_index, &db_ifindex);
		if(ret == NPD_FALSE)
		{
			syslog_ax_intf_err("Can not get interface %s int DB\r\n", \
						   cmd.if_name);
			return INTERFACE_RETURN_CODE_ERROR;
		}
		if(db_ifindex != cmd.l3_index)
		{
			syslog_ax_intf_err("L3 interface index(%d) in KAP is not equal with the one (%d) in DB %s int DB\r\n", \
						   cmd.l3_index, db_ifindex);
			return INTERFACE_RETURN_CODE_ERROR;
		}
		*ifIndex = cmd.l3_index;
				
		syslog_ax_intf_dbg("get interface %s :if %d vid %d port %#x type %d state %d\n", \
						   cmd.if_name, cmd.l3_index, cmd.vId, cmd.l2_index, cmd.dev_type, cmd.dev_state);		
	}
	
	return INTERFACE_RETURN_CODE_SUCCESS;
}

int npd_intf_create_by_netif_index
(
    unsigned int netif_index,
    unsigned short vid,
    unsigned int netif_state,
    unsigned int* ifIndex,
    char* name,
    unsigned char *mac
)
{
    struct if_cfg_struct cmd;

    int ret = 0;
    memset(&cmd, 0, sizeof(struct if_cfg_struct));
    strcpy(cmd.if_name, name);
    cmd.if_flag = KAP_IFF_TAP | KAP_IFF_NO_PI;
    cmd.l3_index = 0;
    cmd.dev_type = npd_get_netif_kap_dev_type(netif_index);
    cmd.dev_state = netif_state;
    cmd.l2_index = netif_index;
    cmd.vId = vid;
   	memcpy(cmd.mac_addr, mac, 6);

    syslog_ax_intf_dbg("create l3 interface by netif index 0x%x, vid %d name %s state %s\n", \
                       netif_index, cmd.vId, cmd.if_name, cmd.dev_state ? "UP" : "DOWN");
    ret = kap_create_intf(&cmd);
    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_err("create l3 if by vlan %d error\n", vid); 
        return ret;
    }
    else
    {
        *ifIndex = cmd.l3_index;
        strcpy(name, cmd.if_name);
        syslog_ax_intf_dbg("new created l3 if name %s index %d\n", cmd.if_name, cmd.l3_index);
        return INTERFACE_RETURN_CODE_SUCCESS;
    }
}

int npd_intf_set_ip_addr_kap_handler
(
    unsigned int ifIndex,
    unsigned int ipv4,
    unsigned int ipv4_mask
)
{
#if 0    
    struct if_cfg_struct cmd;
    struct sockaddr_in *if_data;
    int ret = 0;
	int i = 0;
    memset(&cmd, 0, sizeof(struct if_cfg_struct));
    syslog_ax_intf_dbg("set l3 interface ip address by l3 ifindex %d: %d.%d.%d.%d:%d.%d.%d.%d\r\n", \
                       ifIndex, \
                       (ipv4 >> 24)&0xFF, (ipv4 >> 16)&0xFF, (ipv4 >> 8)&0xFF, ipv4&0xFF, \
                       (ipv4_mask >> 24)&0xFF, (ipv4_mask >> 16)&0xFF, (ipv4_mask >> 8)&0xFF, ipv4_mask&0xFF);
	ret = npd_intf_ip_check(ipv4);
    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_err("Input IP address check error\r\n");
        return ret;
    }

    cmd.l3_index = ifIndex;
    if_data = (struct sockaddr_in *)cmd.if_addr;

    for (i = 0; i < MAX_IP_COUNT; i++)
    {
        if_data[i].sin_family = AF_INET;
        if_data[i].sin_addr.s_addr = INVALID_HOST_IP;
        cmd.netmask[i] = 0;
    }

    pthread_mutex_lock(&semKapIoMutex);
    ret = ioctl(adptVirRxFd, KAPGETIPADDRS, &cmd);
    pthread_mutex_unlock(&semKapIoMutex);

#if 0  //TODO --liz
    if (0 > ret)
    {
        syslog_ax_intf_dbg("Get ip of interface %d failed, ret = %d \r\n", ifIndex, ret);
        /*return INTERFACE_RETURN_CODE_IOCTL_ERROR;*/
    }
    else
#endif
    {
        for (i = 0; i < MAX_IP_COUNT; i++)
        {
            if (INVALID_HOST_IP != if_data[i].sin_addr.s_addr)
            {
				if(if_data[i].sin_addr.s_addr == ipv4 && cmd.netmask[i] == ipv4_mask)
				{
					syslog_ax_intf_dbg("IP address to be setted has already been atctive\r\n");
					return INTERFACE_RETURN_CODE_SUCCESS;
				}
            }
        }
        memset(&cmd, 0, sizeof(struct if_cfg_struct));
        cmd.l3_index = ifIndex;
        if_data = (struct sockaddr_in *)cmd.if_addr;
        if_data->sin_family = AF_INET;
        if_data->sin_addr.s_addr = ipv4;
		cmd.netmask[0] = ipv4_mask;
        pthread_mutex_lock(&semKapIoMutex);
        ret = ioctl(adptVirRxFd, KAPSETIPADDR, &cmd);
        pthread_mutex_unlock(&semKapIoMutex);
        if(ret < 0)
        {
			syslog_ax_intf_dbg("Set ip of interface %d failed, ret=%d \r\n", ifIndex,ret);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
    }
#endif
    return INTERFACE_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_intf_del_by_ifindex
 *
 *  DESCRIPTION:
 *      this routine delete virtual dev
 *
 *  INPUT:
 *      ifindex -- dev index
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *      INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *      INTERFACE_RETURN_CODE_SUCCESS  -  intf del SUCCESSED
 *
 *
 **********************************************************************************/
int npd_intf_del_by_ifindex
(
    unsigned int ifindex
)
{
    struct if_cfg_struct cmd;
    int ret = 0;

    memset(&cmd, 0, sizeof(struct if_cfg_struct));
    syslog_ax_intf_dbg("npd_del_intf_by_ifindex :: intf index %d\n", ifindex);
    
    cmd.l3_index = ifindex;
    ret = kap_del_intf(&cmd);

    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("kap_del_intf err\n");
        return ret;
    }
    else
    {
        syslog_ax_intf_dbg("npd_del_intf succeed\n");
        return INTERFACE_RETURN_CODE_SUCCESS;
    }
}

/**********************************************************************************
 *  npd_intf_config_basemac
 *
 *  DESCRIPTION:
 *      this routine set route mac
 *
 *  INPUT:
 *      devNum
 *      port
 *      mac
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR  - null pointer error
 *      INTERFACE_RETURN_CODE_NAM_ERROR  - maybe driver operation failed
 *      INTERFACE_RETURN_CODE_SUCCESS  - operation success
 *
 *
 **********************************************************************************/
int npd_intf_config_basemac
(
    unsigned int netif_index,
    unsigned char* mac
)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;

    if (NULL == mac)
    {
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    ret = nam_intf_config_basemac(netif_index, mac);
    return ret;
}

int npd_intf_set_proxy_arp_handle
(
	unsigned char *ifname,
	unsigned char proxy_arp
)
{
	int ret = 0;
	char buff[64];

	memset(buff,0,64);
	
	if(TRUE == proxy_arp)
	{
		sprintf(buff,"echo 1 > /proc/sys/net/ipv4/conf/%s/proxy_arp", ifname);
	}
	else
	{
		sprintf(buff,"echo 0 > /proc/sys/net/ipv4/conf/%s/proxy_arp", ifname);
	}

	ret = system(buff);

	return ret;
}
/**********************************************************************************
 *  npd_intf_get_intf_mac
 *
 *  DESCRIPTION:
 *      this routine get dev mac address
 *
 *  INPUT:
 *      ifindex             --  dev index
 *
 *  OUTPUT:
 *      addr            -- mac address
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_SUCCESS
 *
 *
 **********************************************************************************/
#define ENODEV 19
int npd_intf_get_intf_mac
(
    unsigned int ifIndex,
    unsigned char* addr
)
{

    NPD_L3INTERFACE_CTRL l3intf={0};
    int ret;

    l3intf.netif_index = ifIndex;
    if (NULL == addr)
    {
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    ret = dbtable_hash_search(l3intf_netif_index, &l3intf, npd_l3_intf_cmp_netif_index,
            &l3intf);
    if(0 != ret)
        return INTERFACE_RETURN_CODE_NO_SUCH_PORT;

    memcpy(addr, l3intf.mac_addr, 6);

    syslog_ax_intf_dbg("get intf mac: %02x:%02x:%02x:%02x:%02x:%02x\n", \
                       addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return INTERFACE_RETURN_CODE_SUCCESS;
}

int npd_intf_get_proxy_arp(
    unsigned int netif_index,
    unsigned char* proxy_arp
)
{

	NPD_L3INTERFACE_CTRL l3intf={0};
	int ret;

	l3intf.netif_index = netif_index;
	if (NULL == proxy_arp)
	{
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	ret = dbtable_hash_search(l3intf_netif_index, &l3intf, npd_l3_intf_cmp_netif_index,
			&l3intf);
	if(0 != ret)
	{
		syslog_ax_intf_err("intf %#x is not found.\n", netif_index);
		return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
	}

	*proxy_arp = l3intf.proxy_arp;

	syslog_ax_intf_dbg("get intf %#x proxy arp: %s\n", netif_index, (l3intf.proxy_arp==TRUE)?"enable":"disable");

	return INTERFACE_RETURN_CODE_SUCCESS;
}

#ifdef HAVE_PORTAL

unsigned int npd_intf_set_portal_server
(
            unsigned int netif_index,
            unsigned int portal_ctrl,
            unsigned int portal_srv_id,
            unsigned int* ipv4
)
{
	unsigned int result = 0,ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    NPD_L3INTF_ADDR intfAddr;
    
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	
	l3intf_ctrl.portal_ctrl = portal_ctrl;
	l3intf_ctrl.portal_srv_id = portal_srv_id;
	result= dbtable_hash_update(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
    
    intfAddr.ifindex = l3intf_ctrl.ifindex;
    ret = dbtable_hash_head_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index);
	if(0 == ret)
		*ipv4 = intfAddr.ipAddr;

    if(portal_ctrl == TRUE)
        npd_intf_portal_set(TRUE,netif_index);
    else
        npd_intf_portal_set(FALSE,netif_index);
   
	return INTERFACE_RETURN_CODE_SUCCESS;
}

unsigned int npd_intf_get_portal_server
(
    unsigned int *netif_index,
    unsigned int *portal_ctrl,
    unsigned int *portal_srv_id
)
{
	unsigned int result = 0,ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    NPD_L3INTF_ADDR intfAddr;

	memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
	l3intf_ctrl.netif_index = *netif_index;

	if(0 == dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl))
	{
		*portal_ctrl = l3intf_ctrl.portal_ctrl;
		*portal_srv_id = l3intf_ctrl.portal_srv_id;
		return INTERFACE_RETURN_CODE_SUCCESS;
	}
	    
	return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
}


#endif

int npd_intf_set_proxy_arp(
    unsigned int netif_index,
    unsigned char proxy_arp
)
{
	unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	if(l3intf_ctrl.proxy_arp != proxy_arp)
	{
		l3intf_ctrl.proxy_arp = proxy_arp;
		
		result= dbtable_hash_update(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl);
		if(result != 0)
		{
			return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		}		
	}
	return INTERFACE_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 *  npd_intf_set_l3intf_status
 *
 *  DESCRIPTION:
 *      this routine set L3 interface and update the db status and kap status
 *
 *  INPUT:
 *      netif_index  --  l2 netif index
 *      event -- up or down
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR   -  error occured,set failed
 *      INTERFACE_RETURN_CODE_SUCCESS  - set success
 *
 *
 **********************************************************************************/
int npd_intf_set_l3intf_status
(
    unsigned int netif_index,
    enum INTF_STATUS_E event
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	if(l3intf_ctrl.state != event)
	{
    	l3intf_ctrl.state = event;
    	
    	result= dbtable_hash_update(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl);
    	if(result != 0)
    	{
    		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
    	}
		/*
		result = npd_intf_set_status_kap_handler(l3intf_ctrl.ifindex, event);
		if(result != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return INTERFACE_RETURN_CODE_ERROR;
		}
		*/
	}
    return INTERFACE_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_intf_get_l3intf_ctrl_status
 *
 *  DESCRIPTION:
 *      this routine set L3 interface and update the db status and kap status
 *
 *  INPUT:
 *      netif_index  --  l2 netif index
 *      event -- up or down
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR   -  error occured,set failed
 *      INTERFACE_RETURN_CODE_SUCCESS  - set success
 *
 *
 **********************************************************************************/
int npd_intf_get_l3intf_ctrl_status
(
    unsigned int ifindex,
    unsigned int *state
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.ifindex = ifindex;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result != 0)
	{
	    syslog_ax_intf_err("Get interface ctrl status fail! Not get interface!");
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	*state = l3intf_ctrl.ctrl_state;
    return INTERFACE_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 *  npd_intf_set_l3intf_ctrl_status
 *
 *  DESCRIPTION:
 *      this routine set L3 interface and update the db status and kap status
 *
 *  INPUT:
 *      netif_index  --  l2 netif index
 *      event -- up or down
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR   -  error occured,set failed
 *      INTERFACE_RETURN_CODE_SUCCESS  - set success
 *
 *
 **********************************************************************************/
 int npd_intf_set_l3intf_bind_status
(
    unsigned int ifindex,
    int event
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	
	l3intf_ctrl.ifindex= ifindex;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

	l3intf_ctrl.bind = event;
	result = dbtable_hash_update(l3intf_ifindex, &l3intf_ctrl, &l3intf_ctrl);

	if(result == 0)
	{
		return INTERFACE_RETURN_CODE_SUCCESS;
	}

	return INTERFACE_RETURN_CODE_ERROR;
}

int npd_intf_set_l3intf_ctrl_status
(
    unsigned int ifindex,
    enum INTF_CTRL_STATUS_E event
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	
	l3intf_ctrl.ifindex= ifindex;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

	npd_key_database_lock();
	if(INTF_CTRL_STATE_DOWN == event)
	{   
		l3intf_ctrl.ctrl_state = INTF_CTRL_STATE_DOWN;
		l3intf_ctrl.state = npd_intf_get_l3intf_status(l3intf_ctrl.netif_index);
		l3intf_ctrl.state &= l3intf_ctrl.ctrl_state;	 
    	result = dbtable_hash_update(l3intf_ifindex, &l3intf_ctrl, &l3intf_ctrl);
	}
	else if(INTF_CTRL_STATE_UP == event)
	{
		l3intf_ctrl.ctrl_state = INTF_CTRL_STATE_UP;
		l3intf_ctrl.state = npd_intf_get_l3intf_status(l3intf_ctrl.netif_index);
		l3intf_ctrl.state &= l3intf_ctrl.ctrl_state;
    	result = dbtable_hash_update(l3intf_ifindex, &l3intf_ctrl, &l3intf_ctrl);    	
	}
	npd_key_database_unlock();

	if(result == 0)
	{
		return INTERFACE_RETURN_CODE_SUCCESS;
	}

	return INTERFACE_RETURN_CODE_ERROR;
}


int npd_intf_set_l3intf_kap_status
(
    unsigned int netif_index,
    enum INTF_STATUS_E event
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	if( (l3intf_ctrl.state != event) && (INTF_CTRL_STATE_UP == l3intf_ctrl.ctrl_state))
	{
    	l3intf_ctrl.state = event;
    	
    	result= dbtable_hash_update(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl);
    	if(result != 0)
    	{
    		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
    	}

		if( l3intf_ctrl.state == INTF_STATE_DOWN_E &&
			l3intf_ctrl.ipv4 != 0 )
		{
			npd_arp_snooping_del_by_network(l3intf_ctrl.ipv4, l3intf_ctrl.mask);
			npd_arp_snooping_static_valid_set_by_network(l3intf_ctrl.ipv4, l3intf_ctrl.mask, FALSE);
		}
		else if( l3intf_ctrl.state == INTF_STATE_UP_E &&
			l3intf_ctrl.ipv4 != 0 )
		{
			npd_arp_snooping_static_valid_set_by_network(l3intf_ctrl.ipv4, l3intf_ctrl.mask, TRUE);
		}
	}
	
    return INTERFACE_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_intf_get_l3intf_status
 *
 *  DESCRIPTION:
 *      this routine get netif interface status and updatekap interface status
 *
 *  INPUT:
 *      netif_index  --  l2 netif index
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      DEV_LINK_DOWN-the interface is down
 *           DEV_LINK_UP-the interface is up
 *           INTERFACE_RETURN_CODE_ERROR  -  status check failed
 *           INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST - l3 interface not exists
 *
 *
 **********************************************************************************/

unsigned int npd_intf_get_l3intf_status
(
    unsigned int netif_index
)
{
    unsigned int ifindex = ~0UI;
    int ret = 0;
    int intfStatus = INTF_STATE_DOWN_E;
    ret = npd_intf_exist_check(netif_index, &ifindex);

    if (!((TRUE == ret) && (ifindex != ~0UI)))
    {
        ret = INTERFACE_RETURN_CODE_ERROR;
    }
    else
    {
        intfStatus = npd_netif_get_status(netif_index);
        if(intfStatus == INTF_STATE_UP_E)
        {
			ret = INTF_STATE_UP_E;
        }
		else
		{
			ret = INTF_STATE_DOWN_E;
		}

		//npd_intf_set_l3intf_status(netif_index, ret);
    }

    return ret;
}

unsigned int npd_intf_get_l3intf_status_by_ifindex
(
    unsigned int ifindex
)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.ifindex = ifindex;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result != 0)
	{
		return 0;
	}
	
    return l3intf_ctrl.state;
}

unsigned int npd_intf_set_kern_flag
(
    unsigned char* ifname,
    unsigned int event
)
{
    int sock;
	int ret = INTERFACE_RETURN_CODE_SUCCESS;
	struct ifreq cmd;

	if (NULL == ifname)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}

	memset(&cmd, 0, sizeof(struct ifreq));
	strncpy(cmd.ifr_name, (char *)ifname, sizeof(cmd.ifr_name));
    if((sock = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        syslog_ax_intf_err("Socket create failed!");
        return INTERFACE_RETURN_CODE_ERROR;
    }
	if(ioctl(sock,SIOCGIFFLAGS,&cmd) < 0)
	{
		syslog_ax_intf_err("Get interface flag failed!errno %s", strerror(errno));
		ret = INTERFACE_RETURN_CODE_ERROR;
	}
	else
	{
		if(INTF_CTRL_STATE_DOWN == event)
		{
	    	cmd.ifr_ifru.ifru_flags &= ~IFF_UP;
		}
		else {
			cmd.ifr_ifru.ifru_flags |=  (IFF_UP | IFF_RUNNING);
		}
		
	    if(ioctl(sock,SIOCSIFFLAGS,&cmd) < 0)
	    {
	        syslog_ax_intf_err("Set interface flag failed,errno %d:%s", errno, strerror(errno));
	        ret = INTERFACE_RETURN_CODE_ERROR;
	    }
	}
	close(sock);
    return ret;
}
int npd_intf_add_ip_addr
(
    unsigned int ifindex,
    unsigned int ipv4,
    unsigned int ipv4_mask
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTF_ADDR l3intfAddr;
	unsigned int netif_index = 0;
	unsigned int netAddr[2];
	
	l3intfAddr.ifindex = ifindex;
	l3intfAddr.ipAddr = ipv4;
	l3intfAddr.mask = ipv4_mask;
	
	result= dbtable_hash_search(l3intf_addr_index, &l3intfAddr, NULL, &l3intfAddr);

	if( result != 0 )
	{
		result = dbtable_hash_insert(l3intf_addr_index, &l3intfAddr);
	}
	else
	{
		result = INTERFACE_RETURN_CODE_INTERFACE_EXIST;
	}

	npd_intf_netif_get_by_ifindex(l3intfAddr.ifindex, &netif_index);
	netAddr[0] = l3intfAddr.ipAddr;
	netAddr[1] = l3intfAddr.mask;
	npd_key_database_lock();
	netif_notify_relate_event_out(netif_index, l3intfAddr.ipAddr, PORT_NOTIFIER_ADDR_ADD, (char *)netAddr, sizeof(netAddr));
	npd_key_database_unlock();
	netif_app_notify_relate_event(netif_index, netif_index, PORT_NOTIFIER_ADDR_ADD, (char *)netAddr, sizeof(netAddr));
	
    return result;
}

int npd_l3_intf_addr_del(hash_table_index_t *hash, void *data,unsigned int flag)
{
	NPD_L3INTF_ADDR *l3intfAddr = (NPD_L3INTF_ADDR *)data;	
	unsigned int netif_index = 0;
	unsigned int netAddr[2];

	if( l3intfAddr == NULL )
		return -1;

	npd_intf_netif_get_by_ifindex(l3intfAddr->ifindex, &netif_index);
	netAddr[0] = l3intfAddr->ipAddr;
	netAddr[1] = l3intfAddr->mask;
	npd_key_database_lock();
	netif_notify_relate_event_out(netif_index, l3intfAddr->ipAddr, PORT_NOTIFIER_ADDR_DEL, (char *)netAddr, sizeof(netAddr));
	npd_key_database_unlock();
	netif_app_notify_relate_event(netif_index, netif_index, PORT_NOTIFIER_ADDR_DEL, (char *)netAddr, sizeof(netAddr));

	return dbtable_hash_delete(l3intf_addr_index, l3intfAddr, l3intfAddr);
}

int npd_intf_del_ip_addr
(
    unsigned int ifindex,
    unsigned int ipv4,
    unsigned int ipv4_mask
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTF_ADDR l3intfAddr;
	
	l3intfAddr.ifindex = ifindex;
	l3intfAddr.ipAddr = ipv4;
	l3intfAddr.mask = ipv4_mask;
	
	result= dbtable_hash_search(l3intf_addr_index, &l3intfAddr, NULL, &l3intfAddr);

	if(result != 0)
	{
		result = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	else {
		result = npd_l3_intf_addr_del(l3intf_addr_index, &l3intfAddr, TRUE);
	}
	
    return result;
}
#ifdef HAVE_NPD_IPV6
int npd_intf_add_ipv6_addr
(
    unsigned int ifindex,
    ip6_addr *ipv6,
    ip6_addr *ipv6_mask
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_V6_L3INTF_ADDR l3intfAddr;
	
	l3intfAddr.ifindex = ifindex;
	memcpy(&l3intfAddr.ipv6Addr, ipv6, sizeof(l3intfAddr.ipv6Addr));
	memcpy(&l3intfAddr.v6mask, ipv6_mask, sizeof(l3intfAddr.v6mask));
	
	result= dbtable_hash_search(v6l3intf_addr_index, &l3intfAddr, NULL, &l3intfAddr);

	if( result != 0 )
	{
		result = dbtable_hash_insert(v6l3intf_addr_index, &l3intfAddr);
	}
	else
	{
		result = INTERFACE_RETURN_CODE_INTERFACE_EXIST;
	}
	
    return result;
}

int npd_intf_del_ipv6_addr
(
    unsigned int ifindex,
    ip6_addr *ipv6,
    ip6_addr *v6_mask)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_V6_L3INTF_ADDR l3intfAddr;
	
	l3intfAddr.ifindex = ifindex;
	memcpy(&l3intfAddr.ipv6Addr, ipv6, sizeof(l3intfAddr.ipv6Addr));
	memcpy(&l3intfAddr.v6mask, v6_mask, sizeof(l3intfAddr.v6mask) );
	
	result= dbtable_hash_search(v6l3intf_addr_index, &l3intfAddr, NULL, &l3intfAddr);

	if(result != 0)
	{
		result = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	else {
		result = dbtable_hash_delete(v6l3intf_addr_index, &l3intfAddr, &l3intfAddr);
	}
	
    return result;
}
#endif //HAVE_NPD_IPV6

int npd_intf_del_all_ip_addr(unsigned int ifindex)
{
	int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTF_ADDR l3intfAddr;
	
	l3intfAddr.ifindex = ifindex;

	result = dbtable_hash_traversal(l3intf_addr_index, TRUE, &l3intfAddr, npd_l3_intf_addr_filter_by_index,
							npd_l3_intf_addr_del);	
	
    return result;
}

#ifdef HAVE_M4_TUNNEL
unsigned int npd_l3_intf_addr_filter_by_prefix(void * data_inner, void * data)
{
    NPD_L3INTF_ADDR * entry_inner = (NPD_L3INTF_ADDR *)data_inner;
    NPD_L3INTF_ADDR * entry = (NPD_L3INTF_ADDR *)data;

    if ((NULL == entry_inner) || (NULL == entry))
    {
        syslog_ax_intf_err("Npd interface address compare error(null-pointer).");
        return FALSE;
    }

    if ((entry_inner->mask & entry_inner->ipAddr) == (entry_inner->mask & entry->ipAddr))
    {
        return TRUE;
    }

    return FALSE;
}

int npd_intf_in4_fallin_network(unsigned int in4)
{
    NPD_L3INTF_ADDR ip;

    ip.ipAddr = in4;
    ip.mask = 0;
    ip.ifindex = 0;
    
    return dbtable_hash_traversal(l3intf_addr_index,
                            0,
                            &ip,
                            npd_l3_intf_addr_filter_by_prefix,
                            NULL);
}
#endif


/****************************************************
 *npd_intf_l3_mac_legality_check
 *  Description:
 *      to check the mac  of L3 interface legality,
 *      multicast mac , broadcast mac, and all zero mac is in vaild
 *
 *  Input:
 *      ethmac      - source mac
 *
 *  Output:
 *      NULL
 *
 *  ReturnCode:
 *      TRUE        - the L3 macs  legal
 *      FALSE       - the L3 macs  illegal
 *
 ****************************************************/
unsigned int npd_intf_l3_mac_legality_check
(
    unsigned char * ethmac
)
{
    if (NULL == ethmac)
    {
        return FALSE;
    }

    syslog_ax_intf_dbg("check L3 interface mac legality:%02x:%02x:%02x:%02x:%02x:%02x\n",
                       ethmac[0], ethmac[1], ethmac[2], ethmac[3], ethmac[4], ethmac[5]);

    if (npd_arp_snooping_is_brc_mac(ethmac))
    {
        syslog_ax_intf_err("the mac is broadcast mac!\n");
        return FALSE;
    }

    if (npd_arp_snooping_is_muti_cast_mac(ethmac))
    {
        syslog_ax_intf_err("the mac is multicast mac!\n");
        return FALSE;
    }

    if (npd_arp_snooping_is_zero_mac(ethmac))
    {
        syslog_ax_intf_err("the mac is zero mac!\n");
        return FALSE;
    }

    return TRUE;
}

#if 0
/**********************************************************************************
 *  npd_intf_get_ip_addrs
 *
 *  This method is used to get L3 interface ip address and mask.
 *
 *  INPUT:
 *      ifindex - L3 interface index
 *
 *  OUTPUT:
 *      ipAddr - ip address
 *      mask - ip mask
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS  -  get ip success
 *      INTERFACE_RETURN_CODE_FD_ERROR  -  fd illegal
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR  -  ioctl failed or error occured
 *
 **********************************************************************************/
int npd_intf_get_ip_addrs
(
    unsigned int   ifindex,
    unsigned int  *ipAddr,
    unsigned int  *mask
)
{
    struct if_cfg_struct cmd;
    struct sockaddr_in *if_data;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
    int i = 0;

    if (!adptVirRxFd || adptVirRxFd < 0)
    {
        syslog_ax_intf_err("fd invalid\n");
        return INTERFACE_RETURN_CODE_FD_ERROR;
    }
    else
    {
        memset(&cmd, 0, sizeof(cmd));
        /*find if_index*/
        cmd.l3_index = ifindex;
        if_data = (struct sockaddr_in *)cmd.if_addr;

        for (i = 0; i < MAX_IP_COUNT; i++)
        {
            if_data[i].sin_family = AF_INET;
            if_data[i].sin_addr.s_addr = INVALID_HOST_IP;
            cmd.netmask[i] = 0;
        }

        pthread_mutex_lock(&semKapIoMutex);
        ret = ioctl(adptVirRxFd, KAPGETIPADDRS, &cmd);
        pthread_mutex_unlock(&semKapIoMutex);

        if (0 > ret)
        {
            syslog_ax_intf_err("npd_intf_get_ip_addrs::get ip of interface %d failed \n", ifindex);
            return INTERFACE_RETURN_CODE_IOCTL_ERROR;
        }
        else
        {
            syslog_ax_intf_dbg("npd_intf_get_ip_addrs::get if %#0x ip address:\n", ifindex);

            for (i = 0; i < MAX_IP_COUNT; i++)
            {
                ipAddr[i] = if_data[i].sin_addr.s_addr;
                mask[i] = cmd.netmask[i];

                if (INVALID_HOST_IP != ipAddr[i])
                {
                    syslog_ax_intf_dbg("\t%d.%d.%d.%d %d.%d.%d.%d\n",  \
                                       (ipAddr[i] >> 24)&0xFF, (ipAddr[i] >> 16)&0xFF, (ipAddr[i] >> 8)&0xFF, (ipAddr[i])&0xFF, \
                                       (mask[i] >> 24)&0xFF, (mask[i] >> 16)&0xFF, (mask[i] >> 8)&0xFF, (mask[i])&0xFF);
                }
            }
        }
    }

    return INTERFACE_RETURN_CODE_SUCCESS;
}
#endif

/**********************************************************************************
 *  npd_intf_get_info
 *
 *  DESCRIPTION:
 *      this routine get dev infor
 *
 *  INPUT:
 *      name -- dev name
 *
 *  OUTPUT:
 *      ifindex -- dev index
 *      vid     -- vlan id
 *      eth-g-index -- port index
 *      type    -- dev type
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_SUCCESS
 *
 **********************************************************************************/
int npd_intf_get_info
(
    char * name,
    unsigned int *ifIndex,
    unsigned short* vid,
    unsigned int* netif_index
)
{
	unsigned int db_ifindex = 0;
    struct if_cfg_struct cmd;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;

    if ((NULL == name) || (NULL == ifIndex) || \
            (NULL == vid) || (NULL == netif_index))
    {
        return COMMON_RETURN_CODE_NULL_PTR;
    }

    memset(&cmd, 0, sizeof(struct if_cfg_struct));
    /*cmd.if_name[15] = '\0';*/
    strcpy(cmd.if_name, name);
    ret = kap_get_l3intf_info(&cmd);

    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_err("get interface %s info from kmod error %d\n", \
                           cmd.if_name, ret);
        return INTERFACE_RETURN_CODE_ERROR;
    }
    else
    {
		ret = npd_intf_exist_check(cmd.l2_index, &db_ifindex);
		if(ret == NPD_FALSE)
		{
            syslog_ax_intf_err("Can not get interface %s int DB\r\n", \
                           cmd.if_name);
            return INTERFACE_RETURN_CODE_ERROR;
		}
		if(db_ifindex != cmd.l3_index)
		{
            syslog_ax_intf_err("L3 interface index(%d) in KAP is not equal with the one (%d) in DB %s int DB\r\n", \
                           cmd.l3_index, db_ifindex);
            return INTERFACE_RETURN_CODE_ERROR;
		}
        npd_intf_get_global_l3index(db_ifindex, ifIndex);
        *vid = (unsigned short)cmd.vId;
        *netif_index = cmd.l2_index;
        syslog_ax_intf_dbg("get interface %s info:if %d vid %d port %#x type %d state %d\n", \
                           cmd.if_name, cmd.l3_index, cmd.vId, cmd.l2_index, cmd.dev_type, cmd.dev_state);
        return INTERFACE_RETURN_CODE_SUCCESS;
    }
}

/**********************************************************************************
 *  npd_intf_set_mac_address
 *
 *  DESCRIPTION:
 *      this routine set mac address for a specified l3 interface
 *
 *  INPUT:
 *      ifIndex -- l3 interface index
 *      macAddr
 *  OUTPUT:
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_SUCCESS
 *
 **********************************************************************************/
int npd_intf_set_mac_address
(
    unsigned int ifIndex,
    unsigned char * macAddr
)
{
    struct if_cfg_struct pif;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;

    if (!macAddr)
    {
        syslog_ax_intf_err("set interface %d mac address with mac null\n", ifIndex);
        return INTERFACE_RETURN_CODE_ERROR;
    }

    memset(&pif, 0, sizeof(struct if_cfg_struct));
    memcpy(pif.mac_addr , macAddr, 6);
    pif.l3_index = ifIndex;
    ret = kap_set_l3intf_mac_address(&pif);

    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_err("set interface %d mac address to kmod error %d\n", \
                           pif.l3_index, ret);
        return INTERFACE_RETURN_CODE_ERROR;
    }
    else
    {
        syslog_ax_intf_dbg("set interface %d mac %02x:%02x:%02x:%02x:%02x:%02x success\n", \
                           pif.l3_index, pif.mac_addr[0], pif.mac_addr[1], pif.mac_addr[2], \
                           pif.mac_addr[3], pif.mac_addr[4], pif.mac_addr[5]);
        return INTERFACE_RETURN_CODE_SUCCESS;
    }
}
/**********************************************************************************
 *  npd_intf_udp_bc_trap_enable
 *
 *  DESCRIPTION:
 *      this routine set udp bc trap
 *
 *  INPUT:
 *      vlanid
 *      enable
 *
 *  OUTPUT:
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_NAM_ERROR  - enable set failed
 *      INTERFACE_RETURN_CODE_SUCCESS  -  set successed
 *
 **********************************************************************************/
unsigned int npd_intf_udp_bc_trap_enable
(
    unsigned int vlanId,
    unsigned int enable
)
{
    unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
    ret = nam_asic_udp_bc_trap_en((unsigned short)vlanId, enable);

    return ret ;
}


void npd_intf_notify_event
(
	unsigned int netif_index, 
	enum PORT_NOTIFIER_ENT evt,
	char *private, int len
)
{
	unsigned int l3index = 0;
	syslog_ax_intf_dbg("npd_intf_notify_event: index 0x%x, event %d\n", netif_index, evt);
	
	switch(evt)
	{
		case PORT_NOTIFIER_LINKUP_E:
			npd_intf_set_l3intf_kap_status(netif_index,INTF_STATE_UP_E);
			break;
		case PORT_NOTIFIER_LINKDOWN_E:
        case PORT_NOTIFIER_REMOVE:
			npd_intf_set_l3intf_kap_status(netif_index,INTF_STATE_DOWN_E);
			break;
        case PORT_NOTIFIER_DELETE:
			if(NPD_TRUE == npd_intf_gindex_exist_check(netif_index, &l3index))
			{
			    npd_l3_intf_delete(netif_index);
			}
			break;
		case PORT_NOTIFIER_L3DELETE:
			if(NPD_TRUE == npd_intf_gindex_exist_check(netif_index, &l3index))
			{
				npd_intf_del_all_ip_addr(l3index);
			}
		default:
			break;
	}
}

void npd_intf_notify_relate_event
(
	unsigned int farther_netif_index, 
	unsigned int son_netif_index, 
	enum PORT_RELATE_ENT evt,
	char *private, int len
)
{
	unsigned int l3_ifindex = 0;
	if(npd_intf_exist_check(farther_netif_index, &l3_ifindex) == NPD_FALSE)
	{
		return;
	}
	switch(evt)
	{
		case PORT_NOTIFIER_JOIN:
			if(npd_netif_type_get(farther_netif_index) == NPD_NETIF_VLAN_TYPE)
			{
				unsigned short vlan_id = 0;
				vlan_id = npd_netif_vlan_get_vid(farther_netif_index);
				if(npd_netif_type_get(son_netif_index) == NPD_NETIF_TRUNK_TYPE)
				{
					unsigned int trunk_id = 0;
					trunk_id = npd_netif_trunk_get_tid(son_netif_index);
					npd_intf_vlan_add_trunk_hw_handler(vlan_id, trunk_id);
				}
				else if(npd_netif_type_get(son_netif_index) == NPD_NETIF_ETH_TYPE)
				{
					npd_intf_vlan_add_eth_hw_handler(vlan_id, son_netif_index);
				}
				else
				{
					return;
				}
			}
			else if(npd_netif_type_get(farther_netif_index) == NPD_NETIF_TRUNK_TYPE)
			{
				unsigned int trunk_id = 0;
				trunk_id = npd_netif_trunk_get_tid(farther_netif_index);
				if(npd_netif_type_get(son_netif_index) == NPD_NETIF_ETH_TYPE)
				{
					npd_intf_trunk_add_eth_hw_handler(trunk_id, son_netif_index);
				}
				else
				{
					return;
				}
			}
			break;
		case PORT_NOTIFIER_LEAVE:
			if(npd_netif_type_get(farther_netif_index) == NPD_NETIF_VLAN_TYPE)
			{
				unsigned short vlan_id = 0;
				vlan_id = npd_netif_vlan_get_vid(farther_netif_index);
				if(npd_netif_type_get(son_netif_index) == NPD_NETIF_TRUNK_TYPE)
				{
					unsigned int trunk_id = 0;
					trunk_id = npd_netif_trunk_get_tid(son_netif_index);
					npd_intf_vlan_del_trunk_hw_handler(vlan_id, trunk_id);
				}
				else if(npd_netif_type_get(son_netif_index) == NPD_NETIF_ETH_TYPE)
				{
					npd_intf_vlan_del_eth_hw_handler(vlan_id, son_netif_index);
				}
				else
				{
					return;
				}
			}
			else if(npd_netif_type_get(farther_netif_index) == NPD_NETIF_TRUNK_TYPE)
			{
				unsigned int trunk_id = 0;
				trunk_id = npd_netif_trunk_get_tid(farther_netif_index);
				if(npd_netif_type_get(son_netif_index) == NPD_NETIF_ETH_TYPE)
				{
					npd_intf_trunk_del_eth_hw_handler(trunk_id, son_netif_index);
				}
				else
				{
					return;
				}
			}
			break;
		default:
			break;
	}
}

netif_event_notifier_t intf_netif_notifier =
{
    .netif_event_handle_f = &npd_intf_notify_event,
    .netif_relate_handle_f = &npd_intf_notify_relate_event
};

/**********************************************************************************
 *  npd_intf_update_vlan_info_to_advanced_routing_l3intf
 *
 *  DESCRIPTION:
 *      this routine update ports info to kernel dev
 *
 *  INPUT:
 *      vid -- vlan id
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_SUCCESS
 *
 *
 **********************************************************************************/
unsigned int npd_intf_update_vlan_info_to_advanced_routing_l3intf
(
    unsigned short vid
)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
    return ret;
}

int npd_intf_is_eth_mng_check(unsigned int netif_index)
{
	unsigned long netif_type = 0;
	int port_type = 0;
	
	netif_type = npd_netif_type_get(netif_index);
	if (NPD_NETIF_ETH_TYPE == netif_type)
	{
		int slot_index = eth_port_get_slot_by_ifindex(netif_index);
		int eth_local_index = eth_port_get_portno_by_ifindex(netif_index);
		if(PRODUCT_IS_BOX)
		{
		    port_type = npd_get_port_type(
	                         MODULE_TYPE_ON_SUBSLOT_INDEX(0, slot_index),
	                         ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
	                         );	
		}
        else
        {
		    port_type = npd_get_port_type(
	                         MODULE_TYPE_ON_SLOT_INDEX(slot_index),
	                         ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
	                         );	
        }
	}
	if ((NPD_NETIF_ETH_TYPE == netif_type) &&
		(ETH_MNG == port_type))
	{
		return NPD_TRUE;
	}	
	else
	{
		return NPD_FALSE;
	}
}


int npd_intf_exist_check(unsigned int netif_index, unsigned int *ifindex)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    NPD_L3LOCAL_IFINDEX l3local;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		syslog_ax_intf_err("Interface for netif 0x%x is not found.\r\n", netif_index);
		return NPD_FALSE;
	}

    l3local.netif_index = netif_index;
    result = dbtable_hash_search(l3local_netif_index, &l3local, NULL, &l3local);
    if(result != 0)
    {
		syslog_ax_intf_err("Interface for netif 0x%x is not found.\r\n", netif_index);
		return NPD_FALSE;
    }
	*ifindex = l3local.ifindex;
	syslog_ax_intf_event("Interface for netif 0x%x is found, l3 ifindex = %d.\r\n", netif_index, *ifindex);

	return NPD_TRUE;
}

int npd_intf_gindex_exist_check(unsigned int netif_index, unsigned int *ifindex)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;

	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		syslog_ax_intf_err("Interface for netif 0x%x is not found.\r\n", netif_index);
		return NPD_FALSE;
	}
	/* 

    l3local.netif_index = netif_index;
    result = dbtable_hash_search(l3local_netif_index, &l3local, NULL, &l3local);
    if(result != 0)
    {
		syslog_ax_intf_err("Interface for netif 0x%x is not found.\r\n", netif_index);
		return NPD_FALSE;
    }
	*/
	*ifindex = l3intf_ctrl.ifindex;
	syslog_ax_intf_event("Interface for netif 0x%x is found, l3 ifindex = %d.\r\n", netif_index, *ifindex);

	return NPD_TRUE;
}


int npd_intf_netif_get_by_name(unsigned int *netif_index, unsigned char *ifName)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    NPD_L3LOCAL_IFINDEX l3local = {0};

	if(INTERFACE_RETURN_CODE_SUCCESS != npd_intf_ifindex_get_by_ifname(ifName, &l3local.ifindex) )
	{
		syslog_ax_intf_err("Index for Interface %s is not found. \r\n", ifName);
		return NPD_FALSE;
	}
    result = dbtable_hash_search(l3local_ifindex, &l3local, NULL, &l3local);
    if(result != 0)
	{
		syslog_ax_intf_err("ifindex for Interface %s is not found.\r\n", ifName);
		return NPD_FALSE;
	}
    *netif_index = l3local.netif_index;
	syslog_ax_intf_event("Interface %s for netif 0x%x is found, l3 ifindex = %d.\r\n", ifName, *netif_index, l3intf_ctrl.ifindex);
    return NPD_TRUE;
}

int npd_intf_netif_get_by_ifindex(unsigned int ifindex, unsigned int *netif_index)
{
    unsigned int ret =0;
    NPD_L3INTERFACE_CTRL l3intf = {0};

    l3intf.ifindex = ifindex;
	ret = dbtable_hash_search(l3intf_ifindex, &l3intf, npd_l3_intf_cmp_ifindex, &l3intf);
	if(0 != ret)
	{
		syslog_ax_intf_err("ifindex for Interface %#x is not found.\r\n", ifindex);
		return NPD_FALSE;
	}
	*netif_index = l3intf.netif_index;
	syslog_ax_intf_event("Interface for ifindex = %d is found, netif 0x%x.\r\n", ifindex, *netif_index);
	return NPD_TRUE;
}

int npd_intf_attribute_set_by_ifindex(unsigned int ifindex, unsigned int attribute)
{
	unsigned int ret =0;
	NPD_L3INTERFACE_CTRL l3intf = {0};

	l3intf.ifindex = ifindex;
	ret = dbtable_hash_search(l3intf_ifindex, &l3intf, npd_l3_intf_cmp_ifindex, &l3intf);
	if(0 != ret)
	{
		syslog_ax_intf_err("ifindex for Interface %#x is not found.\r\n", ifindex);
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

	l3intf.attribute = attribute;
	ret  = dbtable_hash_update(l3intf_ifindex, &l3intf, &l3intf);
	
	return ret;
}

int npd_intf_attribute_get_by_ifindex(unsigned int ifindex, unsigned int *attribute)
{
	unsigned int ret =0;
	NPD_L3INTERFACE_CTRL l3intf = {0};

	l3intf.ifindex = ifindex;
	ret = dbtable_hash_search(l3intf_ifindex, &l3intf, npd_l3_intf_cmp_ifindex, &l3intf);
	if(0 != ret)
	{
		syslog_ax_intf_err("ifindex for Interface %#x is not found.\r\n", ifindex);
		return NPD_FALSE;
	}

	*attribute = l3intf.attribute;
	return NPD_TRUE;
}

int npd_intf_name_get_by_ifindex(unsigned int ifindex, char *ifname)
{
	unsigned int ret =0;
	NPD_L3INTERFACE_CTRL l3intf = {0};

	l3intf.ifindex = ifindex;
	ret = dbtable_hash_search(l3intf_ifindex, &l3intf, npd_l3_intf_cmp_ifindex, &l3intf);
	if(0 != ret)
	{
		syslog_ax_intf_err("ifindex for Interface %#x is not found.\r\n", ifindex);
		return NPD_FALSE;
	}

	strncpy(ifname, l3intf.ifname, sizeof(l3intf.ifname)-1);

	return NPD_TRUE;
}

int npd_intf_ip_get_by_netif(unsigned int *ipAddr, unsigned int netif_index)
{
    unsigned int result = 0;
	NPD_L3INTERFACE_CTRL l3intf;
	unsigned int ipAddress[MAX_IP_COUNT];
	l3intf.netif_index = netif_index;

	result= dbtable_hash_search(l3intf_netif_index, &l3intf, NULL, &l3intf);
	if(result != 0)
	{
		syslog_ax_intf_err("ifindex for Interface 0x%x is not found.\r\n", netif_index);
		return NPD_FALSE;
	}

	npd_intf_addr_ip_get(l3intf.ifindex, ipAddress, NULL);

	*ipAddr = ipAddress[0];
	syslog_ax_intf_event("Interface for netif 0x%x is found, ipaddress = 0x%x.\r\n", netif_index, ipAddress[0]);
    return NPD_TRUE;
}

int npd_intf_netif_get_by_ip(unsigned int *ifindex, unsigned int ipAddr)
{
	unsigned int intfcnt = 1;
    if( NPD_TRUE == npd_intf_addr_ifindex_get(ifindex, &intfcnt, ipAddr) )
    {
    	syslog_ax_intf_dbg("npd intf get ifindex %#x by IP address %#x\n", *ifindex, ipAddr);
		return NPD_TRUE;
    }
	
	syslog_ax_intf_dbg("npd intf cannot get ifindex by IP address %#x\n", ipAddr);
	return NPD_FALSE;
}

int npd_intf_addr_ifindex_get_bynet( unsigned int *ifindex, unsigned int *intfCnt, unsigned int ipAddr)
{
	int ret = 0, i = 0;
	NPD_L3INTF_ADDR intfAddr;
	intfAddr.ipAddr = ipAddr;
	int mask_len = 0;

	ret = dbtable_hash_head(l3intf_addr_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_net);
	while(0 == ret)
	{
		int temp_len;
		if((ipAddr&intfAddr.mask) == (intfAddr.ipAddr&intfAddr.mask))
		{
    		lib_get_masklen_from_mask(intfAddr.mask, &temp_len);
    		if(temp_len > mask_len)
    		{
    			i = 0;
        		ifindex[i] = intfAddr.ifindex;
				mask_len = temp_len;				
    			if(++i >= *intfCnt)
        			break;
    		}
    		if(temp_len == mask_len)
    		{
        		ifindex[i] = intfAddr.ifindex;
    			if(++i >= *intfCnt)
        			break;
    		}
		}
		ret = dbtable_hash_next(l3intf_addr_index,&intfAddr, &intfAddr, NULL);
	}

	if( i > 0 )
	{
		*intfCnt = i;
		return NPD_TRUE;
	}

	return NPD_FALSE;
}

int npd_intf_addr_ifindex_get( unsigned int *ifindex, unsigned int *intfCnt, unsigned int ipAddr)
{
	int ret = 0, i = 0;
	NPD_L3INTF_ADDR intfAddr;
	intfAddr.ipAddr = ipAddr;

	ret = dbtable_hash_head_key(l3intf_addr_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_ip);
	while(0 == ret)
	{
		ifindex[i] = intfAddr.ifindex;
		if(++i >= *intfCnt)
			break;
		ret = dbtable_hash_next_key(l3intf_addr_index,&intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_ip);
	}

	if( i > 0 )
	{
		*intfCnt = i;
		return NPD_TRUE;
	}

	return NPD_FALSE;
}

int npd_intf_addr_ip_get( unsigned int ifindex, unsigned int *ipAddr, unsigned int *mask)
{
	int i = 0, ret = 0;
	NPD_L3INTF_ADDR intfAddr;

	intfAddr.ifindex = ifindex;

	ret = dbtable_hash_head_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index);
	while( 0 == ret)
	{
		if(ipAddr)
			ipAddr[i] = intfAddr.ipAddr;
		if(mask)
			mask[i] = intfAddr.mask;
		if(++i >= MAX_IP_COUNT )
			break;			
		ret = dbtable_hash_next_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index);
	}

	if(i>0)
	{
		return NPD_TRUE;
	}
	
	return NPD_FALSE;
}


int npd_intf_addr_ip_get_local( unsigned int ifindex, unsigned int *ipAddr, unsigned int *mask)
{
	unsigned int l3Index = 0;

	if(TRUE != npd_intf_get_global_l3index(ifindex, &l3Index))
	{
		return NPD_FALSE;
	}
	
	return npd_intf_addr_ip_get(l3Index, ipAddr, mask);
}

/**********************************************************************************
 *  npd_intf_port_check
 *
 *  check port  wether has l3 interface or pvid has l3 interface.
 *
 *  INPUT:
 *      vid - vlan id
 *      eth_g_index - port global index
 *
 *  OUTPUT:
 *
 *      ifindex - l3 interface index
 *
 *  RETURN:
 *      COMMON_RETURN_CODE_NULL_PTR  - null pointer error
 *      TRUE - find  the vlan has port l3 interface
 *      FALSE - don't find
 *
 **********************************************************************************/
int npd_intf_port_check
(
    unsigned short vid,
    unsigned int eth_g_index,
    unsigned int* ifindex
)
{
    int ret = 0;
    if (NULL == ifindex) {
        return COMMON_RETURN_CODE_NULL_PTR;
    }
    ret = npd_intf_exist_check(eth_g_index, ifindex);
    if ((NPD_PORT_L3INTF_VLAN_ID == vid) && (TRUE == ret) && (*ifindex != (~0UI))) {
        return TRUE;
    }

    ret = npd_vlan_interface_check(vid, ifindex);
    if ((TRUE == ret) && (*ifindex != (~0UI))) {
        return TRUE;
    }
    return FALSE;
}

unsigned int npd_vlan_interface_check(unsigned short vid, unsigned int *ifindex)
{
	unsigned long netif_vlanIndex = 0;
	
	netif_vlanIndex = npd_netif_vlan_index(vid);
	if( netif_vlanIndex == 0)
		return FALSE;

	if( NPD_TRUE != npd_intf_exist_check( netif_vlanIndex, ifindex)){
		return FALSE;
	}

	return TRUE;	
}


/*************************************************
  * npd_intf_vid_get_by_ip
 *
 *DESCRIPTION:
 *    get ip addresses by netlink
 *
 *INPUT:
 *    ifName -- the interface name we want to get its ip
 *OUTPUT:
 *    ifIndex -- the interface ifIndex
 *    ipAddrs -- the pointer of ip addresses we got
 *    masks  -- the pointer of masks for ip address above
 *RETURN:
 *    INTERFACE_RETURN_CODE_ERROR -- get ip addresses FAILED
 *    INTERFACE_RETURN_CODE_SUCCESS -- get ip address success
 *NOTE:
 *
 ************************************************/
int npd_intf_vid_get_by_ip
(
    unsigned int ipAddr, 
    unsigned int netif_index, 
    unsigned short *vid
)
{
	int ret = 0, i = 0, type = 0;
	unsigned char istagged = 0;
	unsigned short vlanId = 0;
    unsigned int l3index[MAX_L3INTF_NUMBER];
	unsigned int net_g_index = 0;
	unsigned int intfCount = MAX_L3INTF_NUMBER;

	ret = npd_intf_addr_ifindex_get_bynet(l3index, &intfCount, ipAddr);
	if( ret == NPD_FALSE )
	{
		return INTERFACE_RETURN_CODE_ERROR;
		
	}

	for(i=0;i<intfCount;i++)
	{
		ret = npd_intf_netif_get_by_ifindex(l3index[i], &net_g_index);
		if(NPD_TRUE == ret)
		{
			type = npd_netif_type_get(net_g_index);
			if(NPD_NETIF_VLAN_TYPE != type)
			{
				if(netif_index == net_g_index)
				{
					*vid = NPD_MAX_VLAN_ID;
				    return INTERFACE_RETURN_CODE_SUCCESS;
				}
			}
			else
			{
				vlanId = npd_netif_vlan_get_vid(net_g_index);
				ret = npd_vlan_check_contain_port(vlanId, netif_index, &istagged);
				if(NPD_TRUE == ret)
				{
					*vid = vlanId;
					return INTERFACE_RETURN_CODE_SUCCESS;
				}
			}
		}
	}
        
	return INTERFACE_RETURN_CODE_ERROR;
}

#ifdef HAVE_NPD_IPV6

int npd_ipv6_intf_addr_ifindex_get_bynet( unsigned int *ifindex, unsigned int *intfCnt, ip6_addr *ipAddr)
{
	int ret = 0, i = 0;
	NPD_V6_L3INTF_ADDR intfAddr;
	memcpy(&intfAddr.ipv6Addr, ipAddr, sizeof(intfAddr.ipv6Addr));
	

	ret = dbtable_hash_head(v6l3intf_addr_index, (void *)&intfAddr, (void *)&intfAddr, npd_ipv6_l3_intf_addr_filter_by_net);
	while(0 == ret)
	{
		ifindex[i] = intfAddr.ifindex;
		if(++i >= *intfCnt)
			break;
		ret = dbtable_hash_next(v6l3intf_addr_index, (void *)&intfAddr, (void *)&intfAddr, npd_ipv6_l3_intf_addr_filter_by_net);
	}

	if( i > 0 )
	{
		*intfCnt = i;
		return NPD_TRUE;
	}

	return NPD_FALSE;
}



int npd_ipv6_intf_addr_ip_get( unsigned int ifindex, ip6_addr *ipAddr, ip6_addr *mask)
{
	int i = 0, ret = 0;
	NPD_V6_L3INTF_ADDR intfAddr;

	intfAddr.ifindex = ifindex;

	ret = dbtable_hash_head_key(v6l3intf_addr_if_index, (void *)&intfAddr, (void *)&intfAddr, npd_v6_l3_intf_addr_filter_by_index);
	while( 0 == ret)
	{
		if(ipAddr)
			memcpy(&ipAddr[i], &intfAddr.ipv6Addr, sizeof(ipAddr[i]));
		if(mask)
			memcpy(&mask[i], &intfAddr.v6mask, sizeof(mask[i]));
		if(++i >= MAX_IP_COUNT )
			break;			
		ret = dbtable_hash_next_key(v6l3intf_addr_if_index, (void *)&intfAddr, (void *)&intfAddr, npd_v6_l3_intf_addr_filter_by_index);
	}

	if(i>0)
	{
		return NPD_TRUE;
	}
	
	return NPD_FALSE;
}




/*************************************************
  * npd_intf_vid_get_by_ipv6
 *
 *DESCRIPTION:
 *    get ip addresses by netlink
 *
 *INPUT:
 *    ifName -- the interface name we want to get its ip
 *OUTPUT:
 *    ifIndex -- the interface ifIndex
 *    ipAddrs -- the pointer of ip addresses we got
 *    masks  -- the pointer of masks for ip address above
 *RETURN:
 *    INTERFACE_RETURN_CODE_ERROR -- get ip addresses FAILED
 *    INTERFACE_RETURN_CODE_SUCCESS -- get ip address success
 *NOTE:
 *
 ************************************************/
int npd_ipv6_intf_vid_get_by_ip
(
    ip6_addr *ipAddr, 
    unsigned int netif_index, 
    unsigned short *vid
)
{
	int ret = 0, i = 0, type = 0;
	unsigned char istagged = 0;
	unsigned short vlanId = 0;
    unsigned int l3index[MAX_L3INTF_NUMBER];
	unsigned int net_g_index = 0;
	unsigned int intfCount = MAX_L3INTF_NUMBER;


	ret = npd_ipv6_intf_addr_ifindex_get_bynet(l3index, &intfCount, ipAddr);
	if( ret == NPD_FALSE )
	{
		return INTERFACE_RETURN_CODE_ERROR;
		
	}

	for(i=0;i<intfCount;i++)
	{
		ret = npd_intf_netif_get_by_ifindex(l3index[i], &net_g_index);
		if(NPD_TRUE == ret)
		{
			type = npd_netif_type_get(net_g_index);
			if(NPD_NETIF_VLAN_TYPE != type)
			{
				if(netif_index == net_g_index)
				{
					*vid = NPD_MAX_VLAN_ID;
				    return INTERFACE_RETURN_CODE_SUCCESS;
				}
			}
			else
			{
				vlanId = npd_netif_vlan_get_vid(net_g_index);
				ret = npd_vlan_check_contain_port(vlanId, netif_index, &istagged);
				if(NPD_TRUE == ret)
				{
					*vid = vlanId;
					return INTERFACE_RETURN_CODE_SUCCESS;
				}
			}
		}
	}
        
	return INTERFACE_RETURN_CODE_ERROR;
}

#endif //HAVE_NPD_IPV6

/**********************************************************************************
 *  npd_intf_eth_create_hw_handler
 *
 *  create intf based on eth ports,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *      addr-mac address
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_eth_create_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("ETH create l3if: the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
    nam_intf_vlan_enable(ifindex, pvid, 1, addr);

	{
    	nam_static_fdb_entry_mac_set_for_l3(addr, pvid, CPU_PORT_VINDEX);
	}
	

    npd_intf_config_basemac(netif_index, addr);
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 1, addr);

	npd_intf_udp_bc_trap_enable(pvid, 1);

    return ret;
}

int npd_intf_eth_ele_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("ETH ele handler: the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
    npd_intf_config_basemac(netif_index, addr);
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 1, addr);

	nam_igmp_packet_trap_cpu(netif_index, 1);

    return ret;
}

/**********************************************************************************
 *  npd_intf_eth_delete_hw_handler
 *
 *  create intf based on eth ports,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_eth_delete_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 0, addr);
	nam_igmp_packet_trap_cpu(netif_index, 0);
    return ret;
}

int npd_intf_eth_disable_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 0, addr);
    return ret;
}

int npd_intf_eth_enable_hw_handler(unsigned int ifindex, unsigned int netif_index,unsigned short pvid,unsigned char* addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
	{
    	nam_static_fdb_entry_mac_set_for_l3(addr, pvid, CPU_PORT_VINDEX);
	}
    npd_intf_config_basemac(netif_index, addr);

    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 1, addr);
	
	npd_intf_udp_bc_trap_enable(pvid, 1);

    return ret;
}

/**********************************************************************************
 *  npd_intf_trunk_create_hw_handler
 *
 *  create intf based on trunk,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *      addr-mac address
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_trunk_create_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
	int i = 0;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int eth_count = 0;
	unsigned int eth_g_index[8];
	unsigned int tid = npd_netif_trunk_get_tid(netif_index);
    ret = npd_trunk_member_port_index_get_all(tid, eth_g_index, &eth_count);

    nam_intf_vlan_enable(ifindex, pvid, 1, addr);

    nam_static_fdb_entry_mac_set_for_l3(addr, pvid, CPU_PORT_VINDEX);

    npd_intf_config_basemac(netif_index, addr);
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 1, addr);

	npd_intf_udp_bc_trap_enable(pvid, 1);

	nam_asic_vlan_entry_cpu_add(pvid);

	for(i = 0; i < eth_count; i++)
	{
		ret = npd_intf_eth_ele_hw_handler(eth_g_index[i], pvid, ifindex, addr);
		if(ret != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return ret;
		}
	}
    return ret;
}

int npd_intf_trunk_ele_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
	int i = 0;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int eth_count = 0;
	unsigned int eth_g_index[8];
	unsigned int tid = npd_netif_trunk_get_tid(netif_index);
    ret = npd_trunk_member_port_index_get_all(tid, eth_g_index, &eth_count);

	for(i = 0; i < eth_count; i++)
	{
		ret = npd_intf_eth_ele_hw_handler(eth_g_index[i], pvid, ifindex, addr);
		if(ret != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return ret;
		}
	}
    return ret;
}

/**********************************************************************************
 *  npd_intf_trunk_delete_hw_handler
 *
 *  create intf based on eth ports,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_trunk_delete_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
	int i = 0;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int eth_count = 0;
	unsigned int eth_g_index[8];
	unsigned int tid = npd_netif_trunk_get_tid(netif_index);
    ret = npd_trunk_member_port_index_get_all(tid, eth_g_index, &eth_count);
	if(ret != NPD_TRUE)
	{
		return ret;
	}
	for(i = 0; i < eth_count; i++)
	{
		ret = npd_intf_eth_delete_hw_handler(eth_g_index[i], pvid, ifindex, addr);
		if(ret != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return ret;
		}
	}
	
    nam_intf_vlan_port_enable(ifindex, pvid, netif_index, 0, addr);
    return ret;
}

/**********************************************************************************
 *  npd_intf_trunk_disable_hw_handler
 *
 *  disable intf based on eth ports,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_trunk_disable_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
	int i = 0;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int eth_count = 0;
	unsigned int eth_g_index[8];
	unsigned int tid = npd_netif_trunk_get_tid(netif_index);
    ret = npd_trunk_member_port_index_get_all(tid, eth_g_index, &eth_count);
	if(ret != NPD_TRUE)
	{
		return ret;
	}
	for(i = 0; i < eth_count; i++)
	{
		ret = npd_intf_eth_disable_hw_handler(eth_g_index[i], pvid, ifindex, addr);
		if(ret != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return ret;
		}
	}
    return ret;
}

int npd_intf_trunk_enable_hw_handler(unsigned int ifindex, unsigned int netif_index, unsigned short pvid, unsigned char* addr)
{
	int i = 0;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int eth_count = 0;
	unsigned int eth_g_index[8];
	unsigned int tid = npd_netif_trunk_get_tid(netif_index);
    ret = npd_trunk_member_port_index_get_all(tid, eth_g_index, &eth_count);

	for(i = 0; i < eth_count; i++)
	{
		ret = npd_intf_eth_enable_hw_handler(ifindex, eth_g_index[i], pvid, addr);
		if(ret != INTERFACE_RETURN_CODE_SUCCESS)
		{
			return ret;
		}
	}
    return ret;
}

/**********************************************************************************
 *  npd_intf_vlan_create_hw_handler
 *
 *  create intf based on vlan,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *      addr-mac address
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_vlan_create_hw_handler(unsigned int vlanif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int netif_index;
	unsigned int vid = npd_netif_vlan_get_vid(vlanif_index);
    npd_pbmp_t bmp;
    unsigned switch_port;
    struct vlan_s vlan = {0};
	int fw_slot_id = 0;
	char intfname[32];
	
	if(vid != pvid)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}

    vlan.vid = vid;
    ret = dbtable_sequence_search(g_vlans, vid, &vlan);
    if(0 != ret)
        return INTERFACE_RETURN_CODE_ERROR;
	ret = INTERFACE_RETURN_CODE_SUCCESS;

    nam_intf_vlan_enable(ifindex, pvid, 1, addr);
	
    nam_static_fdb_entry_mac_set_for_l3(addr, pvid, CPU_PORT_VINDEX);

	/* when vid bind the fw board, it replace the sysmac to the   */

    NPD_PBMP_ASSIGN(bmp, vlan.untag_ports);
    NPD_PBMP_OR(bmp, vlan.tag_ports);
    NPD_PBMP_ITER(bmp, switch_port)
    {
        npd_switch_port_netif_index(switch_port, &netif_index);
        if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
		    ret = npd_intf_eth_ele_hw_handler(netif_index, pvid, ifindex, addr);
        else if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
		    ret = npd_intf_trunk_ele_hw_handler(netif_index, pvid, ifindex, addr);
        else
            continue;
    }
	
    npd_intf_udp_bc_trap_enable(pvid, 1);
	nam_asic_vlan_entry_cpu_add(pvid);
    
    return ret;
}

/**********************************************************************************
 *  npd_intf_vlan_delete_hw_handler
 *
 *  create intf based on vlan,support l3 transmit.
 *
 *  INPUT:
 *      netif_index - l2 netif index
 *      pvid-- vid
 *      ifindex -l3 interface index
 *      addr-mac address
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      COMMON_RETURN_CODE_NULL_PTR
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_NAM_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *      INTERFACE_RETURN_CODE_FD_ERROR
 *      INTERFACE_RETURN_CODE_IOCTL_ERROR
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_intf_vlan_delete_hw_handler(unsigned int vlanif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int netif_index;
	unsigned int vid = npd_netif_vlan_get_vid(vlanif_index);
    npd_pbmp_t bmp;
    struct vlan_s vlan = {0};
    unsigned int switch_port;
    
	if(vid != pvid)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}
    ret = nam_no_static_fdb_entry_mac_vlan_set(addr, pvid, vlanif_index);

    if (NPD_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("set static mac addr ERROR ret %d\n", ret);
        return INTERFACE_RETURN_CODE_FDB_SET_ERROR;
    }

    ret = nam_intf_vlan_enable(ifindex, pvid, 0, addr);

    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_err("l3 intf on vlan enable error %d\n", ret);
        return ret;
    }

	ret = npd_intf_udp_bc_trap_enable(pvid, 0);
    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("npd_udp_bc_trap_enable: return val %d error.", ret);
        return ret;
    }

    vlan.vid = vid;
    ret = dbtable_sequence_search(g_vlans, vid, &vlan);
    if(0 != ret)
        return INTERFACE_RETURN_CODE_ERROR;
	ret = INTERFACE_RETURN_CODE_SUCCESS;
    NPD_PBMP_ASSIGN(bmp, vlan.untag_ports);
    NPD_PBMP_OR(bmp, vlan.tag_ports);
    NPD_PBMP_ITER(bmp, switch_port)
    {
        npd_switch_port_netif_index(switch_port, &netif_index);
        if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
		    ret = npd_intf_eth_delete_hw_handler(netif_index, pvid, ifindex, addr);
        else if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
		    ret = npd_intf_trunk_delete_hw_handler(netif_index, pvid, ifindex, addr);
        else
            continue;
    }
    
	nam_asic_vlan_entry_cpu_del(vid);/*delete cpu port from vlan*/
    return ret;
}

int npd_intf_vlan_add_eth_hw_handler(unsigned short vid, unsigned int eth_g_index)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int netif_index = 0;
	
	netif_index = npd_netif_vlan_get_index(vid);
	l3intf_ctrl.netif_index = netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on VID: %d\r\n", vid);
		return ret;
	}
	return npd_intf_eth_create_hw_handler(eth_g_index, vid, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
}
int npd_intf_vlan_del_eth_hw_handler(unsigned short vid, unsigned int eth_g_index)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int netif_index = 0;
	
	netif_index = npd_netif_vlan_get_index(vid);
	l3intf_ctrl.netif_index = netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on VID: %d\r\n", vid);
		return ret;
	}
	return npd_intf_eth_delete_hw_handler(eth_g_index, vid, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
}

int npd_intf_vlan_add_trunk_hw_handler(unsigned short vid, unsigned int tid)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int vlan_netif_index = 0;
	unsigned int trunk_netif_index = 0;
	
	vlan_netif_index = npd_netif_vlan_get_index(vid);
	trunk_netif_index = npd_netif_trunk_get_index(tid);
	l3intf_ctrl.netif_index = vlan_netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on VID: %d\r\n", vid);
		return ret;
	}
	return npd_intf_trunk_create_hw_handler(trunk_netif_index, vid, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
}
int npd_intf_vlan_del_trunk_hw_handler(unsigned short vid, unsigned int tid)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int vlan_netif_index = 0;
	unsigned int trunk_netif_index = 0;
	
	vlan_netif_index = npd_netif_vlan_get_index(vid);
	trunk_netif_index = npd_netif_trunk_get_index(tid);
	l3intf_ctrl.netif_index = vlan_netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on VID: %d\r\n", vid);
		return ret;
	}
	return npd_intf_trunk_delete_hw_handler(trunk_netif_index, vid, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
}

int npd_intf_trunk_add_eth_hw_handler(unsigned short tid, unsigned int eth_g_index)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int trunk_netif_index = 0;
	
	trunk_netif_index = npd_netif_trunk_get_index(tid);
	l3intf_ctrl.netif_index = trunk_netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on LAG: %d\r\n", tid);
		return ret;
	}

	ret = npd_intf_eth_create_hw_handler(eth_g_index, NPD_PORT_L3INTF_VLAN_ID, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
	if(ret != 0)
	{
        syslog_ax_intf_dbg("Enable eth port l3 mode failed.\r\n");
		return ret;
	}
	return ret;
}

int npd_intf_vlan_disable_hw_handler(unsigned int vlanif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int vid = npd_netif_vlan_get_vid(vlanif_index);
    
    ret = nam_no_static_fdb_entry_mac_vlan_set(addr, pvid, vlanif_index);

    if (NPD_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("set static mac addr ERROR ret %d\n", ret);
        return INTERFACE_RETURN_CODE_FDB_SET_ERROR;
    }

    /* 5 :CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E*/
	/* 0 : CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E */
    /* 2: CPSS_PACKET_CMD_TRAP_TO_CPU_E*/
	if(pvid != NPD_PORT_L3INTF_VLAN_ID)
    {
        nam_vlan_unreg_filter(0, pvid, 2, 0);
        nam_vlan_unreg_filter(0, pvid, 3, 0);
        nam_vlan_unreg_filter(0, pvid, 4, 0);
        nam_vlan_unreg_filter(0, pvid, 5, 0);
	}

    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("npd_trans_ports_on_vlan ERROR ret %d\n", ret);
        return ret;
    }
	
	ret = npd_intf_udp_bc_trap_enable(pvid, 0);
    if (INTERFACE_RETURN_CODE_SUCCESS != ret)
    {
        syslog_ax_intf_dbg("npd_udp_bc_trap_enable: return val %d error.", ret);
        return ret;
    }
    
	nam_asic_vlan_entry_cpu_del(vid);/*delete cpu port from vlan*/
    return ret;
}

int npd_intf_vlan_enable_hw_handler(unsigned int vlanif_index, unsigned int ifindex, unsigned short pvid, unsigned char* addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int vid = npd_netif_vlan_get_vid(vlanif_index);
	if(vid != pvid)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}
	{
    	nam_static_fdb_entry_mac_set_for_l3(addr, pvid, CPU_PORT_VINDEX);
	}
    if(pvid != NPD_PORT_L3INTF_VLAN_ID)
    {
        nam_vlan_unreg_filter(0, pvid, 2, 1);/*non-IPv4 BC*/
        nam_vlan_unreg_filter(0, pvid, 3, 1);/*IPv4 BC*/
        nam_vlan_unreg_filter(0, pvid, 4, 1);/*non-IPv4 BC*/
        nam_vlan_unreg_filter(0, pvid, 5, 1);/*IPv4 BC*/
    }
    
    npd_intf_udp_bc_trap_enable(pvid, 1);
	nam_asic_vlan_entry_cpu_add(pvid);
    
    return ret;
}

int npd_intf_trunk_del_eth_hw_handler(unsigned short tid, unsigned int eth_g_index)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	unsigned int trunk_netif_index = 0;
	
	trunk_netif_index = npd_netif_trunk_get_index(tid);
	l3intf_ctrl.netif_index = trunk_netif_index;
	
	ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Can not find l3 interface based on LAG: %d\r\n", tid);
		return ret;
	}

	ret = npd_intf_eth_delete_hw_handler(eth_g_index, NPD_PORT_L3INTF_VLAN_ID, l3intf_ctrl.ifindex, l3intf_ctrl.mac_addr);
	if(ret != 0)
	{
		syslog_ax_intf_dbg("Disable eth port l3 mode failed.\r\n");
		return ret;
	}

	return ret;
}

#ifdef HAVE_M4_TUNNEL
int npd_intf_ip_tunnel_create_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
	
    if (FALSE == npd_intf_l3_mac_legality_check(addr))
    {
        syslog_ax_intf_err("Tunnel create l3if: the interface mac is illegal\n");
        return INTERFACE_RETURN_CODE_CHECK_MAC_ERROR;
    }
	
    nam_intf_vlan_enable(ifindex, pvid, 1, addr);

    return ret;
}

int npd_intf_ip_tunnel_disable_hw_handler(unsigned int netif_index, unsigned short pvid, unsigned int ifindex, unsigned char *addr)
{
    return 0;
}
#endif

/**********************************************************************************
 *  npd_intf_create_vlan_l3intf
 *
 *  DESCRIPTION:
 *      this routine create vlan virtual dev and set static FDB
 *
 *  INPUT:
 *      l3intf_ctrl --NPD_L3INTERFACE_CTRL
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      INTERFACE_RETURN_CODE_ALREADY_ADVANCED
 *      INTERFACE_RETURN_CODE_VLAN_NOTEXIST
 *      INTERFACE_RETURN_CODE_INTERFACE_EXIST
 *
 **********************************************************************************/
int npd_intf_create_l3intf
(
    NPD_L3INTERFACE_CTRL *l3intf_ctrl
)
{
	port_driver_t *netif_driver = NULL;
    int ret = 0;
	unsigned long netif_type = 0;
    NPD_L3LOCAL_IFINDEX l3local = {0};

	netif_type = npd_netif_type_get(l3intf_ctrl->netif_index);
    l3local.netif_index = l3intf_ctrl->netif_index;
    
	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
		case NPD_NETIF_TRUNK_TYPE:
			netif_driver = port_driver_get(l3intf_ctrl->netif_index);
			if(netif_driver == NULL)
			{
				npd_syslog_dbg("Get driver type for 0x%x failed.\r\n", l3intf_ctrl->netif_index);
				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
			}
			if(netif_driver->allow_vlan == NULL)
			{
				npd_syslog_dbg("No driver func.\r\n");
				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
			}
			ret = (*netif_driver->allow_vlan)(l3intf_ctrl->netif_index, NPD_PORT_L3INTF_VLAN_ID, FALSE);
			if(ret != VLAN_RETURN_CODE_ERR_NONE)
			{
				npd_syslog_dbg("Add netif 0x%x to l3intf vlan failed, error code: %d.\r\n", l3intf_ctrl->netif_index, ret);
				return INTERFACE_RETURN_CODE_ERROR;
			}
            ret =  (*netif_driver->set_pvid)(l3intf_ctrl->netif_index, NPD_PORT_L3INTF_VLAN_ID);
    		if (0 != ret)
    		{
    			npd_syslog_dbg("Set l3intf pvid (%d) for netif 0x%x failed.\r\n", NPD_PORT_L3INTF_VLAN_ID, l3intf_ctrl->netif_index);
				return INTERFACE_RETURN_CODE_ERROR;
    		}
			break;
		case NPD_NETIF_VLAN_TYPE:
			break;
#ifdef HAVE_M4_TUNNEL
        case NPD_NETIF_TUNNEL_TYPE:
        {
            break;
        }
#endif
		default:
			return INTERFACE_RETURN_CODE_ERROR;
	}
	
	ret = nam_intf_tbl_index_get(l3intf_ctrl->ifindex);
	if( ret != 0 )
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}

    if (NPD_NETIF_TUNNEL_TYPE != netif_type)
    {
        ret = npd_intf_create_by_netif_index(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->state, &l3local.ifindex, l3intf_ctrl->ifname, l3intf_ctrl->mac_addr);
    }
#ifdef HAVE_M4_TUNNEL
    else
    {
        ret = npd_tnl_create_by_netif_index(l3intf_ctrl->netif_index, l3intf_ctrl->mac_addr, &l3local.ifindex);
    }
#endif

    dbtable_hash_insert(l3local_ifindex, &l3local);	

	syslog_ax_intf_dbg("npd_intf_create_l3intf: intf index is %#x, local is %#x\r\n", l3intf_ctrl->ifindex, l3local.ifindex);

    if (INTERFACE_RETURN_CODE_SUCCESS == ret)
    {
    	switch(netif_type)
    	{
    		case NPD_NETIF_ETH_TYPE:
    			ret = npd_intf_eth_create_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
    		case NPD_NETIF_TRUNK_TYPE:
    			ret = npd_intf_trunk_create_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
    		case NPD_NETIF_VLAN_TYPE:
    			ret = npd_intf_vlan_create_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
#ifdef HAVE_M4_TUNNEL
            case NPD_NETIF_TUNNEL_TYPE:
            {
                ret = npd_intf_ip_tunnel_create_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
                break;
            }
#endif
    		default:
    			return INTERFACE_RETURN_CODE_ERROR;
    	}
    }
	
	if(ret != INTERFACE_RETURN_CODE_SUCCESS)
	{
		ret = INTERFACE_RETURN_CODE_ERROR;
	}
	else
	{
        char mac[6] = {0};
        if (NPD_NETIF_TUNNEL_TYPE != netif_type)
        {
            if(memcmp(mac, l3intf_ctrl->mac_addr, 6))
            {
                npd_intf_set_mac_address(l3local.ifindex, l3intf_ctrl->mac_addr);
            }
            if(l3intf_ctrl->ipv4 != 0 && l3intf_ctrl->mask != 0)
        	{
        		npd_intf_set_ip_addr_kap_handler(l3local.ifindex, l3intf_ctrl->ipv4, l3intf_ctrl->mask);
        	}
            if(l3intf_ctrl->state != 0)
            {
                npd_intf_set_status_kap_handler(l3local.ifindex, l3intf_ctrl->state);
            }
        }
		if(l3intf_ctrl->ctrl_state == INTF_CTRL_STATE_DOWN)
		{
			switch(netif_type)
	    	{
	    		case NPD_NETIF_ETH_TYPE:
	    			ret = npd_intf_eth_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
	    		case NPD_NETIF_TRUNK_TYPE:
	    			ret = npd_intf_trunk_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
	    		case NPD_NETIF_VLAN_TYPE:
	    			ret = npd_intf_vlan_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
#ifdef HAVE_M4_TUNNEL
                case NPD_NETIF_TUNNEL_TYPE:
                {
                    break;
                }
#endif
	    		default:
	    			return INTERFACE_RETURN_CODE_ERROR;
	    	}
		}

        if (NPD_NETIF_TUNNEL_TYPE != netif_type)
        {
    		if(l3intf_ctrl->proxy_arp == TRUE)
    		{
    			npd_intf_set_proxy_arp_handle((unsigned char *)l3intf_ctrl->ifname, l3intf_ctrl->proxy_arp);
    		}
        }
#ifdef HAVE_ROUTE        
    	if((l3intf_ctrl->attribute&NPD_INTF_ATTR_URPF_STRICT) == NPD_INTF_ATTR_URPF_STRICT 
			|| (l3intf_ctrl->attribute&NPD_INTF_ATTR_URPF_LOOSE) == NPD_INTF_ATTR_URPF_LOOSE
			|| (l3intf_ctrl->bind)
			)
    	{
    	    ret = nam_route_set_ucrpf_enable(l3intf_ctrl->ifindex, 1);
    	}
    	else
    	{
    		ret = nam_route_set_ucrpf_enable(l3intf_ctrl->ifindex, 0);
    	}
		if(l3intf_ctrl->ipmc == TRUE)
		{
			ret = nam_intf_ipmc_enable(l3intf_ctrl->ifindex, l3intf_ctrl->vid, 1);
		}
#endif        
#ifdef HAVE_CAPWAP_ENGINE
		npd_capwap_keep_capwap_port_vlan_relation(l3intf_ctrl->vid,0);
#endif		
	}
    return ret;
}


/**********************************************************************************
 *  npd_intf_delete_l3intf
 *
 *  DESCRIPTION:
 *      this routine delete vlan l3intf
 *
 *  INPUT:
 *      l3intf_ctrl -- NPD_L3INTERFACE_CTRL
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR
 *      INTERFACE_RETURN_CODE_VLAN_NOTEXIST
 *      INTERFACE_RETURN_CODE_SUCCESS
 *
 *
 **********************************************************************************/
int npd_intf_delete_l3intf
(
    NPD_L3INTERFACE_CTRL *l3intf_ctrl
)
{
	port_driver_t *netif_driver = NULL;
    unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned long netif_type = 0;
    NPD_L3LOCAL_IFINDEX l3local;

    l3local.netif_index = l3intf_ctrl->netif_index;
    dbtable_hash_search(l3local_netif_index, &l3local, NULL, &l3local);
    
	netif_type = npd_netif_type_get(l3intf_ctrl->netif_index);
	
    npd_syslog_dbg("del l3_intf:%d, netif index :: %x, vid:%d \n", l3intf_ctrl->ifindex, l3intf_ctrl->netif_index, l3intf_ctrl->vid);

	ret = nam_intf_tbl_index_free(l3intf_ctrl->ifindex);
	if( ret != INTERFACE_RETURN_CODE_SUCCESS)
	{
		npd_syslog_dbg("del l3_intf:free %d fail \r\n", l3intf_ctrl->ifindex);
	}
    
    if (NPD_NETIF_TUNNEL_TYPE != netif_type)
    {
        ret = npd_intf_del_by_ifindex(l3local.ifindex);
    }
#ifdef HAVE_M4_TUNNEL
    else
    {
        npd_tnl_delete_by_netif_index(l3intf_ctrl->netif_index);
    }
#endif

    if (INTERFACE_RETURN_CODE_SUCCESS == ret)
    {
        //npd_arp_snooping_del_by_L3index(l3intf_ctrl->ifindex);

        if (NPD_NETIF_TUNNEL_TYPE != netif_type)
        {
		    npd_intf_set_proxy_arp_handle((unsigned char *)l3intf_ctrl->ifname, 0);
        }
#ifdef HAVE_ROUTE        
   	    ret = nam_route_set_ucrpf_enable(l3intf_ctrl->ifindex, 0);
    	ret = nam_intf_ipmc_enable(l3intf_ctrl->ifindex, l3intf_ctrl->vid, 0);
#endif
#ifdef HAVE_CAPWAP_ENGINE
		npd_capwap_keep_capwap_port_vlan_relation(l3intf_ctrl->vid,1);
#endif

    	switch(netif_type)
    	{
    		case NPD_NETIF_ETH_TYPE:
    			netif_driver = port_driver_get(l3intf_ctrl->netif_index);
    			if(netif_driver == NULL)
    			{
    				npd_syslog_dbg("Get eth port (0x%x) driver type failed.\r\n", l3intf_ctrl->netif_index);
    				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
    			}
    			if(netif_driver->remove_vlan == NULL)
    			{
    				npd_syslog_dbg("No driver func.\r\n");
    				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
    			}
    			ret = (*netif_driver->remove_vlan)(l3intf_ctrl->netif_index, NPD_PORT_L3INTF_VLAN_ID, FALSE);
    			if(ret != VLAN_RETURN_CODE_ERR_NONE)
    			{
    				npd_syslog_dbg("Remove port from 4095 vlan failed, error code: %d.\r\n", ret);
    				return INTERFACE_RETURN_CODE_ERROR;
    			}
    			npd_intf_eth_delete_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
    		case NPD_NETIF_TRUNK_TYPE:
    			netif_driver = port_driver_get(l3intf_ctrl->netif_index);
    			if(netif_driver == NULL)
    			{
    				npd_syslog_dbg("Get eth port (0x%x) driver type failed.\r\n", l3intf_ctrl->netif_index);
    				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
    			}
    			if(netif_driver->remove_vlan == NULL)
    			{
    				npd_syslog_dbg("No driver func.\r\n");
    				return INTERFACE_RETURN_CODE_NO_SUCH_PORT;
    			}
    			ret = (*netif_driver->remove_vlan)(l3intf_ctrl->netif_index, NPD_PORT_L3INTF_VLAN_ID, FALSE);
    			if(ret != VLAN_RETURN_CODE_ERR_NONE)
    			{
    				npd_syslog_dbg("Remove LAG from 4095 vlan failed, error code: %d.\r\n", ret);
    				return INTERFACE_RETURN_CODE_ERROR;
    			}
    			npd_intf_trunk_delete_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
    		case NPD_NETIF_VLAN_TYPE:
    			npd_intf_vlan_delete_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
    			break;
#ifdef HAVE_M4_TUNNEL
            case NPD_NETIF_TUNNEL_TYPE:
            {
                break;
            }
#endif
    		default:
    			return INTERFACE_RETURN_CODE_ERROR;
    	}
    }

    return ret;
}

/**********************************************************************************
 *  npd_intf_get_netif_mac
 *
 *  DESCRIPTION:
 *      this routine get specific mac address for route mode eth-port
 *      as default, L3 eth-port interface in route mode has same system mac, and
 *      value saved in sw is bcast address FF:FF:FF:FF:FF:FF
 *
 *  INPUT:
 *      netif_index  -  global eth-port index
 *
 *  OUTPUT:
 *      macAddr - mac address to set*
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR - if error occurred
 *      INTERFACE_RETURN_CODE_SUCCESS - if all ok
 *
 *
 **********************************************************************************/
unsigned int npd_intf_get_netif_mac
(
    unsigned int netif_index,
    unsigned char *macAddr
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	memcpy(macAddr, l3intf_ctrl.mac_addr, 6);
    return result;
}

unsigned int npd_intf_get_vid
(
    unsigned int local_ifindex,
    unsigned short *vid
)
{
    unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
    NPD_L3LOCAL_IFINDEX l3local;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;

    memset(&l3local, 0, sizeof(NPD_L3LOCAL_IFINDEX));
	l3local.ifindex = local_ifindex;
	ret = dbtable_hash_search(l3local_ifindex, &l3local, NULL, &l3local);
	if(0 == ret)
	{
		l3intf_ctrl.netif_index = l3local.netif_index;
		ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
		if(0 == ret)
		{
			*vid = l3intf_ctrl.vid;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

unsigned int npd_intf_set_mac_by_ifindex
(
    unsigned int ifindex,
    unsigned char *macAddr
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.ifindex = ifindex;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	memcpy(l3intf_ctrl.mac_addr, macAddr, 6);
	result = dbtable_hash_update(l3intf_ifindex, NULL, &l3intf_ctrl);
    return result;
}

/**********************************************************************************
 *  npd_intf_set_netif_mac
 *
 *  DESCRIPTION:
 *      this routine set specific mac address for route mode eth-port
 *      as default, L3 eth-port interface in route mode has same system mac, and
 *      value saved in sw is bcast address FF:FF:FF:FF:FF:FF
 *
 *  INPUT:
 *      eth_g_index  -  global eth-port index
 *      macAddr - mac address to set
 *
 *  OUTPUT:
 *
 *
 *  RETURN:
 *      INTERFACE_RETURN_CODE_ERROR - if error occurred
 *      INTERFACE_RETURN_CODE_SUCCESS - if all ok
 *
 *
 **********************************************************************************/
unsigned int npd_intf_set_netif_mac
(
    unsigned int netif_index,
    unsigned char *macAddr
)
{
    unsigned int result = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	l3intf_ctrl.netif_index = netif_index;
	result= dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
	memcpy(l3intf_ctrl.mac_addr, macAddr, 6);
	result = dbtable_hash_update(l3intf_netif_index, NULL, &l3intf_ctrl);
    return result;
}

long npd_l3_intf_db_create_handler(void *if_struct)
{
	long ret = 0;
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)if_struct;

	ret = npd_intf_create_l3intf(l3intf_ctrl);
 	return ret;
}

long npd_l3_intf_db_delete_handler(void *if_struct)
{
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)if_struct;
    NPD_L3LOCAL_IFINDEX l3local;
	
    l3local.netif_index = l3intf_ctrl->netif_index;

	npd_intf_delete_l3intf(l3intf_ctrl);
    dbtable_hash_delete(l3local_netif_index, &l3local, &l3local);
	return 0;
}

long npd_l3_intf_db_attr_update_handler(void *if_struct_new, void *if_struct)
{
	int ret = 0;
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)if_struct;
	NPD_L3INTERFACE_CTRL *l3intf_ctrl_new = (NPD_L3INTERFACE_CTRL *)if_struct_new;
    NPD_L3LOCAL_IFINDEX l3local;
	unsigned long netif_type = 0;

#ifdef HAVE_M4_TUNNEL
    if (NPD_NETIF_TUNNEL_TYPE == npd_netif_type_get(l3intf_ctrl_new->netif_index))
    {
        return 0;
    }
#endif
    l3local.netif_index = l3intf_ctrl_new->netif_index;

    ret = dbtable_hash_search(l3local_netif_index, &l3local, NULL, &l3local);
    if(ret != 0)
    {
		return -1;
    }
	if(memcmp(l3intf_ctrl->mac_addr, l3intf_ctrl_new->mac_addr, 6) != 0)
	{
		ret = npd_intf_set_mac_address(l3local.ifindex, l3intf_ctrl_new->mac_addr);
		if(ret != 0)
		{
			syslog_ax_intf_dbg("Set l3 interface mac address to kap failed.\r\n");
		}
	}
	if(l3intf_ctrl->state != l3intf_ctrl_new->state)
	{
		ret = npd_intf_set_status_kap_handler(l3local.ifindex, l3intf_ctrl_new->state);
		if(ret != 0)
		{
			syslog_ax_intf_dbg("Set l3 interface status to kap failed.\r\n");
		}

		ret = nam_intf_addr_link_change(l3intf_ctrl->ifname, l3intf_ctrl->netif_index, l3intf_ctrl_new->state);
		if(ret != 0)
		{
			syslog_ax_intf_dbg("Set l3 interface status to nam failed.\r\n");
		}
	}
	if(l3intf_ctrl->ipv4 != l3intf_ctrl_new->ipv4 || l3intf_ctrl->mask != l3intf_ctrl_new->mask)
	{
		ret = npd_intf_set_ip_addr_kap_handler(l3local.ifindex, l3intf_ctrl_new->ipv4, l3intf_ctrl_new->mask);
	}
	if(l3intf_ctrl->ctrl_state!= l3intf_ctrl_new->ctrl_state)
	{
		if(INTF_CTRL_STATE_DOWN == l3intf_ctrl_new->ctrl_state)
		{
			netif_type = npd_netif_type_get(l3intf_ctrl->netif_index);
	    	switch(netif_type)
	    	{
	    		case NPD_NETIF_ETH_TYPE:
	    			npd_intf_eth_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
	    		case NPD_NETIF_TRUNK_TYPE:
	    			npd_intf_trunk_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
	    		case NPD_NETIF_VLAN_TYPE:
	    			npd_intf_vlan_disable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->ifindex, l3intf_ctrl->mac_addr);
	    			break;
	    		default:
	    			break;
	    	}
		}
		else
		{
			netif_type = npd_netif_type_get(l3intf_ctrl->netif_index);
			switch(netif_type)
			{
				case NPD_NETIF_ETH_TYPE:
					npd_intf_eth_enable_hw_handler(l3intf_ctrl->ifindex, l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->mac_addr);
					break;
				case NPD_NETIF_TRUNK_TYPE:
					npd_intf_trunk_enable_hw_handler(l3intf_ctrl->ifindex, l3intf_ctrl->netif_index, l3intf_ctrl->vid, l3intf_ctrl->mac_addr);
					break;
				case NPD_NETIF_VLAN_TYPE:
					npd_intf_vlan_enable_hw_handler(l3intf_ctrl->netif_index, l3intf_ctrl->ifindex, l3intf_ctrl->vid, l3intf_ctrl->mac_addr);
					break;
	    		default:
	    			break;
			}
		}		
	}	
	if(l3intf_ctrl->proxy_arp!= l3intf_ctrl_new->proxy_arp)
	{
		npd_intf_set_proxy_arp_handle((unsigned char *)l3intf_ctrl_new->ifname, l3intf_ctrl_new->proxy_arp);
	}
#ifdef HAVE_ROUTE    
	if(l3intf_ctrl->attribute != l3intf_ctrl_new->attribute)
	{
		if((l3intf_ctrl_new->attribute&NPD_INTF_ATTR_URPF_STRICT) == NPD_INTF_ATTR_URPF_STRICT 
			|| (l3intf_ctrl_new->attribute&NPD_INTF_ATTR_URPF_LOOSE) == NPD_INTF_ATTR_URPF_LOOSE
			)
		{
		    nam_route_set_ucrpf_enable(l3intf_ctrl_new->ifindex, 1);
		}
		else
		{
			nam_route_set_ucrpf_enable(l3intf_ctrl_new->ifindex, 0);
		}
	}
	if(l3intf_ctrl->ipmc != l3intf_ctrl_new->ipmc)
	{
		nam_intf_ipmc_enable(l3intf_ctrl_new->ifindex, l3intf_ctrl_new->vid, l3intf_ctrl_new->ipmc);
	}
#endif    
#if 0    
    if (l3intf_ctrl->bind != l3intf_ctrl_new->bind)
    {
        nam_route_set_source_guard(l3intf_ctrl_new->ifindex, l3intf_ctrl->bind);
    }
#endif
	return ret;
}

unsigned int npd_l3_intf_db_netif_index(void *if_struct)
{
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)if_struct;
	unsigned int netif_index = l3intf_ctrl->netif_index;
	int hash_index = 0;
	hash_index = netif_index >> 28;
	hash_index = hash_index << 6;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
	return hash_index%MAX_L3INTF_NUMBER;
}

unsigned int npd_l3_local_netif_index(void *if_struct)
{
	NPD_L3LOCAL_IFINDEX *l3intf_ctrl = (NPD_L3LOCAL_IFINDEX *)if_struct;
	unsigned int netif_index = l3intf_ctrl->netif_index;
	int hash_index = 0;
	hash_index = netif_index >> 28;
	hash_index = hash_index << 6;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
	return hash_index%MAX_L3INTF_NUMBER;
}

unsigned int npd_l3_intf_cmp_netif_index(void *if_struct_a, void *if_struct_b)
{
	NPD_L3INTERFACE_CTRL *l3intf_ctrl_a = (NPD_L3INTERFACE_CTRL *)if_struct_a;
	NPD_L3INTERFACE_CTRL *l3intf_ctrl_b = (NPD_L3INTERFACE_CTRL *)if_struct_b;
	return (l3intf_ctrl_a->netif_index == l3intf_ctrl_b->netif_index);
}

unsigned int npd_l3_local_cmp_netif_index(void *if_struct_a, void *if_struct_b)
{
	NPD_L3LOCAL_IFINDEX *l3intf_ctrl_a = (NPD_L3LOCAL_IFINDEX *)if_struct_a;
	NPD_L3LOCAL_IFINDEX *l3intf_ctrl_b = (NPD_L3LOCAL_IFINDEX *)if_struct_b;
	return (l3intf_ctrl_a->netif_index == l3intf_ctrl_b->netif_index);
}

unsigned int npd_l3_intf_db_ifindex(void *if_struct)
{
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)if_struct;
	unsigned int ifindex = l3intf_ctrl->ifindex;
	return ifindex%MAX_L3INTF_NUMBER;
}

unsigned int npd_l3_local_db_ifindex(void *if_struct)
{
	NPD_L3LOCAL_IFINDEX *l3intf_ctrl = (NPD_L3LOCAL_IFINDEX *)if_struct;
	unsigned int ifindex = l3intf_ctrl->ifindex;
	return ifindex%MAX_L3INTF_NUMBER;
}

unsigned int npd_l3_intf_cmp_netif_and_ifindex(void *if_struct_a, void *if_struct_b)
{
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_a = (NPD_L3INTERFACE_CTRL *)if_struct_a;
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_b = (NPD_L3INTERFACE_CTRL *)if_struct_b;
    return ((l3intf_ctrl_a->ifindex == l3intf_ctrl_b->ifindex)&&(l3intf_ctrl_a->netif_index == l3intf_ctrl_b->netif_index));
}

unsigned int npd_l3_local_cmp_netif_and_ifindex(void *if_struct_a, void *if_struct_b)
{
    NPD_L3LOCAL_IFINDEX *l3intf_ctrl_a = (NPD_L3LOCAL_IFINDEX *)if_struct_a;
    NPD_L3LOCAL_IFINDEX *l3intf_ctrl_b = (NPD_L3LOCAL_IFINDEX *)if_struct_b;
    return ((l3intf_ctrl_a->ifindex == l3intf_ctrl_b->ifindex)&&(l3intf_ctrl_a->netif_index == l3intf_ctrl_b->netif_index));
}

unsigned int npd_l3_intf_cmp_ifindex(void *if_struct_a, void *if_struct_b)
{
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_a = (NPD_L3INTERFACE_CTRL *)if_struct_a;
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_b = (NPD_L3INTERFACE_CTRL *)if_struct_b;
    return (l3intf_ctrl_a->ifindex == l3intf_ctrl_b->ifindex);
}

unsigned int npd_l3_local_cmp_ifindex(void *if_struct_a, void *if_struct_b)
{
    NPD_L3LOCAL_IFINDEX *l3intf_ctrl_a = (NPD_L3LOCAL_IFINDEX *)if_struct_a;
    NPD_L3LOCAL_IFINDEX *l3intf_ctrl_b = (NPD_L3LOCAL_IFINDEX *)if_struct_b;
    return (l3intf_ctrl_a->ifindex == l3intf_ctrl_b->ifindex);
}

unsigned int npd_l3_intf_cmp_ipaddr(void *if_struct_a, void *if_struct_b)
{
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_a = (NPD_L3INTERFACE_CTRL *)if_struct_a;
    NPD_L3INTERFACE_CTRL *l3intf_ctrl_b = (NPD_L3INTERFACE_CTRL *)if_struct_b;
    return (l3intf_ctrl_a->ipv4 == l3intf_ctrl_b->ipv4);
}

unsigned int npd_l3_intf_addr_filter_by_net(	void *data1, void *data2)
{
	NPD_L3INTF_ADDR *itemA = (NPD_L3INTF_ADDR *)data1;	
	NPD_L3INTF_ADDR *itemB = (NPD_L3INTF_ADDR *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_intf_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return ( (itemA->ipAddr&itemB->mask) == (itemB->ipAddr&itemB->mask));
}

#ifdef HAVE_NPD_IPV6
	
unsigned int npd_ipv6_l3_intf_addr_filter_by_net(	void *data1, void *data2)
{
	NPD_V6_L3INTF_ADDR *itemA = (NPD_V6_L3INTF_ADDR *)data1;	
	NPD_V6_L3INTF_ADDR *itemB = (NPD_V6_L3INTF_ADDR *)data2;

	
	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_intf_err("npd intf route compare null pointers error.");
		return FALSE;
	}
	
	//lib_get_maskv6_from_masklen(itemB->v6mask, &v6_maskB);

#if 0
	return ( (itemA->ipv6Addr.u6_addr32[0]&v6_maskA.u6_addr32[0]) == (itemB->ipv6Addr.u6_addr32[0]&v6_maskB.u6_addr32[0])
			&& (itemA->ipv6Addr.u6_addr32[1]&v6_maskA.u6_addr32[1]) == (itemB->ipv6Addr.u6_addr32[1]&v6_maskB.u6_addr32[1]
			&& (itemA->ipv6Addr.u6_addr32[2]&v6_maskA.u6_addr32[2]) == (itemB->ipv6Addr.u6_addr32[2]&v6_maskB.u6_addr32[2]
			&& (itemA->ipv6Addr.u6_addr32[3]&v6_maskA.u6_addr32[3]) == (itemB->ipv6Addr.u6_addr32[3]&v6_maskB.u6_addr32[3]);
#endif //0
	return IPV6_NET_EQUAL(itemA->ipv6Addr, itemB->ipv6Addr, itemB->v6mask, itemB->v6mask);
}

unsigned int npd_v6_l3_intf_addr_filter_by_index
(
	void *data1,
	void *data2
)
{
	NPD_V6_L3INTF_ADDR *itemA = (NPD_V6_L3INTF_ADDR *)data1;	
	NPD_V6_L3INTF_ADDR *itemB = (NPD_V6_L3INTF_ADDR *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_intf_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return (itemA->ifindex == itemB->ifindex);
}


#endif //HAVE_NPD_IPV6
unsigned int npd_l3_intf_addr_filter_by_ip(	void *data1, void *data2)
{
	NPD_L3INTF_ADDR *itemA = (NPD_L3INTF_ADDR *)data1;	
	NPD_L3INTF_ADDR *itemB = (NPD_L3INTF_ADDR *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_intf_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return (itemA->ipAddr == itemB->ipAddr);
}

unsigned int npd_l3_intf_addr_filter_by_index
(
	void *data1,
	void *data2
)
{
	NPD_L3INTF_ADDR *itemA = (NPD_L3INTF_ADDR *)data1;	
	NPD_L3INTF_ADDR *itemB = (NPD_L3INTF_ADDR *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_intf_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return (itemA->ifindex == itemB->ifindex);
}

unsigned int npd_l3_intf_addr_compare
(
	void *data1,
	void *data2
)
{
	NPD_L3INTF_ADDR *itemA = (NPD_L3INTF_ADDR *)data1;	
	NPD_L3INTF_ADDR *itemB = (NPD_L3INTF_ADDR *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return ((itemA->ipAddr == itemB->ipAddr)&&(itemA->mask == itemB->mask)&&(itemA->ifindex == itemB->ifindex));
}


unsigned int npd_l3_intf_addr_key_gen
(
	void *data
)
{
	unsigned int key = 0;
	NPD_L3INTF_ADDR *item = (NPD_L3INTF_ADDR *)data;
	if(NULL == item) {
		return ~0UI;
	}

	key = jhash_1word(item->ipAddr, 0x35798642);
	
	key %= NPD_L3INTF_ADDR_HASH_SIZE;

	return key;
}

unsigned int npd_l3_intf_addr_if_key_gen
(
	void *data
)
{
	unsigned int key = 0;
	NPD_L3INTF_ADDR *item = (NPD_L3INTF_ADDR *)data;
	if(NULL == item) {
		return ~0UI;
	}

	key = item->ifindex;
	
	key %= MAX_L3INTF_NUMBER;

	return key;
}
#ifdef HAVE_NPD_IPV6 


unsigned int npd_v6_l3_intf_addr_compare
(
	void *data1,
	void *data2
)
{
	NPD_V6_L3INTF_ADDR *itemA = (NPD_V6_L3INTF_ADDR *)data1;	
	NPD_V6_L3INTF_ADDR *itemB = (NPD_V6_L3INTF_ADDR *)data2;
	
	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd intf route compare null pointers error.");
		return FALSE;
	}

	return (IPV6_NET_EQUAL(itemA->ipv6Addr, itemB->ipv6Addr, itemA->v6mask, itemB->v6mask) && (itemA->ifindex == itemB->ifindex));
}


unsigned int npd_v6_l3_intf_addr_key_gen
(
	void *data
)
{
	unsigned int key = 0;
	unsigned int tmp = 0;
	NPD_V6_L3INTF_ADDR *item = (NPD_V6_L3INTF_ADDR *)data;
	if(NULL == item) {
		return ~0UI;
	}
	tmp = item->ipv6Addr.u6_addr32[0] + item->ipv6Addr.u6_addr32[1] + item->ipv6Addr.u6_addr32[2]\
			+item->ipv6Addr.u6_addr32[3] ;

	key = jhash_1word(tmp, 0x35798642);
	
	key %= NPD_L3INTF_ADDR_HASH_SIZE;

	return key;
}

unsigned int npd_v6_l3_intf_addr_if_key_gen
(
	void *data
)
{
	unsigned int key = 0;
	NPD_V6_L3INTF_ADDR *item = (NPD_V6_L3INTF_ADDR *)data;
	if(NULL == item) {
		return ~0UI;
	}

	key = item->ifindex;
	
	key %= MAX_L3INTF_NUMBER;

	return key;
}


#endif //HAVE_NPD_IPV6


int npd_intf_addr_create(NPD_L3INTF_ADDR *l3intf_addr)
{	
	int ret = 0;
	unsigned int netif_index = 0;
	char ifname[MAX_IFNAME_LEN] = {0};

	npd_intf_name_get_by_ifindex(l3intf_addr->ifindex, ifname);
	npd_intf_netif_get_by_ifindex(l3intf_addr->ifindex, &netif_index);
	ret = nam_intf_addr_create(ifname, l3intf_addr->ifindex, netif_index, l3intf_addr->ipAddr, l3intf_addr->mask);

	return ret;
}

int npd_intf_addr_delete(NPD_L3INTF_ADDR *l3intf_addr)
{	
	int ret = 0;
	unsigned int netif_index = 0;	
	char ifname[MAX_IFNAME_LEN] = {0};

	npd_intf_name_get_by_ifindex(l3intf_addr->ifindex, ifname);
	npd_intf_netif_get_by_ifindex(l3intf_addr->ifindex, &netif_index);	
	ret = nam_intf_addr_delete(ifname, l3intf_addr->ifindex, netif_index, l3intf_addr->ipAddr, l3intf_addr->mask);

	return ret;
}


long npd_l3_intfaddr_db_insert(void *data)
{
	long ret = 0;
	NPD_L3INTF_ADDR *l3intf_addr = (NPD_L3INTF_ADDR *)data;
	
	ret = npd_intf_addr_create(l3intf_addr);
	return ret;
}

long npd_l3_intfaddr_db_delete(void *data)
{
	long ret = 0;
	NPD_L3INTF_ADDR *l3intf_addr = (NPD_L3INTF_ADDR *)data;
	
	ret = npd_intf_addr_delete(l3intf_addr);
	return ret;
}


int npd_l3_intf_db_ntoh(void *data)
{	
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)data;
	l3intf_ctrl->ifindex = ntohl(l3intf_ctrl->ifindex);
	l3intf_ctrl->netif_index = ntohl(l3intf_ctrl->netif_index);
	l3intf_ctrl->attribute = ntohl(l3intf_ctrl->attribute);
	l3intf_ctrl->vid = ntohs(l3intf_ctrl->vid);
	l3intf_ctrl->state = ntohl(l3intf_ctrl->state);
	l3intf_ctrl->ipv4 = ntohl(l3intf_ctrl->ipv4);
	l3intf_ctrl->mask = ntohl(l3intf_ctrl->mask);
	l3intf_ctrl->ctrl_state = ntohl(l3intf_ctrl->ctrl_state);

	return 0;
}

int npd_l3_intf_db_hton(void *data)
{	
	NPD_L3INTERFACE_CTRL *l3intf_ctrl = (NPD_L3INTERFACE_CTRL *)data;
	l3intf_ctrl->ifindex = htonl(l3intf_ctrl->ifindex);
	l3intf_ctrl->netif_index = htonl(l3intf_ctrl->netif_index);
	l3intf_ctrl->attribute = htonl(l3intf_ctrl->attribute);
	l3intf_ctrl->vid = htons(l3intf_ctrl->vid);
	l3intf_ctrl->state = htonl(l3intf_ctrl->state);
	l3intf_ctrl->ipv4 = htonl(l3intf_ctrl->ipv4);
	l3intf_ctrl->mask = htonl(l3intf_ctrl->mask);
	l3intf_ctrl->ctrl_state = htonl(l3intf_ctrl->ctrl_state);

	return 0;
}


int l3_intfaddr_db_ntoh(void *data)
{
	NPD_L3INTF_ADDR *l3ntf_addr = (NPD_L3INTF_ADDR *)data;

	l3ntf_addr->ipAddr = ntohl(l3ntf_addr->ipAddr);
	l3ntf_addr->mask = ntohl(l3ntf_addr->mask);
	l3ntf_addr->ifindex = ntohl(l3ntf_addr->ifindex);

	return 0;
}

int l3_intfaddr_db_hton(void *data)
{
	NPD_L3INTF_ADDR *l3ntf_addr = (NPD_L3INTF_ADDR *)data;

	l3ntf_addr->ipAddr = htonl(l3ntf_addr->ipAddr);
	l3ntf_addr->mask = htonl(l3ntf_addr->mask);
	l3ntf_addr->ifindex = htonl(l3ntf_addr->ifindex);

	return 0;
}


/**********************************************************************************
 *  npd_intf_init
 *
 *  init global L3 interface structure.
 *
 *  INPUT:
 *      NULL
 *
 *  OUTPUT:
 *      NULL
 *
 *  RETURN:
 *      NULL
 *
 *
 **********************************************************************************/
void npd_intf_init(void)
{
    char name[16];
    db_table_t *db;
    
    strcpy(name, "l3intf");
    create_dbtable(name, MAX_L3INTF_NUMBER, sizeof(NPD_L3INTERFACE_CTRL),
                   npd_l3_intf_db_attr_update_handler, 
                   NULL,
                   npd_l3_intf_db_create_handler, 
                   npd_l3_intf_db_delete_handler, NULL, NULL, NULL, 
                   npd_l3_intf_db_ntoh, npd_l3_intf_db_hton, DB_SYNC_ALL, &l3intf_table);
	
    dbtable_create_hash_index("netif_index", l3intf_table, MAX_L3INTF_NUMBER, npd_l3_intf_db_netif_index,
                              npd_l3_intf_cmp_netif_index, &l3intf_netif_index);

    if (NULL == l3intf_netif_index)
    {
        npd_syslog_dbg("Create l3intf hash index by netif index failed!!!\n");
        return;
    }

    dbtable_create_hash_index("l3_ifindex", l3intf_table, MAX_L3INTF_NUMBER, npd_l3_intf_db_ifindex,
                              npd_l3_intf_cmp_netif_and_ifindex, &l3intf_ifindex);

    if (NULL == l3intf_ifindex)
    {
        npd_syslog_dbg("Create l3intf hash index by ifindex failed!!!\n");
        return;
    }

    strcpy(name, "l3intflocal");
    create_dbtable(name, MAX_L3INTF_NUMBER, sizeof(NPD_L3LOCAL_IFINDEX),
                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                 NULL, NULL, DB_SYNC_NONE, &db);
    if (NULL == db)
    {
        npd_syslog_dbg("Create l3intf hash index by netif index failed!!!\n");
        return;
    }

    dbtable_create_hash_index("l3localhash", db, MAX_L3INTF_NUMBER, npd_l3_local_netif_index,
                              npd_l3_local_cmp_netif_index, &l3local_netif_index);

    if (NULL == l3local_netif_index)
    {
        npd_syslog_dbg("Create l3local hash index by netif index failed!!!\n");
        return;
    }

    dbtable_create_hash_index("l3_ifindex", db, MAX_L3INTF_NUMBER, npd_l3_local_db_ifindex,
                              npd_l3_local_cmp_ifindex, &l3local_ifindex);

    if (NULL == l3local_ifindex)
    {
        npd_syslog_dbg("Create l3local hash index by ifindex failed!!!\n");
        return;
    }

	strcpy(name, "l3intfaddr");
	create_dbtable(name, MAX_L3INTF_ADDR, sizeof(NPD_L3INTF_ADDR),
				 NULL, NULL, npd_l3_intfaddr_db_insert, npd_l3_intfaddr_db_delete, NULL, NULL, NULL, 
				 l3_intfaddr_db_ntoh, l3_intfaddr_db_hton, DB_SYNC_ALL, &db);
	if (NULL == db)
	{
		npd_syslog_dbg("Create l3intf hash index by netif index failed!!!\n");
		return;
	}

	dbtable_create_hash_index("l3intfaddrhash", db, NPD_L3INTF_ADDR_HASH_SIZE, npd_l3_intf_addr_key_gen,
							  npd_l3_intf_addr_compare, &l3intf_addr_index);

	if (NULL == l3intf_addr_index)
	{
		npd_syslog_dbg("Create l3intf addr hash index by netif index failed!!!\n");
		return;
	}
	dbtable_create_hash_index("l3intfaddrifhash", db, MAX_L3INTF_NUMBER, npd_l3_intf_addr_if_key_gen,
							  npd_l3_intf_addr_compare, &l3intf_addr_if_index);

	if (NULL == l3intf_addr_if_index)
	{
		npd_syslog_dbg("Create l3intf addr ifindex hash index by netif index failed!!!\n");
		return;
	}
#ifdef HAVE_NPD_IPV6
	strcpy(name, "v6l3intfaddr");
	create_dbtable(name, MAX_L3INTF_ADDR, sizeof(NPD_V6_L3INTF_ADDR),
				 NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
				 NULL, NULL, DB_SYNC_ALL, &db);
	if (NULL == db)
	{
		npd_syslog_dbg("Create ipv6 l3intf hash index by netif index failed!!!\n");
		return;
	}

	dbtable_create_hash_index("v6l3intfaddrhash", db, NPD_L3INTF_ADDR_HASH_SIZE, npd_v6_l3_intf_addr_key_gen,
							  npd_v6_l3_intf_addr_compare, &v6l3intf_addr_index);

	if (NULL == v6l3intf_addr_index)
	{
		npd_syslog_dbg("Create ipv6 l3intf addr hash index by netif index failed!!!\n");
		return;
	}
	dbtable_create_hash_index("l3intfaddrifhash", db, MAX_L3INTF_NUMBER, npd_v6_l3_intf_addr_if_key_gen,
							  npd_v6_l3_intf_addr_compare, &v6l3intf_addr_if_index);

	if (NULL == v6l3intf_addr_if_index)
	{
		npd_syslog_dbg("Create ipv6 l3intf addr ifindex hash index by netif index failed!!!\n");
		return;
	}
#endif //HAVE_NPD_IPV6

	nam_intf_tbl_index_init();
	
    register_netif_notifier(&intf_netif_notifier);
    /*nam_route_config();*/
}

int npd_intf_get_global_l3index
(
    unsigned int l3_local_id,
    unsigned int *l3_g_id
)
{
    NPD_L3LOCAL_IFINDEX l3local = {0};
    NPD_L3INTERFACE_CTRL l3intf = {0};
    int ret;
    
    l3local.ifindex = l3_local_id;
    ret = dbtable_hash_search(l3local_ifindex, &l3local, NULL, &l3local);
    if(0 != ret)
    {
        *l3_g_id = 0;
        return FALSE;
    }
    l3intf.netif_index = l3local.netif_index;
    ret = dbtable_hash_search(l3intf_netif_index, &l3intf, npd_l3_intf_cmp_netif_index,
              &l3intf);
    if(0 != ret)
    {
        *l3_g_id = 0;
        return FALSE;
    }
    *l3_g_id = l3intf.ifindex;
	
	syslog_ax_intf_event("global index for local index 0x%x is found, l3 ifindex = 0x%x.\r\n", l3_local_id, *l3_g_id);
    return TRUE;
}

int npd_intf_get
(
    unsigned int l3_local_id,
    NPD_L3INTERFACE_CTRL *l3intf
)
{
    NPD_L3LOCAL_IFINDEX l3local = {0};
    int ret;
    
    l3local.ifindex = l3_local_id;
    ret = dbtable_hash_search(l3local_ifindex, &l3local, NULL, &l3local);
    if(0 != ret)
    {
        return FALSE;
    }
    l3intf->netif_index = l3local.netif_index;
    ret = dbtable_hash_search(l3intf_netif_index, l3intf, npd_l3_intf_cmp_netif_index,
              l3intf);
    if(0 != ret)
        return FALSE;
    return TRUE;
}

int npd_l3_intf_create(unsigned int netif_index, unsigned int *ifindex)
{
	int ret = 0;
	unsigned char route_mode = 0;
	unsigned long netif_type = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
	ret = npd_netif_check_exist(netif_index);
	if(ret != 0)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}
	if(INTERFACE_RETURN_CODE_SUCCESS != nam_intf_tbl_index_alloc(&(l3intf_ctrl.ifindex)))
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}

	l3intf_ctrl.ipmc = FALSE;
	
	l3intf_ctrl.netif_index = netif_index;

	l3intf_ctrl.ctrl_state = INTF_CTRL_STATE_UP;

	l3intf_ctrl.proxy_arp = FALSE;

#ifdef HAVE_MEMORY_SHORT
	if (NPD_TRUE == npd_intf_is_eth_mng_check(l3intf_ctrl.netif_index))
	{
		sprintf(l3intf_ctrl.ifname, "mng1");	
	}
	else
#endif
    {
    	ret = npd_netif_index_to_name(l3intf_ctrl.netif_index, l3intf_ctrl.ifname);
    	if(ret != 0)
    	{
    		nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
	        goto error;
    	}
    }

	ret = npd_system_get_basemac(l3intf_ctrl.mac_addr, 10);
	if(ret != 0)
	{
		nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
        goto error;
	}

	netif_type = npd_netif_type_get(l3intf_ctrl.netif_index);
    
	switch(netif_type)
	{
		int mode = 0;
		case NPD_NETIF_ETH_TYPE:
			l3intf_ctrl.vid = NPD_PORT_L3INTF_VLAN_ID;
        	ret = npd_get_eth_port_route_mode(netif_index, &route_mode);
        	if(ret != 0)
        	{
		        nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
                goto error;
        	}
        	if(route_mode == 0)
        	{
				ret = npd_switch_port_get_vlan_mode(netif_index, &mode);
				if(ret != 0)
				{
		            nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
                    goto error;
				}
				/*
				if(mode != SWITCH_PORT_MODE_ACCESS)
				{
		            nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
        		    return INTERFACE_RETURN_CODE_NOT_ACCESS_MODE;
				}
				*/
        		npd_set_ethport_route_mode(netif_index, 1);
        	}
        	break;
		case NPD_NETIF_TRUNK_TYPE:
			l3intf_ctrl.vid = NPD_PORT_L3INTF_VLAN_ID;
        	ret = npd_get_trunk_route_mode(netif_index, &route_mode);
        	if(ret != 0)
        	{
				syslog_ax_intf_err("Get the route mode of LAG %d failed.\r\n", npd_netif_trunk_get_tid(netif_index));
                goto error;
        	}
        	if(route_mode == 0)
        	{
				ret = npd_switch_port_get_vlan_mode(netif_index, &mode);
				if(ret != 0)
				{
		            nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
                    goto error;
				}
				/*
				if(mode != SWITCH_PORT_MODE_ACCESS)
				{
		            nam_intf_tbl_index_free(l3intf_ctrl.ifindex);
        		    return INTERFACE_RETURN_CODE_NOT_ACCESS_MODE;
				}
				*/
				syslog_ax_intf_dbg("LAG %d is in switch mode, will change it to route mode.\r\n", npd_netif_trunk_get_tid(netif_index));
        		npd_set_trunk_route_mode(npd_netif_trunk_get_tid(netif_index), 1);
        	}
			else
			{
				syslog_ax_intf_dbg("LAG %d has already been in route mode.\r\n", npd_netif_trunk_get_tid(netif_index));
			}
			break;
		case NPD_NETIF_VLAN_TYPE:
			l3intf_ctrl.vid = npd_netif_vlan_get_vid(l3intf_ctrl.netif_index);
			break;
#ifdef HAVE_M4_TUNNEL
        case NPD_NETIF_TUNNEL_TYPE:
        {
            l3intf_ctrl.vid = npd_tunnel_s_vlan_get_by_netif_index(netif_index);
            break;
        }
#endif
		default:
            goto error;
            break;
	}
	npd_key_database_lock();
    dbtable_hash_lock(l3intf_netif_index);
    l3intf_ctrl.state = npd_netif_get_status(l3intf_ctrl.netif_index);
	
    ret = dbtable_hash_insert(l3intf_netif_index, &l3intf_ctrl);
	if(ret != 0)
	{
		ret = INTERFACE_RETURN_CODE_ERROR;
	}
	*ifindex = l3intf_ctrl.ifindex;
    dbtable_hash_unlock(l3intf_netif_index);
	if (NPD_NETIF_TUNNEL_TYPE != netif_type)
    {
    	netif_notify_event(netif_index, PORT_NOTIFIER_L3CREATE);
    	netif_app_notify_event(netif_index, PORT_NOTIFIER_L3CREATE, NULL, 0);
    }
	npd_key_database_unlock(); 
	
	
    return ret;
error:
	return INTERFACE_RETURN_CODE_ERROR;

}

int npd_l3_intf_delete(unsigned int netif_index)
{
	int ret = 0;
	unsigned char route_mode = 0;
	unsigned long netif_type = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	
	l3intf_ctrl.netif_index = netif_index;

	netif_type = npd_netif_type_get(l3intf_ctrl.netif_index);
    ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
    
	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
        	ret = npd_get_eth_port_route_mode(netif_index, &route_mode);
        	if(ret != 0)
        	{
        		return INTERFACE_RETURN_CODE_ERROR;
        	}
        	if(route_mode == 1)
        	{
        		npd_del_ethport_route_mode(netif_index);
        	}
        	break;
		case NPD_NETIF_TRUNK_TYPE:
        	ret = npd_get_trunk_route_mode(netif_index, &route_mode);
        	if(ret != 0)
        	{
        		return INTERFACE_RETURN_CODE_ERROR;
        	}
        	if(route_mode == 1)
        	{
        		npd_del_trunk_route_mode(npd_netif_trunk_get_tid(netif_index));
        	}
			break;
		case NPD_NETIF_VLAN_TYPE:
			break;
#ifdef HAVE_M4_TUNNEL
        case NPD_NETIF_TUNNEL_TYPE:
        {
            break;
        }
#endif
		default:
			return INTERFACE_RETURN_CODE_ERROR;
	}

	npd_intf_del_all_ip_addr(l3intf_ctrl.ifindex);
    
	npd_key_database_lock();
	if (NPD_NETIF_TUNNEL_TYPE != netif_type)
    {
    	netif_notify_event(netif_index, PORT_NOTIFIER_L3DELETE);
    	netif_app_notify_event(netif_index, PORT_NOTIFIER_L3DELETE, NULL, 0);
    }

    ret = dbtable_hash_delete(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl);
	npd_key_database_unlock();
	if(ret != 0)
	{
		ret = INTERFACE_RETURN_CODE_ERROR;
	}
    return ret;
}

int npd_intf_table_is_full()
{
	int ret = dbtable_hash_count(l3intf_netif_index);
	if(ret == -1)
	{
		return TRUE;
	}
	if(ret >= MAX_L3INTF_NUMBER)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/***********************************************************
 *
 * RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      INTERFACE_RETURN_CODE_VLAN_NOTEXIST
 *      INTERFACE_RETURN_CODE_CONTAIN_PROMI_PORT
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_ALREADY_ADVANCED
 *      INTERFACE_RETURN_CODE_INTERFACE_EXIST
 *      INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND
 *      INTERFACE_RETURN_CODE_ERROR
 *
 ***********************************************************/
DBusMessage * npd_dbus_create_l3intf(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    unsigned int ifindex = ~0UI, ret = INTERFACE_RETURN_CODE_SUCCESS;
    unsigned int netif_index = 0;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	ret = npd_intf_exist_check(netif_index, &ifindex);
	if(ret == NPD_TRUE)
	{
		if (NPD_TRUE == npd_intf_is_eth_mng_check(netif_index))
		{
			/* eth port if is eth mng type, the name must be mng started with */
			ret = INTERFACE_RETURN_CODE_ERROR;
		}
		else
		{
			ret = INTERFACE_RETURN_CODE_SUCCESS;
		}
		
	}
	else
	{
        ret = npd_l3_intf_create(netif_index, &ifindex);
	}
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ifindex);
    return reply;
}
/*************************************************************
 *
 * RETURN:
 *      INTERFACE_RETURN_CODE_SUCCESS
 *      INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *      INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *      INTERFACE_RETURN_CODE_VLAN_NOTEXIST
 *      INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST
 *      INTERFACE_RETURN_CODE_ERROR
 *
 *************************************************************/
DBusMessage * npd_dbus_delete_l3intf(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ifindex = ~0UI, ret = INTERFACE_RETURN_CODE_SUCCESS;
    unsigned int netif_index = ~0UI;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	
	ret = npd_intf_exist_check(netif_index, &ifindex);
	if(ret == NPD_TRUE)
	{
        ret = npd_l3_intf_delete(netif_index);
	}
	else
	{
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_shutdown_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int g_ifindex = 0;
    unsigned int netif_index = 0;
	unsigned int l3intf_ctrl_state = 0;
	unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
								 DBUS_TYPE_UINT32, &l3intf_ctrl_state,
	                             DBUS_TYPE_STRING, &ifname,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
		ret = npd_intf_gindex_exist_check(netif_index, &g_ifindex);
	 	if(ret == NPD_TRUE)
		{
	        ret = npd_intf_set_l3intf_ctrl_status(g_ifindex, l3intf_ctrl_state);
		}
		else 
		{
			ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		}
	}
	else
	{
		ret = npd_intf_set_kern_flag(ifname, l3intf_ctrl_state);	
	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}
#ifndef HAVE_ZEBRA

DBusMessage * npd_dbus_add_ipv4_addr(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
    unsigned int netif_index = 0;
	unsigned int ipv4_addr = 0;
	unsigned int ipv4_mask = 0;
	unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                             DBUS_TYPE_STRING, &ifname,
								 DBUS_TYPE_UINT32, &ipv4_addr,
								 DBUS_TYPE_UINT32, &ipv4_mask,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
		unsigned int l3_index = 0;
		ret = npd_intf_ifindex_get_by_ifname(ifname, &l3_index);
		if(ret == INTERFACE_RETURN_CODE_SUCCESS)
		{
		    unsigned int local_index = 0;
			ret = npd_intf_get_global_l3index(l3_index, &local_index);
			if(NPD_TRUE == ret )
			{
		        ret = npd_intf_add_ip_addr(local_index, ipv4_addr, ipv4_mask);
			}
			else
			{
				ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
			}
		}
	}
	else
	{
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_del_ipv4_addr(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
    unsigned int netif_index = 0;
	unsigned int ipv4_addr = 0;
	unsigned int ipv4_mask = 0;
	unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                             DBUS_TYPE_STRING, &ifname,
								 DBUS_TYPE_UINT32, &ipv4_addr,
								 DBUS_TYPE_UINT32, &ipv4_mask,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
		unsigned int l3_index = 0;
		ret = npd_intf_ifindex_get_by_ifname(ifname, &l3_index);
		if(ret == INTERFACE_RETURN_CODE_SUCCESS)
		{
		    unsigned int local_index = 0;
			ret = npd_intf_get_global_l3index(l3_index, &local_index);
			if(NPD_TRUE == ret )
			{
		        ret = npd_intf_del_ip_addr(local_index, ipv4_addr, ipv4_mask);
			}
			else
			{
				ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
			}
		}
	}
	else
	{
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_intf_showrun(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	int ret = -1;
	unsigned int rc = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	char    netif_name[MAX_IFNAME_LEN];
	unsigned int ipAddrs[MAX_IP_COUNT];
	unsigned int masks[MAX_IP_COUNT];
	char *show_buffer = NULL;
    int length = 0;
#if 0
	show_buffer = malloc(NPD_L3_INTF_TABLE_SIZE*512);
#endif
	show_buffer = malloc(MAX_L3INTF_NUMBER*512);
	if(show_buffer == NULL)
	{
		return NULL;
	}
#if 0	
	memset(show_buffer, 0, NPD_L3_INTF_TABLE_SIZE*512);
#endif
	memset(show_buffer, 0, MAX_L3INTF_NUMBER*512);
	memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
    dbus_error_init(&err);
	ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
    while(ret == 0)
    {
		int op_ret = -1;
		int i = 0;
		length = strlen(show_buffer);
		interface_name_convert_to_netif_name(l3intf_ctrl.ifname, netif_name);
		sprintf(show_buffer + length, "interface %s\n", netif_name);
		memset(ipAddrs, 0, sizeof(ipAddrs));
		memset(ipAddrs, 0, sizeof(masks));
		op_ret = npd_intf_addr_ip_get(l3intf_ctrl.ifindex, ipAddrs, masks);
		for(i = 0; i < MAX_IP_COUNT; i++)
		{
			if((ipAddrs[i] != 0) && (masks[i] != 0))
			{
				int mask_len = 0;
			    lib_get_masklen_from_mask(masks[i], &mask_len);
				length = strlen(show_buffer);
				sprintf(show_buffer + length, " ip address ");
				length = strlen(show_buffer);
			    lib_get_string_from_ip(show_buffer + length, ipAddrs[i]);
			    length = strlen(show_buffer);
			    sprintf(show_buffer + length, "/%d\n", mask_len);
			}
		}
#ifdef HAVE_PORTAL		
		if((l3intf_ctrl.portal_ctrl == 1) && (l3intf_ctrl.portal_srv_id == 0))
		{
            length = strlen(show_buffer);
            sprintf(show_buffer + length, " portal server local\n");
		}
#endif		
		length = strlen(show_buffer);
		sprintf(show_buffer + length, " exit\n");
		ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rc);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &show_buffer);
	
    free(show_buffer);
    return reply;
}

#endif

#ifdef HAVE_PORTAL

DBusMessage * npd_dbus_set_portal_server(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
    unsigned int netif_index = 0;
	unsigned int portal_ctrl = 0;
	unsigned int portal_srv_id = 0;
    unsigned int ipv4;
    unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                             DBUS_TYPE_STRING, &ifname,
								 DBUS_TYPE_UINT32, &portal_ctrl,
								 DBUS_TYPE_UINT32, &portal_srv_id,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret)
	{
        ret = npd_intf_set_portal_server(netif_index,portal_ctrl,portal_srv_id,&ipv4);
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32, &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32, &ipv4);
    return reply;
} 

DBusMessage * npd_dbus_show_portal_interface(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	int ret = -1;
    unsigned int num = 0;
	unsigned int rc = INTERFACE_RETURN_CODE_SUCCESS;
    NPD_L3INTERFACE_CTRL l3intf_ctrl;
    
	memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
    dbus_error_init(&err);
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rc);
    ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
    while(ret == 0)
    {		      
		if((l3intf_ctrl.portal_ctrl == 1) && (l3intf_ctrl.portal_srv_id == 0))
            num ++;
    
		ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
    }
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &num);
    
    memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
    ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
    while(ret == 0)
    {		      
		if((l3intf_ctrl.portal_ctrl == 1) && (l3intf_ctrl.portal_srv_id == 0))
		{
            dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &l3intf_ctrl.netif_index);
            dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &l3intf_ctrl.portal_srv_id);
		}
        
		ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
    }
	
    return reply;
} 

DBusMessage * npd_dbus_intf_portal_server_showrun(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	int ret = -1;
	unsigned int rc = INTERFACE_RETURN_CODE_SUCCESS;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
	char    netif_name[MAX_IFNAME_LEN];
	char *show_buffer = NULL;
    char *cursor = NULL;
    int length = 0;
	show_buffer = malloc(MAX_L3INTF_NUMBER*512);
	if(show_buffer == NULL)
	{
		return NULL;
	}

    cursor = show_buffer;
	memset(show_buffer, 0, MAX_L3INTF_NUMBER*512);
	memset(&l3intf_ctrl, 0, sizeof(NPD_L3INTERFACE_CTRL));
    dbus_error_init(&err);
	ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
    while(ret == 0)
    {
		interface_name_convert_to_netif_name(l3intf_ctrl.ifname, netif_name);
		
		if(l3intf_ctrl.portal_ctrl == 1)
		{
            length += sprintf(cursor, "interface %s\n", netif_name);
            cursor = show_buffer + length;
            
            if(l3intf_ctrl.portal_srv_id == 0)
            {
                length += sprintf(cursor, " portal server local\n");
                cursor = show_buffer + length;
            }

            length += sprintf(cursor, " exit\n");
            cursor = show_buffer + length;
		}	
        
		ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rc);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &show_buffer);
	
    free(show_buffer);
    length = 0;
    
    return reply;
}
#endif

#ifdef HAVE_ROUTE
DBusMessage *npd_dbus_intf_urpf_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int g_ifindex = 0;
    unsigned int netif_index = 0;
	unsigned int l3intf_ctrl_state = 0;
	unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                             DBUS_TYPE_STRING, &ifname,
								 DBUS_TYPE_UINT32, &l3intf_ctrl_state,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
		ret = npd_intf_gindex_exist_check(netif_index, &g_ifindex);
	 	if(ret == NPD_TRUE)
		{
			unsigned int old_state = 0;
			ret = npd_intf_attribute_get_by_ifindex(g_ifindex, &old_state);
			if(ret != NPD_TRUE)
			{
				ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
			}
			else
			{
				if(l3intf_ctrl_state == old_state)
				{
					ret = INTERFACE_RETURN_CODE_SUCCESS;
				}
				else
				{
        			if(l3intf_ctrl_state == 1)
        			{
        				l3intf_ctrl_state = NPD_INTF_ATTR_URPF_STRICT;
    					/*应该更新所有路由表项中的URPF标志*/
    					//ret = npd_route_traversal_by_ifindex_update_flags(g_ifindex, l3intf_ctrl_state);
        			}
#if 0                    
        			else if(l3intf_ctrl_state == 2)
        			{
        				l3intf_ctrl_state = NPD_INTF_ATTR_URPF_LOOSE;
    					/*应该更新所有路由表项中的URPF标志*/
    					ret = npd_route_traversal_by_ifindex_update_flags(g_ifindex, l3intf_ctrl_state);
        			}
#endif                    
        	        ret = npd_intf_attribute_set_by_ifindex(g_ifindex, l3intf_ctrl_state);
				}
			}
		}
		else 
		{
			ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		}
	}
	else
	{
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage *npd_dbus_intf_ipmc_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int	enable = 0;
	DBusError 		err;
    int ret = 0;
	unsigned char* ifname = NULL;
	unsigned int netif_index;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
         DBUS_TYPE_STRING, &ifname,
    	 DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID)))
		{		
			 syslog_ax_route_dbg("Unable to get input args\n ");
			if (dbus_error_is_set(&err)) 
			{
				 syslog_ax_route_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
    	NPD_L3INTERFACE_CTRL l3intf = {0};
    
    	l3intf.netif_index = netif_index;
    	ret = dbtable_hash_search(l3intf_netif_index, &l3intf, NULL, &l3intf);
    	if(0 != ret)
    	{
    		syslog_ax_intf_err("netif index for Interface %#x is not found.\r\n", netif_index);
    		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
    	}
        else
        {
    	    l3intf.ipmc = enable;
    	    ret  = dbtable_hash_update(l3intf_netif_index, NULL, &l3intf);
        }
	}
	else
	{
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;	
	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

#endif

#ifdef HAVE_ZEBRA
DBusMessage * npd_dbus_intf_proxy_arp_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = INTERFACE_RETURN_CODE_SUCCESS;
	unsigned int g_ifindex = 0;
    unsigned int netif_index = 0;
	unsigned char l3intf_proxy_arp = 0;
	unsigned char* ifname = NULL;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                             DBUS_TYPE_STRING, &ifname,
	                             DBUS_TYPE_BYTE, &l3intf_proxy_arp,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = npd_intf_netif_get_by_name(&netif_index, ifname);
	if(NPD_TRUE == ret )
	{
		ret = npd_intf_gindex_exist_check(netif_index, &g_ifindex);
	 	if(ret == NPD_TRUE)
		{
	        ret = npd_intf_set_proxy_arp(netif_index, l3intf_proxy_arp);
		}
		else 
		{
			ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		}
	}
	else
		ret = INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}


DBusMessage *npd_dbus_intf_proxy_arp_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    char    netif_name[MAX_IFNAME_LEN];
	char *buff = NULL, *curr = NULL;
	unsigned int buff_len = 0;
	int len = 0, ret = 0;
	
	buff = (char*)malloc(NPD_INTF_SAVE_STATIC_ARP_MEM);
	if(NULL == buff) {
		return NULL;
	}
	memset(buff,0,NPD_INTF_SAVE_STATIC_ARP_MEM);		
	memset(&l3intf_ctrl, 0, sizeof(l3intf_ctrl));
	curr = buff;
	syslog_ax_arpsnooping_dbg("interface proxy arp and urpf show running-config\n");
	
	ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
	while(0 == ret)
	{
		if(l3intf_ctrl.proxy_arp == TRUE || l3intf_ctrl.attribute != 0 
			|| l3intf_ctrl.bind != 0 || l3intf_ctrl.ipmc == FALSE)
		{
            interface_name_convert_to_netif_name(l3intf_ctrl.ifname, netif_name);
                
			len = sprintf(curr, "interface %s\n", netif_name);
			curr += len;
			if(l3intf_ctrl.proxy_arp == TRUE)
			{
    			len = sprintf(curr, " ip proxy-arp\n");
    			curr += len;
			}
			if(l3intf_ctrl.attribute & NPD_INTF_ATTR_URPF_STRICT)
			{
    			len = sprintf(curr, " ip route urpf check\n");
    			curr += len;
			}
			if(l3intf_ctrl.ipmc == FALSE)
			{
				len = sprintf(curr, " no ip multicast-routing\n");
			}
#if 0            
			else if(l3intf_ctrl.attribute & NPD_INTF_ATTR_URPF_LOOSE)
			{
    			len = sprintf(curr, " urpf loose\n");
    			curr += len;
			}
            if(l3intf_ctrl.bind == 1)
            {
    			len = sprintf(curr, " ip-source-guard\n");
    			curr += len;
                
            }
#endif
			
			len = sprintf(curr, "exit\n");
			curr+= len;				
		}
		ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &buff); 
								 
	free(buff);
	buff_len = 0;
	
	return reply;
}

#endif
DBusMessage *npd_dbus_vlan_intf_check(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter iter;

	unsigned int vlan_netif_index = 0;
	unsigned int vlan_if_index = 0;

	unsigned short vlan_id = 0;
	unsigned op_ret = 0;

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT16, &vlan_id,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return NULL;
	}
	vlan_netif_index = npd_netif_vlan_index(vlan_id);
	if (vlan_netif_index == 0)
		op_ret = INTERFACE_RETURN_CODE_VLAN_NOTEXIST;
	else
	{
		op_ret = npd_intf_exist_check(vlan_netif_index, &vlan_if_index);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &op_ret);
	return reply;
	
}

DBusMessage *npd_dbus_l3intf_get_next(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter iter;

	unsigned int netif_index = 0;
	unsigned char *ifname = NULL;
	unsigned op_ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return NULL;
	}

	memset(&l3intf_ctrl,0,sizeof(l3intf_ctrl));
	l3intf_ctrl.netif_index = netif_index;
	if( netif_index == 0 )
	{
		op_ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
	}
	else
	{
		op_ret = dbtable_hash_next(l3intf_netif_index, &l3intf_ctrl, &l3intf_ctrl, NULL);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &op_ret);

	if(INTERFACE_RETURN_CODE_SUCCESS == op_ret)
	{	
		dbus_message_iter_append_basic(&iter, 
										DBUS_TYPE_UINT32, &l3intf_ctrl.netif_index);
		ifname = (unsigned char *)l3intf_ctrl.ifname;
		dbus_message_iter_append_basic(&iter, 
										DBUS_TYPE_STRING, &ifname);
		dbus_message_iter_append_basic(&iter, 
										DBUS_TYPE_BYTE, &l3intf_ctrl.proxy_arp);
		dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &l3intf_ctrl.attribute);
	}
	return reply;
	
}

DBusMessage *npd_dbus_l3intf_get(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter iter;

	unsigned int netif_index = 0;
	unsigned char *ifname = NULL;
	unsigned op_ret = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return NULL;
	}

	memset(&l3intf_ctrl,0,sizeof(l3intf_ctrl));
	l3intf_ctrl.netif_index = netif_index;
	if( netif_index == 0 )
	{
		op_ret = dbtable_hash_head(l3intf_netif_index, NULL, &l3intf_ctrl, NULL);
	}
	else
	{
		op_ret = dbtable_hash_search(l3intf_netif_index, &l3intf_ctrl, NULL, &l3intf_ctrl);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &op_ret);
	if(INTERFACE_RETURN_CODE_SUCCESS == op_ret)
	{
		dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &l3intf_ctrl.netif_index);
		ifname = (unsigned char *)l3intf_ctrl.ifname;
		dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_STRING, &ifname);
		dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_BYTE, &l3intf_ctrl.proxy_arp);
		dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, &l3intf_ctrl.attribute);
		dbus_message_iter_append_basic(&iter,
			                        DBUS_TYPE_UINT32, &l3intf_ctrl.bind);
		dbus_message_iter_append_basic(&iter,
			                        DBUS_TYPE_UINT32, &l3intf_ctrl.ipmc);
	}
	return reply;
	
}


#ifdef __cplusplus
}
#endif
