/*
*this file define structs descripting framework and information of 
*WLAN/WTP/STA which creared,added,set by AC
*/



#ifndef _WID_DEFINE_H
#define _WID_DEFINE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include "wcpss/waw.h"
#include "ACMsgq.h"
#ifndef _WID_TYPE_DEF
#define _WID_TYPE_DEF
typedef unsigned char 		u_int8_t;
typedef unsigned short 		u_int16_t;
typedef unsigned int		u_int32_t;
#endif/*_WCPSS_TYPE_DEF*/
#define WID_ID_LEN	32
#define WID_MAXN_INDEX 64
#define WID_IP_LEN	4
#define WID_MAC_LEN	6
#define WID_DEFAULT_NUM	16
#define WPA_ELEMENT_ID 221
#define ETH_IF_NAME_LEN 16/*Added by weiay 20080710*/
#define ROGUE_AP_REPORT_INT 300
#define NAS_IDENTIFIER_NAME 128
#define WIFI_IOC_IF_CREATE  _IOWR(243, 1, struct interface_INFO)
#define WIFI_IOC_IF_DELETE  _IOWR(243, 2, struct interface_INFO) 
#define WIFI_IOC_IF_UPDATE  _IOWR(243, 6, struct interface_INFO)
#define WIFI_IOC_WSM_SWITCH  _IOWR(243, 7, unsigned int)

#define TEST_SWITCH_WAY  1 /*zhanglei change*/
#define WID_SYSTEM_CMD_LENTH 256
#define WTP_WEP_NUM 4
#define DEFAULT_SN_LENTH 20
#define D_LEN	128
#define SECTOR_NUM 4
#define TX_CHANIMASK_NUM 3

#define WIFI_IOC_MAGIC 244
#define WIFI_IOC_IP_ADD   _IOWR(WIFI_IOC_MAGIC, 4, ex_ip_info)
#define WIFI_IOC_IP_DEL   _IOWR(WIFI_IOC_MAGIC, 5, ex_ip_info)
#define		CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define		CW_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

#define _HAVE_WDS   0
#define _HAVE_WAPI  0

#define	MAX_RATE_CNT	14
#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif


#define DEFAULT_WIFI_QOS_NAME 	"default-wifi-qos"
#define DEFAULT_WIFI_QOS_ID		1

#define WID_SYSLOG_EMERG	0
#define WID_SYSLOG_ALERT	1
#define WID_SYSLOG_CRIT		2
#define WID_SYSLOG_ERR		3
#define WID_SYSLOG_WARNING	4
#define WID_SYSLOG_NOTICE	5
#define WID_SYSLOG_INFO		6
#define WID_SYSLOG_DEBUG	7
#define WID_SYSLOG_DEFAULT	0


#define AP_AUTO_DETECT_DHCP_GATEWAY		"/var/run/wcpss/ap_auto_detect_dhcp_gateway"
#define AP_INTELLIGENT_DHCP_INTERFACE	"vlan0001"

typedef struct ex_ip_INFO
{
 unsigned char  if_name[16];
 unsigned int dip;
 unsigned int sip;
 unsigned char wtpmac[6];
}ex_ip_info;

typedef enum {
	CW_FALSE_DCLI = 0,
	CW_TRUE_DCLI = 1
} CWBool_DCLI;
enum wid_debug{
	WID_DEFAULT = 0x1,
	WID_DBUS = 0x2,
	WID_WTPINFO = 0x4,
	WID_MB = 0x8,/*master and bak*/
	WID_ALL = 0xf
};

typedef enum {
	WID_SULKING = 0,
	WID_DISCOVERY = 1,
	WID_JOIN = 2,
	WID_CONFIGURE = 3,
	WID_DATA_CHECK = 4,
	WID_RUN = 5,
	WID_RESET = 6,
	WID_QUIT = 7
}WTPState;

typedef enum {
	WTP_INIT = 0,
	WTP_UNUSED = 1,
	WTP_NORMAL = 2,
	IF_NOINDEX = 3,
	IF_NOFLAGS = 4,
	IF_DOWN= 5,
	IF_NOADDR = 6,	
	WTP_TIMEOUT = 7
}WTPQUITREASON;


typedef enum {
	NONE_LIST	= 0,
	WHITE_LIST	= 2,
	BLACK_LIST	= 1
}LIST_T;


enum wid_radio_type {
	IEEE80211_11B = 0x1,
	IEEE80211_11A = 0x2,
	IEEE80211_11G = 0x4,
	IEEE80211_11N = 0x8,

};

struct radio_info_type_dcli{
	char radio_type;
	char radio_id;
	char bss_count;
	char txpower;
	char reserved1;
	char reserved2;
	unsigned short reserved3;	
};
struct ifi {
  char    ifi_name[ETH_IF_NAME_LEN];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  unsigned int nas_id_len;
  char nas_id[NAS_IDENTIFIER_NAME];
  char isipv6addr;
  struct ifi  *ifi_next;	/* next of these structures */
};

struct wlanid{
	unsigned char wlanid;
	struct wlanid *next; 
	
};
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Able;
	unsigned char subframe;  //zhangshu add, 2010-10-09
	unsigned int AmpduLimit;
	
}AmpduParameter;

/* zhangshu add for amsdu, 2010-10-09 */
typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Able;
	unsigned char subframe;  //zhangshu add, 2010-10-09
	unsigned int AmsduLimit;
	
}AmsduParameter;

typedef struct
{	
	unsigned char Type;
	unsigned char Op;
	unsigned char L_RadioID;
	unsigned char WlanID;
	unsigned char Mixed_Greenfield;
	
}MixedGreenfieldParameter;
struct tag_wtpid{
	unsigned int wtpid;
	struct tag_wtpid *next; 
	
};
 struct tag_wtpid_list{

 	 int count;	 
	 struct tag_wtpid * wtpidlist;
} ;
typedef struct tag_wtpid_list update_wtp_list;


struct Radio_Wlan_Pair{
	unsigned char radioid;
	unsigned char wlanid;
};

struct interface_INFO{
	char if_name[ETH_IF_NAME_LEN-1];
	unsigned char wlanID;
    int    BSSIndex;
	unsigned int vrid;      
    unsigned int acip;
    unsigned int apip;
    unsigned int protect_type;
    unsigned short acport;
    unsigned short apport;
    unsigned char bssid[MAC_LEN];   
    unsigned char apmac[MAC_LEN];
    unsigned char acmac[MAC_LEN];
    unsigned char ifname[ETH_IF_NAME_LEN];  
    unsigned char WLANID;
    unsigned char wsmswitch;
};
typedef struct interface_INFO IF_info;

