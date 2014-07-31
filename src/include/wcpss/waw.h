#ifndef _WID_ASD_WSM_H
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include "lib/netif_index.h"
#include "lib/npd_bitop.h"

#define	_WID_ASD_WSM_H

typedef unsigned char wlan_t;		/* WLAN ID type */
typedef unsigned char security_t;	/* security ID type */

#define IPv4_LEN	4
#define IPv6_LEN	16
#define MAC_LEN	6
#define PMK_LEN 32
#define PMKID_LEN 16
#define DEFAULT_LEN	256
#define ACIfaces_MAX_NUM 16
#define NAME_LEN	32
#define IF_NAME_MAX 16
#define NAS_PORT_ID_LEN	11
#define LOCAL_USERNAME_LEN 32
#define LOCAL_USERPASSWORD_LEN 32
#define CHALLENGE_LEN     16
#define CHAP_MD5_LEN      16
#define EAP_MD5_CHALL_VALUE_SIZE_LEN 1

#define WLAN_NUM		17
#define L_RADIO_NUM		4
#define L_BSS_NUM	8
#define dot3_Max_Len	1518
#define dot11_Max_Len		2336	
#define port_max_num	MAX_SWITCHPORT_PER_SYSTEM
#define vlan_max_num	4096
#define trunk_max_num  CHASSIS_TRUNK_RANGE_MAX
#define QOS_NUM		16
#define TOTAL_AP_IF_NUM		16
#define EBR_NUM		1024
#define WTP_DEFAULT_NUM		25          /* Lovin change 128 to 64(wid no used) */
#define WTP_MAX_MAX_NUM		25        /* Lovin change 2048 to 64 */
#define WTP_DEFAULT_NUM_AUTELAN	1
#define WTP_DEFAULT_NUM_AU4600	2
#define WTP_DEFAULT_NUM_OEM		8
#define THREAD_NUM	1
#define SOCK_NUM 1 /*wuwl add */
#define SOCK_BUFSIZE (512*1024)
#define ACIPLIST_NUM		129
#define MIXIPLEN   16
#define ESSID_LENGTH	32
#define PATH_LEN 64
#define WID_BAK_AC_PORT		19528
#define ASD_BAK_AC_PORT		29527
#define G_AC_NUM	16
#define GROUP_NUM	4
#define STA_HASH_SIZE 256

#define PORTAL_NUM   1

#define SECURITY_KEY_LEN  64

#define	DEFAULT_VLAN_ID			0x1

#define PREFERRED_AC_PING_COUNT 3
#define PREFERRED_AC_PING_INTERVAL 300
#define PREFERRED_AC_PING_TIMEOUT_WAIT 3

/* #define ASD_MULTI_THREAD_MODE */

#define IP_LENGTH 16

#define NPD_CAPWAP_TUNNEL_TYPE_WLAN    0
#define NPD_CAPWAP_TUNNEL_TYPE_ROAMING 1

#define PRODUCT_BASE_MAC_ADDRESS_PATH "/devinfo/mac"
#define PRODUCT_ENTERPRISE_NAME_PATH "/devinfo/enterprise_name"
#define PRODUCT_MODULE_NAME_PATH "/devinfo/module_name"

#define STRING_TRUE			"TRUE"
#define STRING_FALSE		"FALSE"
#define STRING_ENABLE		"ENABLE"
#define STRING_DISABLE		"DISABLE"
#define STRING_YES			"YES"
#define STRING_NO			"NO"
#define STRING_UP			"UP"
#define STRING_DOWN			"DOWN"
#define STRING_ON			"ON"
#define STRING_OFF			"OFF"
#define STRING_ASCII		"ASCII"
#define STRING_HEX			"HEX"
#define STRING_UNKNOWN		"UNKNOWN"

#define DEFAULT_WLAN		1

#define ELEMENT_NUM(a) sizeof(a)/sizeof(a[0])

#define	WIFI_INDICATOR_AP_MODE			0
#define	WIFI_INDICATOR_STATION_MODE		1

#define WIFI_INDICATOR_STATION_THRESHOLD	5


