#ifndef __MAN_INTF_H__
#define __MAN_INTF_H__
#ifdef HAVE_NPD_IPV6
#include "man_ipv6.h"
#endif //HAVE_NPD_IPV6
#define DCLI_SUCCESS 0
#define DCLI_CREATE_ERROR 		1
#define DCLI_INTF_CHECK_IP_ERR  4
#define DCLI_INTF_CHECK_MAC_ERR		5
#define DCLI_VLAN_BADPARAM			12  
#define DCLI_VLAN_NOTEXISTS			15

#define DCLI_INTF_DIS_ROUTING_ERR 19
#define DCLI_INTF_EN_ROUTING_ERR 20
#define DCLI_INTF_EXISTED 21
#define DCLI_INTF_NOTEXISTED 22
#define DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF 23
#define DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF 24
#define DCLI_ONLY_RUN_IN_VLAN 25
#define DCLI_ALREADY_ADVANCED 26
#define DCLI_NOT_ADVANCED 27
#define DCLI_PARENT_INTF_NOT_EXSIT 28
#define DCLI_PROMI_SUBIF_EXIST 29
#define DCLI_PROMI_SUBIF_NOTEXIST 30
#define DCLI_DBUS_PORT_NOT_IN_VLAN 31
#define DCLI_MAC_MATCHED_BASE_MAC 32
#define DCLI_L3_INTF_NOT_ACTIVE 33
#define DCLI_INTF_NO_HAVE_ANY_IP 34
#define DCLI_INTF_HAVE_THE_IP 35
#define DCLI_INTF_NOT_SAME_SUB_NET 36
#define DCLI_INTF_STATUS_CHECK_ERR 37
#define DCLI_INTF_GET_SYSMAC_ERR   38
#define DCLI_INTF_VLAN_CONTAIN_PROMI_PORT  39
#define DCLI_SUBIF_CREATE_FAILED  40
#define DCLI_SUBIF_ADD_PORT_TO_VLAN_FAILED  41
#define DCLI_ARP_SNOOPING_PORT_ADVANCED_ROUTING 42
#define DCLI_ARP_SNOOPING_ADVANCED_ROUTING    43
#define DCLI_INTERFACE_TRUNK_NOT_EXISTS     44
#define DCLI_INTERFACE_TRUNK_NOT_IN_VLAN    45
#define DCLI_INTERFACE_MAC_MATCHED_INTERFACE_MAC  46
#define DCLI_INTERFACE_PORT_NEED_VLAN 			47
#define DCLI_INTERFACE_ARP_DEL_NAM_OP_ERROR   101

#define IP_ARP_INSPECTION_VALIDATE_SMAC_CHECK 1
#define IP_ARP_INSPECTION_VALIDATE_DMAC_CHECK 2
#define IP_ARP_INSPECTION_VALIDATE_IP_CHECK   4



#define DCLI_ARP_SNOOPING_ERR_NONE		0
#define DCLI_ARP_SNOOPING_ERR_STATIC_EXIST (DCLI_ARP_SNOOPING_ERR_NONE + 8)
#define DCLI_ARP_SNOOPING_ERR_STATIC_NOTEXIST	(DCLI_ARP_SNOOPING_ERR_NONE + 9)
#define DCLI_ARP_SNOOPING_ERR_PORT_NOTMATCH     (DCLI_ARP_SNOOPING_ERR_NONE + 10)
#define DCLI_ARP_SNOOPING_ERR_KERN_CREATE_FAILED (DCLI_ARP_SNOOPING_ERR_NONE + 11)
#define DCLI_ARP_SNOOPING_ERR_STATIC_ARP_FULL    (DCLI_ARP_SNOOPING_ERR_NONE + 13)  /* static arp is full (1k)*/
#define DCLI_ARP_SNOOPING_ERR_HASH_OP_FAILED     (DCLI_ARP_SNOOPING_ERR_NONE + 14)  /* hash push or pull failed*/

