#ifndef __NPD_ETH_PORT_API_H__
#define __NPD_ETH_PORT_API__H__

/* NPD Layer Function*/

extern NPD_ETH_PORT_NOTIFIER_FUNC	portNotifier;
extern sequence_table_index_t *g_eth_ports;

#define ETH_PORT_TYPE_FROM_GLOBAL_INDEX(eth_g_index) eth_port_sw_type_get(eth_g_index)
#define ETH_PORT_ATTRBITMAP_FROM_GLOBAL_INDEX(eth_g_index) eth_port_sw_attr_bitmap_get(eth_g_index)
#define ETH_PORT_MTU_FROM_GLOBAL_INDEX(eth_g_index) eth_port_sw_mtu_get(eth_g_index)
#define ETH_PORT_IPG_FROM_GLOBAL_INDEX(eth_g_index) eth_port_sw_ipg_get(eth_g_index)
#define ETH_PORT_LINK_TIME_CHANGE_FROM_GLOBAL_INDEX(eth_g_index) eth_port_sw_link_time_change_get(eth_g_index)

#define ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slotno,portno) ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_INDEX(CHASSIS_SLOT_NO2INDEX(slotno),ETH_LOCAL_NO2INDEX(CHASSIS_SLOT_NO2INDEX(slotno),(portno)), 0)


#define MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index) npd_get_port_driver_type(eth_g_index)
#define IS_MODULE_DRIVER_TYPE_NBM(eth_g_index) (MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index) == MODULE_DRIVER_NBM)

#define SYS_ETH_PORT_ITER(eth_g_index) \
{ \
	int i_i_i=0, j_j_j = 0, k_k_k = 0, l_l_l = 0; \
	for(i_i_i = 0; i_i_i < MAX_CHASSIS_COUNT; i_i_i++) \
		for(j_j_j = 0; j_j_j < MAX_CHASSIS_SLOT_COUNT; j_j_j++) \
			for(k_k_k = 0; k_k_k < ETH_LOCAL_PORT_COUNT(j_j_j); k_k_k++) \
    			for(l_l_l = 0; l_l_l < MAX_SUBPORT_PER_ETHPORT; l_l_l++) \
    			{ \
    				eth_g_index = ETH_GLOBAL_INDEX(i_i_i, j_j_j, 0, k_k_k, l_l_l);
			
#define SYS_ETH_PORT_ITER_END \
			} \
}

#define LOCAL_CHASSIS_ETH_PORT_ITER(eth_g_index) \
{ \
	int i_i_i=0, j_j_j = 0, k_k_k = 0; \
	for(i_i_i = 0; i_i_i < MAX_CHASSIS_SLOT_COUNT; i_i_i++) \
		for(j_j_j = 0; j_j_j < ETH_LOCAL_PORT_COUNT(i_i_i); j_j_j++) \
    		for(k_k_k = 0; k_k_k < MAX_SUBPORT_PER_ETHPORT; k_k_k++) \
    		{ \
    			eth_g_index = ETH_GLOBAL_INDEX(SYS_LOCAL_CHASSIS_INDEX, i_i_i, 0, j_j_j, k_k_k);
		
#define LOCAL_CHASSIS_ETH_PORT_ITER_END \
		} \
}

#define LOCAL_SLOT_ETH_PORT_ITER(eth_g_index) \
{ \
	int i_i_i=0, j_j_j = 0; \
	for(i_i_i = 0; i_i_i < MAX_ETHPORT_PER_BOARD; i_i_i++) \
    	for(j_j_j = 0; j_j_j < MAX_ETHPORT_PER_BOARD; j_j_j++) \
    	{ \
    		eth_g_index = ETH_GLOBAL_INDEX(SYS_LOCAL_CHASSIS_INDEX, SYS_LOCAL_MODULE_SLOT_INDEX, 0, i_i_i, j_j_j);
	
#define LOCAL_CHASSIS_ETH_PORT_ITER_END \
	} \
}
		
extern int npd_get_port_driver_type(unsigned int eth_g_index);
enum eth_port_type_e npd_get_port_type
(
	int module_type, 
	int eth_port_index
) ;

