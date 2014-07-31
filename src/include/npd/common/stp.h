#ifndef __COMMON_MSTP_H__
#define __COMMON_MSTP_H__

enum stp_state_ent_s {
	STP_DISABLED,		/* (8.4.5) */
	STP_BLOCKING,		/* (8.4.1) */
	STP_LISTENING,		/* (8.4.2) */
	STP_FORWARDING, 	/* (8.4.4) */
	STP_INVALID 		/* the stp port is not there */
};


enum stp_running_mode{
	STP_MODE = 0,
	MST_MODE
};


#define MSTP_PORT_MST_DB_SIZE (MAX_SWITCHPORT_PER_SYSTEM*MAX_MST_ID/4)

typedef struct stp_info_db_s {
   unsigned int port_index; /*switch port index*/
   unsigned int mstid;
   int stp_en;
   enum stp_running_mode mode;
    unsigned int 	pathcost;
    unsigned int 	prio;
    unsigned int 	p2p;
    unsigned int 	edge;
    unsigned int		nonstp;
    enum stp_state_ent_s state;
   unsigned int port_index_count;
   unsigned int smartlink_flag;
   unsigned int erpp_flag;
}stp_info_db_t;

typedef struct vlan_stp_s
{
    unsigned int vid;
    unsigned int mstid;
}vlan_stp_t;

extern hash_table_index_t *ports_stp;

extern sequence_table_index_t *vlans_stp;

#endif