/*added by weiay for rogue ap detection 2008/11/11*/
struct Neighbor_AP_ELE{
	unsigned char BSSID[MAC_LEN];
	unsigned short Rate; /*10 - 540 1080*/
	
	unsigned char Channel; /* 1 - 11	*/
	unsigned char RSSI;
	unsigned char NOISE;
	unsigned char BEACON_INT;
	
	unsigned char status; /*0 none 1 rogue ap 2 normal ap*/
	unsigned char opstatus; 
	unsigned short capabilityinfo;

	unsigned int wtpid;
	time_t fst_dtc_tm;
	time_t lst_dtc_tm;
	unsigned char encrp_type; /*/0 x 1 none 2 web 3shared*/
	unsigned char polcy; /* 0 no policy 1 have policy*/
	
	/*added more status*/
		
	char *ESSID;
	char *IEs_INFO;
	struct Neighbor_AP_ELE *next;
};
#define AP_ETH_IF_NUM		2
#define AP_WIFI_IF_NUM		2
#define AP_ATH_IF_NUM		16
#define WTP_TYPE_DEFAULT_LEN 32
struct ap_ath_info{
  unsigned char	radioid;
  unsigned char	wlanid;
  unsigned char	ath_updown_times;
} ;
typedef struct ap_ath_info wid_ap_ath_info;

struct ap_cpu_info{
  unsigned int value;
  struct ap_cpu_info *next;
};

struct wifi_info{
  unsigned char reportswitch;
  unsigned short reportinterval;
  int collect_time;
  unsigned int cpu;
  unsigned int cpu_collect_average;
  unsigned char temperature;
  unsigned int tx_mgmt;
  unsigned int rx_mgmt;
  unsigned int tx_packets;
  unsigned int tx_errors;
  unsigned int tx_retry;  
  unsigned int tx_bytes;
  unsigned int rx_packets;
  unsigned int rx_errors;
  unsigned int rx_retry;
  unsigned int rx_bytes;
  unsigned char ipmode;/*static--0,dhcp--1*/
  unsigned short memoryall;
  unsigned char memoryuse;
  unsigned short flashall;
  unsigned int flashempty;
  unsigned char wifi_snr;
  unsigned char eth_count;
  unsigned char eth_updown_time[AP_ETH_IF_NUM];
  unsigned char ath_count;
  unsigned char ath_updown_time[AP_ATH_IF_NUM];
  wid_ap_ath_info	ath_if_info[AP_ATH_IF_NUM];
  unsigned char wifi_count;
  unsigned char wifi_state[AP_WIFI_IF_NUM];/*0-not exist,1-up,2-down,3-error*/
  unsigned char cpu_trap_flag;
  unsigned char mem_trap_flag;
  unsigned char temp_trap_flag;
  unsigned char wifi_trap_flag[AP_WIFI_IF_NUM];
  unsigned int tx_unicast;
  unsigned int tx_broadcast;
  unsigned int tx_multicast;
  unsigned int tx_drop;
  unsigned int rx_unicast;
  unsigned int rx_broadcast;
  unsigned int rx_multicast;
  unsigned int rx_drop;
  unsigned int wpi_replay_error;
  unsigned int wpi_decryptable_error;
  unsigned int wpi_mic_error;	
  unsigned int disassoc_unnormal;	/*系统启动以来终端异常断开连接的总次数*/
  unsigned int rx_assoc_norate;	/*因终端不支持基本速率集要求的所有速率而关联失败的总次数*/
  unsigned int rx_assoc_capmismatch;	/*由不在802.11标准制定范围内的原因而关联失败的总次数*/
  unsigned int assoc_invaild;	/*未知原因而导致关联失败的总次数*/
  unsigned int reassoc_deny;	/*由于之前的关联无法识别与转移而导致重新关联失败的总次数*/
} ;
typedef struct wifi_info wid_wifi_info;

struct ap_cpu_mem_statistics{
  unsigned int cpu_value[10];
  unsigned int mem_value[10];
  unsigned int cpu_average;
  unsigned int cpu_times;
  unsigned int cpu_peak_value;
  unsigned int mem_average;
  unsigned int mem_times;
  unsigned int mem_peak_value;
  struct ap_cpu_info * ap_cpu_info_head;
  unsigned int ap_cpu_info_length;
  struct ap_cpu_info * ap_mem_info_head;
  unsigned int ap_mem_info_length;
  
  /*nl add for snr 2010-09-08*/
  unsigned int snr_times;		
  unsigned char snr_max_value;
  unsigned char snr_min_value;
  unsigned char snr_average;
  double snr_math_average;
  struct ap_snr_info * ap_snr_info_head;
  unsigned int ap_snr_info_length;
} ;
typedef struct ap_cpu_mem_statistics ap_cm_statistics;
typedef struct{
	unsigned char type;
	unsigned char ifindex;
	unsigned char state;/*0-not exist/1-up/2-down/3-error*/
	unsigned int eth_rate;/*10M,100M*/
	time_t state_time;
}if_state_time;
struct ap_if_state_time{
  unsigned char report_switch;
  unsigned char report_interval;
  unsigned char eth_num;
  unsigned char wifi_num;
  if_state_time eth[AP_ETH_IF_NUM];
  if_state_time wifi[AP_WIFI_IF_NUM];
} ;
typedef struct ap_if_state_time wid_ap_if_state_time;

struct tag_wids_set{
  unsigned char flooding;
  unsigned char sproof;
  unsigned char weakiv;
  unsigned char reserved; /*reserved*/
} ;
typedef struct tag_wids_set wid_wids_set;

struct tag_wids_statistics{
  unsigned int floodingcount;
  unsigned int sproofcount;
  unsigned int weakivcount;
} ;

typedef struct tag_wids_statistics wid_wids_statistics;


struct tag_wids_device_ele{
  unsigned char bssid[MAC_LEN]; /*attack device mac*/
  unsigned char vapbssid[MAC_LEN]; /* attack des*/
  unsigned char attacktype;
  unsigned char frametype;

  unsigned int attackcount;

  time_t fst_attack;
  time_t lst_attack;

  unsigned char channel;
  unsigned char rssi;
  
  struct tag_wids_device_ele *next;
} ;
 struct tag_wids_device_info{

 	 int count;	 
	 struct tag_wids_device_ele * wids_device_info;
} ;


