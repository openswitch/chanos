#ifndef __PRODUCT_FEATURE_H_
#define __PRODUCT_FEATURE_H_


#define BOOL char

#define FAMILY_UNKNOWN -1
#define PRODUCT_UNKNOWN -1

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

struct product_sysinfo_s {
	char *sn;
	char *name;
	char *basemac;  /* 12 char of mac address  with no : or - spliter.*/
	char *sw_name;
	char *enterprise_name;
	char *enterprise_snmp_oid;
	char *snmp_sys_oid;
	char *built_in_admin_username;
	char *built_in_admin_passwd;
};
typedef struct product_sysinfo_s product_man_param_t;

struct module_info_s {
    int id;
	unsigned char hw_version;
	char *modname;
	char *sn;
	void *ext_slot_data;   /* extend info could be organized as a link list.*/
};


extern unsigned int PPAL_BOARD_TYPE_NONE;
extern unsigned int PPAL_BOARD_TYPE_MAX;
extern unsigned int PRODUCT_MAX_NUM;

typedef struct power_supply_man_param_s power_param_t;
typedef struct fan_man_param_s	fan_param_t;

typedef enum chassis_state_e
{
    CHASSIS_INITIALIZING,
    CHASSIS_INITDONE,
    CHASSIS_RUNNING
}chassis_state_t;

typedef enum board_role_e
{
	SLAVE_BOARD,
	MASTER_BOARD,
	SUB_BOARD
}board_role_t;

typedef enum master_role_e
{
    MASTER_ACTIVE,
    MASTER_STANDBY
}master_role_t;

typedef enum master_priority_e
{
	MASTER_PRIORITY_FORCE_STANDBY,
	MASTER_PRIORITY_INIT,
	MASTER_PRIORITY_RUNNING,
	MASTER_PRIORITY_FORCE_ACTIVE,
}master_priority_t;


typedef enum remote_subboard_running_state_e
{
    SUBBOARD_NOEXIST,
    SUBBOARD_REGISTERING,
    SUBBOARD_REGISTERED,
    SUBBOARD_READY,
    SUBBOARD_RUNNING,
    SUBBOARD_REBOOTING,
    SUBBOARD_REMOVING,
    SUBBOARD_REMOVED,    
    SUBBOARD_ERROR
}remote_subboard_running_state_t;

typedef enum local_slave_running_state_e
{
    LOCAL_SLAVE_INIT,
    LOCAL_SLAVE_WAIT_CONNECT,
    LOCAL_SLAVE_REGISTERING,
    LOCAL_SLAVE_SW_UPGRADING,
    LOCAL_SLAVE_READY,
    LOCAL_SLAVE_RUNNING,
    LOCAL_SLAVE_SWITCHOVERING,
    LOCAL_SLAVE_ERROR
}local_slave_running_state_t;

typedef enum local_master_running_state_e
{
    LOCAL_MASTER_INIT,
    LOCAL_SBYMASTER_WAIT_CONNECT,
    LOCAL_SBYMASTER_REGISTERING,
    LOCAL_SBYMASTER_SW_UPGRADING,
    LOCAL_SBYMASTER_READY,
    LOCAL_SBYMASTER_RUNNING,
    LOCAL_SBYMASTER_SWITCHOVERING,
    LOCAL_SBYMASTER_ERROR,
    LOCAL_ACTMASTER_DISCOVERING,
    LOCAL_ACTMASTER_DBSYNCING,
    LOCAL_ACTMASTER_RUNNING,
    LOCAL_ACTMASTER_SWITCHOVERING
}local_master_running_state_t;

typedef enum remote_board_running_state_e
{
    RMT_BOARD_NOEXIST,
    RMT_BOARD_HWINSERTED,
    RMT_BOARD_SWINSERTED,
    RMT_BOARD_REGISTERING,
    RMT_BOARD_SW_VERERR,    
    RMT_BOARD_REGISTERED,
    RMT_BOARD_READY,
    RMT_BOARD_RUNNING,
    RMT_BOARD_SWITCHOVERING,
    RMT_BOARD_REMOVING,
    RMT_BOARD_ERROR,
    RMT_BOARD_MAX
}remote_board_running_state_t;

