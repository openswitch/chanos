
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* man_udld.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		 uni-direction link detect protocol.
*
* DATE:
*		12/10/2011
*UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 0.01 $
*******************************************************************************/
#ifdef HAVE_UDLD
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <pthread.h>
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/npd_list.h"
#include "npd_database.h"
#include "db_usr_api.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include "udld/udld_main.h"
#include "udld/udld_timer.h"
#include "udld/udld_log.h"
#include "udld/udld_dbus.h"


extern DBusConnection *dcli_dbus_connection;

int man_udld_set_global_enable(unsigned char global_enable)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char global_enable_t = global_enable;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_GLOBAL_ENABLE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&global_enable_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_mode(unsigned char mode)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char mode_t = mode;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_GLOBAL_MODE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&mode_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}


int man_udld_set_recover_mode(unsigned char recover_mode)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char mode_t = recover_mode;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_RECOVER_MODE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&mode_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}


int man_udld_set_log_level(unsigned char log_level)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char mode_t = log_level;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_LOG_LEVEL);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&mode_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}


int man_udld_set_echo_timer(unsigned char echo_timer)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char echo_timer_t = echo_timer;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_ECHO_TIMER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&echo_timer_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_echo_timeout(unsigned char echo_timeout)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char echo_timeout_t = echo_timeout;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_ECHO_TIMEOUT);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&echo_timeout_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_enhanced_echo_timer(unsigned char echo_timer)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char echo_timer_t = echo_timer;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_ENHANCED_TIMER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&echo_timer_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_enhanced_echo_retry(unsigned char retry)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char retry_t = retry;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_ECHO_RETRY);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&retry_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_recover_timer(unsigned char recover_timer)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned char recover_timer_t = recover_timer;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_RECOVER_TIMER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&recover_timer_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_get_global_config(udld_global_ctrl *global_ctrl)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_GET_GLOBAL_CONFIG);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_BYTE,&global_ctrl->global_enable,
							DBUS_TYPE_BYTE,&global_ctrl->mode,
							DBUS_TYPE_BYTE,&global_ctrl->echo_timer,
							DBUS_TYPE_BYTE,&global_ctrl->echo_timeout,
							DBUS_TYPE_BYTE,&global_ctrl->enhanced_timer,
							DBUS_TYPE_BYTE,&global_ctrl->enhanced_retry,
							DBUS_TYPE_BYTE,&global_ctrl->recover_timer,
							DBUS_TYPE_BYTE,&global_ctrl->recover_mode,
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_set_netif_enable(unsigned int eth_g_index, unsigned short netif_enable)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int netif_index = eth_g_index;
	unsigned short netif_enable_t = netif_enable;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_SET_NETIF_ENABLE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_UINT16,&netif_enable_t,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}

int man_udld_get_netif_status(unsigned int eth_g_index, udld_netif_ctrl *netif_ctrl)
{
    DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int netif_index = eth_g_index;
	
	query = dbus_message_new_method_call(
			                UDLD_DBUS_BUSNAME,    \
			                UDLD_DBUS_OBJPATH,    \
			                UDLD_DBUS_INTERFACE,  \
			                UDLD_DBUS_METHOD_GET_NETIF_CONFIG);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return DBUS_RETURN_CODE_NULL_REPLY;
	}

	if(!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_UINT16,&netif_ctrl->enable,
							DBUS_TYPE_UINT16,&netif_ctrl->status,
							DBUS_TYPE_UINT32,&netif_ctrl->remote_netif_index,
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[0],
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[1],
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[2],
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[3],
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[4],
							DBUS_TYPE_BYTE,&netif_ctrl->remote_mac[5],
							DBUS_TYPE_INVALID))
	{
		printf("failed get args.\n");
		dbus_message_unref(reply);	
		ret = DBUS_RETURN_CODE_PARM_ERROR;
	}
	
	dbus_message_unref(reply);

	return ret;
}


#ifdef __cplusplus
}
#endif
#endif


