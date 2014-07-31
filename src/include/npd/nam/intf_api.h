#ifndef __NPD_INTF_API_H__
#define __NPD_INTF_API_H__

/*NPD LAYER API*/
void npd_intf_init();

unsigned int npd_intf_l3_mac_legality_check
(
    unsigned char * ethmac
);

int npd_intf_create_vlan_l3intf
(
	unsigned short 		vid,
	char* 	name,
	unsigned char* 	addr
);

 int npd_intf_del_by_ifindex
(
	unsigned int ifindex
);


 int npd_intf_config_basemac
(
	unsigned int netif_index,
	unsigned char* mac
);


int npd_intf_get_intf_mac
(
	unsigned int ifIndex,
	unsigned char* addr
);

unsigned int npd_intf_get_l3intf_status
(
    unsigned int netif_index
);

unsigned int npd_intf_get_l3intf_status_by_ifindex
(
    unsigned int ifindex
);

int npd_intf_port_check
(
	unsigned short vid,
	unsigned int eth_g_index,
	unsigned int* ifindex
);

int npd_intf_check_by_index
(
	unsigned int eth_g_index,
	unsigned int* isIfindex
);

unsigned int npd_intf_get_netif_mac
(
	unsigned int netif_Index,
	unsigned char *macAddr
);

unsigned int npd_intf_set_netif_mac
(
	unsigned int netif_Index,
	unsigned char *macAddr
);

unsigned int npd_intf_udp_bc_trap_enable
(
	unsigned int vlanId,
	unsigned int enable
);

int npd_netif_check_exist(unsigned int netif_index);
int npd_intf_exist_check(unsigned int netif_index, unsigned int *ifindex);

int npd_netif_get_status(unsigned int netif_index);
int npd_intf_netif_get_by_ifindex
(
    unsigned int ifindex, 
    unsigned int *netif_index
);

int npd_intf_name_get_by_ifindex(unsigned int ifindex, char *ifname);

int npd_intf_vid_get_by_ip
(
    unsigned int ipAddr, 
    unsigned int netif_index, 
    unsigned short *vid
);
unsigned int npd_intf_get_vid
(
    unsigned int local_ifindex,
    unsigned short *vid
);

int npd_intf_get_global_l3index
(
    unsigned int l3_local_id,
    unsigned int *l3_g_id
);

int npd_l3_intf_create(unsigned int netif_index, unsigned int *ifindex);

int npd_intf_gindex_exist_check(unsigned int netif_index, unsigned int *ifindex);
unsigned int npd_l3_intf_addr_filter_by_ip(	void *data1, void *data2);
unsigned int npd_l3_intf_addr_filter_by_net(	void *data1, void *data2);
unsigned int npd_l3_intf_addr_filter_by_index( void *data1, void *data2);
int npd_intf_addr_ip_get( unsigned int ifindex, unsigned int *ipAddr, unsigned int *mask);
int npd_intf_netif_get_by_ip(unsigned int *ifindex, unsigned int ipAddr);

int npd_intf_del_ip_addr
(
    unsigned int ifindex,
    unsigned int ipv4,
    unsigned int ipv4_mask
);

int npd_intf_add_ip_addr
(
    unsigned int ifindex,
    unsigned int ipv4,
    unsigned int ipv4_mask
);
int npd_intf_del_all_ip_addr(unsigned int ifindex);
int npd_intf_portal_set(char isEnable, int netif_index);
int npd_intf_get_l3intf_ctrl_status
(
    unsigned int ifindex,
    unsigned int *state
);

int npd_intf_vlan_add_eth_hw_handler
(
	unsigned short vid, 
	unsigned int eth_g_index
);
int npd_intf_trunk_add_eth_hw_handler
(
	unsigned short tid, 
	unsigned int eth_g_index
);
int npd_intf_vlan_del_eth_hw_handler
(
	unsigned short vid, 
	unsigned int eth_g_index
);
int npd_intf_trunk_del_eth_hw_handler
(
	unsigned short tid, 
	unsigned int eth_g_index
);
int npd_intf_vlan_add_trunk_hw_handler
(
	unsigned short vid, 
	unsigned int tid
);
int npd_intf_vlan_del_trunk_hw_handler
(
	unsigned short vid, 
	unsigned int tid
);
int npd_l3_intf_delete(unsigned int netif_index);
int npd_intf_addr_ifindex_get( unsigned int *ifindex, unsigned int *intfCnt, unsigned int ipAddr);

unsigned int npd_intf_ifindex_get_by_ifname
(
    unsigned char *ifName,
    unsigned int *ifIndex
);

