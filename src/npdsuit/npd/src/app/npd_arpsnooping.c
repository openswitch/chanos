
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_arpsnooping.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for L3 interface ARP snooping process.
*
* DATE:
*		03/10/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.116 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/mng/arpsnoop.h"
#include "npd_arpsnooping.h"
#include "npd_intf.h"

#ifndef NDA_RTA
#define NDA_RTA(r) ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif
#define MAX_IP_COUNT 8
#define INVALID_HOST_IP 0xFFFFFFFF
#define MUTIIPSUPPORT (0)
#define ARP_REQUEST 0x1
#define ARP_REPLY   0x2

extern hash_table_index_t *ser_policy_pb_hash;
extern hash_table_index_t *ser_policy_pb_index;
extern hash_table_index_t *l3intf_netif_index;
extern hash_table_index_t *l3intf_ifindex;

hash_table_index_t *npd_arpsnp_haship_index = NULL;
hash_table_index_t *npd_arpsnp_hashport_index = NULL;
hash_table_index_t *npd_arpsnp_hashmac_index = NULL;
hash_table_index_t *npd_nexthop_hash_index = NULL;
array_table_index_t *npd_arpsnp_cfg_index = NULL;

db_table_t         *npd_arpsnp_dbtbl = NULL;
db_table_t         *npd_nexthop_dbtbl = NULL;
db_table_t		   *npd_arpsnp_cfgtbl = NULL;

db_table_t   *npd_arp_inspection_status_table = NULL;
array_table_index_t *npd_arp_inspection_status_index = NULL;

struct npd_arp_inspection_statistics_s npd_arp_inspection_statistics[MAX_SWITCHPORT_PER_SYSTEM];

int arpSock = 0;  /*netlink socket fd*/
int arp_aging_continue = 0;
int sysKernArpSock = 0;  /*user arp synchronization kernel */
unsigned int arp_smac_check = FALSE;
unsigned int arp_inspection	=	FALSE;
unsigned int arp_absolute_time = 0;
unsigned int arp_drop_enable = FALSE;

pthread_mutex_t arpKernOpMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t namItemOpMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nexthopHashMutex = PTHREAD_MUTEX_INITIALIZER;


static char *npd_arp_showStr = NULL;
static int npd_arp_showStr_len = 0;
unsigned int arp_inspection_global_no ;

netif_event_notifier_t arpsnp_netif_notifier =
{
    .netif_event_handle_f = &npd_arpsnp_notify_event,
    .netif_relate_handle_f = &npd_arpsnp_relate_event
};


int npd_arp_inspection_statistics_init();
int npd_arp_inspection_clear_statistics();
int npd_arp_inspection_init_port_statistic(unsigned int eth_g_index);
int npd_arp_inspection_clean_port_statistic(unsigned int eth_g_index);
int npd_arp_inspection_modify_port_by_trunk(struct arp_inspection_status* entry, int netif_index);


int npd_arp_inspection_db_ntoh(void * data)
{
    int ni = 0;
	struct arp_inspection_status *arpInsp = (struct arp_inspection_status *)data;

	arpInsp->arp_inspection_enable = ntohl(arpInsp->arp_inspection_enable);
	arpInsp->allowzero = ntohl(arpInsp->allowzero);
	NPD_VBMP_VLAN_NTOH(arpInsp->allow_arp_vlans);
	NPD_PBMP_PORT_NTOH(arpInsp->trust);

    for (ni = 0; ni < MAX_SWITCHPORT_PER_SYSTEM; ni++)
    {
        arpInsp->switch_port_control_count[ni] = ntohs(arpInsp->switch_port_control_count[ni]);
    }

	return 0;
}

int npd_arp_inspection_db_hton(void * data)
{
    int ni = 0;
	struct arp_inspection_status *arpInsp = (struct arp_inspection_status *)data;

	arpInsp->arp_inspection_enable = htonl(arpInsp->arp_inspection_enable);
	arpInsp->allowzero = htonl(arpInsp->allowzero);
	NPD_VBMP_VLAN_HTON(arpInsp->allow_arp_vlans);
	NPD_PBMP_PORT_HTON(arpInsp->trust);

    for (ni = 0; ni < MAX_SWITCHPORT_PER_SYSTEM; ni++)
    {
        arpInsp->switch_port_control_count[ni] = htons(arpInsp->switch_port_control_count[ni]);
    }

	return 0;
}

int npd_arp_inspection_switchcontrol_port(struct arp_inspection_status * entry, int flag)
{
    int array_index = 0;
    unsigned int netif_index = 0;

    for (array_index = 0; array_index < MAX_ETHPORT_PER_SYSTEM; array_index++)
    {
        if (entry->switch_port_control_count[array_index])
        {
            netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
            npd_arp_packet_enable_netif(netif_index , flag);
        }
    }
    return 0;
}

long npd_arp_inspection_handle_update(void *newdata, void *olddata)
{
    struct arp_inspection_status *newArpInspCfg = (struct arp_inspection_status *)newdata;
    struct arp_inspection_status *oldArpInspCfg = (struct arp_inspection_status *)olddata;
    int array_index = 0;
    unsigned int netif_index = 0;
	
    if (newArpInspCfg->arp_inspection_enable != oldArpInspCfg->arp_inspection_enable)
    {
        if (newArpInspCfg->arp_inspection_enable)
        {
            npd_arp_inspection_switchcontrol_port(newArpInspCfg, TRUE);
        }
        else
        {
            npd_arp_inspection_switchcontrol_port(newArpInspCfg, FALSE);
        }
    }
    else
    {
        if (newArpInspCfg->arp_inspection_enable)
        {
            for (array_index = 0; array_index < MAX_ETHPORT_PER_SYSTEM; array_index++)
            {
                if ((0 != newArpInspCfg->switch_port_control_count[array_index])
                    && (0 == oldArpInspCfg->switch_port_control_count[array_index]))
                {
                    netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
                    npd_arp_packet_enable_netif(netif_index, TRUE);
                }
                else if ((0 == newArpInspCfg->switch_port_control_count[array_index])
                    && (0 != oldArpInspCfg->switch_port_control_count[array_index]))
                {
                    netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
                    npd_arp_packet_enable_netif(netif_index, FALSE);
                }
            }
        }
    }

    if(!NPD_VBMP_EQ(newArpInspCfg->allow_arp_vlans, oldArpInspCfg->allow_arp_vlans))
    {
        npd_vbmp_t bmp;
        int vlan;

        NPD_VBMP_ASSIGN(bmp, newArpInspCfg->allow_arp_vlans);
        NPD_VBMP_XOR(bmp, oldArpInspCfg->allow_arp_vlans);
        NPD_VBMP_ITER(bmp, vlan)
        {
            if(NPD_VBMP_MEMBER(newArpInspCfg->allow_arp_vlans, vlan))
            {
                nam_vlan_arp_trap_en(vlan, 1);
            }
			else
			{
                nam_vlan_arp_trap_en(vlan, 0);
			}
        }
    }
	return 0;
}

long npd_arp_inspection_handle_insert(void *newdata)
{
    return 0;
}

long npd_arp_inspection_handle_delete(void *deldata)
{
    return 0;
}

unsigned int npd_arp_inspection_status_initialize()
{
	unsigned int ret;	
	struct arp_inspection_status user;
	
	ret = create_dbtable( NPD_ARP_INSPECTION_STATUS_NAME, \
							NPD_ARP_INSPECTION_STATUS_SIZE, 
							sizeof(struct arp_inspection_status),
							npd_arp_inspection_handle_update, 
							NULL,
							npd_arp_inspection_handle_insert, 
							npd_arp_inspection_handle_delete,
							NULL, 
							NULL, NULL, 
        					npd_arp_inspection_db_ntoh,
        					npd_arp_inspection_db_hton,
							DB_SYNC_ALL,
							&(npd_arp_inspection_status_table));
	if( 0 != ret )
	{
		syslog_ax_arpsnooping_err("create arp inspection global dbtable fail\n");
		return NPD_FAIL;
	}

    dbtable_create_array_index(NPD_ARP_INSPECTION_STATUS_INDEX_NAME, 
									npd_arp_inspection_status_table,  
									&npd_arp_inspection_status_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create  arp inspection item fail\n");
		return NPD_FAIL;
	}	
	
	memset(&user,0,sizeof(struct arp_inspection_status));
	
	npd_arp_inspection_global_status_insert(&user);
	
	npd_arp_inspection_statistics_init();
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/*****************************************************
 *npd_arp_snooping_is_brc_mac
 *		check whether the mac address is a broadcast address
 * INPUT:
 *		mac  - the mac address we want to check
 * OUTPUT:
 *		NONE
 * RETURN:
 *		TRUE  - the mac is a broadcast address
 *		FALSE - the mac is not a broadcast address
 * NOTE:
 *
 *****************************************************/
int npd_arp_snooping_is_brc_mac(unsigned char * mac)
{
    if(NULL == mac){
        return FALSE;
	}
	if(0xFF != (mac[0]&mac[1]&mac[2]&mac[3]&mac[4]&mac[5]) )
	{
		return FALSE;
	}
	
	return TRUE;
}

int npd_arp_snooping_is_muti_cast_mac
(
    unsigned char * mac
)
{
  if(mac[0] & 0x1){
      return TRUE;
  }
  else{
      return FALSE;
  }
}


int npd_arp_snooping_is_zero_mac(unsigned char * mac)
{
    if(NULL == mac){
        return FALSE;
	}
	if(mac[0]||mac[1]||mac[2]||mac[3]||mac[4]||mac[5]){
		return FALSE;
	}
	return TRUE;
}


int npd_arp_snooping_are_equal_macs
(
    unsigned char * mac1,
    unsigned char * mac2
)
{
    if(NULL == mac1||NULL == mac2){
		return FALSE;
    }

	if((mac1[0]^mac2[0])||
		(mac1[1]^mac2[1])||
		(mac1[2]^mac2[2])||
		(mac1[3]^mac2[3])||
		(mac1[4]^mac2[4])||
		(mac1[5]^mac2[5])){
		return FALSE;
	}

	return TRUE;
}

/**************************************************************
*npd_arp_snooping_mac_legality_check
*  to check the macs of the packet
* INPUT:
*    ethSmac -- source mac
*    ethDmac -- destination mac
*    arpSmac --sender mac
*    arpDmac -- target mac
*    opCode -- opration code
*  RETURN :
*    FALSE --the macs are illegal
*    TRUE -- the macs are legal
******************************************************************/
int npd_arp_snooping_mac_legality_check
(
    unsigned char * ethSmac,
    unsigned char * ethDmac,
    unsigned char * arpSmac,
    unsigned char * arpDmac,
    unsigned short opCode
)
{
    if((NULL == ethSmac)||(NULL == ethDmac)||\
		(NULL == arpSmac)||(NULL == arpDmac)){
        return FALSE;
	}
	if(ARP_REQUEST == opCode){
		if(((!(npd_arp_snooping_is_brc_mac(ethDmac)))&&(npd_arp_snooping_is_muti_cast_mac(ethDmac)))||\
			((npd_arp_snooping_is_zero_mac(ethDmac))||(npd_arp_snooping_is_muti_cast_mac(ethSmac)))||\
			((npd_arp_snooping_is_zero_mac(ethSmac))) ||\
			((npd_arp_snooping_is_muti_cast_mac(arpSmac))||(npd_arp_snooping_is_zero_mac(arpSmac)))){
            return FALSE; 
		}
	}
	else if(ARP_REPLY == opCode){
        if((npd_arp_snooping_is_muti_cast_mac(ethDmac))||(npd_arp_snooping_is_zero_mac(ethDmac))||\
			(npd_arp_snooping_is_muti_cast_mac(ethSmac))||(npd_arp_snooping_is_zero_mac(ethSmac))||\
			(npd_arp_snooping_is_muti_cast_mac(arpDmac))||(npd_arp_snooping_is_zero_mac(arpDmac))||\
			(npd_arp_snooping_is_muti_cast_mac(arpSmac))||(npd_arp_snooping_is_zero_mac(arpSmac))){
            return FALSE; 
		}
	}
    return TRUE;
	
}

/**********************************************************************************
 * npd_arp_snooping_ip_valid_check
 *
 * ARP snooping check if ip address valid
 *
 *	INPUT:
 *		ipAddr - ip address
 *	
 *	OUTPUT:
 *		gateway - the pointer of interface's ips
 *		mask - the pointer of interface's masks
 * 	RETURN:
 *		ARP_RETURN_CODE_SUCCESS - if no error occurred
 *		ARP_RETURN_CODE_ERROR - if error occurred		
 *
 *	NOTE:
 *
 **********************************************************************************/
int npd_arp_snooping_ip_valid_check
(
	unsigned int ipAddr
)
{
	int retVal = ARP_RETURN_CODE_SUCCESS;
	unsigned int addr = ntohl(ipAddr);
	/* legal ip address must not be broadcast,0,or multicast,*/
	/* or 169.254.x.x or 127.0.0.1*/
	/* or 224.x.x.x to 239.x.x.x */
	/* or 255.255.255.255 */
	if((0==addr) ||
		(~0UL == addr)||
		(0x7F000001 == addr ) ||
		(0xA9FE == ((addr >> 16)&0xffff)) ||
		(0xE0 == ((addr >> 24)&0xF0))) {
		retVal = ARP_RETURN_CODE_ERROR;	
	}
	return retVal;
}


/**********************************************************************************
 * npd_arp_snooping_ip_subnet_check
 *
 * 	ARP snooping check if ip address is compatible with gateway address given
 *
 *	INPUT:
 *		ipAddr - ip address
 *		gateway - the pointer of interface's ips we want to check with
 *		mask  - the pointer of interface's masks we want to check with
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		ARP_RETURN_CODE_SUCCESS - if no error occurred
 *		ARP_RETURN_CODE_ERROR - if error occurred		
 *
 *	NOTE:
 *		ip address should not be gateway address otherwise check fail
 *
 **********************************************************************************/
int npd_arp_snooping_ip_subnet_check
(
	unsigned int ipAddr,
	unsigned int * gateway,
	unsigned int * mask
)
{
	int i = 0;
	
	for(i = 0; i < MAX_IP_COUNT;i++)
	{
		if((INVALID_HOST_IP != gateway[i])&&(0 != mask[i]))
		{
			if((ipAddr & mask[i]) == (gateway[i] & mask[i]))  /* ip address in same range */
			{
				if((~0UL == (ipAddr | mask[i]))&&(INVALID_HOST_IP != mask[i])) { /* specified sub network broadcast address */
				    return ARP_RETURN_CODE_ERROR;
			    }
			    else if((ipAddr == (ipAddr & mask[i]))&&(INVALID_HOST_IP != mask[i])) { /* specified sub network reserved address */
				    return ARP_RETURN_CODE_ERROR;
			    }
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
	}
	return ARP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 * npd_arp_snooping_ip_gateway_check
 *
 * 	ARP snooping check if ip address is compatible with gateway address given by 
 *  	L3 interface index for vrrp or others(the masklen is 32).
 *
 *	INPUT:           
 *           dip  --  destination ip address
 *           gateway  -- the ips of the interface
 *           mask  --  the masks of the interface
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		ARP_RETURN_CODE_SUCCESS - if no error occurred
 *		ARP_RETURN_CODE_ERROR - if error occurred		
 *
 *	NOTE:
 *		dip address should be one of gateway address and the mask is 32
 *
 **********************************************************************************/
int npd_arp_snooping_ip_gateway_check
(
	unsigned int ipAddr,
	unsigned int * gateway
)
{
	int i = 0;
	
	for(i = 0; i < MAX_IP_COUNT;i++){
		if(INVALID_HOST_IP != gateway[i] && ipAddr == gateway[i])
		{
			return ARP_RETURN_CODE_SUCCESS;		
		}
	}
	
	return ARP_RETURN_CODE_ERROR;
}


/**********************************************************************************
 * npd_arp_snooping_check_ip_address
 *
 *  DESCRIPTION:
 *          this routine check the layer 3 interface's ip address
 *  INPUT:
 *          ipAddress - the ip address which we want to check
 *          vid          - the vid we want to check in
 *  OUTPUT:
 *          NULL
 *  RETURN:
 *          ARP_RETURN_CODE_NO_HAVE_THE_IP - l3intf no have the ip but have the same sub net ip with ipAddress
 *          ARP_RETURN_CODE_NO_HAVE_ANY_IP - l3intf no have any ip address
 *          ARP_RETURN_CODE_HAVE_THE_IP       - l3intf already have the ip address
 *          ARP_RETURN_CODE_NOT_SAME_SUB_NET - l3intf no have the same sub net ip with ipAddress
 *          ARP_RETURN_CODE_CHECK_IP_ERROR - check ip address error or no have l3intf
 *
 ***********************************************************************************/
int npd_arp_snooping_check_ip_address
(
    unsigned int ipAddress,
    unsigned short vid,
    unsigned int eth_g_index
)
{
	unsigned int ifindex = ~0UI, l3Index = ~0UI;
	int ret = ARP_RETURN_CODE_CHECK_IP_ERROR;
	unsigned int ipAddrs[MAX_IP_COUNT];
	unsigned int masks[MAX_IP_COUNT];
	int i = 0;
	unsigned int haveIp = 0,haveTheIp = 0,haveSameSubnet = 0;
	unsigned int errorip = 0;
	unsigned int ipno = 0;
	memset(ipAddrs, 0xff, MAX_IP_COUNT*sizeof(unsigned int));
	memset(masks, 0, MAX_IP_COUNT*sizeof(unsigned int));
	if(TRUE != npd_intf_port_check(vid,eth_g_index,&ifindex)){
		return ARP_RETURN_CODE_CHECK_IP_ERROR;
	}
	if(TRUE != npd_intf_get_global_l3index(ifindex, &l3Index)){
		return ARP_RETURN_CODE_CHECK_IP_ERROR;
	}
    ipno = ntohl(ipAddress);
	syslog_ax_arpsnooping_dbg("npd_intf_check_ip_address:: check for ip 0x%x, vid %d, ifIndex 0x%x\n", ipno, vid, eth_g_index );
	
	if(l3Index != ~0UL){
		ret = npd_intf_addr_ip_get(l3Index,ipAddrs,masks);
		if(NPD_TRUE != ret){
	        return ARP_RETURN_CODE_CHECK_IP_ERROR;
		}
		for(i=0;i<MAX_IP_COUNT;i++){
			if(INVALID_HOST_IP != ipAddrs[i]){
			     haveIp = 1;
				 if(ipAddrs[i] == ipAddress){
				 	haveTheIp = 1;
					break;
				 }
 				 if((0 == ipAddress)||
 					(~0UI == ipAddress)||
 					(0x0000007F == (ipno >> 24))||
 					(0x0000BFFF == (ipno >> 16))||
 					(0x00C00000 == (ipno >> 8))||
 					((ipno >> 8) >= 0x00DFFFFF)||
 					(0xffffffff  == (ipAddress | masks[i]))||
 					(0 == (ipAddress & ~masks[i])))
 				 {
 					errorip = 1;
 					syslog_ax_arpsnooping_dbg("ip dip check faile!\n");
					break;
 				 }
				 if((ipAddrs[i] & masks[i]) == (ipAddress & masks[i])){
	                 haveSameSubnet = 1;
					 break;
				 }
			}
		}
		if(haveIp == 0){
			syslog_ax_arpsnooping_err("npd_intf_check_ip_address:: l3intf don't have any ip address!\n");
			ret = ARP_RETURN_CODE_NO_HAVE_ANY_IP;
		}else if(haveTheIp == 1){
	        syslog_ax_arpsnooping_err("npd_intf_check_ip_address:: already have the ip address!\n");
			ret = ARP_RETURN_CODE_HAVE_THE_IP;
		}else if(haveSameSubnet == 1){
	        syslog_ax_arpsnooping_err("npd_intf_check_ip_address:: l3intf don't have the ip and have the same sub net ip!\n");	
			ret = ARP_RETURN_CODE_NO_HAVE_THE_IP;
		}else if(errorip == 1){
		    syslog_ax_arpsnooping_err("npd_intf_check_ip_address:: The ip is illegal ip!\n");	
			ret = ARP_RETURN_CODE_CHECK_IP_ERROR;
		}else{
	        syslog_ax_arpsnooping_dbg("npd_intf_check_ip_address:: don't have same subnet ip address!\n");
			syslog_ax_arpsnooping_dbg("\t the ipAddress is %d.%d.%d.%d\n",\
				(ipno>>24)&0xff,(ipno>>16)&0xff,(ipno>>8)&0xff,ipno&0xff);
			syslog_ax_arpsnooping_dbg("\t the intf ips are \n");
			for (i=0;i<MAX_IP_COUNT;i++){
				if(INVALID_HOST_IP != ipAddrs[i]){
					unsigned char *ip_address = NULL;
					ip_address = (unsigned char *)&ipAddrs[i];
		            syslog_ax_arpsnooping_dbg("\t ip %d.%d.%d.%d mask %d.%d.%d.%d\n",\
						ip_address[0],ip_address[1],ip_address[2],ip_address[3],\
						(masks[i]>>24)&0xff,(masks[i]>>16)&0xff,(masks[i]>>8)&0xff,masks[i]&0xff);
				}
			}
			
			ret = ARP_RETURN_CODE_NOT_SAME_SUB_NET;
		}
	}
    return ret;
}

int npd_arp_snooping_ip_legality_check
(
	unsigned int l3Index, 
	unsigned int *gateway,
	unsigned int *mask,
	unsigned int sip, 
	unsigned int dip
)
{
	unsigned int	netif_index = 0;
	unsigned char	proxy_arp = FALSE;

	
	if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_valid_check(sip))
	{
		syslog_ax_arpsnooping_err("bad source ip %d.%d.%d.%d \n", \
				(sip>>24) & 0xFF,(sip>>16) & 0xFF,(sip>>8) & 0xFF,sip & 0xFF);
		return ARP_RETURN_CODE_CHECK_IP_ERROR;
	}
	
	if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_subnet_check(sip, gateway, mask))
	{
		syslog_ax_arpsnooping_err("Subnet check failed:source ip %d.%d.%d.%d \n", \
				(sip>>24) & 0xFF,(sip>>16) & 0xFF,(sip>>8) & 0xFF,sip & 0xFF);
		syslog_ax_arpsnooping_err("Gateway ip %d.%d.%d.%d, mask:  \n", \
				(gateway[0]>>24) & 0xFF,(gateway[0]>>16) & 0xFF,(gateway[0]>>8) & 0xFF,gateway[0] & 0xFF, \
				(mask[0]>>24) & 0xFF,(mask[0]>>16) & 0xFF,(mask[0]>>8) & 0xFF,mask[0] & 0xFF);
		return ARP_RETURN_CODE_ERROR;
	}
	
	if(ARP_RETURN_CODE_SUCCESS == npd_arp_snooping_ip_gateway_check(sip, gateway)){
		if(sip == dip){
			return ARP_RETURN_CODE_CONFLICTED_IP;
		}
		else{
			/*
			syslog_ax_arpsnooping_err("bad source ip %d.%d.%d.%d \n", \
				(sip>>24) & 0xFF,(sip>>16) & 0xFF,(sip>>8) & 0xFF,sip & 0xFF);
			*/
			return ARP_RETURN_CODE_ERROR;
		}
	}
	
	/*ip check, if dip not valid, arp will be ignored*/
	if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_valid_check(dip)){
		return ARP_RETURN_CODE_ERROR;
	}
	
	if(NPD_TRUE != npd_intf_netif_get_by_ifindex(l3Index, &netif_index)){
		return ARP_RETURN_CODE_ERROR;
	}

	/*if proxy arp is enabled,  subnet check will be ignored*/
	if( INTERFACE_RETURN_CODE_SUCCESS == npd_intf_get_proxy_arp(netif_index, &proxy_arp) &&
		TRUE == proxy_arp){
		return ARP_RETURN_CODE_SUCCESS;
	}

	/*subnet check, if dip not belongs to gateway subnet, arp will be ignored*/
	if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_subnet_check(dip, gateway, mask)){
		syslog_ax_arpsnooping_err("bad dest ip %d.%d.%d.%d \n", \
			(dip>>24) & 0xFF,(dip>>16) & 0xFF,(dip>>8) & 0xFF,dip & 0xFF);
		return ARP_RETURN_CODE_ERROR;
	}

	return ARP_RETURN_CODE_SUCCESS;
}

