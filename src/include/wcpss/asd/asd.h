#ifndef _ASD_H_
#define	_ASD_H_
#include "wcpss/waw.h"

#define ASD_SECURITY_DB_NAME  "asdSecurityDB"
#define ASD_SECURITY_ARR_NAME "asdSecurityArr"
#define ASD_SECURITY_DB_SIZE  (port_max_num + PORTAL_NUM + WLAN_NUM + trunk_max_num)

#define ASD_RADIUS_DB_NAME "asdRadiusDB"
#define ASD_RADIUS_ARR_NAME "asdRadiusArr"
#define ASD_RADIUS_DB_SIZE (MAX_RADIUS_CONF_NUM)

#define ASD_LOCAL_DB_NAME "asdLocalUserDB"
#define ASD_LOCAL_HASH_NAME "asdLocalUserhash"
#define ASD_LOCAL_DB_SIZE 256

#define ASD_SYS_AUTH_DB_NAME "asdSysAuthDB"
#define ASD_SYS_AUTH_ARR_NAME "asdSysAuthArr"
#define ASD_SYS_CONTROL_DB_SIZE 3

#define RAD_ATTR_F_PREDEF 0x1
#define RAD_ATTR_F_AUTH_REQ 0x10
#define RAD_ATRR_F_AUTH_ACCEPT 0x20
#define RAD_ATTR_F_ACCT_REQ 0x40
#define RAD_ATTR_F_ACCT_RESP 0x80

#ifndef _BOOL_TEST
#define _BOOL_TEST 1
typedef unsigned char boolean;
#endif
typedef struct {
	unsigned int ACIP;
	unsigned char ACID;
	unsigned int GroupID;
}Mobility_AC_Info_T;

typedef struct {
    unsigned int Acmnum;
	unsigned char GroupID;
	unsigned char WLANID;
	unsigned char *ESSID;
	unsigned char *name;
	unsigned int host_ip;
	Mobility_AC_Info_T *Mobility_AC[G_AC_NUM];
}Inter_AC_R_Group_T;



typedef enum{
	HEX=2,
	ASCII=1
}key_input_type;


enum cert_type{
	WAPI_X509=1,
	WAPI_GBW=2
};

enum radius_server_filter_mode{
	RADIUS_SRV_FILTER_IP,
	RADIUS_SRV_FILTER_NAME
};

enum radius_attribute_format{
	RAD_ATTR_FORMAT_ASCII,
	RAD_ATTR_FORMAT_HEX
};

enum radius_attribute_usage{
	RAD_ATTR_USAGE_STD,               /*the attribute is used as standard*/
	RAD_ATTR_USAGE_STATIC,            /*the attribute is used as static string or hex number*/
	RAD_ATTR_USAGE_IP,                /*the attribute is used as IP address*/
	RAD_ATTR_USAGE_VLANID,            /*the attribute is used as VLAN ID*/
};

enum radius_type{
	RADIUS_ACCOUNTING_TYPE=0,
	RADIUS_AUTHENTICATION_TYPE=1
};


enum wapi_trap_type{
	ATTACK_INVALID_CERT=1,
	ATTACK_CHALLENGE_REPLAY=2,
	ATTACK_MIC_JUGGLE=3,
	ATTACK_LOW_SAFE_LEVEL=4,
	ATTACK_ADDR_REDIRECTION=5
};

enum asd_debug{ 
	ASD_DEFAULT = 0x1,
	ASD_DBUS = 0x2, 
	ASD_80211 = 0x4, 
	ASD_1X = 0x8, 
	ASD_WPA = 0x10, 
	ASD_WAPI = 0x20,
	ASD_ALL = 0x3f
};
enum asd_vlan_state{
	ASD_CONFIGED_VLAN = 1,
	ASD_GUEST_VLAN = 2,
	ASD_AUTH_FAIL_VLAN = 3,
	ASD_UNKNOWN_VLAN = 4
};



enum auth_mode{
	DEFAULT_EAP_MODE=1,
	EAP_PROXY_MODE=2,
	LOCAL_MODE = 3,
	TACACS_MODE = 4,
};
enum dot1x_statistic_type{
	EAPOL_FRAME_RX,
	EAPOL_FRAME_TX,
	EAPOL_START_FRAME_RX,
	EAPOL_LOGOFF_FRAME_RX,
	EAPOL_RESP_ID_RRAME_RX,
	EAPOL_RESP_FRAME_RX,
	EAPOL_REQ_ID_FRAME_TX,
	EAPOL_REQ_FRAME_TX,
	INVALID_EAPOL_FRAME_RX,
	EAP_LEN_ERR_FRAME_RX,
	LAST_EAPOL_FRAME_VERSION,
	LAST_EAPOL_SRC_MAC,
	UNKNOWN_STATISTICS_TYPE
};

