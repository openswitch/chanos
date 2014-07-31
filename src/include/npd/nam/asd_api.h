#ifndef __NPD_ASD_API_H__
#define __NPD_ASD_API_H__

int npd_asd_msg_init(void);

int npd_asd_send_arp_info_to_wirelss(
	unsigned long   port_index, 
	unsigned short  vlan_id, 
	int     Op,
	unsigned char   STAMAC[MAC_ADDR_LEN],
	unsigned int    ip_addr);

unsigned int npd_asd_check_vlan_valid
( 
	unsigned int netif_index,
	unsigned short vlan_id,
	int state
);

#endif
