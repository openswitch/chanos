#ifndef __COMMON_DHCP_SNOOP_H__
#define __COMMON_DHCP_SNOOP_H__

/*********************************************************
*	macro define													*
**********************************************************/
#define	NPD_DHCP_SNP_ENABLE					(1)			/* DHCP-Snooping global status, enable		*/
#define	NPD_DHCP_SNP_DISABLE				(0)			/* DHCP-Snooping global status, disenable		*/

#define	NPD_DHCP_SNP_OPT82_ENABLE			(1)			/* DHCP-Snooping option82, enable			*/
#define	NPD_DHCP_SNP_OPT82_DISABLE			(0)			/* DHCP-Snooping option82, disenable		*/

#define	NPD_DHCP_RELAY_OPT82_ENABLE			(1)			/* DHCP relay option82, enable			*/
#define	NPD_DHCP_RELAY_OPT82_DISABLE		(0)			/* DHCP relay option82, disenable		*/

#define	NPD_DHCP_SNP_INIT_0					(0)			/* init int/short/long variable				*/

#define NPD_DHCP_MESSAGE_TYPE				(0x35)		/*DHCP option 53, flag of message type		*/

#define NPD_DHCP_BOOTREQUEST				(1)
#define NPD_DHCP_BOOTREPLY					(2)

#define NPD_DHCP_SNP_OPTION_LEN				(1500)		/* length of DHCP-Snooping option			*/

#define NPD_DHCPR_SERVER_PORT             67
#define NPD_DHCPR_CLIENT_PORT             68

#define NPD_DHCP_RELAY_ENABLE  0x1
#define NPD_DHCP_RELAY_DISABLE 0x2


#define NPD_DHCPR_SERVER_SIZE    (128)
#define NPD_DHCPR_SERVER_HASH_SIZE  (128)


/*show running cfg mem*/
#define NPD_DHCPR_SHOWRUNNING_CFG_LEN       (200)
#define NPD_DHCP_SNP_RUNNING_CFG_MEM		(NPD_DHCPR_SERVER_SIZE * NPD_DHCPR_SHOWRUNNING_CFG_LEN)
#define NPD_DHCP_SNP_RUNNING_GLOBAL_CFG_LEN		(367616)    /* 1024 interface; 4096 vlan */

#define NPD_DHCP_SNP_LEASE_INFINITE         (~0UL)

/* type of the DHCP packet */
#define NPD_DHCP_DISCOVER		1
#define NPD_DHCP_OFFER			2
#define NPD_DHCP_REQUEST		3
#define NPD_DHCP_DECLINE		4
#define NPD_DHCP_ACK			5
#define NPD_DHCP_NAK			6
#define NPD_DHCP_RELEASE		7
#define NPD_DHCP_INFORM			8
#define NPD_DHCP_UNKNOWN		9

#define NPD_DHCP_SNP_REQUEST_TIMEOUT                 (60)
#define NPD_DHCP_SNP_REQUEST_TIMEOUT_NAK             (13)

#define NPD_DHCP_SNP_MAC_ADD_LEN        6                        /* length of mac address			*/

#define NPD_DHCP_SNP_REMOTEID_STR_LEN		(64)	/* length of user-defined remote-id string, 64	*/
#define NPD_DHCP_SNP_CIRCUITID_STR_LEN		(64)	/* length of user-defined circuit-id string, 64	*/

#define NPD_DHCP_SNP_OPTION_FIELD		(0)
#define NPD_DHCP_SNP_FILE_FIELD			(1)
#define NPD_DHCP_SNP_SNAME_FIELD		(2)

#define NPD_DHCP_SNP_OPT_CODE			(0)
#define NPD_DHCP_SNP_OPT_LEN			(1)
#define NPD_DHCP_SNP_OPT_DATA			(2)


/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET			0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER			0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL			0x17
#define DHCP_MTU			0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1				0x3a
#define DHCP_T2				0x3b
#define DHCP_VENDOR			0x3c
#define DHCP_CLIENT_ID		0x3d
#define DHCP_OPTION_82		0x52
#define DHCP_CIRCUIT_ID		(0x1)
#define DHCP_REMOTE_ID		(0x2)


#define DHCP_END			0xFF


/*********************************************************
*	struct define													*
**********************************************************/
typedef enum {
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX = 0,			/* DHCP-Snooping format type of option 82: hex	*/
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII,			/* DHCP-Snooping format type of option 82: ascii	*/
	NPD_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_FROMAT_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT = 0,		/* DHCP-Snooping packet fill format type of option 82: extended	*/
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD,			/* DHCP-Snooping packet fill format type of option 82: standard	*/
	NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID = 0,		/* DHCP-Snooping remote-id type of option 82: system mac		*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME,			/* DHCP-Snooping remote-id type of option 82: system name		*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR,			/* DHCP-Snooping remote-id type of option 82: user define string	*/
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC,
	NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_DEFAULT
}NPD_DHCP_SNP_OPT82_REMOTEID_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT = 0,		/* DHCP-Snooping circuit-id type of option 82: default, vlan id + port index	*/
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR,			/* DHCP-Snooping circuit-id type of option 82: user define string	*/
	NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE;

typedef enum {
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE = 0,		/* Config the configuration strategy for option 82: replace			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP,				/* Config the configuration strategy for option 82: drop			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP,				/* Config the configuration strategy for option 82: keep			*/
	NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID
}NPD_DHCP_SNP_OPT82_STRATEGY_TYPE;