typedef struct tag_wids_device_info wid_wids_device;


struct sample_rate_info {
 unsigned char  time;
 unsigned int	past_uplink_throughput;
 unsigned int	current_uplink_throughput;
 unsigned int	past_downlink_throughput;
 unsigned int	current_downlink_throughput;
 unsigned int	uplink_rate;
 unsigned int	downlink_rate;
};
typedef struct sample_rate_info wid_sample_rate_info;


typedef struct{
	int neighborapInfosCount;
	int DeviceInterference;
	struct Neighbor_AP_ELE *neighborapInfos;
} Neighbor_AP_INFOS;

typedef struct
{	
	unsigned char opstate; /*0disable 1 enable*/
	unsigned char flag;
	unsigned short reportinterval;
	unsigned char countermeasures_mode;//0 ap 1 adhoc 2 all
	unsigned char countermeasures_switch;//0 close default 1 open
	unsigned short reserved; //reseved
	
} APScanningSetting;

struct  wlan_stats_info_profile{
	unsigned char type;  /*0-ath, 1-eth, 2-wifi*/  
	unsigned char ifname[16];
	unsigned char radioId;
	unsigned char wlanId;
	unsigned char mac[6];
	unsigned int rx_packets;
	unsigned int tx_packets;
	unsigned int rx_errors;   
 	unsigned int tx_errors; 
	unsigned int rx_drop;   
 	unsigned int tx_drop; 
	unsigned long long rx_bytes;
	unsigned long long tx_bytes; 
	unsigned int rx_rate;
	unsigned int tx_rate;
	unsigned int ast_rx_crcerr;   /*pei add 0220*/
	unsigned int ast_rx_badcrypt;   /*pei add 0220*/
	unsigned int ast_rx_badmic;   /*pei add 0220*/
	unsigned int ast_rx_phyerr;   /*pei add 0220*/

	unsigned int rx_frame;
	unsigned int tx_frame;	
	unsigned int rx_error_frame;
	unsigned int tx_error_frame;	
	unsigned int rx_drop_frame;
	unsigned int tx_drop_frame;	
	
	unsigned int rx_band;
	unsigned int tx_band;	
	unsigned int rx_unicast;
	unsigned int tx_unicast;
	unsigned int rx_multicast;
	unsigned int tx_multicast;
	unsigned int rx_broadcast;
	unsigned int tx_broadcast;
	unsigned int rx_pkt_unicast;
	unsigned int tx_pkt_unicast;
	unsigned int rx_pkt_multicast;
	unsigned int tx_pkt_multicast;
	unsigned int rx_pkt_broadcast;
	unsigned int tx_pkt_broadcast;
	unsigned int rx_pkt_retry;
	unsigned int tx_pkt_retry;
	unsigned int rx_pkt_data;
	unsigned int tx_pkt_data;
	unsigned int rx_retry;
	unsigned int tx_retry;

	unsigned int rx_pkt_mgmt;   // packets received of management          zhangshu modify 2010-09-13
	unsigned int tx_pkt_mgmt;	  // packets transtmitted of management    zhangshu modify 2010-09-13
	unsigned long long rx_mgmt;
	unsigned long long tx_mgmt;	

	unsigned long long rx_sum_bytes;  // zhangshu add,20100913  total number sent by interface
    unsigned long long tx_sum_bytes;  // zhangshu add,20100913  total number received by interface

	unsigned int rx_pkt_control;//zhangshu add for 1.3v,20100913
	unsigned int tx_pkt_control;//zhangshu add for 1.3v,20100913	
	unsigned int rx_errors_frames;   //zhangshu add for error frames, 2010-09-26
	
	struct  wlan_stats_info_profile *next;
};

typedef struct wlan_stats_info_profile wlan_stats_info;

/*added end*/
typedef struct{     
	unsigned char radioId;
	unsigned char wlanId;
	unsigned char mac[6];
	unsigned char mode;  /*11b-0x01,11a-0x02,11g-0x04,11n-0x08,*/
	unsigned char channel;
	unsigned char rssi;
	//unsigned short nRate;
	unsigned short tx_Rate;
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned int rx_bytes;
	unsigned int tx_bytes;

	/* zhangshu add for 1.3v 2010-09-14 */
	unsigned long long rx_data_bytes;        //add 64bit for rx_bytes
    unsigned long long tx_data_bytes;        //add 64bit for tx_bytes
    unsigned int rx_data_frames;             //add data frames from user to ap
    unsigned int tx_data_frames;             //add data frames from ap to user
    unsigned int rx_frames;                  //add total frames from user to ap
    unsigned int tx_frames;                  //add total frames from ap to user
    unsigned int rx_frag_packets;            //add frag packets from user to ap
    unsigned int tx_frag_packets;            //add frag packets from ap to user
    unsigned short rx_Rate;                  //receive rate

	//weichao add
	unsigned char sta_reason;
	unsigned short sub_reason;

	unsigned char ptk_res;
} WIDStationInfo;

typedef struct {
	u_int32_t CMD;
	u_int32_t wlanCMD;
	u_int8_t  radiowlanid[L_RADIO_NUM][WLAN_NUM];
	u_int32_t setCMD;
	u_int32_t radioid[L_RADIO_NUM];
	int staCMD;
	char StaInf[8];
	int keyCMD;
	wAW_StaKey key;
}WID_CMD;

typedef struct {
	unsigned int STAOP;
	unsigned char STAMAC[WID_MAC_LEN];
	unsigned char WLANDomain;
}WID_STA;

typedef struct {
	char*	VlanName;
	u_int32_t	VID;
}WID_VLAN;

struct WID_TUNNEL_WLAN_VLAN {
  char		ifname[ETH_IF_NAME_LEN];	
  struct WID_TUNNEL_WLAN_VLAN  *ifnext;	
};

/*for mib*/
typedef struct {
	unsigned char	wlanid;
	unsigned char	l2_isolation_switch;
}WID_wtp_wlan_l2_isolation;
typedef struct {
	unsigned char	ifindex;
	unsigned char	snr_max;
	unsigned char	snr_min;
	unsigned char	snr_average;
	unsigned char	snr[10];
}WID_wtp_wifi_snr_stats;

typedef struct {
	WID_wtp_wlan_l2_isolation	wlan_l2isolation[L_BSS_NUM];
	unsigned char	dos_def_switch;
	unsigned char	igmp_snoop_switch;
}WID_mib_info;
typedef struct {
	unsigned int state;/*enable or not*/
	unsigned int tx_power;
}WID_oem_sector;

