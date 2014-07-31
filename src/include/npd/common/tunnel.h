#ifndef __COMMON_NPD_TUNNEL_H__
#define __COMMON_NPD_TUNNEL_H__

/* add for tunnel*/
typedef struct tunnel_host_s {
	unsigned int hostdip;
	unsigned int hdiplen;
	struct list_head   list;
}tunnel_host_t;

typedef enum {
	TUNNEL_STATES_START = 0x0,
	TUNNEL_TS_SW_EN = 0x01,
	TUNNEL_TT_SW_EN = 0x02,
	TUNNEL_NH_SW_EN = 0x04,
	TUNNEL_RT_SW_EN = 0x08,
	TUNNEL_TS_HW_EN = 0x10,
	TUNNEL_TT_HW_EN = 0x20,
	TUNNEL_NH_HW_EN = 0x40,
	TUNNEL_RT_HW_EN = 0x80
}TUNNEL_STATES;

typedef struct tunnel_action_s {
	unsigned long		istunnelstart;
	unsigned int		tunnelstartidx;
}tunnel_action_t;

typedef struct tunnel_kernel_msg_s {
	unsigned char 	mac[6];
	unsigned int		srcip;
	unsigned int		dstip;
	unsigned short	vid;
	unsigned char 	portnum;
	unsigned char 	devnum;
}tunnel_kernel_msg_t;

typedef struct tunnel_item_s {
	TUNNEL_STATES		state;
	unsigned char			sysmac[6];
	unsigned int			istuact; 
	unsigned int 			tsindex;/* init 0 because arp init  use 0 1 2*/
	unsigned int 			ttindex;/* init 0 tt start at 895*/
	unsigned int			nhindex;
	unsigned int			hostnum;
	tunnel_kernel_msg_t	kmsg;
	tunnel_action_t		tunact;
	struct list_head 		list1;
}tunnel_item_t;

enum TUNNEL_DB_ACTION {
	TUNNEL_ADD_ITEM = 0,
	TUNNEL_DEL_ITEM,
	TUNNEL_UPDATE_ITEM,
	TUNNEL_OCCUPY_ITEM,
	TUNNEL_RESET_ITEM,
	TUNNEL_ACION_MAX
};

#define TUNNEL_TERM_FRIST_NUM					896
#define TUNNEL_TERM_LAST_NUM					1023
#define TUNNEL_MAX_SIZE						128 /*hash table max len*/
#define TUNNEL_TABLE_SIZE						1024 /*hash table max storage*/

/* add for tunnel*/

/*
  *	Defined for watchdog control
  */
#define SYSTEM_HARDWARE_WATCHDOG_ENABLE		1
#define SYSTEM_HARDWARE_WATCHDOG_DISABLE	0
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET		1
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_GET		0
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_DEFAULT	(0x1F) /* 0x1F is TIME_UNIT, while TIME_UNIT is 1~1.6s by default */



#define WIFI_MAC_LEN 6
#define WIFI_NL_MAX_PAYLOAD 1024

typedef enum {
	EXT_IP = 0,
	INNER_IP = 1
} msgType;

typedef enum {
	IP_ADD = 0,
	IP_DEL = 1
} msgOp;

struct extIPInfo {
	unsigned int dip;
	unsigned int sip;
	unsigned char WTPMAC[WIFI_MAC_LEN];
};

struct innerIPInfo {
	unsigned int inner_IP;
	unsigned int ext_dip;
	unsigned int ext_sip;
};

/* Netlink message format between wifi and npd */
struct wifi_nl_msg {
	msgType type;
	msgOp op;
	union {
		struct extIPInfo extMsg;
		struct innerIPInfo innerMsg;
	} u;
};



#endif

