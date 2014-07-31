#ifndef __MACRO_SYSLOG_H__
#define __MACRO_SYSLOG_H__


/* npd syslog buffer size  - shared by npd/nam/nbm/cpss */
#define NPD_SYSLOG_BUFFER_SIZE	(256)
/* npd/nam/nbm/cpss syslog line buffer size */
#define NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE		(256)
/* syslog line buffer size used in npd */
#define NPD_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in nam */
#define NAM_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in nbm */
#define NBM_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in asic driver */
#define ASIC_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* stp/rstp/mstp syslog buffer size */
#define STP_SYSLOG_BUFFER_SIZE	NPD_SYSLOG_BUFFER_SIZE

#endif