int npd_port_media_get
(
	unsigned int eth_g_index,
	enum eth_port_type_e *portMedia
);

void npd_create_eth_port
(
    unsigned int chassis,
	unsigned int slot_index,
	unsigned int sub_slot,
	unsigned int eth_local_index,
    unsigned int sub_port,
	int state
);


void npd_init_eth_ports(void) ;
extern void npd_init_subslot_eth_ports(int chassis, int slot, int subslot, int state);

extern void npd_remove_subslot_eth_ports(int chassis, int slot, int subslot, int state);
extern void npd_delete_subslot_eth_ports(int chassis, int slot, int subslot, int state);
extern void npd_insert_subslot_eth_ports(int chassis, int slot, int subslot, int state);


struct eth_port_s *npd_get_port_by_index
(
	unsigned int eth_g_index
);


unsigned int npd_eth_port_set_pvid
(
	unsigned int   eth_g_index,
	unsigned short vlanId
);

unsigned int npd_eth_port_get_pvid
(
	unsigned int   eth_g_index,
	unsigned short *vlanId
);

unsigned int npd_eth_port_get_ptrunkid
(
	unsigned int   eth_g_index,
	unsigned short *trunkId
);

int npd_get_port_admin_status
(
	unsigned int eth_g_index,
	unsigned int *status
) ;

int npd_set_port_admin_status
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_link_status
(
	unsigned int eth_g_index,
	int *status
) ;

int npd_set_port_link_status
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_autoNego_status
(
	unsigned int eth_g_index,
	unsigned int *status
) ;

int npd_set_port_autoNego_status
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_set_port_autoNego_duplex
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_set_port_autoNego_speed
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_set_port_autoNego_flowctrl
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_duplex_mode
(
	unsigned int eth_g_index,
	unsigned int *mode
) ;

int npd_set_port_duplex_mode
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_speed
(
	unsigned int eth_g_index,
	unsigned int *speed
) ;

int npd_set_port_speed
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_flowCtrl_state
(
	unsigned int eth_g_index,
	unsigned int *status
) ;

int npd_set_port_flowCtrl_state
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_backPressure_state
(
	unsigned int eth_g_index,
	unsigned int *status
) ;

int npd_set_port_backPressure_state
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_get_port_mru
(
	unsigned int eth_g_index,
	unsigned int *mru
) ;

int npd_set_port_mru
(
	unsigned int eth_g_index,
	unsigned int status
);

int npd_set_port_attr_default
(
	unsigned int eth_g_index
);

int npd_check_eth_port_status
(
	unsigned int eth_g_index
) ;

int npd_eth_port_register_notifier_hook
(
	void
);

int npd_eth_port_counter_statistics
(
	unsigned int eth_g_index,
	eth_port_stats_t *portPtr
);

void npd_eth_port_status_polling_thread
(
	void
);

int npd_port_free_vlan(
    unsigned int eth_g_index,
    int vid,
    int isTagged
    );
int npd_port_allow_vlan(
    unsigned int eth_g_index,
    int vid,
    int isTagged
    );

unsigned int npd_eth_port_get_slotno_portno_by_eth_g_index
(
    unsigned int eth_g_idex,
    unsigned int * slot_no,
    unsigned int * port_no
);

int npd_check_ethport_exist(unsigned int eth_g_index);


/* Declaration for other mod */
unsigned int eth_port_sw_attr_bitmap_get
(
	unsigned int eth_g_index
);
int npd_port_vlan_filter
(
    unsigned int netif_index,
    int flags
);
int npd_port_mac_vlan_enable
(
    unsigned int netif_index,
    int flags
);
int npd_port_subnet_vlan_enable
(
    unsigned int netif_index,
    int flags
);
int npd_port_prefer_subnet_enable
(
    unsigned int netif_index,
    int flags
);
int npd_port_access_qinq_enable
(
    unsigned int netif_index,
    int flags
);

