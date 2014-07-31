#ifndef __COMMON_SWITCH_PORT_H__
#define __COMMON_SWITCH_PORT_H__


enum{
    PORT_SWITCH_PORT,
    PORT_IP_INTF
};

enum{
    SWITCH_PORT_MODE_ACCESS,
    SWITCH_PORT_MODE_TRUNK,
    SWITCH_PORT_MODE_HYBRID
};

enum{ 
   SWITCH_PORT_PRIVLAN_NORMAL = 0,
   SWITCH_PORT_PRIVLAN_PROMI,	   
   SWITCH_PORT_PRIVLAN_ISOLATED,
   SWITCH_PORT_PRIVLAN_COMMUNITY,
};


typedef struct switch_port_db_s{
    unsigned int switch_port_index;
    unsigned char switch_port_name[31];
    int  global_port_ifindex;
    int  fdb_limit;
    npd_vbmp_t allow_tag_vlans;
    npd_vbmp_t allow_untag_vlans;
    int  pvid;
    int inner_pvid;
    int  default_pri;
    int  default_inner_pri;
    int  link_state;
    int stp_flag;
    int vlan_infilter_mode;
    int vlan_efilter_mode;
    int fdb_learning_mode;
    int vlan_access_mode;
    int vlan_private_mode;
    int subnet_vlan_flag;
    int mac_vlan_flag;
    int prefer_subnet;
    int access_qinq;
    int qinq_drop_miss;
    unsigned short tpid;
    unsigned short inner_tpid;
    
    int state;
    int type;	
	int port_isolate;
}switch_port_db_t;


/* Isolate port group */
typedef struct port_isolate_group_s{
    npd_pbmp_t isolate_ports;
    npd_pbmp_t isolate_trunks; /* just for show */
}port_isolate_group_t;

/*port_driver do the hardware configure task related to (eth port/trunk/tunnel port)*/
typedef struct port_driver_s
{
    int type;
    /*switch_port*/
    /*vlan*/
    int (*set_pvid)(unsigned int g_ifindex, int pvid);
    int (*set_inner_pvid)(unsigned int g_ifindex, int pvid);
    int (*allow_vlan)(unsigned int g_ifindex, int vid, int tagmode);
    int (*remove_vlan)(unsigned int g_ifindex, int vid, int tagmode);
    int (*vlan_pri_set)(unsigned int g_ifindex, int vid, int pri, int cfi);
	int (*vlan_mode_set)(unsigned int g_ifindex, int mode);
    /*fdb*/
    int (*fdb_limit_set)(unsigned int g_ifindex, int fdb_limit);
    int (*fdb_limit_set_byvlanport)(unsigned int g_ifindex, int vid, int fdb_limit);
    int (*fdb_learning_mode)(unsigned int g_ifindex, int mode);
    int (*fdb_add)(unsigned char mac[], int vid, unsigned int g_ifindex);
    int (*fdb_delete)(unsigned char mac[], int vid, unsigned int g_ifindex);
    int (*fdb_delete_by_port)(unsigned int g_ifindex);
    int (*fdb_delete_by_vlan_port)(unsigned int g_ifindex, int vid);
    int (*fdb_update)(unsigned int g_ifindex, void *fdb);

    /*encapsalution*/
    int (*encap_set)(unsigned int g_ifindex, int encap);

    /*speed*/
    int (*port_speed)(unsigned int g_ifindex, int *speed);
    int (*port_duplex_mode)(unsigned  int g_ifindex, int *duplex_mode);
    int (*port_link_status)(unsigned int g_ifindex, int *lk);

    /*stp*/
    int (*mstp_port_enable)(unsigned int g_ifindex);
    int (*mstp_port_disable)(unsigned int g_ifindex);
    int (*mstp_set_port_state)(unsigned int g_ifindex, int mstid, int state);

    /*igmp snooping*/
    int (*igmp_trap_set)(int vlan, unsigned int g_ifindex, int flags);
    int (*l2mc_entry_add_port)(void *l2_mc_entry);
    int (*l2mc_entry_del_port)(void *l2_mc_entry);

	/*dhcp snooping*/
	int (*dhcp_trap_set)(int vlan, unsigned int g_ifindex, int flags);
	

    /*qinq*/
    int (*access_qinq_enable)(unsigned int g_ifindex, int mode);
    int (*qinq_drop_miss_enable)(unsigned int g_ifindex, int flags);
	int (*tpid_set)(unsigned int g_ifindex, unsigned short value);
	int (*inner_tpid_set)(unsigned int g_ifindex, unsigned short value);

    /*mac-based, subnet-based vlan*/
    int (*subnet_vlan_enable)(unsigned int g_ifindex, int flags);
    int (*mac_vlan_enable)(unsigned int g_ifindex, int flags);
    int (*prefer_subnet_enable)(unsigned int g_ifindex, int flags);
    int (*port_vlan_filter)(unsigned int g_ifindex, int flags);

    /*private vlan*/
    int (*port_isolate_add)(unsigned int g_ifindex1, unsigned int g_ifindex2);
    int (*port_isolate_del)(unsigned int g_ifindex1, unsigned int g_ifindex2);
    /*layer 3 intf*/

	/*IPv6 mld snooping*/
	int (*mld_trap_set)(int vlan, unsigned int g_ifindex, int flags);
    
}port_driver_t;


extern array_table_index_t *switch_ports;
extern hash_table_index_t *switch_ports_hash;
#endif

