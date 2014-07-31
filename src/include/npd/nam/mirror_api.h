#ifndef __NPD_MIRROR_API_H__
#define __NPD_MIRROR_API_H__


/*NPD LAYER API*/
void npd_mirror_init(void);


unsigned int npd_mirror_profile_create
(
	unsigned int profileId
);


void npd_mirror_profile_config_save
(
	char* showStr
);


int npd_mirror_destination_node_port_get
(
	unsigned int profile,
	unsigned int direct,
	unsigned int *eth_g_index
);



int npd_mirror_src_all_port_destroy
(
	unsigned int dest_eth_g_index
);



void npd_mirror_src_port_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int *totalLen,
	unsigned char *entered
);


int npd_mirror_src_all_vlan_destroy
(
	unsigned int dest_eth_g_index
);


void npd_mirror_src_vlan_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
);


int npd_mirror_src_all_fdb_destroy
(
	unsigned int dest_eth_g_index
);


void npd_mirror_src_fdb_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
);


void npd_mirror_src_policy_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
);

extern unsigned int npd_fdb_static_mirror_entry_del
(
    unsigned char * mac,
    unsigned short vid
);
extern unsigned int npd_fdb_check_static_entry_exist
(
    unsigned char * mac,
    unsigned short vid,
    unsigned int * eth_g_index
);
int npd_mirror_src_acl_check
(
	unsigned int profile,
	unsigned int  ruleIndex
);
int npd_mirror_src_acl_create
(
	unsigned int profile,
	unsigned int ruleIndex
);
int npd_mirror_src_acl_remove
(
	unsigned int profile,
	unsigned int ruleIndex
);

int npd_mirror_delete_source_port_ingress_egress
(
    unsigned char devNum,
    unsigned char portNum,
    unsigned int src_eth_g_index,
    unsigned int profile
);

int npd_mirror_delete_source_port_bidirection
(
    unsigned char devNum,
    unsigned char portNum,
    unsigned int src_eth_g_index
);


/*NAM LAYER API*/
void nam_mirror_init
(
	void
);

unsigned int nam_mirror_analyzer_port_set
(
	unsigned int profile,
	unsigned int src_eth_g_index,
	unsigned int analyzer_eth_g_index,
	unsigned int enable,
	unsigned int direct	
);
 
unsigned int nam_mirror_fdb_entry_set
(
    unsigned short vlanid,
    unsigned int global_index,
    ETHERADDR *macAddr,
    unsigned int flag
);

unsigned int nam_mirror_port_set
(
	unsigned int profile,
	unsigned int eth_g_index,
	unsigned int analyzer_eth_g_index,
	unsigned int enable,
	unsigned int  direct
);
unsigned int nam_acl_drv_mirror_action_update
(
	unsigned int ruleIndex,
	unsigned int enable,
	int netif_index
);

unsigned int nam_mirror_vlan_set
(
	unsigned int profile,
	unsigned short vid,
	unsigned int analyzer_eth_g_index,	
	unsigned int enable
);

unsigned int nam_mirror_remote_vlan_set
(
	unsigned int profile,
	unsigned int analyzer_eth_g_index,
	unsigned short vid,	
	unsigned int direct,	
	unsigned int enable
);

unsigned int nam_profile_check(unsigned int profile);

unsigned int nam_mirror_analyzer_port_set_for_bcm
(
    unsigned char srcDev,
    unsigned char srcPort,
    unsigned char analyzerDev,
	unsigned char analyzerPort,
	unsigned int  direct
	
);

unsigned int nam_mirror_vlan_set_for_bcm
(
	unsigned short vid,
	unsigned char destDev,
	unsigned char destPort,
	unsigned int enable
);


extern unsigned int npd_get_global_index_by_devport
(
    unsigned char devNum,
    unsigned char portNum,
    unsigned int * eth_g_index
);
extern unsigned int npd_get_devport_by_global_index
(
    unsigned int eth_g_index,
    unsigned char * devNum,
    unsigned char * portNum
);

#endif

