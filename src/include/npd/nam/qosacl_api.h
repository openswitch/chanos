#ifndef __NAM_QOSACL_API_H__
#define __NAM_QOSACL_API_H__

#define VOICE_VLAN_RSV_SERVICE_POLICY_ID   0
#define VOICE_VLAN_RSV_QOS_PROFILE_ID       25

typedef enum AccessList_Phase_E
{
	ACL_PHASE1_E = 0x1,
	ACL_PHASE2_E = 0x2,
	ACL_PHASE3_E = 0x4,
	ACL_PHASE12_E = 0x8,
	ACL_PHASEIP6_IN_E = 0x10,
	ACL_PHASEIP6_EG_E = 0x20,
	ACL_PHASE_MAX = 0x21
} ACL_PHASE_E;

typedef enum ACL_TCAM_SET_E
{
	ACL_TCAM_DEL,
	ACL_TCAM_SET,
} TCAM_SET_E;


#define PORT_GROUP_MAX_RULE 16
struct port_group_s
{
	int portGroupId;
	int service_policy_index;
    int dir;
	int AclPhaseId[ACL_PHASE_MAX][PORT_GROUP_MAX_RULE][2];
    struct vlan_based_acl_info_s *head;
};

/* class map array index and hash index */
extern array_table_index_t* class_map_master;
extern hash_table_index_t* class_map_master_name;
/* class map rule hash index */
extern hash_table_index_t* class_map_match_rule;
/* policy map array index and hash index */
extern array_table_index_t* policy_map_index;
extern hash_table_index_t* policy_map_name;
/* policy map rule hash index */
extern hash_table_index_t* policy_map_set_rule;
/* service policy array index and hash index */
extern array_table_index_t* service_policy_index;
extern hash_table_index_t* service_policy_name;
extern array_table_index_t *port_group_array_index;
extern hash_table_index_t  *port_group_hash_index;


/* Check class-map is exist by name */
int class_map_check_exist(const char* map_name);


/* Check policy-map is exist by name */
int policy_map_check_exist(const char * map_name);


/*init class qos structure*/

/*create class map*/

/*delete class map*/

/*rename class map*/
int class_map_rename(const char* oldmapname, const char* newmapname);

/*delete policy map*/
int policy_map_rename(const char* oldmapname, const char* newmapname);


/* show running for class-map. */
char* acl_match_show_running_config(char* showStr, int* safe_len);

/* show running for policy-map. */
char* acl_action_show_running_config(char* showStr, int* safe_len);

unsigned int class_map_rule_filter(void *input, void *exist);
unsigned int policy_map_rule_filter(void *input, void *exist);


void npd_acl_rule_based_tm_timer(void);

char *npd_acl_group_show_running_config(char *showStr, int *safe_len);
char* time_range_info_associate_show_running(char* showStr, int* safe_len);

char* acl_action_show_running_config(char* showStr, int* safe_len);
char* acl_match_show_running_config(char* showStr, int* safe_len);
char* voice_vlan_showrunning(char* showStr, int* safe_len);
char* acl_rule_show_running_config(char* showStr, int* safe_len);
char* periodic_time_range_info_show_running(char* showStr, int* safe_len);
char* abs_time_range_info_show_running(char* showStr, int* safe_len);
int npd_is_rule_in_acl_group(int policy_index);
int service_policy_create_reserved(char* policy_name, int dir, unsigned int id);


/*NPD LAYER FUNCTION*/
int free_last_to_index(int index ,unsigned char unit, char del_flag);
int install_index_to_last(int index, unsigned char unit, char del_flag);
int acl_entry_alloc(int index, unsigned char unit, int offset, unsigned char lkphase);
void* acl_entry_get(int index, unsigned char unit, int offset, unsigned char lkphase, int *entryId);
int acl_entry_id_free(int index,  int offset, unsigned char unit, unsigned char lkphase);
int acl_entry_del(int index, int entry_max, int offset, char unit);
void* acl_rule_entry_get(int index, int offset, unsigned char unit, unsigned char lkphase, int rule_size);
int service_index_init
(    
    int max_rule_num, 
    int rule_entry_max, 
    int (*nam_qos_set_entry)(int , void *, int, char, unsigned char)
);
int class_map_find_by_name(const char* map_name, struct class_map_index_s* class_map);
int policy_map_find_by_name(const char* map_name, struct policy_map_index_s* policy_name);
void class_qos_init();
int class_map_create(const char* map_name);
int class_map_delete(const char *map_name);

int class_map_add_match(struct class_map_index_s* index,
						const char* match_name,
                     	const char* match_arg,
                     	const char* match_mask);
/* Delete specified class match rule. */
int class_map_delete_match(struct class_map_index_s *index,
			           const char *match_name);

/*create policy map*/
int policy_map_create(const char * map_name);

/*delete policy map*/
int policy_map_delete(const char* map_name);


/*create service policy*/
int service_policy_create(char* policy_name, int dir, unsigned int netif_index);
int service_policy_create_bmp(char *policy_name, int dir, npd_pbmp_t portbmp);
int service_policy_create_bmp_after_id(char *policy_name, int dir, npd_pbmp_t portbmp, int start_id);
int service_policy_create_after_id(char* policy_name, int dir, unsigned int netif_index, int start_id);