typedef struct {
	unsigned int state;/*enable or not*/
}WID_oem_tx_chainmask;

typedef struct {
	unsigned short supper_g_type;/*enable or not*/
	unsigned int supper_g_state;
}WID_oem_netgear_g;

typedef enum{
	DISABLE = 0,
	WDS_ANY = 1,
	WDS_SOME = 2
}WDS_STAT;

struct wds_bssid{
	unsigned char BSSID[MAC_LEN];
	struct wds_bssid *next;
};
struct wds_rbmac{
	unsigned char mac[MAC_LEN];
	unsigned char key[32];
	struct wds_rbmac *next;
};


typedef struct {
	u_int32_t	BSSIndex;/*Index of BSSID in AC*/
	u_int8_t	Radio_L_ID;
	u_int32_t	Radio_G_ID;
	u_int8_t	WlanID;
	u_int8_t	*BSSID;
	u_int8_t	State;
	u_int8_t	keyindex;
	u_int8_t	BSS_IF_NAME[ETH_IF_NAME_LEN];
	u_int8_t	BSS_IF_POLICY;
	u_int8_t	BSS_TUNNEL_POLICY;
	u_int8_t	forward_mode;
	u_int8_t	pvid;
	u_int8_t	ath_l2_isolation;
	u_int8_t	cwmmode;/*two states:0---20mode;1---20/40 mode*/
	u_int8_t	nas_id[NAS_IDENTIFIER_NAME];
	u_int32_t	nas_id_len;
	u_int32_t	bss_accessed_sta_num;
	u_int32_t	bss_max_allowed_sta_num;
	u_int32_t	vlanid;	/*bss vlanid,first*/
	u_int32_t	wlan_vlanid;	/*wlan vlanid,second*/
	u_int8_t	band_width;
	u_int8_t	traffic_limit_able;//disable-0//able-1
	u_int32_t	traffic_limit;/*bss traffic limit*/
	u_int32_t	average_rate;/*sta average*/
	u_int32_t	send_traffic_limit;/*bss send traffic limit*/
	u_int32_t	send_average_rate;/*sta send average*/
	u_int32_t 	upcount;
	u_int32_t 	downcount;
	BSSStatistics	BSS_pkt_info;
	struct acl_config *acl_conf;		/*ht add 08.12.15*/
	WDS_STAT	WDSStat;
	struct wds_bssid *wds_bss_list;
	u_int32_t	vMAC_STATE;
	u_int32_t	sta_static_arp_policy;
	u_int8_t	arp_ifname[ETH_IF_NAME_LEN];
}WID_BSS;


struct radio{
	u_int32_t	WTPID;
	u_int8_t	Radio_L_ID;/*Radio Local ID*/
	u_int32_t	Radio_G_ID;/*Radio Global ID*/
	u_int32_t	Radio_Type;/*a/b/g/n*/
	u_int32_t	Radio_Type_Bank;
	/*u_int16_t   Radio_Rate;//11m/bps or 54m/bps*/
	u_int32_t	Support_Rate_Count;
	struct Support_Rate_List *Radio_Rate;/*sz*/
	u_int8_t	Radio_Chan;/*Channel*/
	u_int16_t	Radio_TXP;/*TX power*/
	u_int16_t	Radio_TXPOF;/*TX  power offset*/
	u_int16_t   FragThreshold;/*Max fragmation size*/
	u_int16_t	BeaconPeriod;/*Beacon interval*/
	u_int8_t	IsShortPreamble;/*short preamble is 1*/
	u_int8_t	DTIMPeriod;
	u_int8_t	ShortRetry;
	u_int8_t	LongRetry;
	u_int16_t	rtsthreshold;
	u_int8_t	AdStat;/*Admin State 2(disable)/1(enable)*/
	u_int8_t	OpStat;/*Operate State 2(disable)/1(enable)*/
	u_int32_t	CMD;/*command tag*/
	WID_BSS * BSS[L_BSS_NUM];
	int			QOSID;
	int			QOSstate;
	int 		Radio_country_code; /*wcl add for OSDEVTDPB-31*/
	u_int8_t bandwidth;
	u_int8_t ishighpower;
	u_int8_t	diversity;//disable-0//able-1
	u_int8_t	txantenna;//auto-0//main-1//vice-2
	u_int16_t	channelchangetime;
	char *excommand;
	
	u_int8_t 	isBinddingWlan;
	u_int8_t	BindingWlanCount;
	u_int8_t	auto_channel;
	u_int8_t	auto_channel_cont;
	u_int8_t    channelsendtimes;
	u_int8_t	wifi_state;
	u_int8_t	txpowerautostate;
	u_int32_t 	upcount;
	u_int32_t 	downcount;
	u_int32_t 	rx_data_deadtime;
	/*11 n Parameters start*/
	u_int16_t guardinterval;
	u_int16_t mcs;
	u_int16_t cwmode;
	AmpduParameter	Ampdu;
	AmsduParameter	Amsdu;  //zhangshu add for a-msdu, 2010-10-09
	MixedGreenfieldParameter	MixedGreenfield;
	unsigned char  chainmask_num;  // 1-3  to indicate the number of radio chainmask  zhangshu add
	unsigned char  tx_chainmask_state_value;/*tx chainmask state.such as 0x0011*/
	unsigned char  rx_chainmask_state_value;/*rx chainmask state.such as 0x0011, zhangshu add 2010-10-09 */
	WID_oem_tx_chainmask	*tx_chainmask[TX_CHANIMASK_NUM];
	char channel_offset;
	
	/*11 n Parameters end*/
/*A8 start*/
	WID_oem_sector	*sector[SECTOR_NUM];
	unsigned short	sector_state_value;/*sector state.such as 0x0011.here only sector0 and sector1 is enable*/
	WID_oem_netgear_g supper_g;
	unsigned int REFlag;/*RadioExternFlag*/
	int distance; /*zhanglei add for A8*/
	unsigned int cipherType;/*0 disable  1 wep 2 aes*/
	char wepkey[32];
	struct wds_rbmac *rbmac_list;
	unsigned char inter_vap_able;
	unsigned char intra_vap_able;
	unsigned int keep_alive_period;
	unsigned int keep_alive_idle_time;
	unsigned char congestion_avoidance;
	struct wlanid	*Wlan_Id; /*binding wlan id*/
	unsigned short     txpowerstep;//zhaoruijia,20100917,add radio txpower step
	struct radio *next;
	unsigned char radio_disable_flag;  /*fengwenchao add 20110920 for radio disable config save flag*/
    int radio_countermeasures_flag;
};
typedef struct radio WID_WTP_RADIO;