int npd_arp_snooping_print_pkt
(
	unsigned int netif_index, 
	unsigned int ifIndex, 
	unsigned short vid,
	struct arp_packet_t *arpPacket
)
{
	unsigned int sip = 0,dip = 0;
    
    if(0 == (NPD_LOG_MODULE_FLAG_SET(arpsnooping, NPD_LOG_FLAG_PACKET_RCV))) {
		return 0;
	}
	
	sip = IP_CONVERT_STR2ULONG(arpPacket->sip);
	dip = IP_CONVERT_STR2ULONG(arpPacket->dip);
	syslog_ax_arpsnooping_pkt_rev("***********************************\n");
	syslog_ax_arpsnooping_pkt_rev("%-15s:0x%-x\n","intf(l3if)",	netif_index,ifIndex);
	syslog_ax_arpsnooping_pkt_rev("%-15s:%-10d\n","vid",vid);
	syslog_ax_arpsnooping_pkt_rev("%-15s:%d.%d.%d.%d\n","sender ip",	\
					(sip>>24) & 0xFF,(sip>>16) & 0xFF,(sip>>8) & 0xFF,sip & 0xFF);
	syslog_ax_arpsnooping_pkt_rev("%-15s:%02x:%02x:%02x:%02x:%02x:%02x\n","sender mac",	\
			arpPacket->smac[0],arpPacket->smac[1],arpPacket->smac[2],	\
			arpPacket->smac[3],arpPacket->smac[4],arpPacket->smac[5]);
	syslog_ax_arpsnooping_pkt_rev("%-15s:%d.%d.%d.%d\n","target ip", \
					(dip>>24) & 0xFF,(dip>>16) & 0xFF,(dip>>8) & 0xFF,dip & 0xFF);
	syslog_ax_arpsnooping_pkt_rev("%-15s:%02x:%02x:%02x:%02x:%02x:%02x\n","target mac",	\
			arpPacket->dmac[0],arpPacket->dmac[1],arpPacket->dmac[2],	\
			arpPacket->dmac[3],arpPacket->dmac[4],arpPacket->dmac[5]);
	syslog_ax_arpsnooping_pkt_rev("***********************************\n");

	return 0;
}

/************************************************************************************
 *		NPD arp kernel operation
 *
 ************************************************************************************/

int npd_arp_snooping_create_kern_arp
(
	struct arp_snooping_item_s *item
)
{

	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arpreq arp_cfg;
	struct sockaddr_in* sin = NULL,*mask = NULL;

	
	if(sysKernArpSock <= 0) {
		syslog_ax_arpsnooping_err("npd del kern arp sysKernArpSock error %d\r\n",sysKernArpSock);
		return ARP_RETURN_CODE_ERROR;
	}

	memset(&arp_cfg,0,sizeof(arp_cfg));
    sin = (struct sockaddr_in *) &arp_cfg.arp_pa;
    sin->sin_family = AF_INET;
	sin->sin_port = 0;
    sin->sin_addr.s_addr = item->ipAddr;
	arp_cfg.arp_flags = ATF_COM;
	if(item->isStatic) {
		arp_cfg.arp_flags |= ATF_PERM;	
	}
	arp_cfg.arp_ha.sa_family = 1;  /* Ethernet 10Mbps*/
	memcpy(arp_cfg.arp_ha.sa_data,item->mac,MAC_ADDRESS_LEN);
	mask = (struct sockaddr_in *) &arp_cfg.arp_netmask;
	mask->sin_addr.s_addr = 0xffffffff;

    pthread_mutex_lock(&arpKernOpMutex);
	if ((ret = ioctl(sysKernArpSock, SIOCSARP, &arp_cfg) )< 0) {
		syslog_ax_arpsnooping_err("ARP entry kernel create, err %d\n", errno);
		if (errno == EEXIST) {
			pthread_mutex_unlock(&arpKernOpMutex);
			return ARP_RETURN_CODE_SUCCESS;
		}
		pthread_mutex_unlock(&arpKernOpMutex);
		return ARP_RETURN_CODE_ERROR;
	}
	pthread_mutex_unlock(&arpKernOpMutex);
    syslog_ax_arpsnooping_dbg("npd create kern static arp success \n");
	return ARP_RETURN_CODE_SUCCESS;
}

int npd_arp_snooping_del_kern_arp
(
	struct arp_snooping_item_s *item
)
{
	struct arpreq arp_cfg;
	struct sockaddr_in* sin = NULL;
	int flags = 0,deleted = 0;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd del kern arp item is NULL\r\n");
		return ARP_RETURN_CODE_ERROR;
	}
	
	if(sysKernArpSock <= 0) {
		syslog_ax_arpsnooping_err("npd del kern arp sysKernArpSock error %d\r\n",sysKernArpSock);
		return ARP_RETURN_CODE_ERROR;
	}

	memset(&arp_cfg,0,sizeof(arp_cfg));
    sin = (struct sockaddr_in *) &(arp_cfg.arp_pa);
    sin->sin_family = AF_INET;
	sin->sin_port = 0;
    sin->sin_addr.s_addr = item->ipAddr;

	arp_cfg.arp_flags = ATF_COM;
	if( item->isStatic ) {
		arp_cfg.arp_flags |= ATF_PERM;
	}
	arp_cfg.arp_ha.sa_family = 0;  /* Ethernet 10Mbps*/

	if( NPD_FALSE == npd_intf_name_get_by_ifindex(item->l3Index, arp_cfg.arp_dev) )
	{
		syslog_ax_arpsnooping_err("Cannot del kern arp without dev name\n");
		return ARP_RETURN_CODE_ERROR;
	}
	syslog_ax_arpsnooping_dbg("Find device: %s(sizeof(arp_dev) = %d).\r\n", arp_cfg.arp_dev, sizeof(arp_cfg.arp_dev));
	arp_cfg.arp_dev[15]='\0';
	
    /* if neighter priv nor pub is given, work on both*/
    if (flags == 0)
		flags = 3;
    /* unfortuatelly the kernel interface does not allow us to
       delete private entries anlone, so we need this hack
       to avoid "not found" errors if we try both. */
    deleted = 0;

    /* Call the kernel. */
	pthread_mutex_lock(&arpKernOpMutex);
    if (flags & 2) 
	{
		if (ioctl(sysKernArpSock, SIOCDARP, &arp_cfg) < 0) {
			syslog_ax_arpsnooping_err("ARP entry kernel del dontpub, err %d\n", errno);
		    if ((errno == ENXIO) || (errno == ENOENT)) {				
				if (flags & 1)
				    goto dontpub;
				pthread_mutex_unlock(&arpKernOpMutex);
				return (ARP_RETURN_CODE_SUCCESS);
		    }
			pthread_mutex_unlock(&arpKernOpMutex);
		    return (ARP_RETURN_CODE_ERROR);
		} 
		else
			deleted = 1;
    }
		
    if (!deleted && (flags & 1)) {
	    dontpub:
		arp_cfg.arp_flags |= ATF_PUBL;
		if (ioctl(sysKernArpSock, SIOCDARP, &arp_cfg) < 0) {
			syslog_ax_arpsnooping_err("ARP entry kernel del, err %d\n", errno);
		    if ((errno == ENXIO) || (errno == ENOENT)) {
				pthread_mutex_unlock(&arpKernOpMutex);
				return (ARP_RETURN_CODE_SUCCESS);
		    }
			pthread_mutex_unlock(&arpKernOpMutex);
		    return (ARP_RETURN_CODE_ERROR);
		}
    }
	pthread_mutex_unlock(&arpKernOpMutex);
    syslog_ax_arpsnooping_dbg("delete kernal arp : %d.%d.%d.%d success.\n",\
		(item->ipAddr>>24&0xff),(item->ipAddr>>16&0xff),\
		(item->ipAddr>>8&0xff),(item->ipAddr&0xff));
	return ARP_RETURN_CODE_SUCCESS;
}

int npd_arp_snooping_lookup_arpinfo
(
	unsigned int ifindex,
	unsigned short vlanId,
	unsigned char* pktdata,
	unsigned int *netifIndex
)
{
	struct ether_header_t* layer2 = NULL;
	struct arp_snooping_item_s p_arp_info;
	int status;

	if((NULL == pktdata)||(NULL == netifIndex)){
		return ARP_RETURN_CODE_ERROR;
	}

	syslog_ax_arpsnooping_dbg("Find arp for TX: L3index 0x%x vid %d\n", ifindex, vlanId);
	
	if(NPD_TRUE == npd_check_vlan_exist(vlanId))
	{
		layer2 = (struct ether_header_t*) pktdata;
		
		status = npd_arp_snooping_find_item_bymacvid(layer2->dmac, vlanId, &p_arp_info);
		if( 0 == status)
		{
			*netifIndex = p_arp_info.ifIndex;
			return ARP_RETURN_CODE_SUCCESS;
		}
	}
	
	return ARP_RETURN_CODE_ERROR;
}

/************************************************************************************
 *		NPD arp and nexthop hash operation
 *
 ************************************************************************************/
unsigned int npd_arp_snooping_key_generate
(
	void *data
)
{
	unsigned int key = 0;

	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.\r\n");
		return ~0UI;
	}

    key = jhash_1word(item->ipAddr, 0x35798642);
    key %= NPD_ARPSNP_HASH_IP_SIZE;

	return key;
}


unsigned int npd_arp_snooping_key_port_generate
(
	void *data	
)
{
	unsigned int key = 0; /*for hash key calculate*/
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.\r\n");
		return ~0UI;
	}

	key = (item->ifIndex >> 14)& 0x3FF ;
	
	key %= (NPD_ARPSNP_HASH_PORT_SIZE);
	
	return key;
}


unsigned int npd_arp_snooping_key_mac_generate
(
	void *data	
)
{
	unsigned int key = 0; /*for hash key calculate*/
	unsigned char mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	unsigned int tmpData = 0;
	unsigned int tmpData1 = 0;
	unsigned int tmpData2 = 0;
	int i = 0;
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.\r\n");
		return ~0UL;
	}
	memcpy(mac,item->mac,MAC_ADDR_LEN);
	for(i = 0; i < 3; i++){
		tmpData1 = (tmpData1<<8) + mac[i];
	}
	for(i = 3; i < 6; i++){
		tmpData2 = (tmpData2<<8) + mac[i];
	}

	tmpData = tmpData1 + tmpData2;
	key = tmpData%(NPD_ARPSNP_HASH_MAC_SIZE);
	
	return key;
}

unsigned int npd_arp_snooping_compare
(
	void * data1,
	void * data2	
)
{
	int equal = TRUE;

	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 )
	{
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;

	if(itemA->ifIndex!=itemB->ifIndex) {/* L3 intf index*/
		equal = FALSE;
	    return equal;
	}

	if(0 != memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN)) { /* MAC*/
		equal = FALSE;
	}
	else if(itemA->ipAddr != itemB->ipAddr) { /* IP Address*/
		equal = FALSE;
	}
	return equal;
}


unsigned int npd_arp_snooping_compare_byip
(
	void * data1,
	void * data2	
)
{
	int equal = TRUE;

	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;

	if(itemA->ipAddr != itemB->ipAddr) { /* IP Address*/
		equal = FALSE;
	}

	return equal;

}

unsigned int npd_arp_snooping_filter_by_ifindex
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;
	

	if( itemA->ifIndex != itemB->ifIndex ) {
		return FALSE;
	}

	return TRUE;
}


unsigned int npd_arp_snooping_filter_by_ifindex_vid
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;
	

	if( itemA->ifIndex != itemB->ifIndex ) {
		return FALSE;
	}
    if( itemA->vid != itemB->vid) {
		return FALSE;
	}

	return TRUE;
}

unsigned int npd_arp_snooping_filter_by_l3index
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;
	
	if( itemA->l3Index != itemB->l3Index ) {
		return FALSE;
	}

	return TRUE;
}


unsigned int npd_arp_snooping_filter_by_vid
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;

	if( itemA->vid != itemB->vid )
		return FALSE;

	return TRUE;
}


unsigned int npd_arp_snooping_filter_by_mac
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;
	
	if( memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN) ){
		return FALSE;
	}

	return TRUE;
}


unsigned int npd_arp_snooping_filter_by_macvid
(
	void * data1,
	void * data2	
)
{
	struct arp_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}

	itemA = (struct arp_snooping_item_s *)data1;
	itemB = (struct arp_snooping_item_s *)data2;
	
	if( memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN) ){
		return FALSE;
	}
	else if(itemA->vid != itemB->vid ) {
		return FALSE;
	}
		
	return TRUE;
}


unsigned int npd_arp_snooping_filter_by_static
(
	void *data1,
	void* data2
)
{
	struct arp_snooping_item_s *itemA = (struct arp_snooping_item_s *)data1;
	struct arp_snooping_item_s *itemB = (struct arp_snooping_item_s *)data2;
	
	if((NULL == itemA)||(NULL == itemB))
		return FALSE;

	if(itemA->isStatic != itemB->isStatic )
		return FALSE;

	return TRUE;
}

unsigned int npd_arp_snooping_filter_by_network
(
    void *data1,
    void *data2
)
{
	struct arp_snooping_item_s * itemA = (struct arp_snooping_item_s *)data1;
	unsigned int *ip_mask = (unsigned int *)data2;
	
    if((NULL == itemA)||(NULL == ip_mask)){
		return FALSE;
    }
	if(((itemA->ipAddr) & ip_mask[1]) == (ip_mask[0] & ip_mask[1])){
		return TRUE;
	}
	return FALSE;
}

int npd_arp_snooping_find_item_byip
(
	unsigned int  ipAddr,
	struct arp_snooping_item_s *dbItem
)
{
	struct arp_snooping_item_s data;
	unsigned char *ip_address = NULL;
	int status;

	memset(&data,0,sizeof(struct arp_snooping_item_s));
	data.ipAddr = ipAddr;

	status = dbtable_hash_search(npd_arpsnp_haship_index,&data,
		npd_arp_snooping_compare_byip,dbItem);
	if(0 != status ) {
		syslog_ax_arpsnooping_dbg("not found arp item of %d.%d.%d.%d\n", \
			(ipAddr>>24)&0xFF,(ipAddr>>16)&0xFF,(ipAddr>>8)&0xFF,(ipAddr)&0xFF);
		return -1; /* return null pointer*/
	}
	/*ipAddr in BIG Endian*/
    ip_address = (unsigned char *)&dbItem->ipAddr;
	syslog_ax_arpsnooping_dbg("found arp item %d.%d.%d.%d on port_index(0x%x) vlan %d if %d\r\n",	\
			ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],	\
			dbItem->ifIndex,dbItem->vid,dbItem->l3Index);
	return 0;
}

int npd_arp_snooping_find_item_bymac
(
	unsigned char*  macAddr,
	struct arp_snooping_item_s *dbItem
)
{
	struct arp_snooping_item_s data;
	unsigned char *ip_address = NULL;
	int status;
    if(NULL == macAddr)
    {
    	return -1;
    }
	memset(&data,0,sizeof(struct arp_snooping_item_s));
	memcpy(data.mac, macAddr, MAC_ADDR_LEN);

	status = dbtable_hash_search(npd_arpsnp_hashmac_index,&data,
		npd_arp_snooping_filter_by_mac,dbItem);
	if(0 != status ) {
		syslog_ax_arpsnooping_dbg("not found arp item of %02x:%02x:%02x:%02x:%02x:%02x\n", \
			macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
		return -1; /* return null pointer*/
	}
	/*ipAddr in BIG Endian*/
    ip_address = (unsigned char *)&dbItem->ipAddr;
	syslog_ax_arpsnooping_dbg("found arp item %d.%d.%d.%d on port_index(0x%x) vlan %d if %d\r\n",	\
			ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],	\
			dbItem->ifIndex,dbItem->vid,dbItem->l3Index);
	return 0;
}


int npd_arp_snooping_find_item_bymacvid
(
	unsigned char*  macAddr,
	unsigned short vlanId,
	struct arp_snooping_item_s *dbItem
)
{
	struct arp_snooping_item_s data;
	unsigned char *ip_address = NULL;
	int status;
    if(NULL == macAddr)
    {
    	return -1;
    }
	memset(&data,0,sizeof(struct arp_snooping_item_s));
	memcpy(data.mac, macAddr, MAC_ADDR_LEN);
	data.vid = vlanId;
	
	status = dbtable_hash_search(npd_arpsnp_hashmac_index,&data,npd_arp_snooping_filter_by_macvid,dbItem);
	if(0 != status ) {
		syslog_ax_arpsnooping_dbg("not found arp item of %02x:%02x:%02x:%02x:%02x:%02x\n", \
			macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
		return -1; /* return null pointer*/
	}
	/*ipAddr in BIG Endian*/
    ip_address = (unsigned char *)&dbItem->ipAddr;
	syslog_ax_arpsnooping_dbg("found arp item %d.%d.%d.%d on port_index(0x%x) vlan %d if %d\r\n",	\
			ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],	\
			dbItem->ifIndex,dbItem->vid,dbItem->l3Index);
	return 0;
}

int npd_arp_snooping_show_table(void* entry, char *buf, int buf_len)
{
    int total_len = 0, current_ptr = 0;
	unsigned char *ip_address = NULL;
	char tmp_buf[1024];
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)entry;
	memset(tmp_buf, 0, 1024);
	if(NULL == item)/*print db header*/
	{
		total_len = sprintf(tmp_buf, "%-8s %-15s %-17s %-4s %-6s %-6s\n",	\
				"IFINDEX","IP ADDRESS","MAC ADDRESS","VID","TAG","PORT");
		current_ptr = total_len;
		total_len += sprintf(tmp_buf + current_ptr, "-------- --------------- ----------------- ---- ------ ------ ----\n");
		current_ptr = total_len;
	}
	else
	{
	/*ipAddr in BIG Endian*/
        ip_address = (unsigned char *)&item->ipAddr;
	    total_len += sprintf(tmp_buf + current_ptr, "%-8x %-3d.%-3d.%-3d.%-3d %02x:%02x:%02x:%02x:%02x:%02x %-4d %-6s %-6d\n",	\
			item->l3Index,ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],item->mac[0],item->mac[1],	\
			item->mac[2],item->mac[3],item->mac[4],item->mac[5],item->vid,	\
			item->isTagged ? "TRUE":"FALSE",item->ifIndex);
	}
	if(total_len >= buf_len)
	{
		  return -1;  
	}
	else
	{
	    memcpy(buf, tmp_buf, total_len);
	    return total_len;
	}
}


unsigned int npd_route_nexthop_compare
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct route_nexthop_brief_s *itemA = (struct route_nexthop_brief_s *)data1;
	struct route_nexthop_brief_s *itemB = (struct route_nexthop_brief_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.\r\n");
		return FALSE;
	}
		
	if(itemA->ipAddr != itemB->ipAddr) {	/* ip address*/
		equal = FALSE;
	}
	
	return equal;
}

unsigned int npd_route_nexthop_key_generate
(
	void *data
)
{
	unsigned int key = 0;
	struct route_nexthop_brief_s *item = (struct route_nexthop_brief_s *)data;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.\r\n");
		return ~0UI;
	}

    key = jhash_1word(item->ipAddr, 0x35798642);
    key %= NPD_ARPSNP_HASH_IP_SIZE;
	
	return key;
}


int npd_route_nexthop_find
(	
	unsigned int ipAddr,
	struct route_nexthop_brief_s *nexthopEntry
)
{
	int ret = 0;
	if( NULL == nexthopEntry )
		return NPD_FALSE;
		
	nexthopEntry->ipAddr  = ipAddr;

	ret = dbtable_hash_search(npd_nexthop_hash_index, nexthopEntry, NULL, nexthopEntry);
	if( 0 == ret )
	{
		npd_syslog_dbg("npd get nexthop tbl index %#0x for if %#0x ip %#0x ref %d\r\n",	\
						nexthopEntry->tblIndex,nexthopEntry->l3Index,nexthopEntry->ipAddr, nexthopEntry->rtUsedCnt);
		return NPD_TRUE;
	}

	npd_syslog_dbg("npd not found nexthop tbl index for ip %#0x\r\n", ipAddr);

	return NPD_FALSE;	
}



int npd_route_nexthop_op_item
(
	struct route_nexthop_brief_s *item,
	enum NPD_NEXTHOP_DB_ACTION action,
	unsigned int dupIfIndex
)
{
	int ret = 0;
	struct route_nexthop_brief_s data;

	if(NULL == item) {
		syslog_ax_arpsnooping_err("npd %s nexthop brief item null pointer error.\r\n",(NEXTHOP_ADD_ITEM==action) ? "add":"del");
		return COMMON_RETURN_CODE_NULL_PTR;
	}
	syslog_ax_arpsnooping_dbg("npd nexthop op %d for ip 0x%x l3index 0x%x tblIndex %d\n", action, item->ipAddr, item->l3Index, item->tblIndex);

	ret = dbtable_hash_search(npd_nexthop_hash_index, item, NULL, &data);

	if(NEXTHOP_ADD_ITEM == action) {
		if(0 == ret) {
			ret = dbtable_hash_update(npd_nexthop_hash_index, &data, item);
			syslog_ax_arpsnooping_dbg("npd nexthop brief dup item ip %#0x found when add.\r\n", data.ipAddr);
			return ROUTE_RETURN_CODE_SUCCESS;
		}

		ret = dbtable_hash_insert(npd_nexthop_hash_index, item);		
	}
	else if(NEXTHOP_DEL_ITEM == action)
	{
		if(0 != ret) {
			syslog_ax_arpsnooping_err("npd nexthop brief no item found when delete.\r\n");
			return ROUTE_RETURN_CODE_SUCCESS;
		}

		ret = dbtable_hash_delete(npd_nexthop_hash_index, &data, &data);
	}
	
	return ROUTE_RETURN_CODE_SUCCESS;	
}


int npd_route_nexthop_tblindex_compare
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct route_nexthop_brief_s *itemA = (struct route_nexthop_brief_s *)data1;
	struct route_nexthop_brief_s *itemB = (struct route_nexthop_brief_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.\r\n");
		return FALSE;
	}

	if(itemA->tblIndex != itemB->tblIndex) {	/* HW table index*/
		equal = FALSE;
	}
	
	return equal;
}


