#ifndef __MAN_SYSLOG_H__
#define __MAN_SYSLOG_H__


#define SYSLOG_LOCAL_PORT "/etc/syslog-ng/local_port"
#define SET_SYSLOG_LOCAL_PORT "/usr/bin/set_syslog_local_port.sh"
#define SHOW_SYSLOG_LOCAL_PORT "/usr/bin/show_syslog_local_port.sh"
enum syslog_dest {
    dest_server=0,
    dest_telnet,
    dest_console,
};

void get_syslog_dest_state(char *server_state,char *console_state,char *telnet_state);
int get_syslog_server(int number,char *srv_addr,int *srv_port,char *srv_proto);
int get_syslog_module(int number,char *module_name);
int get_syslog_level(char *lvl);
int get_syslog_dest_level(char *srv_lvl,char *teln_lvl,char *cons_lvl);
int get_syslog_local_port();
int get_syslog_server(int number,char *srv_addr,int *srv_port,char *srv_proto);
int sync_add_syslog_server(char *addr,int port);
int sync_delete_syslog_server(char *addr);
int sync_local_udp_port(char *port);
int sync_syslog_dest_level(int dest,char *lvl);
int sync_syslog_server_state(char *state);

#endif
