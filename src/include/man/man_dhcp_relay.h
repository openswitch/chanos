#ifndef __MAN_DHCP_RELAY_H__
#define __MAN_DHCP_RELAY_H__

#define DHCPR_NAME_LEN (64)
#define MAX_DHCPR_IP_NUM    (8)

struct dcli_dhcpr_query 
{
	unsigned char dhcp_relay_endis;
    unsigned char dhcp_relay_opt82_enable;
};

struct man_dhcp_relay_helper_info_s
{
	unsigned char enDis;
	unsigned short vid;
	unsigned char ifName[32];
    unsigned int    ip_addr[MAX_DHCPR_IP_NUM];
    unsigned short  port[MAX_DHCPR_IP_NUM];
	unsigned int netifIndex;
	unsigned int dhcp_server_status;
};

#define DCLI_DHCP_OPT82_INFOR_STR           "DHCP option information\n"

#endif