#define DCLI_ARP_SNOOPING_ERR_MAX			(DCLI_ARP_SNOOPING_ERR_NONE + 255)

#define MIN_VLANID 1
#define MAX_VLANID 4094
#define MAX_L3INTF_VLANID 4095
#define MIN_BONDID       0
#define MAX_BONDID       7
#define MAXLEN_BOND_CMD  128
#define MAXLEN_BOND_NAME 5   /*bond0~bond7*/
#define ARPSNP_FLAG_DROP 0x2
#define MAC_ADDRESS_LEN 6

#define IF_NAMESIZE 20
#define DCLI_SET_FDB_ERR 0xff


#define INTERFACE_NAMSIZ      20

#ifdef HAVE_NPD_IPV6
#define NDISCSNP_FLAG_NONE    0
#define NDISCSNP_FLAG_HIT     0x1
#define NDISCSNP_FLAG_DROP    0x2
#define NDISCSNP_FLAG_ALL     0xFF
#endif //HAVE_NPD_IPV6


char * dcli_error_info_intf(int errorCode);

#include "netif_index.h"
struct arp_info
{
	unsigned int	ipAddr;
	unsigned char 	mac[MAC_ADDRESS_LEN];
	unsigned int	ifindex;
	unsigned char	isStatic;
	unsigned char   isValid;  /* only used for static arp items.*/
	unsigned int    flags;
    unsigned int    time;
};
struct arp_inspection_info
{
	unsigned int flag;
	unsigned char status;
	unsigned int allowzero;
};

#ifdef HAVE_NPD_IPV6
struct neigh_info
{
	man_ip6_addr	ipAddr;
	unsigned int	mask_len;
	unsigned char 	mac[MAC_ADDRESS_LEN];
	unsigned int	ifindex;
	unsigned char	isStatic;
	unsigned char   isValid;  /* only used for static arp items.*/
	unsigned int    flags;
};
#endif //HAVE_NPD_IPV6

struct dcli_l3intf_ctrl
{
	unsigned int ifindex;
	char ifname[INTERFACE_NAMSIZ];
	unsigned int netif_index;
	unsigned int attribute;
	unsigned char mac_addr[6];
	unsigned short vid;
	unsigned int state;
	unsigned int ipv4;
	unsigned int mask;
	unsigned int ctrl_state;
	unsigned char proxy_arp;
	int bind;
	int ipmc;
};

int get_arp_next_info
(
	unsigned char flag,
	unsigned int arp_ifIndex,
	unsigned int arp_ipAddr,
	unsigned int arp_ipMask,
	ETHERADDR arp_macAddr,
	struct arp_info *info,
	unsigned int *count
);
int clear_arp
(
	unsigned int eth_g_index,
	unsigned int ipAddr,
	unsigned int mask,
	ETHERADDR mac,
	unsigned int flag
);
int create_ip_static_arp
(
	unsigned int ipno,
	ETHERADDR *macAddr,
	unsigned int eth_g_index
);
int no_ip_static_arp
(
	unsigned int eth_g_index,
	unsigned int ipno,
	unsigned int ipmask,
	ETHERADDR macAddr,
	unsigned int flag
);
int transform_dyntostatic_arp
(
	unsigned char flag,
	unsigned int arp_ifIndex,
	unsigned int arp_ipAddr,
	unsigned int arp_ipMask,
	ETHERADDR arp_macAddr
);
int arp_timeout_set
(
	unsigned int timeout,
	unsigned int flag
);
int arp_timeout_get
(
	unsigned int *agetime
);
int create_intf
(
	char *ifname,	
	FILE *fp
);
int delete_intf
(
	char *ifname,
	unsigned short vlanID,
	FILE *fp
);
int delete_ip_address
(
	char* ifname, 
	char* ipstring
);

/**for web check vlan intf***********/
int dcli_vlan_intf_check
(
	unsigned short vlan_id
);


#endif

