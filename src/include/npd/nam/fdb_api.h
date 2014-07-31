#ifndef __NPD_FDB_API_H__
#define __NPD_FDB_API_H__



/*NPD LAYER API*/
int npd_fdb_table_init();


unsigned int npd_fdb_static_entry_add
(
	unsigned char *mac,
	unsigned short vid,
	unsigned int eth_g_index,
	unsigned int  isMirror
);


unsigned int npd_fdb_static_entry_del
(
	unsigned char *mac,
	unsigned short vid
);

unsigned int npd_fdb_number_port_set
(
    unsigned int eth_g_index,
    unsigned int number
);

unsigned int npd_fdb_number_port_set_check
(
     unsigned int eth_g_index,
     unsigned int * number
);
unsigned int npd_fdb_number_vlan_set
(
    unsigned short vlanId,
    int number
);
unsigned int npd_fdb_number_vlan_set_check 
(
    unsigned short vlanId,
    unsigned int *number
);	

unsigned int npd_fdb_show_number_with_port
(
     unsigned char slotNum,
     unsigned char portNum,
     unsigned int * number
);	


int npd_fdb_set_netif_learn_status
(
    unsigned int eth_g_index,
	unsigned int  status
) ;

unsigned int npd_fdb_static_entry_del_by_port
(
	unsigned int eth_g_index
);

unsigned int npd_fdb_static_entry_del_by_vlan
(
	unsigned short vid
);

unsigned int npd_fdb_static_entry_del_by_vlan_port
(
	unsigned int vlanid,
	unsigned int eth_g_index
);


unsigned int npd_fdb_na_msg_per_vlan_set
(
     unsigned short vlanId,
     unsigned int  status,
     unsigned int  flag
);
unsigned int npd_get_slot_index_from_eth_index
(
    unsigned int eth_index
);
unsigned int npd_fdb_static_blacklist_entry_del_by_vlan
(
     unsigned short vlanId
);

unsigned int npd_fdb_static_blacklist_entry_del
(
	char* mac,
	unsigned short vid,
	unsigned char flag
);

unsigned int npd_fdb_static_mirror_entry_del
(
	unsigned char* mac,
	unsigned short vid
);


unsigned int npd_fdb_add_dldp_vlan_system_mac
(
	unsigned short vlanId
);

unsigned int npd_fdb_del_dldp_vlan_system_mac
(
	unsigned short vlanId
);

unsigned int npd_fdb_static_blacklist_entry_add
(
	unsigned char * mac,
	unsigned short vid,
	unsigned char flag
);

int npd_fdb_entry_del_by_vlan_port
( 
	unsigned vlanid, 
	int ifIndex 
);
int npd_fdb_entry_del_by_port
( 	
	int ifIndex 
);
int npd_fdb_entry_del_by_vlan
( 
	int vlanId 
);
unsigned int npd_fdb_static_authen_entry_del
(
    unsigned char* mac,
    unsigned short vid
);
unsigned int npd_fdb_static_authen_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index
);
unsigned int npd_fdb_check_static_authen_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * eth_g_index
);
unsigned int npd_fdb_dynamic_entry_sticky_by_netif
(
    unsigned int netif_index
);


int npd_fdb_check_contain_mirror
( 
	unsigned int profile 
);
unsigned int npd_fdb_check_static_mirror_entry_exist
(
    unsigned char *mac,
	unsigned short vid,
	unsigned int * profile
);
unsigned int npd_fdb_static_mirror_entry_add
(
	unsigned char *mac,
	unsigned short vid,
	unsigned int eth_g_index,
	unsigned int  profile
);
int npd_fdb_static_mirror_entry_count
(
	unsigned int profile
);
unsigned int npd_fdb_get_static_mirror_item
(
    struct fdb_entry_item_s *static_mirror_array,
    unsigned int   size,
    unsigned int profile
);
int npd_fdb_static_mirror_entry_del_by_profile
( 
	unsigned int profile 
);
unsigned int npd_fdb_check_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * eth_g_index
);

extern unsigned int npd_fdb_dynamic_entry_del_by_vlan
(
    unsigned short vid
);
unsigned int npd_fdb_dynamic_entry_del_by_vlan_port
(
    unsigned int vlanid,
    unsigned int netif_index
);

extern int npd_mirror_fdb_source_port_exist_check
(
	unsigned short valnid, 
	unsigned char *mac, 
	unsigned int eth_g_index, 
	unsigned int profile
);
unsigned int npd_fdb_dynamic_entry_del_by_port
(
    unsigned int netif_index
);

/*NAM LAYER API*/
#define FDB_CONFIG_FAIL		1
#define FDB_CONFIG_SUCCESS	0

