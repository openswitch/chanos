#ifndef __COMMON_INTF_H__
#define __COMMON_INTF_H__

#define MAX_IP_COUNT 8
#define MAX_IFNAME_LEN 20

#define NPD_INTF_ATTR_URPF_STRICT 0x1
#define NPD_INTF_ATTR_URPF_LOOSE 0x2



typedef struct _NPD_L3INTERFACE_CTRL_
{
	unsigned int ifindex; //driver ifindex
	char ifname[MAX_IFNAME_LEN];
	unsigned int netif_index;
	unsigned int attribute;
	unsigned char mac_addr[6];
	unsigned short vid;
	unsigned int state;
	unsigned int ipv4;
	unsigned int mask;
	unsigned int ctrl_state;
	unsigned char proxy_arp;
    int bind;
	int ipmc;
#ifdef HAVE_PORTAL
    int portal_ctrl;
    int portal_srv_id;
#endif
}NPD_L3INTERFACE_CTRL;

typedef struct _NPD_L3LOCAL_IFINDEX_
{
    unsigned int ifindex; //kernel ifindex
    unsigned int netif_index;
}NPD_L3LOCAL_IFINDEX;

typedef struct _NPD_L3INTF_ADDR_
{
	unsigned int ipAddr;
	unsigned int mask;
	unsigned int ifindex;
}NPD_L3INTF_ADDR;

#ifdef HAVE_NPD_IPV6	
typedef struct _NPD_V6_L3INTF_ADDR_
{
	ip6_addr ipv6Addr;
	ip6_addr v6mask;
	unsigned int ifindex;
}NPD_V6_L3INTF_ADDR;
#endif //HAVE_NPD_IPV6

enum INTF_CTRL_STATUS_E{
	INTF_CTRL_STATE_DOWN = 0,
	INTF_CTRL_STATE_UP,
	INTF_CTRL_STATE_MAX
};

#endif
