
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* erpp_main.c
*
*
* CREATOR:
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		Autelan uni-direction link detect protocol.
*
* DATE:
*		12/10/2011
*UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 0.01 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "sysdef/npd_sysdef.h"
#include "erpp/erpp_log.h"

unsigned int erpp_log_level = SYSLOG_DBG_DEF;
unsigned int erpp_log_close = 1;

/**********************************************************************************
 * erpp_log_set_debug_value
 * 
 *  DESCRIPTION:
 *	This function set up one erpp debug level
 *
 *  INPUT:
 * 	 	val_mask - debug level value stands for one level 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 1 - if debug level has already been set before.
 *	 0 - debug level setup successfully.
 *
 **********************************************************************************/
int erpp_log_set_debug_value
(
	unsigned int val_mask
)
{
	if(erpp_log_level & val_mask) {
		return 1;
	}
	
	erpp_log_level |= val_mask;
	return 0;
}

/**********************************************************************************
 *	erpp_log_set_no_debug_value
 * 
 *  DESCRIPTION:
 *	This function remove one erpp debug level
 *
 *  INPUT:
 * 	 	val_mask - debug level value stands for one level 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 1 - if debug level has not been set before.
 *	 0 - remove debug level successfully.
 *
 **********************************************************************************/
int erpp_log_set_no_debug_value
(
	unsigned int val_mask
)
{
	if(erpp_log_level & val_mask) {
		erpp_log_level &= ~val_mask;
		return 0;
	} else {
		return 1;
	}
}

/**********************************************************************************
 *	erpp_log_status_get
 * 
 *  DESCRIPTION:
 *	This function get erpp debug level
 *
 *  INPUT:
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 *	 current log settings
 *
 **********************************************************************************/
unsigned int erpp_log_status_get
(
	void
)
{
	return erpp_log_level;
}

/**********************************************************************************
 *	erpp_log_level_set
 * 
 *  DESCRIPTION:
 *	This function set up erpp debug level
 *
 *  INPUT:
 * 	 	level - debug level value
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 *	 0 - debug level setup successfully.
 *
 **********************************************************************************/
int erpp_log_level_set
(
	unsigned int level
)
{	
	erpp_log_level = level;
	return 0;
}

/**********************************************************************************
 *	nbm_syslog_emit
 * 
 *	output the log info to /var/log/daemon.log or startup log file NPD_SYSTEM_STARTUP_LOG_PATH
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void erpp_syslog_emit
(
	char *buf
)
{
	if(!buf){
		printf("erpp syslog emit param error\n");
		return;
	}

	/* write log*/
	syslog(LOG_DEBUG,buf);
	return; 
}


/**********************************************************************************
 *	erpp_syslog_dbg
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void erpp_syslog_dbg
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[ERPP_SYSLOG_BUFFER_SIZE] = {0};
	char *ident = "erpp";

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_DBG & erpp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!ident || !format) {
		return;
	}

	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf, ERPP_SYSLOG_BUFFER_SIZE-1, format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(erpp_log_close) {
		erpp_log_close = 0;
		openlog(ident, 0, LOG_DAEMON);
	}

	/*write log*/
	erpp_syslog_emit(buf);
	return;	
}

/**********************************************************************************
 *	erpp_syslog_err
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void erpp_syslog_err
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[ERPP_SYSLOG_BUFFER_SIZE] = {0};
	char *ident = "erpp";

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_ERR & erpp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!ident || !format) {
		return;
	}

	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf, ERPP_SYSLOG_BUFFER_SIZE-1, format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(erpp_log_close) {
		erpp_log_close = 0;
		openlog(ident, 0, LOG_DAEMON);
	}

	/*write log*/
	erpp_syslog_emit(buf);

	return;	
}

/**********************************************************************************
 *	erpp_syslog_warning
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void erpp_syslog_warning
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[ERPP_SYSLOG_BUFFER_SIZE] = {0};
	char *ident = "erpp";

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_WAR & erpp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!ident || !format) {
		return;
	}

	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf, ERPP_SYSLOG_BUFFER_SIZE-1, format,ptr);
	va_end(ptr);
	
	/* assure log file open only once globally*/
	if(erpp_log_close) {
		erpp_log_close = 0;
		openlog(ident, 0, LOG_DAEMON);
	}

	/*write log*/
	erpp_syslog_emit(buf);

	return;	
}

/**********************************************************************************
 *	erpp_syslog_event
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void erpp_syslog_event
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[ERPP_SYSLOG_BUFFER_SIZE] = {0};
	char *ident = "erpp";

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_EVT & erpp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!ident || !format) {
		return;
	}

	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf, ERPP_SYSLOG_BUFFER_SIZE-1, format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(erpp_log_close) {
		erpp_log_close = 0;
		openlog(ident, 0, LOG_DAEMON);
	}

	/*write log*/
	erpp_syslog_emit(buf);

	return;	
}

void erpp_syslog_init()
{
	char *ident = "erpp";
	if(erpp_log_close) {
		erpp_log_close = 0;
		openlog(ident, 0, LOG_DAEMON);
	}
}

void erpp_syslog_exit()
{
	if(erpp_log_close == 0) {
		erpp_log_close = 1;
		closelog();
	}
}


#ifdef __cplusplus
}
#endif