/***jianchao add 2014-01-06 for wireless terminal isolation***/
#define FAL_WLAN_FLAG_CLIENT_ISO_EN			1 << 0
#define FAL_WLAN_CLIENT_ISOLATION_MODE_PROMICUOUS	0
#define FAL_WLAN_CLIENT_ISOLATION_MODE_ISOLATED		1
#define FAL_WLAN_CLIENT_ISOLATION_MODE_COMMUNITY1	2
#define FAL_WLAN_CLIENT_ISOLATION_MODE_COMMUNITY2	3
/**************************************************/

extern int WTP_NUM;		
extern int G_RADIO_NUM;
extern int BSS_NUM;

enum {
	VRRP_REG_IF,
	VRRP_UNREG_IF
};

typedef enum {
	NO_ROAMING = 0,
	ROAMING_AC = 1,
	RAW_AC = 2
}roam_type;
extern int *gmax_wtp_count;
extern int *gcurrent_wtp_count;
extern int glicensecount ;

struct bak_sock{
	int sock;
	unsigned int ip;
	struct bak_sock *next;
};

typedef enum {
	NO_IF = 0,
	WLAN_IF = 1,
	BSS_IF = 2
}wAW_IF_Type;

typedef unsigned int Counter;

typedef struct {
	unsigned int cipher;
	unsigned int BSSIndex;
	unsigned int WTPID;		
	unsigned int key_len;
	int wpa_group_rekey;
	char key_idx;
	char StaAddr[MAC_LEN];
	char key[DEFAULT_LEN];
	unsigned char SecurityIndex;
	unsigned char OldSecurityIndex;  /*fengwenchao add 20110309 记录更换安全策略之前的安全策略*/
	unsigned char OldSecurityIndex_flag;   /*fengwenchao add 20110310 记录先前的安全策略是否为(open||shared)&&(wep),1-->YES,2-->NO*/	
}wAW_StaKey;

typedef struct
{
	unsigned int wtpid;
	unsigned int g_radio;
	unsigned int l_radio;
	unsigned int bssindex;
	unsigned char wlanid;
	unsigned char pmkid_len;
	unsigned char StaAddr[MAC_LEN];		// STA MAC
	unsigned char pmk[PMK_LEN];
	unsigned char pmkid[PMKID_LEN];
	unsigned char roamflag;
	unsigned char BSSID[MAC_LEN];
}wASD_PMK;


typedef struct{
	unsigned char WlanID;	
	unsigned int SecurityID;
	unsigned int SecurityType;
	unsigned int EncryptionType;
	char WlanName[DEFAULT_LEN];
	char ESSID[DEFAULT_LEN];
	unsigned char WlanState;
	wAW_StaKey WlanKey;
	unsigned int   wlan_max_sta_num;
	unsigned int   balance_para;
	unsigned int   flow_balance_para;
	unsigned char  balance_switch;
	unsigned char  balance_method;
	unsigned char  Roaming_policy;
	char  	as_ip[DEFAULT_LEN];
	unsigned int as_ip_len;
	char  	cert_path[DEFAULT_LEN];
	unsigned int cert_path_len;
	
	char  	ae_cert_path[DEFAULT_LEN];
	unsigned int ae_cert_path_len;
	
	unsigned int ap_max_inactivity; 	   //weichao add
	unsigned char ascii_hex;
	unsigned int PreAuth;
}wASD_WLAN;	/*WID update WLAN information to ASD*/
struct mixwtpip{
	unsigned short addr_family;	
	unsigned short port;
	union {
		unsigned char ipv6_addr[MIXIPLEN];
		unsigned int ipv4_addr;
	} u;
#define m_v4addr u.ipv4_addr
#define m_v6addr u.ipv6_addr
};

typedef struct{
	unsigned int WtpID;	
	unsigned int   wtp_max_sta_num;
	unsigned int   wtp_triger_num;
	unsigned int   wtp_flow_triger;
	

	unsigned char 	WTPMAC[MAC_LEN];
	unsigned int 	WTPIP;
	char 			WTPSN[128];/*WTP Serial No*/
	
	
}wASD_WTP;	/*WID update WLAN information to ASD*/


