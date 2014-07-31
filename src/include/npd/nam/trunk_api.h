#ifndef __NPD_TRUNK_API_H__
#define __NPD_TRUNK_API_H__

/*NPD LAYER API*/
extern sequence_table_index_t * g_trunks;

void npd_init_trunks
(
	void
);
unsigned int npd_check_trunk_exist
(
	unsigned short trunkId
);

int npd_find_trunk
(
	unsigned short trunkId,
	struct trunk_s * trunk
);
int npd_find_trunk_by_name
(
	char *name,
	struct trunk_s * trunk

);
int npd_check_trunk_status
(
	unsigned short trunkId
);
struct trunk_s* npd_create_trunk
(
	unsigned short trunkId
);
struct trunk_s* npd_create_trunk_by_name
(
	char *name
);
unsigned int npd_delete_trunk
(
	unsigned short trunkId
);

unsigned int npd_trunk_check_port_membership
(
	unsigned short trunkId,
	unsigned int   eth_index
);


unsigned int npd_trunk_member_port_index_get_all
(
	unsigned short	trunkId,
	unsigned int	eth_g_index[],
	unsigned int	*mbr_count
);
unsigned int npd_trunk_endis_member_port_index_get
(
	unsigned short	trunkId,
	unsigned int	eth_g_index[],
	unsigned char 	enDis,
	unsigned int	*mbr_count
);


unsigned int npd_trunk_master_port_config
(
	unsigned short trunkId,
	unsigned int   eth_index
);


unsigned int npd_trunk_load_balanc_set
(
	unsigned short trunkId,
	unsigned int lbMode
);

unsigned int npd_trunk_master_port_get
(
	unsigned short trunkId,
	unsigned int   *eth_index
);


unsigned int npd_trunk_activate
(
	unsigned short trunkId,
	char* name
);

unsigned int npd_trunk_port_add
(
	unsigned short trunkId, 
	unsigned int eth_index
);

unsigned int npd_trunk_port_del
(
	unsigned short	trunkId,
	unsigned int	eth_index
);

unsigned int npd_trunk_master_port_set
(
	unsigned short trunkId, 
	unsigned int eth_index
);

unsigned int npd_trunk_destroy_node
(
	unsigned short trunkId
);

int npd_trunk_allow_vlan(
    unsigned int trunkId,
    unsigned int vid,
    int isTaged
    );

int npd_trunk_driver_dhcp_trap_set
(
	int vid,
	unsigned int netif_index,
	int flags
);
int npd_check_trunk_member_exist
(
	unsigned short trunkId
);

int npd_del_trunk_route_mode(unsigned int trunkId);
int npd_set_trunk_route_mode(unsigned int trunkId,unsigned int mode);
int npd_get_trunk_route_mode(unsigned int netif_index,unsigned char *mode);

void npd_lacp_init();


/*NAM LAYER API*/
#define TRUNK_CONFIG_SUCCESS	0x0
#define	TRUNK_CONFIG_FAIL		0xff
#define TRUNK_PORT_EXISTS_GTDB	0xf		/*port already exists in trunkDB*/
#define TRUNK_PORT_MBRS_FULL	(TRUNK_PORT_EXISTS_GTDB+1)
#define TRUNK_MEMBER_NUM_MAX	0x8

unsigned int nam_asic_trunk_entry_active
(
	unsigned short trunkId
);

unsigned int nam_asic_trunk_ports_add
(
	unsigned int eth_g_index,
	unsigned short trunkId,
	unsigned char enDis
);


unsigned int nam_asic_trunk_ports_del
(
	unsigned int eth_g_index,
	unsigned short trunkId
);

unsigned int nam_asic_trunk_delete
(
	unsigned short trunkId
);
unsigned int nam_asic_trunk_load_balanc_set
(
	unsigned short trunkId,
	unsigned int  lbMode
);
unsigned int nam_asic_trunk_get_port_member
(
	unsigned short	trunkId,
	unsigned int* 	memberCont,
	unsigned int*	dev0MbrBmp,
	unsigned int*	dev1MbrBmp
);
unsigned int nam_asic_trunk_get_port_member_bmp
(
	unsigned short	trunkId,
	unsigned int*	dev0MbrBmp,
	unsigned int*	dev1MbrBmp
);


unsigned int nam_asic_trunk_ports_hwadd
(
	unsigned int eth_g_index,
	unsigned short trunkId,
	unsigned char enDis
);

unsigned int nam_asic_trunk_ports_hwdel
(	
	unsigned int eth_g_index,
	unsigned short trunkId
);
unsigned int nam_asic_trunk_port_endis
(
	unsigned int eth_g_index,
	unsigned short trunkId,
	unsigned char enDis
);


int nam_asic_trunk_master_port_set(
    unsigned int trunkId,
	unsigned int eth_g_index
    );


int nam_asic_trunk_linkstate_set(unsigned short trunkId, unsigned char linkstate);

int nam_asic_trunk_vlan_mode_set
(
	unsigned short trunkId, 
	int mode
);
int nam_asic_trunk_forward_mode_set
(
	unsigned short trunkId, 
	int mode
);

#endif

