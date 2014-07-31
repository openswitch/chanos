#ifndef __NPD_ROUTE_H__
#define __NPD_ROUTE_H__

#ifdef HAVE_NPD_IPV6
#include "npd/ipv6.h"
#endif //HAVE_NPD_IPV6

#define BUFLENGTH (4096)

#define NPD_ROUTE_HASHTBL_NAME   "npdRouteHashTbl"
#define NPD_MROUTE_HASHTBL_NAME  "npdmRotueHashTbl"
#define NPD_ROUTE_CFGTBL_NAME    "npdRouteCfgTbl"

#define NPD_ROUTE_HASH_IP_SIZE (NPD_ROUTE_TABLE_SIZE)
#define NPD_ROUTE_HASH_NHP_SIZE (NPD_ROUTE_TABLE_SIZE)

#define NPD_ROUTE_MAX_NH_NUM   (64)


struct rttbl_info
{
	unsigned char rtm_family;
	unsigned int DIP;	/* destination address in TCAM*/
    unsigned int SIP;
    unsigned int sifindex;
	unsigned int *nexthop;	/* Next-Hop ip address Array*/
#ifdef HAVE_NPD_IPV6	
	ip6_addr DIP6;
    ip6_addr SIP6;
	ip6_addr *nexthop6;
#endif	//HAVE_NPD_IPV6	
	unsigned int *ifindex;   /* Next-Hop interface Array*/
    unsigned int *nexthop_flag;
	unsigned int masklen;	/* destination ip mask in TCAM*/
	unsigned int rt_type;
	unsigned char multipath;
	unsigned int sgateway;
	unsigned int rtm_flag;
};


struct netlink_req{
  struct nlmsghdr nl;
  struct rtmsg    rt;
  char            buf[8192];
} ;

unsigned int npd_route_compare
(
	void *data1,
	void *data2
);

unsigned int npd_route_compare_bynhp
(
	void *data1,
	void *data2
);

unsigned int npd_route_cmp_by_ifindex
(
	void *data1,
	void *data2
);

unsigned int npd_route_cmp_by_nhnetwork
(
	void *data1,
	void *data2
);

void npd_route_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);


void npd_route_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);
unsigned int npd_route_cmp_by_network
(
	void *data1,
	void *data2
);


#endif