typedef union board_running_state_u
{
    remote_board_running_state_t rmt_board_running_state;
    remote_board_running_state_t rmt_master_running_state;
    local_slave_running_state_t local_slave_running_state;
    local_master_running_state_t local_master_running_state;
}board_running_state_t;

typedef enum
{
    TIMER,
    HW_INSERT,
    SW_INSERT,
    HW_REMOVE,
    SW_REMOVE,
    ACTIVE_MASTER_ENABLE,
    STANDBY_MASTER_ENABLE,
    TIPC_CONNECT,
    TIPC_BREAK,
    REGISTER_REQUEST,
    REGISTER_RESPONSE,
    STATUS_REPORT,
    MASTER_CMD,
    SWITCHOVER_REPORT,
    QUERY,
    RESET,
    NONCOMPAT_REQUEST,
    BOARD_EVENT_MAX
} state_event_e;

#define MASTER_SLOT_NULL 0
#define MASTER_SLOT_MAX_NUM 2

typedef struct board_manage_fix_param_s
{
    unsigned short slotnum;
    unsigned short master_slotnum;
    unsigned short master_slot_id[MASTER_SLOT_MAX_NUM];
    unsigned long  can_distr;
    unsigned long  can_master_service; /*master also work as line card*/
    unsigned long  topo;              /*full mesh or central fabric, for full mesh line card can be mcu*/
    struct 
    {
        unsigned long  slot_phy2log;
        unsigned long  slot_log2phy;
    }slot_map[16];
	long (*board_inserted)(unsigned long slot_index);	 
	long (*master_board_inserted)(unsigned long slot_index);
    long (*board_insert_detect)();  /*the isr connect and detect thread start*/
    long (*board_reset)(unsigned long slot_index);
    long (*board_poweroff)(unsigned long slot_index);
}board_manage_fix_param_t;

typedef struct pne_fix_param_s
{
    unsigned long  power_num;
    unsigned long  fan_num;
    long (*pne_monitor_start)(); /*isr connect and detect thread start*/
    long (*pne_fanspeed_adjust)(unsigned long index, unsigned long speed);
	long (*power_man_param_init)(power_param_t * param);
	long (*fan_man_param_init)(fan_param_t * param);
}pne_fix_param_t;