typedef struct{
	unsigned char ifaceIndex;
	unsigned char ifaceIP[IPv4_LEN];
	unsigned char ifacePort;
	unsigned int iWTP_NUM;
}W_ifaces;	/*information of interface of WTP DataChannel*/

typedef struct{
	unsigned int WTPID;
	unsigned char WTPMAC[MAC_LEN];
	char *WTPModel;
	unsigned int WTPIP;
	unsigned char ACIfaceNum;
	W_ifaces ACIfaces[ACIfaces_MAX_NUM]; 
}wWSM_DataChannel;	/*WID update WTP information to WSM*/

typedef struct{	
	unsigned int	BSSIndex;
	unsigned int	Radio_G_ID;
	unsigned int rx_pkt_data;
	unsigned int tx_pkt_data;
	unsigned long long rx_data_bytes;	//xiaodawei add rx data bytes for ASD BSSData, 20110224
	unsigned long long tx_data_bytes;	//xiaodawei add tx data bytes for ASD BSSData, 20110224
}BSSData;	
typedef struct{	
	unsigned int	BSSIndex;
	unsigned char	Radio_L_ID;
	unsigned int	Radio_G_ID;
	unsigned char	WlanID;
	unsigned char	BSSID[MAC_LEN];
	unsigned int    bss_max_sta_num;
	unsigned int	nas_id_len;
	unsigned int	protect_type;
	wAW_IF_Type	bss_ifaces_type;
	wAW_IF_Type	wlan_ifaces_type;
	BSSData BssData[L_BSS_NUM];
	char	nas_id[128];
	unsigned int sta_static_arp_policy;
	char arp_ifname[IF_NAME_MAX];
	unsigned int vlanid;//zhanglei add for portal M_nasid
	unsigned int dyvlanid;    /* dynamic vlan */
	unsigned int f802_3;
	char nas_port_id[NAS_PORT_ID_LEN];			//mahz add 2011.5.26
	unsigned char accept_mac[50][MAC_LEN];
	unsigned char deny_mac[50][MAC_LEN];
	unsigned int accept_mac_num ;
	unsigned int deny_mac_num;
	unsigned int   macaddr_acl;
	unsigned int hotspot_id;
	unsigned char aliasSetFlag;
	char aliasESSID[ESSID_LENGTH];

	unsigned int four_way_handshake_on_ac:1;
	unsigned int reserved:31;	
}wAW_BSS;	/*WID update BSS information to ASD and WSM*/


typedef struct{	
	
	unsigned int	tx_bytes;		/*trans bytes,include tx and rx*/
	unsigned int	old_tx_bytes; 	/*old trans bytes,include tx and rx*/
	unsigned int 	trans_rates; 	/*trans rates,kbps*/
	/*add new members in future.*/

}FLOW_IE;  /*xm  09/02/06*/


typedef struct{	
	unsigned int    wtpid;
	FLOW_IE	        radio_flow[L_RADIO_NUM];

}wASD_RADIO_FLOW;	/*WID update RADIO flow information to ASD*/


/********************************
  * add for REQUIREMENTS-524
  * Internal  portal server IP	and mac 
  * external portal server	is 0
  *******************************/
typedef struct {
	
	unsigned int portal_ip;
	unsigned char portal_mac[MAC_LEN];

}PORTAL_INFO;

typedef enum
{
	AUTH_FRAME = 1 ,
	AC_KICK = 2,
	AP_KICK = 3,
	AP_STA_DEAUTH = 4,
	AP_STA_DISASSOC = 5,
	WDS_CHANGE =6,
	WTPD_REBOOT = 7,
	MAX_STA_LEAVE_REASON = 8
}STA_LEAVE_REASON;

#define MAX_STA_LEAVE_SUBREASON		(5)

