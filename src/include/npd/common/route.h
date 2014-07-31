#ifndef __COMMON_NPD_ROUTE_H__
#define __COMMON_NPD_ROUTE_H__

#define NPD_MAX_MINTF 32


#define NPD_ROUTE_VALID 0x1
#define NPD_ROUTE_ECMP  0x2
#define NPD_ROUTE_URPF  0x4
#define NPD_ROUTE_MCAST 0x8
#define NPD_ROUTE_MCAST_CPU 0x10
#define NPD_ROUTE_NH_TUNNEL 0x20

struct npd_route_cfg_s
{
    int multicast_enable;
    int urpf_strict_enable;
    int ipv6_enable;
    int multicast_pim_mode;
};


struct route_nexthop_hwid_s {
	unsigned int	l3Index;  	/* L3 interface index*/
	unsigned int	ipAddr;		/* ip address associated with L3 interface*/
	unsigned int    srcIp;
	unsigned int	tblIndex;	/* Next-Hop Table index to hold Next-Hop detail info*/
	unsigned int	hit;	/*ipmc table hit flag, need sync from slave to active master*/
#ifdef HAVE_NPD_IPV6
	ip6_addr dipv6;
    ip6_addr sipv6;
#endif	
	unsigned int    l2mc_index;
};

struct npd_route_item_s
{
	unsigned int DIP;	/* destination address in TCAM*/
    unsigned int sip;
    unsigned int sifindex;
	unsigned int nexthop;	/* Next-Hop ip address*/
	unsigned int masklen;	/* destination ip mask in TCAM*/
	unsigned int ifindex;	/* Next-Hop interface*/
	unsigned int tblIndex;
	unsigned int rt_type;
	unsigned char isStatic;
	unsigned int flag;
};

#define RTN_L2MULTICAST      0
#define RTN_L2L3MULTICAST 1000

struct npd_mroute_item_s
{
    unsigned int dip;
    unsigned int sip;
    unsigned int svid;
    unsigned int srcl3_g_index;
    unsigned int srcl3_netif_index;
    unsigned int srcl2_netif_index;
    int dst_vid;
    unsigned int dstl3_g_index;
    unsigned int dstl3_netif_index;
    npd_pbmp_t   dst_l2_ports;
    unsigned int tbl_index;
	unsigned int l2mc_index;
    unsigned int rt_type;
    unsigned int flag;
#ifdef HAVE_NPD_IPV6
    ip6_addr dipv6;
    ip6_addr sipv6;
#endif
	unsigned char family;
	npd_pbmp_t   l2_real_ports;
};

enum
{
    URPF_LOOSE,
    URPF_STRICT_ALL,
    URPF_STRICT_EXCLUDE_DEFAULT
};
#ifdef HAVE_NPD_IPV6	

struct npd_route_item_v6_s
{
	ip6_addr DIP;
    ip6_addr sip;
    unsigned int sifindex;
	ip6_addr nexthop;
	unsigned int masklen;	/* destination ip mask in TCAM*/
	unsigned int ifindex;	/* Next-Hop interface*/
	unsigned int tblIndex;
	unsigned int rt_type;
	unsigned char isStatic;
	unsigned int flag;
};
#endif //HAVE_NPD_IPV6




enum NPD_ROUTE_ACTION {
	ROUTE_ADD_ITEM = 0,
	ROUTE_DEL_ITEM,
	ROUTE_UPDATE_ITEM,
#ifdef HAVE_NPD_IPV6				
	ROUTE_ADD_ITEM_V6,
	ROUTE_DEL_ITEM_V6,
	ROUTE_UPDATE_ITEM_V6,
#endif	//HAVE_NPD_IPV6
	ROUTE_ACTION_MAX
};




#endif