typedef struct board_feature_s
{
    int  jumbo_size;
    int  max_macaddr;
    int  max_macapp;
    int  max_macmpls;
    int  max_ext_macaddr;
    int  max_normal_vlan;
    int  max_macbased_vlan;
    int  max_protobased_vlan;
    int  max_ipsubnet_vlan;
    int  max_stpg;
    int  max_trunk;
    int  max_port_per_trunk;
    int  max_fabric_trunk_extra;
    int  max_vlanswitch_ingress;
    int  max_vlanswitch_egress;
    int  max_qinq;
    int  max_range_qinq;
    int  max_sec_per_range_qinq;
    int  max_mirror_des_port;
    
    int  max_l2mc;
    int  max_vr;
    int  max_ip_intf;
    int  max_ip_route;
    int  max_ip_host_extra;
    int  max_ip_ecmp;
    int  max_ip_route_per_ecmp;
    int  max_ipmc_extra;
    int  max_ipv6_route;
    int  max_ipv6_host_extra;
    int  max_ipv6_ecmp;
    int  max_ipv6_route_per_ecmp;
    int  max_ipv6mc_extra;
    int  max_tunnel;
    int  max_dscp_map;
    int  max_ele_per_dscp_map;
    int  max_mpls_virtual_port;
    int  max_mpls_vfi;
    int  max_mpls_lsp;
    int  max_mpls_pushtagnum;
    int  max_mpls_handle_tagnum;
    int  max_vlan_acl_std;
    int  max_ingress_acl_std;
    int  max_egress_acl_std;
    int  vlan_acl_portlist_len;
    int  ing_acl_portlist_len;
    int  egr_acl_portlist_len;
    int  max_qos_meter;
    int  max_qos_counter;
    int  bandwidth_adjust_size;
    
    int  max_vr_ext;
    int  max_ip_intf_ext;
    int  max_ip_route_ext;
    int  max_ip_host_extra_ext;
    int  max_ip_ecmp_ext;
    int  max_ip_route_per_ecmp_ext;
    int  max_ipmc_extra_ext;
    int  max_ipv6_route_ext;
    int  max_ipv6_host_extra_ext;
    int  max_ipv6_ecmp_ext;
    int  max_ipv6_route_per_ecmp_ext;
    int  max_ipv6mc_extra_ext;
    int  max_tunnel_ext;
    int  max_mpls_vfi_ext;
    int  max_mpls_lsp_ext;
    int  max_vlan_acl_std_ext;
    int  max_ingress_acl_std_ext;
    int  max_egress_acl_std_ext;
    int  max_qos_meter_ext;
    int  max_qos_counter_ext;

    int  max_txqueue_system;
    int  max_txqueue_perport;
    int  max_rxqueue_system;
    int  max_rxqueue_perport;
    int  max_shaper_system;
    int  max_shaper_perport;
        
    int  ipv4_route_ele_acl_rule_size;
    int  ipv6_route_ele_acl_rule_size;
    int  mpls_lsp_ele_acl_rule_size;

    BOOL ext_tcam;
    BOOL mac_pending_learn;
    BOOL ext_mac_auto_learn;
    BOOL prohibit_sta_move;
    BOOL class_sta_move;
    BOOL mac_limit_system;
    BOOL mac_limit_per_port;
    BOOL mac_limit_per_lag;
    BOOL mac_limit_per_vlan;
    BOOL mac_limit_per_vpls;
    BOOL mac_delete_per_port;
    BOOL mac_delete_per_lag;
    BOOL mac_delete_per_vlan;
    BOOL mac_delete_perport_pervlan;
    BOOL mac_delete_perlag_pervlan;
    BOOL mac_learn_svl;
    BOOL private_vlan;
    BOOL flow_vlan;
    BOOL vlan_xlate;
    BOOL mac_based_app;
    BOOL mac_based_mpls;
    BOOL same_port_bridge;
    
    BOOL vrf;
    BOOL route_urpf;

    BOOL ipv4_tunnel;
    BOOL ipv6_tunnel;
    BOOL gre_tunnel;
    BOOL mpls_tunnel;

    BOOL mpls_vpls;
    BOOL mpls_h_vpls;
    BOOL mpls_vpn;
    BOOL mpls_te;
    BOOL mpls_frr;
    BOOL mpls_diffserv;

    BOOL preingress_filter;
    BOOL ingress_filter;
    BOOL egress_filter;
    BOOL sg_filter;

    BOOL wred;
    BOOL queue_wdrr;
    BOOL queue_wrr;
    BOOL queue_rr;

    BOOL stack;
    BOOL rspan;
    BOOL sflow;
    BOOL acl_as_route;
    unsigned int sys_rsvd_end_id;
    unsigned int ip_route_start_id;
    unsigned int ip_route_end_id;
    unsigned int acl_start_id;
    unsigned int acl_end_id;
    unsigned int macbased_vlan_start_id;
    unsigned int macbased_vlan_end_id;    
    unsigned int ipsubnet_vlan_start_id;
    unsigned int ipsubnet_vlan_end_id;    

	unsigned int ctrl_num;
	BOOL ctrl_switch;
    BOOL vlan_mirror;
    BOOL fdb_mirror;
    BOOL pcl_eg_mirror;
    BOOL pcl_eg_redirect;
	BOOL can_slave_indpt;

    BOOL support_acl_based_vlan;
    BOOL support_acl_policy_route;
    unsigned int num_of_tcp_comparator;
    unsigned int num_of_udp_comparator;
    int trunk_notcross_board;
    BOOL vct_available;
}board_feature_t;

typedef struct board_feature_s product_feature_t;

typedef struct product_fix_param_s
{
	long product_code;
	long product_type;
	char * product_short_name;
	char * product_name;
	char * serial_no;

    board_manage_fix_param_t *board_manage;
    pne_fix_param_t *pne_fix_param_t;
    product_feature_t *product_pp_feature;
    
    long (*product_man_param_get)(product_man_param_t *param);
    void (*product_reset)();
    void (*product_show_chassis)();
    /*sometime different product will give different logical slot when the physical
    slot is the same*/
    long (*slotno_get)(); 	
	void (*interrupt_handler)();
	void (*master_set)(unsigned long value);

    void (*product_pp_feature_set)();
}product_fix_param_t;
 

