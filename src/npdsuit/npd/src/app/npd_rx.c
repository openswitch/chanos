/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*npd_rx.c
*
*
*CREATOR:
*	zhengzw@autelan.com
*
*DESCRIPTION:
*<some description about this file>
*      In this file The functions prepare for packet Rx from Adapter to 
*      The Virtual Network Interface  .
*
*DATE:
*	11/14/2007
*04/26/2010              zhengzw@autelan.com          Unifying netif index formate with vlan and port-channel
*06/11/2010              zhengzw@autelan.com          L3 interface supported.
*06 /23/2010             zhengzw@autelan.com          Re-coding the packet rx flow. Make it easy to support new protocols.
*08/09/2010              zhengzw@autelan.com          Distribute rx packet processing to any online slot.(RX_TO_MCU,RX_TO_ALL,RX_TO_LOCAL...)
*08/10/2010              zhengzw@autelan.com          LLDP packet supported.
*	
*******************************************************************************/
#ifdef __cplusplus
extern "C" 
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "npd_main.h"
/*include header files end */


#define NAM_PACKET_SOCK_TX_PREFIX "/tmp/packet.tx."
#define NAM_PACKET_SOCK_RX_PREFIX "/tmp/packet.rx."
unsigned char * namPacketTypeStr[] = {
	(unsigned char *)"BPDU",	/* NAM_PACKET_TYPE_BPDU_E = 0 */
	(unsigned char *)"GVRP",	/* NAM_PACKET_TYPE_GVRP_E */
	(unsigned char *)"ARP",	/* NAM_PACKET_TYPE_ARP_E */	
	(unsigned char *)"ARP_REPLY",
	(unsigned char *)"IPv4",	/* NAM_PACKET_TYPE_IPv4_E */
	(unsigned char *)"ICMP",	/* NAM_PACKET_TYPE_IP_ICMP_E */		
	(unsigned char *)"TCP",	/* NAM_PACKET_TYPE_IP_TCP_E */		
	(unsigned char *)"UDP",	/* NAM_PACKET_TYPE_IP_UDP_E */		
	(unsigned char *)"IGMP",	/* NAM_PACKET_TYPE_IP_IGMP_E */	 
	(unsigned char *)"PIM",  /*NAM_PACKET_TYPE_IP_PIM_E*/
	(unsigned char *)"TELNET", /* NAM_PACKET_TYPE_IP_TCP_TELNET_E */	
	(unsigned char *)"SSH",	/* NAM_PACKET_TYPE_IP_TCP_SSH_E */	
	(unsigned char *)"FTP",	/* NAM_PACKET_TYPE_IP_TCP_FTP_E */
	(unsigned char *)"FTP-DA", /* NAM_PACKET_TYPE_IP_TCP_FTP_DATA_E */
	(unsigned char *)"DHCP", /* NAM_PACKET_TYPE_IP_UDP_DHCP_E*/
	(unsigned char *)"IPIP", /*NAM_PACKET_TYPE_IP_IPIP_E*/
	(unsigned char *)"DLDP",	/* NAM_PACKET_TYPE_DLDP_E */	
	(unsigned char *)"VRRP", /*NAM_PACKET_TYPE_VRRP_E*/
	(unsigned char *)"EAPOL", /*NAM_PACKET_TYPE_EAP_E*/
	(unsigned char *)"LLDP", /*NAM_PACKET_TYPE_LLDP_E*/
	(unsigned char *)"LACP", /*NAM_PACKET_TYPE_LACP_E*/
	(unsigned char *)"NDP",  /*NAM_PACKET_TYPE_NDP_E*/
	(unsigned char *)"IPv6", /*NAM_PACKET_TYPE_IPv6_E*/
	(unsigned char *)"UDLD",
	(unsigned char *)"IPMC",
	(unsigned char *)"IPv6UDP",
	(unsigned char *)"ICMP6",  /*NAM_PACKET_TYPE_ICMP6_E*/
	(unsigned char *)"MLD",  /*NAM_PACKET_TYPE_MLD_E*/
	(unsigned char *)"STLK",  /*NAM_PACKET_TYPE_STLK_E*/
	(unsigned char *)"ERPP",  /*NAM_PACKET_TYPE_ERPP_E*/
	(unsigned char *)"SFLOW",  /*NAM_PACKET_TYPE_SFLOW_E*/
	(unsigned char *)"UNKOWN"	/* NAM_PACKET_TYPE_OTHER */		
};

pthread_mutex_t npd_rx_handle_lock = PTHREAD_MUTEX_INITIALIZER;
struct list_head protocol_handle_head =
{
    &protocol_handle_head,
    &protocol_handle_head
};
int local_socket[NAM_PACKET_TYPE_OTHER] = {0};

/**************************************************/
#define  NPD_NETIF_ATTACK_DEFAULT_INTERVAL    2 /*seconds*/
#define  NPD_NETIF_ATTACK_PKTS_NUM             2000

#define  NPD_NETIF_ATTACK_DB_SIZE    2048


npd_netif_attack_t local_netif_attack[2][128];

extern int  nam_port_cpu_block_set(unsigned char unit, unsigned char portnum);
extern int  nam_port_cpu_block_restore(unsigned char unit, unsigned char port, npd_netif_attack_t  *map);
#ifdef HAVE_UDLD
int npd_packet_type_is_udld
(       
	unsigned char  *packetBuff
);
#endif

#ifdef HAVE_NPD_IPV6
extern int npd_ndisc_packet_rx_process
(
	unsigned int   packet_type,
	unsigned int   netif_index,
	unsigned short vid,
	unsigned char  isTagged,
	unsigned char  *packet,
	unsigned long  length
);
#endif

extern int npd_system_get_basemac
(
    unsigned char *macAddr,
    unsigned int  size
);
int npd_packet_netif_attack_handling(unsigned int netif_index)
{
    unsigned char unit, port;
    int ret;
    npd_netif_attack_t *attack;
    return 0;
    ret = npd_get_devport_by_global_index(netif_index, &unit, &port);
    if(NPD_SUCCESS != ret)
        return -1;

    attack = &local_netif_attack[unit][port];
    attack->stat_packets++;
    npd_syslog_pkt_rev("Netif %x(%d,%d) attack count %u\n",
                      netif_index, unit, port, attack->stat_packets);
    return 0;
}

int npd_packet_netif_type_stats_increase(unsigned int netif_index, unsigned int packet_type)
{
    unsigned char unit, port;
    int ret;
    npd_netif_attack_t *attack;

    ret = npd_get_devport_by_global_index(netif_index, &unit, &port);
    if(NPD_SUCCESS != ret)
        return -1;
    if(packet_type >= NAM_PACKET_TYPE_OTHER)
    {
		return -1;
    }
    attack = &local_netif_attack[unit][port];
    attack->rx_type_stats[packet_type]++;
    return 0;
}

int npd_packet_netif_type_stats_clear(unsigned int netif_index)
{
    unsigned char unit, port;
    int ret = 0, i = 0;
    npd_netif_attack_t *attack;

    ret = npd_get_devport_by_global_index(netif_index, &unit, &port);
    if(NPD_SUCCESS != ret)
        return -1;
	if(!SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
	{
	    return -1;
    }
    attack = &local_netif_attack[unit][port];
	for(i = 0; i < NAM_PACKET_TYPE_OTHER; i++)
	{
        attack->rx_type_stats[i] = 0;
        attack->rx_type_stats_last[i] = 0;
	}
    return 0;
}

int npd_packet_netif_type_stats_get(unsigned int netif_index, unsigned int *rx_type_stats, unsigned int *rx_type_pps)
{
    unsigned char unit, port;
    int ret = 0, i = 0;
    npd_netif_attack_t *attack;

    ret = npd_get_devport_by_global_index(netif_index, &unit, &port);
    if(NPD_SUCCESS != ret)
        return -1;
	if(!SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
	{
	    return -1;
    }
    attack = &local_netif_attack[unit][port];
	for(i = 0; i < NAM_PACKET_TYPE_OTHER; i++)
	{
        rx_type_stats[i] = attack->rx_type_stats[i];
        rx_type_pps[i] = attack->rx_type_pps[i];
	}
    return 0;
}

int npd_netif_attack_init()
{
    while(0 == npd_startup_end)
    {
        sleep(1);
    }
    memset(local_netif_attack, 0, sizeof(local_netif_attack));
    return 0;
}

int npd_netif_attack_timer()
{
    int unit, port, unit_no;
    int i = 0;
    unit_no = nam_asic_get_instance_num();
    for(unit= 0; unit < unit_no; unit++)
    {
        for(port = 0; port < 128; port++)
        {
            npd_netif_attack_t *attack = &local_netif_attack[unit][port];
#ifdef HAVE_ANTI_ATTACK
            {
                if(attack->stat_packets > NPD_NETIF_ATTACK_PKTS_NUM)
                {
                    nam_port_cpu_block_set(unit, port);
                    attack->netif_deny2cpu = NPD_TRUE;
                    attack->netif_denytimes++;
                    attack->netif_holddeny_times = attack->netif_denytimes;
                }
                else if(NPD_TRUE == attack->netif_deny2cpu)
                {
                    if(0 >= attack->netif_holddeny_times)
                    {
                        /*用pri 1作为正常流量, 0 用来设置当端口进行攻击时的pri*/
                        nam_port_cpu_block_restore(unit, port, attack);
                        attack->netif_deny2cpu = NPD_FALSE;
                    }
                    else
                    {
                        attack->netif_holddeny_times--;
                    }
                }
                else if(0 < attack->netif_denytimes)
                {
                    attack->netif_denytimes--;
                }
                attack->stat_packets = 0;
            }
#endif
			for(i = 0; i < NAM_PACKET_TYPE_OTHER; i++)
			{
				attack->rx_type_pps[i] = attack->rx_type_stats[i] - attack->rx_type_stats_last[i];
			    attack->rx_type_stats_last[i] = attack->rx_type_stats[i];
			}
			/*usleep(100);*/
        }
    }
    return 0;
}


#define  ADPT_THREAD_PRIO 	200
pthread_mutex_t semKapIoMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semRtRwMutex = PTHREAD_MUTEX_INITIALIZER;

int       adptVirRxFd = -1;
protocol_handle *npd_packet_handle_search(struct list_head *current_list, int type);



int npd_packet_type_is_BPDU
(       
    unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;

	if((macByte[0]==0x01)&&(macByte[1]==0x80)
		&& (macByte[2]==0xc2) && (macByte[3]==0x00)
		&& (macByte[4]==0x00) && (macByte[5]==0x00)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}
int npd_packet_type_is_EAP
(       
    unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;

	if( layer2->etherType == htons(0x888E)){
		return NPD_TRUE;
	}
#ifdef HAVE_CAPWAP_ENGINE
{
	unsigned char	*pos = NULL;
	unsigned char	qos_eap_feature[]={0xaa,0xaa,0x03,0x00,0x00,0x00,0x88,0x8e};

	/*wireless eapol key with qos enable*/
	pos =(unsigned char*)(layer2+1);
	pos = pos+2;
	if(memcmp(qos_eap_feature, pos, sizeof(qos_eap_feature)) == 0){
		return NPD_TRUE;
	}
}
#endif	
	return NPD_FALSE;
}
int npd_packet_type_is_ARP
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;

	layer2 = (ether_header_t*)(packetBuff);

	if(htons(0x0806) == layer2->etherType) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}

int npd_packet_type_is_ARP_REPLY
(
    unsigned char *packetBuff
)
{
    ether_header_t *layer2 = NULL;
    arp_header_t *arp = NULL;
    
    layer2 = (ether_header_t*)(packetBuff);
    arp = (arp_header_t*)(layer2+1);

    if( (htons(0x0806) == layer2->etherType)
        &&(htons(0x0002) == arp->opCode)
        )
    {
        return NPD_TRUE;
    }
    return NPD_FALSE;
}

int npd_packet_type_is_ipmc_data
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
    
	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version) &&
		(0xE00000FF < ntohl(*(int *)(layer3->dip))) &&
		(0xEFFFFFFF > ntohl(*(int *)(layer3->dip)))) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}


int npd_packet_type_is_IPv4
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}


