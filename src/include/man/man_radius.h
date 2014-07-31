#ifdef HAVE_AAA
#ifndef _MAN_RADIUS_H_
#define _MAN_RADIUS_H_

#define RADIUS_ATTR_MAX 100

struct radius_serv_attr_def{
	unsigned int rad_attr_flag;
	unsigned char rad_attr_type;
	unsigned char rad_attr_subtype;
	unsigned char rad_attr_format;
	unsigned char rad_attr_mode;
	unsigned char rad_attr_usage;
	unsigned char rad_attr_value[32];
};

typedef struct{
	int radius_id;
	int acct_count;
	int auth_count;
	int retrans;
	int time_duration;
	char account_disable;
	char attribute_4_enable;
	char attribute_4_value[IP_LENGTH];	
	struct radius_serv_attr_def attrDef[RADIUS_ATTR_MAX];
}radius_info_type;

typedef struct{
	int radius_id;
	unsigned char server_name[PATH_LEN];
	unsigned char domain[PATH_LEN];
	unsigned char ip_addr[IP_LENGTH];
	unsigned char secret[PATH_LEN];
	int port;
	int type;
	boolean primary;
}radius_server_info_type;


typedef struct{
	int radius_id;
	int type;
	char ip_addr[IP_LENGTH];
	char server_name[PATH_LEN];
	boolean primary;
	char attribute_4_value[IP_LENGTH];
	int retrans;
	int time_duration;
	char account_disable;
	char attribute_4_enable;
}radius_serv_detail_info_type;


typedef struct{
	char ip_addr[IP_LENGTH];
	char server_name[PATH_LEN];
	unsigned int access_accepts;
	unsigned int access_challenges;
	unsigned int access_rejects;
	unsigned int bad_authenticators;
	unsigned int index;
	unsigned int malformed_responses;
	unsigned int packets_dropped;
	unsigned int requests;
	unsigned int responses;
	unsigned int retransmissions;
	unsigned int round_trip_time;
	unsigned int timeouts;
	unsigned int unknown_types;
}radius_statistics_type;


extern char ASD_DBUS_BUSNAME[];
extern char ASD_DBUS_OBJPATH[];
extern char ASD_DBUS_INTERFACE[];
extern char ASD_DBUS_STA_OBJPATH[];
extern char ASD_DBUS_STA_INTERFACE[];
extern char ASD_DBUS_SECURITY_OBJPATH[];
extern char ASD_DBUS_SECURITY_INTERFACE[];	
extern char ASD_DBUS_AC_GROUP_OBJPATH[];
extern char ASD_DBUS_AC_GROUP_INTERFACE[];

int dcli_radius_clear_radius_statistics(int radius_id);
int dcli_radius_conf_accounting_mode(int radius_id, char status);
int dcli_radius_config_attribute_4_ipaddr(int radius_id, int status, char *host_ip);
int dcli_radius_config_server_ip_addr(int radius_id, int port, char *ip, char *shared_secret, char *server_name, int type);
int dcli_radius_config_server_key_by_ip(int radius_id, char *ip, char *shared_secret, int type);
int dcli_radius_config_server_key_by_name(int radius_id, char *name, char *shared_secret, int type);
int dcli_radius_del_server_by_ip(int radius_id, char *ip, int type);
int dcli_radius_del_server_by_name(int radius_id, char *ip, int type);
int dcli_radius_config_primary_server_by_ip(int radius_id, char *ip_addr, int type);
int dcli_radius_config_primary_server_by_name(int radius_id, char *name, int type);
int dcli_radius_config_server_retransmit(int radius_id, int times);
int dcli_radius_config_server_timeout(int radius_id, int seconds);
int dcli_radius_show_radius(int radius_id, radius_info_type *radius_info);
int dcli_radius_show_server(radius_server_info_type *radius_server_info, radius_server_info_type *radius_server_ret_info);
int dcli_radius_show_statistics_by_ip(int radius_id, int type, char *ip_addr, radius_statistics_type *radius_statis_info);
int dcli_radius_show_statistics_by_name(int radius_id, int type, char *name, radius_statistics_type *radius_statis_info);
//int dcli_radius_show_running_config() ;


#endif
#endif
