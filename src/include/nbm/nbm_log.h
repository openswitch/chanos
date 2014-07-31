#ifndef __NBM_LOG_H__
#define __NBM_LOG_H__

#include "npd/npd_log.h"
/** Logging levels for NPD daemon
 */

#define NBM_OK              0
#define NBM_ERR             (NBM_OK + 1)

#define SYSLOG_LOG_EMERG    LOG_EMERG
#define SYSLOG_LOG_ALERT    LOG_ALERT
#define SYSLOG_LOG_CRIT     LOG_CRIT
#define SYSLOG_LOG_ERR      LOG_ERR
#define SYSLOG_LOG_WARNING  LOG_WARNING
#define SYSLOG_LOG_NOTICE   LOG_NOTICE
#define SYSLOG_LOG_INFO     LOG_INFO

#define nbm_syslog_dbg npd_syslog_product_dbg 
#define nbm_syslog_err npd_syslog_product_err 
#define nbm_syslog_event npd_syslog_product_event
#define nbm_syslog_pkt_rx npd_syslog_product_pkt_rev
#define nbm_syslog_pkt_tx npd_syslog_product_pkt_sent
#define nbm_syslog_warning  npd_syslog_product_warning


#endif				/* LOGGER_H */