enum
{
	AUTH_TO_ONE_WLAN = 1,
	AUTH_TO_WLAN = 2
};
enum
{
	SEND_DEAUTH = 1,
	SEND_DISASSOC = 2,
	SEND_BROADCAST_DEAUTH = 3,
	VAP_DISABLE_DELETE =4
};
enum{
	STA_SIGNAL_WEAK = 1,
	RETRANSMIT_TOO_MORE =2,
	IDLE_TIME_OUT =3
};
typedef struct {
	unsigned char StaState;
	unsigned char RoamingTag;
	unsigned char STAMAC[MAC_LEN];
	unsigned char RBSSID[MAC_LEN];
	unsigned int BSSIndex;
	unsigned int WTPID;
	unsigned int preBSSIndex;
    unsigned int  ip_addr;
	unsigned int count;			/*used in sta info report*/
	/*ADD IES*/
	unsigned char radioId;
	unsigned char wlanId;
	unsigned char mode;  /*11b-0x01,11a-0x02,11g-0x04,11n-0x08,*/
	unsigned char channel;
	unsigned char rssi;
	unsigned short tx_Rate;            //zhangshu modify 10-09-14
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned int rx_bytes;
	unsigned int tx_bytes;
	unsigned int traffic_limit;		/*ht add 091014*/
	unsigned int send_traffic_limit;
	unsigned int vlan_id;
	/* Inter-AC roaming member */
	roam_type ac_roam_tag;
	struct mixwtpip ac_ip;
	unsigned char length;/*mark ipv4 or ipv6*/
	unsigned int ipv4Address;
	char	arpifname[16];

	/* zhangshu append 2010-09-14 */
    unsigned long long rx_data_bytes;        //新加64bit，值还是上面的rx_bytes
    unsigned long long tx_data_bytes;        //新加64bit，值还是上面的tx_bytes
    unsigned int rx_data_frames;             //新加    （AP收到用户的数据帧数）
    unsigned int tx_data_frames;             //新加    （AP发给用户的数据帧数）
    unsigned int rx_frames;                  //新加   （AP收到用户的总帧数）
    unsigned int tx_frames;                  //新加   （AP发给用户的总帧数）
    unsigned int rx_frag_packets;            //新加   （AP收到用户的被分片的包数）
    unsigned int tx_frag_packets;            //新加   （AP发给用户的被分片的包数)
    unsigned short rx_Rate;                  //receive rate
    unsigned int vrrid;
	unsigned int  acct_input_gigawords; /* Acct-Input-Gigawords */
	unsigned int  acct_output_gigawords; /* Acct-Output-Gigawords */
	struct in6_addr ipv6Address;
	unsigned int delay;
	unsigned char sta_reason;
	unsigned short   sub_reason;
	unsigned int authorize_failed;
	unsigned char ptk_res;
	//PORTAL_INFO portal_info;
}aWSM_STA;	/*ASD update STA information to WSM*/


typedef struct {
	unsigned char Sta_authtype;
	unsigned char StaState;
	unsigned char STAMAC[MAC_LEN];
	unsigned int PORTID;
	unsigned int VLANID;	
	unsigned int Sta_ip;
	unsigned char sta_id[NAME_LEN+1];
	unsigned char sta_passwd[NAME_LEN+1];
}WIRED_STA;	/*ASD update STA information to NPD*/


typedef enum {
	Init_state = 0,
	ForceUnauthorized = 1,
	Auto = 2,
	ForceAuthorized = 3,	
	MacBased = 4,
	MAB = 5,
	PORTAL = 6
}PortTypes;

typedef struct {
	unsigned int PORTID;
	unsigned int vlan_num;
	npd_vbmp_t VLAN_map;
	PortTypes port_ctrl;
	unsigned short VLAN_ID;
	unsigned char STAMAC[MAC_LEN];
}WIRED_PORT;	/*NPD update port information to ASD*/

typedef struct {		
	unsigned int VLANID;
	unsigned int port_num;
	npd_pbmp_t PORT_map;
}WIRED_VLAN;	/*NPD update vlan information to ASD*/

typedef struct {		
	unsigned int WTPID;
	unsigned int BufLen;
	unsigned char CMDBuf[80];
}wASD_CMD;	
typedef enum{
	init_state = 0,
	unauth_state = 1,
	guest_vlan_state = 2,
	auth_fail_state = 3,
	auth_successs_state = 4,
	invalid_state = 5
}port_auth_state;