typedef enum {
		WIRELESS_BLACK_LIST = 1,
		WIRELESS_WHITE_LIST = 2,
		WIRELESS_NONE_LIST = 0
} mac_filter_policy;

typedef struct {
 char *as_ip;
 unsigned int as_ip_len;
 unsigned int as_port;
 unsigned char multi_cert;	/*0-disable,1-enable*/
 char *certification_path;
 unsigned int certification_path_len;

 char *ae_cert_path;
 unsigned int ae_cert_path_len;
 char *ca_cert_path;
 unsigned int ca_cert_path_len;
 char *unite_cert_path;
 unsigned int unite_cert_path_len; 
 enum cert_type certification_type; 
}WAPI_AS;


struct wapi_sub_security{
	unsigned int 	CertificateUpdateCount;	/*default 3*/
	unsigned int 	MulticastUpdateCount;	/*default 3*/
	unsigned int 	UnicastUpdateCount;	/*default 3*/
	unsigned int 	BKLifetime;	/*default 43200*/
	unsigned int 	BKReauthThreshold;	/*default 70*/
	unsigned int 	SATimeout;	/*default 60*/
	unsigned char 	WapiPreauth;			/*default 0,no*/
	unsigned char   MulticaseRekeyStrict;   /*default 0,no*/	
	unsigned char 	UnicastCipherEnabled;			/*default 1,yes*/
	unsigned char   AuthenticationSuiteEnable;   /*default 1,yes*/
	unsigned char	MulticastCipher[4];			/*default 00-14-72-01*/
};

struct security {
 unsigned char type;
 unsigned int securityType;
 unsigned int encryptionType; 
 unsigned int SecurityID;
 unsigned int RadiusID;
 char name[32];
 key_input_type keyInputType;
 char SecurityKey[SECURITY_KEY_LEN];
 PortTypes port_dox1x_ctrl;//port control type
 int eap_reauth_priod;  /*xm 08/09/02*/
 boolean eap_reauth_ctrl; //jianchao add 10/07/20
 boolean link_status;//port link up or link down
// int acct_interim_interval;	/*ht 090205*/
 unsigned int guest_vlan_period;/*jianchao add 10/06/08*/
 boolean eapol_packet_recieved;//marks port have recieved eapol packet during lifetime
// enum asd_vlan_state vlan_state;//marks current vlan state
 port_auth_state auth_state;//marks port authentication state
 
 unsigned short untag_vlan_id;//user configured access vlan
 unsigned short auth_succ_vlan_id;  //vlan to join after auth success, it maybe come from AAA server 
 unsigned short guest_vlan_id; // guest vlan
 unsigned short auth_fail_vlan_id; //vlan to join after auth fail
 
 unsigned int quiet_Period; /*ht 090727*/
 unsigned int tx_period;/*jianchao add 10/06/08*/
 unsigned int supp_timeout;/*jianchao add 10/06/08*/
 unsigned int serv_timeout;/*jianchao add 10/06/08*/
 
 unsigned int max_retrans;/*jianchao add 10/06/08*/
 unsigned int max_users; /*max user num per port*/
 unsigned int extensible_auth;
// unsigned int radius_extend_attr;
 unsigned int pre_auth;
 unsigned int pmk_alive_period;
// unsigned int wired_radius;
// struct radius_client_data *asd_radius;
 WAPI_AS wapi_as;
 
 unsigned char wapi_ucast_rekey_method;
 unsigned int wapi_ucast_rekey_para_t;
 unsigned int wapi_ucast_rekey_para_p;

 unsigned char wapi_mcast_rekey_method;
 unsigned int wapi_mcast_rekey_para_t;
 unsigned int wapi_mcast_rekey_para_p;	/*	xm0701*/
	int wpa_group_rekey;
	int wpa_keyupdate_timeout;
 
 struct wapi_sub_security wapi_config;/*nl 091102*/

 unsigned char mac_authbypass;
#ifdef HAVE_PORTAL
 unsigned int portalSrvID;
#endif
 unsigned int wlan_flag;
};

typedef struct security security_profile;

/***********************************************/

/*typedef struct  {
       unsigned char slot;
       unsigned char port;
	   unsigned char stat;
}SLOT_PORT_ENABLE_S;*/

typedef struct  {
       unsigned int port_index;
	   unsigned int vlan_id;
	   unsigned char security;
}PORTINDEX_VLANID_S;
/***********************************************/
/*sz20080825*/ 
/*typedef struct  {
	   unsigned int port;
	   unsigned int vlanid;
       unsigned int stat;
}VLAN_PORT_ENABLE;*/

typedef struct  {
       unsigned int slot;
	   unsigned int port;
	   unsigned int portindex;
	   unsigned int vlanid;
	   unsigned int securityid;
}SLOT_PORT_VLAN_SECURITY;

typedef struct  {
	   unsigned int slot;
	   unsigned int port;
	   unsigned int portindex;
	   unsigned int vlanid;
       unsigned int stat;
}SLOT_PORT_VLAN_ENABLE;

