#ifndef __COMMON_BOARD_H__
#define __COMMON_BOARD_H__

struct stk_port_s {
	unsigned int netif_index;
	int state;
	int trunkid;	
	int link_state;	
	int peer_slot;
};

typedef struct stk_trunk_s {
	unsigned int trunk_id;
    char name[31];	
	unsigned int g_index;
	int state;
	unsigned int linkstate;
    npd_pbmp_t   ports;	
	unsigned int master_port_index;
	int load_balance_mode;
}stk_trunk_t;


typedef enum service_board_type_e {
	SERVICE_BOARD_TYPE_NONE,
	SERVICE_BOARD_TYPE_FIREWALL,
	SERVICE_BOARD_TYPE_AC,
	SERVICE_BOARD_TYPE_DPI,
	SERVICE_BOARD_TYPE_MAX
}service_board_type;


/*service board struct definition*/
typedef struct service_board_db_s
{
	unsigned char state;
	unsigned char flow_recover;	
	unsigned int slot_id;
	unsigned int backup_slotid;
	unsigned int service_flag;
	npd_vbmp_t   vlanbmp;
    unsigned int load_balance_mode;	
}serv_board_db_t;

#endif