/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
typedef struct terminalDisturbInfo{
    unsigned char reportswitch;   // 0-off, 1-on
	unsigned short reportpkt;
	unsigned short sta_trap_count;
} terminal_disturb_info;

/*fengwenchao add 20111117 for GM-3*/
struct heart_time_value_head
{
	unsigned int heart_time_value;
	struct heart_time_value_head *next;
};

struct heart_time
{	
	unsigned char heart_statistics_switch;
	unsigned int heart_statistics_collect;
	unsigned int heart_time_delay;
	unsigned int heart_transfer_pkt;
	unsigned int heart_lose_pkt;
	unsigned int heart_time_value_length;
	unsigned int heart_time_avarge;
	struct heart_time_value_head *heart_time_value_head;
};
/*fengwenchao add end*/
struct wtp_lan_vlan		/* ENR-23 */
{
	unsigned char state;		/* 0 : disable, 1 : enable */
	unsigned short vlanid;
};

struct wtp{
	u_int32_t	WTPID;
	char* 	WTPSN;/*WTP Serial No*/
	char*	WTPNAME;
	char*	WTPModel;/*model of wtp which determin the number of Radios and rates supported in WTP*/
	char*	APCode;/*   inner develop code*/
	u_int8_t	RadioCount;
	char*	WTPIP;
	u_int8_t*	WTPMAC;
	u_int8_t	WTPStat;/*wtp state (online(1) /offline(0))*/
	u_int32_t	WFR_Index;/*First Radio Global Index*/
	u_int8_t	CTR_ID;/*CAPWAP Control Tunnel ID*/
	u_int8_t	DAT_ID;/*CAPWAP Data Tunnel ID*/
	u_int8_t 	isused;
	u_int8_t 	quitreason;/*quit reason*/
	int 	wtp_login_mode; /*static 0;dynamic 1*/
	unsigned char apply_wlan_num;
	unsigned char apply_wlanid[WLAN_NUM];/*binding wlan id store apply wlan id*/
	char BindingIFName[ETH_IF_NAME_LEN];/*20080710*/
	unsigned char radio_num;
	WID_WTP_RADIO * WTP_Radio[L_RADIO_NUM];
	WID_CMD		*CMD;
	Neighbor_AP_INFOS *NeighborAPInfos;/*added by weiay 2008/11/11*/
    Neighbor_AP_INFOS *NeighborAPInfos2;
	Neighbor_AP_INFOS *rouge_ap_infos;
	unsigned int  wtp_allowed_max_sta_num;/*xm add 08/12/04*/
	unsigned int  wtp_triger_num;/*xm add 08/12/04*/
	wlan_stats_info apstatsinfo[TOTAL_AP_IF_NUM]; /*ath 4 wifi 2 eth 2 total num 8*/
	unsigned long long rx_bytes;/*total rx byte for this ap*/
	unsigned long long tx_bytes;/*total tx byte for this ap*/
	unsigned long long rx_bytes_before;/*tmp total rx byte for this ap*/
	unsigned long long tx_bytes_before;/*tmp total tx byte for this ap*/
	unsigned int  wtp_flow_triger;/*xm add 09/02/05*/
	unsigned int ap_ipadd;
	unsigned int ap_mask_new;
	unsigned int ap_gateway;
	unsigned int ap_dnsfirst;
	unsigned int ap_dnssecend;
	u_int8_t resetflag;
	u_int8_t ap_mask;
	u_int8_t	ap_sta_report_switch;
	u_int16_t	ap_sta_report_interval;
	char *sysver;
	char *ver;
	char *codever;/*used to recognize the version of oem production,so we can config the txantenna*/
	char *location;
	char *netid;
	unsigned char reportswitch;
	unsigned short reportinterval;
	int collect_time;
	wid_wifi_info	wifi_extension_info;
	wid_sample_rate_info	wid_sample_throughput;
	WID_mib_info	mib_info;
	ap_cm_statistics	apcminfo;
	wid_ap_if_state_time	apifinfo;
	wid_wids_statistics wids_statist;
	wid_wids_device *wids_device_list;
	time_t	*add_time;
	time_t	imagedata_time;
	time_t	config_update_time;
	time_t	manual_update_time;
	u_int8_t	updateStat; //0waitupdate//1update 2updatesuccess
	u_int8_t updatefailcount;//0noaccess//1overmaxcount 2pkterror
	u_int8_t	updatefailstate; //0waitupdate//1update 2updatesuccess
	u_int32_t	ElectrifyRegisterCircle;
	u_int32_t	wep_flag[WTP_WEP_NUM];/*which bss binding a wlan use wep*/
	WID_wtp_wifi_snr_stats wtp_wifi_snr_stats;
	struct msgqlist *ControlList;
	struct msgqlist *ControlWait;
	unsigned int rate;
	unsigned int old_bytes;
	char*	updateversion;
	char*	updatepath;
	char sendsysstart;
	char wtp_trap_switch;
	unsigned int wtp_cpu_use_threshold;
	unsigned int wtp_mem_use_threshold;
	unsigned int wtp_rogue_ap_threshold;
	unsigned int wtp_rogue_terminal_threshold;	
	u_int8_t	ap_sta_wapi_report_switch;
	u_int8_t	ap_sta_wapi_report_interval;
	int EchoTimer;
	struct wtp *next;
	int dhcp_snooping;
	int sta_ip_report;
	int neighbordeatimes;
	unsigned int elem_num;
	/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
	terminal_disturb_info ter_dis_info;
	
	struct heart_time heart_time;   /*fengwenchao add 20111117 for GM-3*/
	int rid;	/* rid from capwap head */
	struct wtp_lan_vlan lan_vlan;	/* ENR-23 */
	unsigned int four_way_handshake_on_ac:1;
	unsigned int reserved:31;
};
typedef struct wtp WID_WTP;

#define UPDATEIMGWTPID  0xffff

typedef struct updateImgACToAP_s
{
    pthread_mutex_t g_update_img_ac_to_ap_mutex;
    int tftpd_timer_is_running;
    int tftpd_is_running;
    int tftpd_request_timer_id;
}updateImgACToAP_t;

/*xm add for balence sta*/
struct bss_s{
	unsigned int bss_index;
	unsigned int arrival_num;

