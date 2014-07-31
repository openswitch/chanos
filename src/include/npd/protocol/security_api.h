#ifndef __PRODUCT_SECURITY_API_H__
#define __PRODUCT_SECURITY_API_H__

#define NPD_ASD_AUTH_MAC_MODE   0
#define NPD_ASD_AUTH_PORT_MODE  1
#define NPD_ASD_AUTH_MAB_MODE   3   


extern int	npd_asd_fd;
extern struct	sockaddr_un		asd_table_addr; 

enum npd_mng_asd_action {
	NPD_MSG_ASD_ENABLE_SYSTEM_AUTH_CONTROL,
	NPD_MSG_ASD_DISABLE_SYSTEM_AUTH_CONTROL,
	NPD_MSG_ASD_SWITCHOVER,
	NPD_MNG_ASD_ENABLE_DOT1X,
	NPD_MNG_ASD_DISABLE_DOT1X,
	NPD_MNG_ASD_ADD_USER,
	NPD_MNG_ASD_DEL_USER,
	NPD_MNG_ASD_STA_ARP_ADD,
	NPD_MNG_ASD_STA_ARP_DELETE,
	NPD_MNG_ASD_SET_PORT_AUTH,
	NPD_MNG_ASD_SET_MAC_AUTH,
	NPD_MNG_ASD_SET_MAB_AUTH,
	NPD_MNG_ASD_UPDATE_USER_FLOWCTRL,
	NPD_MNG_ASD_SET_PORT_GUEST_VLAN,
	NPD_MNG_ASD_SET_PORT_AUTH_FAIL_VLAN,
	NPD_MNG_ASD_SET_PORT_AUTH_SUCCESS_VLAN,
	NPD_MNG_ASD_MV_PORT_TO_GUEST_VLAN,
	NPD_MNG_ASD_MV_PORT_TO_AUTH_FAIL_VLAN,
	NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_ALLOW,
	NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_DENY,
	NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_REINIT,
#ifdef HAVE_CAPWAP_ENGINE		
	NPD_MNG_ASD_UFDB_ENTRY_ADD,
	NPD_MNG_ASD_UFDB_ENTRY_DEL,
#endif //HAVE_CAPWAP_ENGINE	
	NPD_MNG_ASD_GET_PORT_INFO,
#ifdef HAVE_PORTAL
	NPD_MNG_ASD_SET_PORTAL,
	NPD_MNG_ASD_DEL_PORTAL,
#endif
} NPD_MNG_ASD_ACT;

struct npd_mng_asd 
{
	unsigned int action; 
	unsigned int ifIndex;
	unsigned int vlanId;
	unsigned char mac[6];
	unsigned char unit;
	unsigned int uplink;
	unsigned int downlink;
	unsigned int link_state;
	unsigned int type;
	unsigned char enable;
	unsigned int ipaddr;
	unsigned int l4port;
	unsigned int flags;
};

struct npd_asd_item_s {
	//unsigned char isEnable;
	unsigned char authMode;
	unsigned int ifIndex;
	//unsigned int vlanId;
	//unsigned short guest_vlan_id;
	//unsigned short auth_fail_vlan_id;
	//unsigned short auth_success_vlan_id;
	unsigned char authorized;

};

#ifdef HAVE_PORTAL
#define MAX_PORTAL_SERVER_NUM 1
#define NAME_LEN	32

typedef struct npd_asd_portal_srv_s{
	char srvID;	
	char flag;   
	char enable;
	char type;
	int srv_addr;
	int srv_port;
}npd_asd_portal_srv;

typedef struct {
	char authtype;
	char state;
	unsigned char id[NAME_LEN+1];
	unsigned char passwd[NAME_LEN+1];
	unsigned int ipaddr;
	unsigned int netif_index;
}portal_user_info_s;
#endif



int npd_asd_set_dot1x_by_ifindex
( 
	unsigned int ifIndex, 
	unsigned int vlanId, 
	unsigned char flag
);

int npd_asd_get_authMode_by_ifindex
( 
	unsigned int ifIndex, 
	//unsigned int vlanId, 
	unsigned char *mode
);

int npd_asd_set_authMode_by_ifindex
( 
	unsigned int ifIndex, 
	unsigned int vlanId, 
	unsigned char mode
);

int npd_asd_recv_info
(
	struct npd_mng_asd *msg,
	unsigned int  infoLen,
	int *len
);

unsigned int npd_asd_recvmsg_proc(struct npd_mng_asd * asd_msg);

int npd_asd_msg_init(void);

int npd_asd_set_guest_vlan
( 
	unsigned int ifIndex, 
	unsigned short guest_vlan
);

int npd_asd_get_guest_vlan
( 
	unsigned int ifIndex, 
	unsigned short *guest_vlan
);

int npd_asd_set_auth_fail_vlan
( 
	unsigned int ifIndex, 
	unsigned short auth_fail_vlan
);

int npd_asd_get_auth_fail_vlan
( 
	unsigned int ifIndex, 
	unsigned short *auth_fail_vlan
);

int npd_asd_set_auth_success_vlan
( 
	unsigned int ifIndex, 
	unsigned short auth_success_vlan
);

int npd_asd_get_auth_success_vlan
( 
	unsigned int ifIndex, 
	unsigned short *auth_success_vlan
);


int npd_asd_check_authMode_MAB(unsigned int netif_index);

int npd_asd_send_new_mac_auth(
	unsigned long port_index, 
	unsigned short vlan_id, 
	unsigned char *mac);


#endif