typedef struct tempe_man_param_s
{
    char *name;
    int  celcius;
    enum
    {
        TEMPE_NORMAL,
        TEMPE_OVER,
        TEMPE_DEAD_OVER
    } status;
}tempe_man_param_t;

typedef struct power_supply_man_param_s
{
	unsigned ps_index;
    char name[20];
    int inserted;
    enum
    {
        POWER_AC,
        POWER_48VDC,
    } type;
    enum
    {
        POWER_SUPPLY_NORMAL,
        POWER_SUPPLY_ALARM,
    }status;
}power_supply_man_param_t;

typedef struct fan_man_param_s
{
	unsigned fan_index;
    char name[20];
    unsigned long speed;
    int inserted;
    int	status;
}fan_man_param_t;


typedef struct product_param_s
{
	unsigned long family_type;
	unsigned long  product_id;
	unsigned long local_module_id;
	unsigned char local_module_hw_version;
    unsigned long active_master_slot;
	int local_chassis_slot_no;  /* front panel slot number*/
    chassis_state_t chassis_state;
    
    product_fix_param_t *fix_param;
    product_man_param_t sys_info;
    power_supply_man_param_t* power_supply_param;
    fan_man_param_t*  fan_param;
    unsigned char base_mac[6];
}product_param_t;


typedef struct module_info_s board_man_param_t;



typedef struct ams_info_s
{
    unsigned char name[32];
    unsigned short type;
    unsigned short index;
    unsigned short dev_id;
    unsigned short rev_id;
} ams_info_t;

typedef struct ams_fix_param_s
{
    unsigned short type;
    unsigned short num;
    int (*ams_pre_init)(int index);
    int (*ams_driver_init)(int index);
    int (*ams_enable)(int index);
    int (*ams_disable)(int index);
    int (*ams_info_get)(int index, ams_info_t *info);
    long (*ams_led_proc)(int index);
}ams_fix_param_t;


#define   SUBSLOT_MAX_NUM 9
typedef struct sub_board_fix_param_s
{
    int   sub_slotnum;
    int   sub_slot_portnum;
    int   sub_slot_port_map[SUBSLOT_MAX_NUM]; /*sub slot id map to sub slot port id*/
	long (*sub_slot_inserted)(unsigned long sub_slot);	 
	long (*sub_slot_distributed)(unsigned long sub_slot);
    long (*sub_slot_insert_detect)();  /*the isr connect and detect thread start*/
    long (*sub_slot_board_type_get)(unsigned long sub_slot, unsigned long *type);
    long (*sub_slot_man_param_get)(unsigned long sub_slot, board_man_param_t *man_param);
    long (*sub_slot_board_init)(unsigned long sub_slot);
    
}sub_board_fix_param_t;

typedef struct fiber_module_man_param_s
{
	int port_type;
	int alarm_type;

	int temperature;/* C */
	int voltage; /* V */
	int tx_bias[4]; /* mA */
	int tx_power[4]; /* mw or dBM */
	int rx_power[4]; /* mw or dBM */

	//int transceiver_type;
	//int connector_type;
	unsigned int identifier;
	unsigned int connector;
	unsigned int transceiver_class;
	int wavelength; /* nm */
	int ddm_implemented; /* 1:YES, 0:NO */

	int transmission_media;
	int smf_km_length;
	int smf_100m_length;
	int om2_length;
	int om1_length;
	int om4_or_copper_length;
	int om3_length;
	
	char vendor_name[16];
	char vendor_pn[16];
	char vendor_sn[16];
	char date_code[12];
}fiber_module_man_param_t;

typedef struct fiber_module_fix_param_s
{
    long (*fiber_module_inserted)(unsigned long panel_port);
    long (*fiber_module_insert_detect)();
    long (*fiber_module_info_get)(unsigned long panel_port, fiber_module_man_param_t* param);
}fiber_module_fix_param_t;

#define CTRL_CHAN_MAX_NUM 3
#define CTRL_CHAN_MASTER 1
#define CTRL_CHAN_SLAVE  2

