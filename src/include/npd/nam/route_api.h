#ifndef __NPD_ROUTE_API_H__
#define __NPD_ROUTE_API_H__

/*NPD LAYER API*/
int npd_route_init();

void npd_mroute_lock();
void npd_mroute_unlock();
int npd_mroute_nexthop_check_free(struct npd_mroute_item_s *mroute);

int npd_route_update_by_nhp(unsigned int ipAddr, int valid);
#ifdef HAVE_NPD_IPV6	
int npd_route_v6_update_by_nhp(ip6_addr ipAddr, int valid);
#endif //HAVE_NPD_IPV6

int npd_mroute_pim_mode_set(int mode);
int npd_mroute_nexthop_alloc(struct npd_mroute_item_s *mroute);
int npd_route_get_egrintf_by_dip(unsigned int dip, unsigned char *mac, unsigned int *netif_index, unsigned int *vlan_id);
int npd_route_next_hop_get_by_network(unsigned int ipAddr, int masklen, unsigned int nh[]);
unsigned int npd_route_nexthop_compare(void *data1, void *data2);
int  npd_route6_update_tunnel(hash_table_index_t* hash, void* data, unsigned int flag);
void npd_route6_update_by_tunnel(unsigned int ifindex, int cmd);
unsigned int get_NextHop_TBL_Entry(unsigned int ifIndex,unsigned int nextHopIp);
unsigned int get_ipv6_NextHop_TBL_Entry(unsigned int ifIndex, ip6_addr nextHopIp);
void npd_mroute_ipmap2mac(unsigned int family, unsigned char *dmip, char *mc_addr);
unsigned int npd_mroute_compar_ip(void *data1,	void *data2);
unsigned int npd_mroute_compar_ip_vlan(void *data1, void *data2);



/*NAM LAYER API*/


#define NAM_ROUTE_VALID 0x1
#define NAM_ROUTE_PCL_ID 0
#define NAM_ROUTE_ECMP  0x2
#define NAM_ROUTE_URPF  0x4
#define NAM_ROUTE_MCAST 0x8
#define NAM_ROUTE_MCAST_CPU 0x10
#define NAM_ROUTE_NH_TUNNEL 0x20

int nam_mroute_hit_getandclear(void *data, int *flag);
int nam_route_set_ucrpf_enable
(
    unsigned int ifindex,
    unsigned int rfpen
);

unsigned int  nam_set_host_route
(
	unsigned int DIP,
	unsigned int ifindex
) ;
unsigned int  nam_del_host_route
(
	unsigned int DIP,
	unsigned int ifindex
) ;
unsigned int lpm_entry_search(unsigned int DIP,unsigned int masklen,unsigned int* entryIndex);
void  nam_route_count_get
(
    unsigned int *route_count
);
int nam_route_init();
int  nam_update_mroute_info
(
    struct npd_mroute_item_s *new,
    struct npd_mroute_item_s *old
);

int  nam_add_mroute_info
(
    struct npd_mroute_item_s *item
);

int  nam_del_mroute_info
(
    struct npd_mroute_item_s *item
);

int nam_del_ipv6_route_info
(
   	ip6_addr dip,
	ip6_addr nexthop,
	unsigned int masklen,
	unsigned int ifindex,
    unsigned int rt_type,
    unsigned int flag
);

unsigned int  nam_set_ipv6_host_route
(
	ip6_addr DIP,
	unsigned int ifindex
);

unsigned int  nam_del_ipv6_host_route
(
	ip6_addr	DIP,
	unsigned int ifindex
) ;
int nam_set_route_v6_info
(
	unsigned int flag,
    ip6_addr dip,
	ip6_addr nexthop,
	unsigned int masklen,
	unsigned int ifindex,
	unsigned int routeEntryBaseIndex
);

int  nam_set_route_v6
(
    ip6_addr DIP,
    ip6_addr nexthop,
    unsigned int masklen,
    unsigned int ifindex,
    unsigned int rt_type,
    unsigned int flag
);

int nam_del_route_v6_info
(
    ip6_addr dip,
	ip6_addr nexthop,
	unsigned int masklen,
	unsigned int ifindex,
    unsigned int rt_type,
    unsigned int flag
);
int nam_del_route_info
(
    unsigned int dip,
	unsigned int nexthop,
	unsigned int masklen,
	unsigned int ifindex,
    unsigned int rt_type,
    unsigned int flag
);
int nam_set_route_info
(
	unsigned int flag,
    unsigned int dip,
	unsigned int nexthop,
	unsigned int masklen,
	unsigned int ifindex,
	unsigned int routeEntryBaseIndex
);
int  set_LPM_TBL_Entry
(
    unsigned int DIP,
    unsigned int nexthop,
    unsigned int masklen,
    unsigned int ifindex,
    unsigned int rt_type,
    unsigned int flag
);


#endif

