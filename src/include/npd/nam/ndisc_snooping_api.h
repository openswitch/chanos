#ifndef _NPD_NDISC_SNOOPING_API_H_
#define _NPD_NDISC_SNOOPINT_API_H_

void npd_ndisc_snp_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);

void npd_ndisc_snp_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);


unsigned int npd_ndisc_snooping_compare_byip
(
	void * data1,
	void * data2	
);
int npd_ipv6_route_nexthop_find
(	
	ip6_addr ipAddr,
	struct route_ipv6_nexthop_brief_s *nexthopEntry
);
unsigned int npd_ndisc_snooping_del_by_network(ip6_addr *ipv6_addr,unsigned int ipv6_masklen);
unsigned int npd_ndisc_snooping_del_static_by_network(ip6_addr *ipv6_addr,unsigned int ipv6_masklen);
unsigned int npd_ndisc_snooping_static_valid_set_by_net
(
	ip6_addr *ipaddr,
	int masklen,
	unsigned int isValid
);


unsigned int npd_ipv6_route_nexthop_tblindex_get
(
	unsigned int l3Index,
	ip6_addr ipAddr,
	unsigned int *tblIndex
);
unsigned int npd_ndisc_snooping_compare
(
	void * data1,
	void * data2	
);
unsigned int npd_ndisc_snooping_count_all
(
    void
);
void npd_ndisc_snooping_drop_handle();

void npd_init_neighbour_snooping(void);
int npd_ndisc_snooping_solicit_send
(
	struct ndisc_snooping_item_s *item
);
int npd_ndisc_snooping_valid_check(
	ip6_addr ipv6_addr, 
	unsigned int netif_index, 
	unsigned int *gl3index,
	unsigned short *vlanId
);


/*NAM LAYER API*/

#define NAM_NDISCSNP_FLAG_NONE    0
#define NAM_NDISCSNP_FLAG_HIT     0x1
#define NAM_NDISCSNP_FLAG_DROP    0x2
#define NAM_NDISCSNP_FLAG_ALL     0xFF
int nam_ndisc_snooping_op_item
(
	struct ndisc_snooping_item_s *dbItem,
	unsigned int action,
	unsigned int *tblIndex
);

#endif  