int npd_intf_table_is_full();
int npd_intf_addr_ifindex_get_bynet( 
    unsigned int *ifindex, 
    unsigned int *intfCnt, 
    unsigned int ipAddr);
unsigned int npd_l3_local_cmp_ifindex(void *if_struct_a, void *if_struct_b);
int npd_intf_get_proxy_arp(
    unsigned int netif_index,
    unsigned char* proxy_arp
);
unsigned int npd_l3_intf_cmp_ifindex(void *if_struct_a, void *if_struct_b);
int npd_intf_attribute_get_by_ifindex(unsigned int ifindex, unsigned int *attribute);
int npd_intf_ip_get_by_netif(unsigned int *ipAddr, unsigned int netif_index);

int npd_intf_netif_get_by_name(unsigned int *netif_index, unsigned char *ifName);

void npd_tracking_init(void) ;
int npd_ipv6_intf_vid_get_by_ip
(
    ip6_addr *ipAddr, 
    unsigned int netif_index, 
    unsigned short *vid
);
int npd_ipv6_intf_addr_ip_get( unsigned int ifindex, ip6_addr *ipAddr, ip6_addr *mask);


#ifdef HAVE_NPD_IPV6 
int npd_intf_add_ipv6_addr
(
    unsigned int ifindex,
    ip6_addr *ipv6,
    ip6_addr *ipv6_mask
);
int npd_intf_del_ipv6_addr
(
    unsigned int ifindex,
    ip6_addr *ipv6,
    ip6_addr *v6_mask
);
unsigned int npd_ipv6_l3_intf_addr_filter_by_net
(
    void *data1, 
    void *data2
);
unsigned int npd_v6_l3_intf_addr_filter_by_index
(
    void *data1, 
    void *data2
);
#endif //HAVE_NPD_IPV6

/*NAM LAYER API*/
extern int nam_vlan_unreg_filter
(
	unsigned char devNum,
	unsigned short vlanId,
	unsigned int packetType,
	unsigned int cmd
);

extern int nam_arp_trap_enable
(
    unsigned int netif_index,
	unsigned int enable
);
unsigned int nam_asic_udp_bc_trap_en(unsigned short vid,unsigned int enable);
extern int nam_intf_tbl_index_alloc(unsigned int *index);
extern int nam_intf_tbl_index_get(unsigned int index);
extern int nam_intf_tbl_index_free(unsigned int index);
extern int nam_intf_tbl_index_init();
int nam_intf_addr_create
(
    char *name, 
    unsigned int ifindex, 
    unsigned int netif_index, 
    unsigned int ipAddr, 
    unsigned int mask
);

int nam_intf_addr_delete
(
    char *name, 
    unsigned int ifindex, 
    unsigned int netif_index, 
    unsigned int ipAddr, 
    unsigned int mask
);
int nam_intf_addr_link_change
(
    char *name, 
    unsigned int netif_index, 
    unsigned int linkstate
);

int nam_intf_addr_bind_slot
(
    char *name, 
    unsigned int netif_index, 
    unsigned int slotid,
    unsigned char enable
);


int nam_intf_addr_flow_recovery
(
    char *name, 
    unsigned int netif_index, 
    unsigned int isRecover
);
int nam_intf_vlan_enable
(
    unsigned int ifindex, 
    unsigned short vid, 
    unsigned int enable, 
    unsigned char* addr
);

int nam_intf_vlan_port_enable
(
    unsigned int ifindex, 
    unsigned short vid, 
    unsigned int netif_index, 
    unsigned int enable, 
    unsigned char* addr
);

int nam_subintf_vlan_port_enable
(
    unsigned int ifindex, 
    unsigned short vid, 
    unsigned int netif_index, 
    unsigned int enable, 
    unsigned char* addr
);
int nam_intf_config_basemac
(
	unsigned int netif_index,
	unsigned char* mac
);
int nam_intf_ipmc_enable(unsigned int ifindex, int vid, int enable);

int npd_intf_get_info
(
   char * name,
   unsigned int * ifIndex,
   unsigned short * vid,
   unsigned int * eth_g_index
);

int npd_vrrp_intf_vmac_check(unsigned int ifindex, unsigned char *vmac_addr);

int npd_intf_set_l3intf_ctrl_status
(
    unsigned int ifindex,
    enum INTF_CTRL_STATUS_E event
);
int nam_get_trunk_by_peer_slot(unsigned int slot_id, unsigned char unit, unsigned int *trunkId);
int nam_intf_fw_group_change(unsigned old_slot_arr[], unsigned int slot_arr[], unsigned int ext_slot_arr[]);
int nam_get_trunk_by_fw_group(unsigned char unit, unsigned int *trunkId);

#endif
