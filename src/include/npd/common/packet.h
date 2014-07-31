#ifndef __COMMON_PACKET_H__
#define __COMMON_PACKET_H__

#define NAM_SDMA_TX_PER_BUFFER	1652
#define NAM_ARP_SOLICIT_PKTSIZE	80
#define BROADCAST_EXCLUDE_NETIF 1
#define NPD_PKT_RESERVED_LEN  64

/* structure definition begin */
#define IP_ADDR_LEN     4
#define BUFF_LEN		5
#define IGMP_SNOOPING_BUFLEN 1024	/*fill in the blank in packet buff */ /*1024*/
#define PIM_SNOOPING_BUFLEN 2048	/*fill in the blank in packet buff */ /*2048*/
#define MLD_SNOOPING_BUFLEN 1024  


/* MACRO definition begin */
#define ADAPTER_RX_PACKET_LEN_MAX 		    1536
#define ADAPTER_RX_PACKET_BUFF_NUM_MAX   	5
#define SOCK_PATH 		  					"/tmp/rstp_socket"

#define NAM_TX_MAX_PACKET_LEN    4096

typedef enum _NAM_PACKET_TX_DISPATCH_TYPE_
{
	NAM_PACKET_TX_FWD,
	NAM_PACKET_TX_DROP,
	NAM_PACKET_TX_BC
}NAM_PACKET_TX_DISPATCH_TYPE;


typedef struct  ether_header_t
{
	unsigned char		dmac[6];		/* destination eth addr	*/
	unsigned char		smac[6];		/* source ether addr	*/
	unsigned short		etherType;
}ether_header_t;

typedef struct ether_8021q_t
{
	unsigned short 		tpid; 	/* vlan tagged packet TPID value */
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned short		pri:3,	/* 802.1Q vlan priority */
						cfi:1,  /* 802.1Q CFI */
						vid:12;	/* 802.1Q VID */
#else
	unsigned short		vid:12,	/* 802.1Q VID */
						cfi:1,  /* 802.1Q CFI */
	                    pri:3;	/* 802.1Q vlan priority */
#endif
}ether_8021q_t;

/* ethernet ip packet ICMP structure*/
typedef struct icmp_header_t {
	unsigned char	type;				/* icmp type: 8-echo request 0-echo reply*/
	unsigned char 	code;				
	unsigned short 	checksum;			/* icmp packet checksum*/
	unsigned short	id;					/* identification*/
}icmp_header_t;

typedef struct icmp_packet_t {
	struct icmp_header_t 	header;
}icmp_packet_t;

typedef struct ip_header_t
{
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned char      version:4,
				       hdrLength:4;
    unsigned char      dscp:6;
    unsigned char      ecn:2;
#else
    unsigned char      hdrLength:4,
				       version:4;
    unsigned char      ecn:2;
    unsigned char      dscp:6;
#endif
    unsigned short     totalLen;
    unsigned short     identifier;
    unsigned short     flags_off;
    unsigned char      ttl;
    unsigned char      ipProtocol;
    unsigned short     checkSum;
    unsigned char      sip[IP_ADDR_LEN];
    unsigned char      dip[IP_ADDR_LEN];
}ip_header_t;

typedef struct arp_header_t{
	unsigned short 	hwType;				/* hardware type: 0x0001-ethernet */
	unsigned short 	protType;			/* protocol type:0x0800-IP */
	unsigned char 	hwSize;				/* hardware size */
	unsigned char 	protSize;			/* protocol size */
	unsigned short 	opCode;				/* 0x0001-request 0x0002-reply */
	unsigned char  	smac[MAC_ADDRESS_LEN];/* sender's MAC address */
	unsigned char 	sip[IP_ADDR_LEN];	/* sender's ip address */
	unsigned char 	dmac[MAC_ADDRESS_LEN];/* target's MAC address */
	unsigned char 	dip[IP_ADDR_LEN];	/* target's ip address */
}arp_header_t;

typedef struct igmp_header_t
{
	unsigned char	type;
	unsigned char	code;
	unsigned short	checksum;
	unsigned long	groupaddr;
}igmp_header_t;

typedef struct udp_header_t
{
	unsigned short source;
	unsigned short	dest;
	unsigned short len;
	unsigned short check;
}udp_header_t;

typedef struct tcp_header_t
{
	unsigned short	source;
	unsigned short	dest;
	unsigned short	seq;
	unsigned short	ack_seq;
	unsigned short	doff:4,
					res1:4,
					cwr:1,
					ece:1,
					urg:1,
					ack:1,
					psh:1,
					rst:1,
					syn:1,
					fin:1;
	unsigned short	window;
	unsigned short	check;
	unsigned short	urg_ptr;
}tcp_header_t;