#ifdef HAVE_NPD_IPV6

/*
 *	NextHeader field of IPv6 header
 */

int
ip6t_ext_hdr(unsigned char nexthdr)
{
	return ( (nexthdr == IPPROTO_HOPOPTS)   ||
		 (nexthdr == IPPROTO_ROUTING)   ||
		 (nexthdr == IPPROTO_FRAGMENT)  ||
		 (nexthdr == IPPROTO_ESP)       ||
		 (nexthdr == IPPROTO_AH)        ||
		 (nexthdr == IPPROTO_NONE)      ||
		 (nexthdr == IPPROTO_DSTOPTS) );
}


int npd_packet_type_is_IPv6
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	struct ip6_hdr 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (struct ip6_hdr *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if(htons(0x86dd) != layer2->etherType){
		return NPD_FALSE;
	}
	if(layer3->ip6_nxt){
		return NPD_TRUE;
	}
	return NPD_FALSE;
}

#if HAVE_DHCPV6_RELAY
int npd_packet_type_is_IPv6_udp_dhcpv6
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
    struct ip6_hdr 	*layer3 = NULL;
    udp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (struct ip6_hdr *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if (htons(0x86dd) != layer2->etherType)
	{
		return NPD_FALSE;
	}
    
	if (0x11 != layer3->ip6_nxt)
	{
        return NPD_FALSE;
	}
    layer4 = (udp_header_t*)((unsigned int *)layer3 + (sizeof(struct ip6_hdr) / 4));

	if (((htons(547) == layer4->dest) && (htons(546) == layer4->source)) /* DHCPv6 request */
		|| ((htons(547) == layer4->source) && (htons(546) == layer4->dest)) /* DHCPv6 reply */
		|| ((htons(547) == layer4->source) && (htons(547) == layer4->dest))) /* DHCPv6 Relay */
	{ 
		return NPD_TRUE;
	}
    
	return NPD_FALSE;
}
#endif

int npd_packet_get_icmp6hdr
(       
	unsigned char  *packetBuff,
	struct icmp6_hdr **header
)
{
	ether_header_t  *layer2 = NULL;
	struct ip6_hdr 	*layer3 = NULL;
	struct icmp6_hdr *hdr;
	unsigned char *ptr = NULL;
	unsigned char nexthdr;
	unsigned short len;

	layer2 = (ether_header_t*)(packetBuff);
	if(htons(0x86dd) != layer2->etherType){
		return NPD_FALSE;
	}
	
	layer3 = (struct ip6_hdr *)(layer2 + 1);
	/* pointer to the 1st exthdr */
	nexthdr = layer3->ip6_nxt;
	/* available length */
	len = ntohs(layer3->ip6_plen);
	/*pointer to current exthdr*/
	ptr = (unsigned char*)(layer3 + 1);

	while (ip6t_ext_hdr(nexthdr)) {
		const struct ip6_ext *hp;
		unsigned short hdrlen;

		/* Is there enough space for the next ext header? */
		if (len < (int)sizeof(struct ip6_ext))
			return NPD_FALSE;
		/* No more exthdr -> evaluate */
		if (nexthdr == NEXTHDR_NONE) {
			return NPD_FALSE;
		}
		 /*ESP -> evaluate */
		if (nexthdr == NEXTHDR_ESP) {
			return NPD_FALSE;
		}

		hp = (struct ip6_ext *)ptr;
		
		/* Calculate the header length */
		if (nexthdr == NEXTHDR_FRAGMENT)
			hdrlen = 8;
		else if (nexthdr == NEXTHDR_AUTH)
			hdrlen = (hp->ip6e_len + 2) << 2;
		else
			hdrlen = (hp->ip6e_len + 1) << 3;
		len -= hdrlen;


		nexthdr = hp->ip6e_nxt;
		ptr = (unsigned char*)((unsigned char *)ptr+hdrlen);
		if(NULL == ptr){
			return NPD_FALSE;
		}
	}

	if(nexthdr != NEXTHDR_ICMP){
		return NPD_FALSE;
	}
	
	hdr = (struct icmp6_hdr *)ptr;
	if((NULL == hdr) || len < sizeof(*hdr)){
		return NPD_FALSE;
	}

    *header = hdr;
	return NPD_TRUE;
}



int npd_packet_type_is_icmp6
(       
	unsigned char  *packetBuff
)
{
	struct icmp6_hdr *hdr;
    return npd_packet_get_icmp6hdr(packetBuff, &hdr);
}


int npd_packet_type_is_ndisc
(       
	unsigned char  *packetBuff
)
{
	struct icmp6_hdr *hdr;
	unsigned char type;

    if (!npd_packet_get_icmp6hdr(packetBuff, &hdr)) {
        return NPD_FALSE;
    }
    
	type = hdr->icmp6_type;
	switch (type) {
    	case ND_ROUTER_SOLICIT:
    	case ND_ROUTER_ADVERT:
    	case ND_NEIGHBOR_SOLICIT:
    	case ND_NEIGHBOR_ADVERT:
    	case ND_REDIRECT:
    		return NPD_TRUE;
    	default:
    		return NPD_FALSE;
	}
	
}



int npd_packet_type_is_mld
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;
	struct icmp6_hdr *hdr;
	unsigned char type;

	layer2 = (ether_header_t*)(packetBuff);

	macByte = layer2->dmac;
	if((macByte[0]!=0x33)||(macByte[1]!=0x33)) {
        return NPD_FALSE;
    }

    if (!npd_packet_get_icmp6hdr(packetBuff, &hdr)) {
        return NPD_FALSE;
    }

	type = hdr->icmp6_type;
	if( type == 130 ||		//MLD Group Query
		type == 131 ||		//MLD Group Report
		type == 132 ||		//MLD Group Leave
		type == 143 )		//MLD version 2 Group Report
	{
		return NPD_TRUE;
	}

	return NPD_FALSE;	
}

#endif //HAVE_NPD_IPV6


int npd_packet_type_is_ICMP
(       
	unsigned char  *packetBuff
)
{
    ether_header_t  *layer2 = NULL;
	ip_header_t 	*ip = NULL;
    struct icmp_header_t   *icmp = NULL;

    layer2 = (ether_header_t*)(packetBuff);
	ip = (ip_header_t *)(layer2+1);
    icmp = (struct icmp_header_t*)(ip+1); 

	if((htons(0x0800) == layer2->etherType) 
        && (4 == ip->version) 
        && (1 == ip->ipProtocol)
        && (8 == icmp->type)
        ) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}


unsigned int npd_packet_type_is_TCP
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if((htons(0x0800) == layer2->etherType) && 
		 (4 == layer3->version) && 
			(0x6 == layer3->ipProtocol)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}


unsigned int npd_packet_type_is_Telnet
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	tcp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (tcp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);

	if((htons(0x0800) == layer2->etherType) && 
		 (4 == layer3->version) && 
		  	(0x6 == layer3->ipProtocol)) {
			if((htons(0x17) == layer4->dest) ||
				(htons(0x17) == layer4->source)){/* tcp sport or dport 23 */
				return NPD_TRUE;
			}
	}

	return NPD_FALSE;
}


unsigned int npd_packet_type_is_Ssh
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	tcp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (tcp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);

	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version) && 
		(0x6 == layer3->ipProtocol)) {
		if((htons(0x16) == layer4->dest)||
			(htons(0x16) == layer4->source)){/* tcp sport or dport 22 */
			return NPD_TRUE;
		}
	}

	return NPD_FALSE;
}


unsigned int npd_packet_type_is_Ftp
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	tcp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (tcp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);

	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version) && 
		(0x6 == layer3->ipProtocol)) {
		if((htons(0x15) == layer4->dest)||
			(htons(0x15) == layer4->source)){/* tcp sport or dport 21 */
			return NPD_TRUE;
		}
	}

	return NPD_FALSE;
}


unsigned int npd_packet_type_is_Ftp_Data
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	tcp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (tcp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);

	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version) && 
		(0x6 == layer3->ipProtocol)) {
		if((htons(0x14) == layer4->dest)||
			(htons(0x14) == layer4->source)){/* tcp sport or dport 20 */
			return NPD_TRUE;
		}
	}

	return NPD_FALSE;
}

unsigned int npd_packet_type_is_UDP
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if((htons(0x0800) == layer2->etherType) && 
		 (4 == layer3->version) && 
		 	(0x11 == layer3->ipProtocol)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}



unsigned int npd_packet_type_is_IPIP
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	if((htons(0x0800) == layer2->etherType) && 
		 (4 == layer3->version) && 
		 	(0x04 == layer3->ipProtocol)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}



int npd_packet_type_is_Dhcp
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	udp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (udp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);
	

	if ((htons(0x0800) == layer2->etherType) && 
			(4 == layer3->version) &&
				(0x11 == layer3->ipProtocol)) 
	{
		if ((htons(67) == layer4->dest)||(htons(68) == layer4->dest)) /* DHCP request */
		{ 
			return NPD_TRUE;
		}
	}
	return NPD_FALSE;
	
}

int npd_packet_type_is_IGMP
(       
	unsigned char  *packetBuff
)
{
	unsigned char ret = NPD_FALSE;
	ether_header_t  *layer2 = NULL;
	ip_header_t		*layer3 = NULL;
	unsigned char 	*macByte = NULL;
	unsigned int	protcl;
	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	macByte = layer2->dmac;
	protcl = layer3->ipProtocol;
	if((macByte[0]==0x01)&&(macByte[1]==0x00)&&(macByte[2]==0x5e)) {
		if(!npd_packet_type_is_IPv4((unsigned char*)packetBuff)) {
			ret = NPD_FALSE;
		}
		else if(2 == protcl) { /*ip protocol type IGMP*/
			ret = NPD_TRUE;
		}
		else {
			ret = NPD_FALSE;
		}
	}
	else {
		ret = NPD_FALSE;
	}

	return ret;
}

int npd_packet_type_is_PIM
(       
	unsigned char  *packetBuff
)
{
	unsigned char ret = NPD_FALSE;
	ether_header_t  *layer2 = NULL;
	ip_header_t		*layer3 = NULL;
	unsigned char 	*macByte = NULL;
	unsigned int	protcl;
	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	macByte = layer2->dmac;
	protcl = layer3->ipProtocol;

	if(!npd_packet_type_is_IPv4((unsigned char*)packetBuff)) {
		ret = NPD_FALSE;
	}
	else if(103 == protcl) { /*ip protocol type PIM*/
		ret = NPD_TRUE;
	}
	else {
		ret = NPD_FALSE;
	}

	return ret;
}


int npd_packet_type_is_DLDP
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;

	layer2 = (ether_header_t*)(packetBuff);

	if (htons(0x9003) == layer2->etherType) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}



unsigned int npd_packet_type_is_VRRP
(       
	unsigned char  *packetBuff
)
{
	unsigned char ret = NPD_FALSE;
	ether_header_t  *layer2 = NULL;
	ip_header_t		*layer3 = NULL;
	unsigned char 	*macByte = NULL;
	unsigned int	protcl;
	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));

	macByte = layer2->dmac;
	protcl = layer3->ipProtocol;
	if((htons(0x0800) == layer2->etherType) && 
		(4 == layer3->version)) {
		if((macByte[0]==0x01)&&(macByte[1]==0x00)&&(macByte[2]==0x5e)) {
	       if(IPPROTO_VRRP == protcl){
	          ret = NPD_TRUE;
		   }
	    }
	}

	return ret;

}

int npd_packet_type_is_lldp
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;

	if( layer2->etherType != htons(0x88cc))
		return NPD_FALSE;
	
	if((macByte[0]==0x01)&&(macByte[1]==0x80)&& (macByte[2]==0xc2)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}



int npd_packet_type_is_lacp
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;

	if( layer2->etherType != htons(0x8809))
		return NPD_FALSE;
	
	if((macByte[0]==0x01)&&(macByte[1]==0x80)&& (macByte[2]==0xc2)) {
		return NPD_TRUE;
	}
	else {
		return NPD_FALSE;
	}
}