typedef enum{
	INVALID_OP = -1,
	WID_ADD = 0,
	WID_DEL = 1,
	WID_MODIFY = 2,
	STA_INFO = 3,
	RADIO_INFO = 4,  /* flow infomation  xm add */
	WTP_DENEY_STA=5,
	STA_COME=6,
	STA_LEAVE=7,
	VERIFY_INFO=8 ,
	VERIFY_FAIL_INFO=9,
	WTP_DE_DENEY_STA=10,
	BSS_INFO = 11,
	ASSOC_FAIL_INFO=12,
	JIANQUAN_FAIL_INFO=13,
	CHANNEL_CHANGE_INFO=14,
	WID_UPDATE = 15,
	WID_CONFLICT = 16,
	WID_ONE_UPDATE = 17,
	TRAFFIC_LIMIT = 18,
	WIDS_INFO = 19,
	WIDS_SET = 20,
	WAPI_INVALID_CERT = 21,
	WAPI_CHALLENGE_REPLAY = 22,
	WAPI_MIC_JUGGLE = 23,
	WAPI_LOW_SAFE_LEVEL = 24,
	WAPI_ADDR_REDIRECTION = 25,
	OPEN_ROAM = 26,
	VRRP_IF = 27,
	SET_GUEST_VLAN = 28,
	SET_AUTH_FAIL_VLAN = 29,
	SET_CONFIGED_ACCESS_VLAN = 30,
	MV_PORT_TO_CONFIGED_VLAN_AND_DENY = 31,
	MV_PORT_TO_GUEST_VLAN = 32,
	MV_PORT_TO_AUTH_FAIL_VLAN = 33,
	MV_PORT_TO_CONFIGED_VLAN_AND_ALLOW = 34,
	MV_PORT_TO_CONFIGED_VLAN_AND_REINIT=35,
	LINK_UP = 36,
	LINK_DOWN = 37,
	DOT1X_SYS_SET = 38,
	DOT1X_SYS_UNSET = 39,
	UPLOAD_PORT_STATUS = 40,
	NPD_ASD_SET_RESULT = 41,
	WIRED_JOIN=42,
	WIRED_LEAVE=43,
	SWITCHOVER_OP = 44,
	STA_ARP_ADD = 45,
	STA_ARP_DELETE = 46,
	STA_ARP_UPDATE = 47,
	MACAUTH_STA = 48,
	MODIFY_PVID = 49,
	WPA_PMK = 50,
	KEY_NEGOTI = 51,
	STA_LEAVE_REPORT = 52,
	GET_PORT_INFO = 53,
	STA_WAPI_INFO = 54,
	PORTALAUTH_STA = 55
}Operate;


typedef enum{
	CAPWAP_ADD,
	CAPWAP_DEL,
	CAPWAP_MOD,
	CAPWAP_ENABLE,
	CAPWAP_DISABLE,
	CAPWAP_BSS_CREATE,
	CAPWAP_BSS_DELETE,
	CAPWAP_BSS_ACTIVE,
	CAPWAP_BSS_INACTIVE,
	CAPWAP_WLAN_SET_PVID,
}CAPWAP_TO_NPD_ACTION;

typedef enum{
	WLAN_TYPE = 0,
	WTP_TYPE = 1,
	BSS_TYPE = 2,
	STA_TYPE = 3,
	SKEY_TYPE = 4,
	PORT_TYPE = 5,
	VLAN_TYPE = 6,
	STA_PKT_TYPE = 7,
	TRAP_TYPE = 8,
	BSS_PKT_TYPE = 9,
	CMD_TYPE = 10,
	BAK_TYPE = 11,
	BSS_TRAFFIC_LIMIT_TYPE = 12,
	WIDS_TYPE = 13,
	AP_REPORT_STA_TYPE = 14,
	SYSTEM_AUTH_CONTROL = 15,
	SWITCHOVER_TYPE = 16,
	UFDB_STA_TYPE = 17,
	PMK_TYPE = 18,
	PORTAL_TYPE = 19
}MsgType;


/*802.11 packet process methods on ap*/
typedef enum {
	CW_LOCAL_BRIDGING = 1, 			//local forwarding
	CW_802_DOT_3_TUNNEL = 2, 		//encapusulating with 802.3 payload format
	CW_802_DOT_11_TUNNEL = 4,		//encapusulating with 802.11 payload format
	CW_ALL_ENC = 7,					// ?
	CW_802_IPIP_TUNNEL = 8			//?
} CWframeTunnelMode;


