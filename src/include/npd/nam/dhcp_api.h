#ifndef __NAM_DHCP_API_H__
#define __NAM_DHCP_API_H__

unsigned int npd_dhcp_relay_message_replace
(
	unsigned char *packetBuffs,
	unsigned char *isBroadcast,
	unsigned int *output_index,
	unsigned long *buffLen,
	unsigned short *vlanId
);
unsigned int npd_dhcpr_check_global_status
(
	void
);
unsigned char npd_dhcpr_check_intf_status
(
	unsigned int netif_index
);
unsigned int npd_dhcpr_relay_server_send
(
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netifIndex,
	unsigned short vid
);
unsigned int npd_dhcp_snp_lease_delete_by_vlan
(
	unsigned short vid,
	unsigned char flag
);
unsigned int npd_dhcp_relay_global_status_get
(
	struct npd_dhcp_relay_global_status* user
);
unsigned int npd_dhcpr_send_to_agent
(
	unsigned char *data,
	unsigned long pktLen,
	unsigned int ipAddr, /* must be network byte order */
	unsigned int port   /* must be network byte order */
);
unsigned int npd_dhcpr_init();

unsigned int npd_dhcp_snp_init();

unsigned int npd_dhcp_snp_check_vlan_status
(
	unsigned short vlanId
);

unsigned int npd_dhcp_snp_get_global_status
(
	unsigned char *status
);

unsigned int npd_dhcp_snp_packet_rx_process
(
	int packetType,
	unsigned char *packetBuffs,
	unsigned long* buffLen,
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vid
);

unsigned int npd_dhcp_information_checksum
(
	struct iphdr* ip,
	struct udphdr* udp
);
unsigned int npd_dhcp_snp_global_cfg_get
(
	struct NPD_DHCP_SNP_GLOBAL_STATUS *user
);

void npd_dhcp_snp_send_nak_to_client(NPD_DHCP_SNP_USER_ITEM_T* entry);
unsigned int npd_dhcp_snp_packet_tx_hook
(
	int packetType,
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netif_index,
	unsigned char isTagged,
	unsigned short vid
);

unsigned int npd_dhcp_packet_information_process
(
	unsigned short vid, 
	unsigned int ifindex, 
	unsigned char isTagged, 
	unsigned char *packetBuffs, 
	unsigned long  *buffLen,
	int option_enable_flag
);

unsigned short npd_dhcp_snp_checksum
(
	void *addr,
	unsigned int count
);

unsigned int npd_dhcp_packet_record_user_info
(
	unsigned short vid,
	unsigned int netif_index,
	NPD_DHCP_MESSAGE_T *data,
	unsigned char message_type,
	unsigned int trustMode
);
int npd_dhcp_snp_tbl_initialize
(
	void
);

unsigned int npd_dhcp_snp_tbl_destroy
(
	void
);


unsigned int npd_dhcp_snp_tbl_item_find
(
	NPD_DHCP_SNP_USER_ITEM_T *user
);


unsigned int npd_dhcp_snp_tbl_fill_item
(
	NPD_DHCP_SNP_USER_ITEM_T *userA,
	NPD_DHCP_SNP_USER_ITEM_T *userB
);


unsigned int npd_dhcp_snp_tbl_item_insert
(
	NPD_DHCP_SNP_USER_ITEM_T *user
);


unsigned int npd_dhcp_snp_tbl_item_delete
(
	NPD_DHCP_SNP_USER_ITEM_T *user
);

unsigned int npd_dhcp_snp_tbl_identity_item
(
	NPD_DHCP_SNP_USER_ITEM_T *userA,
	NPD_DHCP_SNP_USER_ITEM_T *userB
);


unsigned int npd_dhcp_snp_tbl_refresh_bind
(
	NPD_DHCP_SNP_USER_ITEM_T *preuser,
	NPD_DHCP_SNP_USER_ITEM_T *user
);

unsigned int npd_dhcp_snp_tbl_static_binding_insert
(
	NPD_DHCP_SNP_USER_ITEM_T user
);


