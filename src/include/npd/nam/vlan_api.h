#ifndef __NPD_VLAN_API_H__
#define __NPD_VLAN_API_H__


extern sequence_table_index_t *g_vlans;
extern sequence_table_index_t *pvlan_primary_vid_sequence;
extern sequence_table_index_t *pvlan_isolate_vid_sequence;

void npd_init_vlan(void) ;

unsigned int npd_check_vlan_exist
(
	unsigned short vlanId
);

int npd_check_vlan_status
(
	unsigned short vlanId
);

struct vlan_s *npd_find_vlan_by_vid
(
	unsigned short vlanId
);

struct vlan_s* npd_find_vlan_by_name
(
	char *name
);

struct vlan_s* npd_create_vlan_by_vid
(
	unsigned short vlanId
);

int npd_delete_vlan_by_vid
(
	unsigned short vlanId
);

unsigned int npd_vlan_check_port_membership
(
	unsigned short vlanId,
	unsigned int   eth_g_index,
	unsigned char  isTagged
);

unsigned int npd_vlan_member_port_index_get
(
	unsigned short	vlanId,
	unsigned char	isTagged,
	unsigned int	eth_g_index[],
	unsigned int	*mbr_count
);

unsigned int npd_vlan_check_contain_port
(
	unsigned short vlanId,
	unsigned int   eth_g_index,
	unsigned char  *isTagged
);


/*
* make untag port free from the vlan.
*/

unsigned int npd_vlan_set_port_pvid
(
	unsigned int eth_g_index,
	unsigned short	pvid
);
unsigned int npd_vlan_port_pvid_get
(
	unsigned int	eth_g_index,
	unsigned short	*pvid
);

unsigned int npd_vlan_interface_check
(
	unsigned short vlanId,
	unsigned int   *ifIndex
);

unsigned int npd_vlan_activate
(
	unsigned short vlanId,
	char *name
);

void npd_save_vlan_cfg
(
	char* buf,
	int bufLen
);

unsigned int npd_vlan_check_contain_trunk
(
	unsigned short vlanId,
	unsigned short trunkId,
	unsigned char  *isTagged
);
int npd_igmp_port_link_change
(
	unsigned int eth_g_index,	
	enum PORT_NOTIFIER_ENT event
);


unsigned int npd_check_vlan_real_exist
(
	unsigned short vlanId
);


int npd_netif_free_vlan
(
    unsigned int netif_index,
    unsigned int vid,
    int isTagged
);

int npd_netif_free_allvlan(
    unsigned int netif_index
    );
int npd_netif_free_alluntag_vlan(
    unsigned int netif_index
    );


int npd_netif_allow_vlan(
    unsigned int netif_index,
    unsigned int vid,
    int isTaged,
    int pvid_set
    );

unsigned int vlan_assoc_mac_show_filter(
	void *in, 
	void* out
	);
unsigned int vlan_assoc_subnet_show_filter(
	void *in, 
	void* out
	);
unsigned int vlan_assoc_protocol_group_show_filter(
	void *in, 
	void* out
	);
unsigned int vlan_assoc_protocol_vlanid_show_filter(
	void *in, 
	void* out
	);
unsigned int vlan_assoc_protocol_netif_show_filter(
	void *in, 
	void* out
	);
int npd_vlan_assoc_mac_set(unsigned char isAdd, unsigned char *mac, unsigned short cur_vid);

unsigned int vlan_xlate_netif_show_filter(
	void *in, void* out
	);

unsigned int npd_vlan_eline_outervlan_filter(
	void *in, 
	void* out
	);
unsigned int npd_vlan_eline_netif_filter(
	void *in, 
	void* out
	);
unsigned int npd_vlan_eline_netif_outer_filter(
	void *in,
	void *out
	);

int npd_vlan_is_port_in_privlan(
	unsigned int netif_index
	);

int npd_netif_free_all_xlate(
	unsigned int netif_index
	);
unsigned int npd_vlan_check_contain_netif(
	unsigned short vlanId,
	unsigned int netif_index,
	unsigned char  *isTagged
	);

unsigned short npd_vlan_get_id_by_name( char *name);
int npd_put_vlan(struct vlan_s *vlan);
unsigned int npd_pvlan_netif_type_get(
	unsigned int netif_index,
	unsigned int* pvlan_port_type
	);


void npd_key_database_lock();
void npd_key_database_unlock();
void npd_gvrp_init();


/*NAM LAYER API*/
unsigned int nam_asic_vlan_entry_active
(
	unsigned int product_id,
	unsigned short vlanId
);

unsigned int nam_asic_vlan_entry_ports_add
(
	unsigned int eth_g_index,
	unsigned short vlanId,
	unsigned char isTagged
); 