enum NAM_PACKET_TYPE_ENT nam_packet_parse_txtype 
(
	unsigned char  *packet
)
{
	enum NAM_PACKET_TYPE_ENT packetType = NAM_PACKET_TYPE_OTHER;

	if(npd_packet_type_is_ARP(packet)) {
		packetType = NAM_PACKET_TYPE_ARP_E;
	}
	else if(npd_packet_type_is_ipmc_data(packet))
	{
		packetType = NAM_PACKET_TYPE_IPMC_DATA_E;
	}
	else if(npd_packet_type_is_IPv4(packet)) {
		packetType = NAM_PACKET_TYPE_IPv4_E;
	}
#ifdef HAVE_NPD_IPV6	
	else if(npd_packet_type_is_IPv6(packet)) {
		packetType = NAM_PACKET_TYPE_IPv6_E;
	}
#endif //HAVE_NPD_IPV6	
    else if(npd_packet_type_is_BPDU(packet)) {
		packetType = NAM_PACKET_TYPE_BPDU_E;
	}
	if(npd_packet_type_is_EAP(packet)) {
		packetType = NAM_PACKET_TYPE_EAP_E;
	}
	else if(npd_packet_type_is_IGMP(packet)) {
		packetType = NAM_PACKET_TYPE_IP_IGMP_E;
	}
	else if(npd_packet_type_is_DLDP(packet)) {
		packetType = NAM_PACKET_TYPE_DLDP_E;
	}
	else if(npd_packet_type_is_lldp(packet)) {
		packetType = NAM_PACKET_TYPE_LLDP_E;
	}
	else if(npd_packet_type_is_lacp(packet)) {
		packetType = NAM_PACKET_TYPE_LACP_E;
	}
#ifdef HAVE_UDLD
	else if(npd_packet_type_is_udld(packet)) {
		packetType = NAM_PACKET_TYPE_UDLD_E;
	}
#endif
#ifdef HAVE_SMART_LINK
	else if(npd_packet_type_is_stlk(packet)) {
		packetType = NAM_PACKET_TYPE_STLK_E;
	}
#endif
	return packetType;
}

void npd_dump_rxtx_packet_detail(unsigned char *buffer,unsigned long buffLen)
{
	unsigned int i;
	char lineBuffer[64] = {0}, *bufPtr = NULL;
	unsigned int curLen = 0;

	if(!buffer)
		return;
	if(!NPD_LOG_MODULE_FLAG_SET(all, NPD_LOG_FLAG_PACKET_RCV)) 
        return;
	
	npd_syslog_pkt_rev(".......................RX.......................%d\n",buffLen);
	bufPtr = lineBuffer;
	curLen = 0;
	for(i = 0;i < buffLen ; i++)
	{
		curLen += sprintf(bufPtr,"%02x ",buffer[i]);
		bufPtr = lineBuffer + curLen;
		
		if(0==(i+1)%16) {
			npd_syslog_pkt_rev("%s\n",lineBuffer);
			memset(lineBuffer,0,sizeof(lineBuffer));
			curLen = 0;
			bufPtr = lineBuffer;
		}
	}
	
	if((buffLen%16)!=0)
	{
		npd_syslog_pkt_rev("%s\n",lineBuffer);
	}
	
	npd_syslog_pkt_rev(".......................RX.......................\n");
}


int npd_packet_rx_local_dispatch(int packet_type, unsigned char *buff, int len)
{
	int sock = 0;
    struct sockaddr_un  userToNamAddr;
	memset(&userToNamAddr,0,sizeof(userToNamAddr));
	sock = local_socket[packet_type];
	if(sock == 0)
	{
		return -1;
	}
	userToNamAddr.sun_family = AF_LOCAL;
	sprintf(userToNamAddr.sun_path, "%s%s", NAM_PACKET_SOCK_RX_PREFIX, (nam_get_packet_type_str(packet_type)));
	return sendto(sock, buff, len, 0, (struct sockaddr*)&userToNamAddr, sizeof(userToNamAddr));
}

#ifdef HAVE_SFLOW
/********************************************************************************************
 * 	npd_packet_type_is_SFLOW
 **********************************************************************************************/
int npd_packet_type_is_sflow
(       
	unsigned char  *packetBuff
)
{
    return NPD_FALSE;
}
int npd_packet_rx_sflow
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
		packet_txrx_ctrl   *rxPacket;
		int  length = 0;
		unsigned int array_index;

		rxPacket = (packet_txrx_ctrl *)(packetBuffs);
	 	array_index = eth_port_array_index_from_ifindex(netif_index);
		rxPacket->sync_ctrl.netif_index = array_index;
		rxPacket->sync_ctrl.packet_len = buffLen;
		
		npd_syslog_pkt_rev("rx_sflow: rxPacket->sync_ctrl.netif_index = %u\n", array_index);
		if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char*)rxPacket, rxPacket->sync_ctrl.packet_len + NPD_PKT_RESERVED_LEN)) == -1)
		{
			npd_syslog_pkt_rev("sendto() error fd(%d) packet(%dB) send(%dB)!\n", local_socket[packet_type], buffLen,length);
			return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
		else
		{
			npd_syslog_pkt_rev("sendto() fd(%d) packet(%dB) total send(%dB)\n", \
											local_socket[packet_type],buffLen,length);
		}
	
		return NAM_PACKET_RX_COMPLETE;
}

#endif

/*
报文处理函数的返回值规则:
#define NAM_PACKET_RX_LOCAL_OP_ERR -3
#define NAM_PACKET_RX_HANDLER_NULL  -2
#define NAM_PACKET_RX_TYPE_MISMATCH -1
#define NAM_PACKET_RX_COMPLETE 0
#define NAM_PACKET_RX_DO_MORE 1
因为协议有包含关系，很多情况下，
子协议处理函数处理完以后还希望父协议处理，
返回值就用NAM_PACKET_RX_DO_MORE
如果在子协议中完全处理完了，就用NAM_PACKET_RX_COMPLETE
如果子协议处理函数中其它错误，用NAM_PACKET_RX_LOCAL_OP_ERR
如果是该子协议没有处理函数，返回NAM_PACKET_RX_HANDLER_NULL
*/
int npd_packet_rx_rstp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int    totalLen = 0;
	packet_txrx_ctrl   *rxPacket;
	int  length = 0;

	npd_syslog_dbg("npd_packet_rx_rstp::");
	
	rxPacket = (packet_txrx_ctrl   *)(packetBuffs);
 
	rxPacket->sync_ctrl.netif_index = netif_index;
	rxPacket->sync_ctrl.packet_len = buffLen;
		 

    if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char*)rxPacket, rxPacket->sync_ctrl.packet_len + NPD_PKT_RESERVED_LEN)) == -1)
	{
		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n", local_socket[packet_type], buffLen,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	else
	{
		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",	\
										local_socket[packet_type],buffLen,totalLen);
	}

	return NAM_PACKET_RX_COMPLETE;
}

int npd_packet_rx_dldp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	unsigned int length = 0;
	unsigned int stcLen = NPD_PKT_RESERVED_LEN;
    unsigned int pktlen = buffLen + stcLen;
	packet_sync_ctrl *rxPacket = NULL;

	npd_syslog_dbg("DLDP receive ");

	rxPacket = (packet_sync_ctrl *)(packetBuffs);

	rxPacket->netif_index = netif_index;
	rxPacket->vid = vid;
 
	if (pktlen-stcLen > DLDP_BUFLEN)
	{
		npd_syslog_err("packet %d(B) oversized\r\n", buffLen);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}

	/*8-length of append vid & ifindex*/
	rxPacket->nlh.nlmsg_len = pktlen;
	rxPacket->nlh.nlmsg_type = NPD_DLDP_TYPE_PACKET_MSG;

	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, pktlen)) == -1)
	{
		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n",
							local_socket[packet_type], buffLen, length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	else
    {
		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",
							local_socket[packet_type], buffLen, length);
	}

	return NAM_PACKET_RX_COMPLETE;
}

#define WIRELESS_QOS_FILL_UP_LEN	10
int npd_packet_rx_eap
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	packet_sync_ctrl* rxPacket = (packet_sync_ctrl*)packetBuffs;

	ether_header_t	*layer2 = (ether_header_t*)(packetBuffs + NPD_PKT_RESERVED_LEN);
#ifdef HAVE_CAPWAP_ENGINE	
	if( layer2->etherType != htons(0x888E)){
		memmove(&layer2->etherType, (unsigned char*)(&layer2->etherType)+WIRELESS_QOS_FILL_UP_LEN, buffLen-WIRELESS_QOS_FILL_UP_LEN-MAC_ADDR_LEN*2);
	}
#endif
	npd_syslog_dbg("EAP receive: ethernet type(%x) netif(%x) vid(%d) ",\
							layer2->etherType, netif_index,(unsigned int)vid);

	if(length > MAX_EAP_LEN)
	{
		npd_syslog_err("packet %d(B) oversized\n",buffLen);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}

	rxPacket->netif_index = netif_index;
	rxPacket->vid = vid;
	rxPacket->packet_len = length;
			
	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, (NPD_PKT_RESERVED_LEN + length))) == -1)
	{
		npd_syslog_err("send to ASD error fd(%d) packet(%dB) send(%dB)!\n",
							  local_socket[packet_type],buffLen,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}	
	else
	{
		npd_syslog_dbg("send to EAP fd(%d) packet(%dB) total send(%dB).\r\n",	\
							local_socket[packet_type],buffLen,length);
	}
	
	return NAM_PACKET_RX_COMPLETE;
}

int npd_packet_rx_lldp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	packet_sync_ctrl *rxPacket = (packet_sync_ctrl *)packetBuffs;

	npd_syslog_dbg("LLDP receive ");

	if(length > MAX_LLDP_LEN)
	{
		npd_syslog_err("packet %d(B) oversized\n",length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
    if(son_netif_index != 0)
    {
	    rxPacket->netif_index	= son_netif_index;
    }
	else
	{
		rxPacket->netif_index	= netif_index;
	}
	rxPacket->vid = vid;
	rxPacket->nlh.nlmsg_len = length + NPD_PKT_RESERVED_LEN;
	rxPacket->nlh.nlmsg_type = (unsigned short)packet_type;
			
	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, (NPD_PKT_RESERVED_LEN + length))) == -1)
	{
		npd_syslog_err("send to lldp error fd(%d) packet(%dB) send(%dB)!\n",
							  local_socket[packet_type],buffLen,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}	
	else
	{
		npd_syslog_dbg("send to lldp fd(%d) packet(%dB) total send(%dB).\r\n",	\
							local_socket[packet_type],buffLen,length);
	}
	
	return NAM_PACKET_RX_COMPLETE;
}
/*包含ARP等所有需要上LINUX协议栈的报文*/
int npd_packet_rx_ipv4
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int	length = buffLen;
	VIRT_PKT_INFO_STC   rxPktBuf;
    unsigned int l3Intf = 0;
    
	npd_syslog_dbg(" %s ", nam_get_packet_type_str(packet_type));
    if((NPD_TRUE == npd_vlan_interface_check(vid,&l3Intf))&& (~0UI != l3Intf))
    {
		rxPktBuf.dev_type = NAM_INTERFACE_VID_E;
	    npd_syslog_dbg(" VLAN l3 interface (%d)\r\n", l3Intf);
    }
	else
	{
		if((NPD_TRUE == npd_intf_exist_check(netif_index,&l3Intf))&& (~0UI != l3Intf))
		{
			if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_PORT_E;
	            npd_syslog_dbg(" ETH l3 interface (%d)\r\n", l3Intf);
			}
			else
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_TRUNK_E;
	            npd_syslog_dbg(" TRUNK l3 interface (%d)\r\n", l3Intf);
			}
		}
		else
		{
			npd_syslog_dbg(" NO l3 interface found\r\n");
			return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
	}	

	//npd_arp_snoop_drop_check(packet_type,netif_index,vid,packetBuffs+NPD_PKT_RESERVED_LEN,buffLen);
	
	rxPktBuf.l2_index = netif_index;
	rxPktBuf.vId = vid;
	rxPktBuf.l3_index = l3Intf;
	rxPktBuf.data_addr = (unsigned int)(packetBuffs+NPD_PKT_RESERVED_LEN);
	rxPktBuf.data_len = buffLen;

	length = write(adptVirRxFd,&rxPktBuf,sizeof(VIRT_PKT_INFO_STC));
	if(length != rxPktBuf.data_len)
	{
		npd_syslog_dbg("pass packet(%dB) to kap driver error(%dB)\r\n",		\
								rxPktBuf.data_len,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	npd_syslog_dbg("pass packet(%dB) to kap driver\n",length);

	return NAM_PACKET_RX_COMPLETE;
}


long npd_packet_rx_vrrp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int length = buffLen;
	unsigned int stcLen = NPD_PKT_RESERVED_LEN;
	packet_sync_ctrl *rxPacket = (packet_sync_ctrl *)packetBuffs;

	npd_syslog_dbg("vrrp receive ");
	
	rxPacket->netif_index = netif_index;
	rxPacket->packet_len = (unsigned int)length;
	if (length > MAX_VRRP_LEN)
	{
		npd_syslog_dbg("packet %d(B) oversized\n", buffLen);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}

	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, length +stcLen)) == -1)
	{
		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n",
							local_socket[packet_type], buffLen, length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	else 
	{
	
		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",
							local_socket[packet_type], buffLen, length);
	}

	return NAM_PACKET_RX_COMPLETE;
}