typedef struct {
	unsigned int WTPID;
	unsigned int bss_cnt;
} BSS_pkt_header;

typedef struct {
	unsigned int WTPID;
	unsigned char radioid;
	unsigned char channel;
} WTP_channel_change;

typedef struct{
	unsigned int vrrid;
	unsigned int state;
	struct sockaddr_in ipaddr;
	unsigned int virip;
	unsigned int BSSIndex;
	unsigned char virname[NAME_LEN];
}wASD_BAK;
typedef struct{
	unsigned char able;
	unsigned int bssindex;
	unsigned int value;/*上行*/
	unsigned int average_value;/*上行*/
	unsigned int send_value;/*下行*/
	unsigned int send_average_value;/*下行*/
}traffic_limit_info;

typedef struct{
	unsigned int bssindex;
	unsigned char bssid[MAC_LEN]; //attack device mac
	unsigned char vapbssid[MAC_LEN]; // attack des
	unsigned char attacktype;
	unsigned char frametype;
	unsigned char channel;
	unsigned char rssi;
}wids_info;

typedef struct {
	unsigned char if_name[IF_NAME_MAX];
	struct mixwtpip	 ip;
	unsigned int op;
}vrrp_interface;

typedef struct{
	unsigned int lasttime;
	unsigned char able;
}wids_set;

typedef struct{     
	unsigned char RadioId;
	unsigned char WlanId;
	unsigned char mac[6];
	unsigned char ControlledPortStatus;
	unsigned char SelectedUnicastCipher[4];
	unsigned int WAPIVersion;
	unsigned int WPIReplayCounters;
	unsigned int WPIDecryptableErrors;
	unsigned int WPIMICErrors;
} WIDStaWapiInfo;

typedef struct{
	unsigned int WTPID;
	unsigned int sta_num;
	WIDStaWapiInfo StaWapiInfo[64];
} WIDStaWapiInfoList;

/* definition of  struct of table information communicated among WID,ASD or WSM*/


#define FREE_BSS_ALL_STA		(0xff)
#define MAX_STAINFO_NUM			(64)
#define MAX_STAINFO_MSGSIZE		(16)

typedef struct {
	MsgType Type;
	Operate Op;
	union{
		wASD_WLAN WLAN;
		wAW_BSS  BSS;
		wWSM_DataChannel  DataChannel;
		aWSM_STA  STA;
		wAW_StaKey KEY;
		wASD_PMK PMK;
		wASD_WTP WTP;
		wASD_RADIO_FLOW RadioFlow;
		wASD_CMD CMDOP;
		BSS_pkt_header bss_header;
		WTP_channel_change WTP_chchange;
		wASD_BAK BAK;
		traffic_limit_info traffic_limit;
		wids_set WIDS_set;
		wids_info WIDS_info;
		vrrp_interface vrrp_if;
		WIDStaWapiInfoList StaWapi;
		aWSM_STA  STAINFO[MAX_STAINFO_MSGSIZE];
	}u;  /*xm0723*/
}TableMsg;

typedef struct{
	npd_pbmp_t port_map;
	npd_pbmp_t link_map;
	npd_pbmp_t trunk_map;
	npd_pbmp_t trunk_link_map;
	unsigned char mac[MAC_LEN];
}WIRED_PORT_STATUS;

typedef struct {
	unsigned char	STAMAC[MAC_LEN];
	unsigned int	netif_index;
	unsigned int	link_state;
	unsigned int	flags;
}UFDB_STA;	

#ifdef HAVE_PORTAL
typedef struct {
	unsigned char enable;
	unsigned char srvID;
	unsigned int ipaddr;
	unsigned int port;
}PORTAL_SRV;
#endif

typedef struct {
	MsgType wType;
	Operate wOp;
	int result;
	union{
		WIRED_STA wSTA;
		WIRED_PORT wPORT;
		WIRED_VLAN wVLAN;
		WIRED_PORT_STATUS wSTATUS;
		UFDB_STA ufdb_sta;
#ifdef HAVE_PORTAL
		PORTAL_SRV wPortal;
#endif
	}u;
}WIRED_TableMsg;


