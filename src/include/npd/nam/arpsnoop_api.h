#ifndef __NPD_ARPSNOOP_API_H__
#define __NPD_ARPSNOOP_API__H__



void npd_init_arpsnooping(void);

unsigned int npd_route_nexthop_tblindex_get
(
	unsigned int ifIndex,
	unsigned int ipAddr,
	unsigned int *tblIndex
);

unsigned int npd_route_nexthop_iteminfo_get
(
	unsigned char devNum,
	unsigned int tblIndex,
	struct arp_snooping_item_s *item,
	unsigned int* refCnt
);

int npd_route_nexthop_find
(	
	unsigned int ipAddr,
	struct route_nexthop_brief_s *nexthopEntry
);

int npd_arp_snooping_ip_gateway_check
(
	unsigned int ipAddr,
	unsigned int * gateway
);


int npd_arp_snooping_ip_subnet_check
(
	unsigned int ipAddr,
	unsigned int * gateway,
	unsigned int * mask
);


int npd_arp_snooping_ip_valid_check
(
	unsigned int ipAddr
);


int npd_arp_snooping_find_item_byip
(
	unsigned int  ipAddr,
	struct arp_snooping_item_s *dbItem
);


int npd_arp_snooping_learning
(
	struct arp_snooping_item_s *item
);


int npd_arp_packet_rx_process
(
	unsigned int   srcType,
	unsigned int   intfId,
	unsigned int   ifIndex,
	unsigned short vid,
	unsigned char  isTagged,
	unsigned char  *packet,
	unsigned long  length
);

int npd_arp_snooping_del
(
    hash_table_index_t *hash,
	void *data,
	unsigned int del_flag
);

unsigned int npd_arp_snooping_del_by_ifindex
(
	unsigned int   eth_g_index
);

unsigned int npd_arp_snooping_del_by_ifindex_vid
(
	unsigned int   eth_g_index,
	unsigned short vlan_id
);

unsigned int npd_arp_snooping_del_by_network
(
	unsigned int ip,
	unsigned int mask
);

unsigned int npd_arp_snooping_del_static_by_network
(
	unsigned int ip,
	unsigned int mask
);

unsigned int npd_arp_snooping_static_valid_set_by_network
(
	unsigned int ip,
	unsigned int mask,
	unsigned int isValid
);

int npd_arp_snooping_lookup_arpinfo
(
	unsigned int ifindex,
	unsigned short vlanId,
	unsigned char* pktdata,
	unsigned int *netifIndex
);

unsigned int npd_arp_snooping_del_by_network
(
	unsigned int ip,
	unsigned int mask
);

int npd_arp_snooping_find_item_bymac
(
	unsigned char*  macAddr,
	struct arp_snooping_item_s *dbItem
);
int npd_arp_snooping_del_static
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
);

int npd_arp_snooping_is_brc_mac(unsigned char * mac);

int npd_arp_snooping_is_muti_cast_mac
(
    unsigned char * mac
);

int npd_arp_snooping_is_zero_mac(unsigned char * mac);

unsigned int npd_arp_snooping_count_all( void);

int npd_arp_snooping_solicit_send
(
	struct arp_snooping_item_s *item
);

/*NAM LAYER API*/
int nam_arp_table_index_init
(
	void
);

int nam_arp_get_mactbl_index
(
	index_elem	 *val
);
int nam_arp_free_mactbl_index
(
	index_elem	 val
);
int nam_arp_get_nexthop_tbl_index
(
	index_elem	 *val
);
int nam_arp_free_nexthop_tbl_index
(
	index_elem	 val
);
int nam_arp_snooping_op_item
(
	struct arp_snooping_item_s *dbItem,
	unsigned int action,
	unsigned int *tblIndex
);
int nam_arp_snooping_get_item
(
	unsigned char devNum,
	unsigned int  tblIndex,
	struct arp_snooping_item_s *val
);
unsigned int nam_arp_solicit_send
(
	struct arp_snooping_item_s *item,
	unsigned char *sysMac,
	unsigned int gateway
);

unsigned int nam_arp_aging_dest_mac_broadcast
(
	unsigned int isBroadCast
);
int nam_arp_nexthop_tbl_index_get
(
    index_elem val
);
int nam_arp_mactbl_index_get
(
	index_elem val
);
unsigned long nam_arp_gratuitous_send
(
	unsigned int netif_index,
	unsigned short vid,
	unsigned char *smac,
	unsigned char *dmac,
	unsigned int ipAddress
);
unsigned int nam_vlan_arp_trap_en(unsigned short vid,unsigned int enable);
int nam_arp_get_nexthop_tbl_range_index
(
	 int count,
	 index_elem	 *val
);
int nam_arp_free_nexthop_tbl_range_index
(
	int count,
	index_elem val
);
#endif