enum NAM_PACKET_TYPE_ENT {
	NAM_PACKET_TYPE_BPDU_E = 0,
	NAM_PACKET_TYPE_GVRP_E = 1,
	NAM_PACKET_TYPE_ARP_E = 2,
	NAM_PACKET_TYPE_ARP_REPLY_E = 3,
	NAM_PACKET_TYPE_IPv4_E = 4,
	NAM_PACKET_TYPE_IP_ICMP_E = 5,
	NAM_PACKET_TYPE_IP_TCP_E = 6,
	NAM_PACKET_TYPE_IP_UDP_E = 7,
	NAM_PACKET_TYPE_IP_IGMP_E = 8,
	NAM_PACKET_TYPE_IP_PIM_E = 9,
	NAM_PACKET_TYPE_IP_TCP_TELNET_E = 10,
	NAM_PACKET_TYPE_IP_TCP_SSH_E = 11,
	NAM_PACKET_TYPE_IP_TCP_FTP_E = 12,
	NAM_PACKET_TYPE_IP_TCP_FTP_DATA_E = 13,
	NAM_PACKET_TYPE_IP_UDP_DHCP_E = 14,
	NAM_PACKET_TYPE_IP_IPIP_E = 15,
	NAM_PACKET_TYPE_DLDP_E = 16,
	NAM_PACKET_TYPE_VRRP_E = 17,
	NAM_PACKET_TYPE_EAP_E = 18,
	NAM_PACKET_TYPE_LLDP_E = 19,
	NAM_PACKET_TYPE_LACP_E = 20,
	NAM_PACKET_TYPE_NDP_E = 21,
	NAM_PACKET_TYPE_IPv6_E = 22,
	NAM_PACKET_TYPE_UDLD_E = 23,
	NAM_PACKET_TYPE_IPMC_DATA_E = 24,
	NAM_PACKET_TYPE_IPv6_UDP_E = 25,
	NAM_PACKET_TYPE_ICMP6_E = 26,
	NAM_PACKET_TYPE_MLD_E = 27,
	NAM_PACKET_TYPE_STLK_E = 28,
	NAM_PACKET_TYPE_ERPP_E= 29,
	NAM_PACKET_TYPE_SAMPLE_E= 30,
	NAM_PACKET_TYPE_VLAG_E = 31,
	NAM_PACKET_TYPE_OTHER
};

enum NAM_INTERFACE_TYPE_ENT{
    NAM_INTERFACE_PORT_E = 0,
    NAM_INTERFACE_TRUNK_E,
    NAM_INTERFACE_VIDX_E,
    NAM_INTERFACE_VID_E,
    NAM_INTERFACE_WTP_E,
    NAM_INTERFACE_UNKNOWN_E
};

typedef struct npd_netif_attack_s
{
    unsigned int netif_index;
    int netif_deny2cpu;
    int netif_2cpu_state;
    unsigned int stat_packets;
    int netif_denytimes;
    int netif_holddeny_times;
    int detect_free;
	unsigned int rx_type_stats[NAM_PACKET_TYPE_OTHER];
	unsigned int rx_type_stats_last[NAM_PACKET_TYPE_OTHER];
	unsigned int rx_type_pps[NAM_PACKET_TYPE_OTHER];
}npd_netif_attack_t;

/*****RSTP BPDU PKT********/
#define MAX_BPDU_LEN    1200
typedef struct 
{
	unsigned int 		intfId;
	unsigned int		pktLen;
}BPDU_PKT_PARAM_STC;

/*****IGMP SNOOP PKT******/

/*IGMP Snoop kernel message type*/
#define	IGMP_SNP_TYPE_MIN				0
#define	IGMP_SNP_TYPE_NOTIFY_MSG		1	/*notify message*/
#define	IGMP_SNP_TYPE_PACKET_MSG		2	/*Packet message*/
#define IGMP_SNP_TYPE_DEVICE_EVENT		3	/*device message*/
#define	IGMP_SNP_TYPE_MAX				9

/*IGMP Snoop message flag*/
#define	IGMP_SNP_FLAG_MIN				0
#define	IGMP_SNP_FLAG_PKT_UNKNOWN		1	/*Unknown packet*/
#define	IGMP_SNP_FLAG_PKT_IGMP			2	/*IGMP,PIM packet*/
#define	IGMP_SNP_FLAG_ADDR_MOD			3	/*notify information for modify address*/
#define	IGMP_SNP_FLAG_MAX				9	

/*****MLD SNOOP PKT******/

/*MLD Snoop kernel message type*/
#define	MLD_SNP_TYPE_MIN				0
#define	MLD_SNP_TYPE_NOTIFY_MSG		1	/*notify message*/
#define	MLD_SNP_TYPE_PACKET_MSG		2	/*Packet message*/
#define MLD_SNP_TYPE_DEVICE_EVENT		3	/*device message*/
#define	MLD_SNP_TYPE_MAX				9