int npd_route_nexthop_get_by_tblindex
(
	unsigned int routeEntryIndex,
	struct route_nexthop_brief_s *nexthopEntry
)
{
	int ret = 0;
	if(nexthopEntry == NULL )
		return NPD_FALSE;

	nexthopEntry->tblIndex = routeEntryIndex;
	
	ret = dbtable_hash_search(npd_nexthop_hash_index, nexthopEntry, NULL, nexthopEntry);

	if( 0 != ret ) {
		return NPD_FALSE;
	}

	syslog_ax_arpsnooping_dbg("npd got nexthop info index %#0x by tbl index %#0x ip %#0x\r\n",	\
						nexthopEntry->l3Index,nexthopEntry->tblIndex,nexthopEntry->ipAddr);
	
	return NPD_TRUE;	
}



unsigned int npd_route_nexthop_tblindex_find
(
	unsigned int l3Index,
	unsigned int ipAddr,
	struct route_nexthop_brief_s *next_hop_entry
)
{	
	int ret = 0;
	unsigned char *ip_address = NULL;
	ret = npd_route_nexthop_find(ipAddr, next_hop_entry);
	if(NPD_TRUE != ret)
	{
		return ROUTE_RETURN_CODE_NO_RESOURCE;
	}
	/*ipAddr in BIG Endian*/
    ip_address = (unsigned char *)&ipAddr;
	syslog_ax_arpsnooping_dbg("npd find nexthop index %#0x ip %d.%d.%d.%d ref %d\r\n",	\
								l3Index,ip_address[0],ip_address[1],ip_address[2],		\
								ip_address[3],next_hop_entry->rtUsedCnt);

	return ROUTE_RETURN_CODE_SUCCESS;
}


unsigned int npd_route_nexthop_tblindex_get
(
	unsigned int l3Index,
	unsigned int ipAddr,
	unsigned int *tblIndex
)
{
	int ret = 0;
	struct route_nexthop_brief_s data;
	unsigned char *ip_address = NULL;

	ret = npd_route_nexthop_find(ipAddr, &data);
	if(NPD_TRUE != ret) {
		return ROUTE_RETURN_CODE_NO_RESOURCE;
	}
	else {
		*tblIndex = data.tblIndex;
	    /*ipAddr in BIG Endian*/
        ip_address = (unsigned char *)&ipAddr;
		syslog_ax_arpsnooping_dbg("npd got nexthop tbl index %#0x for index %#0x ip %d.%d.%d.%d ref %d\r\n",	\
										*tblIndex,l3Index,ip_address[0],ip_address[1],ip_address[2],		\
										ip_address[3],data.rtUsedCnt);
	}

	return ROUTE_RETURN_CODE_SUCCESS;
}
unsigned int npd_route_nexthop_iteminfo_get
(
	unsigned char devNum,
	unsigned int tblIndex,
	struct arp_snooping_item_s *item,	
	unsigned int *refCnt
)
{
	struct route_nexthop_brief_s routeData;
	unsigned int retVal = ROUTE_RETURN_CODE_SUCCESS;

	if(NULL == item) {
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	retVal = nam_arp_snooping_get_item(devNum,tblIndex,item);
	if(ARP_RETURN_CODE_ACTION_TRAP2CPU == retVal) {
		syslog_ax_arpsnooping_dbg("npd get route nexthop info from hw table at %#0x command TRAP_TO_CPU\r\n",tblIndex);
		retVal = npd_route_nexthop_get_by_tblindex(tblIndex, &routeData);
		if(ROUTE_RETURN_CODE_SUCCESS == retVal) {
			item->l3Index = routeData.l3Index;
			*refCnt = routeData.rtUsedCnt;
		}
		else { /* not found */
			item->l3Index = 0;
			*refCnt = 0;
		}
		retVal = ROUTE_RETURN_CODE_ACTION_TRAP2CPU;
	}
	else if(ARP_RETURN_CODE_ACTION_HARD_DROP == retVal) {
		syslog_ax_arpsnooping_dbg("npd get route nexthop info from hw table at %#0x command HEAD_DROP\r\n",tblIndex);
		retVal = ROUTE_RETURN_CODE_ACTION_HARD_DROP;
	}
	else if(ARP_RETURN_CODE_SUCCESS != retVal) {
		syslog_ax_arpsnooping_err("npd get route nexthop info from hw table at %#0x error\r\n",tblIndex);
		retVal = ROUTE_RETURN_CODE_ERROR;
	}
	else {		
		retVal = npd_route_nexthop_get_by_tblindex(tblIndex, &routeData);
		if(ROUTE_RETURN_CODE_SUCCESS == retVal) {
			item->l3Index = routeData.l3Index;
			*refCnt = routeData.rtUsedCnt;
		}
		else { /* not found */
			item->l3Index = 0;
			*refCnt = 0;
		}
		retVal = ROUTE_RETURN_CODE_SUCCESS;
	}
	
	return retVal;	
}

void npd_route_nexthop_show_item
(
	void *data,
	char *title
)
{
	struct route_nexthop_brief_s *item = (struct route_nexthop_brief_s *)data;
    unsigned char *ip_address = NULL;
	if(NULL == item){
		return;
	}
    ip_address = (unsigned char *)&item->ipAddr;
	syslog_ax_arpsnooping_dbg("%-8x %-3d.%-3d.%-3d.%-3d %-8x\n",	\
			item->l3Index,ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],item->tblIndex);
	return;
}

void npd_route_nexthop_show_table
(
	void
)
{
	syslog_ax_arpsnooping_dbg("%-8s %-15s %-12s\n","IFINDEX","IP ADDRESS","TABLE INDEX");
	syslog_ax_arpsnooping_dbg("-------- --------------- ------------\n");
	
	dbtable_hash_show(npd_nexthop_hash_index,"",npd_route_nexthop_show_item);
	return;
}

unsigned int npd_arp_snooping_count_all
(
    void
)
{
    unsigned int count = 0;
	count = dbtable_hash_count(npd_arpsnp_haship_index);
	return count;
}

/************************************************************************************
 *		NPD arp add/del operation
 *
 ************************************************************************************/
	

 int npd_arp_snooping_create_static_arp
(
	unsigned int L3ifindex,
	unsigned int ipAddr,
	unsigned int ipMask,
	unsigned char* mac,
	unsigned short vid,
	unsigned int eth_g_index
)
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s arpItem,arpInfo;
	unsigned char isTagged = FALSE;
	unsigned int arpCount = 0;
	struct arp_snooping_item_s dupItem;
	unsigned int l3intf_status = 0, netif_status = 0;
	int status;
	
	arpCount = npd_arp_snooping_count_all();
	if( NPD_ARPSNP_TABLE_SIZE <= arpCount){
		npd_arp_snooping_drop_handle();
		arpCount = npd_arp_snooping_count_all();
		if( NPD_ARPSNP_TABLE_SIZE <= arpCount)
		{
			syslog_ax_arpsnooping_err("get static arp count %d \n",arpCount);
			return ARP_RETURN_CODE_STATIC_ARP_FULL;
		}
	}
	memset(&arpItem,0,sizeof(struct arp_snooping_item_s));
	memset(&dupItem,0,sizeof(struct arp_snooping_item_s));
	if(NPD_MAX_VLAN_ID != vid)
	{
	    isTagged = npd_vlan_check_port_membership(vid,eth_g_index,TRUE);
	}

	l3intf_status = npd_intf_get_l3intf_status_by_ifindex(L3ifindex);
	npd_check_netif_status(eth_g_index, (int *)&netif_status);
	syslog_ax_arpsnooping_dbg("static ip info ifindex %d,ipAddr %#02x ,ipMask %d,\r\n",L3ifindex,ipAddr,ipMask);
	arpItem.l3Index = L3ifindex;
	arpItem.ipAddr = ipAddr;
	arpItem.isStatic = TRUE;
	arpItem.isTagged = isTagged;
	arpItem.isValid = (l3intf_status & netif_status);
	arpItem.vid = vid;
	arpItem.ifIndex = eth_g_index;
	memcpy(arpItem.mac,mac,MAC_ADDRESS_LEN);
	/* 这个添加的地方也得加上锁 */
	dbtable_hash_lock(npd_arpsnp_haship_index);

	status = npd_arp_snooping_find_item_byip(ipAddr, &arpInfo);
	if((0 != status) || (FALSE == arpInfo.isStatic)) 
	{
		if( 0 == status )
		{
			status = dbtable_hash_delete(npd_arpsnp_haship_index, &arpInfo, &arpInfo);
		}
		
		if((ARP_RETURN_CODE_SUCCESS != npd_arp_snooping_create_kern_arp(&arpItem))){/*kernal success */
			arpItem.isValid = FALSE;
		}		
		syslog_ax_arpsnooping_dbg("static arp insert: ip 0x%x, l3index 0x%x, ifindex 0x%x vlan %d\r\n", arpItem.ipAddr,\
							arpItem.l3Index, arpItem.ifIndex, arpItem.vid);
		status = dbtable_hash_insert( npd_arpsnp_haship_index, &arpItem);
#ifdef HAVE_ROUTE
		npd_route_update_by_nhp(ipAddr, arpItem.isValid);
		npd_policy_route_update_by_nhp(ipAddr, arpItem.isValid);
#endif
#ifdef HAVE_M4_TUNNEL
    npd_tunnel_update_by_arp(ipAddr, arpItem.isValid);
#endif
		if( status == 0 ) {
			ret = ARP_RETURN_CODE_SUCCESS;					
		}		
	}
	else{
		ret = ARP_RETURN_CODE_STATIC_EXIST;
	}
	dbtable_hash_unlock(npd_arpsnp_haship_index);
	return ret;
}


int npd_arp_snooping_del
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
)
{
	int ret;
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;

	if( item == NULL )
		return ARP_RETURN_CODE_ERROR;
		
	if(TRUE == item->isStatic) {
		return ARP_RETURN_CODE_NOTEXISTS;
	}
	
    ret = npd_arp_snooping_del_all(hash, item,kern_del_flag);

	return ret; 
}


int npd_arp_snooping_del_static
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
)
{
	/*not static arp don't delete*/
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;
	if( item == NULL)
		return ARP_RETURN_CODE_ERROR;
	
	if(TRUE != item->isStatic) {
		return ARP_RETURN_CODE_STASTIC_NOTEXIST;
	}
	
    return npd_arp_snooping_del_all(NULL, item,kern_del_flag);
}


int npd_arp_snooping_del_all
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag    
)
{
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;
	int ret = ARP_RETURN_CODE_SUCCESS;
	int status = 0;	
	
	if(NULL == item) {
		return COMMON_RETURN_CODE_NULL_PTR;
	}
	/*first del kernel arp*/
	if((kern_del_flag == TRUE)&&\
		((FALSE == item->isStatic)||(TRUE == item->isValid))) 
	{
		npd_arp_snooping_del_kern_arp(item);
	}
#ifdef HAVE_ROUTE
	status = npd_route_update_by_nhp(item->ipAddr, FALSE);
#endif
#ifdef HAVE_M4_TUNNEL
    npd_tunnel_update_by_arp(item->ipAddr, FALSE);
#endif
	status = dbtable_hash_delete(npd_arpsnp_haship_index, item, NULL);

	status = npd_policy_route_update_by_nhp(item->ipAddr, FALSE);

	return ret;	
}


unsigned int npd_arp_snooping_del_by_ifindex
(
	unsigned int   eth_g_index
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));
	comparator.ifIndex = eth_g_index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index,kern_del_flag,&comparator,\
									npd_arp_snooping_filter_by_ifindex,npd_arp_snooping_del);
	
	syslog_ax_arpsnooping_dbg("npd clear arp on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index,devNum,portNum, retVal);
	return retVal;
}

unsigned int npd_arp_snooping_del_by_ifindex_vid
(
	unsigned int   eth_g_index,
	unsigned short vlan_id
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));
	comparator.ifIndex = eth_g_index;
    comparator.vid = vlan_id;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index, kern_del_flag,&comparator,\
									npd_arp_snooping_filter_by_ifindex_vid, npd_arp_snooping_del);
	
	syslog_ax_arpsnooping_dbg("npd clear arp on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index, devNum, portNum, retVal);
	return retVal;
}

unsigned int npd_arp_snooping_del_by_l3index
(	
	unsigned int   L3ifindex
)
{
	unsigned int retVal = 0;
	struct arp_snooping_item_s data;
	unsigned int kern_del_flag = TRUE;

	memset(&data, 0,sizeof(struct arp_snooping_item_s));
	data.l3Index = L3ifindex;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal(npd_arpsnp_hashport_index,kern_del_flag,&data,\
								npd_arp_snooping_filter_by_l3index,npd_arp_snooping_del);
	
	syslog_ax_arpsnooping_dbg("npd clear %d arp items on l3 index %#0x \n",retVal,L3ifindex);
	
	return retVal;
}

unsigned int npd_arp_snooping_del_by_network(unsigned int ip,unsigned int mask)
{			
	struct arp_snooping_item_s item;
	unsigned int ip_mask[2] = {0};
	unsigned int delCount = 0;
	ip_mask[0] = ip;
	ip_mask[1] = mask;
	memset(&item,0,sizeof(struct arp_snooping_item_s));
	delCount = dbtable_hash_traversal(npd_arpsnp_haship_index,NPD_TRUE,ip_mask,\
									npd_arp_snooping_filter_by_network,npd_arp_snooping_del);
	if(delCount > 0){
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",delCount);
	}
	
	return ARP_RETURN_CODE_SUCCESS;
}


unsigned int npd_arp_snooping_del_static_by_network(unsigned int ip,unsigned int mask)
{			
	struct arp_snooping_item_s item;
	unsigned int ip_mask[2] = {0};
	unsigned int delCount = 0;
	ip_mask[0] = ip;
	ip_mask[1] = mask;
	memset(&item,0,sizeof(struct arp_snooping_item_s));
	delCount = dbtable_hash_traversal(npd_arpsnp_haship_index,NPD_TRUE,ip_mask,\
									npd_arp_snooping_filter_by_network,npd_arp_snooping_del_static);
	if(delCount > 0){
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",delCount);
	}
	
	return ARP_RETURN_CODE_SUCCESS;
}

unsigned int npd_arp_snooping_del_static_by_ifindex
(
	unsigned int   eth_g_index
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));
	comparator.ifIndex = eth_g_index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index,kern_del_flag,&comparator,\
									npd_arp_snooping_filter_by_ifindex,npd_arp_snooping_del_static);
	
	syslog_ax_arpsnooping_dbg("npd clear static arp on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index,devNum,portNum, retVal);
	return retVal;
}

unsigned int npd_arp_snooping_del_static_by_ifindex_vid
(
	unsigned int   eth_g_index,
	unsigned short vlan_id
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0, sizeof(struct arp_snooping_item_s));
	comparator.ifIndex = eth_g_index;
    comparator.vid=vlan_id;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index, kern_del_flag, &comparator,\
									npd_arp_snooping_filter_by_ifindex_vid, npd_arp_snooping_del_static);
	
	syslog_ax_arpsnooping_dbg("npd clear static arp on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index,devNum,portNum, retVal);
	return retVal;
}

unsigned int npd_arp_snooping_del_static_by_l3index
(
	unsigned int l3index
)
{
	unsigned int retVal = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));
	comparator.l3Index = l3index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal(npd_arpsnp_hashport_index,kern_del_flag,&comparator,\
									npd_arp_snooping_filter_by_l3index,npd_arp_snooping_del_static);
	
	syslog_ax_arpsnooping_dbg("npd clear static arp on l3index %#x total %d items deleted\n", \
					l3index,retVal);
	return retVal;
}


 int npd_arp_snooping_del_static_by_mac_L3index
(
	unsigned int ifindex,
	unsigned int ipAddr,
	unsigned int ipMask,
	unsigned char* mac,
	unsigned short vid,
	unsigned int eth_g_index
)
{
	int ret = 0;
	struct arp_snooping_item_s arpInfo;

	int status;
	
	memset(&arpInfo,0,sizeof(struct arp_snooping_item_s));

	status = npd_arp_snooping_find_item_byip(ipAddr, &arpInfo);
	if((0 != status)||(NPD_TRUE != arpInfo.isStatic)) {
		ret = ARP_RETURN_CODE_STASTIC_NOTEXIST;
	}
	else if(arpInfo.ifIndex != eth_g_index){
		ret = ARP_RETURN_CODE_PORT_NOTMATCH;/* arp item port not consistent*/
	}
	else if(memcmp(arpInfo.mac, mac, MAC_ADDR_LEN))
	{
		ret = ARP_RETURN_CODE_NOTCONSISTENT;
	}
	else { /* all check success*/
		ret = npd_arp_snooping_del_all(NULL, &arpInfo,NPD_TRUE);
	}
	return ret;
}

unsigned int npd_arp_snooping_del_all_by_ifindex
(
	unsigned int   eth_g_index
)
{
	unsigned int retVal = 0;
	struct arp_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));
	comparator.ifIndex = eth_g_index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index,kern_del_flag,&comparator,\
											npd_arp_snooping_filter_by_ifindex,npd_arp_snooping_del_all);
	
	syslog_ax_arpsnooping_dbg("npd clear arp on eth-port %#x total %d items deleted\n", \
					eth_g_index, retVal);
	return retVal;
}



int npd_arp_snooping_dyntostatic
(
    hash_table_index_t *hash,
	void *data,
	unsigned int flag
)
{
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;
	int ret = ARP_RETURN_CODE_SUCCESS;
	int status = 0;	
	unsigned char *ip_address = NULL;
	
	if(NULL == item) {
		return COMMON_RETURN_CODE_NULL_PTR;
	}
    ip_address = (unsigned char *)&item->ipAddr;
	syslog_ax_arpsnooping_dbg("npd transform dyn to static ifindex %#x ip %d.%d.%d.%d vlan %d\r\n", \
			item->ifIndex,ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3], item->vid);

	if((TRUE == item->isStatic)||(ARPSNP_FLAG_DROP & item->flag))
	{
		syslog_ax_arpsnooping_err("npd transform syn to static ifindex %#x ip %d.%d.%d.%d is static in sw hash table\r\n", \
		item->ifIndex,ip_address[0],ip_address[1],	\
		ip_address[2],ip_address[3]);
		return ARP_RETURN_CODE_SUCCESS;
	}

	item->isStatic = TRUE;
	status = npd_arp_snooping_create_kern_arp(item);
	status = dbtable_hash_update(hash, item, item);

	return ret;	
}

/************************************************************************************
 *		NPD arp valid invalid operation
 *
 ************************************************************************************/


int npd_arp_snooping_static_valid_set
(
	hash_table_index_t *hash, 
	void *data,
	unsigned int flag
)
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s *item = (struct arp_snooping_item_s *)data;

	if( NULL == item )
		return ARP_RETURN_CODE_ERROR;

	if( item->isStatic != TRUE || flag == item->isValid ) {
		syslog_ax_arpsnooping_err(" Arp 0x%x check flag fail: static %d Valid %d, flag %d\r\n",
										item->ipAddr,item->isStatic, item->isValid, flag);
		return ARP_RETURN_CODE_ERROR;
	}	

	if( flag == FALSE )
	{
#ifdef HAVE_ROUTE
		ret = npd_route_update_by_nhp(item->ipAddr, FALSE);
#endif
#ifdef HAVE_M4_TUNNEL
        npd_tunnel_update_by_arp(item->ipAddr, FALSE);
#endif
		ret = npd_arp_snooping_del_kern_arp(item);
		
	}
	else{
		if(INTF_STATE_UP_E != npd_intf_get_l3intf_status_by_ifindex(item->l3Index))
		{
			syslog_ax_arpsnooping_err(" Arp check flag fail: netintf 0x%x is not up\r\n", item->ifIndex);
			return ARP_RETURN_CODE_ERROR;
		}
		
		if( INTF_STATE_UP_E != npd_netif_get_status(item->ifIndex) )
		{
			syslog_ax_arpsnooping_err(" Arp check flag fail: netintf 0x%x is not up\r\n", item->ifIndex);
			return ARP_RETURN_CODE_ERROR;
		}
		
		ret = npd_arp_snooping_create_kern_arp(item);
	}

	item->isValid = flag;
	
	ret = dbtable_hash_update(npd_arpsnp_haship_index, item, item);
	if( 0 != ret ) {
		syslog_ax_arpsnooping_err(" Arp 0x%x update flag fail: flag %d\r\n", item->ipAddr, flag );
		return ARP_RETURN_CODE_ERROR;
	}
#ifdef HAVE_ROUTE
	else if(flag == TRUE )
	{
		ret = npd_route_update_by_nhp(item->ipAddr, TRUE);
		ret = npd_policy_route_update_by_nhp(item->ipAddr, TRUE);
	}
	else if (flag == FALSE)
	{
		ret = npd_policy_route_update_by_nhp(item->ipAddr, FALSE);		
	}
#endif

#ifdef HAVE_M4_TUNNEL
    npd_tunnel_update_by_arp(item->ipAddr, (flag == TRUE) ? TRUE : FALSE);
#endif
	return ARP_RETURN_CODE_SUCCESS;
}



unsigned int npd_arp_snooping_static_valid_set_by_ifindex
(
	unsigned int   ifIndex,
	unsigned int isValid
)
{
	unsigned int retVal = 0;
	struct arp_snooping_item_s comparator;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));	
	comparator.ifIndex = ifIndex;
	
	retVal = dbtable_hash_traversal_key(npd_arpsnp_hashport_index, isValid,&comparator,\
					npd_arp_snooping_filter_by_ifindex,npd_arp_snooping_static_valid_set);

	if(retVal > 0){
	    syslog_ax_arpsnooping_dbg("npd static arp set %s on eth-port %#x total %d items set\n", \
					    isValid ? "Valid":"Invalid",ifIndex,retVal);
	}
	return retVal;
}

unsigned int npd_arp_snooping_static_valid_set_by_l3index
(
	unsigned int   ifIndex,
	unsigned int isValid
)
{
	unsigned int retVal = 0;
	struct arp_snooping_item_s comparator;

	memset(&comparator, 0,sizeof(struct arp_snooping_item_s));	
	comparator.l3Index = ifIndex;
	
	retVal = dbtable_hash_traversal(npd_arpsnp_hashport_index, isValid,&comparator,\
					npd_arp_snooping_filter_by_l3index,npd_arp_snooping_static_valid_set);

	if(retVal > 0){
	    syslog_ax_arpsnooping_dbg("npd static arp set %s on eth-port %#x total %d items set\n", \
					    isValid ? "Valid":"Invalid",ifIndex,retVal);
	}
	return retVal;
}



unsigned int npd_arp_snooping_static_valid_set_by_network
(
	unsigned int ip,
	unsigned int mask,
	unsigned int isValid
)
{
	unsigned int ip_mask[2] = {0};
	unsigned int addCount = 0;
	ip_mask[0] = ip;
	ip_mask[1] = mask;

	addCount = dbtable_hash_traversal(npd_arpsnp_haship_index,isValid,(void *)ip_mask,\
										npd_arp_snooping_filter_by_network,npd_arp_snooping_static_valid_set);
	if(addCount > 0){
		syslog_ax_arpsnooping_dbg("arp %s static by ip and mask,delete %d items \n",\
						isValid ? "Valid":"Invalid", addCount);
	}
		
	return addCount;
}



