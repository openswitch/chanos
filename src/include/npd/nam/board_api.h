#ifndef __NPD_BOARD_API_H__
#define __NPD_BOARD_API_H__

int npd_board_service_traffic_set
(
	unsigned int slot_id, 
	unsigned short vlanId, 
	unsigned int endis
);

#define FW_GROUP_SLOT_ID 0x0

int npd_service_board_init();

int npd_board_service_vlan_check(unsigned short vlanId);

int npd_board_service_slot_check(unsigned short vlanId, int *slot_id);

int npd_serv_board_vlan_change(unsigned short vid, unsigned char flag);

int npd_board_service_state_change(int slot_id, int state);
int npd_board_service_delete(int slot_id );

int npd_board_service_set(unsigned int slot_id, unsigned int service_type );

unsigned int npd_query_sw_version(int board_type);

#endif