#define CTRL_CHAN_ALL_LINE_BOARD -1
#define CTRL_CHAN_OTHER_MCU_BOARD -2

typedef struct ipp_fix_param_s
{
    int   ipp_portnum;
    char*   ipp_phyport_map[CTRL_CHAN_MAX_NUM]; /*ctrl chan id to physical port id*/

    int   ipp_board_map[CTRL_CHAN_MAX_NUM];
    
}ipp_fix_param_t;

#define MAX_TEMPERATURE_SENSOR  2  
typedef struct temper_fix_param_s
{
    int num;
    char *name[MAX_TEMPERATURE_SENSOR];
}temper_fix_param_t;

typedef enum pse_type_s
{
	PSE_IEEE_POE,		/* Support IEEE 802.3af standard  */	
	PSE_IEEE_POEPLUS 	/* Support IEEE 802.3at standard  */
}pse_type_t;


typedef struct poe_module_fix_param_s
{
	 int poe_ports[2];
	 int  pse_total_power;
	 int  pse_guard_band;
	 unsigned char pse_mpss;
	 pse_type_t pse_type;
	 long (*poe_init)();
	 long (*poe_operate)(void *in_data, void *out_data);
}poe_module_fix_param_t;

/*for led status*/
#define SYSTEM_LED_POWER_STATUS   0xf   /*4 bits to store up to 4 power supply status*/
#define SYSTEM_LED_POWER_BITS      4
#define SYSTEM_LED_FAN_STATUS     0x30   /*2 bits to store up to 2 fan unit status*/
#define SYSTEM_LED_FAN_BITS        6
#define SYSTEM_LED_TEMPE_STATUS   0xb0   /*2 bits to store up to 2 temperature status*/
#define SYSTEM_LED_TEMPE_BITS      8
#define SYSTEM_LED_RUN_STATUS     0xf00 /*4 bits to store system running status*/
#define SYSTEM_LED_RUN_BITS        12
#define SYSTEM_LED_RESERVED       0xffffffff>>12  /*others bits to reservied*/

typedef struct board_fix_param_s
{
    int board_code;
    int board_type;
    char *full_name;
    char *short_name;

    int   have_pp;
    BOOL   master_flag;
#define SERVICE_AS_EXTERNAL_SYSTEM  1    
#define SERVICE_AS_INTERNAL_SYSTEM  2
#define SERVICE_AS_PORT_EXT          3
#define SERVICE_AS_INDEPENDENT_SYSTEM  4
    int   service_flag;  /*service board*/
    int   panel_portnum;

    /*sub board manage params*/
    sub_board_fix_param_t *subboard_fix_param;

    /*Internal path protocol param*/
    ipp_fix_param_t *ipp_fix_param;
	
    temper_fix_param_t *temper_fix_param;
	long (* os_upgrade)(unsigned int slot_index);
	long (* board_ready_config)(unsigned int slot_index);
    board_feature_t *feature;
    sdktype_t sdk_type;
    int *board_support_product;
	int (* board_support_slot)(int product_type, int slot_id, int subslot_id);
	long (* get_image_info)(char *name, char *version,unsigned int *time,char *board_type);	
}board_fix_param_t;

typedef struct board_spec_fix_param_s
{
    int board_type;
    /*SFP/XFP/GBIC management params*/
    fiber_module_fix_param_t *fiber_module_fix_param; 
	
	/*poe management params*/
    poe_module_fix_param_t *poe_module_fix_param;

    ams_fix_param_t  *ams_param[ASIC_TYPE_MAX];

    long (*slotno_get)();
    long (*reset)();
    long (*get_reset_type)();

    long (*sys_led_lighting)(unsigned long status);
    long (*pne_monitor_start)();

    long (*board_man_param_get)(board_man_param_t *param);
    long (* local_conn_init)(int product_type);
    long (* system_conn_init) (int product_type, int insert_board_type, int insert_slotid);
    long (* system_conn_deinit) (int product_type, int delete_board_type, int delete_slotid);
    long (* asic_config_init)();
    long (* asic_after_data_sync_conf)();
	long (* dual_master_handle)(int slot_id);    /*出现双主控时对主控之间的物理通路的恢复函数*/
    long (* master_switch)(int product_type, int switch_board_type, int switch_slotid);
}board_spec_fix_param_t;

