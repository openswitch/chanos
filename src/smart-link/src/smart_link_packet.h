#ifndef __SMART_LINK_PACKET_H_
#define __SMART_LINK_PACKET_H_

#define SL_MUTICAST_MAC	"\x01\x80\xc2\x55\xaa\x0f"
#define SL_STLK_TYPE 0x9f11

#define SL_VERSION  0x00
#define CLEAR_FDB   0x01
#define FLUSH_ARP   0x02
#define SL_AUTHMODE_DLT 0x00

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

struct ethernet_header_s
{
    unsigned char dmac[6];
    unsigned char smac[6];
    unsigned short type;
};

struct nlmsghdr
{
	unsigned int		nlmsg_len;		/* Length of message including header */
	unsigned short		nlmsg_type;		/* Message content */
	unsigned short		nlmsg_flags;	/* Additional flags */
	unsigned int		nlmsg_seq;		/* Sequence number */
	unsigned int		nlmsg_pid;		/* Sending process PID */
};

struct smart_link_packet_sync_ctrl_s
{
    struct nlmsghdr nlh;
    int packet_type;
    int flag;
    unsigned int netif_index;
    unsigned int son_netif_index;
    unsigned short vid;
    unsigned short istagged;
    int packet_len;
};

#define NPD_PKT_RESERVED_LEN    64
union smart_link_packet_sync_ctrl_u
{
    char reserved[NPD_PKT_RESERVED_LEN];
    struct smart_link_packet_sync_ctrl_s sync_ctrl;
};
#undef NPD_PKT_RESERVED_LEN

int smart_link_packet_socket_init();
int smart_link_app_msg_sock_init();
int smart_link_sysmac_init();
int smart_link_recv_msg(struct thread * );
void smart_link_send_msg(char*, unsigned int);
void smart_link_send_advertise(struct smart_link_s* );

#endif