int npd_arp_snooping_reply_send
(	unsigned short vid, 
	unsigned int netif_index, 
	unsigned int dip, unsigned char *dmac,
	unsigned int sip, unsigned char *smac 
)
{
	unsigned char isTagged = 0;
	unsigned char	*data_buff = NULL;
	struct ether_header_t *layer2 = NULL;
	struct arp_header_t *layer3 = NULL;

	npd_vlan_check_contain_port(vid, netif_index, &isTagged);

	data_buff = npd_packet_alloc(NPD_ARP_PKTSIZE);
	if(NULL == data_buff)
	{
		npd_syslog_dbg("malloc dma err when send solicit arp\r\n");
		return COMMON_RETURN_CODE_NO_RESOURCE;
	}
	memset(data_buff,0,NPD_ARP_PKTSIZE);

	/*
	 * Build up ARP solicit packet
	 */
	/* layer 2 */
	layer2 = (struct ether_header_t *)data_buff;
	memcpy(layer2->dmac,dmac,ETH_ALEN);
	memcpy(layer2->smac,smac,ETH_ALEN);
	layer2->etherType = htons(0x0806);

	/* layer 3 */
	layer3 = (struct arp_header_t *)(layer2 + 1);
	layer3->hwType		= htons(0x1); /* ethernet hardware */
	layer3->protType	= htons(0x0800); /* IP */
	layer3->hwSize		= 0x6;
	layer3->protSize	= 0x4;
	layer3->opCode		= htons(ARP_REPLY); /* request */
	memcpy(layer3->smac,smac,ETH_ALEN);
	memcpy(layer3->dmac,dmac,ETH_ALEN);
	/*make sure sip and dip in network endian*/
	memcpy(layer3->sip, &sip, sizeof(sip));
	memcpy(layer3->dip, &dip, sizeof(dip));
	
	nam_packet_tx_unicast_by_netif(2, netif_index, vid,  isTagged, data_buff, NPD_ARP_PKTSIZE);

	npd_packet_free(data_buff);
	return ARP_RETURN_CODE_SUCCESS;
}
int npd_arp_snooping_solicit_send
(
	struct arp_snooping_item_s *item
)
{
	int result = ARP_RETURN_CODE_SUCCESS;
	unsigned char sysMac[MAC_ADDR_LEN];
	unsigned int gateway[MAX_IP_COUNT];
	unsigned int mask[MAX_IP_COUNT];
	int i = 0;
	
	if(NULL == item) {
		return COMMON_RETURN_CODE_NULL_PTR;
	}
	
	memset(gateway, 0, MAX_IP_COUNT*sizeof(unsigned int));
	memset(mask, 0, MAX_IP_COUNT*sizeof(unsigned int));	
	memcpy(sysMac, PRODUCT_MAC_ADDRESS, MAC_ADDR_LEN);
	/*
	 *  Get gateway ip address
	 */
	result = npd_intf_addr_ip_get(item->l3Index,gateway,mask);
	if(NPD_TRUE != result) {
		syslog_ax_arpsnooping_err("get gateway ip error when send arp solicit,ret %#0x \n",result);
		return ARP_RETURN_CODE_ERROR;
	}
#ifdef HAVE_VRRP
	/*Get source MAC address if possible*/
	npd_vrrp_intf_vmac_check(item->l3Index, sysMac);
#endif
	/* Start send packet */
	for(i=0;i<MAX_IP_COUNT;i++){
		unsigned char *ip_address = NULL;
		if((0 != gateway[i])&&((item->ipAddr&mask[i]) == (gateway[i]&mask[i]))){
			ip_address = (unsigned char *)&gateway[i];
		    syslog_ax_arpsnooping_dbg("send arp solicit from %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d\n", \
				    sysMac[0],sysMac[1],sysMac[2],sysMac[3],sysMac[4],sysMac[5],	\
				    ip_address[0],ip_address[1],ip_address[2],ip_address[3]);
		    result = nam_arp_solicit_send(item,sysMac,gateway[i]);
			syslog_ax_arpsnooping_dbg("nam arp solicit send %s ret %#0x \n",\
				(result == ARP_RETURN_CODE_SUCCESS)?"SUCCESS":"FAILED",result);
		}
	}	
	return ARP_RETURN_CODE_SUCCESS;
}

void npd_arpsnp_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
    int type = npd_netif_type_get(netif_index);
	unsigned int l3index = 0;
    int array_port_index = 0;
    struct arp_inspection_status entry;

	syslog_ax_arpsnooping_dbg("npd notify arpsnp index event: index 0x%x event %d\n", netif_index, evt);

    switch(evt)
    {	    
		case PORT_NOTIFIER_DISCARD:
	    case PORT_NOTIFIER_LINKDOWN_E:		
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_arp_snooping_static_valid_set_by_ifindex( netif_index , FALSE);
				npd_arp_snooping_del_by_ifindex( netif_index );	
			}
			break;		
		case PORT_NOTIFIER_DELETE:
		case PORT_NOTIFIER_L2DELETE:
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_arp_snooping_del_static_by_ifindex( netif_index );
				npd_arp_snooping_del_by_ifindex( netif_index );	
                npd_arp_inspection_set_trust( FALSE, netif_index );
                if (PORT_NOTIFIER_L2DELETE == evt)
                {
                    npd_arp_inspection_clean_port_statistic(netif_index);
                }

                if ((type == NPD_NETIF_ETH_TYPE) && (PORT_NOTIFIER_L2DELETE == evt))
                {
                    memset(&entry, 0, sizeof(struct arp_inspection_status));
                    if (0 == npd_arp_inspection_global_status_get(&entry))
                    {
                        array_port_index = netif_array_index_from_ifindex(netif_index);
                        entry.switch_port_control_count[array_port_index] = 0;
                        npd_arp_inspection_global_status_update(&entry);
                    }
                }
			}
            else if (NPD_NETIF_VLAN_TYPE == type)
            {
                memset(&entry, 0, sizeof(struct arp_inspection_status));
                if (0 == npd_arp_inspection_global_status_get(&entry))
                {
                    unsigned short vlan_id = npd_netif_vlan_get_vid(netif_index);
                    
                    NPD_VBMP_VLAN_REMOVE(entry.allow_arp_vlans, vlan_id);
                    npd_arp_inspection_global_status_update(&entry);
                }
            }
			break;

        case PORT_NOTIFIER_L2CREATE:        
            npd_arp_inspection_init_port_statistic(netif_index);
            break;
		case PORT_NOTIFIER_LINKUP_E:
		case PORT_NOTIFIER_FORWARDING:
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_arp_snooping_static_valid_set_by_ifindex( netif_index , TRUE);
			}
	        break;
		case PORT_NOTIFIER_L3DELETE:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_arp_snooping_del_static_by_l3index( l3index );
				npd_arp_snooping_del_by_l3index( l3index );	
			}
			break;
		case PORT_NOTIFIER_L3LINKDOWN:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_arp_snooping_static_valid_set_by_l3index( l3index , FALSE);
				npd_arp_snooping_del_by_l3index( l3index );	
			}
			break;
		case PORT_NOTIFIER_L3LINKUP:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_arp_snooping_static_valid_set_by_l3index(l3index, TRUE);
			}
			break;
	    default:
	        break;
    }

    return;
}

void npd_arpsnp_relate_event(
    unsigned int father_index,
    unsigned int son_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
    unsigned short vlan_id = 0;
    int array_port = 0;
    int array_port_index = 0;
    struct arp_inspection_status entry;
    
    unsigned long son_type = npd_netif_type_get(son_index);
    unsigned long father_type = npd_netif_type_get(father_index);

	syslog_ax_arpsnooping_dbg("npd notify arpsnp relate event: vlan 0x%x index 0x%x event %d\n", \
											father_index, son_index, event);

    memset(&entry, 0, sizeof(struct arp_inspection_status));
    (void)npd_arp_inspection_global_status_get(&entry);

    if (NPD_NETIF_VLAN_TYPE == father_type)
    {
        if ((son_type != NPD_NETIF_ETH_TYPE) && (son_type != NPD_NETIF_TRUNK_TYPE))
    	{
    	    return ;
    	}

        vlan_id = npd_netif_vlan_get_vid(father_index);
        array_port = netif_array_index_from_ifindex(son_index);
        
    	switch(event)
    	{
    		case PORT_NOTIFIER_LEAVE:
            {
                if (NPD_VBMP_MEMBER(entry.allow_arp_vlans, vlan_id))
                {
                    if (entry.switch_port_control_count[array_port] > 0)
                    {
                        entry.switch_port_control_count[array_port]--;
                    }

                    if (NPD_NETIF_TRUNK_TYPE == son_type)
                    {
                        npd_arp_inspection_modify_port_by_trunk(&entry, son_index);
                    }
                }

    			npd_arp_snooping_del_static_by_ifindex_vid(son_index,vlan_id);
    			npd_arp_snooping_del_by_ifindex_vid(son_index,vlan_id);
                
    			break;
    	    }
            case PORT_NOTIFIER_JOIN:
            {
                if (NPD_VBMP_MEMBER(entry.allow_arp_vlans, vlan_id))
                {
                    entry.switch_port_control_count[array_port]++;

                    if (NPD_NETIF_TRUNK_TYPE == son_type)
                    {
                        npd_arp_inspection_modify_port_by_trunk(&entry, son_index);
                    }
                }
                break;
            }
    		default:
            {
    			break;
    		}
    	}
    }
    else if (NPD_NETIF_TRUNK_TYPE == father_type)
    {
        if (son_type != NPD_NETIF_ETH_TYPE)
    	{
    	    return;
    	}

        array_port = netif_array_index_from_ifindex(father_index);

        if (entry.switch_port_control_count[array_port] > 0)
        {
            switch(event)
        	{
        		case PORT_NOTIFIER_LEAVE:
                {
                    array_port_index = netif_array_index_from_ifindex(son_index);
                    entry.switch_port_control_count[array_port_index] = 0;
        			break;
        	    }
                case PORT_NOTIFIER_JOIN:
                {
                    array_port_index = netif_array_index_from_ifindex(son_index);
                    entry.switch_port_control_count[array_port_index] = entry.switch_port_control_count[array_port];
                    break;
                }
        		default:
                {
        			break;
        		}
        	}
        }
    }

    (void)npd_arp_inspection_global_status_update(&entry);

	return;
}

int npd_arp_snooping_age(hash_table_index_t *hash, void * data, unsigned int flag)
{
	struct arp_snooping_item_s *arpItem;
	struct npd_arpsnp_cfg_s cfgItem;

    dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);

	if( data == NULL )
		return ARP_RETURN_CODE_ERROR;

	arpItem = (struct arp_snooping_item_s *)data;

	if(TRUE == arpItem->isStatic)
		return ARP_RETURN_CODE_SUCCESS;

	if( (flag & ARPSNP_FLAG_DROP) && (arpItem->flag & ARPSNP_FLAG_DROP))
	{
		if(!(arpItem->flag & ARPSNP_FLAG_HIT))
		{
			npd_arp_snooping_solicit_send(arpItem);
			npd_arp_snooping_del(hash, arpItem, TRUE);
		}
		else if(arpItem->flag & ARPSNP_FLAG_HIT)
		{
			arpItem->flag &= ~ARPSNP_FLAG_HIT;
			dbtable_hash_update(hash,arpItem,arpItem);
			npd_arp_snooping_solicit_send(arpItem);
		}		
		//npd_arp_snooping_del(hash, arpItem, TRUE);
		return ARP_RETURN_CODE_SUCCESS;
	}

	if( flag & ARPSNP_FLAG_HIT )
	{
		if(!(arpItem->flag & ARPSNP_FLAG_HIT))
		{
			npd_arp_snooping_solicit_send(arpItem);
            if(arp_absolute_time - arpItem->time > cfgItem.timeout)
			    npd_arp_snooping_del(hash, arpItem, TRUE);
			return ARP_RETURN_CODE_SUCCESS;
		}
		else if(arpItem->flag & ARPSNP_FLAG_HIT)
		{
			arpItem->flag &= ~ARPSNP_FLAG_HIT;
			dbtable_hash_update(hash,arpItem,arpItem);
			npd_arp_snooping_solicit_send(arpItem);
		}
	}
	return ARP_RETURN_CODE_SUCCESS;;		
}


void npd_arp_snooping_drop_handle()
{
	dbtable_hash_traversal(npd_arpsnp_haship_index, ARPSNP_FLAG_DROP, NULL, NULL, npd_arp_snooping_age);
}

void npd_arp_snooping_age_handle(struct arp_snooping_item_s *start)
{
    unsigned int count = 0;
    int ret;
	unsigned char zero_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	struct arp_snooping_item_s age_item;
	struct npd_arpsnp_cfg_s cfgItem;

    dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);
	if(cfgItem.timeout != 0)
    {
    	if (0 == memcmp(start->mac, zero_mac, MAC_ADDR_LEN))
    	{
        	ret = dbtable_hash_head(npd_arpsnp_haship_index, start, start, NULL);
    	}
    	else
    	{
        	ret = dbtable_hash_head_key(npd_arpsnp_haship_index, start, start, NULL);
    	}
        while(0 == ret)
        {
    		/* if static item, skip it */
    		if (start->isStatic)
    		{
            	ret = dbtable_hash_next(npd_arpsnp_haship_index, start, start, NULL);			
    			continue;
    		}
    		/*it set for acl policy route, skip it*/
    		{
    			struct service_policy_route_s item;
    			int hash_ret = 0;
    			memset(&item, 0, sizeof(item));
    			item.nexthopv4 = start->ipAddr;
    			hash_ret = dbtable_hash_head_key(ser_policy_pb_hash, &item, &item, NULL);
    			if(0 == hash_ret)
    			{
            	    ret = dbtable_hash_next(npd_arpsnp_haship_index, start, start, NULL);	
    				continue;
    			}
    		}
    		
    		/*by zhanwei
    		需要先获取下一个再老化，
    		否则存在表项老化删除，
    		导致获取下一个并不是真实的下一个*/
    		memcpy(&age_item, start, sizeof(struct arp_snooping_item_s));
            ret = dbtable_hash_next(npd_arpsnp_haship_index, start, start, NULL); 
            if((arp_absolute_time - age_item.time) > (cfgItem.timeout-300))
            {
                npd_arp_snooping_age(npd_arpsnp_haship_index, &age_item, ARPSNP_FLAG_ALL);
                count++;
                if(count > NPD_ARP_ITEM_ONCE_HANDLE)
        			break;
            }
        }
        if(0 != ret)
        {
            memset(start, 0, sizeof(*start));
            arp_aging_continue = 0;
        }
        else
            arp_aging_continue = 1;
    }
#ifdef HAVE_ROUTE    
    /*for policy route*/
	{
		struct service_policy_route_s item;
		memset(&item, 0, sizeof(struct service_policy_route_s));
		ret = dbtable_hash_head(ser_policy_pb_index, &item, &item, NULL);
		while(0 == ret)
		{
			int status;
            struct arp_snooping_item_s arp_item;
			memset(&arp_item, 0, sizeof(struct arp_snooping_item_s));
			arp_item.ipAddr = item.nexthopv4;
        	status = dbtable_hash_search(npd_arpsnp_haship_index,&arp_item,
        		npd_arp_snooping_compare_byip,&arp_item);
			if(0 != status)
			{
        		unsigned int ip = 0;
                unsigned int l3index[MAX_L3INTF_NUMBER];
            	unsigned int net_g_index = 0;
            	unsigned int intfCount = MAX_L3INTF_NUMBER;
        
        	    ip = item.nexthopv4;
            	ret = npd_intf_addr_ifindex_get_bynet(l3index, &intfCount, ip);
            	if( ret == NPD_FALSE )
            	{
            		return;
            	}
         		ret = npd_intf_netif_get_by_ifindex(l3index[0], &net_g_index);
        
        		arp_item.ipAddr = ip;
        		if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(net_g_index))
        		    arp_item.vid = npd_netif_vlan_get_vid(net_g_index);
        		else
        		{
        			arp_item.vid = NPD_PORT_L3INTF_VLAN_ID;
        			arp_item.ifIndex = net_g_index;
        		}
        		npd_arp_snooping_solicit_send(&arp_item);
			}

			ret = dbtable_hash_next(ser_policy_pb_index, &item, &item, NULL);
		}
	}
#endif
}


int npd_arp_snooping_sync(void)
{
	int dropCnt = 0;
	struct timeval tv;
	int curtime = 0;
    struct arp_snooping_item_s arpItem = {0};
    
	npd_init_tell_whoami("npdArpSync",0);
	
	while(1)
	{
		tv.tv_sec = NPD_ARP_AGE_INTERVAL;
		tv.tv_usec = 0;		

		select(0, NULL, NULL, NULL, &tv);

        arp_absolute_time += NPD_ARP_AGE_INTERVAL;
		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			continue;
		if( dropCnt++ > NPD_ARP_AGE_DROP_CNT)
		{
			dropCnt = 0;
			npd_arp_snooping_drop_handle();
		}
		
        if(arp_aging_continue)
        {
			npd_arp_snooping_age_handle(&arpItem);
        }
		curtime++;
		if((curtime * NPD_ARP_AGE_INTERVAL) >=  30)
		{
			curtime = 0;
			npd_arp_snooping_age_handle(&arpItem);
		}
	}
	
	return ARP_RETURN_CODE_SUCCESS;
}

long npd_arpsnp_dbtbl_app_handle_update( void *newItem, void *oldItem )
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s *origItem = NULL, *updateItem = NULL;

	syslog_ax_arpsnooping_dbg("npd arp app handle update\n");
	
	if( ( newItem == NULL ) || ( oldItem == NULL ) )
		return ARP_RETURN_CODE_ERROR;

	origItem = (struct arp_snooping_item_s *)oldItem;
	updateItem = (struct arp_snooping_item_s *)newItem;
	
	if(sysKernArpSock <= 0) {
		syslog_ax_arpsnooping_err("npd del kern arp sysKernArpSock error %d\r\n",sysKernArpSock);
		return ARP_RETURN_CODE_ERROR;
	}

	syslog_ax_arpsnooping_dbg("App sync arp: port 0x%x ip 0x%x, mac %02x:%02x:%02x:%02x:%02x:%02x, vid %d, static %d\n", 
										updateItem->ifIndex, updateItem->ipAddr,
										updateItem->mac[0],	updateItem->mac[1], updateItem->mac[2],updateItem->mac[3],
										updateItem->mac[4],updateItem->mac[5], updateItem->vid, updateItem->isStatic);
    if(!npd_arp_snooping_compare(origItem, updateItem))
	{
		ret = npd_arp_snooping_del_kern_arp(origItem);
		if( ret != ARP_RETURN_CODE_SUCCESS )
		{
			syslog_ax_arpsnooping_err("App sync arp: del orig arp port 0x%x ip 0x%x mac %02x:%02x:%02x:%02x:%02x:%02x fail, ret %d\r\n",
										updateItem->ifIndex, updateItem->ipAddr, updateItem->mac[0],
										updateItem->mac[1],updateItem->mac[2],updateItem->mac[3],
										updateItem->mac[4],updateItem->mac[5], ret);
		}
	}
    /*此处不再有必要*/
    /*
    if(updateItem->isStatic)
	    npd_arp_snooping_create_kern_arp(updateItem);
	*/
	return ARP_RETURN_CODE_SUCCESS;
}

long npd_arpsnp_dbtbl_handle_update(  void *newItem, void *oldItem )
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s *origItem = NULL, *updateItem = NULL;
	struct route_nexthop_brief_s nextHopItem;
	unsigned int tblIndex = 0;

	syslog_ax_arpsnooping_dbg("npd arp handle update\n");
	
	if( ( newItem == NULL ) || ( oldItem == NULL ) )
		return ARP_RETURN_CODE_ERROR;

	origItem = (struct arp_snooping_item_s *)oldItem;
	updateItem = (struct arp_snooping_item_s *)newItem;

	if( origItem->isValid != updateItem->isValid )
	{
		if( updateItem->isValid == TRUE )
		{
			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_arp_snooping_op_item(updateItem,ARP_SNOOPING_ADD_ITEM,&tblIndex);	
			pthread_mutex_unlock(&namItemOpMutex);
			if(ARP_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_arpsnooping_err("nam add item error %d\r\n",ret); 	
				return ret;
			}			

			syslog_ax_arpsnooping_dbg("npd set nexthop hash table\r\n");
			memset(&nextHopItem, 0, sizeof(struct route_nexthop_brief_s));
			npd_intf_port_check(updateItem->vid, updateItem->ifIndex, &(nextHopItem.l3Index));
			nextHopItem.ipAddr	= updateItem->ipAddr;
			nextHopItem.tblIndex= tblIndex;
			pthread_mutex_lock(&nexthopHashMutex);
			ret = npd_route_nexthop_op_item(&nextHopItem,NEXTHOP_ADD_ITEM,nextHopItem.l3Index); 
			pthread_mutex_unlock(&nexthopHashMutex);
			if(ROUTE_RETURN_CODE_SUCCESS != ret) {
				syslog_ax_arpsnooping_err("npd nexthop brief add to db error %d\r\n",ret);
				return ARP_RETURN_CODE_ERROR;
			}
#ifdef HAVE_ROUTE
			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_set_host_route(updateItem->ipAddr, updateItem->ifIndex);
			pthread_mutex_unlock(&namItemOpMutex);
			if(ARP_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_arpsnooping_err("nam add host route error %d\r\n",ret);		
				return ret;
			}						
#endif
		}
		else 
		{
#ifdef HAVE_ROUTE
			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_del_host_route(updateItem->ipAddr, updateItem->ifIndex);
			pthread_mutex_unlock(&namItemOpMutex);
			if(ARP_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_arpsnooping_err("nam delete host route error %d\r\n",ret);		
				return ret;
			}
#endif
			pthread_mutex_lock(&nexthopHashMutex);			
			ret = npd_route_nexthop_find(updateItem->ipAddr, &nextHopItem);
			if(NPD_FALSE == ret) {
				syslog_ax_arpsnooping_err("npd arp set invalid ifindex %#x ip %d.%d.%d.%d not found in sw hash table\r\n", \
						updateItem->l3Index,(updateItem->ipAddr>>24) & 0xFF,(updateItem->ipAddr>>16) & 0xFF,	\
						(updateItem->ipAddr>>8) & 0xFF,updateItem->ipAddr & 0xFF);
				pthread_mutex_unlock(&nexthopHashMutex);
				return ARP_RETURN_CODE_SUCCESS;
			}

			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_arp_snooping_op_item(updateItem,ARP_SNOOPING_DEL_ITEM,&(nextHopItem.tblIndex));
			pthread_mutex_unlock(&namItemOpMutex);
			if(ARP_RETURN_CODE_SUCCESS != ret) {
				syslog_ax_arpsnooping_err("nam del arp snooping item at %d error %d\r\n", nextHopItem.tblIndex,ret);
			}
			/* delete route_nexthop item only if not used by any route entry*/
			ret = npd_route_nexthop_op_item(&nextHopItem,NEXTHOP_DEL_ITEM,0); 
			
			pthread_mutex_unlock(&nexthopHashMutex);			
		}		
	}

	if (origItem->isStatic != updateItem->isStatic)
	{
		pthread_mutex_lock(&namItemOpMutex);
		ret = nam_arp_snooping_op_item(updateItem,ARP_SNOOPING_UPDATE_ITEM,&tblIndex);
		pthread_mutex_unlock(&namItemOpMutex);
		
	}

	return ARP_RETURN_CODE_SUCCESS;  
}