typedef enum{
	IEEE802_11_MGMT = 10,
	IEEE802_11_EAP	= 11,
	IEEE802_3_EAP	= 12,
	IEEE_OTHER	= 64
}DataType;

/*definition of  struct of data information communicated between ASD and WSM*/
typedef struct {
	DataType Type;
	unsigned int WTPID;
	unsigned int Radio_G_ID;
	unsigned int BSSIndex;
	int DataLen;
	char Data[dot11_Max_Len];
}DataMsg;

typedef struct {
	MsgType type;
	unsigned int cnt;
}STAStatisticsMsg;

typedef struct {
	unsigned int WTPID;
	unsigned int Radio_G_ID;
	unsigned int BSSIndex;
	unsigned char STAMAC[MAC_LEN];
	unsigned long long rx_unicast;
	unsigned long long tx_unicast;
	unsigned long long rx_broadcast;
	unsigned long long tx_broadcast;
	unsigned long long rx_pkt_unicast;
	unsigned long long tx_pkt_unicast;
	unsigned long long rx_pkt_broadcast;
	unsigned long long tx_pkt_broadcast;
	unsigned long long retry;
	unsigned long long retry_pkt;
	unsigned long long err;
}STAStatistics;

typedef struct {
	unsigned int BSSIndex;
	unsigned int Radio_G_ID;
	unsigned long long rx_unicast;
	unsigned long long tx_unicast;
	unsigned long long rx_broadcast;
	unsigned long long tx_broadcast;
	unsigned long long rx_pkt_unicast;
	unsigned long long tx_pkt_unicast;
	unsigned long long rx_pkt_broadcast;
	unsigned long long tx_pkt_broadcast;
	unsigned long long retry;
	unsigned long long retry_pkt;
	unsigned long long err;
}BSSStatistics;

typedef struct {
	DataType wType;
	unsigned int PORTID;
	unsigned int VLANID;
	int DataLen;
	char Data[dot11_Max_Len];
}WIRED_DataMsg;

typedef struct {
	time_t  end_time;
	time_t	begin_time;
	unsigned int sta_num;
}CHN_TM;	/*xm add  09.5.13*/
struct wtp_access_info{
	struct wtp_access_info *next;
	struct wtp_access_info *hnext;
	unsigned int ip;
	unsigned char * WTPMAC;
	char * model;
	char * apcode;
	char * sn;
	char * version;
	char * codever;
	char * ifname;
};

typedef struct{
	unsigned int num;
	struct wtp_access_info * wtp_list;
	struct wtp_access_info * wtp_list_hash[256];	
}WID_ACCESS;

