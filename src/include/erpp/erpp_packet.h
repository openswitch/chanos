#ifndef __ERPP_PACKET_H_
#define __ERPP_PACKET_H_

#define ERPP_MUTICAST_MAC	"\x01\x80\xc2\x00\x00\xaa"
#define ERPP_FRAME_HEADER_TYPE 0x8100
#define ERPP_VERSION  0x00
#define CLEAR_FDB_FLAG     0x01
#define FIX_PRI            0x1110
#define DSAP_SSAP          0xAAAA
#define FIX_CONTROL        0x02
#define FRAME_LENGTH       0x48
#define FIX_OUI            "\x00\xE0\x2B"
#define FIX_1              0x00bb
#define FIX_2              0x99
#define FIX_3              0x0b

#define ERPP_LENGTH        0x50
#define ERPP_VERSION       0x00
#define VALIDATE_BYTE_BEGIN        0xAAAAAAAA
#define VALIDATE_BYTE_END          0xCCCCCCCC

enum erpp_frame_type
{
    HELLO_PACKET = 1,
	COMPLETE_FLUSH_PACKET,
	COMMON_FLUSH_PACKET,
	LINK_DOWN_PACKET,
	EDGE_HELLO_PACKET,
	FAULT_PACKET
};

struct erpp_packet_fix_s
{
	unsigned  short pri: 4;
	unsigned  short vlanid: 12;
	unsigned  short frame_length;
	unsigned  short dsap_ssap;
	unsigned  char	control;
	unsigned  char	oui[3];
	unsigned  short fixup_1;
	unsigned  char	fixup_2;
	unsigned  char	fixup_3;
};

struct erpp_packet_s
{
	struct erpp_packet_fix_s  fix;
	unsigned  short erpp_length;
	unsigned  char erpp_version;/*0X00*/
	unsigned  char erpp_type;
	unsigned  short domain_id;
	unsigned  short ring_id;
	unsigned  short control_vlan_id;
	unsigned  char sys_mac[6];
	unsigned  short hello_timer;
	unsigned  short fail_timer;
	unsigned  char level;
	unsigned  char reserve[3];/*0x0*/
	unsigned  char passward[16];
}; 

struct ethernet_header_s
{
    unsigned char dmac[6];
    unsigned char smac[6];
    unsigned short type;
};

struct erpp_packet_sync_ctrl_s
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

union erpp_packet_sync_ctrl_u
{
    char reserved[NPD_PKT_RESERVED_LEN];
    struct erpp_packet_sync_ctrl_s sync_ctrl;
};

#define ERPP_PACKET_LENGTH   sizeof(union erpp_packet_sync_ctrl_u)+\
	                         sizeof(struct ethernet_header_s)+\
	                         sizeof(struct erpp_packet_s)


int erpp_packet_socket_init();
int erpp_msg_sock_init();
void erpp_send_msg(char*, unsigned int);

#endif