long npd_arpsnp_dbtbl_handle_insert( void *newItem )
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s *opItem = (struct arp_snooping_item_s *)newItem;
	struct route_nexthop_brief_s nextHopItem;
	unsigned int tblIndex = 0;

	syslog_ax_arpsnooping_dbg("npd arp handle insert\n");
	
	if( opItem == NULL || opItem->isValid == FALSE)
		return ARP_RETURN_CODE_SUCCESS;
	
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_arp_snooping_op_item(opItem,ARP_SNOOPING_ADD_ITEM,&tblIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(ARP_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_arpsnooping_err("nam add item error %d\r\n",ret);		
		return ret;
	}
	memset(&nextHopItem, 0, sizeof(struct route_nexthop_brief_s));
	npd_intf_port_check(opItem->vid, opItem->ifIndex, &(nextHopItem.l3Index));
	nextHopItem.ipAddr	= opItem->ipAddr;
	nextHopItem.tblIndex= tblIndex;

	pthread_mutex_lock(&nexthopHashMutex);
	ret = npd_route_nexthop_op_item(&nextHopItem,NEXTHOP_ADD_ITEM,nextHopItem.l3Index); 
	pthread_mutex_unlock(&nexthopHashMutex);
	if(ROUTE_RETURN_CODE_SUCCESS != ret) {
		syslog_ax_arpsnooping_err("npd nexthop brief add to db error %d\r\n",ret);
		return ret;
	}
#ifdef HAVE_ROUTE
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_set_host_route(opItem->ipAddr, opItem->ifIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(ARP_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_arpsnooping_err("nam add host route error %d\r\n",ret);		
		return ret;
	}
#endif		
	syslog_ax_arpsnooping_dbg("npd add item at hw nexthop table %d ok\r\n",tblIndex);
	return ARP_RETURN_CODE_SUCCESS;	
}

long npd_arpsnp_dbtbl_handle_delete( void *delItem )
{
	int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s *opItem = (struct arp_snooping_item_s *)delItem;
	struct route_nexthop_brief_s routeNextHop;
    unsigned char *ip_address = NULL;
	syslog_ax_arpsnooping_dbg("npd arp handle delete\n");

	if( opItem == NULL || opItem->isValid == FALSE)
		return ARP_RETURN_CODE_SUCCESS;

#ifdef HAVE_ROUTE
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_del_host_route(opItem->ipAddr, opItem->ifIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(ARP_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_arpsnooping_err("nam delete host route error %d\r\n",ret);		
		return ret;
	}
#endif
	pthread_mutex_lock(&nexthopHashMutex);
	ret = npd_route_nexthop_find(opItem->ipAddr, &routeNextHop);
	pthread_mutex_unlock(&nexthopHashMutex);
	ip_address = (unsigned char *)&opItem->ipAddr;
	if(NPD_FALSE == ret) {
		syslog_ax_arpsnooping_err("npd delete ifindex %#x ip %d.%d.%d.%d not found in sw hash table\r\n", \
				opItem->l3Index,ip_address[0],ip_address[1],	\
				ip_address[2],ip_address[3]);		
		return ARP_RETURN_CODE_SUCCESS;
	}
	
	syslog_ax_arpsnooping_dbg("npd delete ifindex %#x ip %d.%d.%d.%d found at %d ref %d of HW table\r\n", \
			opItem->l3Index,ip_address[0],ip_address[1],	\
			ip_address[2],ip_address[3],routeNextHop.tblIndex, routeNextHop.rtUsedCnt);

	
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_arp_snooping_op_item(opItem,ARP_SNOOPING_DEL_ITEM,&(routeNextHop.tblIndex));
	pthread_mutex_unlock(&namItemOpMutex);
	if(ARP_RETURN_CODE_SUCCESS != ret) {
		syslog_ax_arpsnooping_err("nam del arp snooping item at %d error %d\r\n",routeNextHop.tblIndex,ret);
		ret = ARP_RETURN_CODE_NAM_ERROR;
	}
	else {
		syslog_ax_arpsnooping_dbg("nam del arp snooping item at %d of HW table ok\r\n",	routeNextHop.tblIndex);
	}


	/* delete route_nexthop item only if not used by any route entry*/
	ret = npd_route_nexthop_op_item(&routeNextHop,NEXTHOP_DEL_ITEM,0); 
	
	syslog_ax_arpsnooping_dbg("npd del item at arp table ok\r\n");
	return ret;
}

long npd_arpsnp_dbtbl_app_handle_delete(  void *delItem )
{

	struct arp_snooping_item_s *delArpEntry = NULL;

	if(NULL == delItem) {
		syslog_ax_arpsnooping_err("npd del kern arp item is NULL\r\n");
		return ARP_RETURN_CODE_ERROR;
	}

	delArpEntry = (struct arp_snooping_item_s *)delItem;
	
	if(sysKernArpSock <= 0) {
		syslog_ax_arpsnooping_err("npd del kern arp sysKernArpSock error %d\r\n",sysKernArpSock);
		return sysKernArpSock;
	}

	npd_arp_snooping_del_kern_arp(delArpEntry);	
	
	return ARP_RETURN_CODE_SUCCESS;
}


long npd_arpsnp_cfgtbl_handle_update( void *data1, void *data2)
{
    struct npd_arpsnp_cfg_s *new_cfg = (struct npd_arpsnp_cfg_s *)data1;
    arp_drop_enable = new_cfg->arp_drop_enable;
	return ARP_RETURN_CODE_SUCCESS;
}

long npd_arpsnp_cfgtbl_handle_insert( void *data1)
{
    struct npd_arpsnp_cfg_s *new_cfg = (struct npd_arpsnp_cfg_s *)data1;
    arp_drop_enable = new_cfg->arp_drop_enable;
	return ARP_RETURN_CODE_SUCCESS;
}

int npd_arpsnp_dbtbl_handle_ntoh(void *data)
{
	struct arp_snooping_item_s *arpItem = (struct arp_snooping_item_s *)data;

	arpItem->ifIndex = ntohl(arpItem->ifIndex);
	arpItem->l3Index = ntohl(arpItem->l3Index);
	arpItem->ipAddr = ntohl(arpItem->ipAddr);
	arpItem->vid = ntohs(arpItem->vid);
	arpItem->vidx = ntohs(arpItem->vidx);
	arpItem->flag = ntohl(arpItem->flag);
    arpItem->time = ntohl(arpItem->time);

	return 0;
}

int npd_arpsnp_dbtbl_handle_hton(void *data)
{
	struct arp_snooping_item_s *arpItem = (struct arp_snooping_item_s *)data;

	arpItem->ifIndex = htonl(arpItem->ifIndex);
	arpItem->l3Index = htonl(arpItem->l3Index);
	arpItem->ipAddr = htonl(arpItem->ipAddr);
	arpItem->vid = htons(arpItem->vid);
	arpItem->vidx = htons(arpItem->vidx);
	arpItem->flag = htonl(arpItem->flag);
    arpItem->time = htonl(arpItem->time);

	return 0;
}

int npd_arpsnp_cfgtbl_handle_ntoh(void *data)
{
	struct npd_arpsnp_cfg_s *arpsnpCfg = (struct npd_arpsnp_cfg_s *)data;

	arpsnpCfg->timeout = ntohl(arpsnpCfg->timeout); 

	return 0;
}

int npd_arpsnp_cfgtbl_handle_hton(void *data)
{
	struct npd_arpsnp_cfg_s *arpsnpCfg = (struct npd_arpsnp_cfg_s *)data;

	arpsnpCfg->timeout = htonl(arpsnpCfg->timeout); 

	return 0;
}

int npd_arp_snooping_table_init()
{
	int ret;
	struct npd_arpsnp_cfg_s cfgItem;
	
	ret = create_partsync_dbtable( NPD_ARPSNP_HASHTBL_NAME, 
                    NPD_ARPSNP_TABLE_SIZE, 
                    sizeof(struct arp_snooping_item_s),
                    sizeof(struct arp_snooping_item_s)-sizeof(unsigned int),
					npd_arpsnp_dbtbl_handle_update, 
					npd_arpsnp_dbtbl_app_handle_update,
					npd_arpsnp_dbtbl_handle_insert, 
					npd_arpsnp_dbtbl_handle_delete,
					npd_arpsnp_dbtbl_app_handle_delete,
					NULL, 
					NULL, 
					npd_arpsnp_dbtbl_handle_ntoh,
					npd_arpsnp_dbtbl_handle_hton,
					DB_SYNC_ALL,
					&(npd_arpsnp_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_arpsnooping_err("create npd arp database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("iparp", npd_arpsnp_dbtbl,NPD_ARPSNP_HASH_IP_SIZE, npd_arp_snooping_key_generate,\
													npd_arp_snooping_compare, &npd_arpsnp_haship_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd arp ip hash table fail\n");
		return NPD_FAIL;
	}	

	ret = dbtable_create_hash_index("ifindexarp", npd_arpsnp_dbtbl, NPD_ARPSNP_HASH_PORT_SIZE, npd_arp_snooping_key_port_generate,\
														npd_arp_snooping_compare, &npd_arpsnp_hashport_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd arp port hash table fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("macarp", npd_arpsnp_dbtbl,NPD_ARPSNP_HASH_MAC_SIZE, npd_arp_snooping_key_mac_generate,\
													npd_arp_snooping_compare, &npd_arpsnp_hashmac_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd arp ip hash table fail\n");
		return NPD_FAIL;
	}	

	ret = create_dbtable( NPD_ARPSNP_HASHTBL_NAME, NPD_ARPSNP_TABLE_SIZE, sizeof(struct route_nexthop_brief_s),\
						NULL, NULL,	NULL, NULL,	NULL, NULL, NULL, NULL, NULL, DB_SYNC_NONE,&(npd_nexthop_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_arpsnooping_err("create npd nexthop database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ipnexthop", npd_nexthop_dbtbl,NPD_ARPSNP_HASH_IP_SIZE, npd_route_nexthop_key_generate,\
													npd_route_nexthop_compare, &npd_nexthop_hash_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd nexthop hash table fail\n");
		return NPD_FAIL;
	}	

#if 1
	ret = create_dbtable( NPD_ARPSNP_CFGTBL_NAME, 1, sizeof(struct npd_arpsnp_cfg_s),\
					npd_arpsnp_cfgtbl_handle_update, 
					NULL,
					npd_arpsnp_cfgtbl_handle_insert, 
					NULL,
					NULL, 
					NULL,
					NULL,
					npd_arpsnp_cfgtbl_handle_ntoh,
					npd_arpsnp_cfgtbl_handle_hton,
					DB_SYNC_ALL,
					&(npd_arpsnp_cfgtbl));
	if( 0 != ret )
	{
		syslog_ax_arpsnooping_err("create npd config database fail\n");
		return NPD_FAIL;
	}
	ret = dbtable_create_array_index("cfgarp",npd_arpsnp_cfgtbl,&npd_arpsnp_cfg_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd config array index fail\n");
		return NPD_FAIL;
	}
		
	cfgItem.timeout = NPD_ARP_AGE_AGE_CNT;
    cfgItem.arp_drop_enable = 0;
	ret = dbtable_array_insert_byid(npd_arpsnp_cfg_index, 0, &cfgItem);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("insert npd config array item fail\n");
		return NPD_FAIL;
	}
	dbtable_table_show_func_install(npd_arpsnp_dbtbl, (dbtable_show_func)npd_arp_snooping_show_table);
#endif

	return NPD_OK;
}

void npd_init_arpsnooping
(
	void
)
{
	syslog_ax_arpsnooping_dbg("init arp snooping database\r\n");
	
	npd_arp_snooping_table_init();
	
	/* TODO:init arp Next-Hop table index and arp Mac table index*/
	syslog_ax_arpsnooping_dbg("init nam arp table\r\n");
	nam_arp_table_index_init();

	npd_arp_inspection_status_initialize();
	
	/*create socket deleted kernel arp */
	sysKernArpSock = socket(AF_INET,SOCK_DGRAM,0);
	
	//nam_thread_create("npdArpSync",(void *)npd_arp_snooping_sync2kern,NULL,TRUE,FALSE);
	nam_thread_create("npdArpSync",(void *)npd_arp_snooping_sync,NULL,TRUE,FALSE);
	
	register_netif_notifier(&arpsnp_netif_notifier);
	
	syslog_ax_arpsnooping_dbg("finish init arp snooping database\r\n");
	
	return;	
}

/************************************************************************************
 *		NPD ARP rx/aging operation
 *
 ************************************************************************************/


int npd_arp_snooping_learning
(
	struct arp_snooping_item_s *item
)
{
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	struct arp_snooping_item_s dupItem;	
	int status;
	int replace = FALSE;
	unsigned int arpCount = 0;
	unsigned int l3intf_bind = 0;
	NPD_L3INTERFACE_CTRL l3intf_ctrl;
    int result;
    
	l3intf_ctrl.ifindex = item->l3Index;
	result= dbtable_hash_search(l3intf_ifindex, &l3intf_ctrl, npd_l3_intf_cmp_ifindex, &l3intf_ctrl);
	if(result == 0)
	{
		l3intf_bind = l3intf_ctrl.bind;
		if(INTF_CTRL_STATE_UP != l3intf_ctrl.ctrl_state )
			item->isValid = FALSE;
	}
	else
	{
		item->isValid = FALSE;
	}

	memset(&dupItem,0,sizeof(struct arp_snooping_item_s));

	dbtable_hash_lock(npd_arpsnp_haship_index);
	
	status = npd_arp_snooping_find_item_byip( item->ipAddr, &dupItem);
	if( 0 == status && dupItem.isStatic != TRUE )
	{
        if(!npd_arp_snooping_compare(item, &dupItem))
        {
#ifdef HAVE_ROUTE
            if(l3intf_bind)
            {
                status = -1;
                replace = FALSE;
            }
            else
            {
				npd_route_update_by_nhp(item->ipAddr, FALSE);
				
#endif
#ifdef HAVE_M4_TUNNEL
                npd_tunnel_update_by_arp(item->ipAddr, FALSE);
#endif
	        	status = dbtable_hash_delete( npd_arpsnp_haship_index, &dupItem, &dupItem);
	            replace = TRUE;
#ifdef HAVE_ROUTE
			}
#endif
        }
        else if(dupItem.flag & ARPSNP_FLAG_DROP)
        {
#ifdef HAVE_ROUTE
            if(l3intf_bind)
            {
                status = -1;
                replace = FALSE;
            }
            else
            {
	        	npd_route_update_by_nhp(item->ipAddr, FALSE);
#endif
#ifdef HAVE_M4_TUNNEL
                npd_tunnel_update_by_arp(item->ipAddr, FALSE);
#endif
        		status = dbtable_hash_delete( npd_arpsnp_haship_index, &dupItem, &dupItem);
            	replace = TRUE;
#ifdef HAVE_ROUTE
			}
#endif
        }
		else if(!(dupItem.flag & ARPSNP_FLAG_HIT))
		{
			dupItem.flag |= ARPSNP_FLAG_HIT;
			dupItem.time = item->time;
			syslog_ax_arpsnooping_dbg("Arp learning: update arp %#x by hit\n", dupItem.ipAddr);
			status = dbtable_hash_update(npd_arpsnp_haship_index, &dupItem, &dupItem);
		}
	}
    else if(0 == status && (NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(dupItem.ifIndex)))
    {
		if(!npd_arp_snooping_compare(item, &dupItem))
		{
	        replace = TRUE;
#ifdef HAVE_ROUTE        
    	   	npd_route_update_by_nhp(item->ipAddr, FALSE);
#endif
#ifdef HAVE_M4_TUNNEL
            npd_tunnel_update_by_arp(item->ipAddr, FALSE);
#endif
   			status = dbtable_hash_delete( npd_arpsnp_haship_index, &dupItem, &dupItem);
		}
#ifdef HAVE_AAA	
        if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(item->ifIndex))
        {
		    npd_asd_send_arp_info_to_wirelss(item->ifIndex, item->vid, 1,\
		                    item->mac, item->ipAddr);
        }
#endif
		
    }
	else if( 0 == status && dupItem.isStatic == TRUE )
	{
	    if(!npd_arp_snooping_compare(item, &dupItem))
	    {
        	dbtable_hash_unlock(npd_arpsnp_haship_index);
        	
        	return ARP_RETURN_CODE_ERROR;
	    }
		else
		{
        	dbtable_hash_unlock(npd_arpsnp_haship_index);
        	
        	return ARP_RETURN_CODE_SUCCESS;
		}
	}
	
    if((TRUE == replace) || (0 != status))	
    {
		arpCount = npd_arp_snooping_count_all(); /*To avoid ARP entries are occupied by Drop entries*/
		if(NPD_ARPSNP_TABLE_SIZE <= arpCount){
			npd_arp_snooping_drop_handle();		
		}
		arpCount = npd_arp_snooping_count_all();
		if(NPD_ARPSNP_TABLE_SIZE > arpCount)
		{
             if(!l3intf_bind)
             {
	    	 	syslog_ax_arpsnooping_dbg("npd_arp_snooping_learning: vid %d, vidx %d, static %d, valid %d\r\n", \
	    	                       item->vid, item->vidx, item->isStatic, item->isValid);
    	    	 status = dbtable_hash_insert( npd_arpsnp_haship_index, item);
#ifdef HAVE_ROUTE
    	    	 npd_route_update_by_nhp(item->ipAddr, TRUE);
    	    	 npd_policy_route_update_by_nhp(item->ipAddr, TRUE);
#endif             
#ifdef HAVE_M4_TUNNEL
                 npd_tunnel_update_by_arp(item->ipAddr, TRUE);
#endif              
#ifdef HAVE_AAA			
				/*notify ASD to update wireless station IP infor*/
                if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(item->ifIndex))
                {
                    npd_asd_send_arp_info_to_wirelss(item->ifIndex, item->vid, 1,\
                                    item->mac, item->ipAddr);
                }
#endif


			 }
             else
             {
                status = -1;
             }
		}
    }
	
	if( 0 != status )
		ret = ARP_RETURN_CODE_ERROR;

	dbtable_hash_unlock(npd_arpsnp_haship_index);
	
	return ret;	
}


int npd_arp_snoop_drop_check
(
	unsigned int   srcType,
	unsigned int   netif_index,
	unsigned short vid,
	unsigned char  *packet,
	unsigned long  length
)
{
	struct ether_header_t *layer2 = NULL;
	struct ip_header_t	  *ipHdr = NULL;
	struct arp_snooping_item_s dbItem;
	unsigned int dip = 0;
	unsigned int l3Index = 0;
	int ret = 0;
		
	layer2 = (struct ether_header_t *)packet;	
	ipHdr = (struct ip_header_t *)((char*)packet + sizeof(struct ether_header_t));
	dip = *(unsigned int *)(ipHdr->dip);
	
	ret = npd_arp_snooping_find_item_byip(dip,&dbItem);
	if(0 == ret )
	{
		syslog_ax_arpsnooping_err("Found arp entry for this ip %#x\n", dip);
		goto check_end;
	}

	if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_valid_check(dip))
	{
		syslog_ax_arpsnooping_err("bad dest ip %d.%d.%d.%d \n", \
			ipHdr->dip[0],ipHdr->dip[1],ipHdr->dip[2],ipHdr->dip[3]);
		goto check_end;
	}
	
	if(NPD_TRUE == npd_intf_netif_get_by_ip(&l3Index, dip))
	{
		syslog_ax_arpsnooping_err("dip %d.%d.%d.%d is for l3 interface %#x\n", \
			ipHdr->dip[0],ipHdr->dip[1],ipHdr->dip[2],ipHdr->dip[3], l3Index);
		goto check_end;
	}
	
	syslog_ax_arpsnooping_dbg("Receive dest %d.%d.%d.%d unknown ipv4 packet on interface %#x\n", 
							ipHdr->dip[0],ipHdr->dip[1],ipHdr->dip[2],ipHdr->dip[3]);

	dbItem.ipAddr = dip;
	dbItem.ifIndex  = netif_index;
	dbItem.flag |= ARPSNP_FLAG_DROP;
	dbItem.isValid = TRUE;
	npd_arp_snooping_learning(&dbItem);

check_end:
	return ARP_RETURN_CODE_SUCCESS;	
}

int npd_arp_inspection_check(unsigned int netif_index, unsigned short vid, struct ether_header_t* layer2, struct arp_packet_t* arpPacket)
{
    unsigned char endis = 0;
    unsigned char vlan_flag;
    unsigned char isFound = 0;
    unsigned char flag = 0;
    int ni = 0;
    int nlport = 0;
    unsigned int trust_mode = 0;
    struct arp_inspection_status user;

	memset(&user, 0, sizeof(struct arp_inspection_status));

	if (npd_arp_inspection_global_status_get(&user))
	{
		syslog_ax_arpsnooping_dbg("Get global ARP inspection information faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}

    nlport = netif_array_index_from_ifindex(netif_index);

    if ((nlport > MAX_SWITCHPORT_PER_SYSTEM)
        || (nlport < 0))
    {
        return ARP_RETURN_CODE_ERROR;
    }
    
	if ((ARP_RETURN_CODE_SUCCESS == npd_arp_inspection_check_vlan_endis(vid, &vlan_flag))
        && (vlan_flag))
	{
#ifdef HAVE_DHCP_SNP
        if ((DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_get_global_status(&endis))
            && (0 != endis))
        {
			if (DHCP_SNP_RETURN_CODE_EN_VLAN == npd_dhcp_snp_check_vlan_status(vid))
			{
				if ((ARP_RETURN_CODE_SUCCESS == npd_arp_inspection_check_trust(netif_index, &trust_mode, &user))
                    && (!trust_mode))
				{
					if (ARP_RETURN_CODE_SUCCESS == npd_dhcp_snp_query_arp_inspection(netif_index, arpPacket, &isFound))
                    {
						if (!isFound)
						{
                            npd_arp_inspection_statistics[nlport].drop++;
                            return ARP_RETURN_CODE_ERROR;
						}						
					}
                    else
                    {
        			    npd_arp_inspection_statistics[nlport].drop++;
						return ARP_RETURN_CODE_ERROR;
					}		
				}
			}
            else
            {
                return ARP_RETURN_CODE_ERROR;
            }
		}
        else
        {
            return ARP_RETURN_CODE_ERROR;
        }
#endif
	}

	for (ni = 1;  ni < 5; ni ++)
	{
		npd_arp_inspection_validate_type_check(&user, ni, &flag);
		if (flag)
		{
			if (!npd_query_arp_inspection(layer2, arpPacket, &user, ni, &isFound))
			{
				if (!isFound)
				{
                    npd_arp_inspection_statistics[nlport].drop++;
					return ARP_RETURN_CODE_ERROR;
				}
			}
			else
			{
                npd_arp_inspection_statistics[nlport].drop++;
				return ARP_RETURN_CODE_ERROR;
			}							
		}
	}

    npd_arp_inspection_statistics[nlport].permit++;

    return ARP_RETURN_CODE_SUCCESS;
}


int npd_arp_packet_rx_process
(
	unsigned int   srcType,
	unsigned int   netif_index,
	unsigned int   ifIndex,
	unsigned short vid,
	unsigned char  isTagged,
	unsigned char  *packet,
	unsigned long  length
)
{
	struct ether_header_t *layer2 = NULL;
	struct arp_packet_t	   *arpPacket = NULL;
	struct arp_snooping_item_s dbItem;
	unsigned int sip = 0,dip = 0;
	unsigned int retVal = ARP_RETURN_CODE_SUCCESS;
	int ret = 0;
	unsigned char sysmac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	unsigned char arpValid = TRUE, arpRelay = FALSE, isBC = FALSE;
	unsigned char brcmac[MAC_ADDR_LEN]= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    unsigned int    un_arp_inspection_status = 0; 
	unsigned int	gateway[MAX_IP_COUNT] = {0};
	unsigned int	mask[MAX_IP_COUNT] = {0};
	unsigned int	l3Index = 0;

	unsigned int 	learn_mode = 1;

	npd_syslog_dbg("receive packet from thread %d(%d)",getpid(),(int)syscall(SYS_gettid));
	if(NULL == packet) {
		syslog_ax_arpsnooping_err("npd process arp packet as packet null\r\n");
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	npd_module_learning_mode_get(netif_index, 0, &learn_mode);
	if (learn_mode == 0)
	{
		return retVal;
	}
	
	layer2 = (struct ether_header_t *)packet;
	arpPacket = (struct arp_packet_t *)((char*)packet + sizeof(struct ether_header_t));
	sip = *(unsigned int*)arpPacket->sip;
	dip = *(unsigned int*)arpPacket->dip;

    if(0 == memcmp(brcmac, layer2->dmac, MAC_ADDR_LEN))
	{
		isBC = TRUE;
	}
	
    memset(gateway, 0xff, sizeof(gateway));
	npd_arp_snooping_print_pkt(netif_index, ifIndex, vid, arpPacket);	

	/* TODO:check mac address*/
	retVal= npd_arp_snooping_mac_legality_check(layer2->smac,layer2->dmac, \
		                                arpPacket->smac,arpPacket->dmac,\
		                                ntohs(arpPacket->opCode));  /*the four macs legality check (muticast ,broadcast,zero mac ,etc)*/
	if(TRUE != retVal){
		syslog_ax_arpsnooping_err("arp packet mac legality check error \n");
        return ARP_RETURN_CODE_ERROR;
	}

    /* IP ARP_INSPECTION处理 */	
    npd_arp_inspection_check_global_endis(&un_arp_inspection_status);
    if (un_arp_inspection_status)
    {
        if (ARP_RETURN_CODE_ERROR == npd_arp_inspection_check(netif_index, vid, layer2, arpPacket))
        {
            char name[50];
            memset(name, 0, sizeof(name));

            npd_netif_index_to_user_fullname(netif_index, name);
            npd_syslog_official_event(
                "\nReceive an invalid arp packet which can not pass inspection.\n"
                "ARP packet sender IP %d.%d.%d.%d, MAC %.02x:%.02x:%.02x:%.02x:%.02x:%.02x \n"
                "           target IP %d.%d.%d.%d, MAC %.02x:%.02x:%.02x:%.02x:%.02x:%.02x \n",
                arpPacket->sip[0], arpPacket->sip[1], arpPacket->sip[2], arpPacket->sip[3],
                arpPacket->smac[0], arpPacket->smac[1], arpPacket->smac[2], arpPacket->smac[3],
                arpPacket->smac[4], arpPacket->smac[5], 
                arpPacket->dip[0], arpPacket->dip[1], arpPacket->dip[2], arpPacket->dip[3],
                arpPacket->dmac[0], arpPacket->dmac[1], arpPacket->dmac[2], arpPacket->dmac[3],
                arpPacket->dmac[4], arpPacket->dmac[5]);
           npd_syslog_official_event(
                "\nEthernet header source MAC %.02x:%.02x:%.02x:%.02x:%.02x:%.02x \n"
                "                  destination MAC %.02x:%.02x:%.02x:%.02x:%.02x:%.02x \n"
                "           from %s VLAN %d.\n",
                layer2->smac[0],layer2->smac[1],layer2->smac[2],layer2->smac[3],
                layer2->smac[4],layer2->smac[5],
                layer2->dmac[0],layer2->dmac[1],layer2->dmac[2],layer2->dmac[3],
                layer2->dmac[4],layer2->dmac[5],
                name, vid);

            return ARP_RETURN_CODE_ERROR;
        }
    }
	
#ifdef HAVE_CAPWAP_ENGINE	
    if( TRUE == isBC && ntohs(arpPacket->opCode) == 0x1 )
	{
		ret = npd_arp_snooping_find_item_byip(dip, &targetEntry);
		if(0 == ret&& NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(targetEntry.ifIndex))
		{
			npd_arp_snooping_reply_send(vid, netif_index, sip, arpPacket->smac, targetEntry.ipAddr, targetEntry.mac);
			arpRelay = TRUE;
		}
	}
#endif
    /*以下处理需要本机具有三层接口*/
 
	if(TRUE != npd_intf_get_global_l3index(ifIndex, &l3Index)){
		if(ARP_RETURN_CODE_ERROR == npd_arp_snooping_ip_valid_check(sip))
		{
			return ARP_RETURN_CODE_ERROR;
		}
		else {
			arpValid = FALSE;		
			l3Index = ~0UL;
		}
	}
	else	
	{
		ret = npd_intf_addr_ip_get(l3Index,gateway,mask);
		retVal = npd_arp_snooping_ip_legality_check(l3Index, gateway, mask, sip, dip);
		if(ARP_RETURN_CODE_SUCCESS != retVal){
			arpValid = FALSE;
			if(ARP_RETURN_CODE_CONFLICTED_IP == retVal){
	            /*有终端在仿冒交换机，发免费ARP进行修正*/
				if(vid != DEFAULT_VLAN_ID)
				{
    	            /*规避与默认配置的冲突*/
    		        memcpy(sysmac, PRODUCT_MAC_ADDRESS, MAC_ADDR_LEN);
    				nam_arp_gratuitous_send(netif_index,vid,sysmac,layer2->smac,dip);
				}
				return ARP_RETURN_CODE_ERROR;
			}
			else if(ARP_RETURN_CODE_CHECK_IP_ERROR == retVal)
			{
				return ARP_RETURN_CODE_ERROR;
			}
		}
	}

	if( un_arp_inspection_status 
		&& (FALSE == arpRelay)
		&& (ARP_RETURN_CODE_SUCCESS != npd_arp_snooping_ip_gateway_check(dip, gateway))
	  )
    {
        /*如果没有三层接口,且启动了ARP Inspection,需要在二层转发已经通过检查的ARP包，因为此时
	          包是trap到CPU而不是mirror上来的*/
        npd_packet_tx_broadcast_exclude_netif(2, vid, packet, length, netif_index);            
    }

	/* TODO: Check wireline ARP entry if need learn, depend on chip forwarding mode for
	         broadcast ARP packet*/
	if(arpValid == FALSE && NPD_NETIF_WIRELESS_TYPE != npd_netif_type_get(netif_index))
		return ARP_RETURN_CODE_ERROR;

	memset(&dbItem, 0, sizeof(struct arp_snooping_item_s));
    dbItem.l3Index = l3Index;
	dbItem.isStatic = (NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))?TRUE:FALSE;
	dbItem.isValid = arpValid;
	dbItem.ipAddr 	= sip;	/* usually we learning source ip address*/
	dbItem.isTagged = isTagged;
	dbItem.vid		= vid;
	dbItem.ifIndex  = netif_index;
	dbItem.flag     |= ARPSNP_FLAG_HIT;
    dbItem.time = arp_absolute_time;
	memcpy(dbItem.mac,arpPacket->smac,MAC_ADDRESS_LEN);

	retVal = npd_arp_snooping_learning( &dbItem );
	if(ARP_RETURN_CODE_SUCCESS != retVal) 
	{
		syslog_ax_arpsnooping_err("npd arp snooping learning error %d\n",retVal);
		return retVal;
	}
#ifdef HAVE_CAPWAP_ENGINE
	retVal = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_STA_ARP_ADD, &dbItem);
	if(ARP_RETURN_CODE_SUCCESS != retVal){
		syslog_ax_arpsnooping_err("func:%s return value error %d\r\n", __func__, retVal);
		return retVal;
	}
#endif	
	retVal = npd_arp_snooping_ip_gateway_check(dip, gateway);
	if(retVal != ARP_RETURN_CODE_SUCCESS){
		syslog_ax_arpsnooping_err("npd arp snooping ip gateway check error %d\n",retVal);
	}
	return retVal;
}


int npd_arp_inspection_statistics_init()
{
    int n_array_index = 0;

    for (n_array_index = 0; n_array_index < MAX_SWITCHPORT_PER_SYSTEM; n_array_index++)
    {
        npd_arp_inspection_statistics[n_array_index].is_valid = 0;
        npd_arp_inspection_statistics[n_array_index].eth_g_index = 0;
    }

    return 0;
}

int npd_arp_inspection_clear_statistics()
{
    int ni = 0;

    for (ni = 0; ni < MAX_SWITCHPORT_PER_SYSTEM; ni++)
    {
        npd_arp_inspection_statistics[ni].permit = 0;
        npd_arp_inspection_statistics[ni].drop = 0;
    }

    return 0;
}

int npd_arp_inspection_init_port_statistic(unsigned int eth_g_index)
{
    int n_array_index = 0;

    n_array_index = netif_array_index_from_ifindex(eth_g_index);

    syslog_ax_arpsnooping_dbg("AIS: init n_array_index = %d\n", n_array_index);

    if ((n_array_index < MAX_SWITCHPORT_PER_SYSTEM)
        && (n_array_index >= 0))
    {
        npd_arp_inspection_statistics[n_array_index].is_valid = 1;
        npd_arp_inspection_statistics[n_array_index].eth_g_index = eth_g_index;
		npd_arp_inspection_statistics[n_array_index].permit = 0;
        npd_arp_inspection_statistics[n_array_index].drop = 0;
    }

    return 0;
}

int npd_arp_inspection_clean_port_statistic(unsigned int eth_g_index)
{
    int n_array_index = 0;

    n_array_index = netif_array_index_from_ifindex(eth_g_index);

    syslog_ax_arpsnooping_dbg("AIS: clean n_array_index = %d\n", n_array_index);
    if ((n_array_index < MAX_SWITCHPORT_PER_SYSTEM)
        && (n_array_index >= 0))
    {
        npd_arp_inspection_statistics[n_array_index].is_valid = 0;
        npd_arp_inspection_statistics[n_array_index].eth_g_index = 0;  
		npd_arp_inspection_statistics[n_array_index].permit = 0;
        npd_arp_inspection_statistics[n_array_index].drop = 0;
    }

    return 0;
}

unsigned int npd_arp_inspection_global_status_get
(
	struct arp_inspection_status *user
)
{
	return dbtable_array_get(npd_arp_inspection_status_index,arp_inspection_global_no, user);
}

unsigned int npd_arp_inspection_global_status_insert
(
	struct arp_inspection_status *user
)
{
	return dbtable_array_insert(npd_arp_inspection_status_index,&arp_inspection_global_no, user);
}

unsigned int npd_arp_inspection_global_status_update
(
	struct arp_inspection_status *user
)
{
    unsigned int ret;
    npd_key_database_lock();
	ret = 	dbtable_array_update(npd_arp_inspection_status_index,arp_inspection_global_no,NULL, user);
    npd_key_database_unlock();
    return ret;
}

unsigned int npd_arp_inspection_check_global_endis
(
	unsigned int *endis
)
{
	struct arp_inspection_status user;
	memset(&user,0,sizeof(struct arp_inspection_status));
	if(!npd_arp_inspection_global_status_get(&user))
	{	
		syslog_ax_arpsnooping_dbg("global arp inspection is %s",user.arp_inspection_enable?"enable":"disable");
		*endis = user.arp_inspection_enable;
		return ARP_RETURN_CODE_SUCCESS;
	}else{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}	
}

unsigned int npd_arp_inspection_set_global_endis
(
	unsigned int endis
)
{
	struct arp_inspection_status user;
	memset(&user,0,sizeof(struct arp_inspection_status));
    
	if (!npd_arp_inspection_global_status_get(&user))
	{	
		if (user.arp_inspection_enable != endis)
		{
			user.arp_inspection_enable = endis ;
			syslog_ax_arpsnooping_dbg("global arp inspection will be set %s\r\n",user.arp_inspection_enable?"enable":"disable");
			npd_arp_inspection_global_status_update(&user);
			return ARP_RETURN_CODE_SUCCESS;
		}
        else
        {
            syslog_ax_arpsnooping_dbg("global arp inspection already set %s\r\n",user.arp_inspection_enable?"enable":"disable");
		    return ARP_RETURN_CODE_ALREADY_SET;
        }
	}
    else
	{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}

    return ARP_RETURN_CODE_SUCCESS;
}

unsigned int npd_arp_inspection_check_vlan_endis
(
	unsigned short vid,
	unsigned char *endis
)
{
	struct arp_inspection_status user;
	memset(&user,0,sizeof(struct arp_inspection_status));
	if(!npd_arp_inspection_global_status_get(&user))
	{	
		if(NPD_VBMP_MEMBER(user.allow_arp_vlans,vid))
		{
		    *endis = 1;
		    syslog_ax_arpsnooping_dbg("arp inspection in vlan %d is %s\r\n",vid,*endis?"enable":"disable");
		    return ARP_RETURN_CODE_SUCCESS;
		}else
		{
		    *endis = 0;
		    syslog_ax_arpsnooping_dbg("arp inspection in vlan %d is %s\r\n",vid,*endis?"enable":"disable");
		    return ARP_RETURN_CODE_SUCCESS;
		}
	}else{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}

    return ARP_RETURN_CODE_SUCCESS;
}


unsigned int npd_arp_inspection_traversal_vlan_endis
(
	unsigned short *vlanid
)
{
	struct arp_inspection_status user;
	memset(&user,0,sizeof(struct arp_inspection_status));
	unsigned short i = 0;
	if(!npd_arp_inspection_global_status_get(&user))
	{
		NPD_VBMP_ITER(user.allow_arp_vlans,i)
		{
			if(i > *vlanid)
			{
				*vlanid = i;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		return ARP_RETURN_CODE_ERROR;
	}else{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}
	return ARP_RETURN_CODE_SUCCESS;
}

int npd_arp_inspection_modify_port_by_trunk(struct arp_inspection_status* entry, int netif_index)
{
    int array_index = 0;
    int array_port_tindex = 0;
    struct trunk_s trunk;

    memset(&trunk, 0, sizeof(struct trunk_s));
    array_index = trunk_array_index_from_ifindex(netif_index);
    
    trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
    if (0 == dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk))
    {
        NPD_PBMP_ITER(trunk.ports, array_port_tindex)
        {
            entry->switch_port_control_count[array_port_tindex] = entry->switch_port_control_count[array_index];
        }
    }
    else
    {
        syslog_ax_arpsnooping_dbg("Get trunk struct failed!\n");
        return ARP_RETURN_CODE_ERROR;
    }
    return 0;
}

unsigned int npd_arp_inspection_set_vlan_endis
(
	unsigned short vid,
	unsigned char endis
)
{
    int array_port = 0;
    int netif_index = 0;
    int netif_type = 0;
    struct vlan_s vlan;
	struct arp_inspection_status user;
	memset(&user, 0, sizeof(struct arp_inspection_status));
    
	if (!npd_arp_inspection_global_status_get(&user))
	{	
		if (endis == NPD_VBMP_MEMBER(user.allow_arp_vlans,vid))
		{
			syslog_ax_arpsnooping_dbg("vlan arp inspection already set %s\r\n",endis?"enable":"disable");
			return  ARP_RETURN_CODE_ALREADY_SET;
		}

        memset(&vlan, 0, sizeof(struct vlan_s));

        vlan.vid = vid;
        if (0 != dbtable_sequence_search(g_vlans, vlan.vid, &vlan))
        {
            syslog_ax_arpsnooping_dbg("Get vlan struct failed!\n");
            return ARP_RETURN_CODE_ERROR;
        }
        
		if (!endis)
		{
			NPD_VBMP_VLAN_REMOVE(user.allow_arp_vlans,vid);
			syslog_ax_arpsnooping_dbg("vlan arp inspection set %s\r\n",endis?"enable":"disable");

            NPD_PBMP_ITER(vlan.untag_ports, array_port)
            {
                if (user.switch_port_control_count[array_port] > 0)
                {
                    user.switch_port_control_count[array_port]--;
                    netif_index = netif_array_index_to_ifindex(array_port);
                    netif_type = npd_netif_type_get(netif_index);
                
                    if (NPD_NETIF_TRUNK_TYPE == netif_type)
                    {
                        if (0 != npd_arp_inspection_modify_port_by_trunk(&user, netif_index))
                        {
                            syslog_ax_arpsnooping_dbg("Modify port by trunk failed!\n");
                            return ARP_RETURN_CODE_ERROR;
                        }
                    }
                }
            }
            
            NPD_PBMP_ITER(vlan.tag_ports, array_port)
            {
                if (user.switch_port_control_count[array_port] > 0)
                {
                    user.switch_port_control_count[array_port]--;
                    netif_index = netif_array_index_to_ifindex(array_port);
                    netif_type = npd_netif_type_get(netif_index);
                
                    if (NPD_NETIF_TRUNK_TYPE == netif_type)
                    {
                        if (0 != npd_arp_inspection_modify_port_by_trunk(&user, netif_index))
                        {
                            syslog_ax_arpsnooping_dbg("Modify port by trunk failed!\n");
                            return ARP_RETURN_CODE_ERROR;
                        }
                    }
                }
            }
		}
        else
        {
			NPD_VBMP_VLAN_ADD(user.allow_arp_vlans,vid);
			syslog_ax_arpsnooping_dbg("vlan arp inspection set %s\r\n",endis?"enable":"disable");

            NPD_PBMP_ITER(vlan.untag_ports, array_port)
            {
                user.switch_port_control_count[array_port]++;
                netif_index = netif_array_index_to_ifindex(array_port);
                netif_type = npd_netif_type_get(netif_index);
                
                if (NPD_NETIF_TRUNK_TYPE == netif_type)
                {
                    if (0 != npd_arp_inspection_modify_port_by_trunk(&user, netif_index))
                    {
                        syslog_ax_arpsnooping_dbg("Modify port by trunk failed!\n");
                        return ARP_RETURN_CODE_ERROR;
                    }
                }
            }
            
            NPD_PBMP_ITER(vlan.tag_ports, array_port)
            {
                user.switch_port_control_count[array_port]++;

                netif_index = netif_array_index_to_ifindex(array_port);
                netif_type = npd_netif_type_get(netif_index);
                
                if (NPD_NETIF_TRUNK_TYPE == netif_type)
                {
                    if (0 != npd_arp_inspection_modify_port_by_trunk(&user, netif_index))
                    {
                        syslog_ax_arpsnooping_dbg("Modify port by trunk failed!\n");
                        return ARP_RETURN_CODE_ERROR;
                    }
                }
            }
		}
	}
    else
    {
        syslog_ax_arpsnooping_dbg("get faile!\n");
        return ARP_RETURN_CODE_ERROR;
    }

    npd_arp_inspection_global_status_update(&user);

	return ARP_RETURN_CODE_SUCCESS;
}



unsigned int npd_arp_inspection_check_trust
(
	unsigned int eth_g_index,
	unsigned int *trust_mode,
	struct arp_inspection_status* user
)
{
	unsigned int ret;
	unsigned int port_index;

	if(NULL != user)
	{	
		port_index = eth_port_array_index_from_ifindex(eth_g_index);
		ret = NPD_PBMP_MEMBER(user->trust,port_index);
		if(ret == 1){
			*trust_mode = 1;
			return ARP_RETURN_CODE_SUCCESS;	
		}else{
			*trust_mode = 0;
			return ARP_RETURN_CODE_SUCCESS;	
		}		
	}
    else
	{
		return ARP_RETURN_CODE_ERROR;
	}

    return ARP_RETURN_CODE_ERROR;
}


unsigned int npd_arp_inspection_traversal_trust_check
(
	unsigned int *trust_cont,
	unsigned int *trust_port
)
{
	struct arp_inspection_status user;
	unsigned int g_index = 0;
	unsigned int if_index =0;
	memset(&user,0,sizeof(struct arp_inspection_status));
	if(!npd_arp_inspection_global_status_get(&user))
	{	
		NPD_PBMP_ITER(user.trust,g_index)
		{
			if_index = eth_port_array_index_to_ifindex(g_index);
			syslog_ax_arpsnooping_dbg("g_index = %d ! if_index = %d\n",g_index,if_index);
			*(trust_port + *trust_cont) = if_index;
			(*trust_cont) ++;
			syslog_ax_arpsnooping_dbg("arp inspection in port %d trust\r\n",g_index);
		}
		syslog_ax_arpsnooping_dbg("get success!\n");
		return ARP_RETURN_CODE_SUCCESS;
	}else{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}
}

unsigned int npd_arp_inspection_traversal_trust_nextget
(
	unsigned int *trust_port
)
{
	struct arp_inspection_status user;
	unsigned int if_index = 0;
	int g_index = -1;
	unsigned int p_index = 0;
	
	if_index = *trust_port;

	if(0 != if_index)
	{
		g_index = eth_port_array_index_from_ifindex(if_index);
	}
	memset(&user,0,sizeof(struct arp_inspection_status));
	if(!npd_arp_inspection_global_status_get(&user))
	{
		NPD_PBMP_ITER(user.trust,p_index)
		{
			if((int)p_index > g_index)
			{
				if_index = eth_port_array_index_to_ifindex(p_index);
				*trust_port = if_index;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		return ARP_RETURN_CODE_ERROR;
	}
    else
	{
		syslog_ax_arpsnooping_dbg("get faile!\n");
		return ARP_RETURN_CODE_ERROR;
	}
}


unsigned int npd_arp_inspection_set_trust
(
	unsigned int trust_mode,
	unsigned int eth_g_index 
)
{
	struct arp_inspection_status user;
	unsigned int ret =0;
	unsigned int port_index;
	memset(&user,0,sizeof(struct arp_inspection_status));
	if(!npd_arp_inspection_global_status_get(&user))
	{	
		port_index = eth_port_array_index_from_ifindex(eth_g_index);
		syslog_ax_arpsnooping_dbg("eth_g_index = %d port_index  = %d\n",eth_g_index,port_index);
		ret = NPD_PBMP_MEMBER(user.trust,port_index);
		syslog_ax_arpsnooping_dbg("user.trust = %d\n",user.trust);
		syslog_ax_arpsnooping_dbg("ret = %d trustmode = %d\n",ret,trust_mode);
		if(trust_mode == ret){
			syslog_ax_arpsnooping_dbg("arp inspection in the port already set %s\r\n",trust_mode?"trust":"notrust");
			return ARP_RETURN_CODE_SUCCESS;	
		}
		if(trust_mode){
			NPD_PBMP_PORT_ADD(user.trust,port_index);
			syslog_ax_arpsnooping_dbg("arp inspection in the port set %s\r\n",trust_mode?"trust":"notrust");
			npd_arp_inspection_global_status_update(&user);
			return ARP_RETURN_CODE_SUCCESS;
		}
		if(!trust_mode){
			NPD_PBMP_PORT_REMOVE(user.trust,port_index);
			npd_arp_inspection_global_status_update(&user);
			syslog_ax_arpsnooping_dbg("arp inspection in the port set %s\r\n",trust_mode?"trust":"notrust");
			return ARP_RETURN_CODE_SUCCESS;
		}
	}
	
	syslog_ax_arpsnooping_dbg("get faile!\n");
	return ARP_RETURN_CODE_ERROR;
}

unsigned int npd_arp_inspection_validate_type_check
(
	struct arp_inspection_status *user,
	unsigned int aiv_type,
	unsigned char *status
)
{
	unsigned char temp = ARP_RETURN_CODE_SUCCESS;
	{	
		temp = user->arp_inspection_type;
		if(aiv_type == ARP_INSPECTION_SRC_MAC){
			if(temp&IP_ARP_INSPECTION_VALIDATE_SMAC_CHECK){
				*status = 1;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		if(aiv_type == ARP_INSPECTION_DST_MAC){
			if(temp&IP_ARP_INSPECTION_VALIDATE_DMAC_CHECK){
				*status = 1;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		if(aiv_type == ARP_INSPECTION_IP){
			if((temp&IP_ARP_INSPECTION_VALIDATE_IP_CHECK) && (!user->allowzero)){
				*status = 1;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		if(aiv_type == ARP_INSPECTION_IP_ZERO){
			if((temp&IP_ARP_INSPECTION_VALIDATE_IP_CHECK) && (user->allowzero)){
				*status = 1;
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		*status = 0;
		return ARP_RETURN_CODE_SUCCESS;		
	}
}

unsigned int npd_arp_inspection_validate_type_set
(
	struct arp_inspection_status *user,
	unsigned int aiv_type,
	unsigned int op,
	unsigned int allowzero
)
{
	unsigned char status = ARP_RETURN_CODE_SUCCESS;
	{	
		if(!npd_arp_inspection_validate_type_check(user, aiv_type,&status))
		{
			if( status == op )
			{
				return ARP_RETURN_CODE_ALREADY_SET;
			}
		}
		else{
			return ARP_RETURN_CODE_ERROR;
		}


		if(aiv_type == ARP_INSPECTION_SRC_MAC)
		{
			if(op == 1){
				user->arp_inspection_type |= IP_ARP_INSPECTION_VALIDATE_SMAC_CHECK;
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}
			if(op == 0){
				user->arp_inspection_type &= (~IP_ARP_INSPECTION_VALIDATE_SMAC_CHECK);
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}
		}
		if(aiv_type == ARP_INSPECTION_DST_MAC)
		{
			if(op == 1){
				user->arp_inspection_type |= IP_ARP_INSPECTION_VALIDATE_DMAC_CHECK;
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}
			if(op == 0){
				user->arp_inspection_type &= (~IP_ARP_INSPECTION_VALIDATE_DMAC_CHECK);
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}			
		}
		if((aiv_type == ARP_INSPECTION_IP)||(aiv_type == ARP_INSPECTION_IP_ZERO))
		{
			user->allowzero = allowzero;
			if(op == 1){
				user->arp_inspection_type |= IP_ARP_INSPECTION_VALIDATE_IP_CHECK;
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}
			if(op == 0){
				user->arp_inspection_type &= (~IP_ARP_INSPECTION_VALIDATE_IP_CHECK);
				npd_arp_inspection_global_status_update(user);
				return ARP_RETURN_CODE_SUCCESS;
			}	
		}
	}

	
	return ARP_RETURN_CODE_ERROR;
}

unsigned int npd_dhcp_snp_query_arp_inspection
(
    unsigned int netif_index,
	struct arp_packet_t	   *arpPacket,
	unsigned char *isFound
)
{
	NPD_DHCP_SNP_USER_ITEM_T item;
	memset(&item,0,sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	
	if (!isFound) {
		syslog_ax_arpsnooping_dbg("query dhcp snp db for arp proxy error, parameter is null.\n");
		return ARP_RETURN_CODE_ERROR;
	}
	memcpy(item.chaddr,arpPacket->smac,MAC_ADDR_LEN);
#ifdef HAVE_DHCP_SNP
	if(!npd_dhcp_snp_tbl_item_find(&item))
	{		
		if ((*(unsigned int*)arpPacket->sip == item.ip_addr)
            && (netif_index == item.ifindex))
		{
			*isFound = 1;
			return ARP_RETURN_CODE_SUCCESS;
		}else
		{		
			*isFound = 0;
			return ARP_RETURN_CODE_SUCCESS;
		}		
	}
#endif
	return ARP_RETURN_CODE_ERROR;
}

unsigned int npd_arp_inspection_reserved_ip_check
(
    unsigned int sip,
    unsigned int dip,
	struct arp_packet_t	 *arpPacket,
	struct arp_inspection_status *user,
	unsigned char *endis
)
{
    unsigned int ip_check;
	ip_check = ntohl(sip);
	if(((0 == sip)&&(user->allowzero == 0))||
		(~0UL == ip_check)||
		(0x0000007F == (ip_check >> 24))||
		(0x0000BFFF == (ip_check >> 16))||
		(0x00C00000 == (ip_check >> 8))||
		((ip_check >> 8) >= 0x00DFFFFF))
	{
		*endis = 0;
		return DHCP_SNP_RETURN_CODE_OK;			
	}
	ip_check = ntohl(dip);
	if(ntohs(arpPacket->opCode) != ARP_REQUEST){

		if((0==ip_check)||
			(~0UL == ip_check)||
			(0x0000007F == (ip_check >> 24))||
			(0x0000BFFF == (ip_check >> 16))||
			(0x00C00000 == (ip_check >> 8))||
			((ip_check >> 8) >= 0x00DFFFFF))
		{
			*endis = 0;
			return DHCP_SNP_RETURN_CODE_OK;			
		}
	}
	*endis = 1;
	return DHCP_SNP_RETURN_CODE_OK;			
}


unsigned int npd_query_arp_inspection
(
	struct ether_header_t *layer2 ,
	struct arp_packet_t	   *arpPacket,
	struct arp_inspection_status *user,
    unsigned int aiv_type,
	unsigned char *isFound
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int sip = DHCP_SNP_RETURN_CODE_OK;
	unsigned int dip = DHCP_SNP_RETURN_CODE_OK;

	sip = *(unsigned int*)arpPacket->sip;
	dip = *(unsigned int*)arpPacket->dip;

	if(aiv_type == ARP_INSPECTION_SRC_MAC)
	{
		ret = memcmp(layer2->smac,arpPacket->smac,sizeof(arpPacket->smac));
		if(!ret)
		{
			*isFound = 1;
			return DHCP_SNP_RETURN_CODE_OK;			
		}else
		{
			*isFound = 0;
			return DHCP_SNP_RETURN_CODE_OK;			
		}
	}
	if(aiv_type == ARP_INSPECTION_DST_MAC)
	{
		if(ntohs(arpPacket->opCode) == ARP_REPLY)
		{
			ret = memcmp(layer2->dmac,arpPacket->dmac,sizeof(arpPacket->dmac));
			if(!ret)
			{
				*isFound = 1;
				return DHCP_SNP_RETURN_CODE_OK;			
			}else
			{
				*isFound = 0;
				return DHCP_SNP_RETURN_CODE_OK;			
			}
		}else{
    		*isFound = 1;
			return DHCP_SNP_RETURN_CODE_OK;			
		}
	}

	if((aiv_type == ARP_INSPECTION_IP)||(aiv_type == ARP_INSPECTION_IP_ZERO))
	{	
		npd_arp_inspection_reserved_ip_check(sip, dip,  arpPacket,user, isFound);
		return DHCP_SNP_RETURN_CODE_OK;		
	}


/*

		if(npd_arp_inspection_global_status_get(&user)){
			syslog_ax_arpsnooping_dbg("get arp db faile!\n");
			return DHCP_SNP_RETURN_CODE_ERROR;			
		}
		if(aiv_type == ARP_INSPECTION_IP_ZERO)
		{
			if(((arpPacket->opCode == ARP_REPLY)&&(0==sip)&&(0==dip))||
				((arpPacket->opCode == ARP_REQUEST)&&(0==sip)))
			isFound = 1;
			syslog_ax_arpsnooping_dbg("ip check pass!\n");
			return DHCP_SNP_RETURN_CODE_OK;			
		}
		
		if(((0==sip)&&(user.allowzero == 0)) ||
			(~0UI == sip)||
			(0x7F000001 == sip ) ||
			(0xA9FE == ((sip >> 16) & 0xffff))||
			((arpPacket->sip[0] > 0xE0)&&(arpPacket->sip[0] < 0xF0))) {

			isFound = 0;
			syslog_ax_arpsnooping_dbg("ip check faile!\n");
			return DHCP_SNP_RETURN_CODE_ERROR;			
		}

		
		if(arpPacket->opCode == ARP_REPLY){
			if(((0==dip)&&(user.allowzero == 0)) ||
				(~0UI == dip)||
				(0x7F000001 == dip ) ||
				(0xA9FE == ((dip >> 16) & 0xffff))||
				((arpPacket->dip[0] >= 0xE0)&&(arpPacket->dip[0] < 0xF0))) {

				isFound = 0;
				syslog_ax_arpsnooping_dbg("ip check faile!\n");
				return DHCP_SNP_RETURN_CODE_ERROR;			
			}
		}
		isFound = 1;
		syslog_ax_arpsnooping_dbg("ip check pass!\n");
		return DHCP_SNP_RETURN_CODE_OK;		*/	
	return DHCP_SNP_RETURN_CODE_OK;	
}

DBusMessage * npd_dbus_arp_inspection_enable(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int isEnable = 2;
	unsigned int ret = 0;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&isEnable,
		DBUS_TYPE_INVALID))) {
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = npd_arp_inspection_set_global_endis(isEnable);
	if(ret != ARP_RETURN_CODE_SUCCESS){
		syslog_ax_arpsnooping_err("set enable faile.\n");
		}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	return reply;
}


	

DBusMessage *npd_dbus_arp_inspection_vlan_enable
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned char isEnable = ARP_RETURN_CODE_SUCCESS;
	unsigned short vlanId = ARP_RETURN_CODE_SUCCESS;
	unsigned int flag = ARP_RETURN_CODE_SUCCESS;
	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_arpsnooping_err("return error caused dbus.\n");
		return NULL;
	}
	syslog_ax_arpsnooping_dbg("To set ARP inspection at vlan %d. The value is %d\n",vlanId,isEnable);

	ret = npd_arp_inspection_check_global_endis(&flag);
	if (!ret)
	{
		if (flag)
		{
			ret = npd_arp_inspection_set_vlan_endis(vlanId,isEnable);
			if(ret != ARP_RETURN_CODE_SUCCESS){
				syslog_ax_arpsnooping_err("set vlan faile.\n");
				ret = ARP_RETURN_CODE_ERROR;
			}
		}
        else
        {
			syslog_ax_arpsnooping_err("global inspection not enable.\n");
			ret = ARP_RETURN_CODE_INSP_NOTENABLE;
		}
	}
    else
    {
		syslog_ax_arpsnooping_err("get tbl error!.\n");
		ret = ARP_RETURN_CODE_ERROR;	
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}
    

DBusMessage * npd_dbus_arp_inspection_trust_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int trust_mode = ARP_RETURN_CODE_SUCCESS;
	unsigned int eth_g_index = ARP_RETURN_CODE_SUCCESS;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned int flag = ARP_RETURN_CODE_SUCCESS;
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32, &eth_g_index,
			DBUS_TYPE_UINT32, &trust_mode,
			DBUS_TYPE_INVALID))) {
			syslog_ax_arpsnooping_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
		return NULL;
	}
	ret = npd_arp_inspection_check_global_endis(&flag);
	if(!ret)
	{
		if(flag){
			ret = npd_arp_inspection_set_trust(trust_mode,eth_g_index);
			if(ret != ARP_RETURN_CODE_SUCCESS){
				syslog_ax_arpsnooping_err("set trust faile.\n");
				ret = ARP_RETURN_CODE_ERROR;
			}
		}else{
			ret = ARP_RETURN_CODE_INSP_NOTENABLE;
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	return reply;
}

DBusMessage * npd_dbus_arp_inspection_validate_set(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int aiv_type = ARP_INSPECTION_INVALID;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned int allowzero = ARP_RETURN_CODE_SUCCESS;
	unsigned int op;
	struct arp_inspection_status user = {0};

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32, &aiv_type,
			DBUS_TYPE_UINT32, &op,
			DBUS_TYPE_UINT32, &allowzero,
			DBUS_TYPE_INVALID))) {
			syslog_ax_arpsnooping_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
	}
	memset(&user,0,sizeof(struct arp_inspection_status));
	if (0 != npd_arp_inspection_global_status_get(&user))
	{
		syslog_ax_arpsnooping_dbg("Failed to get the global status of ARP inspection!\n");
		ret = ARP_RETURN_CODE_ERROR;
	}	
    else
    {
    	if (user.arp_inspection_enable)
    	{
    	    ret = npd_arp_inspection_validate_type_set(&user, aiv_type,op,allowzero);
    	}
        else
        {
            ret = ARP_RETURN_CODE_INSP_NOTENABLE;
        }
    }

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	return reply;
}

DBusMessage * npd_dbus_arp_inspection_check_global(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;	
	dbus_error_init(&err);
	struct arp_inspection_status user;
	
	memset (&user,0,sizeof(user));
	ret = npd_arp_inspection_global_status_get(&user);
	if(ret)
	{
		syslog_ax_arpsnooping_err("get global status faile.\n");
		ret = ARP_RETURN_CODE_ERROR;
	}
	syslog_ax_arpsnooping_dbg("get global status %s success.\n",user.arp_inspection_enable ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(user.arp_inspection_enable));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.arp_inspection_type));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(user.allowzero));

	return reply;
}

DBusMessage * npd_dbus_arp_inspection_check_vlan_by_vid(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned short vlanid = ARP_RETURN_CODE_SUCCESS;
	unsigned char endis = ARP_RETURN_CODE_SUCCESS;
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanid,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_arpsnooping_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_arpsnooping_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_arp_inspection_check_vlan_endis(vlanid,&endis);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &endis);

	return reply;
}

DBusMessage * npd_dbus_arp_inspection_check_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned short vlanid = 0;
	dbus_error_init(&err);
	syslog_ax_arpsnooping_dbg("npd_dbus_arp_inspection_check_vlan!\n ");
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanid,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_arpsnooping_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_arpsnooping_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_arp_inspection_traversal_vlan_endis(&vlanid);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16,
									 &vlanid);

	return reply;
}

DBusMessage * npd_dbus_arp_inspection_check_trust(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned int trust_port = 0;
	dbus_error_init(&err);
	syslog_ax_arpsnooping_dbg("npd_dbus_arp_inspection_check_trust!\n ");
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &trust_port,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_arpsnooping_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_arpsnooping_err("return error caused dbus.\n");
		return NULL;
	}

	//ret = npd_arp_inspection_traversal_trust_check(&trust_cnt,trust_port);
	ret = npd_arp_inspection_traversal_trust_nextget(&trust_port);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &trust_port);


	return reply;
}

