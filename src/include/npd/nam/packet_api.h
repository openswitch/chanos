#ifndef __NPD_PACKET_API_H__
#define __NPD_PACKET_API_H__

/*NPD LAYER API*/
extern 	int                 adptVirRxFd;
extern int nam_packet_tx_test_unicast
(
    int packet_type,
    unsigned char devNum,
    unsigned char portNum,
    unsigned int vid,
    unsigned int isTagged,
    unsigned char* data,
    unsigned int dataLen
);
extern int nam_packet_tx_unicast_send
(
    unsigned int netif_index,
    unsigned int vid,
    unsigned int isTagged,
    unsigned char* data,
    unsigned int dataLen,
    unsigned char* dma_buff
);

#ifdef HAVE_DHCP_SNP
extern unsigned int npd_dhcp_snp_packet_tx_hook
(
	int packetType,
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netif_index,
	unsigned char isTagged,
	unsigned short vid
);
#endif

int nam_packet_tx_unicast_by_netif
(
    int packet_type,
    unsigned int netif_index,
    unsigned int vid,
    unsigned int isTagged,
    unsigned char* data,
    unsigned int dataLen
);
int nam_packet_tx_broadcast_global
(
    int packet_type,
    unsigned int vid,
    unsigned char* data,
    unsigned int dataLen
);

int nam_packet_tx_unicast_by_slot
(
    int packet_type,
    unsigned int slot_id,
    unsigned int vid,
    unsigned int isTagged,
    unsigned char* data,
    unsigned int dataLen
);

unsigned char *npd_packet_alloc(unsigned int size);
unsigned int npd_packet_free(unsigned char *data);
unsigned long npd_packet_rx_adapter_init();

int npd_packet_type_is_BPDU
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_lacp
(       
	unsigned char  *packetBuff
);
#ifdef HAVE_SFLOW
/********************************************************************************************
 * 	npd_packet_type_is_SFLOW
 **********************************************************************************************/
int npd_packet_type_is_sflow
(       
	unsigned char  *packetBuff
);
#endif
int npd_packet_type_is_EAP
(       
    unsigned char  *packetBuff
);
int npd_packet_type_is_ARP
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_ARP_REPLY
(
    unsigned char *packetBuff
);
int npd_packet_type_is_ipmc_data
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_IPv4
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_IPv6
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_IPv6_udp_dhcpv6
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_icmp6
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_ndisc
(       
	unsigned char  *packetBuff
);

int npd_packet_type_is_mld
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_ICMP
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_TCP
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_Telnet
(       
	unsigned char  *packetBuff
);

unsigned int npd_packet_type_is_Ssh
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_Ftp
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_Ftp_Data
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_UDP
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_IPIP
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_Dhcp
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_IGMP
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_PIM
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_DLDP
(       
	unsigned char  *packetBuff
);
unsigned int npd_packet_type_is_VRRP
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_lldp
(       
	unsigned char  *packetBuff
);
int npd_packet_type_is_stlk
(
    unsigned char* packetBuff
);

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
);

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
);
int nam_packet_tx_broadcast
(
unsigned int vid,
unsigned char* data,
unsigned int dataLen
);

int ip6t_ext_hdr(unsigned char nexthdr);


enum NAM_PACKET_TYPE_ENT nam_packet_parse_txtype 
(
	unsigned char  *packet
);

int npd_packet_netif_type_stats_get(unsigned int netif_index, unsigned int *rx_type_stats, unsigned int *rx_type_pps);
int npd_packet_netif_type_stats_clear(unsigned int netif_index);

int npd_netif_attack_init();
int npd_netif_attack_timer();

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
);

int npd_packet_rx_socket_init(int type, int service_priority);
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
);
int npd_packet_tx_broadcast_exclude_netif
(
    int packet_type,
    unsigned int vid,
    unsigned char* data,
    unsigned int dataLen,
    unsigned int netif_index
);
unsigned long nam_packet_tx_arp_reply
(
    unsigned int netif_index,
    unsigned short vid,
    unsigned char isTagged,
    unsigned char *smac,
    unsigned char *dmac,
    unsigned int sip,
    unsigned int dip
);

unsigned long nam_packet_tx_arp_solicit
(
    unsigned int netif_index,
    unsigned short vid,
    unsigned char *smac,
    unsigned char *dmac,
    unsigned int sip,
    unsigned int dip
);


/*NAM LAYER API*/
unsigned long nam_packet_rx_adapter_init(void);
unsigned char * nam_get_packet_type_str
(
	enum NAM_PACKET_TYPE_ENT packetType
);


unsigned long nam_packet_tx_adapter_init();
void nam_packet_tx_session_dma_free(NAM_PACKET_TX_SESSION *nam_packet_tx_session);
unsigned char *	nam_dma_cache_malloc
(       
	unsigned char  device,
	unsigned int  size
);
unsigned int nam_dma_cache_free
(       
	unsigned char  device,
	unsigned char  *data
);
int nam_packet_tx_broadcast_exclude_netif
(
    unsigned int vid,
    unsigned char* data,
    unsigned int dataLen,
    unsigned int netif_index
);

extern unsigned char *namPacketTypeStr[];


#endif

