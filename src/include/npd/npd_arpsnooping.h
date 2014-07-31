#ifndef __NPD_ARP_SNOOPING_H__
#define __NPD_ARP_SNOOPING_H__

#define NPD_ARPSNP_HASHTBL_NAME   "npdArpSnpHashTbl"
#define NPD_ARPSNP_CFGTBL_NAME    "npdArpSnpCfgTbl"

#define NPD_ARPSNP_HASH_IP_SIZE (NPD_ARPSNP_TABLE_SIZE)
#define NPD_ARPSNP_HASH_PORT_SIZE  (NPD_ARPSNP_TABLE_SIZE)
#define NPD_ARPSNP_HASH_MAC_SIZE (NPD_ARPSNP_TABLE_SIZE)


#define NPD_ARP_AGE_INTERVAL 5
#define NPD_ARP_AGE_DROP_CNT 2
#define NPD_ARP_AGE_AGE_CNT  600

#define NPD_ARP_ITEM_ONCE_HANDLE 100

#define NPD_ARP_PKTSIZE  128

#define NPD_ARP_INSPECTION_STATUS_NAME "arp_inspection_status_table"
#define NPD_ARP_INSPECTION_STATUS_SIZE 1
#define NPD_ARP_INSPECTION_STATUS_INDEX_NAME "arp_inspection_status_index"


struct arp_inspection_status
{
	unsigned int arp_inspection_enable;
	unsigned char arp_inspection_type;
	npd_vbmp_t allow_arp_vlans;
	npd_pbmp_t trust;
	unsigned int allowzero;
    unsigned short switch_port_control_count[MAX_SWITCHPORT_PER_SYSTEM];
};


/* ethenet ARP packet*/
struct arp_packet_t {
	unsigned short 	hwType;				/*hardware type: 0x0001-ethernet*/
	unsigned short 	protocol;			/* protocol type:0x0800-IP*/
	unsigned char 	hwSize;				/* hardware size*/
	unsigned char 	protSize;			/*protocol size*/
	unsigned short 	opCode;				/* 0x0001-request 0x0002-reply*/
	unsigned char  	smac[MAC_ADDR_LEN];	/* sender's MAC address*/
	unsigned char 	sip[IP_ADDR_LEN]; 	/* sender's ip address*/
	unsigned char 	dmac[MAC_ADDR_LEN];	/* target's MAC address*/
	unsigned char 	dip[IP_ADDR_LEN];	/* target's ip address*/
};

/* ethernet packet structure*/
struct ip_packet_t {
	struct ip_header_t		layer3;
	unsigned char 			*payload;
};

/* common ethernet packet format*/
struct ethernet_packet_t {
	struct ether_header_t		layer2;
	union {
		struct ip_packet_t 		ip;
		struct arp_packet_t 	arp;
	}payload;
};


struct npd_arp_inspection_statistics_s
{
    unsigned int is_valid;
    unsigned int eth_g_index;
    unsigned int permit;
    unsigned int drop;
};


#define IP_CONVERT_STR2ULONG(buf) 	\
		((buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|(buf[3]) )





int npd_arp_snooping_show_table(void* entry, char *buf, int buf_len);

int npd_arp_snooping_del_all
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag    
);

int npd_arp_snooping_static_valid_set
(
    hash_table_index_t *hash, 
	void *data,
	unsigned int flag
);

int npd_arp_snooping_del_kern_arp
(
	struct arp_snooping_item_s *item
);

int npd_arp_snooping_sync2kern
(
	void
);


int npd_arp_snooping_mac_legality_check
(
    unsigned char * ethSmac,
    unsigned char * ethDmac,
    unsigned char * arpSmac,
    unsigned char * arpDmac,
    unsigned short opCode
);


int npd_arp_snooping_create_kern_arp
(
	struct arp_snooping_item_s *item
);


 int npd_arp_snooping_create_static_arp
(
	unsigned int ifindex,
	unsigned int ipAddr,
	unsigned int ipMask,
	unsigned char* mac,
	unsigned short vid,
	unsigned int eth_g_index
);


 int npd_arp_snooping_del_static_by_mac_L3index
(
	unsigned int ifindex,
	unsigned int ipAddr,
	unsigned int ipMask,
	unsigned char* mac,
	unsigned short vid,
	unsigned int eth_g_index
);


int npd_dbus_save_static_arp
(
    hash_table_index_t *hash, 
	struct arp_snooping_item_s *dbItem,
	int flag
);


void npd_dbus_save_arp_inspection_cfg
(
	char** showStr,
	unsigned int* avalidLen
);


unsigned int npd_arp_snooping_filter_by_static
(
	void *data1,
	void *data2
);



unsigned int npd_arp_snooping_filter_by_network
(
    void *data1,
    void *data2
);
unsigned int npd_arp_snooping_del_static_by_ifindex
(
	unsigned int   eth_g_index
);

unsigned int npd_arp_snooping_del_static_by_ifindex_vid
(
	unsigned int   eth_g_index,
	unsigned short vlan_id
);


int npd_arp_snooping_find_item_bymacvid
(
	unsigned char*  macAddr,
	unsigned short vlanId,
	struct arp_snooping_item_s *dbItem
);

void npd_arp_snooping_drop_handle();

int npd_arp_snooping_check_ip_address
(
    unsigned int ipAddress,
    unsigned short vid,
    unsigned int eth_g_index
);


void npd_arpsnp_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);

void npd_arpsnp_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);


unsigned int npd_dhcp_snp_query_arp_inspection
(
    unsigned int netif_index,
	struct arp_packet_t	   *arpPacket,
	unsigned char *isFound
);
unsigned int npd_arp_inspection_check_global_endis
(
	unsigned int *endis
);
unsigned int npd_arp_inspection_set_global_endis
(
	unsigned int endis
);
unsigned int npd_arp_inspection_check_vlan_endis
(
	unsigned short vid,
	unsigned char *endis
);
unsigned int npd_arp_inspection_set_vlan_endis
(
	unsigned short vid,
	unsigned char endis
);
unsigned int npd_arp_inspection_check_trust
(
	unsigned int eth_g_index,
	unsigned int *trust_mode,
	struct arp_inspection_status* user
);
unsigned int npd_arp_inspection_set_trust
(
	unsigned int trust_mode,
	unsigned int eth_g_index 
);
unsigned int npd_arp_inspection_validate_type_set
(
	struct arp_inspection_status *user,
	unsigned int aiv_type,
	unsigned int op,
	unsigned int allowzero
);
unsigned int npd_arp_inspection_validate_type_check
(
	struct arp_inspection_status *user,
	unsigned int aiv_type,
	unsigned char *status
);
unsigned int npd_query_arp_inspection
(
	struct ether_header_t *layer2 ,
	struct arp_packet_t	   *arpPacket,
	struct arp_inspection_status *user,
	unsigned int aiv_type,
	unsigned char *isFound
);
unsigned int npd_arp_inspection_global_status_get
(
	struct arp_inspection_status *user
);
unsigned int npd_arp_inspection_global_status_insert
(
	struct arp_inspection_status *user
);
unsigned int npd_arp_inspection_global_status_update
(
	struct arp_inspection_status *user
);



#endif