int npd_dbus_save_static_arp
(
    hash_table_index_t *hash, 
	struct arp_snooping_item_s *dbItem,
	int flag
)
{
	unsigned short trunkId;
	char ifName[32];
	char* tmp = NULL,*buf = NULL;
	int len = 0;
	char ipstring[16];
	
	if( NULL == dbItem || npd_arp_showStr == NULL ) {
		return 0;
	}
	
	buf = npd_arp_showStr + npd_arp_showStr_len;
	tmp = buf;

	if(0 != dbItem->ipAddr) {
		if((npd_arp_showStr_len + 65) > NPD_INTF_SAVE_STATIC_ARP_MEM) {
			return 0;
		}
		else {
			memset(ifName, 0, 32);
			memset(ipstring, 0, 16);
	        lib_get_string_from_ip(ipstring,dbItem->ipAddr);
			if( NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(dbItem->ifIndex) ){
				if( 0 == parse_eth_index_to_name(dbItem->ifIndex, ifName) )
				{
					len = sprintf(buf,"ip static-arp %s %02x:%02x:%02x:%02x:%02x:%02x %s\n",\
						ifName,dbItem->mac[0],dbItem->mac[1],dbItem->mac[2],dbItem->mac[3],\
						dbItem->mac[4],dbItem->mac[5], ipstring);
				}
			}
			else {
				trunkId = npd_netif_trunk_get_tid(dbItem->ifIndex);
				len = sprintf(buf,"ip static-arp port-channel %d %02x:%02x:%02x:%02x:%02x:%02x %s\n",\
					trunkId,dbItem->mac[0],dbItem->mac[1],dbItem->mac[2],dbItem->mac[3],\
					dbItem->mac[4],dbItem->mac[5], ipstring);
			}
		}
	}
	else 
		return 0;

	npd_arp_showStr_len += len;
	return 0;
}