/*MLD Snoop message flag*/
#define	MLD_SNP_FLAG_MIN				0
#define	MLD_SNP_FLAG_PKT_MLD			1	/*MLD,PIM packet*/
#define	MLD_SNP_FLAG_ADDR_MOD			2	/*notify information for modify address*/
#define	MLD_SNP_FLAG_MAX				9	


/*************************************************************/
/* for DLDP															*/
#define	NPD_DLDP_TYPE_PACKET_MSG	(2)				/*Packet message				*/

#define DLDP_BUFLEN 				(128)			/*fill in the blank in packet buff	*/

/*************************************************************/

/*****RSTP BPDU PKT********/
#define MAX_VRRP_LEN    1200
#define IPPROTO_VRRP	112	/* IP protocol number -- rfc2338.5.2.4*/
#define MAX_EAP_LEN     512
#define MAX_LLDP_LEN    1024

typedef struct
{
	enum NAM_INTERFACE_TYPE_ENT dev_type;
	unsigned int l3_index;
	unsigned int l2_index;
	unsigned int vId;
	unsigned int data_len;		
	unsigned int data_addr;	
}VIRT_PKT_INFO_STC;

#define BUFF_LEN	5

typedef struct _PACKET_RX_PARAM_S_ {
	unsigned char src_port;					/* Source port used in header/tag. */
	char src_trunk; 				/* Source trunk group ID used in header/tag, -1 if src_port set . */
	unsigned short src_mod; 				/* Source module ID used in header/tag. */
	unsigned short pkt_len; 				/* Packet length according to flags. */
	unsigned short tot_len; 				/* Packet length as transmitted or received. */
	unsigned char rx_unit;					/* Local rx unit. */
	unsigned char rx_port;					/* Local rx port; not in HG hdr. */
	unsigned char rx_untagged;				/* The packet was untagged on ingress. */
	unsigned int rx_classification_tag;	/* Classification tag. */
	unsigned int rx_matched;				/* Field processor matched rule. */
	unsigned int rx_timestamp;			/* Time stamp of time sync protocol packets. */
	char dma_channel;				/* DMA channel used; may be -1. */
	unsigned int outerTag;				/* Outer vlan tag if Q-in-Q packet, other vlan id */
	unsigned int innerTag;				/* Inner vlan tag if Q-in-Q packet, other no use */
	unsigned int is_sampled;            /*Is sampled packet*/
}PACKET_RX_PARAM_T;

typedef struct _PACKET_RX_INFO_S_ {
	unsigned int buffsNum;
	unsigned char* buffPtr[BUFF_LEN];
	unsigned int buffLength[BUFF_LEN];
	PACKET_RX_PARAM_T rxparams;
}PACKET_RX_INFO_T;

typedef enum _NAM_PACKET_RX_DISPATCH_TYPE_
{
	NAM_PACKET_RX_TO_LOCAL,
	NAM_PACKET_RX_TO_MCU,
	NAM_PACKET_RX_TO_ALL,
	NAM_PACKET_RX_TO_SAMPLE
}NAM_PACKET_RX_DISPATCH_TYPE;
#define NAM_PACKET_RX_LOCAL_OP_ERR -3
#define NAM_PACKET_RX_HANDLER_NULL  -2
#define NAM_PACKET_RX_TYPE_MISMATCH -1
#define NAM_PACKET_RX_COMPLETE 0
#define NAM_PACKET_RX_DO_MORE 1
typedef struct _protocol_handle_
{
	struct list_head list;
	enum NAM_PACKET_TYPE_ENT type;
	char *desc;
	int flag;
	int (*protocol_filter)(unsigned char  *packetBuff);
	int (*protocol_handler)(int packet_type,
		                    unsigned char *packetBuffs, 
		                    unsigned long buffLen, 
		                    unsigned int netif_index,
		                    unsigned int son_netif_index,
		                    unsigned short vid,
		                    unsigned char isTagged,
		                    int flag
                            );
	struct list_head sub_list;
}protocol_handle;

typedef struct _packet_sync_ctrl_
{
	struct nlmsghdr		nlh;
	int packet_type;
	int flag;
	unsigned int netif_index;
	unsigned int son_netif_index;
	unsigned short vid;
	unsigned short istagged;
	int packet_len;
}packet_sync_ctrl;

typedef union _packet_txrx_ctrl_u_
{
    char reserved[NPD_PKT_RESERVED_LEN];
    packet_sync_ctrl sync_ctrl;
}packet_txrx_ctrl;