#ifdef HAVE_NPD_IPV6
extern int npd_ndisc_packet_rx_process
(
	unsigned int   packet_type,
	unsigned int   netif_index,
	unsigned short vid,
	unsigned char  isTagged,
	unsigned char  *packet,
	unsigned long  length
);
int npd_packet_rx_ipv6
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int	length = buffLen;
	VIRT_PKT_INFO_STC   rxPktBuf;
    unsigned int l3Intf = 0;
    
	npd_syslog_dbg(" %s ", nam_get_packet_type_str(packet_type));
    if((NPD_TRUE == npd_vlan_interface_check(vid,&l3Intf))&& (~0UI != l3Intf))
    {
		rxPktBuf.dev_type = NAM_INTERFACE_VID_E;
	    npd_syslog_dbg(" VLAN l3 interface (%d)\r\n", l3Intf);
    }
	else
	{
		if((NPD_TRUE == npd_intf_exist_check(netif_index,&l3Intf))&& (~0UI != l3Intf))
		{
			if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_PORT_E;
	            npd_syslog_dbg(" ETH l3 interface (%d)\r\n", l3Intf);
			}
			else
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_TRUNK_E;
	            npd_syslog_dbg(" TRUNK l3 interface (%d)\r\n", l3Intf);
			}
		}
		else
		{
			npd_syslog_dbg(" NO l3 interface found\r\n");
			return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
	}	

	rxPktBuf.l2_index = netif_index;
	rxPktBuf.vId = vid;
	rxPktBuf.l3_index = l3Intf;
	rxPktBuf.data_addr = (unsigned int)(packetBuffs+NPD_PKT_RESERVED_LEN);
	rxPktBuf.data_len = buffLen;

	length = write(adptVirRxFd,&rxPktBuf,sizeof(VIRT_PKT_INFO_STC));
	if(length != rxPktBuf.data_len)
	{
		npd_syslog_dbg("pass packet(%dB) to kap driver error(%dB)\r\n",		\
								rxPktBuf.data_len,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	npd_syslog_dbg("pass packet(%dB) to kap driver\n",length);

	return NAM_PACKET_RX_COMPLETE;
}

#if HAVE_DHCPV6_RELAY
int npd_packet_rx_ipv6_udp_dhcpv6
(
    int packet_type,
    unsigned char *packetBuffs, 
    unsigned long buffLen, 
    unsigned int netif_index,
    unsigned int son_netif_index,
    unsigned short vid,
    unsigned char isTagged,
    int flag
)
{
    return npd_dhcpv6_relay_packet_rx_process(isTagged, vid, packet_type, netif_index, buffLen, packetBuffs + NPD_PKT_RESERVED_LEN);
}
#endif
int npd_packet_rx_ndisc
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int ret = -1;
	ret = npd_ndisc_packet_rx_process(packet_type, netif_index, vid, isTagged,packetBuffs+NPD_PKT_RESERVED_LEN,buffLen);
	if( ret == NDISC_RETURN_CODE_PACKET_DROP )
	{
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	return npd_packet_rx_ipv6(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);

}

int npd_packet_rx_mld_snp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	packet_sync_ctrl *rxPacket = (packet_sync_ctrl *)packetBuffs;
	unsigned int msgSize = 0;
    
	npd_syslog_dbg("mld snooping receive ");
    npd_packet_rx_ipv6(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);

	rxPacket->nlh.nlmsg_type = MLD_SNP_TYPE_PACKET_MSG;
	rxPacket->nlh.nlmsg_flags = MLD_SNP_FLAG_PKT_MLD;

	npd_syslog_dbg("packet type(%d) flag(%d) intf(%d) vid(%d) ",\
							rxPacket->nlh.nlmsg_type,rxPacket->nlh.nlmsg_flags,	\
							netif_index,(unsigned int)vid);

	rxPacket->netif_index = netif_index;
	rxPacket->vid = (unsigned int )vid;
	/*rxPacket->trunkflag = 0;*/ /*not support trunk interface*/

	if(buffLen > MLD_SNOOPING_BUFLEN)
	{
		npd_syslog_err("packet %d(B) oversized\r\n",buffLen);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}

	/*
	 * Unix socket data size:nlmsghdr + ifindex + vlanid + actual packet size 
	 */
	msgSize = buffLen + NPD_PKT_RESERVED_LEN;

	/*8-length of append vid & ifindex*/
	rxPacket->nlh.nlmsg_len = msgSize;

	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, msgSize)) == -1)
	{
		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n",
							local_socket[packet_type], buffLen, length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	else
    {
		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",
							local_socket[packet_type], buffLen, length);
	}

	return NAM_PACKET_RX_COMPLETE;
}

#endif //HAVE_NPD_IPV6

int npd_packet_rx_ipv4_igmp_snp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	packet_sync_ctrl *rxPacket = (packet_sync_ctrl *)packetBuffs;
	unsigned int iphdrLen = 0x14;/* no ip header option as default */
	unsigned int msgSize = 0;
    
	npd_syslog_dbg("igmp snooping receive ");
    npd_packet_rx_ipv4(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);

	ether_header_t  *layer2 = (ether_header_t*)(packetBuffs + NPD_PKT_RESERVED_LEN);
	ip_header_t 	*layer3 = (ip_header_t*)(layer2 + 1);
	iphdrLen = (layer3->hdrLength)<<2;/*ip header length: 20/24*/
	igmp_header_t	*layer4 = (igmp_header_t*)((char*)layer3 + iphdrLen);

	rxPacket->nlh.nlmsg_type = IGMP_SNP_TYPE_PACKET_MSG;
	switch(layer4->type)
	{
		case 0x11:/*membership Query*/
		case 0x12:/*V1 membership Report*/
		case 0x16:/*V2 membership Report*/
		case 0x22:/*v3 membership Report*/
		case 0x17:/*group leave*/
			rxPacket->nlh.nlmsg_flags = IGMP_SNP_FLAG_PKT_IGMP;
			break;
		default :
			rxPacket->nlh.nlmsg_flags = IGMP_SNP_FLAG_PKT_UNKNOWN;
				
	}

	npd_syslog_dbg("packet type(%d) flag(%d) intf(%d) vid(%d) ",\
							rxPacket->nlh.nlmsg_type,rxPacket->nlh.nlmsg_flags,	\
							netif_index,(unsigned int)vid);

	rxPacket->netif_index = netif_index;
	rxPacket->vid = (unsigned int )vid;
	/*rxPacket->trunkflag = 0;*/ /*not support trunk interface*/

	if(buffLen > IGMP_SNOOPING_BUFLEN)
	{
		npd_syslog_err("packet %d(B) oversized\r\n",buffLen);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}

	/*
	 * Unix socket data size:nlmsghdr + ifindex + vlanid + actual packet size 
	 */
	msgSize = buffLen + NPD_PKT_RESERVED_LEN;

	/*8-length of append vid & ifindex*/
	rxPacket->nlh.nlmsg_len = msgSize;

	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, msgSize)) == -1)
	{
		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n",
							local_socket[packet_type], buffLen, length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	else
    {
		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",
							local_socket[packet_type], buffLen, length);
	}

	return NAM_PACKET_RX_COMPLETE;
}


