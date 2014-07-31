#ifndef _UDLD_LOG_H_
#define _UDLD_LOG_H_

/** Logging levels for NPD daemon
 */

#define UDLD_OK              0
#define UDLD_ERR             (UDLD_OK + 1)

#define SYSLOG_LOG_EMERG    LOG_EMERG
#define SYSLOG_LOG_ALERT    LOG_ALERT
#define SYSLOG_LOG_CRIT     LOG_CRIT
#define SYSLOG_LOG_ERR      LOG_ERR
#define SYSLOG_LOG_WARNING  LOG_WARNING
#define SYSLOG_LOG_NOTICE   LOG_NOTICE
#define SYSLOG_LOG_INFO     LOG_INFO

#define UDLD_SYSLOG_BUFFER_SIZE 256


enum {
	SYSLOG_DBG_DEF = 0x0,	/* default value*/
	SYSLOG_DBG_DBG = 0x1,	/*normal */
	SYSLOG_DBG_WAR = 0x2,	/*warning*/
	SYSLOG_DBG_ERR = 0x4,	/* error*/
	SYSLOG_DBG_EVT = 0x8,	/* event*/
	SYSLOG_DBG_INTERNAL = 0x10, /*internal debug*/
	SYSLOG_DBG_ALL = 0xFF	/* all*/
};

extern unsigned int udld_log_level ;

void udld_syslog_dbg(char *format, ...);
void udld_syslog_warning(char *format, ...);
void udld_syslog_err(char *format, ...);
void udld_syslog_event(char *format, ...);
void udld_syslog_emit(char *buf);

int udld_log_level_set(unsigned int level);

void udld_syslog_init();
void udld_syslog_exit();

//#define UDLD_SYSLOG_DBG(x) printf x

#endif

