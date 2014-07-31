#ifndef __COMMON_NDISCSNOOP_H__
#define __COMMON_NDISCSNOOP_H__

#define NPD_NDISC_AGE_AGE_CNT  600


#define NDISCSNP_FLAG_NONE    0
#define NDISCSNP_FLAG_HIT     0x1
#define NDISCSNP_FLAG_DROP    0x2
#define NDISCSNP_FLAG_ALL     0xFF

#define NPD_NDISC_AGE_INTERVAL 5
#define NPD_NDISC_AGE_DROP_CNT 2
#define NPD_NDISC_AGE_CNT  600

#define NPD_NDISC_ITEM_ONCE_HANDLE 100

#define NPD_NDISC_PACKET_NS_LEN  128

enum NPD_NDISC_SNOOPING_DB_ACTION {
	NDISC_SNOOPING_ADD_ITEM = 0,
	NDISC_SNOOPING_DEL_ITEM,
	NDISC_SNOOPING_UPDATE_ITEM,
	NDISC_SNOOPING_ACION_MAX
};


/*
 *	IPv6 address structure
 */

struct ndisc_snooping_item_s {
	unsigned int	ifIndex;
	unsigned int    l3Index;
	unsigned char 	mac[MAC_ADDRESS_LEN];
	ip6_addr		ipAddr;
	unsigned char	isTagged;
	unsigned short	vid;
	unsigned short	vidx;
	unsigned char	isStatic;
	unsigned char   isValid;  /* only used for static neighbour items.*/
	unsigned int    flag;
};
struct npd_ndiscsnp_cfg_s
{
	unsigned int timeout;   /* neighbour entry ageout time. */
};
/**
 *
 *	Next-Hop Table info (NHT)
 *	here use tblIndex to hold Next-Hop Table index 
 *
 */
struct route_ipv6_nexthop_brief_s {
	unsigned int	l3Index;  	/* L3 interface index*/
	ip6_addr		ipAddr;		/* ip address associated with L3 interface*/
	unsigned int	tblIndex;	/* Next-Hop Table index to hold Next-Hop detail info*/
	unsigned int	rtUsedCnt;	/* counter for this Next-Hop used by Route entry*/
};


typedef struct{
	unsigned int   srcType;
	unsigned int   netif_index;
	unsigned int   ifIndex;
	unsigned short vid;
	unsigned char  isTagged;
	ip6_addr sip;
	ip6_addr dip;
}npd_ndisc_ctrl_stc;

struct ipv6_checksum_pseudo
{
	ip6_addr saddr;
	ip6_addr daddr;
	unsigned int len;
	unsigned int nexthdr;
};
#endif