int npd_packet_rx_ipv4_PIM
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	packet_sync_ctrl *rxPacket = (packet_sync_ctrl *)packetBuffs;
	unsigned int msgSize = 0;
	VIRT_PKT_INFO_STC   rxPktBuf;
    unsigned int l3Intf = 0;
    unsigned short ip_flags = 0;
	char *new_buffer = NULL;
    
	npd_syslog_dbg("pim receive ");
	
	ether_header_t  *layer2 = (ether_header_t*)(packetBuffs + NPD_PKT_RESERVED_LEN);
	ip_header_t 	*layer3 = (ip_header_t*)(layer2 + 1);
	
	rxPacket->nlh.nlmsg_type = IGMP_SNP_TYPE_PACKET_MSG;
	rxPacket->nlh.nlmsg_flags = IGMP_SNP_FLAG_PKT_IGMP;

    npd_syslog_dbg("packet type(%d) flag(%d) intf(%d) vid(%d) ",\
							rxPacket->nlh.nlmsg_type,rxPacket->nlh.nlmsg_flags,	\
							netif_index,(unsigned int)vid);

	rxPacket->netif_index = netif_index;
	rxPacket->vid = (unsigned int )vid;

	/*
	 * Unix socket data size:nlmsghdr + ifindex + vlanid + actual packet size 
	 */
	msgSize = buffLen + NPD_PKT_RESERVED_LEN;

	/*8-length of append vid & ifindex*/
	rxPacket->nlh.nlmsg_len = msgSize;

    if((NPD_TRUE == npd_vlan_interface_check(vid,&l3Intf))&& (~0UI != l3Intf))
    {
		rxPktBuf.dev_type = NAM_INTERFACE_VID_E;
	    npd_syslog_dbg(" VLAN l3 interface (%d)\r\n", l3Intf);
    }
	else
	{
		if((NPD_TRUE == npd_intf_exist_check(netif_index,&l3Intf))&& (~0UI != l3Intf))
		{
			if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_PORT_E;
	            npd_syslog_dbg(" ETH l3 interface (%d)\r\n", l3Intf);
			}
			else
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_TRUNK_E;
	            npd_syslog_dbg(" TRUNK l3 interface (%d)\r\n", l3Intf);
			}
		}
		else
		{
        	if(buffLen > PIM_SNOOPING_BUFLEN)
        	{
        		npd_syslog_err("packet %d(B) oversized\r\n",buffLen);
        		return NAM_PACKET_RX_LOCAL_OP_ERR;
        	}

        	if((length = npd_packet_rx_local_dispatch(NAM_PACKET_TYPE_IP_IGMP_E, (unsigned char *)rxPacket, msgSize)) == -1)
        	{
        		npd_syslog_dbg("sendto() error fd(%d) packet(%dB) send(%dB)!\n",
        							local_socket[packet_type], buffLen, length);
        		return NAM_PACKET_RX_LOCAL_OP_ERR;
        	}
        	else
            {
        		npd_syslog_dbg("sendto() fd(%d) packet(%dB) total send(%dB)\n",
        							local_socket[packet_type], buffLen, length);
        	}
        
        	return NAM_PACKET_RX_COMPLETE;
		}
	}	
	
	ip_flags = ntohs(layer3->flags_off);
	ip_flags = (ip_flags & 0x2000);
	rxPktBuf.l2_index = netif_index;
	rxPktBuf.vId = vid;
	rxPktBuf.l3_index = l3Intf;
	
    if(ip_flags == 0)
    {
        new_buffer = malloc(buffLen + NPD_PKT_RESERVED_LEN);
		if(new_buffer == NULL)
		{
		    return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
		
        layer3->totalLen = htons(ntohs(layer3->totalLen) + NPD_PKT_RESERVED_LEN);
	
    	layer3->checkSum = 0;
    	
    	layer3->checkSum = npd_dhcp_snp_checksum(layer3, sizeof(ip_header_t));
    	
		memcpy(new_buffer, (packetBuffs + NPD_PKT_RESERVED_LEN), buffLen);
		memcpy(new_buffer + buffLen, packetBuffs, NPD_PKT_RESERVED_LEN);
		
	    rxPktBuf.data_addr = (unsigned int)(new_buffer);
	    rxPktBuf.data_len = buffLen + NPD_PKT_RESERVED_LEN;
    }
	else
	{
	    rxPktBuf.data_addr = (unsigned int)(packetBuffs+NPD_PKT_RESERVED_LEN);
	    rxPktBuf.data_len = buffLen;
	}
	length = write(adptVirRxFd,&rxPktBuf,sizeof(VIRT_PKT_INFO_STC));
	if(length != rxPktBuf.data_len)
	{
		npd_syslog_dbg("pass packet(%dB) to kap driver error(%dB)\r\n",		\
								rxPktBuf.data_len,length);
		if(new_buffer != NULL)
		{
		    free(new_buffer);
		}
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	if(new_buffer != NULL)
	{
	    free(new_buffer);
	}
	npd_syslog_dbg("pass packet(%dB) to kap driver\n",length);

	return NAM_PACKET_RX_COMPLETE;
}

#ifdef HAVE_UDLD
#include "udld/udld_main.h"
#include "udld/udld_packet.h"
/********************************************************************************************
 * 	npd_packet_type_is_udld
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is udld pdu or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              NPD_TRUE - indicate the packet is lldp packet
 *              NPD_FALSE - indicate the packet is not lldp packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
int npd_packet_type_is_udld
(       
	IN unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;

	if( layer2->etherType != htons(0x89AB))
		return NPD_FALSE;
	
	if((macByte[0]==0x01)&&(macByte[1]==0x80)&& (macByte[2]==0xc2)&&(macByte[3]==0x00)&&(macByte[4]==0x00)&&(macByte[5]==0x02))
	{
		return NPD_TRUE;
	}
	else if((macByte[0]==0x00)&&(macByte[1]==0x1f)&& (macByte[2]==0x64)&&(macByte[3]==0xff)&&(macByte[4]==0xff)&&(macByte[5]==0xff))
	{
		return NPD_TRUE;
	} else
	{
		return NPD_FALSE;
	}
}


int npd_packet_rx_udld
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	udld_skb *rxPacket = (udld_skb *)packetBuffs;

	npd_syslog_dbg("UDLD receive ");

	if(length > (sizeof(udld_skb)))
	{
		npd_syslog_err("packet %d(B) oversized\n",length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
    if(son_netif_index != 0)
    {
	    rxPacket->netif_index = son_netif_index;
    }
	else
	{
		rxPacket->netif_index = netif_index;
	}
	rxPacket->vid = (unsigned int )vid;
	rxPacket->nlh.nlmsg_len = length + NPD_PKT_RESERVED_LEN;
	rxPacket->nlh.nlmsg_type = (unsigned short)packet_type;
			
	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, rxPacket->nlh.nlmsg_len)) == -1)
	{
		npd_syslog_err("send to udld error fd(%d) packet(%dB) send(%dB)!\n",
							  local_socket[packet_type],buffLen,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}	
	else
	{
		npd_syslog_dbg("send to udld fd(%d) packet(%dB) total send(%dB).\r\n",	\
							local_socket[packet_type],buffLen,length);
	}
	
	return NAM_PACKET_RX_COMPLETE;
}
#endif
#ifdef HAVE_ERPP
#include "erpp/erpp_packet.h"
int npd_packet_type_is_erpp
(       
	IN unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	unsigned char 	*macByte = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	macByte = layer2->dmac;
	
	if((macByte[0]==0x01)&&(macByte[1]==0x80)&& (macByte[2]==0xc2)&&(macByte[3]==0x00)&&(macByte[4]==0x00)&&(macByte[5]==0xAA))
	{
		return NPD_TRUE;
	}
	else
	{
		return NPD_FALSE;
	}
}


int npd_packet_rx_erpp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{ 
	int length = buffLen;
	union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)packetBuffs;

	npd_syslog_dbg("ERPP receive ");

	if(length > ERPP_PACKET_LENGTH)
	{
		npd_syslog_err("packet %d(B) oversized\n",length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
    if(son_netif_index != 0)
    {
	    rxPacket->sync_ctrl.netif_index = son_netif_index;
    }
	else
	{
		rxPacket->sync_ctrl.netif_index = netif_index;
	}
	rxPacket->sync_ctrl.vid = (unsigned int )vid;
	rxPacket->sync_ctrl.nlh.nlmsg_len = length + NPD_PKT_RESERVED_LEN;
	rxPacket->sync_ctrl.nlh.nlmsg_type = (unsigned short)packet_type;
			
	if((length = npd_packet_rx_local_dispatch(packet_type, (unsigned char *)rxPacket, rxPacket->sync_ctrl.nlh.nlmsg_len)) == -1)
	{
		npd_syslog_err("send to erpp error fd(%d) packet(%dB) send(%dB)!\n",
							  local_socket[packet_type],buffLen,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}	
	else
	{
		npd_syslog_dbg("send to erpp fd(%d) packet(%dB) total send(%dB).\r\n",	\
							local_socket[packet_type],buffLen,length);
	}
	
	return NAM_PACKET_RX_COMPLETE;
}
#endif
#ifdef HAVE_SMART_LINK
int npd_packet_type_is_stlk(IN unsigned char* packetBuff)
{
    ether_header_t* layer2 = NULL;
    unsigned char* dmac = NULL;

    layer2 = (ether_header_t*)(packetBuff);
    dmac = layer2->dmac;

    if (htons(0x9f11) != layer2->etherType)
    {
        return NPD_FALSE;
    }

    if ((0x01 == dmac[0])
        && (0x80 == dmac[1])
        && (0xc2 == dmac[2])
        && (0x00 == dmac[3])
        && (0x00 == dmac[4])
        && (0x0f == dmac[5]))
    {
        return NPD_TRUE;
    }
    else
    {
        return NPD_FALSE;
    }

    return NPD_FALSE;
}

int npd_packet_rx_stlk
(
    int packet_type,
    unsigned char *packetBuffs, 
    unsigned long buffLen, 
    unsigned int netif_index,
    unsigned int son_netif_index,
    unsigned short vid,
    unsigned char isTagged,
    int flag
)
{
    return npd_smart_link_packet_rx_process(
        packet_type,
        packetBuffs + NPD_PKT_RESERVED_LEN,
        buffLen,
        netif_index,
        isTagged,
        vid
        );
}

#endif

long npd_packet_rx_ipv4_tcp_telnet
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	/*do telnet packet op here*/
	return npd_packet_rx_ipv4(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);
}

int npd_packet_rx_ipv4_icmp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	/*do icmp packet op here*/
	return npd_packet_rx_ipv4(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);
}

int npd_packet_rx_ipv4_udp_dhcp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int ret = -1;
#ifdef HAVE_DHCP_SNP	
	ret = npd_dhcp_snp_packet_rx_process(packet_type, packetBuffs+NPD_PKT_RESERVED_LEN, &buffLen, netif_index, isTagged, vid);
	if( ret == DHCP_SNP_RETURN_CODE_PKT_DROP )
	{
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
#endif
	return npd_packet_rx_ipv4(packet_type, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, flag);
}

int npd_packet_local_rx_arp_reply
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int	length = 0, ret = 0;
	VIRT_PKT_INFO_STC   rxPktBuf;
    unsigned int l3Intf = 0;
	npd_syslog_dbg(" %s at netif 0x%x", nam_get_packet_type_str(packet_type), netif_index);

    if((NPD_TRUE == npd_vlan_interface_check(vid,&l3Intf))&& (~0UI != l3Intf))
    {
		rxPktBuf.dev_type = NAM_INTERFACE_VID_E;
	    npd_syslog_dbg(" VLAN l3 interface (%d)\r\n", l3Intf);
    }
	else
	{
		if((NPD_TRUE == npd_intf_exist_check(netif_index,&l3Intf))&& (~0UI != l3Intf))
		{
			if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_PORT_E;
	            npd_syslog_dbg(" ETH l3 interface (%d)\r\n", l3Intf);
			}
			else
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_TRUNK_E;
	            npd_syslog_dbg(" TRUNK l3 interface (%d)\r\n", l3Intf);
			}
		}
	}

	if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		ret = npd_arp_packet_rx_process(rxPktBuf.dev_type,netif_index,l3Intf,vid,isTagged,packetBuffs+NPD_PKT_RESERVED_LEN,buffLen);
		if(ARP_RETURN_CODE_SUCCESS != ret)
		{
        	return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
	}
	
	if(l3Intf == 0)
	{
	    return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	
	rxPktBuf.l2_index = netif_index;
	rxPktBuf.vId = vid;
	rxPktBuf.l3_index = l3Intf;
	rxPktBuf.data_addr = (unsigned int)packetBuffs+NPD_PKT_RESERVED_LEN;
	rxPktBuf.data_len = buffLen;

	length = write(adptVirRxFd,&rxPktBuf,sizeof(VIRT_PKT_INFO_STC));
	if(length != rxPktBuf.data_len)
	{
		npd_syslog_dbg("pass packet(%dB) to kap driver error(%dB)\r\n",		\
								rxPktBuf.data_len,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	npd_syslog_dbg("pass packet(%dB) to kap driver\n",length);

	return NAM_PACKET_RX_DO_MORE;
    
}

int npd_packet_rx_arp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen, 
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	int ret = -1;
	int	length = 0;
	VIRT_PKT_INFO_STC   rxPktBuf;
    unsigned int l3Intf = ~0UI;
	npd_syslog_dbg(" %s at netif 0x%x", nam_get_packet_type_str(packet_type), netif_index);
    if((NPD_TRUE == npd_vlan_interface_check(vid,&l3Intf))&& (~0UI != l3Intf))
    {
		rxPktBuf.dev_type = NAM_INTERFACE_VID_E;
	    npd_syslog_dbg(" VLAN l3 interface (%d)\r\n", l3Intf);
    }
	else
	{
		if((NPD_TRUE == npd_intf_exist_check(netif_index,&l3Intf))&& (~0UI != l3Intf))
		{
			if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_PORT_E;
	            npd_syslog_dbg(" ETH l3 interface (%d)\r\n", l3Intf);
			}
			else
			{
		        rxPktBuf.dev_type = NAM_INTERFACE_TRUNK_E;
	            npd_syslog_dbg(" TRUNK l3 interface (%d)\r\n", l3Intf);
			}
		}
		else if( NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
		{
			rxPktBuf.dev_type = NAM_INTERFACE_WTP_E;
			npd_syslog_dbg(" Wifi port interface (%d)\r\n", l3Intf);
		}
		else 
		{
			npd_syslog_dbg(" NO l3 interface found\r\n");
			//return NAM_PACKET_RX_LOCAL_OP_ERR;
		}
	}

	ret = npd_arp_packet_rx_process(rxPktBuf.dev_type,netif_index,l3Intf,vid,isTagged,packetBuffs+NPD_PKT_RESERVED_LEN,buffLen);
    if(ARP_RETURN_CODE_SUCCESS != ret)
    {
        return NAM_PACKET_RX_LOCAL_OP_ERR;
    }
	rxPktBuf.l2_index = netif_index;
	rxPktBuf.vId = vid;
	rxPktBuf.l3_index = l3Intf;
	rxPktBuf.data_addr = (unsigned int)packetBuffs+NPD_PKT_RESERVED_LEN;
	rxPktBuf.data_len = (unsigned int)buffLen;

	length = write(adptVirRxFd,&rxPktBuf,sizeof(VIRT_PKT_INFO_STC));
	if(length != rxPktBuf.data_len)
	{
		npd_syslog_dbg("pass packet(%dB) to kap driver error(%dB)\r\n",		\
								rxPktBuf.data_len,length);
		return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	
	if(ARP_RETURN_CODE_SUCCESS != ret)
	{
        return NAM_PACKET_RX_LOCAL_OP_ERR;
	}
	return NAM_PACKET_RX_COMPLETE;
}

int npd_pkt_current_fill_queue = 0;
npd_pkt_list_t *npd_pkt_queue[8];

int npd_thread_packet_rx_npd_sync_handler(int sock, char *buff, int len, void *private_data)
{
    protocol_handle *f_handle_s = NULL;
    int ret;
	packet_sync_ctrl *packet_sync_c = (packet_sync_ctrl *)(buff);

	if(len < 0)
	{
		return -1;
	}
	
    npd_syslog_pkt_rev("Recv packet by sync handler: packet type id = %d.\r\n", packet_sync_c->packet_type);
    if(packet_sync_c->packet_len < 16)
    {
        return -1;
    }
	if(packet_sync_c->flag == NAM_PACKET_RX_TO_MCU || packet_sync_c->flag == NAM_PACKET_RX_TO_SAMPLE)
	{
		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
            npd_syslog_dbg(" to MCU ");
			ret = tipc_client_async_send(NAM_PACKET_RX_SERVICE, SYS_MASTER_ACTIVE_SLOT_INDEX + 1, 
			     buff, len);
			return TIPC_BUFFER_OWNED;
		}
	}
	else if(packet_sync_c->flag == NAM_PACKET_RX_TO_ALL)
	{
		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			ret = tipc_client_async_send(NAM_PACKET_RX_SERVICE, SYS_MASTER_ACTIVE_SLOT_INDEX + 1, 
			             buff, len);
		}
	}
	/*do SAMPLE first*/
	if(packet_sync_c->flag == NAM_PACKET_RX_TO_SAMPLE)
	{
	    if(packet_sync_c->packet_type != NAM_PACKET_TYPE_SAMPLE_E)/*known packet, do sample first, then do it's own handler*/
	    {
        	f_handle_s = npd_packet_handle_search(&protocol_handle_head, NAM_PACKET_TYPE_SAMPLE_E);
        	if(f_handle_s != NULL)
        	{
                npd_syslog_pkt_rev("Find protocol handler: desc = %s.\r\n", f_handle_s->desc);
            	(*f_handle_s->protocol_handler)(NAM_PACKET_TYPE_SAMPLE_E, 
            		                                   (unsigned char *)buff,
            		                                   packet_sync_c->packet_len,
            		                                   packet_sync_c->netif_index,
            		                                   packet_sync_c->son_netif_index,
            		                                   packet_sync_c->vid,
            		                                   packet_sync_c->istagged,
            		                                   NAM_PACKET_RX_TO_LOCAL
            		                                   );
        	}
	    }
	}
	f_handle_s = npd_packet_handle_search(&protocol_handle_head, packet_sync_c->packet_type);
	if(f_handle_s == NULL)
	{
		return TIPC_BUFFER_OWNED;
	}
    npd_syslog_pkt_rev("Find protocol handler: desc = %s.\r\n", f_handle_s->desc);
	(*f_handle_s->protocol_handler)(packet_sync_c->packet_type, 
		                                   (unsigned char *)buff,
		                                   packet_sync_c->packet_len,
		                                   packet_sync_c->netif_index,
		                                   packet_sync_c->son_netif_index,
		                                   packet_sync_c->vid,
		                                   packet_sync_c->istagged,
		                                   NAM_PACKET_RX_TO_LOCAL
		                                   );
    return TIPC_BUFFER_OWNED;
}


