#ifndef _UDLD_PACKET_H_
#define _UDLD_PACKET_H_

#define UDLD_PACKET_RX					"/tmp/packet.rx.UDLD"
#define UDLD_PACKET_TX					"/tmp/packet.tx.UDLD"

#define UDLD_MAX_PACKET_LEN 64
#define UDLD_PACKET_MAGIC 0xBA98
#define UDLD_PACKET_TYPE  0x89AB

typedef struct _udld_pdu_s_
{
	unsigned char dmac[6];
	unsigned char smac[6];
	unsigned short eth_type;
	unsigned short magic;
	unsigned short packet_type;
	unsigned char local_mac[6];
	unsigned int netif_index;
	unsigned char remote_mac[6];
	unsigned int remote_netif;
}udld_pdu;

typedef struct _udld_skb_s_{
	struct nlmsghdr nlh;
	int packet_type;
	int flag;
	unsigned int netif_index;
	unsigned int son_netif_index;
	unsigned short vid;
	unsigned short istagged;
	int packet_len;
	char pad[24];
	char buf[UDLD_MAX_PACKET_LEN];
}udld_skb;

int udld_packet_echo_tx(udld_netif_ctrl *netif_ctrl);
int udld_packet_respond_tx(udld_netif_ctrl *netif_ctrl);
int udld_packet_init();

#endif