enum
{
    BOARD_EMPTY,
    BOARD_INSERTED
};

struct board_param_s;

#define MAX_SFP_NUM_PER_BOARD 64
#define MAX_ASIC_NUM_PER_BOARD   8        

typedef long (*event_handle_func_t)(struct board_param_s *board, struct board_param_s *src_board, state_event_e event, char* pdu);
typedef event_handle_func_t state_event_func_t[BOARD_EVENT_MAX];

typedef struct state_desc_s
{
    int state;
    char *description;
    state_event_func_t *funcs;
}state_desc_t;

typedef struct board_param_s
{
    unsigned short  slot_index;
    unsigned int  inserted;
    unsigned int online_removed;
	unsigned long configure_type;
	unsigned long running_time;
	unsigned long activepriority;
	board_role_t workmode;
	master_role_t redundancystate;
	int runstate;
    int *rmtstate;
    state_desc_t *state_function;
	float cpuusage;
    int reset_times;
    
    board_fix_param_t *fix_param;
    board_spec_fix_param_t *fix_spec_param;

    fiber_module_man_param_t fiber_module_info[MAX_SFP_NUM_PER_BOARD];

    ams_info_t ams_info[ASIC_TYPE_MAX][MAX_ASIC_NUM_PER_BOARD];

    board_man_param_t man_param;

    tempe_man_param_t temperature[MAX_TEMPERATURE_SENSOR];

    unsigned long led_status;

    struct board_param_s *sub_board[SUBSLOT_MAX_NUM];
    struct board_param_s *mother_board;

    void (*l3_srv_config)();
    int l3_srv_slot;
    int l3_srv_port;
}board_param_t;

typedef struct family_spec_fix_param_s
{
	unsigned long family_type;
	board_spec_fix_param_t ** board_spec_param_arr;
}family_spec_fix_param_t;

typedef struct family_common_fix_param_s
{
    unsigned long family_type;
	product_fix_param_t ** product_param_arr;
	product_conn_type_t ** product_conn_arr;
	board_conn_type_t ** board_conn_arr;
	board_conn_type_t ** board_conn_fullmesh_arr;
	board_fix_param_t ** board_param_arr;
}family_common_fix_param_t;

extern product_fix_param_t *snros_system_param;
extern board_fix_param_t  *snros_local_board;
extern board_spec_fix_param_t  *snros_local_board_spec;
extern board_fix_param_t **snros_board_param;
extern board_param_t *localmoduleinfo ;
extern board_param_t **chassis_slots ;
extern product_param_t productinfo;

#define PRODUCT_ID (productinfo.product_id)
#define PRODUCT_IS_CHASSIS (!PRODUCT_IS_BOX)
#define PRODUCT_MAC_ADDRESS (productinfo.base_mac)
#define PRODUCT_SYSTEM_NAME (productinfo.sys_info.name)

#define chassis_sys_reset \
    (snros_local_board_spec->reset)
#define chassis_sys_get_reset_type \
    (snros_local_board_spec->get_reset_type)
#define chassis_sys_reset_ext \
    (snros_system_param->board_manage->board_reset)

#define ipp_ioctl \
    (snros_local_board_spec->ipp_fix_param->ipp_ioctl)
#define ipp_transmit \
    (snros_local_board_spec->ipp_fix_param->ipp_transmit)
#define ipp_rtnhook \
    (snros_local_board_spec->ipp_fix_param->ipp_rtnhook)

#define chassis_manage_eth_ioctl \
    (snros_local_board_spec->manage_eth_fix_param->manage_eth_ioctl)
#define chassis_manage_eth_transmit \
    (snros_local_board_spec->manage_eth_fix_param->manage_eth_transmit)
#define chassis_manage_eth_rtnhook \
    (snros_local_board_spec->manage_eth_fix_param->manage_eth_rtnhook)
#define chassis_manage_eth_recv_enable_set \
    (snros_local_board_spec->manage_eth_fix_param->manage_eth_recv_enable_set)
#define chassis_manage_eth_rtnlinkchange \
    (snros_local_board_spec->manage_eth_fix_param->manage_eth_rtnlinkchange)

