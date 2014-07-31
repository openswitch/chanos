#ifndef __MAN_LACP_H__
#define __MAN_LACP_H__

#define ETH_PORT  2
#define TRUNK     4

#define TRUE          1
#define FALSE         0
#define SUCCESS       0
#define FAIL         -1


int dcli_config_port_lacp_endis(unsigned int eth_g_index, unsigned int enDis);

int dcli_config_all_port_lacp_enDis(unsigned int enDis);
int dcli_config_aggregation_mode(unsigned short trunk_id, unsigned int enDis);

#endif
