#ifdef HAVE_AAA
#ifndef _MAN_SECURITY_H_
#define _MAN_SECURITY_H_


#define FORCE_UNAUTHORIZED	1
#define FORCE_AUTHORIZED	2
#define AUTO 				3
#define MAC_BASED			4

#define DEFAULT_VALUE	0

#define DEFAULT_SHARED_KEY	"default"
#define DEFAULT_AUTH_PORT	1812
#define DEFAULT_ACCT_PORT	1813

#define MAX_PASSWD_LEN 20
#define MIN_PASSWD_LEN 3

#define MAX_NAME_LEN 20
#define MIN_NAME_LEN 3
#define NULL_STRING ""

#define LOCAL_USER_NUM 256

typedef enum STRING_TYPE_S {
	NAME_TYPE_STR,
	PASSWD_TYPE_STR
}STRING_TYPE_T;

#define ENABLE 1


typedef struct{

}auth_method_item;


typedef struct{
	boolean sys_auth_control;
	unsigned int authentication_mode;
}dot1x_summary_type;

typedef struct{
	unsigned int security_type;
	unsigned int encryption_type;
	PortTypes port_dot1x_ctrl;
	boolean eap_reauth_ctrl;
	char name[PATH_LEN];	
	unsigned char mac_auth_bypass;
}asd_show_dot1x_type;

typedef struct{
	int eap_reauth_period;
	int guest_vlan_period;
	int quiet_period;
	int tx_period;
	int supp_timeout;
	int serv_timeout;
	char name[PATH_LEN];
}asd_dot1x_timer_type;


typedef struct {
	char name[PATH_LEN];
	unsigned char SecurityKey[PATH_LEN];
	unsigned short guest_vlan_id;
	unsigned short auth_fail_vlan_id;
	unsigned int RadiusID;
	unsigned int security_type;
	unsigned int encryption_type;
	unsigned int keyInputType;
	unsigned int extensible_auth;
	unsigned int pre_auth;
	PortTypes port_dot1x_ctrl;
	boolean eap_reauth_ctrl;
	unsigned int reauth_period;
	unsigned int guest_vlan_period;
	unsigned int quiet_Period;
	unsigned int tx_period;
	unsigned int supp_timeout;
	unsigned int serv_timeout;
	unsigned int vlan_state;
	unsigned int auth_state;
	unsigned int max_retrans;
	unsigned int max_users;
	unsigned char mac_auth_bypass;
}asd_dot1x_detail_type;


typedef struct{
	char name[PATH_LEN];
	int eapol_frames_recv;
	int eapol_frames_trans;
	int eapol_start_frames_recv;
	int eapol_logoff_frames_recv;
	int last_eapol_frame_version;
	int eap_resp_id_frame_recv;
	int eap_resp_frame_recv;
	int eap_req_id_frame_trans;
	int eap_req_frame_trans;
	int invalid_eapol_frame_recv;
	int eap_len_err_frames_recv;
	unsigned char last_eapol_src_mac[MAC_LEN];
	
}asd_dot1x_statistics_type;



typedef struct{
	char interface_name[PATH_LEN];
	int netif_index;
	short vlan_id;
	unsigned char mac_addr[MAC_LEN];
}asd_dot1x_client_type;


struct  dcli_local_user{
	char username[LOCAL_USERNAME_LEN+1];
	char password[LOCAL_USERPASSWORD_LEN+1];
};



extern char ASD_DBUS_BUSNAME[];
extern char ASD_DBUS_OBJPATH[];
extern char ASD_DBUS_INTERFACE[];
extern char ASD_DBUS_STA_OBJPATH[];
extern char ASD_DBUS_STA_INTERFACE[];
extern char ASD_DBUS_SECURITY_OBJPATH[];
extern char ASD_DBUS_SECURITY_INTERFACE[];	
extern char ASD_DBUS_AC_GROUP_OBJPATH[];
extern char ASD_DBUS_AC_GROUP_INTERFACE[];

int Check_IP_Format(char* str);
void ReInitDbusPath(int index, char * path, char * newpath);
int asd_get_if_array_index(unsigned int netif_index);
int dcli_str_check(const char *str, STRING_TYPE_T type, int max__len, int min_len);
int dcli_check_reply(DBusMessage *reply, DBusError err);
int dcli_asd_config_security_type(unsigned int security_ID, unsigned int type, unsigned int index);
int dcli_asd_config_encryption_type(unsigned int security_id,unsigned int type,unsigned int index);
int dcli_asd_config_security_profile_key(unsigned int security_id, unsigned char input_type_of_key, unsigned char *key, unsigned int index);
int dcli_asd_config_authentication_mode(int authentication_mode);
int dcli_asd_show_authentication_mode(int *authentication_mode);
int dcli_asd_show_dot1x_summary(dot1x_summary_type *dot1x_summary_info);
int dcli_asd_clear_dot1x_statistics(int *port_index, int p_count);
int dcli_asd_config_guest_vlan(unsigned int *netif_index, int num ,unsigned short vlan_id);
int dcli_asd_config_auth_fail_vlan(unsigned int *netif_index, int num, unsigned short vlan_id);
int dcli_asd_config_max_req(unsigned int netif_index,  int count);
int dcli_asd_config_max_user(unsigned int netif_index, int count);
int dcli_asd_config_dot1x_control(unsigned int security_id, unsigned char stat);
int dcli_asd_reauthenticate_interface(int *netif_index, int p_count);
int dcli_asd_config_reauthentication(unsigned int netif_index, int stat);
int dcli_asd_config_system_auth_control(int stat);
int dcli_asd_config_guest_vlan_period(unsigned int netif_index, int period);
int dcli_asd_config_reauth_period(unsigned int netif_index, int period);
int dcli_asd_config_quiet_period(unsigned int netif_index, int period);
int dcli_asd_config_tx_period(unsigned int netif_index, int period);
int dcli_asd_config_supp_timeout(unsigned int netif_index, int period);
int dcli_asd_config_server_timeout(unsigned int netif_index, int period);
int dcli_asd_show_dot1x(int netif_index, asd_show_dot1x_type *dot1x_show_info, int *next_netif_index);
int dcli_asd_show_dot1x_timer(int netif_index, asd_dot1x_timer_type *dot1x_show_timer_info, int *next_netif_index);
int dcli_asd_show_dot1x_detail(int netif_index, asd_dot1x_detail_type *dot1x_detail_info);
int dcli_asd_show_dot1x_statistics(int netif_index, asd_dot1x_statistics_type *statistics_info);
//int dcli_security_show_running_config();
//char *dcli_hansi_security_show_running_config(int index) ;


#endif
#endif
