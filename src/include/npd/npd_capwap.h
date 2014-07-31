#ifndef __NPD_CAPWAP_H__
#define __NPD_CAPWAP_H__

/* include header file begin */
/* kernel or sys part */
/* user or app part */
/* include header file end */

#define NPD_CAPWAP_TUNNEL_NAME					"npdCapwapTunnelHashTbl"
#define NPD_CAPWAP_BSSID_NAME					"npdCapwapBssidTbl"
#define NPD_CAPWAP_TS_QOS_NAME					"npdCapwapTsQosHashTbl"
#define NPD_CAPWAP_TT_QOS_NAME					"npdCapwapTtQosTbl"
#define NPD_CAPWAP_GLOBAL_CONF					"npdCapwapGlobalConf"
#define NPD_CAPWAP_GLOBAL_ARR_TABLE_NAME		"npdCapwapGlobalConfArr"
#define NPD_CAPWAP_L3_CONF						"npdCapwapGlobalConf"
#define NPD_CAPWAP_L3_ARR_TABLE_NAME			"npdCapwapGlobalConfArr"


#define NPD_CAPWAP_TUNNEL_TABLE_SIZE			(512)
#define NPD_CAPWAP_BSSID_TABLE_SIZE				(2048)
#define NPD_CAPWAP_TS_QOS_TABLE_SIZE			(128)
#define NPD_CAPWAP_TT_QOS_TABLE_SIZE			(128)
#define NPD_CAPWAP_GLOBAL_CONF_TABLE_SIZE		(1)
#define NPD_CAPWAP_L3_CONF_TABLE_SIZE			(1)


#define NPD_CAPWAP_TUNNEL_HASH_TABLE_SIZE		(64)
#define NPD_CAPWAP_BSSID_HASH_TABLE_SIZE		(256)
#define NPD_CAPWAP_TS_QOS_HASH_TABLE_SIZE		(16)
#define NPD_CAPWAP_TT_QOS_HASH_TABLE_SIZE		(16)

#define NPD_CAPWAP_GLOBAL_CONF_ID				(0)
#define NPD_CAPWAP_L3_CONF_ID					(0)


#define NPD_CAPWAP_RETURN_CODE_BASE				(0x300000)

#define DAL_ENABLE								1
#define DAL_DISABLE								0 

#define CAPWAP_TTL_DEFAULT_VALUE				64

#define CAPWAP_TUNNEL_TERM_CTRL_PORT            (5246)
#define CAPWAP_TUNNEL_TERM_DATA_PORT            (5247)

typedef enum{
	CAPWAP_TUNNEL,
	CAPWAP_BSSID,
	CAPWAP_WLAN,
	CAPWAP_UFDB,
	CAPWAP_TS_QOS,
	CAPWAP_TT_QOS,
	CAPWAP_GLOBAL_CONF,
	CAPWAP_L3_INT_CONF,
	CAPWAP_WIFI_INDICATOR_MODE_CONF,
	CAPWAP_WIFI_INDICATOR_STA_THRESHOLD,
	CAPWAP_AP_AUTO_DETECT_CONF,
	CAPWAP_INVALID_TYPE
}CAPWAP_TO_NPD_TYPE;


#define L3_NAME_LEN	64
typedef struct npd_capwap_global_conf_item_s {
	unsigned char	global_en_dis;
}npd_capwap_global_conf_t;

typedef struct npd_capwap_L3_conf_item_s {
	unsigned char 	l3_interface_name[L3_NAME_LEN];
	unsigned int 	ip_addr;
}npd_capwap_L3_conf_t;
typedef struct npd_capwap_wifi_indicator_mode_conf_item_s {
	unsigned char 	indicator_mode;
	unsigned int 	ap_id;
	unsigned int 	station_threshold;
}npd_capwap_wifi_indicator_mode_conf_item_t;


struct npd_mng_capwap 
{
	unsigned char 			msg_type;
	unsigned int			cap_action;
	union{
		struct capwap_db_tunnel_entry_s		tunnel_db_entry;
		struct npd_bssid_item_s				bssid_entry;
		struct npd_ts_qos_item_s			ts_qos_entry;
		struct npd_tt_qos_item_s			tt_qos_entry;
		npd_capwap_L3_conf_t				capwap_l3_conf;
		npd_capwap_port_pvid_t				capwap_port_pvid_conf;
		npd_capwap_wifi_indicator_mode_conf_item_t wifi_indicator_conf;
	}u;
};
struct capwap_bss_s {
    unsigned int netif_index;
    int state;
    unsigned int switch_port_index;
};
struct wifi_indicator_ops {
int* (*init)( unsigned int ap_id);
int* (*modify_station_threshold)( unsigned int ap_id);
int* (*ap_login)(unsigned int ap_id, void *argv);
int* (*ap_logout)(unsigned int ap_id, void *argv);
int* (*station_login)(unsigned int ap_id, void *argv);
int* (*station_logout)(unsigned int ap_id, void *argv);
int* (*station_arp_add)(unsigned int ap_id, void *argv);
int* (*station_arp_del)(unsigned int ap_id, void *argv);
int* (*throughput_check)(unsigned int ap_id);
};

typedef struct wifi_indicator_ip_status_s{
	unsigned int	ip_addr;
	unsigned char	status;
}wifi_indicator_ip_status_t;


void* npd_capwap_mng_thread_dgram(void);
void* npd_capwap_wifi_indicator_control_thread(void);
#endif