unsigned int npd_dhcp_snp_tbl_static_binding_delete
(
	NPD_DHCP_SNP_USER_ITEM_T user
);

int npd_dhcp_snp_status_item_table_initialize();

unsigned int npd_dhcp_snp_status_item_insert
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item	
);


unsigned int npd_dhcp_snp_status_item_delete
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
);

unsigned int npd_dhcp_snp_status_item_find	
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
);
unsigned int npd_dhcp_snp_status_item_refresh
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *preitem,
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
);


unsigned int npd_dhcp_snp_lease_delete_by_vid
(
	unsigned int vid
);

unsigned int npd_dhcp_snp_lease_delete_by_port
(
	unsigned int ifindex,
	unsigned char flag
);

unsigned int npd_dhcp_snp_get_trust_mode
(
	unsigned short vid,
	unsigned int ifindex,
	unsigned int *trust_mode
);

int npd_dhcp_snp_global_status_initialize
(
	void
);
unsigned int npd_dhcp_snp_bind_show
(
	struct NPD_DHCP_SNP_USER_ITEM_S *user
);

unsigned int npd_dhcp_snp_timer_thread(void* arg);

unsigned int npd_dhcp_snp_self_sock_init();
unsigned int npd_dhcp_snp_lease_delete_by_vlan_port
(
	unsigned short vid,
	unsigned int ifindex,
	unsigned char flag
);
unsigned int npd_dhcp_snp_lease_delete_by_vlan
(
	unsigned short vid,
	unsigned char flag
);


unsigned int npd_dhcpv6_relay_table_init();
unsigned int npd_dhcpv6_relay_packet_rx_process
(
    unsigned char isTagged,
    unsigned short vid,
    int packetType,
    unsigned int netif_index,
    unsigned long buffLen,
    unsigned char *packetBuffs
);

unsigned int npd_dhcp_snp_opt82_enable
(
	void
);

unsigned int npd_dhcp_snp_opt82_disable
(
	void
);

unsigned int npd_dhcp_snp_get_opt82_status
(
	unsigned char *status
);

unsigned int npd_dhcp_snp_set_opt82_status
(
	unsigned char isEnable
);


unsigned int npd_dhcp_snp_set_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char strategy_mode
);

unsigned int npd_dhcp_snp_get_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *strategy_mode
);

unsigned int npd_dhcp_snp_set_opt82_port_circuitid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char circuitid_mode,
	char *circuitid_content
);

unsigned int npd_dhcp_snp_get_opt82_port_circuitid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *circuitid_mode,
	unsigned char *circuitid_content
);


unsigned int npd_dhcp_snp_get_opt82_port_remoteid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *remoteid_mode,
	unsigned char *remoteid_content
);

unsigned int npd_dhcp_snp_check_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
);

unsigned int npd_dhcp_snp_remove_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int opt_len,
	unsigned int *del_len
);

unsigned int npd_dhcp_snp_cat_opt82_sting
(
	unsigned char *circuitid_str,
	unsigned char *remoteid_str,
	unsigned char *opt82_str
);

unsigned int npd_dhcp_snp_attach_opt82_string
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned char *string
);

unsigned int npd_dhcp_snp_end_option
(
	unsigned char *optionptr
); 

unsigned int npd_dhcp_snp_attach_opt82
(
	unsigned char *packet,
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vlanid,
	unsigned long *bufflen
);

unsigned int npd_dhcp_snp_check_global_status
(
	void
);



unsigned int npd_dhcp_snp_vlan_enable_set
(
    unsigned char is_enable,
    unsigned short vlan_id
);


unsigned int npd_dhcp_snp_set_port_trust_mode
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned int trust_mode
);


void *npd_dhcp_snp_get_option
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
);

unsigned int npd_dhcp_snp_global_cfg_set
(
	struct NPD_DHCP_SNP_GLOBAL_STATUS *user
);

unsigned int npd_dhcp_information_snp_opt82_status_get
(
	unsigned char *snp_opt82_status
);


#endif