	/*unsigned int sta_num;
	unsigned int rd_flow;   xm add 09/02/06*/
	
	struct bss_s *next;
};

typedef struct bss_s bss_arrival_num;


struct log_node_s
{	
	unsigned char mac[6]; 
	
	bss_arrival_num *from_bss_list;
	unsigned int list_len;
	
	struct log_node_s *next;
	
};
typedef struct log_node_s log_node;

struct PMK_BSSInfo{
	unsigned int BSSIndex;
	struct PMK_BSSInfo *next;
};

struct PMK_STAINFO{
	struct PMK_STAINFO *next;
	struct PMK_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int PreBssIndex;
	unsigned char *BSSIndex;
	struct PMK_BSSInfo *bss;
	unsigned int BssNum;
	unsigned int idhi;
	unsigned int idlo;	
};

struct ROAMING_STAINFO{
	struct ROAMING_STAINFO *next;
	struct ROAMING_STAINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int BssIndex;
	unsigned int PreBssIndex;
	unsigned char BSSID[MAC_LEN];
	unsigned char PreBSSID[MAC_LEN];
};

struct PreAuth_BSSINFO{
	struct PreAuth_BSSINFO *next;
	struct PreAuth_BSSINFO *hnext;
	unsigned char addr[MAC_LEN];
	unsigned int BSSIndex;
};


#define WLAN_STA_HASH_SIZE				(256)	/* wlan->sta_hash bucket size */
#define WLAN_PREAUTH_BSSINFO_HASH_SIZE	(256)	/* wlan->bss_hash bucket size */
#define WLAN_ROAMING_STAINFO_HASH_SIZE	(256)	/* wlan->r_sta_hash bucket size */

struct wlan{
	u_int8_t		*WlanName;
	u_int8_t		WlanID;
	u_int8_t		*ESSID;
	u_int32_t		S_WTP_BSS_List[WTP_MAX_MAX_NUM][L_RADIO_NUM];/*Static WTP BSS List*/
	u_int8_t		Status;/*WLAN Status 1(default disable)/0(enable)*/
	u_int32_t	 	SecurityID;
	u_int8_t		HideESSid; /* 1  hide  0 not hide*/
	u_int8_t		wlan_if_policy;
	int  wpa_group_rekey;
	u_int8_t		forward_mode;
	u_int16_t		pvid;
	u_int8_t		WlanKey[DEFAULT_LEN];
	u_int32_t		KeyLen;
	u_int32_t		SecurityType;
	u_int32_t		EncryptionType;
	u_int32_t		SecurityIndex;
	u_int8_t		asic_hex;/* 0 asic; 1 hex*/
	u_int8_t		AsIp[DEFAULT_LEN];
	u_int32_t		IpLen;
	u_int8_t		ASCerPath[DEFAULT_LEN];
	u_int32_t		ASCerLen;
	u_int8_t		AECerPath[DEFAULT_LEN];
	u_int32_t		AECerLen;
	u_int32_t		vlanid;
	u_int8_t		wlan_1p_priority;/*0-7;0 means no priority*/
	struct WID_TUNNEL_WLAN_VLAN *tunnel_wlan_vlan;

	unsigned int ap_max_inactivity; 	   //weichao add
	u_int32_t 		wlan_accessed_sta_num;
 	u_int32_t 		wlan_max_allowed_sta_num;      /*xm add  08/12/01*/
	u_int32_t 		sta_list_len;
	log_node  		*sta_from_which_bss_list;
	u_int32_t 		balance_para;
	u_int32_t 		flow_balance_para;
	u_int8_t 		extern_balance;				/*xm0814*/
	u_int8_t 		balance_switch;            /*remember init them*/
	u_int8_t 		balance_method;            /*remember init them*/
	u_int8_t 		Roaming_Policy;			/*Roaming (1 enable /0 disable)*/
	u_int8_t 		isolation_policy;				/* (1 enable /0 disable)*/
	u_int8_t 		multicast_isolation_policy;	/* (1 enable /0 disable)*/
	u_int8_t 		sameportswitch;		/* (1 enable /0 disable)*/
	struct acl_config *acl_conf;				/*ht add 08.12.15*/

	u_int32_t		wlan_send_traffic_limit;/*nl add 20100318*/
	u_int32_t 		wlan_traffic_limit;	/*nl add 20100318*/
	struct PMK_STAINFO *sta_list;		
	struct PMK_STAINFO *sta_hash[256];	
	u_int32_t 		num_sta;	
	struct PreAuth_BSSINFO *bss_list;		
	struct PreAuth_BSSINFO *bss_hash[256];	
	u_int32_t		num_bss;
	u_int32_t		PreAuth;	
	struct ROAMING_STAINFO *r_sta_list;		
	struct ROAMING_STAINFO *r_sta_hash[256];	
	u_int32_t		r_num_sta;	
	WDS_STAT		WDSStat;	
	wid_wifi_info	wifi_extension_info;
	u_int8_t		AC_Roaming_Policy;
	u_int8_t		group_id;
	unsigned char filter_list_name[DEFAULT_LEN];
	int  filter_list_policy;
	struct wlan		*next;
	struct rsn_pmksa_cache *pmksa;	/* STA PMK CACHE */
};
typedef struct wlan WID_WLAN;

typedef struct{
	u_int8_t 		forward_mode;
	u_int8_t 		l3_en_dis;
	u_int8_t		l3_ifname[DEFAULT_LEN];
	u_int32_t		ip_addr;
	u_int8_t		indicator_mode;
	u_int32_t		ap_id;
	u_int32_t		station_threshold;
	u_int32_t		preferred_ac;
	u_int32_t		ap_auto_detect;
	u_int32_t		detect_ap_gateway;
	u_int32_t		detect_ap_netmask;
}capwap_config_t;

struct AUTOAPINFO
{
	unsigned char*		mac;
	unsigned char*		model;
	unsigned char*		realmodel;
	unsigned char*		sn;
	unsigned int		oemoption;
};
typedef struct AUTOAPINFO WIDAUTOAPINFO;




typedef enum {
	WID_WPA_CIPHER_NONE = 1,
	WID_WPA_CIPHER_WEP40 = 2,
	WID_WPA_CIPHER_WEP104 = 4,
	WID_WPA_CIPHER_WEP128 = 8,
	WID_WPA_CIPHER_TKIP = 16,
	WID_WPA_CIPHER_CCMP = 32,
	WID_WPA_CIPHER_AES_128_CMAC = 64,
	WID_WAPI_CIPHER_SMS4 = 128
}wid_cipher;