typedef struct  {
       unsigned int vlanid;
       unsigned int port;
	   unsigned int securityid;
}VLAN_PORT_SECURITY;
/*sz20080825 */

struct _wtp_asd {
	unsigned int	WTPID;
	/*unsigned int	WTPModel;*/

 	unsigned int ap_accessed_sta_num;
 	unsigned int ap_max_allowed_sta_num;
	unsigned int wtp_triger_num;
	unsigned int wtp_flow_triger;
	struct acl_config *acl_conf;
	FLOW_IE      radio_flow_info[L_RADIO_NUM];   /*xm add*/

	unsigned char ra_ch[4];

	unsigned char 	WTPMAC[MAC_LEN];
	unsigned int 	WTPIP;
	char 			WTPSN[128];/*WTP Serial No*/
};
typedef struct _wtp_asd ASD_WTP_ST;  /*xm add  08/12/01*/


typedef struct  {

	   unsigned long wl_up_flow;		/*	无线上行端口的总流量*/
	   unsigned long wl_dw_flow;		/*	无线下行端口的总流量*/
	   
	   unsigned long ch_dw_pck;			/*	信道下行总的包数*/
	   unsigned long ch_dw_los_pck;		/*	信道下行总的丢包数*/
	   unsigned long ch_dw_mac_err_pck;	/*	信道下行总的MAC错包数*/
	   unsigned long ch_dw_resend_pck;	/*	信道下行总的重传包数*/
	   
	   unsigned long ch_up_frm;			/*	信道上行总的帧数*/
	   unsigned long ch_dw_frm;			/*	信道下行总的帧数*/
	   unsigned long ch_dw_err_frm;		/*	信道下行总的错帧数*/
	   unsigned long ch_dw_los_frm;		/*	信道下行总的丢帧数*/
	   unsigned long ch_dw_resend_frm;	/*	信道下行总的重传帧数*/

	   
	   unsigned long ch_up_los_frm;		/*信道上行总的丢帧数*/
	   unsigned long ch_up_resend_frm;

/*信道上行总的重传帧数*/
	   unsigned long long send_bytes;		/*发送的数据帧字节数*/
	   
}BSS_MIB_INFO_ST;	/*	xm0616*/


typedef struct  {

	unsigned char	bssid[MAC_LEN];				/*	xm0630*/
	
	unsigned char	ControlledAuthControl;		/*	是否启用鉴权(待定)*/
	unsigned char	ControlledPortControl;		/*	端口的控制类型*/
	unsigned char	wapiEnabled;				/*	是否启用WAPI*/
	
	unsigned long long	CertificateUpdateCount;		/*	鉴权握手的重试次数*/
	unsigned long long	MulticastUpdateCount;		/*	MSK握手的重试次数*/
	unsigned long long	UnicastUpdateCount;			/*	单播密钥握手的重试次数*/
	
	unsigned char	AuthenticationSuite;			/*	选择的AKM套件*/
	unsigned char	AuthSuiteSelected;				/*	选择的AKM*/
	   
}BSS_WAPI_MIB_INFO_ST;	/*	xm0623*/


typedef struct  {
	
	unsigned long	UnicastRekeyTime;		/*	单播密钥有效时间*/
	unsigned int	UnicastRekeyPackets;	/*	单播密钥有效的数据包数量*/
	
	unsigned long	MulticastRekeyTime;		/*	组播密钥有效时间*/
	unsigned int	MulticastRekeyPackets;	/*	组播密钥有效的数据包数量*/
	
	unsigned char	ControlledPortStatus;	/*	鉴权控制端口的状态*/

	unsigned char	BKIDUsed[16];			/*	上一个使用的BKID	//	xm0626*/

}STA_WAPI_MIB_INFO_ST;	/*	xm0623*/

struct maclist {
	unsigned char addr[MAC_LEN];
	unsigned char add_reason;	//0 for static,1 for dynamic
	unsigned char vapbssid[MAC_LEN];
	unsigned char attacktype;
	unsigned char frametype;
	unsigned char channel;
	unsigned char rssi;
	time_t add_time;
	struct maclist *next;
};


struct dcli_acl_config {
	unsigned int  	macaddr_acl;		/*0-none;1-black;2-white*/	
	unsigned char	wids_set;
	unsigned int 	wids_last_time;
	unsigned int 	num_deny_mac;
	unsigned int 	num_wids_mac;
	struct maclist *deny_mac;
	unsigned int 	num_accept_mac;
	struct maclist *accept_mac;
};

#define BIT(x) (1 << (x))