unsigned  int nam_asic_vlan_entry_ports_del
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned char  needJoin
);


unsigned int nam_asic_set_port_vlan_ingresflt
(	
	unsigned int eth_g_index,
	unsigned char enDis
);
unsigned int nam_asic_set_port_pvid
(
	unsigned int eth_g_index,
	unsigned short pvid
);
unsigned int nam_asic_get_port_pvid
(
	unsigned int eth_g_index,
	unsigned short* pvid
);

unsigned int nam_asic_set_port_inner_pvid
(
	unsigned int eth_g_index,
	unsigned short pvid
);
unsigned int nam_asic_get_port_inner_pvid
(
	unsigned int eth_g_index,
	unsigned short* pvid
);

unsigned int nam_asic_vlan_entry_delete
(
	unsigned short vlanId
);
unsigned int nam_vlan_update_vid
(
	unsigned int product_id,
	unsigned short vid_old,
	unsigned short vid_new
);
unsigned int nam_asic_vlan_port_route_en
(
	unsigned char devNum,
	unsigned char portNum
);
unsigned int nam_asic_vlan_port_route_dis
(
	unsigned char devNum,
	unsigned char portNum
);
unsigned int nam_asic_port_l3intf_vlan_entry_set
(
	unsigned int product_id,
	unsigned short vlanId
);

unsigned int nam_asic_vlan_get_port_members
(
	unsigned short	vlanId,
	unsigned int	*untagmbrIndxArr,
	unsigned int	*tagmbrIndxArr
	
);

unsigned int nam_vlan_na_msg_enable
(
    unsigned char devNum,
    unsigned short vlanId,
    unsigned int  enable
);


unsigned int nam_set_igmp_enable
(
	unsigned short vlanId,
	unsigned long  enable
);

unsigned int nam_asic_igmp_trap_set_by_vid
(
    unsigned char   devNum,
    unsigned short  vlanId,
    unsigned long 	enable
);

unsigned int nam_asic_igmp_trap_set_by_devport
(
	unsigned short	vid,
	unsigned int eth_g_index,
    unsigned long 	enable
);

unsigned int nam_set_mld_enable
(
	unsigned short vlanId,
	unsigned long  enable
);

unsigned int nam_asic_vlan_ipv4_mc_enable
(
	unsigned char devNum,	
	unsigned short vlanId,	
	unsigned long enable
);

unsigned int nam_asic_vlan_igmp_enable_drop_unmc
(
	unsigned char devNum,
	unsigned short vlanId
);

unsigned int nam_asic_vlan_igmp_enable_forword_unmc
( 
	unsigned char devNum,
	unsigned short vlanId
);

unsigned int nam_asic_vlan_ipv4_mcmode_set
(
	unsigned char devNum,
	unsigned short vlanId
);
unsigned int nam_asic_l2mc_member_del
(
	unsigned int eth_g_index,
	unsigned short vidx,
	unsigned int group_ip,
	unsigned short vlanId
);
unsigned int nam_asic_l2mc_member_add
(
	unsigned int eth_g_index,
	unsigned short vidx,
	unsigned int group_ip,
	unsigned short vlanId
);
unsigned int nam_asic_l2mc_group_del
(
	unsigned char devNum,
	unsigned short vidx,
	unsigned int   group_ip,
	unsigned short vlanId
);
unsigned int nam_asic_l2mc_v6member_add
(
	unsigned int eth_g_index,
	unsigned short vidx,
	ip6_addr group_ip,
	unsigned short vlanId
);
unsigned int nam_asic_l2mc_v6member_del(
	unsigned int eth_g_index,
	unsigned short vidx,
	ip6_addr group_ip,
	unsigned short vlanId
);
unsigned int nam_asic_group_mbr_bmp
(
	unsigned char devNum,
	unsigned short vidx,
	unsigned int *groupMbrBmp
);

unsigned int nam_asic_vlan_autolearn_set
(
    unsigned char devNum,
    unsigned short vlanId,
    unsigned int  autolearn
);

int nam_config_vlan_mtu
(
	unsigned char devNum,
	unsigned int mtuIndex,
	unsigned int mtuValue
);

int nam_config_vlan_egress_filter
(
	unsigned char devNum,
	unsigned int isable
);

int nam_config_vlan_filter
(
	unsigned short vlanId,
	unsigned int filterType,
	unsigned int enDis
);


int nam_config_vlan_egress_filter
(
	unsigned char devNum,
	unsigned int isable
);

int nam_config_vlan_filter(
	unsigned short vlanId,
	unsigned int filterType,
	unsigned int enDis
);

