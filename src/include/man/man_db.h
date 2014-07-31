#ifndef _MAN_DB_H
#define _MAN_DB_H
#include <stdint.h>
#include <unistd.h> 

#define SYNC_CONFIG(func,name,argc,extra) \
    int func (char *para) { \
    int ret=-1; \
    ret=db_update_via_para(name, argc, para, extra); \
    if(ret) { \
    printf("%%Config %s failed \r\n",name); \
    return -1;} \
    return 0; } 
    
#define SET_HOST_NAME "set_host_name"
#define SET_DNS_SERVER "dns_add"
#define NO_DNS_SERVER "dns_del"
#define RIP_BUFFER_SIZE "rip_buffer_size"
#define SET_IDLE_TIME "set_idle_time"
#define CLI_LOG "cli_log"
#define ERASE_MEMORY "del_memory"
#define SYS_DESC "set_sysdesc"
#define SYS_CONTACT "sys_contact"
#define SYS_TIME "sys_time"
#define OFFSET "offset"
#define SYS_LOCATION "sys_location"
#define NET_ELEMENT "net_element"
#define ADD_USER "user_add"
#define DEL_USER "user_del"
#define CHANGE_PWD "user_pwd"
#define CHANGE_ROLE "user_role"
#define SET_CONSOLE_PWD "set_console_pwd"
#define SERVER_LEVEL "server_level"
#define TELNET_LEVEL "telnet_level"
#define CONSOLE_LEVEL "console_level"
#define LOCAL_UDP_PORT "local_udp_port"
#define ADD_MODULE "add_module"
#define DEL_MODULE "del_module"
#define ADD_SERVER "add_server"
#define DEL_SERVER "del_server"
#define CONTROL_SERVER "control_server"
#define CONFIG_DEST "config_dest"
#define CONFIG_PAM "config_pam"
#define ADD_RADIUS "add_radius"
#define DEL_RADIUS "del_radius"
#define ADD_TACPLUS "add_tacplus"
#define DEL_TACPLUS "del_tacplus"
#define DNS_EXTRA "dns:"
#define SYSLOG_EXTRA "syslog:"
#define NTP_EXTRA "ntp:"
#define TACPLUS_EXTRA "tacplus:"
#define RADIUS_EXTRA "radius:"
#define PERMIT_EXTRA "permit:"
#define DENY_EXTRA "deny:"
#define CONFIG_TELNETD "config_telnetd"
#define CONFIG_SSHD "config_sshd"
#define DEL_TELNETD "del_telnetd"
#define DEL_SSHD "del_sshd"
#define PWD_AGING_DAY "pwd_aging_day"
#define PWD_RETRY "pwd_retry"


#define LIGHT_PROXY_MODE "li_proxy_mode"
#define LIGHT_BACKEND_TCP "li_back_tcp"
#define LIGHT_BACKEND_TIPC "li_back_tipc"
#define ADD_LIGHT_PROXY_IP "add_li_ip"
#define ADD_LIGHT_PROXY_TIPC "add_li_tipc"
#define DEL_LIGHT_PROXY_PATH "del_li_path"
#define MANUFACTURE_TEST "manufacture_test"

#define ADD_NTP_SERVER "ntp_server_add"
#define DEL_NTP_SERVER "ntp_server_del"
#define NTP_SERVER_MODE "ntp_server_mode"

/*************************************/
#define ADD_TELNETD_RULES "add_telnetd_rules"
#define DEL_TELNETD_RULES "del_telnetd_rules"
#define ADD_SSHD_RULES "add_sshd_rules"
#define DEL_SSHD_RULES "del_sshd_rules"
#define ADD_HTTPD_RULES "add_httpd_rules"
#define DEL_HTTPD_RULES "del_httpd_rules"
#define ADD_SNMPD_RULES "add_snmpd_rules"
#define DEL_SNMPD_RULES "del_snmpd_rules"

#define HTTPD_EXTRA "http:"
#define SNMP_EXTRA "snmp:"
#define TELNETD_EXTRA "in.telnetd:"
#define SSHD_EXTRA "sshd:"

/*************************************/

#define MAX_PARA_SIZE 256
#define CONFIG_NAME_SIZE 50
#define PARA_ITEM_SIZE 100
typedef int (*config_handler)(int argc, char *argv[],char *extra);
struct config_common_t 
{
    int  argc;
    char name[CONFIG_NAME_SIZE]; //this element must be unique
    char para[MAX_PARA_SIZE];
    char extra_para[PARA_ITEM_SIZE];
    config_handler handler;  
};
int db_update_by_name(char *name,int argc, char *argv[],char *extra);
int db_entry_delete_by_para(char *name,int argc, char *argv[], char *extra);
int db_update_via_para(char *name,int argc,char *para, char *extra);
int db_entry_delete_via_para(char *name,int argc, char *para, char *extra);

#endif