int npd_packet_rx_npd_sync_handler(int sock, char *buff, int len, void *private_data)
{
    npd_pkt_list_t *queue = npd_pkt_queue[npd_pkt_current_fill_queue];
    int count;
    int loop = 0;
    int discard = FALSE;
    int size;
    char *old_buff;

	if(len < 0)
	{
		return -1;
	}

    if(1 == NPD_SYSTEM_CPU_NUM)
    {
        npd_thread_packet_rx_npd_sync_handler(sock, buff, len, private_data);
        return 0;
    }

    count = npd_pkt_list_count(queue);
    while(count >= 1000)
    {
        loop++;
        if(loop > NPD_SYSTEM_CPU_NUM)
        {
            discard = TRUE;
            break;
        }
        npd_pkt_current_fill_queue = (npd_pkt_current_fill_queue+1)%NPD_SYSTEM_CPU_NUM;
        queue = npd_pkt_queue[npd_pkt_current_fill_queue]; 
        count = npd_pkt_list_count(queue);
    }
    npd_pkt_list_lock(queue);
    if(discard)
    {
        old_buff = npd_pkt_list_remove_head(queue, &size);
        free(old_buff);
    }
    npd_pkt_list_add_tail(queue, buff, len);
    npd_pkt_list_unlock(queue);

    npd_pkt_current_fill_queue = (npd_pkt_current_fill_queue+1)%NPD_SYSTEM_CPU_NUM;
    return TIPC_BUFFER_OWNED;
}


unsigned npd_thread_rx_handling(void *arg)
{
    npd_pkt_list_t *pkt_list = (npd_pkt_list_t*)arg;
    int len;
    char *buff;
    char name[50];
    int i;

    for(i = 0; i < NPD_SYSTEM_CPU_NUM; i++)
    {
        if(pkt_list == npd_pkt_queue[i])
        {
            sprintf(name, "npdrxthread%d", i);
        	npd_init_tell_whoami(name,0);
            break;
        }
    }

    while(1)
    {
        npd_pkt_list_lock(pkt_list);
        npd_pkt_list_wait(pkt_list);
		buff = npd_pkt_list_remove_head(pkt_list, &len);
		npd_pkt_list_unlock(pkt_list);
		while(buff)
		{
            npd_thread_packet_rx_npd_sync_handler(0, buff, len, NULL);
			free(buff);
            npd_pkt_list_lock(pkt_list);
    		buff = npd_pkt_list_remove_head(pkt_list, &len);
            npd_pkt_list_unlock(pkt_list);
		}
    }
    return 0;
}

int npd_thread_rx_init()
{
    int i;

    for(i = 0; i < NPD_SYSTEM_CPU_NUM; i++)
    {
        char name[20] = {0};

        sprintf(name, "npd_thread_rx%d", i);

        npd_create_pkt_list(&npd_pkt_queue[i]);
        
        nam_thread_create(name, npd_thread_rx_handling, npd_pkt_queue[i],
            NPD_TRUE, NPD_FALSE);
    }
    return 0;
}

int npd_packet_rx_socket_init(int type, int service_priority)
{
	#ifdef _NAM_RX_MULTITHREAD_
	int rx_service_type = NAM_PACKET_RX_SERVICE+type;
	#else
	int rx_service_type = NAM_PACKET_RX_SERVICE;
	static int rx_client_inited = 0;
	#endif
	int ret = -1;
	int sock = -1;
    struct sockaddr_un  namToUserAddr;
    if(type >= NAM_PACKET_TYPE_OTHER || type < 0)
    {
		return -1;
    }
	npd_syslog_dbg("Create packet rx service for %s, type id:%d, service type:%d\r\n", nam_get_packet_type_str(type), type, rx_service_type);
	memset(&namToUserAddr,0,sizeof(namToUserAddr));
	if((sock = socket(AF_LOCAL,SOCK_DGRAM,0))== -1)
	{ 
		npd_syslog_err("create packet rx socket for %s failed\r\n", nam_get_packet_type_str(type));
		return NPD_FAIL;
	}

	fcntl(sock, F_SETFL, O_NONBLOCK);
	namToUserAddr.sun_family = AF_LOCAL;
	sprintf(namToUserAddr.sun_path, "%s%s", NAM_PACKET_SOCK_TX_PREFIX, nam_get_packet_type_str(type));
	if(unlink(namToUserAddr.sun_path) != 0) 
	{
	}
	
	if(bind(sock,(struct sockaddr *)&namToUserAddr,sizeof(namToUserAddr)) == -1)
	{	
		npd_syslog_err("bind socket %d (path:%s) fail \r\n", sock, namToUserAddr.sun_path);
		close(sock);
		return NPD_FAIL;
	}
	chmod(namToUserAddr.sun_path, 0777);

	#ifndef _NAM_RX_MULTITHREAD_
    if(rx_client_inited == 0)
    {
	#endif
    	ret = tipc_client_init(rx_service_type, NULL);
    	if(ret != 0)
    	{
    		npd_syslog_err("%s %d: Create TIPC client for PACKET RX service failed.\r\n", __func__, __LINE__);
    		return -1;
    	}
    	ret = tipc_server_init(rx_service_type, SYS_LOCAL_MODULE_SLOT_INDEX + 1, npd_packet_rx_npd_sync_handler, NULL, NULL);
    	if(ret != 0)
    	{
    		npd_syslog_err("%s %d: Create TIPC socket as server failed.\r\n", __func__, __LINE__);
    		return -1;
    	}
		osal_thread_read_buffer_length_set(rx_service_type, 2000);
    	nam_thread_create("NPD_RX", (unsigned int (*)(void*))osal_thread_master_run, (void *)rx_service_type, NPD_TRUE, NPD_FALSE);
	#ifndef _NAM_RX_MULTITHREAD_
		rx_client_inited = 1;
    }
	#endif
	local_socket[type] = sock;

	return sock;

}


unsigned long npd_packet_rx_adapter_init
(
	GT_VOID
)
{	
	unsigned long	rc = 0;


	/*The 3rd Param of function open was 0,only when open a file in CREATE mode, the 3rd param is needed  */
	if((adptVirRxFd = open("/dev/kap0",O_RDWR))<0)
	{
		npd_syslog_err("open kap %s file descriptor error\n","/dev/kap0");
		return NPD_FAIL;
	}
	npd_syslog_dbg("open kap %s file descriptor %d ok\n","/dev/kap0",adptVirRxFd);
#ifdef HAVE_SFLOW
	npd_packet_rx_handle_register(NAM_PACKET_TYPE_SAMPLE_E, 
							  "sflow", NAM_PACKET_RX_TO_MCU, 
							  npd_packet_type_is_sflow, npd_packet_rx_sflow);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_SAMPLE_E, 40);
#endif

    npd_packet_rx_handle_register(NAM_PACKET_TYPE_BPDU_E, 
		                          "BPDU", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_BPDU, npd_packet_rx_rstp);

	npd_packet_rx_socket_init(NAM_PACKET_TYPE_BPDU_E, 40);
	
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_DLDP_E, 
		                          "DLDP", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_DLDP, npd_packet_rx_dldp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_DLDP_E, 39);
	
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_EAP_E, 
		                          "EAP", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_EAP, npd_packet_rx_eap);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_EAP_E, 38);
	
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_LLDP_E, 
		                          "LLDP", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_lldp, npd_packet_rx_lldp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_LLDP_E, 38);

#ifdef HAVE_UDLD
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_UDLD_E, 
		                          "UDLD", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_udld, npd_packet_rx_udld);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_UDLD_E, 38);
#endif
#ifdef HAVE_SMART_LINK
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_STLK_E, 
		                             "STLK", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_stlk, npd_packet_rx_stlk);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_STLK_E, 38);
#endif

#ifdef HAVE_ERPP
	npd_packet_rx_handle_register(NAM_PACKET_TYPE_ERPP_E, 
								  "ERPP", NAM_PACKET_RX_TO_MCU, 
								  npd_packet_type_is_erpp, npd_packet_rx_erpp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_ERPP_E, 38);
#endif

    npd_packet_rx_handle_register(NAM_PACKET_TYPE_ARP_E, 
		                          "ARP", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_ARP, npd_packet_rx_arp);
    local_socket[NAM_PACKET_TYPE_ARP_E] = adptVirRxFd;
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_ARP_E, NAM_PACKET_TYPE_ARP_REPLY_E,
                                  "ARP_REPLY", NAM_PACKET_RX_TO_MCU,
                                  npd_packet_type_is_ARP_REPLY, npd_packet_local_rx_arp_reply);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_ARP_E, 38);
	
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_IPv4_E, 
		                          "IPv4", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_IPv4, npd_packet_rx_ipv4);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_IPv4_E, 37);
	
    local_socket[NAM_PACKET_TYPE_IPv4_E] = adptVirRxFd;
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_ICMP_E, 
		                             "ICMP", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_ICMP, npd_packet_rx_ipv4_icmp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_IP_ICMP_E, 37);
	
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_IGMP_E, 
		                             "IGMP", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_IGMP, npd_packet_rx_ipv4_igmp_snp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_IP_IGMP_E, 37);
    
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_PIM_E, 
		                             "PIM", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_PIM, npd_packet_rx_ipv4_PIM);
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_UDP_DHCP_E, 
		                             "DHCP", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_Dhcp, npd_packet_rx_ipv4_udp_dhcp);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_IP_UDP_DHCP_E, 37);