/* Flags for L2 learn limit. */
#define NAM_L2_LEARN_LIMIT_SYSTEM           0x00000001 /* Limit is system wide. */
#define NAM_L2_LEARN_LIMIT_VLAN             0x00000002 /* Limit is on per VLAN
                                                          basis. */
#define NAM_L2_LEARN_LIMIT_PORT             0x00000004 /* Limit is on per port
                                                          basis. */
#define NAM_L2_LEARN_LIMIT_TRUNK            0x00000008 /* Limit is on per trunk
                                                          basis. */
#define NAM_L2_LEARN_LIMIT_ACTION_DROP      0x00000010 /* Drop if over limit. */
#define NAM_L2_LEARN_LIMIT_ACTION_CPU       0x00000020 /* Send to CPU if over
                                                          limit. */
#define NAM_L2_LEARN_LIMIT_ACTION_PREFER    0x00000040 /* Use system drop/CPU if
                                                          over both system limit
                                                          and non-system limit,
                                                          used by system wide
                                                          setting only. */

typedef struct nam_learn_limit_s {
 unsigned int  flags;		 /* NAM_L2_LEARN_LIMIT_xxx actions and qualifiers. */
 unsigned short vlan;	 /* VLAN identifier. */
 int port;	 /* Port number. */
 int trunk;  /* Trunk identifier. */
 int limit; 		 /* Maximum number of learned entries, -1 for unlimited. */
} nam_learn_limit_t;

typedef struct nam_l2_addr_s{
    int hit;
    unsigned char mac[6];
    unsigned short vid;
    unsigned int if_index;
    unsigned int is_static;
}nam_l2_addr_t;


struct npd_asd_ufdb_s
{
	unsigned char mac[MAC_ADDR_LEN];
	unsigned int link_state;
	unsigned int netif_index;
	unsigned int pvid_when_add;
};



int nam_fdb_table_agingtime_set(unsigned int timeout);

 int nam_fdb_table_agingtime_get(unsigned int*	timeout);
 int nam_static_fdb_entry_mac_vlan_port_set
 (
	 unsigned char macAddr[],
	 unsigned short vlanId,
	 unsigned int globle_index
 ) ;

 int nam_no_static_fdb_entry_mac_vlan_set
(
	unsigned char * macAddr,
	unsigned short vlanId,
	unsigned int netif_index
) ;
unsigned int nam_static_fdb_entry_mac_vlan_trunk_set
(
	 unsigned char macAddr[],
	 unsigned short vlanId,
	 unsigned short trunkId
);

 int nam_fdb_entry_mac_vlan_drop
(
	unsigned char  macAddr[],
	unsigned short vlanId,
	unsigned int flag,
	unsigned char isStatic
);


 int nam_fdb_entry_mac_vlan_no_drop
(
	unsigned char macAddr[],
	unsigned short vlanId,
	unsigned int  flag 
);

unsigned int  nam_asic_fdb_aa_msg_enable(unsigned int dev,unsigned int status);

unsigned int nam_show_fdb_vlan(NPD_FDB *fdb , unsigned int start_id, unsigned int size,unsigned short vlan );
unsigned int nam_show_fdb_count();
unsigned int nam_show_fdb (NPD_FDB *fdb,unsigned int start_id, unsigned int size);
unsigned int nam_show_single_unit_fdb(NPD_FDB *fdb,unsigned int unit, unsigned int size);

unsigned int nam_show_dynamic_fdb (NPD_FDB *fdb, unsigned int start_id, unsigned int size);

unsigned int nam_show_static_fdb (NPD_FDB *fdb,unsigned int size);
unsigned int nam_fdb_port_learn_status_set(unsigned int eth_g_index, unsigned int status);
unsigned int nam_fdb_na_msg_vlan_set(unsigned short vlanId,unsigned int status);
unsigned int nam_fdb_na_msg_l3portvlan_set(unsigned short vlanId,   unsigned int status);
unsigned int nam_fdb_table_delete_entry_with_trunk(unsigned short trunk_no);


unsigned int nam_fdb_static_table_delete_entry_with_vlan
(
	 unsigned short vlanId
);
unsigned int nam_fdb_static_table_delete_entry_with_port
(
	 unsigned int eth_g_index
);

int nam_fdb_table_delete_entry_with_port(unsigned int index);

int nam_static_fdb_entry_mac_set_for_vrrp
(
	unsigned short vlanId
) ;
int nam_static_fdb_entry_mac_del_for_vrrp
(
    unsigned short vlanId
) ;


int nam_fdb_limit4vlan(
    unsigned short vid,
    int fdb_limit
    );
 int nam_static_fdb_entry_mac_set_for_l3