/* dcli_sta_info sta_flags copy from STA flags */
#define WLAN_STA_AUTH 				BIT(0)
#define WLAN_STA_ASSOC 				BIT(1)
#define WLAN_STA_PS 				BIT(2)
#define WLAN_STA_TIM 				BIT(3)
#define WLAN_STA_PERM 				BIT(4)
#define WLAN_STA_AUTHORIZED 		BIT(5)
#define WLAN_STA_PENDING_POLL 		BIT(6) /* pending activity poll not ACKed */
#define WLAN_STA_SHORT_PREAMBLE 	BIT(7)
#define WLAN_STA_PREAUTH 			BIT(8)
#define WLAN_STA_WME 				BIT(9)
#define WLAN_STA_MFP 				BIT(10)
#define WLAN_STA_HT 				BIT(11)
#define WLAN_STA_WPS 				BIT(12)
#define WLAN_STA_MAYBE_WPS 			BIT(13)
#define WLAN_STA_ROAMOUT			BIT(14)
#define WLAN_STA_ROAMING_L2			BIT(16)	/* L2 ROAM */  
#define WLAN_STA_ROAMING_L3			BIT(17) /* L3 ROAM */
#define WLAN_STA_REAUTH				BIT(18)	/* recv double auth */
#define WLAN_STA_BALANCE_MULASSOC	BIT(19)	/* wlan balance assoc more then 3 times */
#define WLAN_STA_BALANCE_NOAP		BIT(20)	/* no ap list to balance */
#define WLAN_STA_DEL 				BIT(27) /*set when ap told sta is leave*/
#define WLAN_STA_ROAMING 			BIT(28)
#define WLAN_STA_FREE 				BIT(29) 
#define WLAN_STA_AUTH_ACK 			BIT(30)
#define WLAN_STA_NONERP 			BIT(31)


struct dcli_sta_info
{
	struct dcli_sta_info *next; /* next entry in sta list */
	unsigned char addr[6];
	unsigned char *ip;	
	
	unsigned char 	wlan_id;
	unsigned long 	security_id;
	unsigned char	radio_l_id;
	unsigned int radio_g_id;
	unsigned int bssindex;
	unsigned int wtp_id;

	unsigned int port_id;
	unsigned int vlan_id;
	
	unsigned int sta_flags;
	unsigned int pae_state;
	unsigned int backend_state;
	
	unsigned int sta_traffic_limit;		 /*上行*//*指station 上传到AP的带宽限制 */
	unsigned int sta_send_traffic_limit; /*下行*//*指station 从AP下载的带宽限制 */

	time_t StaTime;		/*Access time*/
	
	unsigned int snr;
	unsigned long long rr;  
	unsigned long long tr;   //receive rate    transmit rate   byte
	unsigned long long tp;   //throughput
	unsigned long long rxbytes;
	unsigned long long txbytes;
	unsigned long long rxpackets;
	unsigned long long txpackets;
	unsigned long long retrybytes;
	unsigned long long retrypackets;
	unsigned long long errpackets;
	
	double flux ;

	/*sta info single intensity etc.*/
	unsigned char mode;  //11b-0x01,11a-0x02,11g-0x04,11n-0x08,
	unsigned char channel;
	unsigned char rssi;
	unsigned short nRate;
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned char info_channel;

	unsigned long	UnicastRekeyTime;		/*	单播密钥有效时间*/
	unsigned int	UnicastRekeyPackets;	/*	单播密钥有效的数据包数量*/
	
	unsigned long	MulticastRekeyTime; 	/*	组播密钥有效时间*/
	unsigned int	MulticastRekeyPackets;	/*	组播密钥有效的数据包数量*/
	
	unsigned char	ControlledPortStatus;	/*	鉴权控制端口的状态*/
	unsigned char	BKIDUsed[16];			/*	上一个使用的BK?ID*/
	
	/*packets*/
	unsigned long long rx_pkts ;
	unsigned long long tx_pkts ;
	unsigned long long rtx ; 
	unsigned long long rtx_pkts ;
	unsigned long long err_pkts ;

	

};

struct dcli_bss_info {
	struct dcli_bss_info *next;

	struct dcli_acl_config acl_conf;
	
	unsigned char 	mac[6];
	unsigned char 	WlanID;
	unsigned int 	SecurityID;
	unsigned char 	Radio_L_ID;
	unsigned int 	Radio_G_ID;
	unsigned int 	WtpID;
	unsigned int 	BSSIndex;
	unsigned char 	bssid[6];

	unsigned int 	PortID;		/*for wired bss*/
	unsigned int 	VlanID;

	unsigned int 	num_sta; 		/* number of entries in sta_list */
	struct dcli_sta_info *sta_list; /* STA info list head */
	struct dcli_sta_info *sta_last; /* STA info last head */

	unsigned int num_assoc;
	unsigned int num_reassoc;
	unsigned int num_assoc_failure;
	unsigned int assoc_success;
	unsigned int reassoc_success;

	/*statics info*/
	unsigned int access_sta_num ; 
	unsigned int auth_req_num ; 
	unsigned int auth_resp_num ; 
	unsigned int total_deny_num;
	