unsigned int eth_port_local_check
(
	unsigned int eth_g_index
);
int npd_port_set_pvid
(
    unsigned int eth_g_index,
    int pvid
);
int npd_port_set_inner_pvid
(
    unsigned int eth_g_index,
    int pvid
);
int npd_port_set_inner_pvid
(
    unsigned int eth_g_index,
    int pvid
);
int npd_get_port_swspeed
(
    unsigned int eth_g_index,
    int *speed
);
int npd_port_isolate_add
(
    unsigned int src_netif_index,
    unsigned int dst_netif_index
);
int npd_port_isolate_del
(
    unsigned int src_netif_index,
    unsigned int dst_netif_index
);
int npd_port_dhcp_trap_set
(
    int vid,
    unsigned int netif_index,
    int flags
);
int npd_port_tpid_set
(
    unsigned int netif_index,
    unsigned short value
);
int npd_port_inner_tpid_set
(
    unsigned int netif_index,
    unsigned short value
);
int npd_port_set_vlan_pri(
    unsigned int eth_g_index,
    int vid,
    int pri,
    int cfi
);
int npd_port_fdb_learning_mode
(
    unsigned int eth_g_index,
    int mode
);
int npd_check_port_switch_mode
(
	unsigned int eth_g_index
);

void* eth_port_rate_poll_thread(void);

int npd_port_vlan_mode_set(unsigned int eth_g_index, int mode);

int npd_get_eth_port_route_mode(unsigned int eth_g_index, unsigned char *mode);

int npd_del_ethport_route_mode(unsigned int eth_g_index);

int npd_set_ethport_route_mode(unsigned int eth_g_index,unsigned int mode);

int npd_arp_packet_enable_netif(
    unsigned int netif_index,
    int enable);

int npd_switch_port_exist_check(unsigned int netif_index);

int npd_dhcp_packet_enable(int enable);

int npd_eth_port_startup_end_update();
int npd_put_port(struct eth_port_s* portInfo);
int npd_arp_packet_enable(
    unsigned int netif_index,
    int enable);

int npd_bpdu_packet_enable(
    unsigned int netif_index,
    int enable);

int npd_8021x_packet_enable(
    unsigned int netif_index,
    int enable);

int npd_nd_packet_enable(
    unsigned int netif_index,
    int enable);
int npd_stack_port_counter_statistics
(
	unsigned int slot_no, 
	unsigned int port_no,
	struct eth_port_counter_s *portPtr
);
unsigned int eth_port_legal_check(unsigned int eth_g_index);
int npd_eth_port_startup_end_update();
int eth_port_sw_duplex_get(unsigned int eth_g_index);
int eth_port_sw_speed_get(unsigned int eth_g_index);
int eth_port_sw_attr_update(unsigned int eth_g_index, unsigned int type, unsigned int value);


/*NAM LAYER API*/

#define	PORT_STORM_CONTROL_STREAM_DLF            0x01
#define	PORT_STORM_CONTROL_STREAM_MCAST          0x02
#define	PORT_STORM_CONTROL_STREAM_BCAST          0x04
#define	PORT_STORM_CONTROL_STREAM_UCAST          0x08  
#define	PORT_STORM_CONTROL_STREAM_ALL          (PORT_STORM_CONTROL_STREAM_DLF | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST| \
	                                            PORT_STORM_CONTROL_STREAM_UCAST) 
#define	ETH_PORT_SPEED_10000_BPS 10000000000
#define ETH_PORT_SPEED_1000_BPS 1000000000
#define	ETH_PORT_SPEED_100_BPS  100000000
#define ETH_PORT_SPEED_10_BPS  10000000



extern int nam_read_eth_port_info(unsigned int eth_g_index, int speed, struct eth_port_s *ethport);

extern int nam_port_inner_tpid_set(unsigned int eth_g_index, unsigned short value);
extern int nam_port_tpid_set(unsigned int eth_g_index, unsigned short value);
extern int nam_port_vlan_filter(unsigned int eth_g_index, int flags);
extern int nam_port_isolate_add(unsigned int src_netif_index, unsigned int dst_netif_index);
extern int nam_port_isolate_del(unsigned int src_netif_index,unsigned int dst_netif_index);
extern unsigned int nam_asic_igmp_trap_set_by_devport(
	unsigned short	vid,
	unsigned int eth_g_index,
    unsigned long 	enable
);
extern int npd_port_dhcp_trap_set(
    int vid,
    unsigned int netif_index,
    int flags
);