void npd_dbus_save_arp_inspection_cfg
(
	char** buffer,
	unsigned int* avalidLen
)
{
	unsigned int len = 0;
	unsigned int switch_port_index = 0;
	unsigned int if_index = 0;
	unsigned int vid = 0;
	char *showStr = NULL;
	char port_name[50] = {0};
	char * arpChecksmac =		"ip arp inspection validate src-mac\n";
	char * arpCheckdmac =		"ip arp inspection validate dst-mac\n";
	char * arpCheckip   =		"ip arp inspection validate ip\n";
	char * arpCheckipallowzero =		"ip arp inspection validate allow-zero-ip\n";
	char * arpInspectionStr = 	"ip arp inspection enable\n";
	char * arpInspectiontrustStr =	"ip arp inspection trust\n";
	struct arp_inspection_status user;
	memset(&user,0,sizeof(struct arp_inspection_status));
    if (0 != npd_arp_inspection_global_status_get(&user))
    {
        return ;
    }
    
	if((NULL == buffer )||(NULL == *buffer)||(NULL == avalidLen)) {
		return ;
	}
	else
	{
        showStr = *buffer;
        
		if(NPD_TRUE == user.arp_inspection_enable) {	
			if((strlen(arpInspectionStr)+1) > *avalidLen){
				return ;
			}
			len = sprintf((showStr),arpInspectionStr);
			showStr += len;
			*avalidLen -= len;
		}
		else
			return;

		if((user.arp_inspection_type) & IP_ARP_INSPECTION_VALIDATE_SMAC_CHECK)
		{
			if((strlen(arpChecksmac)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpChecksmac);
			showStr += len;
			*avalidLen -= len;
		}
		
		if((user.arp_inspection_type) & IP_ARP_INSPECTION_VALIDATE_DMAC_CHECK)
		{
			if((strlen(arpCheckdmac)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpCheckdmac);
			showStr += len;
			*avalidLen -= len;
		}
		
		if((NPD_TRUE != user.allowzero)&&((user.arp_inspection_type) & IP_ARP_INSPECTION_VALIDATE_IP_CHECK))
		{
			if((strlen(arpCheckip)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpCheckip);
			showStr += len;
			*avalidLen -= len;
		}
		
		if((NPD_TRUE == user.allowzero)&&((user.arp_inspection_type) & IP_ARP_INSPECTION_VALIDATE_IP_CHECK))
		{
			if((strlen(arpCheckipallowzero)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpCheckipallowzero);
			showStr += len;
			*avalidLen -= len;
		}

		NPD_VBMP_ITER(user.allow_arp_vlans,vid)
		{
			if(10 > *avalidLen)
				return ;
			len = sprintf((showStr),"vlan %d\n",vid);
			showStr += len;
			*avalidLen -= len;

			if((strlen(arpInspectionStr)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpInspectionStr);
			showStr += len;
			*avalidLen -= len;

			if(5 > *avalidLen)
				return ;
			len = sprintf((showStr),"exit\n");
			showStr += len;
			*avalidLen -= len;			
		}
		
		NPD_PBMP_ITER(user.trust,switch_port_index)
		{

			if_index = eth_port_array_index_to_ifindex(switch_port_index);

			npd_netif_index_to_user_fullname(if_index,port_name);
			if(40 > *avalidLen)
				return ;
			len = sprintf((showStr),"interface %s\n",port_name);
			showStr += len;
			*avalidLen -= len;

			if((strlen(arpInspectiontrustStr)+1) > *avalidLen){
	            return ;
			}
			len = sprintf((showStr),arpInspectiontrustStr);
			showStr += len;
			*avalidLen -= len;

			if(5 > *avalidLen)
				return ;
			len = sprintf((showStr),"exit\n");
			showStr += len;
			*avalidLen -= len;	
		}
	}
    
	return ;
}

DBusMessage * npd_dbus_ip_static_arp(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned int ipno = 0,ipmaskLen = 0;
	unsigned short vlanId = 0;
	unsigned char macAddr[6]= {0};
	unsigned int ifindex = ~0UI,eth_g_index = 0, g_ifindex = 0;
	unsigned char isTagged = FALSE;
	unsigned char baseMac[6]={0};

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		         DBUS_TYPE_UINT32,&eth_g_index,
				 DBUS_TYPE_BYTE,&macAddr[0],
				 DBUS_TYPE_BYTE,&macAddr[1],
				 DBUS_TYPE_BYTE,&macAddr[2],
				 DBUS_TYPE_BYTE,&macAddr[3],
				 DBUS_TYPE_BYTE,&macAddr[4],
				 DBUS_TYPE_BYTE,&macAddr[5],
				 DBUS_TYPE_UINT32,&ipno,
				 DBUS_TYPE_UINT32,&ipmaskLen,
				 DBUS_TYPE_INVALID))) {
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_arpsnooping_dbg("npd_arpsnooping: ip static-arp 0x%x %02x:%02x:%02x:%02x:%02x:%02x ip 0x%x ipmaskLen: %d vid: %d\n",
			                        eth_g_index,macAddr[0],macAddr[1],macAddr[2],
			                        macAddr[3],macAddr[4],macAddr[5],ipno, ipmaskLen,vlanId);
    syslog_ax_arpsnooping_dbg("%-15s:%ld.%ld.%ld.%ld\n"," IP",(ipno>>24)&0xFF,(ipno>>16)&0xFF, \
				(ipno>>8)&0xFF,ipno&0xFF);	

	ret = npd_intf_vid_get_by_ip(ipno,eth_g_index,&vlanId);
	if(ARP_RETURN_CODE_SUCCESS == ret)
	{
		if(NPD_MAX_VLAN_ID != vlanId)
		{
			ret = npd_check_vlan_real_exist(vlanId);
			if(ret != TRUE){
				syslog_ax_arpsnooping_err("vid %d is bad paramter!\n",vlanId);
				ifindex = ~0UI;
				ret = ARP_RETURN_CODE_VLAN_NOTEXISTS;
			}
			else if(FALSE == npd_vlan_check_contain_port(vlanId,eth_g_index,&isTagged)){
	            syslog_ax_arpsnooping_err("the port_index 0x%x is not in the vlan %d.\n",eth_g_index,vlanId);
				ret = ARP_RETURN_CODE_PORT_NOT_IN_VLAN;
			}
			else{
                ret = ARP_RETURN_CODE_SUCCESS;
			}
		}

		if(ARP_RETURN_CODE_SUCCESS == ret){
			if(TRUE != npd_intf_port_check(vlanId,eth_g_index,&ifindex)) {
				syslog_ax_arpsnooping_err("layer 3 interface not exist!\n");
				ret = ARP_RETURN_CODE_INTERFACE_NOTEXIST;
			}
			else if(ARP_RETURN_CODE_NO_HAVE_THE_IP !=(ret = npd_arp_snooping_check_ip_address(ipno,vlanId,eth_g_index)))
			{
	            if(ARP_RETURN_CODE_HAVE_THE_IP == ret){
	                syslog_ax_arpsnooping_err("ip is the same as system!\n");
				}else if(ARP_RETURN_CODE_CHECK_IP_ERROR == ret){
	                syslog_ax_arpsnooping_err("ip check error!\n");
				}else if(ARP_RETURN_CODE_NOT_SAME_SUB_NET == ret){
				    syslog_ax_arpsnooping_err("ip not in the same subnet!\n");
				}else if(ARP_RETURN_CODE_NO_HAVE_ANY_IP == ret){
	 			    syslog_ax_arpsnooping_err("interface ip not set!\n");
				}else if(ARP_RETURN_CODE_CHECK_IP_ERROR == ret){
	                syslog_ax_arpsnooping_err("check interface ip failed!\n");
				}
			}
			else{
				ret = npd_intf_get_intf_mac(ifindex,baseMac);
				if(0 == memcmp(macAddr,baseMac,MAC_ADDRESS_LEN))
				{	
					syslog_ax_arpsnooping_err("try to add static-arp with interface mac address FAILED!\n");
		            ret = ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC;
				}
				else{
					if(INTERFACE_RETURN_CODE_SUCCESS != ret){
			            syslog_ax_arpsnooping_dbg("get the interface mac address FAILED!\n");
					}
					npd_intf_get_global_l3index(ifindex, &g_ifindex);
				    ret = npd_arp_snooping_create_static_arp(g_ifindex,ipno,ipmaskLen,macAddr,vlanId,eth_g_index);
				}
		    }
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);									 
	return reply;								 

}

DBusMessage * npd_dbus_no_ip_static_arp(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int eth_g_index = 0;
	unsigned int arpCount = 0;
	unsigned int ipAddr = 0;
	unsigned int mask = 0;
	unsigned int flag = 0;
	unsigned int kern_del_flag = TRUE;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned char mac[6] = {0};
	struct arp_snooping_item_s item;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&eth_g_index,
								    DBUS_TYPE_UINT32,&ipAddr,
								    DBUS_TYPE_UINT32,&mask,
									DBUS_TYPE_BYTE, &mac[0],
									DBUS_TYPE_BYTE, &mac[1],
									DBUS_TYPE_BYTE, &mac[2],
									DBUS_TYPE_BYTE, &mac[3],
									DBUS_TYPE_BYTE, &mac[4],
									DBUS_TYPE_BYTE, &mac[5],
								    DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    if(0x1 == flag)
    {
		ret = npd_arp_snooping_find_item_byip(ipAddr, &item);
		if(ret == 0)
		{
			ret = npd_arp_snooping_del_static(npd_arpsnp_haship_index, &item, TRUE);
			syslog_ax_arpsnooping_dbg("static arp delete by ip success ! \n");
		}
		ret = ARP_RETURN_CODE_SUCCESS;
    }
	else if(0x2 == flag)
	{
		memcpy(item.mac, mac, MAC_ADDRESS_LEN);
		arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index,NPD_TRUE,&item,\
										npd_arp_snooping_filter_by_mac,npd_arp_snooping_del_static);
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",arpCount);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	else if(0x3 == flag)
	{
    	arpCount = npd_arp_snooping_del_static_by_ifindex(eth_g_index);
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",arpCount);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	else if(0x4 == flag)
	{
		npd_arp_snooping_del_static_by_network(ipAddr,mask);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	else if(0xf == flag)
	{
    	arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index,kern_del_flag,NULL,NULL,npd_arp_snooping_del_static);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	return reply;

}


DBusMessage *npd_dbus_clear_arp(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int eth_g_index = 0;
	unsigned int arpCount = 0;
	unsigned int ipAddr = 0;
	unsigned int mask = 0;
	unsigned int flag = 0;
	unsigned int kern_del_flag = TRUE;
	unsigned int ret = ARP_RETURN_CODE_SUCCESS;
	unsigned char mac[6] = {0};
	struct arp_snooping_item_s item;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                            		DBUS_TYPE_UINT32,&eth_g_index,
                            	    DBUS_TYPE_UINT32,&ipAddr,
                            	    DBUS_TYPE_UINT32,&mask,
									DBUS_TYPE_BYTE, &mac[0],
									DBUS_TYPE_BYTE, &mac[1],
									DBUS_TYPE_BYTE, &mac[2],
									DBUS_TYPE_BYTE, &mac[3],
									DBUS_TYPE_BYTE, &mac[4],
									DBUS_TYPE_BYTE, &mac[5],
                            	    DBUS_TYPE_UINT32,&flag,
                            		DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(0x1 == flag)
	{
		ret = npd_arp_snooping_find_item_byip(ipAddr, &item);
		if(ret == 0)
		{
			ret = npd_arp_snooping_del(npd_arpsnp_haship_index, &item, TRUE);
			syslog_ax_arpsnooping_dbg("Arp delete by ip success ! \n");
		}
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	else if(0x2 == flag)
	{
		memcpy(item.mac, mac, MAC_ADDR_LEN);

		arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index,NPD_TRUE,&item,\
										npd_arp_snooping_filter_by_mac,npd_arp_snooping_del);
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",arpCount);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
    else if(0x3 == flag)
    {		
    	arpCount = npd_arp_snooping_del_by_ifindex(eth_g_index);
		
		syslog_ax_arpsnooping_dbg("arp delete by ip and mask,delete %d items \n",arpCount);
		ret = ARP_RETURN_CODE_SUCCESS;
    }
	else if(0x4 == flag)
	{
		npd_arp_snooping_del_by_network(ipAddr,mask);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	else if(0xf == flag)
	{
    	arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index,kern_del_flag,NULL,NULL,npd_arp_snooping_del);
		ret = ARP_RETURN_CODE_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	return reply;


}


/*************************************************************
 *
 * OUTPUT:
 *		showStr : String - the result of static arp running-config 
 *					e.g. "ip static-arp 1/1 00:00:00:00:00:01 192.168.0.2/32 1\n"
 *
 *************************************************************/
DBusMessage *npd_dbus_show_ip_arp_info(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int j = 0;
	unsigned char flag = 0;
	unsigned int ifIndex = 0;	
	unsigned int ipmask[2]= {0,0};
	unsigned char mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	unsigned char isStatic = 0, isValid = 0;
	unsigned int ret = NPD_DBUS_SUCCESS;
	struct arp_snooping_item_s dbItem;
	unsigned int arpFlag = 0;
	unsigned int arp_ifIndex = 0;
	unsigned int arp_ipAddr = 0;
	unsigned char arp_mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	unsigned int arpCount = 0;
    unsigned int arpTime = 0;
	DBusError err;
	
	memset(&dbItem,0, sizeof(struct arp_snooping_item_s));
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_BYTE, &flag,
				DBUS_TYPE_UINT32,&ifIndex,
				DBUS_TYPE_UINT32, &(ipmask[0]),
				DBUS_TYPE_UINT32, &(ipmask[1]),
				DBUS_TYPE_BYTE, &mac[0],
				DBUS_TYPE_BYTE, &mac[1],
				DBUS_TYPE_BYTE, &mac[2],
				DBUS_TYPE_BYTE, &mac[3],
				DBUS_TYPE_BYTE, &mac[4],
				DBUS_TYPE_BYTE, &mac[5],
				DBUS_TYPE_UINT32,&arp_ifIndex,
				DBUS_TYPE_UINT32,&arp_ipAddr,
				DBUS_TYPE_BYTE, &arp_mac[0],
				DBUS_TYPE_BYTE, &arp_mac[1],
				DBUS_TYPE_BYTE, &arp_mac[2],
				DBUS_TYPE_BYTE, &arp_mac[3],
				DBUS_TYPE_BYTE, &arp_mac[4],
				DBUS_TYPE_BYTE, &arp_mac[5],
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	if( 1 == flag )
	{
		if(0 == arp_ipAddr)
		{
			dbItem.ipAddr = ipmask[0];
			arpCount = dbtable_hash_traversal_key(npd_arpsnp_haship_index, TRUE, &dbItem, npd_arp_snooping_compare_byip, NULL);
			ret = dbtable_hash_head_key(npd_arpsnp_haship_index, &dbItem, &dbItem, npd_arp_snooping_compare_byip);
		}
		else
		{
			dbItem.ifIndex = arp_ifIndex;
			dbItem.ipAddr = arp_ipAddr;
			memcpy(dbItem.mac,arp_mac,6);
			ret = dbtable_hash_next_key(npd_arpsnp_haship_index, &dbItem, &dbItem, npd_arp_snooping_compare_byip);
		}
	}
	else if( 2 == flag )
	{
		if(0 == arp_ipAddr)
		{
			memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
			arpCount = dbtable_hash_traversal_key(npd_arpsnp_hashmac_index, TRUE, &dbItem, npd_arp_snooping_filter_by_mac, NULL);
			ret = dbtable_hash_head_key(npd_arpsnp_hashmac_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_mac);
		}
		else
		{
			dbItem.ifIndex = arp_ifIndex;
			dbItem.ipAddr = arp_ipAddr;
			memcpy(dbItem.mac,arp_mac,6);
			ret = dbtable_hash_next_key(npd_arpsnp_hashmac_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_mac);
		}
	}
	else if( 3 == flag )
	{
		dbItem.ifIndex = ifIndex;
		if(0 == arp_ifIndex)
		{
			dbItem.ifIndex = ifIndex;
			arpCount = dbtable_hash_traversal_key(npd_arpsnp_hashport_index, TRUE, &dbItem, npd_arp_snooping_filter_by_ifindex, NULL);
			ret = dbtable_hash_head_key(npd_arpsnp_hashport_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_ifindex);
		}
		else
		{
			dbItem.ifIndex = arp_ifIndex;
			dbItem.ipAddr = arp_ipAddr;
			memcpy(dbItem.mac,arp_mac,6);
			ret = dbtable_hash_next_key(npd_arpsnp_hashport_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_ifindex);
		}
	}
	else if( 4 == flag )
	{
		if(0 == arp_ipAddr)
		{
			arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index, TRUE, ipmask, npd_arp_snooping_filter_by_network, NULL);
			ret = dbtable_hash_head(npd_arpsnp_haship_index, NULL, &dbItem, NULL);
			while(0 == ret )
			{
				if(TRUE == npd_arp_snooping_filter_by_network(&dbItem,ipmask))
					break;
				ret = dbtable_hash_next(npd_arpsnp_haship_index, &dbItem, &dbItem, NULL);
			}
		}
		else
		{
			dbItem.ifIndex = arp_ifIndex;
			dbItem.ipAddr = arp_ipAddr;
			memcpy(dbItem.mac,arp_mac,6);
			ret = dbtable_hash_next(npd_arpsnp_haship_index, &dbItem, &dbItem, NULL);
			while(0 == ret )
			{
				if(TRUE == npd_arp_snooping_filter_by_network(&dbItem,ipmask))
					break;
				ret = dbtable_hash_next(npd_arpsnp_haship_index, &dbItem, &dbItem, NULL);
			}
		}
	}
	else if( 0xf == flag )
	{
		if(0 == arp_ipAddr)
		{
			arpCount = dbtable_hash_traversal(npd_arpsnp_haship_index, TRUE, NULL, NULL, NULL);
			ret = dbtable_hash_head(npd_arpsnp_haship_index, NULL, &dbItem, NULL);
		}
		else
		{
			dbItem.ifIndex = arp_ifIndex;
			dbItem.ipAddr = arp_ipAddr;
			memcpy(dbItem.mac,arp_mac,6);
			ret = dbtable_hash_next(npd_arpsnp_haship_index, &dbItem, &dbItem, NULL);
		}
	}

	
	dbus_message_iter_append_basic(&iter,
							DBUS_TYPE_UINT32,
									&ret);
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32,
								 &arpCount);
	if(NPD_DBUS_SUCCESS == ret)
	{
        struct npd_arpsnp_cfg_s cfgItem;

        ret = dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);

		ipmask[0] = dbItem.ipAddr;
		memcpy(mac,dbItem.mac,6);
		ifIndex = dbItem.ifIndex;
		isStatic = dbItem.isStatic;
		isValid = dbItem.isValid;
		arpFlag = dbItem.flag;
        if(isStatic)
            arpTime = 0;
        else
            arpTime = cfgItem.timeout - (arp_absolute_time - dbItem.time);
		
		dbus_message_iter_append_basic (&iter,
								DBUS_TYPE_UINT32,
									 &ipmask[0]);
			
			for (j = 0; j < 6; j++ ) {
				
				dbus_message_iter_append_basic(&iter,
								  		DBUS_TYPE_BYTE,
								  		&(mac[j]));  /*mac*/
			}
			
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
										&(arpFlag)); /*flag*/
			
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
										&(ifIndex)); /*ifIndex*/

		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_BYTE,
									  &(isStatic)); /*isStatic*/
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_BYTE,
									&(isValid)); /*isStatic*/
        dbus_message_iter_append_basic(&iter,
                                DBUS_TYPE_UINT32,
                                &arpTime);
	}
	
	return reply;
}



/*************************************************************
 *
 * OUTPUT:
 *		showStr : String - the result of static arp running-config 
 *					e.g. "ip static-arp 1/1 00:00:00:00:00:01 192.168.0.2/32 1\n"
 *
 *************************************************************/
DBusMessage *npd_dbus_ip_dyntostatic_arp(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned char flag = 0;
	unsigned int ifIndex = 0;
	unsigned int arpCount = 0;
	unsigned int ipmask[2]= {0};
	unsigned char mac[6] = {0};
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int status = 0;
	struct arp_snooping_item_s dbItem;
	DBusError err;
	
	memset(&dbItem,0, sizeof(struct arp_snooping_item_s));

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_BYTE, &flag,
				DBUS_TYPE_UINT32,&ifIndex,
				DBUS_TYPE_UINT32, &(ipmask[0]),
				DBUS_TYPE_UINT32, &(ipmask[1]),
				DBUS_TYPE_BYTE, &mac[0],
				DBUS_TYPE_BYTE, &mac[1],
				DBUS_TYPE_BYTE, &mac[2],
				DBUS_TYPE_BYTE, &mac[3],
				DBUS_TYPE_BYTE, &mac[4],
				DBUS_TYPE_BYTE, &mac[5],				
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_eth_port_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	if( 1 == flag )
	{		
		status = npd_arp_snooping_find_item_byip(ipmask[0], &dbItem);
	    if(0 != status)
	    {
		    ret = NPD_DBUS_ERROR;
	    }
		else
		{
			ret = npd_arp_snooping_dyntostatic(npd_arpsnp_haship_index, &dbItem, TRUE);
		}
    }
	else if( 2 == flag )
	{
		memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
		arpCount = dbtable_hash_traversal( npd_arpsnp_hashmac_index, TRUE, &dbItem,\
													npd_arp_snooping_filter_by_mac, npd_arp_snooping_dyntostatic);
		if(0 == arpCount) 
		{
			ret = NPD_DBUS_ERROR;
		}	
	}
	else if( 3 == flag )
	{	
	    dbItem.ifIndex = ifIndex;
		arpCount = dbtable_hash_traversal( npd_arpsnp_hashport_index, TRUE, &dbItem,\
								                   npd_arp_snooping_filter_by_ifindex, npd_arp_snooping_dyntostatic);
		if(0 == arpCount)
		{
			ret = NPD_DBUS_ERROR;
		}	
	}
	else if( 4 == flag )
	{
		arpCount = dbtable_hash_traversal( npd_arpsnp_haship_index, TRUE, ipmask,\
													npd_arp_snooping_filter_by_network, npd_arp_snooping_dyntostatic);
		if(0 == arpCount) 
		{
			ret = NPD_DBUS_ERROR;
		}		
	}
	else if( 0xf == flag )
	{
		arpCount = dbtable_hash_traversal( npd_arpsnp_haship_index, TRUE, NULL, NULL, 
			                                             npd_arp_snooping_dyntostatic);
		if(0 == arpCount) 
		{
			ret = NPD_DBUS_ERROR;
		}	
	}
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	return reply;
}

DBusMessage *npd_dbus_ip_set_arp_agetime(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int flag = 0;
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int timeout = 0;
	struct npd_arpsnp_cfg_s cfgItem;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_UINT32, &timeout,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	ret = dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);
	if(0 == ret)
	{
		if(1 == flag)
		{
			cfgItem.timeout = timeout;
		}
		else
		{
			cfgItem.timeout = NPD_ARP_AGE_AGE_CNT;
		}
		ret = dbtable_array_update(npd_arpsnp_cfg_index, 0, &cfgItem, &cfgItem);
		if(0 != ret)
		{
			syslog_ax_arpsnooping_err("Update cfg failed!");
			ret = NPD_DBUS_ERROR;
		}
	}
	else{
		syslog_ax_arpsnooping_err("Get cfg failed!");
		ret = NPD_DBUS_ERROR;
	}
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	return reply;
}


