#ifndef SYSLOG_CONF_H
#define SYSLOG_CONF_H

#define LOG_STR_SIZE 256
#define DEST_STR_SIZE 100
#define SRC_STR_SIZE 100
#define FILTER_STR_SIZE 50
#define IP_STR_SIZE 20
#define MAX_BUFFER_SIZE 256
#define DEST_NAME_SIZE 20
#define SYSLOG_RESTART_CMD "sudo /etc/init.d/syslog-ng restart"
#define CONFIG_TMP_FILE "/etc/syslog-ng/conf.tmp"
#define LEVEL_TMP_FILE "/etc/syslog-ng/level.tmp"
#define MODULE_TMP_FILE "/etc/syslog-ng/module.tmp"
#define STATE_TMP_FILE "/etc/syslog-ng/state.tmp"
#define SERVER_SRC_NAME "src_one"
#define GLOBAL_LEVEL_NAME "f_global"
#define DEFAULT_SERVER_FILTER "f_global"
#define SET_LEVEL_SCRIPT "sudo /usr/bin/set_syslog_level.sh"
#define DEL_SERVER_SCRIPT "sudo /usr/bin/del_syslog_server.sh"
#define ADD_SERVER_SCRIPT "sudo /usr/bin/add_syslog_server.sh"
#define SHOW_SERVER_SCRIPT "sudo /usr/bin/show_syslog_server.sh"
#define ADD_MODULE_SCRIPT "sudo /usr/bin/add_syslog_module.sh"
#define DEL_MODULE_SCRIPT "sudo /usr/bin/del_syslog_module.sh"
#define SHOW_LEVEL_SCRIPT "sudo /usr/bin/show_syslog_level.sh"
#define SHOW_MODULE_SCRIPT "sudo /usr/bin/show_syslog_module.sh"
#define ENABLE_SERVER_SCRIPT "sudo /usr/bin/enable_syslog_server.sh"
#define DISABLE_SERVER_SCRIPT "sudo /usr/bin/disable_syslog_server.sh"
#define ENABLE_DEST_SCRIPT "sudo /usr/bin/enable_syslog_dest.sh"
#define DISABLE_DEST_SCRIPT "sudo /usr/bin/disable_syslog_dest.sh"
#define SHOW_STATE_SCRIPT "sudo /usr/bin/show_syslog_dest_state.sh"
#endif