/*
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_NDP_E, 
		                          "NDP", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_ndisc, npd_packet_rx_ndisc);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_NDP_E, 37);
    local_socket[NAM_PACKET_TYPE_IPv6_E] = adptVirRxFd;
*/
#ifdef HAVE_NPD_IPV6
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_IPv6_E, 
		                          "IPv6", NAM_PACKET_RX_TO_MCU, 
		                          npd_packet_type_is_IPv6, npd_packet_rx_ipv6);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_IPv6_E, 37);

    local_socket[NAM_PACKET_TYPE_IPv6_E] = adptVirRxFd;
#ifdef HAVE_DHCPV6_RELAY
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv6_E, NAM_PACKET_TYPE_IPv6_UDP_E, 
		                             "DHCPv6", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_IPv6_udp_dhcpv6, npd_packet_rx_ipv6_udp_dhcpv6);
    npd_packet_rx_socket_init(NAM_PACKET_TYPE_IPv6_UDP_E, 37);
#endif

    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv6_E, NAM_PACKET_TYPE_ICMP6_E, 
		                             "ICMP6", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_icmp6, npd_packet_rx_ipv6);
    
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_ICMP6_E, NAM_PACKET_TYPE_NDP_E, 
		                             "NDP", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_ndisc, npd_packet_rx_ndisc);
	npd_packet_rx_socket_init(NAM_PACKET_TYPE_NDP_E, 37);
#ifdef HAVE_MLD_SNP
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_ICMP6_E, NAM_PACKET_TYPE_MLD_E, 
		                             "MLD", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_mld, npd_packet_rx_mld_snp);
    npd_packet_rx_socket_init(NAM_PACKET_TYPE_MLD_E, 37);
#endif
#endif //HAVE_NPD_IPV6

    nam_packet_rx_adapter_init();
	/*
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IPv4_E, NAM_PACKET_TYPE_IP_TCP_E, 
		                             "TCP", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_TCP, NULL);
    npd_packet_rx_sub_handle_register(NAM_PACKET_TYPE_IP_TCP_E, NAM_PACKET_TYPE_IP_TCP_TELNET_E, 
		                             "TELNET", NAM_PACKET_RX_TO_MCU, 
		                             npd_packet_type_is_Telnet, npd_packet_rx_ipv4_tcp_telnet);
    */

    npd_thread_rx_init();

	return rc;
}


protocol_handle *npd_packet_handle_search(struct list_head *current_list, int type)
{
    struct list_head *pos = NULL;
    struct list_head *sub_list = NULL;
    protocol_handle *f_handle_s = NULL;
    if(list_empty(current_list))
    {
		return NULL;
    }
    list_for_each(pos, current_list)
    {
        f_handle_s = list_entry(pos, protocol_handle, list);
        if(f_handle_s->type == type)
        {
			return f_handle_s;
        }
		else
		{
			sub_list = &f_handle_s->sub_list;
			f_handle_s = npd_packet_handle_search(sub_list, type);
			if(f_handle_s != NULL)
			{
				return f_handle_s;
			}
		}
    }
	return NULL;
}

int npd_packet_sub_seach_then_register(struct list_head *current_list, int father_type, protocol_handle *protocol_handle_s)
{
	int ret = NPD_FALSE;
    struct list_head *pos = NULL;
	struct list_head *next_list = NULL;
    protocol_handle *f_handle_s = NULL;
    if(list_empty(current_list))
    {
		return NPD_FALSE;
    }
    list_for_each(pos, current_list)
    {
        f_handle_s = list_entry(pos, protocol_handle, list);
        if(f_handle_s->type == father_type)
        {
            pthread_mutex_lock(&npd_rx_handle_lock);
            list_add(&protocol_handle_s->list, &f_handle_s->sub_list);
            pthread_mutex_unlock(&npd_rx_handle_lock);
			return NPD_TRUE;
        }
		else
		{
			next_list = &f_handle_s->sub_list;
			ret = npd_packet_sub_seach_then_register(next_list, father_type, protocol_handle_s);
			if(ret == NPD_TRUE)
			{
				return NPD_TRUE;
			}
		}
    }
	return NPD_FALSE;
}

int npd_packet_sub_handle_register
(
	struct list_head *handle_list,
	int father_type,
	int type, 
	char *desc, 
	int flags, 
	int (*protocol_filter)(unsigned char  *packetBuff), 
	int (*protocol_handler)(int packet_type,
							unsigned char *packetBuffs, 
			                unsigned long buffLen, 
			                unsigned int netif_index,
			                unsigned int son_netif_index,
			                unsigned short vid,
			                unsigned char isTagged,
			                int flag
	                        )
)
{
	protocol_handle *protocol_handle_s = NULL;
	protocol_handle_s = malloc(sizeof(protocol_handle));
	memset(protocol_handle_s, 0, sizeof(protocol_handle));
	protocol_handle_s->type = type;
	protocol_handle_s->desc = desc;
	protocol_handle_s->flag = flags;
	protocol_handle_s->protocol_filter = protocol_filter;
	protocol_handle_s->protocol_handler = protocol_handler;
	protocol_handle_s->sub_list.next = &protocol_handle_s->sub_list;
	protocol_handle_s->sub_list.prev = &protocol_handle_s->sub_list;
	return npd_packet_sub_seach_then_register(handle_list, father_type, protocol_handle_s);
}


int npd_packet_handle_register
(
	struct list_head *handle_list,
	int type, 
	char *desc, 
	int flags, 
	int (*protocol_filter)(unsigned char  *packetBuff), 
	int (*protocol_handler)(int packet_type,
                    unsigned char *packetBuffs, 
	                unsigned long buffLen, 
	                unsigned int netif_index,
	                unsigned int son_netif_index,
	                unsigned short vid,
	                unsigned char isTagged,
	                int flag
                    )
)
{	
	protocol_handle *protocol_handle_s = NULL;
	protocol_handle_s = malloc(sizeof(protocol_handle));
	memset(protocol_handle_s, 0, sizeof(protocol_handle));
	protocol_handle_s->type = type;
	protocol_handle_s->desc = desc;
	protocol_handle_s->flag = flags;
	protocol_handle_s->protocol_filter = protocol_filter;
	protocol_handle_s->protocol_handler = protocol_handler;
	protocol_handle_s->sub_list.next = &protocol_handle_s->sub_list;
	protocol_handle_s->sub_list.prev = &protocol_handle_s->sub_list;
    pthread_mutex_lock(&npd_rx_handle_lock);
    list_add(&protocol_handle_s->list, handle_list);
    pthread_mutex_unlock(&npd_rx_handle_lock);
    return 0;
}


int npd_packet_handle_unregister(struct list_head *current_list, int type)
{
	int ret = -1;
    struct list_head *pos = NULL;
    struct list_head *pos_sub = NULL;
    struct list_head *sub_list = NULL;
    protocol_handle *f_handle_s = NULL;
    if(list_empty(current_list))
    {
		return -1;
    }
    list_for_each(pos, current_list)
    {
        f_handle_s = list_entry(pos, protocol_handle, list);
        if(f_handle_s->type == type)
        {
			sub_list = &f_handle_s->sub_list;
			if(list_empty(sub_list))
			{
			    list_del(pos);
			    free(f_handle_s);
			}
			else
			{
				list_for_each(pos_sub, sub_list)
				{
					f_handle_s = list_entry(pos_sub, protocol_handle, list);
			        list_del(pos_sub);
			        free(f_handle_s);
				}
			}
			return 0;
        }
		else
		{
			sub_list = &f_handle_s->sub_list;
			ret = npd_packet_handle_unregister(sub_list, type);
			if(ret == 0)
			{
				return 0;
			}
		}
    }
	return -1;
}



int npd_packet_rx_handle_register
(
int type, 
char *desc, 
int flags, 
int (*protocol_filter)(unsigned char  *packetBuff), 
int (*protocol_handler)(int packet_type,
                        unsigned char *packetBuffs, 
		                unsigned long buffLen, 
		                unsigned int netif_index,
		                unsigned int son_netif_index,
		                unsigned short vid,
		                unsigned char isTagged,
		                int flag
                        )
)
{
	return npd_packet_handle_register(&protocol_handle_head,
												type, desc, flags,			
												protocol_filter,
												protocol_handler);	
}

int npd_packet_rx_sub_handle_register
(
int father_type,
int type, 
char *desc, 
int flags, 
int (*protocol_filter)(unsigned char  *packetBuff), 
int (*protocol_handler)(int packet_type,
						unsigned char *packetBuffs, 
		                unsigned long buffLen, 
		                unsigned int netif_index,
		                unsigned int son_netif_index,
		                unsigned short vid,
		                unsigned char isTagged,
		                int flag
                        )
)
{
	return npd_packet_sub_handle_register(&protocol_handle_head,
										father_type, type,
										desc, flags,
										protocol_filter,
										protocol_handler);	
}