#define chassis_get_product_sysparam \
    (snros_system_param->product_man_param_get)
    
#define chassis_get_local_sysparam \
    (snros_local_board_spec->board_man_param_get)

#define SYS_PRODUCT_BASEMAC (productinfo.base_mac)
#define SYS_ENTERPRISE_NAME (productinfo.sys_info.enterprise_name?productinfo.sys_info.enterprise_name:"")

/*the following is global slot information*/
#define SYS_MASTER_ACTIVE_SLOTNO_UNKNOWN 0x00

extern product_fix_param_t **product_type_array;
extern board_fix_param_t **module_basic_info;
extern board_spec_fix_param_t **module_spec_info;

#define NPD_FDB_TABLE_SIZE (snros_system_param->product_pp_feature->max_macaddr)
#define NPD_FDB_BOARD_TABLE_SIZE (snros_local_board->feature->max_macaddr)

#define NPD_ROUTE_TABLE_SIZE (snros_system_param->product_pp_feature->max_ip_route\
                                    + snros_system_param->product_pp_feature->max_ip_route_ext)
#define NPD_ROUTE_V6_TABLE_SIZE  (snros_system_param->product_pp_feature->max_ipv6_route\
                                    + snros_system_param->product_pp_feature->max_ipv6_route_ext)
#define NPD_ROUTE_PER_ECMP (snros_system_param->product_pp_feature->max_ip_route_per_ecmp)
#define NPD_MROUTE_TABLE_SIZE (snros_system_param->product_pp_feature->max_ipmc_extra\
                                    + snros_system_param->product_pp_feature->max_ipmc_extra_ext)
#define NPD_ARPSNP_TABLE_SIZE (snros_system_param->product_pp_feature->max_ip_host_extra\
                                    + snros_system_param->product_pp_feature->max_ip_host_extra_ext)
#define NPD_NDISCSNP_TABLE_SIZE (snros_system_param->product_pp_feature->max_ipv6_host_extra\
                                    + snros_system_param->product_pp_feature->max_ipv6_host_extra_ext)
#define NPD_L3INTF_SIZE        (snros_system_param->product_pp_feature->max_ip_intf\
                                    + snros_system_param->product_pp_feature->max_ip_intf_ext)
#define NPD_VLAN_TABLE_SIZE   (snros_system_param->product_pp_feature->max_normal_vlan)                                  
#define NPD_MAX_VLAN_ID 4095
#define NPD_MAX_PROTO_VLAN_ID (snros_system_param->product_pp_feature->max_protobased_vlan-1)
#define PROTO_VLAN_TABLE_SIZE (snros_system_param->product_pp_feature->max_protobased_vlan)
#ifdef HAVE_CHASSIS_SUPPORT
#define PROTO_VLAN_PORT_TABLE_SIZE 1024
#else
#define PROTO_VLAN_PORT_TABLE_SIZE 16
#endif
#define NPD_SUPPORT_ECMP (snros_system_param->product_pp_feature->max_ip_route_per_ecmp != 0)
#define NPD_ROUTE_USE_ACL (snros_system_param->product_pp_feature->acl_as_route)

/*mac based vlan db function*/
#define NPD_MACBASE_VLAN_SIZE  (snros_system_param->product_pp_feature->max_macbased_vlan)
#define NPD_SUBNETBASE_VLAN_SIZE (snros_system_param->product_pp_feature->max_ipsubnet_vlan)

#define VLAN_XLATE_TABLE_SIZE (snros_system_param->product_pp_feature->max_qinq)
#define VLAN_ELINE_TABLE_SIZE (snros_system_param->product_pp_feature->max_vlanswitch_ingress)    

#define NPD_IGMP_SNP_TABLE_SIZE (snros_system_param->product_pp_feature->max_l2mc)
#define MAX_MST_TABLE_SIZE (snros_system_param->product_pp_feature->max_stpg)

#define NPD_SUPPORT_PREINGRESS_FILTER   (snros_local_board->feature->preingress_filter != 0)
#define NPD_SUPPORT_INGRESS_FILTER      (snros_local_board->feature->ingress_filter != 0)
#define NPD_SUPPORT_EGRESS_FILTER       (snros_local_board->feature->egress_filter != 0)