extern int nam_port_qinq_enable(unsigned int eth_g_index, int enable);
extern int nam_port_qinq_drop_miss_enable(unsigned int eth_g_index, int enable);
extern int nam_port_subnet_vlan_enable(unsigned int eth_g_index, int enable);
extern int nam_port_mac_vlan_enable(unsigned int eth_g_index, int enable);
extern int nam_port_mac_vlan_enable(unsigned int eth_g_index, int enable);
extern int nam_port_vlan_filter(unsigned int eth_g_index, int flags);
extern int nam_port_subnet_prefer(unsigned int eth_g_index, int enable);

int nam_set_ethport_ipg
(
	unsigned int eth_g_index,
	int speed,
	unsigned char  value
);

int nam_set_ethport_inband_autoneg
(
	unsigned int eth_g_index,
	unsigned long state
);

int nam_set_ethport_enable
(
	unsigned int eth_g_index,
	unsigned long	portAttr
);
int nam_set_ethport_force_linkup
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_set_ethport_force_linkdown
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_set_ethport_force_auto
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_set_ethport_duplex_autoneg
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_get_ethport_duplex_autoneg
(
	unsigned int eth_g_index,
	unsigned long *state
);
int nam_set_ethport_fc_autoneg
(
	unsigned int eth_g_index,
	unsigned long state,
	unsigned long pauseAdvertise
);
int nam_get_ethport_fc_autoneg
(
	unsigned int eth_g_index,
	unsigned long *state
);
int nam_set_ethport_speed_autoneg
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_get_ethport_speed_autoneg
(
	unsigned int eth_g_index,
	unsigned long *state
);
int nam_set_ethport_duplex_mode
(
	unsigned int eth_g_index,
	PORT_DUPLEX_ENT	dMode
);
int nam_set_ethport_speed
(
	unsigned int eth_g_index,
	PORT_SPEED_ENT speed
);
int nam_set_ethport_flowCtrl
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_set_ethport_PeriodFCtrl
(
	unsigned int eth_g_index,
	unsigned long enable
);
int nam_set_ethport_backPres
(
	unsigned int eth_g_index,
	unsigned long state
);
int nam_set_ethport_mru
(
	unsigned int eth_g_index,
	unsigned int mruSize
);
int nam_set_ethport_eee
(
    unsigned int eth_g_index,
    PORT_EEE_ENT dMode
);
unsigned int nam_get_port_en_dis 
(
	unsigned int eth_g_index,
	unsigned long* portAttr
);
unsigned int nam_get_port_link_state
(
	unsigned int eth_g_index,
	unsigned long* portAttr
);

unsigned int nam_get_port_autoneg_state
(
	unsigned int eth_g_index,
	unsigned long* portAttr
);
unsigned int nam_set_ethport_autoneg_state
(
    unsigned int eth_g_index,
    unsigned long state
);

int nam_combo_ethport_check
(
	unsigned int eth_g_index
);
int nam_set_phy_reboot
(
	unsigned int eth_g_index
);

unsigned int nam_get_port_duplex_mode
(
	unsigned int eth_g_index,
	unsigned int* portAttr
);
unsigned int nam_get_port_speed
(
	unsigned int eth_g_index,
	unsigned int* portAttr
);
unsigned int nam_get_port_config_speed
(
	unsigned int eth_g_index,
	unsigned int* portAttr
);
unsigned int nam_get_port_config_flowcontrol
(
	unsigned int eth_g_index,
	unsigned int* portAttr
);

unsigned int nam_get_port_flowCtrl_state
(
	unsigned int eth_g_index,
	unsigned long* portAttr
);
unsigned int nam_get_port_backPres_state
(
	unsigned int eth_g_index,
	unsigned long* portAttr
);
unsigned int nam_get_port_mru
(
	unsigned int eth_g_index,
	unsigned int* portMru
);