int npd_packet_rx_mac_filter(unsigned char *packet_buff)
{
    /*****************************************************
	 * if smac is system-mac or default system base-mac, drop the packet
	 *****************************************************/
	int ret = NPD_FALSE;
	ether_header_t *layer2 = NULL;
	unsigned char sysMac[MAC_ADDRESS_LEN + 1] = {0};
	unsigned char default_basemac[MAC_ADDRESS_LEN + 1]
		= {0x00, 0x0D, 0xEF, 0x00, 0x0D, 0xEF};
	/* get layer 2 head of packet */
	layer2 = (ether_header_t *)(packet_buff);
	
	/* get system mac*/
	ret = npd_system_get_basemac(sysMac, MAC_ADDRESS_LEN);
	if (0 != ret)
	{
		npd_syslog_err("get system mac error when rx packet!\n");
		return NPD_FALSE;
	}
	npd_syslog_dbg("system mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		sysMac[0], sysMac[1], sysMac[2], sysMac[3], sysMac[4], sysMac[5]);

	if (0 == memcmp(layer2->smac, sysMac, MAC_ADDRESS_LEN))
	{
		npd_syslog_warning("packet's smac is system mac: %02x:%02x:%02x:%02x:%02x:%02x, drop it!\n",
								layer2->smac[0], layer2->smac[1], layer2->smac[2],
								layer2->smac[3], layer2->smac[4], layer2->smac[5]);
		return NPD_FALSE;
	}
	if (0 == memcmp(layer2->smac, default_basemac, MAC_ADDRESS_LEN))
	{
		npd_syslog_warning("packet's smac is default system base-mac: 00:0D:EF:00:0D:EF, drop it!\n");
		return NPD_FALSE;
	}
	return NPD_TRUE;
}
int npd_thread_packet_rx_list_handler(
    struct list_head *protocol_list, 
    unsigned char *packetBuffs, 
    unsigned long buffLen, 
    unsigned int netif_index,
    unsigned int son_netif_index,
    unsigned short vid,
    unsigned char isTagged,
    unsigned int packet_flags)
{
	int ret = -1;
    struct list_head *current_list = NULL;
    struct list_head *pos = NULL;
    protocol_handle *protocol_handle_s = NULL;
		
	npd_syslog_dbg("Packet type:");
    list_for_each(pos, protocol_list)
    {
        protocol_handle_s = list_entry(pos, protocol_handle, list);
        if(protocol_handle_s->protocol_filter)
        {
			if((*protocol_handle_s->protocol_filter)(packetBuffs) == NPD_TRUE)
			{
                npd_syslog_dbg(" %s ", protocol_handle_s->desc);
				current_list = &protocol_handle_s->sub_list;
				if(list_empty(current_list))
				{
                	packet_sync_ctrl *packet_sync_c = NULL;
                    char *buff = NULL;
					if(NPD_SYSTEM_CPU_NUM > 1)
					{
                        buff = malloc(1600 + NPD_PKT_RESERVED_LEN);
                        if(NULL == buff)
                            return NAM_PACKET_RX_LOCAL_OP_ERR;
					}
					else
					{
						buff = (char *)(packetBuffs - NPD_PKT_RESERVED_LEN);
					}
                	packet_sync_c = (packet_sync_ctrl *)(buff);
                	packet_sync_c->packet_type = protocol_handle_s->type;
	                if(packet_flags == 1)
	                {
	                    packet_sync_c->flag = NAM_PACKET_RX_TO_SAMPLE;
	                }
					else
					{
                        packet_sync_c->flag = protocol_handle_s->flag;
					}
                	packet_sync_c->packet_len = buffLen;
                	packet_sync_c->netif_index = netif_index;
                	packet_sync_c->son_netif_index = son_netif_index;
                	packet_sync_c->vid = vid;
                	packet_sync_c->istagged = isTagged;
					if(NPD_SYSTEM_CPU_NUM > 1)
					{
                        memcpy(buff + NPD_PKT_RESERVED_LEN, packetBuffs, buffLen);
					}
					
					if(son_netif_index)
                        npd_packet_netif_type_stats_increase(son_netif_index, protocol_handle_s->type);
					else
					{
                        npd_packet_netif_type_stats_increase(netif_index, protocol_handle_s->type);
					}
                    npd_packet_rx_npd_sync_handler(0, buff, buffLen + NPD_PKT_RESERVED_LEN, NULL);
                    packet_flags = 0;
                    return 0;
				}
				else
				{
				    ret = npd_thread_packet_rx_list_handler(current_list, packetBuffs, buffLen, netif_index, son_netif_index, vid, isTagged, 0);
					if(ret != 0 && ret != NAM_PACKET_RX_LOCAL_OP_ERR)
					{
                    	packet_sync_ctrl *packet_sync_c = NULL;
                        char *buff = NULL;
						if(NPD_SYSTEM_CPU_NUM > 1)
						{
                            buff = malloc(buffLen + NPD_PKT_RESERVED_LEN);
                            if(NULL == buff)
                                return NAM_PACKET_RX_LOCAL_OP_ERR;
						}
						else
						{
							buff = (char *)(packetBuffs - NPD_PKT_RESERVED_LEN);
						}
                    	packet_sync_c = (packet_sync_ctrl *)(buff);
                    	packet_sync_c->packet_type = protocol_handle_s->type;
    	                if(packet_flags == 1)
    	                {
    	                    packet_sync_c->flag = NAM_PACKET_RX_TO_SAMPLE;
    	                }
    					else
    					{
                            packet_sync_c->flag = protocol_handle_s->flag;
    					}
                    	packet_sync_c->packet_len = buffLen;
                    	packet_sync_c->netif_index = netif_index;
                    	packet_sync_c->son_netif_index = son_netif_index;
                    	packet_sync_c->vid = vid;
                    	packet_sync_c->istagged = isTagged;
						
						if(NPD_SYSTEM_CPU_NUM > 1)
						{
                            memcpy(buff + NPD_PKT_RESERVED_LEN, packetBuffs, buffLen);
						}
						
					    if(son_netif_index)
                            npd_packet_netif_type_stats_increase(son_netif_index, protocol_handle_s->type);
					    else
                            npd_packet_netif_type_stats_increase(netif_index, protocol_handle_s->type);
					
                        npd_packet_rx_npd_sync_handler(0, buff, buffLen + NPD_PKT_RESERVED_LEN, NULL);
                        packet_flags = 0;
                        return 0;
       
					}
                    
				}
			}
        }
    }
	if(packet_flags == 1)
    {
        	packet_sync_ctrl *packet_sync_c = NULL;
            char *buff = NULL;
			if(NPD_SYSTEM_CPU_NUM > 1)
			{
                buff = malloc(1600 + NPD_PKT_RESERVED_LEN);
                if(NULL == buff)
                    return NAM_PACKET_RX_LOCAL_OP_ERR;
			}
			else
			{
				buff = (char *)(packetBuffs - NPD_PKT_RESERVED_LEN);
			}
        	packet_sync_c = (packet_sync_ctrl *)(buff);
        	packet_sync_c->packet_type = NAM_PACKET_TYPE_SAMPLE_E;
            packet_sync_c->flag = NAM_PACKET_RX_TO_SAMPLE;
        	packet_sync_c->packet_len = buffLen;
        	packet_sync_c->netif_index = netif_index;
        	packet_sync_c->son_netif_index = son_netif_index;
        	packet_sync_c->vid = vid;
        	packet_sync_c->istagged = isTagged;
			if(NPD_SYSTEM_CPU_NUM > 1)
			{
                memcpy(buff + NPD_PKT_RESERVED_LEN, packetBuffs, buffLen);
			}
			
			if(son_netif_index)
                npd_packet_netif_type_stats_increase(son_netif_index, NAM_PACKET_TYPE_SAMPLE_E);
			else
                npd_packet_netif_type_stats_increase(netif_index, NAM_PACKET_TYPE_SAMPLE_E);
			
            npd_packet_rx_npd_sync_handler(0, buff, buffLen + NPD_PKT_RESERVED_LEN, NULL);

            return 0;
	}
	return -1;
}
void npd_packet_rx_print_packet_buffer(unsigned char *buffer,unsigned long buffLen)
{
	unsigned int i;
	unsigned char lineBuffer[64] = {0}, *bufPtr = NULL;
	unsigned int curLen = 0;

	if(!buffer)
		return;
	
	npd_syslog_pkt_rev("-----------------------RX-----------------------%d\r\n", buffLen);
	bufPtr = lineBuffer;
	curLen = 0;
	
	for(i = 0;i < buffLen; i++)
	{
		curLen += sprintf((char*)bufPtr,"%02x ",buffer[i]);
		bufPtr = lineBuffer + curLen;
		
		if(0==(i+1)%16) {
			npd_syslog_pkt_rev("%s\r\n",lineBuffer);
			memset(lineBuffer,0,sizeof(lineBuffer));
			curLen = 0;
			bufPtr = lineBuffer;
		}
	}
	
	if((buffLen%16)!=0)
	{
		npd_syslog_pkt_rev("%s\r\n",lineBuffer);
	}
	
	npd_syslog_pkt_rev("-----------------------RX-----------------------\r\n");
}

struct timeval last_log_time;
unsigned long npd_packet_rx_handling
(
	unsigned char device,
	unsigned char channel,
	void* data
)
{
	PACKET_RX_PARAM_T *rxParams = NULL;
	PACKET_RX_INFO_T *adptpktInfo = (PACKET_RX_INFO_T *)data; 
	unsigned long     	numOfBuff;
	unsigned int netif_index = 0;
	unsigned int son_netif_index = 0;
	unsigned short	vid = 0, trunkId = 0;
	unsigned char 	isTagged = NPD_FALSE;
	unsigned int    ret = 0;
	enum NAM_INTERFACE_TYPE_ENT type;
	int  i = 0;

	if(0 == npd_startup_end)
	{
		return NPD_SUCCESS;
	}
	
	if(NULL == adptpktInfo)
	{
		return NPD_SUCCESS;
	}

	numOfBuff 	= adptpktInfo->buffsNum;
	rxParams = &(adptpktInfo->rxparams);
	vid = rxParams->outerTag;

	
#ifdef HAVE_CAPWAP_ENGINE			
    if(rxParams->src_port == 0xFF)
	{
		type = NAM_INTERFACE_WTP_E;
	}
    else 
#endif	//HAVE_CAPWAP_ENGINE
	if(rxParams->src_trunk > 0)
	{
		{
			type = NAM_INTERFACE_TRUNK_E;
			trunkId = rxParams->src_trunk;
			netif_index = npd_netif_trunk_index(trunkId);
			ret = npd_get_global_index_by_devport(rxParams->rx_unit, rxParams->rx_port, &son_netif_index);
			if(ret != 0)
			{
				return NPD_FAIL;
			}
	        npd_packet_netif_attack_handling(son_netif_index);
		}
		
	}
	else if(PHY_2_PANEL(rxParams->rx_unit, rxParams->rx_port) == -1 )
	{
		type = NAM_INTERFACE_UNKNOWN_E;
		netif_index = 0;
	}
	else
	{
		type = NAM_INTERFACE_PORT_E;
	    ret = npd_get_global_index_by_modport(rxParams->src_mod, rxParams->src_port, &netif_index);
		if(ret != 0)
		{
			return NPD_FAIL;
		}
        npd_packet_netif_attack_handling(netif_index);
        npd_eth_port_get_ptrunkid(netif_index, &trunkId);
        if((unsigned short)-1 != trunkId)
        {
            type = NAM_INTERFACE_TRUNK_E;
			son_netif_index = netif_index;
            netif_index = npd_netif_trunk_index(trunkId);
        }
	}

	if(ret != NPD_SUCCESS)
	{
		return NPD_FAIL;
	}
	if(vid == 0)
	{
		ret = npd_vlan_port_pvid_get(netif_index, &vid);
		if(ret != 0)
		{
			vid = 4095;
		}
	}

    {
        npd_syslog_pkt_rev("RX pkt from netif 0x%x, mux type %s\n",
                      netif_index, 
    					(NAM_INTERFACE_PORT_E == type)? "port":	\
    					(NAM_INTERFACE_TRUNK_E == type)? "trunk":	\
    					(NAM_INTERFACE_WTP_E == type)? "wifi":	\
    					(NAM_INTERFACE_VID_E == type)? "vid":"vidx");
    }

    for(i = 0; i < numOfBuff; i++)
    {
		if(NPD_LOG_MODULE_FLAG_SET(all, NPD_LOG_FLAG_PACKET_RCV))
		{
		    npd_packet_rx_print_packet_buffer(adptpktInfo->buffPtr[i], adptpktInfo->buffLength[i]);
		}
#ifdef HAVE_CAPWAP_ENGINE  				
		if(type == NAM_INTERFACE_WTP_E)
		{
			ether_header_t *layer2 = NULL;
			layer2 = adptpktInfo->buffPtr[i];			
			if( 0 != npd_capwap_get_netif_by_mac_vid(layer2->smac, &vid, &netif_index ))
				continue;
			npd_packet_netif_attack_handling(netif_index);
		}
#endif //HAVE_CAPWAP_ENGINE  	
		if( type == NAM_INTERFACE_UNKNOWN_E)
		{
			ether_header_t *layer2 = (ether_header_t *)adptpktInfo->buffPtr[i];
			
			if( 0 != npd_fdb_check_entry_exist(layer2->smac, vid, &netif_index))
				continue;
		}
	/*如果STP状态为阻塞，丢弃包*/
#ifdef HAVE_BRIDGE_STP
        if((!npd_packet_type_is_BPDU(adptpktInfo->buffPtr[i]))
			#ifdef HAVE_ERPP 
			&& (!npd_packet_type_is_erpp(adptpktInfo->buffPtr[i]))
			#endif
		  )
    	{
			int retVal;
			NAM_RSTP_PORT_STATE_E state = 0;
    		retVal = npd_mstp_get_port_state(vid,netif_index,&state);/*input vid & port index (intfId)*/
    		if((0 == retVal)&&( NAM_STP_PORT_STATE_DISCARD_E == state))
            {
                struct timeval tnow;
                struct timezone tzone;
                char name[50] = {0};
                gettimeofday(&tnow, &tzone);
    
                if (tnow.tv_sec - last_log_time.tv_sec >= 3)
                {
                    npd_netif_index_to_user_fullname(netif_index, name);
                    npd_syslog_official_event(
                        "Receive an packet from %s which STP state is BLOCK.", name);
                }
                last_log_time.tv_sec = tnow.tv_sec;
    			return NPD_SUCCESS;
    		}
    	}
#endif
		
		npd_thread_packet_rx_list_handler(&protocol_handle_head, adptpktInfo->buffPtr[i], adptpktInfo->buffLength[i], netif_index, son_netif_index, vid, isTagged, rxParams->is_sampled);
    }
	return NPD_SUCCESS;

}


#ifdef __cplusplus
}
#endif

