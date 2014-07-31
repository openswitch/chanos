#ifndef __NPD_CAPWAP_API_H__
#define __NPD_CAPWAP_API_H__

int npd_capwap_init(void);

int npd_capwap_get_global_conf(unsigned int conf_id, struct npd_capwap_global_conf_s *global_conf);
int npd_capwap_get_L3_conf(unsigned int conf_id, struct npd_capwap_L3_conf_s *L3_conf);
int npd_capwap_get_bssid_by_netif( unsigned int netif_index, char *bssid);
int npd_capwap_get_info_by_bssid
(
    char *bssid, 
    unsigned int *wtp_ipaddr, 
    unsigned int *sourceIp, 
    unsigned short *sPort, 
    unsigned short *dPort, 
    unsigned int *encap_format
);
int npd_capwap_get_pvid_by_netif( unsigned int netif_index, unsigned short *pvid);
int npd_capwap_keep_capwap_port_vlan_relation(unsigned short vlan_id, unsigned int op);
int  npd_capwap_get_netif_by_mac_vid
(
	unsigned char *mac,
	unsigned short vid,
	unsigned int *netif_index
);




#endif