int nam_vlan_subnet_tbl_index_free(unsigned int index);
int nam_vlan_subnet_tbl_index_alloc(unsigned int *index);
int nam_vlan_mac_tbl_index_free(unsigned int index);
int nam_vlan_mac_tbl_index_alloc(unsigned int *index);
unsigned int nam_asic_vlan_entry_delete(unsigned short vlanId) ;

int nam_vlan_private_set
(
	unsigned int vid,
	unsigned int enable
);
int nam_vlan_cross_connect_set(
	unsigned int vid,
	unsigned int type
	);
unsigned int nam_asic_vlan_entry_active
(
	unsigned int product_id,
	unsigned short vlanId
);


unsigned int nam_vlan_init(void);

int nam_vlan_deassoc_subnet(
    unsigned int vid,
    unsigned int ipaddr,
    unsigned int mask,
    unsigned int tbl_index
    );
int nam_vlan_assoc_subnet(
    unsigned int vid,
    unsigned int ipaddr,
    unsigned int mask,
    unsigned int tbl_index
    );
int nam_proto_class_del(
    int proto_group_index,
    unsigned int ether_frame, 
    unsigned short ether_type
    );
int nam_proto_class_add(
     int proto_group_index,
    unsigned int ether_frame, 
    unsigned short ether_type
    );
int nam_proto_vlan_port_del(
    unsigned int netif_index, 
    int proto_group_index,
    unsigned int vid,
    unsigned int ether_frame, 
    unsigned short ether_type
    );
int nam_proto_vlan_port_add(
    unsigned int netif_index, 
    int proto_group_index,
    unsigned int vid,
    unsigned int ether_frame, 
    unsigned short ether_type
    );

int nam_vlan_deassoc_mac(
    unsigned int vid,
    unsigned char mac[6],
    unsigned int tbl_index
    );
int nam_vlan_assoc_mac(
    unsigned int vid,
    unsigned char mac[6],
    unsigned int tbl_index
    );
unsigned int nam_asic_vlan_entry_cpu_del(unsigned short vlanId);

unsigned int nam_asic_igmp_trap_set_by_vid
(
    unsigned char   devNum,
    unsigned short  vlanId,
    unsigned long 	enable
);
;
unsigned int nam_asic_igmp_trap_set_by_devport(
	unsigned short	vid,
	unsigned int eth_g_index,
    unsigned long 	enable
);
;
unsigned int nam_set_mld_enable
(
	unsigned short vlanId,
	unsigned long  enable
);
unsigned int nam_asic_mld_trap_set_by_devport(
	unsigned short	vid,
	unsigned int eth_g_index,
    unsigned long 	enable
);

unsigned int nam_set_UdpBcPkt_enable
(
	unsigned short vlanId,
	unsigned long  enable
);


unsigned int nam_asic_dhcp_trap_set_by_devport(
	unsigned short	vid,
	unsigned int eth_g_index,
    unsigned long 	enable
);


unsigned int nam_asic_set_port_pvid
(
	unsigned int eth_g_index,
	unsigned short pvid
);
unsigned int nam_asic_test_set_port_pvid
(
    unsigned char devNum,
    unsigned char portNum,
	unsigned short pvid
);
unsigned int nam_asic_get_port_pvid
(
	unsigned int eth_g_index,
	unsigned short* pvid
);

unsigned int nam_asic_set_port_inner_pvid
(
	unsigned int eth_g_index,
	unsigned short pvid
);

unsigned int nam_asic_get_port_inner_pvid
(
	unsigned int eth_g_index,
	unsigned short* pvid
);

/*add cpu port to vlan in tagged mode. called when l3 interface being created.*/
unsigned int nam_asic_vlan_entry_cpu_add(unsigned short vlanId);

unsigned int nam_vlan_add_trunk
(
        unsigned short tid,
        unsigned short vlanId,
        unsigned char isTagged
);

unsigned int nam_vlan_del_trunk
(
        unsigned short tid,
        unsigned short vlanId,
        unsigned char isTagged
);

unsigned int nam_asic_l2mc_v6member_add
(
	unsigned int eth_g_index,
	unsigned short vidx,
	ip6_addr group_ip,
	unsigned short vlanId
);

unsigned int nam_asic_l2mc_v6member_del(
	unsigned int eth_g_index,
	unsigned short vidx,
	ip6_addr group_ip,
	unsigned short vlanId
);
unsigned int nam_test_vlan_entry_ports_adddel
(
	unsigned short vlanId,
	unsigned char devNum,
	unsigned char portNum,
	unsigned char  needJoin
);
unsigned int nam_test_vlan_entry_active
(
	unsigned short vlanId
); 


#endif /* _NPD_VLAN_H*/
