#ifndef __NPD_STP_API_H__
#define __NPD_STP_API_H__

/*NPD LAYER API*/

void npd_smart_link_stp_flag
(
	unsigned int netif_index,
	unsigned int mstid,
	unsigned int is_enable,
	unsigned int stp_port_status
);

int npd_smart_link_msg_init(void);

int npd_smart_link_packet_rx_process
(
    int packet_type,
    unsigned char* packet,
    int length,
    unsigned int netif_index,
    unsigned short vlan_id,
    char is_tagged
);

void npd_erpp_stp_flag
(
	unsigned int netif_index,
	unsigned int mstid,
	unsigned int is_enable,
	unsigned int stp_port_status
);

int npd_erpp_init();

void npd_init_stp(void);

int	npd_rstp_msg_init();

int npd_mstp_get_port_state
(
    unsigned short vid,
    unsigned int port_index,
    NAM_RSTP_PORT_STATE_E*  state 
);

int npd_udld_init();

/*NAM LAYER API*/

enum{
    NAM_STG_STP_DISABLE = 0, /* Disabled. */
    NAM_STG_STP_BLOCK , /* BPDUs/no learns. */
    NAM_STG_STP_LISTEN , /* BPDUs/no learns. */
    NAM_STG_STP_LEARN , /* BPDUs/learns. */
    NAM_STG_STP_FORWARD, /* Normal operation. */
    NAM_STG_STP_COUNT  
};


int nam_asic_stp_port_enable
(
    unsigned int eth_g_index, 
   unsigned char stp_enable
);
int  nam_vlan_stpid_get
(
     unsigned short vid,
     unsigned int* stg
);
int nam_stp_state_get
(
    unsigned int eth_g_index, 
    unsigned short stg,
    unsigned int* stpState
);
int nam_stp_state_set
(
    unsigned int eth_g_index, 
    unsigned short stg,
    unsigned int stpState

);

int nam_stp_vlan_bind_stg
(
   unsigned short vlanid,
   unsigned int stg
);

int nam_stp_vlan_unbind_stg
(
    unsigned char devNum,
    unsigned short vlanid,
    unsigned int stg
);

int nam_stg_check
(
   int unit,
   int stg
);

int nam_asic_stp_port_enable
(
    unsigned int eth_g_index, 
	unsigned char stp_enable
);

int nam_asic_stp_port_state_update
(
	unsigned char devNum,
	unsigned int portId,
	unsigned char stpState
);

int nam_asic_stp_init
(
	void
);
#endif