unsigned int nam_asic_port_pkt_statistic
(
	unsigned int eth_g_index,
	eth_port_stats_t *portPktCount
);

unsigned int nam_set_bcm_ethport_phy
(
	unsigned int eth_g_index
);

unsigned int nam_get_ethport_eee 
(
    unsigned int eth_g_index,
    unsigned int* portAttr
);

int nam_egress_filter_enable
(
	unsigned int eth_g_index,
    unsigned long enable
);
int nam_eth_port_sc_bps_set
(
    unsigned int eth_g_index,
    unsigned int cntype,
    unsigned int cnvalue,
    unsigned int portType
);

int nam_eth_port_sc_pps_set
(
    unsigned int eth_g_index,
    unsigned int cntype,
    unsigned int cnvalue,
    unsigned int portType
);

int nam_eth_port_sc_bps_set
(
    unsigned int eth_g_index,
    unsigned int cntype,
    unsigned int cnvalue,
    unsigned int portType
);
int nam_set_ethport_ipg
(
    unsigned int eth_g_index,
    int speed,
    unsigned char  value
);
int nam_port_phy_port_media_type
(
    unsigned int eth_g_index,
    enum eth_port_type_e 	*portMedia
);

int nam_vct_phy_enable
(
	unsigned int netif_index,
	unsigned int state
);

int nam_vct_phy_read
(
	unsigned int eth_g_index,
	unsigned short *state,
	unsigned int *len,
	unsigned int *result
);

int nam_eth_port_insert(unsigned int netif_index);
int nam_set_ethport_linkstate(unsigned int netif_index, unsigned char linkstate);
int nam_ethport_set_ipsg(unsigned int netif_index, int ip_sg);
int nam_eth_port_remove(unsigned int netif_index);
int nam_set_port_loopback(unsigned int netif_index, int mode);
int nam_get_port_loopback(unsigned int netif_index, int *mode);
unsigned int nam_get_stack_port_link_state
(
    unsigned char devNum,
    unsigned char portNum,
    unsigned long* portAttr
);

int nam_dhcp_packet_trap_cpu(
    unsigned int netif_index,
    int enable);

int nam_arp_packet_trap_cpu(
    unsigned int netif_index,
    int enable);


int nam_bpdu_packet_trap_cpu(
    unsigned int netif_index,
    int enable);


int nam_8021x_packet_trap_cpu(
    unsigned int netif_index,
    int enable);


int nam_nd_packet_trap_cpu(
    unsigned int netif_index,
    int enable);


int nam_igmp_packet_trap_cpu(
    unsigned int netif_index,
    int enable);




int nam_port_vlan_mode_set
(
	unsigned int netif_index, 
	int mode
);

unsigned int nam_asic_clear_port_pkt_stat
(
    unsigned int eth_g_index
);

int nam_get_ethport_ipg
(
    unsigned int eth_g_index,
    int speed,
    unsigned char  *portIPG
);

int nam_eth_port_forward_mode_set
(
	unsigned int netif_index,
	int mode
);

int nam_set_eth_port_trans_media
(
    unsigned int eth_g_index,
    unsigned int  media
);

unsigned int nam_asic_clear_stack_port_pkt_stat
(
	unsigned char devNum, 
	unsigned char portNum
);

unsigned int nam_asic_stack_port_pkt_statistic
(
    unsigned char devNum,
    unsigned char portNum,
	struct eth_port_counter_s *portPktCount    
);

int nam_get_eth_port_trans_media
(
    unsigned int eth_g_index,
    unsigned int  *media
);

int combo_port_active_medium_get(unsigned int eth_g_index, int *active_medium);
int nam_test_set_port_localswitch(unsigned char unit,unsigned char port, int en, int vid);
int nam_test_set_port_endis(unsigned char unit,unsigned char port, int en);
int nam_test_set_stack_eth_port(unsigned char unit, unsigned char port);
unsigned int nam_test_get_port_link_state
(
    unsigned char unit,
    unsigned char port,
    unsigned long* portAttr
);




#endif