/*************************************************************
 *
 * OUTPUT:
 *		showStr : 
 *
 *************************************************************/
DBusMessage *npd_dbus_ip_show_arp_agetime(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int timeout = 0;
	struct npd_arpsnp_cfg_s cfgItem;
	DBusError err;
	
	dbus_error_init(&err);
	
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	ret = dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);
	if(0 == ret)
	{
		timeout = cfgItem.timeout;
	}
	else{
		syslog_ax_arpsnooping_err("Get cfg failed!");
		ret = NPD_DBUS_ERROR;
	}
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&timeout);

	return reply;
}

DBusMessage *npd_dbus_ip_set_arp_drop(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int flag = 0;
	unsigned int ret = NPD_DBUS_SUCCESS;
	struct npd_arpsnp_cfg_s cfgItem;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_arpsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	ret = dbtable_array_get(npd_arpsnp_cfg_index, 0, &cfgItem);
	if(0 == ret)
	{
	    cfgItem.arp_drop_enable = flag;
		
		ret = dbtable_array_update(npd_arpsnp_cfg_index, 0, &cfgItem, &cfgItem);
		if(0 != ret)
		{
			syslog_ax_arpsnooping_err("Update cfg failed!");
			ret = NPD_DBUS_ERROR;
		}
	}
	else
	{
		syslog_ax_arpsnooping_err("Get cfg failed!");
		ret = NPD_DBUS_ERROR;
	}
	
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	return reply;
}


/*************************************************************
 *
 * OUTPUT:
 *		showStr : 
 *
 *************************************************************/
DBusMessage *npd_dbus_ip_show_arp_drop(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int drop_flag = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	drop_flag = arp_drop_enable;
	
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&drop_flag);

	return reply;
}

/*************************************************************
 *
 * OUTPUT:
 *		showStr : String - the result of static arp running-config 
 *					e.g. "ip static-arp 1/1 00:00:00:00:00:01 192.168.0.2/32 1\n"
 *
 *************************************************************/
DBusMessage *npd_dbus_static_arp_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	struct arp_snooping_item_s dbItem;
	struct npd_arpsnp_cfg_s cfgItem;
	int len = 0, ret = 0;
	
	npd_arp_showStr = (char*)malloc(NPD_INTF_SAVE_STATIC_ARP_MEM);
	if(NULL == npd_arp_showStr) {
		return NULL;
	}
	memset(npd_arp_showStr,0,NPD_INTF_SAVE_STATIC_ARP_MEM);
	npd_arp_showStr_len = 0;
	
	memset(&dbItem, 0, sizeof(struct arp_snooping_item_s));
	dbItem.isStatic = 1;
	
	syslog_ax_arpsnooping_dbg("arp global show running-config\n");
	dbtable_array_get(npd_arpsnp_cfg_index,0,&cfgItem);
	if( NPD_ARP_AGE_AGE_CNT != cfgItem.timeout)
	{
		len = sprintf(npd_arp_showStr,"ip arp timeout %u\n", cfgItem.timeout);
		npd_arp_showStr_len += len;
	}
	if(cfgItem.arp_drop_enable)
	{
		len = sprintf(npd_arp_showStr + npd_arp_showStr_len, "ip arp drop-unreachable\n");
		npd_arp_showStr_len += len;
	}

	syslog_ax_arpsnooping_dbg("static arp show running-config\n");

	ret = dbtable_hash_head(npd_arpsnp_haship_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_static);
	while(0 == ret)
	{
		npd_dbus_save_static_arp(npd_arpsnp_haship_index, &dbItem, TRUE);

		ret = dbtable_hash_next(npd_arpsnp_haship_index, &dbItem, &dbItem, npd_arp_snooping_filter_by_static);
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
                                     &npd_arp_showStr); 
								 
	free(npd_arp_showStr);
	npd_arp_showStr = NULL;
	npd_arp_showStr_len = 0;
	
	return reply;
}

DBusMessage * npd_dbus_arp_inspection_clear_statistics(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*        reply;
	DBusMessageIter     iter;
    unsigned int        ret = ARP_RETURN_CODE_SUCCESS;

    npd_arp_inspection_clear_statistics();

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
                                     &ret); 
    
 	return reply;
}

DBusMessage * npd_dbus_arp_inspection_satistics(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*        reply;

    int ni = 0;
    unsigned int npd_ais_valid[MAX_SWITCHPORT_PER_SYSTEM];
    unsigned int npd_ais_index[MAX_SWITCHPORT_PER_SYSTEM];
    unsigned int npd_ais_permit[MAX_SWITCHPORT_PER_SYSTEM];
    unsigned int npd_ais_drop[MAX_SWITCHPORT_PER_SYSTEM];
    unsigned int* ptr_npd_ais_valid = npd_ais_valid;
    unsigned int* ptr_npd_ais_index = npd_ais_index;
    unsigned int* ptr_npd_ais_permit = npd_ais_permit;
    unsigned int* ptr_npd_ais_drop = npd_ais_drop;
    
    memset(npd_ais_permit, 0, sizeof(npd_ais_permit));
    memset(npd_ais_drop, 0, sizeof(npd_ais_drop));

    for (ni = 0; ni < MAX_SWITCHPORT_PER_SYSTEM; ni++)
    {
        npd_ais_valid[ni] = npd_arp_inspection_statistics[ni].is_valid;
        npd_ais_index[ni] = npd_arp_inspection_statistics[ni].eth_g_index;
        npd_ais_permit[ni] = npd_arp_inspection_statistics[ni].permit;
        ptr_npd_ais_drop[ni] = npd_arp_inspection_statistics[ni].drop;
    }
    
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
	{
		npd_syslog_err("Npd ARP inspection new method faild\n");
		return reply;
	}

    dbus_message_append_args(reply,
                            DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_npd_ais_valid,
							MAX_SWITCHPORT_PER_SYSTEM,

                            DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_npd_ais_index,
							MAX_SWITCHPORT_PER_SYSTEM,
							 
							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_npd_ais_permit,
							MAX_SWITCHPORT_PER_SYSTEM,

                            DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_npd_ais_drop,
							MAX_SWITCHPORT_PER_SYSTEM, 

							DBUS_TYPE_INVALID);
    
 	return reply;
}

DBusMessage *npd_dbus_arp_inspection_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	char *buffer = NULL;
	unsigned int buffsize = 4096;
	buffer = (char*)malloc(4096);
	if(NULL == buffer) {
		return NULL;
	}
	memset(buffer,0,buffsize);
	syslog_ax_arpsnooping_dbg("arp inspection show running-config\n");

	npd_dbus_save_arp_inspection_cfg(&buffer,&buffsize);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
                                     &buffer); 
								 
	free(buffer);
	buffer = NULL;

	return reply;
}

#ifdef __cplusplus
}
#endif