#define NPD_SUPPORT_SG_FILTER (snros_local_board->feature->sg_filter != 0)

#define MAX_COUNTER_NUM   (snros_system_param->product_pp_feature->max_qos_counter)
#define MAX_POLICER_NUM   (snros_system_param->product_pp_feature->max_qos_meter)
#define MAX_ING_QUEUE_NUM (snros_system_param->product_pp_feature->max_rxqueue_system)
#define MAX_ING_QUEUE_PER_PORT (snros_system_param->product_pp_feature->max_rxqueue_perport)
#define MAX_QUEUE_PER_GROUP 4
#define MAX_EGR_QUEUE_NUM  (snros_system_param->product_pp_feature->max_txqueue_system)
#define MAX_EGR_QUEUE_PER_PORT (snros_system_param->product_pp_feature->max_txqueue_perport)
                                   
#define MAX_CLASSMAP_INDEX_NUM	(localmoduleinfo->fix_param->feature->max_ingress_acl_std \
                                      + localmoduleinfo->fix_param->feature->max_ingress_acl_std_ext\
                                      + localmoduleinfo->fix_param->feature->max_egress_acl_std)
#define MAX_TUNNEL_TABLE_SIZE  (snros_system_param->product_pp_feature->max_tunnel)                                    

#define NPD_L3_SRV_SLOT  (localmoduleinfo->l3_srv_slot)
#define NPD_L3_SRV_PORT  (localmoduleinfo->l3_srv_port)

#define ROUTE_TABLE_START_ID (snros_system_param->product_pp_feature->ip_route_start_id)
#define ROUTE_TABLE_END_ID (snros_system_param->product_pp_feature->ip_route_end_id)
#define ACL_TABLE_START_ID (snros_system_param->product_pp_feature->acl_start_id)
#define ACL_TABLE_END_ID (snros_system_param->product_pp_feature->acl_end_id)
#define MACBASE_VLAN_TABLE_START_ID (snros_system_param->product_pp_feature->macbased_vlan_start_id)
#define MACBASE_VLAN_TABLE_END_ID (snros_system_param->product_pp_feature->macbased_vlan_end_id)
#define IPSUBNET_VLAN_TABLE_START_ID (snros_system_param->product_pp_feature->ipsubnet_vlan_start_id)
#define IPSUBNET_VLAN_TABLE_END_ID (snros_system_param->product_pp_feature->ipsubnet_vlan_end_id)

#define MIRROR_MAX_DEST_PORT (snros_system_param->product_pp_feature->max_mirror_des_port)
#define MIRROR_VLAN (snros_system_param->product_pp_feature->vlan_mirror)
#define MIRROR_FDB (snros_system_param->product_pp_feature->fdb_mirror)
#define MIRROR_EG_PCL (snros_system_param->product_pp_feature->pcl_eg_mirror)
#define REDIRECT_EG_PCL (snros_system_param->product_pp_feature->pcl_eg_redirect)

#define NPD_TRUNK_NUMBER_MAX (snros_system_param->product_pp_feature->max_trunk)
#define NPD_TRUNKID_END (NPD_TRUNK_NUMBER_MAX - 1)
#define NPD_VLAG_NUMBER_MAX 64

#define NPD_BOARD_CTRL_NUM  	(snros_local_board->feature->ctrl_num)
#define NPD_BOARD_CTRL_SWITCH  	(snros_local_board->feature->ctrl_switch)

#define BOARD_CAN_SLAVE_INDPT	(snros_local_board->feature->can_slave_indpt)
#define NPD_ACL_BASED_VLAN_SUPPORT ((snros_system_param->product_pp_feature->support_acl_based_vlan) \
                                    & (snros_local_board->feature->support_acl_based_vlan))
#define NPD_ACL_POLICY_ROUTE_SUPPORT (snros_local_board->feature->support_acl_policy_route)
#define TCP_CMP_MAX     (snros_system_param->product_pp_feature->num_of_tcp_comparator)
#define UDP_CMP_MAX     (snros_system_param->product_pp_feature->num_of_udp_comparator)

#define VCT_IS_AVAILABLE     (snros_local_board->feature->vct_available)

#endif