enum asd_dbus_result {
	ASD_DBUS_SUCCESS,                        /*  0  */
	ASD_DBUS_ERROR,
	ASD_DBUS_MALLOC_FAIL,
	ASD_DBUS_SET_ERROR,
	ASD_DBUS_GET_ARG_FAIL,
	ASD_DBUS_FAIL_TO_GET_REPLY,
	ASD_STA_NOT_EXIST,
 	ASD_WLAN_NOT_EXIST,
	ASD_WAPI_WLAN_NOT_EXIST,
	ASD_WTP_NOT_EXIST,
	ASD_SECURITY_NOT_EXIST,                  /*  11  */
	ASD_SECURITY_BE_USED,	
	ASD_SECURITY_ACCT_NOT_EXIST,
	ASD_SECURITY_ACCT_BE_USED,	
	ASD_SECURITY_AUTH_NOT_EXIST,
	ASD_SECURITY_AUTH_BE_USED,
	ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE,
	ASD_SECURITY_TYPE_WITHOUT_8021X,
	ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH,
	ASD_SECURITY_PROFILE_NOT_INTEGRITY,
	ASD_SECURITY_PROFILE_NOT_BIND_WLAN,         /*  20  */
	ASD_WLAN_HAS_BEEN_BINDED,
	ASD_SECURITY_KEY_NOT_PERMIT,
	ASD_SECURITY_KEY_LEN_NOT_PERMIT,
	ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX,
	ASD_SECURITY_KEY_HEX_FORMAT,
	ASD_SECURITY_KEY_HAS_BEEN_SET,
	ASD_SECURITY_WLAN_SHOULD_BE_DISABLE,
	ASD_SECURITY_MODIFY_ERROR,
	ASD_SECURITY_ID_INVALID,
	ASD_SECURITY_UNKNOWN_SECURITY_TYPE,          /*  30  */
	ASD_SECURITY_PARAMETER_OUT_OF_RANGE,
	ASD_SECURITY_TYPE_HAS_CHANGED,	
	ASD_SECURITY_GUEST_VLAN_NOT_SUPPORT,
	ASD_SECURITY_AUTH_FAIL_VLAN_NOT_SUPPORT,
	ASD_SECURITY_VLAN_INVALID,
	ASD_RADIUS_NO_CONFIG,
	ASD_RADIUS_EXCEED_ALLOW_NUM,
	ASD_RADIUS_EXIST,
	ASD_RADIUS_NOT_EXIST,
	ASD_RADIUS_NO_CONNECT,                     /*  40  */
	ASD_RADIUS_MODIFY_ERROR,
	ASD_BSS_NOT_EXIST,
	ASD_BSS_VALUE_INVALIDE,
	ASD_WLAN_VALUE_INVALIDE,
	ASD_UNKNOWN_ID,
	ASD_EXTENSIBLE_AUTH_NOT_SUPPORT,
	ASD_PRE_AUTH_NOT_SUPPORT,
	ASD_UPDATE_ERROR,	
	ASD_WTP_ID_LARGE_THAN_MAX,
	ASD_RADIO_ID_LARGE_THAN_MAX,            /*  50  */
	ASD_BSS_ID_LARGE_THAN_MAX,	
	ASD_WIDS_OPEN,	
	ASD_MAC_ADD_ALREADY,
	ASD_AC_GROUP_ID_USED,
	ASD_AC_GROUP_ID_NOT_EXIST,
	ASD_AC_GROUP_ESSID_NOT_EXIST,        
	ASD_AC_GROUP_MEMBER_EXIST,
	ASD_AC_GROUP_MEMBER_NOT_EXIST,
	ASD_ARP_GROUP_EXIST,
	ASD_NEED_REBOOT,        /*  60  */
	ASD_IFNAME_NOT_EXIST,
	ASD_IP_ADDR_INVALID,
	ASD_SYS_AUTH_CONTROL_NOT_ENABLE,
	ASD_GET_MIB_INFO_ERROR,
	ASD_CHANGE_AUTHENTICATION_MODE_ERROR,
	ASD_GET_AUTHENTICATION_MODE_ERROR,
	ASD_FINISH_ITER,
	ASD_DOT1X_NOT_ENABLE_ON_INTERFACE,
	ASD_STR_CONTAINS_INVALID_CHARACTER,
	ASD_STR_IS_TOO_LONG,        /*  70  */
	ASD_STR_IS_TOO_SHORT,		
	ASD_FAIL_TO_GET_INTERFACE_NAME,
	ASD_CHANNEL_ID_INVALID,
	ASD_MAC_FILTER_LIST_LARGE_THAN_MAX,
    ASD_MAC_FILTER_LIST_NOT_EXIST,
    ASD_MAC_FILTER_LIST_ADD_MAC_LARGE_THAN_MAX,
    ASD_MAC_DEL_NOT_EXIST,
	ASD_MAC_FILTER_IS_USED,
	ASD_MAC_FILTER_USE_ONLY_ONE,
	ASD_SECURITY_LOCAL_USER_FAIL,
	ASD_SECURITY_LOCAL_USER_ID_THAN_MAX,
	ASD_SECURITY_LOCAL_USER_EXIST,
	ASD_SECURITY_LOCAL_USER_NOEXIST
	
};

typedef struct
{
	DataType Type;
	unsigned int WTPID;
	unsigned int Radio_G_ID;
	unsigned int BSSIndex;
	unsigned int DataLen;
} DataMsgHead;


typedef struct capwap_conf_info_s{
	unsigned char	forward_mode;
	unsigned char	l3_en_dis;
	unsigned char	ifname[DEFAULT_LEN];
	unsigned int	tunnel_ip;
	unsigned int	preferred_ac;
}capwap_conf_info_t;

#endif
