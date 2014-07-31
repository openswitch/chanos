#ifndef NTP_CONF_H
#define NTP_CONF_H


#define NTP_TMP_FILE "/var/run/ntp.tmp"
#define NTP_START_CMD "sudo /etc/init.d/ntp start"
#define NTP_STOP_CMD "sudo /etc/init.d/ntp stop"
#define NTP_RESTART_CMD "sudo /etc/init.d/ntp restart"
#define SET_LEVEL_SCRIPT "sudo /usr/bin/set_syslog_level.sh"
#define NTP_DEL_SERVER_SCRIPT "ntp_client.sh no"
#define NTP_ADD_SERVER_SCRIPT "ntp_client.sh add"
#define NTP_SHOW_SERVER_SCRIPT "sudo ntp_show_server.sh"
#define NTP_SERVER_MODE_SCRIPT "ntp_server.sh"
#define ENABLE_DEST_SCRIPT "sudo /usr/bin/enable_syslog_dest.sh"
#define DISABLE_DEST_SCRIPT "sudo /usr/bin/disable_syslog_dest.sh"
#define SHOW_STATE_SCRIPT "sudo /usr/bin/show_syslog_dest_state.sh"
#endif