(
	/*unsigned char devNum,*/
	unsigned char macAddr[],
	unsigned short vlanId,
	unsigned int globle_index
) ;
int nam_fdb_set_port_status
(
	unsigned char	devNum,
	unsigned char	portNum,
	unsigned long	portAttr
);
unsigned int nam_del_fdb_port
(
	unsigned int eth_g_index
);

unsigned int nam_del_fdb_vlan
(
	unsigned short vlanid
);

unsigned int nam_del_fdb_vlan_port
(
	unsigned short vlanid,
	unsigned int  eth_g_index
);

/* external variables reference*/
extern unsigned int g_cpu_port_index;
extern unsigned int g_spi_port_index;



extern unsigned int npd_vlan_member_port_index_get
(
	unsigned short	vlanId,
	unsigned char	isTagged,
	unsigned int	eth_g_index[],
	unsigned int	*mbr_count
);


int nam_static_fdb_entry_mac_set_for_vrrp
(
	unsigned short vlanId
);


int nam_static_fdb_entry_mac_del_for_vrrp
(
    unsigned short vlanId
);

int nam_fdb_limit_set
(
    unsigned char     unit,
    nam_learn_limit_t limit
);

int nam_fdb_limit4vlan(
    unsigned short vid,
    int fdb_limit
    );

unsigned int nam_fdb_port_learn_status_set
(	
	unsigned int eth_g_index, 
	unsigned int status
);

unsigned int nam_fdb_na_msg_per_port_set
(
    unsigned char devNum,
    unsigned char port,
    unsigned int status
);
unsigned int nam_fdb_na_msg_vlan_set
(
   unsigned short vlanId,
   unsigned int status
);
unsigned int nam_show_single_unit_fdb 
(
	NPD_FDB *fdb,
	unsigned int unit,
	unsigned int size
);

int nam_fdb_au_rate_set
(
    unsigned char  devNum,
    unsigned int   rate,
    unsigned int enable
);

unsigned int nam_fdb_table_delete_entry_with_vlan_port
(
	unsigned int vlanid, 
	unsigned int index
);

int nam_fdb_table_delete_entry_with_vlan_trunk
(
	unsigned int vlanid, 
	unsigned int trunkid
);
unsigned int nam_show_fdb_port
(
	NPD_FDB *fdb,
	unsigned int start_id,
	unsigned int size,
	unsigned int in_eth_g_index
);
unsigned int nam_show_fdb_mac (NPD_FDB *fdb,unsigned char macAddr[6],unsigned int size);
unsigned int nam_show_fdb_one (NPD_FDB* fdb,unsigned char macAddr[6],unsigned short	vlanId);
int nam_fdb_static_system_source_mac_del
(
	unsigned short vlanId,
	unsigned int   flag
);
int nam_fdb_static_system_source_mac_add
(
	unsigned short vlanId,
	unsigned int   flag
);
int nam_fdb_table_delete_entry_with_vlan(unsigned int vlanid);
unsigned int nam_fdb_app_entry_add
(
    unsigned short vlanid,
    ETHERADDR *macAddr,
    unsigned int eth_g_index,
    unsigned int flag
);

unsigned int nam_fdb_app_entry_del
(
    unsigned short vlanid,
    ETHERADDR *macAddr,
    unsigned int flag
);


extern int nam_l2_addr_get
(
    unsigned char mac_addr[], 
    unsigned short vid, 
    nam_l2_addr_t *l2addr
);

extern int nam_fdb_entry_mac_vlan_port_set
(
        unsigned char macAddr[],
        unsigned short vlanId,
        unsigned int globle_index
);

extern int nam_fdb_entry_mac_vlan_port_delete
(
	unsigned char macAddr[],
	unsigned short vlanId,
	unsigned int globle_index
);
int nam_static_fdb_entry_indirect_set_for_igmp
(
    unsigned int dip,
    unsigned short vidx,
    unsigned short vlanId,
    unsigned int isRoute
);
int nam_static_fdb_entry_indirect_set_for_mld
(
    ip6_addr dip,
    unsigned short vidx,
    unsigned short vlanId,
    unsigned int isRoute
);
int nam_static_fdb_entry_indirect_delete_for_igmp
(
	unsigned int dip,
	unsigned short vid
); 
int nam_static_fdb_entry_indirect_delete_for_mld
(
    ip6_addr dip,
    unsigned short vid
);
unsigned int nam_asic_l2mc_member_del(
	unsigned int eth_g_index,
	unsigned short vidx,
	unsigned int group_ip,
	unsigned short vlanId
);

unsigned int nam_asic_l2mc_group_del(
	unsigned char devNum,
	unsigned short vidx,
	unsigned int   group_ip,
	unsigned short vlanId
);
int nam_fdb_swshadow(int enable);

#endif