#define CWSetField32(src,start,len,val)	 src |= ((~(0xFFFFFFFF << len)) & val) << (32 - start - len)
#define WSMProtocolStore32(msg, val)   bcopy(&(val), (msg), 4)
#define IEEE80211_RETRIEVE(dest, src, offset, len)  memcpy((unsigned char*)(dest),((unsigned char*)(src)+offset), (len))

#define        CW_PROTOCOL_VERSION                                     0

#define        CW_TRANSPORT_HEADER_VERSION_START                       0
#define        CW_TRANSPORT_HEADER_VERSION_LEN                         4

#define        CW_TRANSPORT_HEADER_TYPE_START                          4
#define        CW_TRANSPORT_HEADER_TYPE_LEN                            4

// Radio ID number (for WTPs with multiple radios)
#define        CW_TRANSPORT_HEADER_RID_START                           13
#define        CW_TRANSPORT_HEADER_RID_LEN                             5

// Length of CAPWAP tunnel header in 4 byte words 
#define        CW_TRANSPORT_HEADER_HLEN_START                          8
#define        CW_TRANSPORT_HEADER_HLEN_LEN                            5

// Wireless Binding ID
#define        CW_TRANSPORT_HEADER_WBID_START                          18
#define        CW_TRANSPORT_HEADER_WBID_LEN                            5

// Format of the frame
#define        CW_TRANSPORT_HEADER_T_START                             23
#define        CW_TRANSPORT_HEADER_T_LEN                               1

// Is a fragment?
#define        CW_TRANSPORT_HEADER_F_START                             24
#define        CW_TRANSPORT_HEADER_F_LEN                               1

// Is NOT the last fragment?
#define        CW_TRANSPORT_HEADER_L_START                             25
#define        CW_TRANSPORT_HEADER_L_LEN                               1

// Is the Wireless optional header present?
#define        CW_TRANSPORT_HEADER_W_START                             26
#define        CW_TRANSPORT_HEADER_W_LEN                               1

// Is the Radio MAC Address optional field present?
#define        CW_TRANSPORT_HEADER_M_START                             27
#define        CW_TRANSPORT_HEADER_M_LEN                               1

// Is the message a keep alive?
#define        CW_TRANSPORT_HEADER_K_START                             28
#define        CW_TRANSPORT_HEADER_K_LEN                               1

// Set to 0 in this version of the protocol
#define        CW_TRANSPORT_HEADER_FLAGS_START                         29
#define        CW_TRANSPORT_HEADER_FLAGS_LEN                           3

// ID of the group of fragments
#define        CW_TRANSPORT_HEADER_FRAGMENT_ID_START                   0
#define        CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN                     16

// Position of this fragment in the group 
#define        CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START               16
#define        CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN                 13

// Set to 0 in this version of the protocol
#define        CW_TRANSPORT_HEADER_RESERVED_START                      29
#define        CW_TRANSPORT_HEADER_RESERVED_LEN    3


// Set to 0 in this version of the protocol
#define        CW_TRANSPORT_HEADER_RADIO_MAC_ADDR_PADDING_START        24
#define        CW_TRANSPORT_HEADER_RADIO_MAC_ADDR_PADDING_LEN		   8


#define IEEE8023_DEST_MAC_START                0
#define IEEE8023_SRC_MAC_START                 6
#define IEEE8023_MAC_LEN                               6
#define IEEE8023_TYPE_START                            12
#define IEEE8023_TYPE_LEN                              2

typedef struct {
	char RSSI;
	char SNR;
	int dataRate;
} CWBindingTransportHeaderValues;

typedef struct capwap_data_header_t{
	int payloadType;
	int type;
	int isFragment;
	int last;
	int fragmentID;
	int fragmentOffset;
	int keepAlive;
	CWBindingTransportHeaderValues *bindingValuesPtr;
} CW_DATA_HEADER_T;


struct ieee80211_frame {
       unsigned char i_fc[2];
       unsigned char i_dur[2];
       unsigned char i_addr1[6];
       unsigned char i_addr2[6];
       unsigned char i_addr3[6];
       unsigned char i_seq[2];
};
 
struct ieee80211_llc {
 unsigned char llc_dsap;
 unsigned char llc_ssap;
 unsigned char llc_cmd;
 unsigned char llc_org_code[3];
 unsigned char llc_ether_type[2];
} ;

struct capwap_head {
 unsigned int capwap1;
 unsigned int capwap2;
 unsigned int capwap3;
 unsigned int capwap4;
};

/*communicate with WIFI*/
struct wifi_struct
{
	unsigned int len_addr;
	unsigned int data_addr;
};


#define TX_BUFFER_NUM 1
typedef struct _NAM_PACKET_TX_SESSION_
{
	int sock;
	unsigned char *dma_buff[TX_BUFFER_NUM*2];
	int len[TX_BUFFER_NUM*2];
	int local;
}NAM_PACKET_TX_SESSION;


#endif