	unsigned int auth_success_num ; //5
	unsigned int auth_fail_num ; 
	unsigned int auth_invalid_num ; 
	unsigned int auth_timeout_num ; 
	unsigned int auth_refused_num ; 
	unsigned int auth_others_num ; //10
	unsigned int assoc_req_num ; 
	unsigned int assoc_resp_num ; 
	unsigned int assoc_invalid_num ; 
	unsigned int assoc_timeout_num ; 
	unsigned int assoc_refused_num ; //15
	unsigned int assoc_others_num ; 
	unsigned int reassoc_request_num ; 
	unsigned int reassoc_success_num ; 
	unsigned int reassoc_invalid_num ; 
	unsigned int reassoc_timeout_num ; //20
	unsigned int reassoc_refused_num ; 
	unsigned int reassoc_others_num ; 
	unsigned int identify_request_num ; 
	unsigned int identify_success_num ; 
	unsigned int abort_key_error_num ; //25
	unsigned int abort_invalid_num ; 
	unsigned int abort_timeout_num ; 
	unsigned int abort_refused_num ; 
	unsigned int abort_others_num ; 
	unsigned int deauth_request_num ; //30
	unsigned int deauth_user_leave_num ; 
	unsigned int deauth_ap_unable_num ; 
	unsigned int deauth_abnormal_num ; 
	unsigned int deauth_others_num ; 
	unsigned int disassoc_request_num ; //35
	unsigned int disassoc_user_leave_num ; 
	unsigned int disassoc_ap_unable_num ; 
	unsigned int disassoc_abnormal_num ; 
	unsigned int disassoc_others_num ; 

	unsigned int rx_mgmt_pkts ;//40
	unsigned int tx_mgmt_pkts ;
	unsigned int rx_ctrl_pkts ;
	unsigned int tx_ctrl_pkts ;
	unsigned int rx_data_pkts ;
	unsigned int tx_data_pkts ;//45
	unsigned int rx_auth_pkts ;
	unsigned int tx_auth_pkts ;

	unsigned int	traffic_limit;
	unsigned int	sta_average_traffic_limit;
	unsigned int	send_traffic_limit;	/*下行,ht add 090902*/
	unsigned int	sta_average_send_traffic_limit;//51

	unsigned int	acc_tms;
	unsigned int	auth_tms;
	unsigned int	repauth_tms;

	unsigned int	wai_sign_errors;
	unsigned int	wai_hamc_errors;
	unsigned int	wai_auth_res_fail;
	unsigned int	wai_discard;
	unsigned int	wai_timeout;
	unsigned int	wai_format_erros;
	unsigned int	wai_cert_handshake_fail;
	unsigned int	wai_unicast_handshake_fail;
	unsigned int	wai_multi_handshake_fail;
	unsigned int	wpi_mic_errors;		// D8.2.3
	unsigned int	wpi_replay_counters;
	unsigned int	wpi_decryptable_errors;
	unsigned char 	WapiEnabled;

	unsigned long wl_up_flow;			/*	无线上行端口的总流量*/
	unsigned long wl_dw_flow;			/*	无线下行端口的总流量*/
	   
	unsigned long ch_dw_pck;			/*	信道下行总的包数*/
	unsigned long ch_dw_los_pck;		/*	信道下行总的丢包数*/
	unsigned long ch_dw_mac_err_pck;	/*	信道下行总的MAC错包数*/
	unsigned long ch_dw_resend_pck; 	/*	信道下行总的重传包数*/
	   
	unsigned long ch_up_frm;			/*	信道上行总的帧数*/
	unsigned long ch_dw_frm;			/*	信道下行总的帧数*/
	unsigned long ch_dw_err_frm;		/*	信道下行总的错帧数*/
	unsigned long ch_dw_los_frm;		/*	信道下行总的丢帧数*/
	unsigned long ch_dw_resend_frm; 	/*	信道下行总的重传帧*/

	unsigned long ch_up_los_frm;		/*信道上行总的丢帧数*/
	unsigned long ch_up_resend_frm; 	/*信道上行总的重传帧数*/		
	unsigned long long send_bytes;		/*发送的数据帧字节数*/

	unsigned char	ControlledAuthControl;		/*	是否启用鉴权(待定)*/
	unsigned char	ControlledPortControl;		/*	端口的控制类型*/
	unsigned char	wapiEnabled;				/*	是否启用WAPI*/
	
	unsigned long long	CertificateUpdateCount; 	/*	鉴权握手的重试次数*/
	unsigned long long	MulticastUpdateCount;		/*	MSK握手的重试次数*/
	unsigned long long	UnicastUpdateCount; 		/*	单播密钥握手的重试次数*/
	
	unsigned char	AuthenticationSuite;			/*	选择的AKM套件*/
	unsigned char	AuthSuiteSelected;				/*	选择的AKM*/


};


struct dcli_wtp_info{
	struct dcli_wtp_info *next;

	struct dcli_bss_info *bss_list;
	struct dcli_bss_info *bss_last;
	