typedef enum {
	WID_WPA_KEY_MGMT_IEEE8021X = 1,
	WID_WPA_KEY_MGMT_PSK = 2,
	WID_WPA_KEY_MGMT_NONE = 4,
	WID_WPA_KEY_MGMT_IEEE8021X_NO_WPA = 8,
	WID_WPA_KEY_MGMT_WPA_NONE = 16,
	WID_WPA_KEY_MGMT_FT_IEEE8021X = 32,
	WID_WPA_KEY_MGMT_FT_PSK = 64,
	WID_WPA_KEY_MGMT_SHARED = 128,	
	WID_WPA2_KEY_MGMT_IEEE8021X = 256,
	WID_WPA2_KEY_MGMT_PSK = 512,
	WID_WPA2_KEY_MGMT_FT_IEEE8021X = 1024,
	WID_WPA2_KEY_MGMT_FT_PSK = 2048,
	WID_WAPI_KEY_MGMT_PSK = 4096, 
	WID_WAPI_KEY_MGMT_CER = 8192
}wid_wpa_key_mgmt;


typedef enum{
	NO_INTERFACE = 0,  				//<-->local mac & local forwarding
	WLAN_INTERFACE = 1, 			//<-->split mac & tunnel forwarding
	BSS_INTERFACE = 2   			//<-->split mac & tunnel forwarding
}wid_if_policy;


typedef struct{
	struct sockaddr_un addr;
	int 	addrlen;
}unixAddr;

struct white_mac{
	unsigned char elem_mac[6];
	struct white_mac *next;
};

typedef struct{
	int imaccount;
	struct white_mac *list_mac;
}white_mac_list;


/*xm add////////////////*/
struct oui_node{
	unsigned char oui[3];
	struct oui_node *next;
};


typedef struct{
	unsigned int list_len;
	struct oui_node *oui_list;
}OUI_LIST_S;


struct essid_node{
	char *essid;
	unsigned int len;
	struct essid_node *next;
};


typedef struct{
	unsigned int list_len;
	struct essid_node *essid_list;
}ESSID_LIST_S;


struct attack_mac_node{
	unsigned char mac[6];
	struct attack_mac_node *next;
};


typedef struct{
	unsigned int  list_len;
	struct attack_mac_node*attack_mac_list;
}ATTACK_MAC_LIST_S;


typedef struct{
	unsigned int WTPID;
	unsigned char flags;
	unsigned char channel;
	unsigned short txpower;	
	unsigned char H_channel_list[4];	
	unsigned char N_channel_list[4];
	unsigned int WTPID_List[3][2];
}WTP_RRM_INFO;

typedef struct{
	unsigned int wtpid;
	unsigned char neighbor_rssi[4];
	unsigned char txpower;
	unsigned char pre_txpower;
	unsigned char wtp_cnt;
}transmit_power_control;

extern WID_WLAN		*AC_WLAN[WLAN_NUM];
extern WID_WTP		**AC_WTP;
extern WID_WTP_RADIO	**AC_RADIO;
extern WID_BSS		**AC_BSS;
extern WID_WLAN		*ASD_WLAN[WLAN_NUM];
extern WID_WTP		**ASD_WTP;
extern WID_WTP_RADIO	**ASD_RADIO;
extern WID_BSS		**ASD_BSS;
extern WID_ACCESS	*AC_WTP_ACC;
extern Neighbor_AP_INFOS *gRogueAPInfos;/*added by weiay 2008/11/11*/
extern APScanningSetting gapscanset;
extern white_mac_list *pwhite_mac_list;
extern white_mac_list *pblack_mac_list;
extern int TableSock;
extern int TableSend;
extern int DataSend;
#if 1//ASD_MULTI_THREAD_MODE
extern int 		LoopSend;
extern unixAddr ASD_LOOP;
#endif

extern unixAddr toWSM;
extern unixAddr toWID;
extern unixAddr toCHILL;
extern unsigned char apstatistics;
extern wid_wids_set gwids;
extern unsigned char gwidsinterval;
extern unsigned char gprobethreshold;
extern unsigned char gotherthreshold;
extern unsigned int glasttimeinblack;
extern update_wtp_list *updatewtplist;
extern update_wtp_list *updatefailwtplist;
extern unsigned int checkwtpcount;
extern int gtrapflag;
extern int gtrap_ap_run_quit_trap_switch;
extern int gtrap_ap_cpu_trap_switch;
extern int gtrap_ap_mem_trap_switch;
extern int gtrap_rrm_change_trap_switch;
extern int gtrap_flash_write_fail_trap_switch;
extern int gtrap_channel_device_ap_switch;
extern int gtrap_channel_device_interference_switch;
extern int gtrap_channel_terminal_interference_switch;
extern int gtrap_rogue_ap_threshold_switch;
extern int gtrap_wireless_interface_down_switch;
extern int gtrap_channel_count_minor_switch;
extern int gtrap_channel_change_switch;
extern int gtrap_ap_run_quit_switch;
extern unsigned char updatemaxfailcount;
extern unsigned char aphotreboot;
extern unsigned char gwidspolicy;
extern wid_wids_device *wids_ignore_list;
extern unsigned char gessidfilterflag;
extern unsigned char gmacfilterflag;




extern OUI_LIST_S 			g_oui_list;
extern ESSID_LIST_S 		g_essid_list;
extern ATTACK_MAC_LIST_S 	g_attack_mac_list;  /*xm add */



/*country code area*/
enum country_code_result{
	COUNTRY_CHINA_CN,/* 0*/
	COUNTRY_EUROPE_EU,/* 1*/
	COUNTRY_USA_US,/* 2*/
	COUNTRY_JAPAN_JP,/* 3*/
	COUNTRY_FRANCE_FR,/* 4*/
	COUNTRY_SPAIN_ES,/* 5*/
	COUNTRY_CODE_SUCCESS,
	COUNTRY_CODE_NO_CHANGE,
	COUNTRY_CODE_ERROR_SMALL_LETTERS,
	COUNTRY_CODE_ERROR
};
enum channel_cwmode{
	CHANNEL_CWMODE_SUCCESS,/* 0*/
	CHANNEL_CWMODE_HT40,/* 1*/
	CHANNEL_CWMODE_HT20,
	CHANNEL_CWMODE_ERROR,
	CHANNEL_CROSS_THE_BORDER      /*fengwenchao add 20110323  信道越界*/
};

