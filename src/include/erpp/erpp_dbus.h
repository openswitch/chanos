#ifndef __ERPP_DBUS_H_
#define __ERPP_DBUS_H_


#define	NPD_DBUS_ERPP_OBJPATH	"/aw/npd/erpp"
#define	NPD_DBUS_ERPP_INTERFACE	"aw.npd.erpp"

#define ERPP_DBUS_BUSNAME		"aw.erpp"
#define ERPP_DBUS_OBJPATH		"/aw/erpp"
#define ERPP_DBUS_INTERFACE		"aw.erpp"

#define ERPP_DBUS_METHOD_DOMAIN                 "erpp_domain"
#define ERPP_DBUS_METHOD_BIND_CONTRIL_VLAN      "erpp_control_vlan"
#define ERPP_DBUS_METHOD_BIND_INSTANCE          "erpp_intance"
#define ERPP_DBUS_METHOD_RING_CONFIGURE         "erpp_ring_configure"
#define ERPP_DBUS_METHOD_RING_ENABLE            "erpp_ring_enable"
#define ERPP_DBUS_METHOD_ENABLE                 "erpp_enable"
#define ERPP_DBUS_METHOD_TIMER_SET              "erpp_timer_set"
#define ERPP_DBUS_METHOD_INFO_GET               "erpp_info_get"
#define ERPP_DBUS_METHOD_LOG_SET                "erpp_log_set"
#define ERPP_DBUS_METHOD_SHOW_RUNNING           "erpp_show_running"


#define ERPP_STR "Configure frpp\n"
#define ERPP_DOMAIN_STR "Specify frpp domain\n"
#define ERPP_RING_STR "Specify frpp ring\n"

#define ERPP_DOMAIN_SIZE 8
#define ERPP_RING_SIZE 4
#define DCLI_ERPP_WARNING_MESSAGE_SIZE  64
#define ERPP_TIMER_HELLO_DEFAULT    1
#define ERPP_TIMER_FAIL_DEFAULT     3

enum erpp_port_status
{
    FORWARDING,
	BLOCK,
};
enum erpp_node_role
{
    NODE_MASTER,
    NODE_TRANSMIT,
    NODE_EDGE,
    NODE_ASSISTANT_EDGE,
};

enum erpp_port_role
{
   PORT_PRIMARY,
   PORT_SECONDARY,
   PORT_COMMON,
   PORT_EDGE,
};

enum erpp_master_node_status
{
   NODE_COMPLETE,
   NODE_FAIL,
};

enum erpp_edge_node_status
{
   ERPP_LINK_NORMAL=1,
   ERPP_LINK_FAIL=2,
};

enum erpp_node_status
{
   LINK_UP,
   LINK_DOWN,
   PRE_FORWARDING,
};

enum erpp_timer_type
{
    HELLO_TYPE,
	FAIL_TYPE,
};
struct erpp_global_configure_s
{
    int is_enable;
};

struct erpp_port_s
{
    unsigned int netif_index; 
    int erpp_port_role;      
    int erpp_port_status;
};

struct erpp_node_s
{
    int erpp_node_role;
    int erpp_node_status;
    struct erpp_port_s port[2];/*0 prinmary port 1 secondary port*/
};

struct erpp_ring_s
{
    int ring_id;                  /*环ID号*/
    int level;                   /*环的优先级*/
    int is_enable;               /*环是否在域上使能了*/	
    struct erpp_node_s node;     /*节点也就是交换机*/
};

struct erpp_domain_s
{
    int domain_id;               /*域ID号*/
	unsigned short control_vlan_id[2]; /*控制vlan id 0 主环 1子环*/
    int protect_instance_id;   /*保护实例ID*/
	int hello_timer;             /*hello报文发送间隔定时器*/
    int fail_timer;               /*多久没收到hello报文认为是fail定时器*/
	int fault_timer;
	int timer_count[3];          /*0 HELLO  1FAIL* 2 major*/  	
	int flush_flag;
	int packet_ring;
	struct erpp_ring_s ring[ERPP_RING_SIZE];
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

void erpp_master_board_set(int );
int erpp_domain_search(struct erpp_domain_s* );
int erpp_domain_update(struct erpp_domain_s* );
int erpp_domain_delete(struct erpp_domain_s* );
int erpp_domain_get_next(struct erpp_domain_s* );
int erpp_db_init();
int erpp_dbus_should_set();
int erpp_dbus_sock_init();

#endif