	struct dcli_radio_info *radio_list;
	struct dcli_radio_info *radio_last;
	
	struct dcli_acl_config acl_conf;
	
	unsigned int 	WtpID;
	unsigned char  	mac[6];

	unsigned int 	num_sta;
	unsigned int 	num_bss;
	unsigned int    num_radio;

	unsigned int 	num_assoc;
	unsigned int 	num_reassoc;
	unsigned int 	num_assoc_failure;
	unsigned int 	num_normal_sta_down;
	unsigned int	num_abnormal_sta_down;

	unsigned long long	wtp_total_past_online_time;	//	xm0703	
	unsigned int		acc_tms;
	unsigned int		auth_tms;
	unsigned int		repauth_tms;
	unsigned int		auth_success_num;
	unsigned int		auth_fail_num;
	unsigned int		auth_invalid_num;
	unsigned int		auth_timeout_num;
	unsigned int		auth_refused_num;		//10
	unsigned int		auth_others_num;
	unsigned int		assoc_req_num;
	unsigned int		assoc_resp_num;
	unsigned int		assoc_invalid_num;
	unsigned int		assoc_timeout_num;		//15
	unsigned int		assoc_refused_num;
	unsigned int		assoc_others_num;
	unsigned int		reassoc_request_num;
	unsigned int		reassoc_success_num;
	unsigned int		reassoc_invalid_num;		//20
	unsigned int		reassoc_timeout_num;
	unsigned int		reassoc_refused_num;
	unsigned int		reassoc_others_num;
	unsigned int		identify_request_num;
	unsigned int		identify_success_num;		//25
	unsigned int		abort_key_error_num;
	unsigned int		abort_invalid_num;
	unsigned int		abort_timeout_num;
	unsigned int		abort_refused_num;
	unsigned int		abort_others_num;		//30											
	unsigned int		deauth_request_num;
	unsigned int		deauth_user_leave_num;
	unsigned int		deauth_ap_unable_num;
	unsigned int		deauth_abnormal_num;
	unsigned int		deauth_others_num;		//35
	unsigned int		disassoc_request_num;
	unsigned int		disassoc_user_leave_num;
	unsigned int		disassoc_ap_unable_num;
	unsigned int		disassoc_abnormal_num;
	unsigned int		disassoc_others_num;		//40	
	unsigned int		rx_mgmt_pkts;
	unsigned int		tx_mgmt_pkts;
	unsigned int		rx_ctrl_pkts;
	unsigned int		tx_ctrl_pkts;
	unsigned int		rx_data_pkts;	//45
	unsigned int		tx_data_pkts;
	unsigned int		rx_auth_pkts;
	unsigned int		tx_auth_pkts;	//48
		
	unsigned int 	ap_wapi_version ;
	unsigned int 	wai_sign_errors ;
	unsigned int 	wai_hamc_errors ;
	unsigned int 	wai_auth_res_fail ;
	unsigned int 	wai_discard ;	//5
	unsigned int 	wai_timeout ;
	unsigned int 	wai_format_erros ;
	unsigned int 	wai_cert_handshake_fail ;
	unsigned int 	wai_unicast_handshake_fail ;
	unsigned int 	wai_multi_handshake_fail ; //10
	unsigned int 	wpi_mic_errors ;
	unsigned int 	wpi_replay_counters ;
	unsigned int 	wpi_decryptable_errors ;//13


#if 1	
/*for extern show wtp sta*/
	unsigned int deny_num ;
	unsigned int bss_deny_num ;


#endif
};

struct dcli_radio_info{
	struct dcli_radio_info *next;
	unsigned int radioid;			
	unsigned int acc_tms;
	unsigned int auth_tms;
	unsigned int repauth_tms;						
	unsigned int auth_success_num;//5
	unsigned int auth_fail_num;
	unsigned int auth_invalid_num;
	unsigned int auth_timeout_num;
	unsigned int auth_refused_num;
	unsigned int auth_others_num;//10
						
	unsigned int assoc_req_num;
	unsigned int assoc_resp_num;
				
	unsigned int assoc_invalid_num;
	unsigned int assoc_timeout_num;
	unsigned int assoc_refused_num;//15
	unsigned int assoc_others_num;
						
	unsigned int reassoc_request_num;
	unsigned int reassoc_success_num;
	unsigned int reassoc_invalid_num;
	unsigned int reassoc_timeout_num;//20
	unsigned int reassoc_refused_num;
	unsigned int reassoc_others_num;
						
	unsigned int identify_request_num;
	unsigned int identify_success_num;
	unsigned int abort_key_error_num;//25
	unsigned int abort_invalid_num;
	unsigned int abort_timeout_num;
	unsigned int abort_refused_num;
	unsigned int abort_others_num;
	
	unsigned int deauth_request_num;//30
	unsigned int deauth_user_leave_num;
	unsigned int deauth_ap_unable_num;
	unsigned int deauth_abnormal_num;
	unsigned int deauth_others_num;
	
