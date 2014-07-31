
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*command_db_sync.h
*
*
* CREATOR:
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		header of sync db for quagga commands and dcli.
*
* DATE:
*		8/09/2010	
*UPDATE:
*08/17/2010              pangxf@autelan.com            Using DB. Bug shooting.
*09/15/2010              pangxf@autelan.com            Re-define the structs in following classes: 
*                                                                                     'Golable configuration', 'Local port configuration'
*                                                                                     'Remote port information', 'State machine'.
*  FILE REVISION NUMBER:
*  		$Revision: 1.06 $	
*******************************************************************************/
#ifndef COMMAND_DB_SYNC_H
#define COMMAND_DB_SYNC_H
#include "man_sysconf.h"
#include "man_syslog.h"
#include "man_user.h"
#include "man_db.h"

#define DB_PATH_SIZE 64
#define COMMAND_SERVICE 1588 // used for tansfer sync msg
#define COMMAND_LOCAL_SERVICE 1592 //used for local db service
#define CONFIG_INDEX_NAME "config"
#define CONFIG_TABLE_NAME "config"
#define ENTRY_NUM 200
#define SYNCD_SWITCH_OVER_INSTANCE 6

#define DECLARE_CONFIG_HANDLER(funcname) \
     int funcname(int argc, char *argv[], char *extra)

#define CMD_SIZE  256
#define SYS_TIME_SIZE 30
#define OFFSET_BUFFER_SIZE 100
#define DUMP_STR_SIZE 2048

#define IDLE_TIME_DEFAULT 10
#define IDLE_TIMEOUT_CONFIG_FILE "/var/run/idle_timeout.conf"
#define CLI_LOG_CONFIG_FILE "/var/run/cli_log.conf"

#define ADD_RADSERVER_SCRIPT "/usr/bin/add_radius.sh"
#define DEL_RADSERVER_SCRIPT "/usr/bin/del_radius.sh"
#define ADD_TACSERVER_SCRIPT "/usr/bin/add_tacplus.sh"
#define DEL_TACSERVER_SCRIPT "/usr/bin/del_tacplus.sh"
#define SHOW_PAM_SCRIPT "/usr/bin/show_pam.sh"
#define SHOW_RADIUS_SCRIPT "/usr/bin/show_radius.sh"
#define SHOW_TACPLUS_SCRIPT "/usr/bin/show_tacplus.sh"
#define RADIUS_ADDR_FILE "/etc/radius.tmp"
#define TACPLUS_CLIENT_FILE "/etc/tacplus.tmp"
#define ENABLE_RADIUS_SCRIPT "/usr/bin/enable_radius.sh"
#define ENABLE_TACPLUS_SCRIPT "/usr/bin/enable_tacplus.sh"
#define DISABLE_RADIUS_SCRIPT "/usr/bin/disable_radius.sh"
#define DISABLE_TACPLUS_SCRIPT "/usr/bin/disable_tacplus.sh"


#define SHOW_LIGHTTPD_PROXY_SCRIPT "sudo /usr/bin/show_lighttpd_proxy.sh"
#define ADD_LIGHTTPD_PROXY_SCRIPT "sudo /usr/bin/add_lighttpd_proxy.sh"
#define ADD_LIGHTTPD_PROXY_TIPC_SCRIPT "sudo /usr/bin/add_lighttpd_proxy_tipc.sh"
#define DEL_LIGHTTPD_PROXY_SCRIPT "sudo /usr/bin/del_lighttpd_proxy.sh"
#define RESTART_LIGHTTPD_CMD "sudo /etc/init.d/lighttpd restart"
#define LIGHTTPD_BACKEND_MODE "sudo /usr/bin/lighttpd_backend_mode.sh"
#define LIGHTTPD_PROXY_MODE "sudo /usr/bin/lighttpd_proxy_mode.sh"
#define PROXY_PATH_TMP_FILE "/etc/lighttpd/proxy-path.tmp"
#define PROXY_TYPE_FILE "/etc/lighttpd/proxy-type.conf"
#define BACKEND_MODE_TMP_FILE "/etc/lighttpd/backend-mode.tmp"
#define MANUFACTURE_TEST_SCRIPT "sudo /usr/bin/manu_test.sh"

#define LOGIN_FILTER_SCRIPT "/usr/bin/set_login_filter.sh"
#define DEL_LOGIN_FILTER_SCRIPT "/usr/bin/del_login_filter.sh"
#define HOSTS_ALLOW_FILE "/etc/hosts.allow"
#define HOSTS_DENY_FILE "/etc/hosts.deny"
#define TELNETD_EXTRA "in.telnetd:"
#define SSHD_EXTRA "sshd:"

#define HTTPD_EXTRA "http:"
#define SNMP_EXTRA "snmp:"


#ifdef HAVE_DIFF_OS
#define SET_TIME_SCRIPT "/usr/bin/set_time.sh"
#else
#define SET_TIME_SCRIPT "sudo /usr/bin/set_time.sh"
#endif

#ifndef __constant_htons
#define __constant_htons(x)  htons(x)
#endif

#define SYS_LOCATION_SIZE 128
#define NET_ELEMENT_SIZE 128


#endif
