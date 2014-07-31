#ifndef __NPD_NETIF_EVENT_H__
#define __NPD_NETIF_EVENT_H__

enum  PORT_NOTIFIER_ENT {
	PORT_NOTIFIER_LINKUP_E = 0,
	PORT_NOTIFIER_LINKDOWN_E,
	PORT_NOTIFIER_LINKPOLL_E,	/* for eth-port link status polling message*/
	PORT_NOTIFIER_CREATE,
	PORT_NOTIFIER_INSERT,  /*hot plug in*/
	PORT_NOTIFIER_REMOVE,  /*hot plug out*/
	PORT_NOTIFIER_DELETE,
	PORT_NOTIFIER_L2CREATE,
	PORT_NOTIFIER_L2DELETE,
	PORT_NOTIFIER_STPEN,
	PORT_NOTIFIER_STPTC,
	PORT_NOTIFIER_FORWARDING,
	PORT_NOTIFIER_DISCARD,
	PORT_NOTIFIER_L3CREATE,
	PORT_NOTIFIER_L3DELETE,
	PORT_NOTIFIER_L3LINKUP,
	PORT_NOTIFIER_L3LINKDOWN,
	NOTIFIER_SLOT_INSERT,
	NOTIFIER_SLOT_REMOVE,
	NOTIFIER_SLOT_DELETE,
	NOTIFIER_SWITCHOVER,
	NOTIFIER_MSTP_MODE,
	NOTIFIER_RSTP_MODE,
	NOTIFIER_POE_OPERATE,
	PORT_NOTIFIER_L2ADDR_ADD,
	PORT_NOTIFIER_L2ADDR_DEL, 
    PORT_NOTIFIER_TRACK_UP,
    PORT_NOTIFIER_TRACK_DOWN,
    PORT_NOTIFIER_TRACK_DELETE,
    PORT_NOTIFIER_IPMCHIT,
    PORT_NOTIFIER_ETHPORT_STAT,
	PORT_NOTIFIER_TYPE_MAX	
};

enum  PORT_RELATE_ENT {
	PORT_NOTIFIER_JOIN = 0,
	PORT_NOTIFIER_LEAVE,
	PORT_NOTIFIER_ADDR_ADD,
	PORT_NOTIFIER_ADDR_DEL,
	PORT_NOTIFIER_RELATE_MAX
};

extern struct list_head netif_notifier_head;


typedef void (*netif_event_notify_t)(unsigned int netif_index, enum PORT_NOTIFIER_ENT event, char *data, int datalen);
typedef void (*netif_relate_event_notify_t)(unsigned int father_index, unsigned int son_ifindex, enum PORT_RELATE_ENT event, char *data, int datalen);

typedef struct netif_event_notifier_s
{
    struct list_head list;
    netif_event_notify_t  netif_event_handle_f;
    netif_relate_event_notify_t netif_relate_handle_f;
}netif_event_notifier_t;

struct netif_event_msg_hdr
{
	unsigned int	nlmsg_len;					/* Length of message including header */
	unsigned short	nlmsg_type;					/* Message content 	*/
	unsigned short	nlmsg_flags;				/* Additional flags		*/
	unsigned int	nlmsg_seq;					/* Sequence number	*/
	unsigned int	nlmsg_pid;					/* Sending process PID	*/
};

typedef struct _netif_remote_event_hdr_
{
	struct netif_event_msg_hdr		nlh;
	unsigned int netif_index;
	unsigned int relate_netif_index;
	unsigned int event_code;
}netif_remote_event_hdr;

typedef void (*netif_app_event_op)(unsigned int netif_index, int event, char *private, int len);
typedef void (*netif_app_relate_event_op)(unsigned int netif_index, unsigned int son_netif_index, int event, char *private, int len);


extern void register_netif_notifier(netif_event_notifier_t *notifier);
extern void unregister_netif_notifier(netif_event_notifier_t *notifier);
extern void netif_notify_event(unsigned int netif_index, enum PORT_NOTIFIER_ENT event);
extern void netif_notify_relate_event(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event);
extern int netif_remote_notify_event(int dest_slot, unsigned int netif_index, enum PORT_NOTIFIER_ENT event, void *private_data, int len);
extern int  netif_app_event_init(int app_instance);
extern int  netif_app_event_handle(int sock, netif_app_event_op event_ops, netif_app_relate_event_op relate_event_ops);
extern int  netif_app_event_op_register(netif_app_event_op event_ops, netif_app_relate_event_op relate_event_ops);
extern int netif_app_event_op_unregister();
extern int netif_app_notify_event(unsigned int netif_index, enum PORT_NOTIFIER_ENT event, void * private_data, int len);
extern int netif_app_notify_relate_event(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event, void * private_data, int len);
void netif_notify_relate_event_out(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event, char *private, int len);
extern int netif_app_event_hton(netif_remote_event_hdr *app_event);
int netif_app_event_ntoh(netif_remote_event_hdr *app_event);
int netif_event_init();

#endif