	unsigned int disassoc_request_num;//35
	unsigned int disassoc_user_leave_num;
	unsigned int disassoc_ap_unable_num;
	unsigned int disassoc_abnormal_num;
	unsigned int disassoc_others_num;

	unsigned int rx_mgmt_pkts;//40
	unsigned int tx_mgmt_pkts;
	unsigned int rx_ctrl_pkts;
	unsigned int tx_ctrl_pkts;
	unsigned int rx_data_pkts;
	unsigned int tx_data_pkts;//45
	unsigned int rx_auth_pkts;
	unsigned int tx_auth_pkts;//47

	unsigned int num_bss;			
	struct dcli_bss_info *bss_list;
	struct dcli_bss_info *bss_last;

};

struct dcli_wlan_info{
	struct dcli_wlan_info *next;
	unsigned char 	WlanID;

	unsigned int 	num_assoc;
	unsigned int 	num_reassoc;
	unsigned int 	num_assoc_failure;
	unsigned int 	num_normal_sta_down;
	unsigned int	num_abnormal_sta_down;
	
	unsigned int rx_pkts ;	
	unsigned int tx_pkts ;	
	unsigned int assoc_req_num ; 
	unsigned int assoc_resp_num ; 
	unsigned long long rx_bytes ;
	unsigned long long tx_bytes ;

	unsigned int assoc_fail_num ;
	unsigned int sta_assoced_num;
	unsigned int reassoc_num ; 
	unsigned int reassoc_success_num ; 

	unsigned int assoc_req_interim ;
	unsigned int assoc_resp_interim ;
	unsigned int assoc_success_interim ;

	unsigned int 	num_sta;
	unsigned int 	num_bss;
	struct dcli_acl_config	 acl_conf;
	struct dcli_bss_info 	*bss_list;
	struct dcli_bss_info 	*bss_last;
};

struct dcli_channel_info {
	
	struct dcli_channel_info *next ;

	struct dcli_channel_info *channel_list ;
	struct dcli_channel_info *channel_last ;
	
	unsigned char channel_id ;
	unsigned int sta_num ;
	time_t StaTime ;
	
} ;


struct dcli_ac_info{
	unsigned int 	num_assoc;
	unsigned int 	num_reassoc;
	unsigned int 	num_assoc_failure;
	unsigned int 	num_normal_sta_down;
	unsigned int	num_abnormal_sta_down;

	unsigned int 	num_sta;
	unsigned int 	num_sta_wired;
	unsigned int 	num_sta_all;
	unsigned int 	num_local_roam;
	unsigned int	num_unconnect_sta;
	unsigned int 	num_bss_wireless;
	unsigned int 	num_bss_wired;	
	unsigned int 	num_bss;
	
	unsigned int 	num_wtp;
	unsigned int 	num_wlan;
	struct dcli_bss_info *bss_list;
	struct dcli_bss_info *bss_last;
	struct dcli_wtp_info *wtp_list;
	struct dcli_wtp_info *wtp_last;
	struct dcli_wlan_info *wlan_list;
	struct dcli_wlan_info *wlan_last;
};


struct dcli_security {
	struct dcli_security 	*next;
	unsigned int 	SecurityType;
	unsigned int 	EncryptionType; 
	unsigned int 	SecurityID;
	char 			*name;
	char 			*host_ip;
	char 			*SecurityKey;
	unsigned int	keyInputType;
	int 			eap_reauth_period;  
	int 			acct_interim_interval;	
	unsigned int 	quiet_period;
	unsigned int 	extensible_auth;
//	unsigned int 	radius_extend_attr;
	unsigned int 	pre_auth;
	unsigned int 	wired_radius;
	WAPI_AS 	 	wapi_as;
	 
	unsigned char 	wapi_ucast_rekey_method;
	unsigned int 	wapi_ucast_rekey_para_t;
	unsigned int 	wapi_ucast_rekey_para_p;
	unsigned char 	wapi_mcast_rekey_method;
	unsigned int 	wapi_mcast_rekey_para_t;
	unsigned int 	wapi_mcast_rekey_para_p;	
	unsigned char 	WapiPreauth;			/*default 0,no*/
	unsigned char   MulticaseRekeyStrict;   /*default 0,no*/
	unsigned int 	CertificateUpdateCount;	/*default 3*/
	unsigned int 	MulticastUpdateCount;	/*default 3*/
	unsigned int 	UnicastUpdateCount;	/*default 3*/
	unsigned int 	BKLifetime;	/*default 43200*/
	unsigned int 	BKReauthThreshold;	/*default 70*/
	unsigned int 	SATimeout;	/*default 60*/
	unsigned char 	UnicastCipherEnabled;			/*default 1,yes*/
	unsigned char   AuthenticationSuiteEnable;   /*default 1,yes*/
	unsigned char	MulticastCipher[4];	
	unsigned char 	RadiusID;


