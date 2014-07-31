#ifndef __COMMON_CAPWAP_API_H__
#define __COMMON_CAPWAP_API_H__

#define CAPWAP_PORT_MAX_VLAN_NUM	17


#define WIFI_INDICATOR_EVENT_SWITCH_MODE				1
#define WIFI_INDICATOR_EVENT_MODIFY_STATION_THRESHOLD	2
#define WIFI_INDICATOR_EVENT_AP_LOGIN					3
#define WIFI_INDICATOR_EVENT_AP_LOGOUT					4
#define WIFI_INDICATOR_EVENT_STA_LOGIN					5
#define WIFI_INDICATOR_EVENT_STA_LOGOUT					6
#define WIFI_INDICATOR_EVENT_STA_ARP_ADD				7
#define WIFI_INDICATOR_EVENT_STA_ARP_DEL				8
#define WIFI_INDICATOR_EVENT_THROUGHPUT_CHECK			9


typedef unsigned char npd_bssid_t[6];

typedef struct npd_wlan_key_s {
    npd_bssid_t bssid;
} npd_wlan_key_t;

struct npd_tunnel_item_s {
    unsigned int cflag;
    unsigned int type;
    unsigned int remote_ip_ver;
    unsigned int remote_ip4;
    unsigned int remote_ip6[4];
    unsigned int remote_l4_port;
    unsigned int l4_type;
    unsigned int mtu;
    unsigned int frame_format;
    unsigned int radio_mac_incl;
    unsigned int vlan_generation_en;
};
struct npd_bssid_item_s {
	unsigned int	netif_index;
	npd_wlan_key_t	BSS_ID;
	unsigned int	tunnel_id;
	unsigned short	pvid;
	unsigned int	cflag;
	unsigned int	swith_port_index;
};
struct npd_ts_qos_item_s {
	unsigned int	CAPWAP_DSCP:6;
	unsigned int	WLAN_TID:4;
	unsigned int	HDR_DP:1;
	unsigned int	HDR_PRI:4;
	unsigned int	HDR_CDEI:1;
	unsigned int	HDR_CPCP:3;
	unsigned int	HDR_SDEI:1;
	unsigned int	HDR_SPCP:3;
};
struct npd_tt_qos_item_s {
	unsigned int	HDR_DP:1;
	unsigned int	HDR_PRI:4;
	unsigned int	HDR_CDEI:1;
	unsigned int	HDR_CPCP:3;
	unsigned int	HDR_SDEI:1;
	unsigned int	HDR_SPCP:3;
};


typedef struct npd_capwap_port_vlan_ref_s{
	unsigned short vlan_array[CAPWAP_PORT_MAX_VLAN_NUM];
	unsigned char  ref_cnt[CAPWAP_PORT_MAX_VLAN_NUM];
}npd_capwap_port_vlan_ref_t;



typedef struct npd_capwap_tunnel_entry_s {
    unsigned int cflag;
    unsigned int type;
    unsigned int remote_ip_ver;
    unsigned int remote_ip4;
    unsigned int remote_ip6[4];
    unsigned int remote_l4_port;
    unsigned int l4_type;
    unsigned int mtu;
    unsigned int frame_format;
    unsigned int radio_mac_incl;
    unsigned int vlan_generation_en;
	unsigned char mac[6];
} npd_capwap_tunnel_entry_t;

typedef struct capwap_db_tunnel_entry_s {
	unsigned int tunnel_id;
	npd_capwap_tunnel_entry_t tunnel_entry;
} capwap_db_tunnel_entry_t;

typedef struct npd_capwap_wlan_s {
    unsigned int cflag;
    unsigned int radio_id;          /* Radio ID */
    unsigned int tunnel_id;
} npd_capwap_wlan_t;
typedef struct npd_wlan_vlan_s {
    unsigned int cflag;
    unsigned int svid_cmd;
    unsigned int cvid_cmd;
    unsigned int svid;
    unsigned int cvid;
} npd_wlan_vlan_t;

typedef struct npd_wlan_client_key_s {
    npd_bssid_t mac;
    unsigned int vid;
} npd_wlan_client_key_t;

typedef struct npd_wlan_client_s {
    unsigned int cflag;
    npd_bssid_t wtp;            /* The access point */
    unsigned int tid_en;
} npd_wlan_client_t;


typedef struct npd_capwap_port_pvid_s {
	unsigned short	join_pvid;
	unsigned short	leave_pvid;
}npd_capwap_port_pvid_t;
struct npd_capwap_global_conf_s {
	unsigned int	global_status;
};

struct npd_capwap_L3_conf_s {
	unsigned char	L3_ifname[64];
	unsigned int	ip_addr;
	unsigned int	L3_status;
	npd_capwap_port_vlan_ref_t capwap_port_vlan_ref;
	unsigned char	wifi_indicator_mode;
	unsigned int	ap_id;
	unsigned int	station_threshold;
};


#endif
