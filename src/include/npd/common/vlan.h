#ifndef __COMMON_VLAN_H__
#define __COMMON_VLAN_H__

#define ALIAS_NAME_SIZE 0x15

#define NPD_VLAN_NUMBER_MAX			(4096)
#define NPD_VLAN_IFNAME_SIZE			21
#define NPD_VLAN_BITMAP_LENGTH		64	/*port index bitmap row count*/
#define NPD_VLAN_DRV_BITMAP_LENGTH	2	/*driver port bitmap row count*/
#define NPD_VLAN_BITMAP_WIDTH			32	/*bitmap column count*/
#define VLAN_CONFIG_SUCCESS 			0
#define VLAN_CONFIG_FAIL				0xff	
#define BRG_MC_SUCCESS					0
#define BRG_MC_FAIL						0xff


#define NPD_VLAN_MTU_VALUE  1522

#define DEFAULT_VLAN_ID			0x1
#define NPD_PORT_L3INTF_VLAN_ID	0xfff 


enum vlan_type_e {
	BRG_VLAN_PORT_BASED_E = 0,
	BRG_VLAN_PROTOCOL_BASED_E,
	BRG_VLAN_PORT_PROTOCOL_BASED_E,
	BRG_VLAN_TYPE_MAX
};

/*
 * VLAN info main structure.
 */
typedef enum{
    VLAN_TYPE_STATIC,
    VLAN_TYPE_DYNAMIC 
}vlan_type_t;

typedef struct vlan_s{
    unsigned int vid;
    unsigned int g_ifindex;
    char name[ALIAS_NAME_SIZE];
    int fdb_limit;
    int mtu;
    int fdb_learning_mode;
    int link_status;
    npd_pbmp_t untag_ports;
    npd_pbmp_t tag_ports;
    unsigned int isAutoCreated;				/* whether the vlan is auto created flag */
/*    int private_enable; */
	unsigned int   pvlan_type;
	unsigned int   IsEnable;
	unsigned int   groupId;
	unsigned int   EgrGroupId;
	unsigned int   isStatic;
	unsigned int   forward_mode;
}vlan_t;

typedef struct macbase_vlan_s{
    unsigned int vid;
    unsigned char mac[6];
	unsigned int tbl_index;
}macbase_vlan_t;

typedef struct subnetbase_vlan_s{
    unsigned int vid;
    unsigned int ipaddr;
    unsigned int mask;
	unsigned int tbl_index;	
}subnetbase_vlan_t;

enum{
    VLAN_PROTOCOL_FRAME_ETHER2,
    VLAN_PROTOCOL_FRAME_LLC,
    VLAN_PROTOCOL_FRAME_SNAP
};

typedef struct protobase_vlan_s{
    int proto_group_index;
    unsigned int ether_frame;
    unsigned short eth_type;
}protobase_vlan_t;

typedef struct proto_vlan_port_s{
    int proto_group_index;
    unsigned short vid;
    unsigned int netif_index;
}proto_vlan_port_t;

/* private vlan */
typedef struct pvlan_primary_s{
    unsigned int vid;
	npd_vbmp_t secondary_vlans;
	npd_pbmp_t ports;	
}pvlan_primary_t;

typedef struct pvlan_isolate_s{	
    unsigned int vid;
	unsigned int primary_vid;
	npd_pbmp_t ports;
}pvlan_isolate_t;


#define XLATE_INVALID_VID 0

typedef struct npd_vlan_qinq_s
{
	unsigned short tpid;	
	unsigned short inner_tpid;
}npd_vlan_qinq_t;

enum 
{
    XLATE_PUSH_INNER,
    XLATE_PUSH_OUTER,
    XLATE_PUSH_BOTH,
    XLATE_REWRITE_OUTER,
    XLATE_REWRITE_INNER,
    XLATE_REWRITE_BOTH,
    XLATE_ALL_PUSH_OUTER,
    XLATE_ALL_PUSH_BOTH,        
    XLATE_POP_INNER,
    XLATE_POP_OUTER,
    XLATE_POP_BOTH,    
    XLATE_E_REWRITE_OUTER,
    XLATE_E_REWRITE_INNER,
    XLATE_E_REWRITE_BOTH
};


typedef struct vlan_xlate_db_entry_s{
    unsigned int netif_index;
    int xlate_type;
    int priority;
    unsigned int ingress_outer_start_vid;
    unsigned int ingress_outer_vid_num;
    unsigned int ingress_inner_start_vid;
    unsigned int ingress_inner_vid_num;
    unsigned int egress_outer_vid;
    unsigned int egress_inner_vid;
}vlan_xlate_db_entry_t;


enum {
	FORWARD_BRIDGING,
	FORWARD_SINGLE_XC,		/* VLAN Single Cross Connect */
	FORWARD_DOUBLE_XC		/* VLAN Double Cross Connect */
};

enum {
	VLAN_ELINE_TYPE_SINGLE,
	VLAN_ELINE_TYPE_DOUBLE
};

typedef struct vlan_eline_db_entry_s{
	unsigned int eline_id;
	unsigned int eline_type;
    unsigned int outer_vid;
    unsigned int inner_vid;
    unsigned int netif_index_first;
    unsigned int netif_index_second;
}vlan_eline_db_entry_t;

extern sequence_table_index_t *g_vlans;

#endif