/*qos area*/
#define WID_QOS_ARITHMETIC_NAME_LEN 20


struct qos {
 u_int8_t	QueueDepth;
 u_int16_t	CWMin;
 u_int16_t	CWMax;
 u_int8_t	AIFS;
 u_int16_t	TXOPlimit;
 u_int8_t	Dot1PTag;
 u_int8_t	DSCPTag;
 u_int8_t	ACK;
 u_int8_t	mapstate;
 u_int8_t	wmm_map_dot1p;
 u_int8_t	dot1p_map_wmm_num;
 u_int8_t	dot1p_map_wmm[8];
 u_int8_t	qos_average_rate;
 u_int8_t	qos_max_degree;
 u_int8_t	qos_policy_pri;
 u_int8_t	qos_res_shove_pri;
 u_int8_t	qos_res_grab_pri;
 u_int8_t	qos_max_parallel;
 u_int8_t	qos_bandwidth;
 u_int8_t	qos_bandwidth_scale;
 u_int8_t	qos_use_wred;
 u_int8_t	qos_use_traffic_shaping;
 u_int8_t	qos_use_flow_eq_queue;
 u_int8_t	qos_flow_average_rate;
 u_int8_t	qos_flow_max_degree;
 u_int8_t	qos_flow_max_queuedepth;
};
typedef struct qos qos_profile;

struct wid_qos {
 unsigned int	QosID;
 char			*name;
 qos_profile * radio_qos[4];
 qos_profile * client_qos[4];
 unsigned char	qos_total_bandwidth;
 unsigned char	qos_res_scale;
 unsigned char	qos_share_bandwidth;
 unsigned char	qos_res_share_scale;
 char			qos_manage_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 char			qos_res_grab_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 char			qos_res_shove_arithmetic[WID_QOS_ARITHMETIC_NAME_LEN];
 unsigned char	qos_use_res_grab;
 unsigned char	qos_use_res_shove;
 struct wid_qos *next;
};
typedef struct wid_qos AC_QOS;

#define WID_QOS_CWMIN_DEFAULT	15
#define WID_QOS_CWMAX_DEFAULT	15
#define WID_QOS_AIFS_DEFAULT	15

extern AC_QOS *WID_QOS[QOS_NUM];

enum wid_qos_type {
	WID_BESTEFFORT=0,
	WID_BACKGTOUND,
	WID_VIDEO,
	WID_VOICE
};
typedef enum{
	unkonwn_type=0,
	averagerate_type,
	max_burstiness_type,
	manage_priority_type,
	shove_priority_type,
	grab_priority_type,
	max_parallel_type,
	bandwidth_type,
	bandwidth_percentage_type,
	flowqueuelenth_type,
	flowaveragerate_type,
	flowmaxburstiness_type
}flow_parameter_type;
typedef enum{
	qos_unkonwn_type=0,
	totalbandwidth_type,
	resourcescale_type,
	sharebandwidth_type,
	resourcesharescale_type
}qos_parameter_type;

struct model_info {
 char	*model;
 unsigned short	ap_eth_num;
 unsigned short	ap_wifi_num;
 unsigned short	ap_11a_antenna_gain;
 unsigned short	ap_11bg_antenna_gain;
 unsigned int	ap_if_mtu;
 unsigned int	ap_if_rate;
 char	*hw_version;
 char	*sw_name;
 char	*sw_version;
 char	*sw_supplier;
 char	*supplier;
};
typedef struct model_info model_infomation;
struct model_code_info {
 char	*code;
 unsigned char	cpu_type;
 unsigned char	mem_type;
 unsigned char	ap_eth_num;
 unsigned char	ap_wifi_num;
 unsigned char	ap_antenna_gain;
 unsigned char	support_mode[L_RADIO_NUM];
 unsigned int	ap_if_mtu;
 unsigned int	ap_if_rate;
 unsigned int	card_capacity;
 unsigned int	flash_capacity;
 char	*hw_version;
 char	*sw_name;
 char	*sw_version;
 char	*sw_supplier;
 char	*supplier;
};
typedef struct model_code_info wid_code_infomation;
typedef struct {
	unsigned char  state; 				/*0-disable, 1-num balance, 2-flow balance*/
	unsigned int   num_balance_para;	/*default : 1*/
	unsigned int   flow_balance_para;	/*default : 1 (Mbps)*/
}ac_balance_flag;
struct sample_info {
 unsigned char	monitor_time;
 unsigned char	sample_time;
 unsigned char	monitor_switch;
 unsigned char	sample_switch;
};
typedef struct sample_info wid_sample_info;
/*ethereal bridge area*/
struct WID_EBR_IF {
  char	*ifname;	
  struct WID_EBR_IF  *ifnext;	
};
typedef struct WID_EBR_IF EBR_IF_LIST;

struct wid_ebr {
 unsigned int	EBRID;
 char			*name;
 unsigned char	state;						/* (1 enable /0 disable)*/
 unsigned char	isolation_policy;			/* (1 enable /0 disable)*/
 unsigned char	multicast_isolation_policy;	/* (1 enable /0 disable)*/
 unsigned char	sameportswitch;		/* (1 enable /0 disable)*/
 EBR_IF_LIST	*iflist;
 EBR_IF_LIST	*uplinklist;
 struct wid_ebr *next;
};
typedef struct wid_ebr ETHEREAL_BRIDGE;

extern ETHEREAL_BRIDGE *WID_EBR[EBR_NUM];
/*auto ap area*/
typedef struct{
  unsigned char	wlannum;
  unsigned char	wlanid[L_BSS_NUM]; 
}wid_auto_ap_wlan_bind_t;

typedef struct {
	unsigned char	auto_ap_switch;
	unsigned char	save_switch;
	wid_auto_ap_wlan_bind_t	auto_ap_bind_conf;
	unsigned int	ap_auto_detect;
	unsigned int	ap_intelligent_dhcp_gw;
}wid_auto_ap_ctrl_t;

struct wid_ac_ip{
	char *ip;
	unsigned char priority;
	struct wid_ac_ip *next;
};

typedef struct{
	unsigned char GroupID;
	unsigned char *ifname;
	unsigned int ipnum;
	struct wid_ac_ip *ip_list;
}wid_ac_ip_group;

typedef struct{
	unsigned int ebr_num;
	ETHEREAL_BRIDGE **EBR;
}DCLI_EBR_API_GROUP;

struct ret_vlaue_str_pair {
	int value;
	unsigned char *string;
};

#endif/*_WID_DEFINE_H*/
