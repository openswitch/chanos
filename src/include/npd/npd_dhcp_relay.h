#ifndef _NPD_DHCP_RELAY_H_
#define _NPD_DHCP_RELAY_H_

#define NPD_DHCPR_SERVER_DB      "npdDhcprDB"

#define NPD_DHCPR_SERVER_HASH_NAME  "npdDhcprHash"

#define MAX_DHCPR_IP_NUM    (8)

#define NPD_DHCPR_MAX_HOPS    4    /*缺省最大HOP值*/
#define NPD_DHCP_RELAY_DEF_HOPS_LIMIT     16

typedef struct npd_dhcp_relay_server
{
	unsigned char enDis;
	unsigned short vid;
	unsigned char ifName[32];
	struct sockaddr_in svrAddr[MAX_DHCPR_IP_NUM];
	unsigned int netifIndex;
	unsigned int dhcp_server_status;
}NPD_DHCPR_SERVER;


unsigned int npd_dhcpr_server_compare_byIndex(NPD_DHCPR_SERVER *data1, NPD_DHCPR_SERVER *data2);
int npd_dhcpr_server_get_byIndex(unsigned int index, NPD_DHCPR_SERVER *dhcprServer);
int npd_dhcpr_server_set(NPD_DHCPR_SERVER *dhcprServer);
int npd_dhcpr_server_del(NPD_DHCPR_SERVER *dhcprServer);


int npd_dhcp_server_flag_check
(
	unsigned int ifName,
	unsigned int *status
);




unsigned int npd_dhcp_information_relay_opt82_status_get
(
	unsigned char *relay_opt82_status
);

unsigned int npd_dhcp_relay_set_opt82_status
(
	unsigned char isEnable
);


#endif
