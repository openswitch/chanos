#ifndef __COMMON_ARPSNOOP_H__
#define __COMMON_ARPSNOOP_H__

enum NPD_ARP_INSPECTION_VALIDATE {
	ARP_INSPECTION_INVALID = 0,
	ARP_INSPECTION_SRC_MAC,
	ARP_INSPECTION_DST_MAC,
	ARP_INSPECTION_IP,
	ARP_INSPECTION_IP_ZERO
};
enum NPD_PACKET_INTF_SRC_TYPE {
	ARP_INTF_SRC_ETHPORT_E = 0,
	ARP_INTF_SRC_TRUNK_E,
	ARP_INTF_SRC_VIDX_E,
	ARP_INTF_SRC_VID_E,
	ARP_INTF_SRC_MAX
};

#define ARPSNP_FLAG_NONE    0
#define ARPSNP_FLAG_HIT     0x1
#define ARPSNP_FLAG_DROP    0x2
#define ARPSNP_FLAG_DISCARD 0x4
#define ARPSNP_FLAG_ALL     0xFF

enum NPD_ARPSNOOPING_DB_ACTION {
	ARP_SNOOPING_ADD_ITEM = 0,
	ARP_SNOOPING_DEL_ITEM,
	ARP_SNOOPING_UPDATE_ITEM,
	ARP_SNOOPING_ACION_MAX
};

enum NPD_NEXTHOP_DB_ACTION {
	NEXTHOP_ADD_ITEM = 0,
	NEXTHOP_DEL_ITEM,
	NEXTHOP_UPDATE_ITEM,
	NEXTHOP_ACTION_MAX
};

struct arp_snooping_item_s {
	unsigned int	ifIndex;
	unsigned int    l3Index;
	unsigned char 	mac[MAC_ADDRESS_LEN];
	unsigned int	ipAddr;
	unsigned char	isTagged;
	unsigned short	vid;
	unsigned short	vidx;
	unsigned char	isStatic;
	unsigned char   isValid;  /* only used for static arp items.*/
	unsigned int    flag;
    unsigned int    time;
};

struct npd_arpsnp_cfg_s
{
	unsigned int timeout;   /* ARP entry ageout time. */
	unsigned int arp_drop_enable;   /* ARP drop entry enable */
};
/**
 *
 *	Next-Hop Table info (NHT)
 *	here use tblIndex to hold Next-Hop Table index 
 *
 */
struct route_nexthop_brief_s {
	unsigned int	l3Index;  	/* L3 interface index*/
	unsigned int	ipAddr;		/* ip address associated with L3 interface*/
	unsigned int    srcIp;
	unsigned int	tblIndex;	/* Next-Hop Table index to hold Next-Hop detail info*/
	unsigned int	rtUsedCnt;	/* counter for this Next-Hop used by Route entry*/
};
#endif

