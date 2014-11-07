#ifndef _MAN_ETH_PORT_H_
#define _MAN_ETH_PORT_H_

//originally defined in dcli_eth_port.h, 
#define DCLI_ERROR_NONE          0 
#define DCLI_ERROR_DBUS         (DCLI_ERROR_NONE + 1)

//original difined in command.h 
#define SHOWRUN_PERLINE_SIZE  81


int dcli_eth_port_get_next_g_index(unsigned int eth_g_index, unsigned int *out_eth_g_index);

int dcli_get_eth_port_attr(unsigned int eth_g_index, struct eth_port_s *port_info, unsigned char *lacp_sta, unsigned char *gvrp_sta);

int dcli_get_eth_port_ipg(unsigned int eth_g_index, unsigned char *port_ipg);

int dcli_eth_port_get_one_port_index(unsigned int port_index, unsigned int* netif_index) ;

int dcli_get_switchport(unsigned int netif_index, struct switch_port_db_s *port, char *untag_vlan_list, char *tag_vlan_list);

int dcli_get_eth_port_stat(unsigned int eth_g_index, eth_port_stats_t *ptr, unsigned int *linkupcount, unsigned int *linkdowncount);

int dcli_get_eth_port_rate(unsigned int eth_g_index, unsigned int *inbandwidth,unsigned int *outbandwidth);

int dcli_get_eth_port_sfp_atrr(unsigned int eth_g_index, eth_port_sfp *sfpInfo);

int dcli_get_eth_port_transceiver_atrr(unsigned int eth_g_index, fiber_module_man_param_t *tcvInfo);
	
int clear_eth_port_stat(unsigned int eth_g_index);

int dcli_eth_port_config_buffer_mode(unsigned char isEnable);

int dcli_config_eth_port(unsigned int eth_g_index, unsigned int *route, unsigned int *out_eth_g_index);

int dcli_config_eth_port_descr(unsigned int eth_g_index, char *descr);

int dcli_port_ratelimit_set(unsigned int eth_g_index, int flow_dir, unsigned int bandwidth, unsigned int burstsize);

int dcli_config_eth_port_mode(unsigned int eth_g_index, unsigned int mode);

int dcli_eth_port_admin_set(unsigned int eth_g_index, int mode);

int dcli_config_ethport_ipg(unsigned int port_index, unsigned char port_ipg);

int dcli_config_eth_port_link_state(unsigned int port_index, unsigned int isEnable, unsigned int *out_port_index);

int dcli_config_ethport_attr(unsigned int type, unsigned int port_index, unsigned int value, unsigned int *out_port_index);

int dcli_ethport_mtu_set(unsigned int port_index, int mtu, unsigned int *out_port_index);

int dcli_config_eth_port_eee(unsigned int port_index, unsigned int isEnable);

int dcli_eth_port_get_running_config(unsigned int eth_g_index, char *showStr);

int dcli_eth_port_get_next_g_index(unsigned int eth_g_index, unsigned int *out_eth_g_index);

unsigned int dcli_eth_port_config_sc_common(unsigned char modeType, unsigned int g_index, unsigned int scMode, unsigned int scType, unsigned int scValue, unsigned int* ret);

unsigned int dcli_global_config_sc_common(unsigned int scType, unsigned int* ret);

int dcli_eth_port_config_vct(unsigned int eth_g_index, unsigned int mode, unsigned int *out_eth_g_index);

int dcli_eth_port_get_vct_info(unsigned int port_index, unsigned int *Isable, unsigned short *state, unsigned int *len, unsigned int *result, unsigned int *out_port_index);



unsigned int dcli_get_ethport_number(unsigned int eth_g_index);

int dcli_get_port_stat_by_slotno_and_portno
(
	unsigned char slot_no, 
	unsigned char port_no, 
	struct eth_port_counter_s *ptr,
	unsigned int *linkupcount, 
	unsigned int *linkdowncount,
	unsigned int *link_state, 
	unsigned char *mod,
	unsigned char *dev,
	unsigned char *port	
);
int dcli_get_next_stack_port
(
	unsigned char slot_no, 
	unsigned char port_no, 
	unsigned char *out_slot_no,
	unsigned char *out_port_no,
	int *devNum,
	int *devPort,
	int *module_id
);

int dcli_clear_stack_stat_by_slotno_and_portno(unsigned char slot_no, unsigned char port_no);

#endif