typedef enum {
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID
}NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE;

typedef enum {
	NPD_DHCP_SNP_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	NPD_DHCP_SNP_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	NPD_DHCP_SNP_BINDING_OPERATE_TYPE_INVALID
}NPD_DHCP_SNP_BINDING_OPERATE_TYPE;


/*********************************************************
*	struct define													*
**********************************************************/
typedef enum {
	NPD_DHCP_SNP_PORT_MODE_NOTRUST = 0,					/* DHCP-Snooping trust mode of port: no trust	*/
	NPD_DHCP_SNP_PORT_MODE_NOBIND,						/* DHCP-Snooping trust mode of port: trust but no bind	*/
	NPD_DHCP_SNP_PORT_MODE_TRUST,						/* DHCP-Snooping trust mode of port: trust	*/
	NPD_DHCP_SNP_PORT_MODE_INVALID
}NPD_DHCP_SNP_PORT_MODE_TYPE;

typedef struct NPD_DHCP_MESSAGE_S
{
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned int xid;
	unsigned short secs;
	unsigned short flags;
	unsigned int ciaddr;
	unsigned int yiaddr;
	unsigned int siaddr;
	unsigned int giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned int cookie;
	unsigned char options[NPD_DHCP_SNP_OPTION_LEN]; /* 312 - cookie */ 
} NPD_DHCP_MESSAGE_T;

typedef struct NPD_DHCP_OPTION_S{
  unsigned char	code;
  unsigned char	leng;
  unsigned char	value[];
}NPD_DHCP_OPTION_T;

typedef struct NPD_DHCP_SNP_SHOW_ITEM_S
{
	unsigned int   bind_type;
	unsigned char  chaddr[6];
	unsigned int   ip_addr;
	unsigned short vlanId;
	unsigned int   ifindex;
	unsigned int   lease_time;
	unsigned int   count;
}NPD_DHCP_SNP_SHOW_ITEM_T;


typedef struct NPD_DHCP_SNP_SHOW_TRUST_PORTS_S
{
	unsigned short vlanId;
	unsigned char tag_mode;
	unsigned int trust_mode;
	unsigned int index;	
	unsigned char endis;
	unsigned int count;
}NPD_DHCP_SNP_SHOW_TRUST_PORTS_T;

typedef struct NPD_DHCP_SNP_ETHER_HEAD_S
{
	unsigned char  dmac[6];		/* destination eth addr	*/
	unsigned char  smac[6];		/* source ether addr	*/
	unsigned short etherType;
}NPD_DHCP_SNP_ETHER_HEAD_T;


struct npd_dhcp_relay_global_status
{
	unsigned char dhcp_relay_endis;
    unsigned char dhcp_relay_opt82_enable;
};

typedef enum NPD_DHCP_SNP_BIND_STATE_S {
	NPD_DHCP_SNP_BIND_STATE_REQUEST         = 0,
	NPD_DHCP_SNP_BIND_STATE_BOUND           = 1,
} NPD_DHCP_SNP_BIND_STATE_T;

typedef enum NPD_DHCP_SNP_BIND_TYPE_S {
	NPD_DHCP_SNP_BIND_TYPE_DYNAMIC         = 0,
	NPD_DHCP_SNP_BIND_TYPE_STATIC          = 1,
} NPD_DHCP_SNP_BIND_TYPE_T;

struct npd_dhcp_snp_timeout_s
{
    unsigned int    expire_time;
    char            hw_addr[NPD_DHCP_SNP_MAC_ADD_LEN];
    struct npd_dhcp_snp_timeout_s* next;
};
typedef struct NPD_DHCP_SNP_USER_ITEM_S
{
	unsigned int  bind_type;
	unsigned char state;
	unsigned char haddr_len;
	unsigned char chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned short vlanId;
	unsigned int ip_addr;
	unsigned int lease_time;
	unsigned int sys_escape; /*添加绑定表项时系统启动以来所过的时间 */
	unsigned int cur_expire;	   /* 当前使用的有效的IP地址状态超时时间,仅显示时使用*/
	unsigned int ifindex;
	unsigned int flags;
    int have_sg;
    unsigned int xid;
    unsigned int server_id;
    unsigned int relay_ip;
}NPD_DHCP_SNP_USER_ITEM_T;

struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT
{
	unsigned int global_port_ifindex;
	unsigned int trust_mode;
	unsigned char opt82_strategy;		/* DHCP-Snooping option82 strategy type of port	*/
	unsigned char opt82_circuitid;		/* DHCP-Snooping option82 circuitid type of port	*/
										/* DHCP-Snooping option82 circuitid content of port */
	unsigned char opt82_circuitid_str[NPD_DHCP_SNP_CIRCUITID_STR_LEN];
};

struct NPD_DHCP_SNP_GLOBAL_STATUS
{
	unsigned char dhcp_snp_enable;
	unsigned char dhcp_snp_opt82_enable;
	unsigned char dhcp_snp_opt82_format_type;
	unsigned char dhcp_snp_opt82_fill_format_type;
	unsigned char dhcp_snp_opt82_remoteid_type;
	unsigned char dhcp_snp_opt82_remoteid_str[NPD_DHCP_SNP_REMOTEID_STR_LEN];
    npd_vbmp_t vlan_admin_status;
/*    npd_vbmp_t vlan_run_status;   */
    unsigned short switch_port_control_count[MAX_SWITCHPORT_PER_SYSTEM];
};

#endif