	unsigned char	AuthenticationSuiteEnabled;
	unsigned char	IsInstalledCer;
	char			RekeyMethod[30];
	unsigned char	ControlledAuthControl;
	unsigned int  	ConfigVersion;
	unsigned char	UnicastKeysSupported;
	unsigned char	WapiSupported;
	unsigned int  	MulticastCipherSize ;
	unsigned int  	UnicastCipherSize ;
	unsigned int 	MulticastRekeyPackets ;	
	unsigned int 	UnicastRekeyPackets ;
	unsigned char 	UnicastCipher[4];
	unsigned char 	AuthenticationSuite[4];
	unsigned char 	UnicastCipherSelected[4];
	unsigned char 	MulticastCipherSelected[4];
	unsigned char 	UnicastCipherRequested[4];
	unsigned char 	MulticastCipherRequested[4];
	unsigned char 	AuthSuitSelected_Auth[4];	
	unsigned char 	AuthSuitRequested_Psk[4];
	unsigned char 	MulticastRekeyStrict;
	unsigned char	WapiPreauthEnabled;
};

struct sta_static_info {
	struct sta_static_info *next; 
	struct sta_static_info *hnext; 
	unsigned char	addr[6];
	unsigned int 	vlan_id;
	unsigned int	sta_traffic_limit;		/*上行*//*指station 上传到AP的带宽限制 */
	unsigned int	sta_send_traffic_limit; /*下行*//*指station 从AP下载的带宽限制 */
};

typedef struct{
	unsigned char identity[NAME_LEN+1];
	unsigned int Security_ID;
	unsigned int WlanID;
	unsigned int BSSIndex;
	unsigned int Radio_G_ID;
	unsigned int WTPID;
	unsigned char ip_addr[IP_LENGTH];
	unsigned char mac[6];
}wireless_user_info_st;

 struct  asd_local_user{
	char username[LOCAL_USERNAME_LEN+1];
	char password[LOCAL_USERPASSWORD_LEN+1];
};

typedef struct{
	unsigned char identity[NAME_LEN+1];
	unsigned int netif_index;
	unsigned short vlan_id;
	unsigned char ip_addr[IP_LENGTH];
	unsigned char mac[6];
}wired_user_info_st;

#ifdef HAVE_PORTAL

#define MAX_PORTAL_SERVER_NUM    1
#define MAX_INTF_BIND_PORTAL_SRV 8

#define PORTAL_SRV_F_VALID    0x1

typedef struct asd_portal_srv_stc{
	char flag;   
	char enable;
	char type;
	char srvID;	
	int srv_addr;
	int srv_port;
	char URL[DEFAULT_LEN];
	char key[NAME_LEN+1];
}asd_portal_srv;

typedef struct asd_portal_cfg_stc{
	int enable;
	asd_portal_srv portalSrv[MAX_PORTAL_SERVER_NUM];
}asd_portal_cfg;

#endif
//extern security_profile *ASD_SECURITY[];
extern ASD_WTP_ST **ASD_WTP_AP;

#define MAX_NUM_OF_VLANID 256
#define __NETGEAR__

//int asd_port_sta_transfer(unsigned long port_index, unsigned short s_vlan, unsigned short d_vlan);


#define DEFAULT_SECURITY_TYPE IEEE8021X
#define DEFAULT_ENCRYPTION_TYPE WEP
#define DEFAULT_KEY_INPUT_TYPE 
#define DEFAULT_PORT_DOT1X_CONTROL Init_state
#define DEFAULT_EAP_REAUTH_PERIOD 3600
#define DEFAULT_EAP_REAUTH_CONTROL TRUE
#define DEFAULT_GUEST_VLAN_PERIOD 90
#define DEFAULT_GUEST_VLAN_ID 0
#define DEFAULT_AUTH_FAIL_VLAN_ID 0
#define DEFAULT_QUIET_PERIOD 60
#define DEFAULT_TX_PERIOD 30
#define DEFAULT_SUPP_TIMEOUT 30
#define DEFAULT_SERV_TIMEOUT 30
#define DEFAULT_MAX_RETRANS 2
#define DEFAULT_MAX_USERS 1024
#define DEFAULT_MAB FALSE
#define DEFAULT_MAC_PERIOD 1
#define DEFAULT_EXTENSIBLE_AUTH FALSE
#define DEFAULT_PREAUTH FALSE
#define DEFAULT_SYSTEM_AUTH_CONTROL FALSE
#define DEFAULT_AUTH_MODE DEFAULT_EAP_MODE
#define DEFAULT_RADIUS_HOST_IP "127.0.0.1"
#define DEFAULT_ACCOUNTING_DISABLE FALSE
#define DEFAULT_RADIUS_RETRANSMIT_TIMES 4
#define DEFAULT_RADIUS_RETRANSMIT_PERIOD 3

#endif