/*delete service policy*/
int service_policy_destroy(char* map_name, int dir, unsigned int netif_index);
int service_policy_destroy(char* map_name, int dir, unsigned int netif_index);
int service_policy_destroy_bmp(char* map_name, int dir, npd_pbmp_t portbmp);
int service_policy_find_by_name_dir(const char* map_name, int dir, struct service_policy_s* service_policy);
    
/* Add policy-map set statement to the route map. */
int policy_map_add_set(struct policy_map_index_s *index,
								const char *set_name,
								const char *set_arg);

/* Delete policy map set rule. */
int policy_map_delete_set(struct policy_map_index_s *index,
								const char *set_name);

/* Delete class-map from policy map. */
int policy_map_no_class(const char* classname, const char* policyname);

/* Delete class-map from policy map. */
int policy_map_class(const char* classname, const char* policyname);

int npd_classmap_rule_del_by_vid(unsigned int vid);
int npd_policy_route_update_by_nhp(unsigned int ipAddr, int valid);

int npd_qos_netif_cfg_set_by_index(unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr);
int npd_policer_get_by_index(unsigned int index, QOS_POLICER_STC *profilePtr);
int npd_qosprofile_get_by_index(unsigned int index, QOS_PROFILE_STC *profilePtr);
int npd_qos_netif_cfg_get_by_index(unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr);
int npd_qos_netif_cfg_del_by_index(unsigned int netifIndex);
int npd_policer_get_by_index( unsigned int index, QOS_POLICER_STC *profilePtr );
int npd_policer_set_by_index( unsigned int index, QOS_POLICER_STC *profilePtr );
void npd_qos_init(void);
unsigned int npd_qos_port_unbind_opt(unsigned int eth_g_index,unsigned int policyMapIndex);
char* acl_ipv6_show_running_config(char* showStr, int* safe_len);




/*NAM Layer API*/
unsigned int nam_qos_profile_entry_set
(
     unsigned int     profileIndex,
     QOS_PROFILE_STC *profile
);

unsigned int nam_acl_diffserv_group_enable(int enable);

unsigned int nam_qos_dscp_to_profile_entry
(
	unsigned int dscp,
	unsigned int profileIndex,
	int unit,
	int port
);

unsigned int nam_qos_port_bind_policy_map
(
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *poMap,
	unsigned int netif_index
);

unsigned int nam_qos_support_check(	void);

unsigned int nam_qos_port_unbind_policy_map
(
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *poMap,
	unsigned int netif_index
);

unsigned int nam_qos_traffic_shape_port
(
	unsigned int netifIndex,
	unsigned char mode,
	unsigned int  burstSize,
	unsigned long maxRatePtr
);

unsigned int nam_qos_traffic_shape_queue_port
(
	unsigned int netifIndex,
	unsigned char mode,
	unsigned int  tcQueue,
	unsigned int  burstSize,
	unsigned long userRatePtr
);
unsigned int nam_qos_del_traffic_shape_queue_port
(
	unsigned int netifIndex,
	unsigned char mode,
	unsigned int  tcQueue
);

int nam_qos_cpu_flow_cortrol
(
	char *protocol,
	int bandwith, 
	int priority
);

int nam_get_trunk_by_peer_slot(unsigned int slot_id, unsigned char unit, unsigned int *trunkId);

unsigned int nam_qos_read_counter
(
    unsigned int policerId,
	unsigned int   counterSetIndex,
	unsigned long long *conform_bytes,
	unsigned long long *conform_pkts,
	unsigned long long *exceed_bytes,
	unsigned long long *exceed_pkts,
	unsigned long long *violate_bytes,
	unsigned long long *violate_pkts
);

unsigned int nam_qos_set_counter
(
    unsigned int    policerIndex,
	unsigned int    counterSetIndex,
	int isEnable
);

int nam_qos_port_eg_remark(unsigned int netif_index, int remark);

int nam_qos_port_in_remark(unsigned int netif_index, int remark);

int nam_qos_port_default_qos_profile(unsigned int netif_index, unsigned int index);
unsigned int nam_qos_port_bind_policy_map_l2_set
(
    QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *poMap, 
    unsigned int netif_index
);
unsigned int nam_qos_port_bind_policy_map_l3_set
(
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC* poMap,
	unsigned int netif_index
);

int nam_qos_port_trust(unsigned int netif_index, int trust);


unsigned int nam_qos_queue_set(unsigned int netif_index, unsigned int wrrFlag, unsigned int  *wrrWeight );


unsigned int nam_qos_traffic_queue_set();
unsigned int nam_qos_policer_create(int* policer_index, int dir);


unsigned int nam_qos_policer_set_cir_cbs(unsigned int policer_index, QOS_POLICER_STC *policer);

unsigned int nam_qos_policer_destroy(int policer_index, int dir);

int nam_qos_init();
unsigned int nam_qos_policer_init();

unsigned int nam_qos_policer_entry_set
(
	unsigned int 		policerIndex,
	QOS_POLICER_STC 	*policer
);

unsigned int nam_qos_policer_init();

unsigned int nam_qos_policer_cir_cbs_check
(
	unsigned int  cir,
	unsigned int  cbs
);

unsigned int nam_acl_drv_mirror_action_update
(
	unsigned int ruleIndex,
	unsigned int enable,
	int netif_index
);


unsigned int npd_qos_profile_index_check
(
	unsigned int profileIndex
);


#endif

