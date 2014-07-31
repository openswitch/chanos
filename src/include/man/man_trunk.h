#ifndef _MAN_TRUNK_H_
#define _MAN_TRUNK_H_

/********previous defined in dcli_lacp.h********************************/
#define ETH_PORT  2
#define TRUNK     4
#define MANUAL_MODE   0
#define STATIC_MODE   1
#define DYNAMIC_MODE  2

#define TRUE          1
#define FALSE         0
#define SUCCESS       0
#define FAIL         -1


int dcli_create_trunk_by_id_name(unsigned short trunkId, char *trunkName);

int dcli_create_trunk_by_name(char *trunkName, unsigned short *trunkId, unsigned int *route);

int dcli_create_trunk_by_id(unsigned short trunkId, unsigned int *route);

int dcli_add_del_trunk_member(unsigned char isAdd, unsigned int eth_g_index, unsigned short trunkId);

int dcli_delete_trunk_by_id(unsigned short trunkId);

int dcli_delete_trunk_by_name(char *trunkName);

int dcli_get_aggregator_by_trunk_id
(
	unsigned short trunk_id, 
	unsigned int *lacp_enable_state, 
	unsigned char en_actor_status[], 
	unsigned char dis_actor_status[],
	unsigned short key_port_manual[],
	struct lacpport_actor en_trunk_actor[], 
	struct lacpport_actor dis_trunk_actor[], 
	struct lacpport_partner trunk_partner[], 
	struct aggregator *trunk_aggregator
);

int dcli_get_trunk_by_id(unsigned short trunk_id, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag);

int dcli_get_trunk_by_name(char *trunkName, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag);

int dcli_config_trunk_load_balanc(unsigned short trunkId, unsigned int loadBalanMode);

int dcli_get_trunklist(unsigned short trunk_id, trunk_t *trunk_info, unsigned int enport_index[], unsigned int disport_index[], unsigned char *masterFlag);

int dcli_config_trunk_update_name(unsigned int netif_index, char *trunkName);

int dcli_check_trunk_exist(unsigned short trunk_id);

int dcli_config_trunk_update_name_by_id(unsigned short trunk_id, char *trunk_name);

int dcli_create_trunk_intf(unsigned short trunk_id);

int dcli_delete_trunk_intf(unsigned short trunk_id);


#endif
