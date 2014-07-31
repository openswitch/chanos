#ifndef __NAM_ETH_H__
#define __NAM_ETH_H__

#define PORT_STATE_GET_SUCCESS	0x0
#define PORT_STATE_GET_FAIL		0xFF

/* NAM error macro */
#define NAM_ERR_NONE					0
#define	NAM_ERR_SLOT_OUT_OF_RANGE	(NAM_ERR_NONE + 1)
#define NAM_ERR_PORT_OUT_OF_RANGE	(NAM_ERR_NONE + 2)
#define NAM_ERR_MODULE_NOT_SUPPORT	(NAM_ERR_NONE + 3)
#define NAM_ERR_DEVICE_NUMBER			(NAM_ERR_NONE + 4)
#define NAM_ERR_PORT_ON_DEVICE		(NAM_ERR_NONE + 5)
#define NAM_ERR_NO_MATCH				(NAM_ERR_NONE + 6)
#define NAM_ERR_HW					(NAM_ERR_NONE + 7)
#define NAM_ERR_MAX					(NAM_ERR_NONE + 255)

#define NAM_ERR_UNSUPPORT           6
#define NAM_ERR_GENERNAL            1


extern char* nam_err_msg[];


/* Port type mode */
typedef enum {
   PORT_FE_TYPE_E = 0,
   PORT_GE_TYPE_E
}NAM_ETH_PORT_STREAM_TYPE_E;


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

extern int nam_eth_port_insert(unsigned int netif_index);
extern int nam_set_ethport_linkstate(unsigned int netif_index, unsigned char linkstate);
extern int nam_ethport_set_ipsg(unsigned int netif_index, int ip_sg);
extern int nam_eth_port_remove(unsigned int netif_index);
extern int nam_set_port_loopback(unsigned int netif_index, int mode);
extern int nam_get_port_loopback(unsigned int netif_index, int *mode);
extern unsigned int nam_get_stack_port_link_state
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
int npd_stack_port_counter_statistics
(
	unsigned int slot_no, 
	unsigned int port_no,
	struct eth_port_counter_s *portPtr
);

unsigned int nam_asic_stack_port_pkt_statistic
(
    unsigned char devNum,
    unsigned char portNum,
	struct eth_port_counter_s *portPktCount    
);

#endif
