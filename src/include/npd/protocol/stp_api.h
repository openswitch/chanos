#ifndef __PROTOCOL_STP_API_H__
#define __PROTOCOL_STP_API_H__

typedef enum{
	DISCARDING,
	LEARNING,
	FORWARDING
} PORT_STATE;

typedef enum stp_link_state{
   STP_LINK_STATE_DOWN_E = 0 ,
   STP_LINK_STATE_UP_E,
   STP_LINK_STATE_MAX
}STP_LINK_STATE;

typedef enum 
{
		PORT_ENABLE_E = 0,
		PORT_DISABLE_E,
		INTERFACE_ADD_E,
		INTERFACE_DEL_E,
		VLAN_ADD_ON_MST_E,
		VLAN_DEL_ON_MST_E,
		FDB_ENTRY_ADD_E,
		FDB_ENTRY_DEL_E,
		FDB_ENTRY_CLEAR_E,
		STP_STATE_UPDATE_E,
		STP_RECEIVE_TCN_E,
		LINK_CHANGE_EVENT_E,
		STP_GET_SYS_MAC_E,
		STP_SWITCHOVER,
		INTERFACE_ADD_ALL_E,
		RSTP_OP_TYPE_MAX
}RSTP_OP_TYPE_E;

typedef struct npd_rstp_intf_op_param
{
	unsigned short vid;
	unsigned int	portIdx;
    int duplex_mode;
    int speed;
}NPD_RSTP_INTF_OP_PARAM_T;

typedef struct npd_mstp_vlan_op_param
{
	unsigned short vid;
	unsigned int untagbmp[2];
	unsigned int tagbmp[2];
}NPD_MSTP_VLAN_OP_PARAM_T;

typedef struct npd_rstp_fdb_op_param
{
	unsigned int	portIdx;
	unsigned char	MacDa[6];
}NPD_RSTP_FDB_OP_PARAM_T;

typedef struct npd_recv_info_param
{
	unsigned int 	mstid;
	unsigned short vid;
	unsigned int		portIdx;
	NAM_RSTP_PORT_STATE_E	portState;	
}NPD_RECV_INFO_PARAM_T;

typedef enum npd_rstp_link_event 
{
    LINK_PORT_DOWN_E = 0,
 	LINK_PORT_UP_E ,
	LINK_PORT_MAX
}NPD_RSTP_LINK_ENT;

typedef struct npd_rstp_link_op_param
{
	unsigned int		portIdx;
	NPD_RSTP_LINK_ENT	event;
	unsigned int        speed;
	unsigned int        duplex_mode;
}NPD_RSTP_LINK_OP_PARAM_T;
	
typedef union
{
	NPD_RSTP_INTF_OP_PARAM_T	cmdIntf;
	NPD_MSTP_VLAN_OP_PARAM_T  cmdVlan;
	NPD_RSTP_FDB_OP_PARAM_T		cmdFdb;
	NPD_RECV_INFO_PARAM_T		cmdStp;
	NPD_RSTP_LINK_OP_PARAM_T	cmdLink;
	
}NPD_RSTP_OP_PARAM_U;

typedef struct
{
	RSTP_OP_TYPE_E		type;
	unsigned int		length;
	NPD_RSTP_OP_PARAM_U	cmdData;
}NPD_CMD_STC;

typedef struct
{
	int ret;
	NPD_CMD_STC cmd;
}NPD_RECV_INFO;

#define SL_MSG_COOKIE_ID 0xff0f0301
#define SL_MSG_COOKIE_ID_END 0x01030fff

enum 
{
    SL_MSG_COOKIE,
    SL_MSG_BLOCK_PORT,
    SL_MSG_UNBLOCK_PORT,
    SL_MSG_INSTANCE,
    SL_MSG_BACK_ID,
    SL_MSG_COOKIE_END,
    SL_MSG_MAX
};

struct smart_link_msg_s
{
    unsigned int cookie;
    unsigned int block_port;
    unsigned int unblock_port;
    unsigned int instance;
    unsigned int back_id;
    unsigned int cookie_end;
};

struct advertise_s
{
    char version;
    char pad;
    char action;
    char auth_mode;
    char password[16];
    char smac[6];
    short advertise_vlan;
    npd_vbmp_t data_vlan;
};


#define NPD_ADV_VLAN_LIST_MAX   8

struct npd_smart_link_port_s
{
    unsigned int netif_index;   /* XXX: just as global_port_ifindex in switch_port_db_s*/
    short adv_vlan_list[NPD_ADV_VLAN_LIST_MAX];
};

struct erpp_port_s
{
    unsigned int netif_index; 
    int erpp_port_role;      
    int erpp_port_status;
};

struct erpp_msg_to_npd_s
{
	unsigned int validate_byte_begin;/*0xAA*/
	struct erpp_port_s port[2];
	char flush_flag;
	char instance;
	unsigned int is_enable;
	unsigned int validate_byte_end; /*0xCC*/
};


#endif

