
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_diag.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		NPD implement for ASIC diagnosis configuration.
*
* DATE:
*		03/24/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.18 $	
*******************************************************************************/
#ifdef HAVE_DIAG
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "nbm/nbm_cpld.h"

DBusMessage * npd_dbus_diagnosis_hw_watchdog_control(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply = NULL;
	unsigned int ret = DIAG_RETURN_CODE_SUCCESS;
	unsigned int enabled = SYSTEM_HARDWARE_WATCHDOG_ENABLE;
	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &enabled,
									DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args in hw watchdog control");
		if (dbus_error_is_set(&err)) {
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = DIAG_RETURN_CODE_ERROR;
	}

	ret = nbm_hardware_watchdog_control_set(enabled);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32, &ret,
							 DBUS_TYPE_INVALID);
	return reply;
}	

DBusMessage * npd_dbus_diagnosis_hw_watchdog_timeout_op(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	unsigned int ret = DIAG_RETURN_CODE_SUCCESS;
	unsigned int timeout = 0, opType = 0;
	unsigned int enabled = SYSTEM_HARDWARE_WATCHDOG_DISABLE;
	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &opType,
									DBUS_TYPE_UINT32, &timeout,
									DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args in hw watchdog timeout set");
		if (dbus_error_is_set(&err)) {
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = DIAG_RETURN_CODE_ERROR;
	}

	/* Get operation */
	if(SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_GET == opType) {
		ret = nbm_hardware_watchdog_control_get(&enabled);
		if(!ret) {
			timeout = 0;
			ret = nbm_hardware_watchdog_timeout_get(&timeout);
			if(!ret) {
				npd_syslog_dbg("hardware watchdog %s timeout %d\n", \
						(SYSTEM_HARDWARE_WATCHDOG_ENABLE == enabled) ? "enabled" : "disabled", timeout);;
			}
		}
		
	}
	/* Set operation */
	else if(SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET == opType) {
		ret = nbm_hardware_watchdog_timeout_set(timeout);
		if(!ret) {
			timeout = 0;
			nbm_hardware_watchdog_control_get(&enabled);
			nbm_hardware_watchdog_timeout_get(&timeout);
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32, &ret,
							 DBUS_TYPE_UINT32, &timeout,
							 DBUS_TYPE_UINT32, &enabled,
							 DBUS_TYPE_INVALID);
	return reply;
}	



DBusMessage * npd_dbus_diagnosis_env_monitor_control(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply = NULL;
	unsigned int ret = DIAG_RETURN_CODE_SUCCESS;
	unsigned int enabled = 0;
	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &enabled,
									DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args in hw watchdog control");
		if (dbus_error_is_set(&err)) {
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = DIAG_RETURN_CODE_ERROR;
	}

	ENVIROMENT_MONITOR_ENABLE = enabled;
	//ret = nbm_hardware_watchdog_control_set(enabled);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32, &ret,
							 DBUS_TYPE_INVALID);
	return reply;
}	

DBusMessage * npd_dbus_diagnosis_reset_board(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply = NULL;
	unsigned int ret = DIAG_RETURN_CODE_SUCCESS;
	unsigned int slotno = 0;
	unsigned int slot_index = 0;
	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &slotno,
									DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args in hw watchdog control");
		if (dbus_error_is_set(&err)) {
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = DIAG_RETURN_CODE_ERROR;
		goto error;
	}
	slot_index = slotno - 1;
	//ENVIROMENT_MONITOR_ENABLE = enabled;
	ret = nbm_board_reset(slot_index);
	if (ret != NPD_SUCCESS)
	{
		ret = DIAG_RETURN_CODE_ERROR;
	}
	
	reply = dbus_message_new_method_return(msg);

error:	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32, &ret,
							 DBUS_TYPE_INVALID);
	return reply;
}	


DBusMessage * npd_dbus_dbtable_show(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusMessageIter		iter;
	unsigned int ret = DIAG_RETURN_CODE_SUCCESS;
	char*			dbname = NULL;
	char*			showStr = NULL;

	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								    DBUS_TYPE_STRING,&dbname,
									DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args in dbtable show\n");
		if (dbus_error_is_set(&err))
		{
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = DIAG_RETURN_CODE_ERROR;
		goto error;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	ret = dbtable_table_show(dbname, &showStr, 1);
	if (ret != NPD_SUCCESS)
	{
		ret = DIAG_RETURN_CODE_ERROR;
		goto error;
	}
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING, 
									 &showStr);
	free(showStr);
    return reply;
error:	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32, &ret,
							 DBUS_TYPE_INVALID);
	return reply;
}	


#ifdef __cplusplus
}
#endif
#endif

