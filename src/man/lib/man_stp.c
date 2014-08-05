
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_common_stp.c
*
*
*CREATOR:
*	chengjun@autelan.com
*
*DESCRIPTION:
*	APIs used in DCLI for stp process.
*
*DATE:
*	04/03/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.47 $		
*******************************************************************************/
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"

#include "command.h"

#include "dbus/npd/npd_dbus_def.h"
#include "man_str_parse.h"
#include "man_stp.h"

extern DBusConnection *dcli_dbus_connection;
int dcli_debug_out = 0;

char* stp_port_role[7] = {
			  	"DI",
				  "A",
				  "B",
				  "R",
				  "DE",
				  "M",
				  "NSTP"
	};
char* stp_port_role_full[7] = {
			  	"DISABLED",
				  "ALTERNATED",
				  "BACKUP",
				  "ROOT",
				  "DESIGNATED",
				  "MASTER",
				  "NSTP"
	};

char* stp_port_state[6] = {
		"DI",
		"DC",
		"L",
		"F",
		"N",
		"ERR-DISABLE"
	};
char* stp_port_state_full[6] = {
		"DISABLED",
		"DISCARDING",
		"LEARNING",
		"FORWARDING",
		"NSTP",
		"GUARD-DISABLED"
	};




/**********************************************************************************
 *  dcli_get_broad_product_id
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
int dcli_get_broad_product_id(int *id)
{
	DBusMessage *query, *reply;
	DBusError err; 
	unsigned int op_ret = 0;
	unsigned int product_id = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_GET_BROAD_TYPE);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_UINT32,&product_id,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	} 
	*id = product_id;
	dbus_message_unref(reply);
    return op_ret;
}


/**********************************************************************************
 *  dcli_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
/*Get the global state and stp mode of the bridge*/
int dcli_get_brg_g_state(int *stpmode)
{
	DBusMessage *query, *reply;
	DBusError err; 
	unsigned int op_ret = 0;
	int mode = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_GET_PROTOCOL_STATE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_UINT32,&mode,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	} 
	*stpmode = mode;
	return op_ret;
}

/**********************************************************************************
 *  dcli_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
int dcli_set_stp_running_mode
(
	DCLI_STP_RUNNING_MODE mode
)
{
	
}

int dcli_enable_g_stp(
    DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable
)
{
    int ret;
    switch(mode)
    {
        case DCLI_STP_M:
    		ret = dcli_enable_g_stp_to_protocol(DCLI_STP_M,isEnable);
    		/*set stp version*/
    		/*vty_out(vty,"stp dcli to protocol %s\n",isEnable ? "enable" : "disable");*/
    		if(isEnable){
    			ret |= dcli_set_bridge_force_version(STP_FORCE_VERS);
    			/*vty_out(vty,"stp dcli force isenable  finish \n");*/
    		}
            break;
        case DCLI_MST_M:
    		ret = dcli_enable_g_stp_to_protocol(DCLI_MST_M,isEnable);
    		/*set stp version*/
    		/*vty_out(vty,"stp dcli to protocol %s\n",isEnable ? "enable" : "disable");*/
    		if(isEnable){
    			ret |= dcli_set_bridge_force_version(MST_FORCE_VERS);
    			/*vty_out(vty,"stp dcli force isenable  finish \n");*/
    		}
            break;
        default:
            return DCLI_STP_FATAL;
    }
    return ret;
}


/**********************************************************************************
 *  dcli_enable_g_stp_to_protocol
 *
 *	DESCRIPTION:
 * 		enable or disable stp protocol
 *
 *	INPUT:
 *		type 			- stop/mstp
 *		isEnable	- en/disable
 *	
 * RETURN:
 *		CMD_WARNING   
 *		CMD_SUCCESS   
 *
 **********************************************************************************/

int dcli_enable_g_stp_to_protocol
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0,state = 0;
	int stp_mode = 0;
	
	state = dcli_get_brg_g_state(&stp_mode);
	if(1 == isEnable) 
	{
		if(1 == state){/*has already enabled ,return*/
           return STP_RETURN_CODE_STP_HAVE_ENABLED;
		}
	}
    else 
    {
        if(0 == state)
        {
           return STP_RETURN_CODE_STP_NOT_ENABLED;
        }
    }

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_STPM_ENABLE);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&isEnable,
									DBUS_TYPE_UINT32,&mode,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,180*1000, &err);
    
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}


/**********************************************************************************
 *  dcli_enable_g_stp_to_npd
 *
 *	DESCRIPTION:
 * 		enable or disable stp to npd
 *
 *	INPUT:
 *		NONE
 *	
 * RETURN:
 *		CMD_WARNING   
 *		CMD_SUCCESS   
 *
 **********************************************************************************/
int dcli_enable_g_stp_to_npd(unsigned int enable)
{
	DBusMessage *query, *reply;
	DBusError err; 
	unsigned int op_ret;



	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP);

	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&enable,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,180*1000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_enable_netif(unsigned int netif_index, int isEnable)
{
	int lk = 0;
	int ret = 0;
	unsigned int mode = DCLI_STP_M;
    int state, stp_mode;
	unsigned int speed;
	
    lk = 0;
    speed = 1000;

	state = dcli_get_brg_g_state(&stp_mode);
    if(state == 0)
    {
        return STP_RETURN_CODE_STP_NOT_ENABLED;
    }

    ret = dcli_enable_stp_on_one_port_to_protocol(netif_index,isEnable,lk,speed);
	if(STP_RETURN_CODE_ERR_NONE == ret) 
    {		
		ret = dcli_enable_stp_on_one_port_to_npd(mode,netif_index,isEnable);
	}
	return ret;
}

int dcli_stp_config_priority
(	
	unsigned int prio
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	unsigned int mstid = 0;
	
	if(prio < MIN_BR_PRIO){
		return  STP_RETURN_CODE_Small_Bridge_Priority;
	}
	else if (prio > MAX_BR_PRIO)
	{
		return  STP_RETURN_CODE_Large_Bridge_Priority;
	}
	else if(0 != (prio%4096))
	{
		return STP_RETURN_CODE_Priority_Format_Err;
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&prio,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_max_age(unsigned int maxAgeValue)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	
	query = dbus_message_new_method_call( 				 \
										RSTP_DBUS_NAME,		 \
										RSTP_DBUS_OBJPATH,	 \
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_MAXAGE);

    dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&maxAgeValue,
								DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);


	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_hello_time(unsigned int helloTimeValue)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  op_ret;
		
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_HELTIME);

	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&helloTimeValue,
								DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_forward_delay(unsigned int forwardDelayValue)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  op_ret;
	
	query = dbus_message_new_method_call(					\
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_CONFIG_FORDELAY);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&forwardDelayValue,							
							DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_config_stp_default()
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	unsigned int mstid = 0;
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32&mstid,						
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_netif_rootguard(
     unsigned int netif_index,
     int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int mstid = 0;
	unsigned int op_ret = 0;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_ROOTGUARD);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&netif_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;    
}

int dcli_stp_config_netif_bpduguard(
     unsigned int netif_index,
     int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int mstid = 0;
	unsigned int op_ret = 0;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_BPDUGUARD);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&netif_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;    
}
int dcli_stp_config_netif_pathcost(
     unsigned int netif_index,
     int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int mstid = 0;
	unsigned int op_ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&netif_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;     
}

int dcli_stp_config_netif_pri(
     unsigned int netif_index,
     int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int mstid = 0;
	unsigned int op_ret;

	
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORTPRIO);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&mstid,
						DBUS_TYPE_UINT32,&netif_index,
						DBUS_TYPE_UINT32,&value,
						DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;  
}

int dcli_stp_config_netif_nonstp(unsigned int port_index, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int mstid = 0;
	unsigned int op_ret;
	
	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NONSTP);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret; 
}

int dcli_stp_config_netif_p2p_mode(unsigned int port_index, unsigned int p2p_mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int mstid = 0;
	unsigned int op_ret;

	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(						\
									RSTP_DBUS_NAME,					\
									RSTP_DBUS_OBJPATH,				\
									RSTP_DBUS_INTERFACE,			\
									RSTP_DBUS_METHOD_CONFIG_P2P);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&p2p_mode,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret; 
}

int dcli_stp_config_netif_edge(unsigned int port_index, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int mstid = 0;
	unsigned int op_ret;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_EDGE);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret; 
}
int dcli_stp_config_netif_mcheck(unsigned int port_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	unsigned int mstid = 0;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(					\
									RSTP_DBUS_NAME,				\
									RSTP_DBUS_OBJPATH,			\
									RSTP_DBUS_INTERFACE,		\
									RSTP_DBUS_METHOD_CONFIG_MCHECK);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret; 
}

int dcli_stp_config_netif_default(unsigned int port_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	unsigned int mstid = 0;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 							 		
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_region_name(char *name)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	unsigned int value;	
	value = strlen(name);
	if(value > 31)
	{
		return  STP_RETURN_CODE_REGION_NAME_ERR;
	}

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_REG_NAME);

    dbus_message_append_args(query,
									DBUS_TYPE_STRING,&name,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_revision(unsigned short value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;

	if((value < MIN_BR_REVISION) || (value > MAX_BR_REVISION ))
	{
		return  CMD_WARNING;
	}
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_BR_REVISION);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&value,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_mst_priority(unsigned int mstid, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;

	if (value < MIN_BR_PRIO || value > MAX_BR_PRIO) 
	{
		return STP_RETURN_CODE_BRIDGE_PRI_OUT_OF_RANGE;
	}
	else if (mstid < MIN_MST_ID || mstid > MAX_MST_ID)
	{
		return STP_RETURN_CODE_MSTID_OUT_OF_RANGE;
	}
	else if (0 != (value%4096))
	{
		return STP_RETURN_CODE_BRIDGE_PRI_FORMAT_ERR;
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_mst_root_guard(unsigned int netif_index, unsigned int mstid, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PORT_ROOTGUARD);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&netif_index,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_mst_bpdu_guard(unsigned int netif_index, unsigned int mstid, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PORT_BPDUGUARD);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&netif_index,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_mst_max_hops(unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	
	if(value < MIN_REMAINING_HOPS)
	{
		return STP_RETURN_CODE_Small_Max_Hops;
	}
	else if (value > MAX_REMAINING_HOPS)
	{
		return STP_RETURN_CODE_Large_Max_Hops;
	}
	
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									MSTP_DBUS_METHOD_CONFIG_MAXHOPS);

	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_mst_netif_path_cost(unsigned int mstid, unsigned int port_index, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
		
	if (!((value > 0)&&(value % 20 ==0)))
	{
		return STP_RETURN_CODE_PORT_PATH_COST_FORMAT_ERR;
	}

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 	
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_mst_netif_priority(unsigned int mstid, unsigned int port_index, unsigned int value)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;

	if(value < MIN_PORT_PRIO || value > MAX_PORT_PRIO )
	{
		return STP_RETURN_CODE_MST_PORT_PRIORITY_OUT_OF_RANGE;
	}
	else if(0 != (value%16))
	{
		return STP_RETURN_CODE_MST_PORT_PRIORITY_FORMAT_ERR;
	}

	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORTPRIO);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&mstid,
						DBUS_TYPE_UINT32,&port_index,
						DBUS_TYPE_UINT32,&value,
						DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_mst_default(unsigned int mstid)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &mstid,
							DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}
int dcli_stp_config_mst_netif_default(unsigned int mstid, unsigned int port_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,				 		
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_mst_vlan_to_instance(unsigned short vid, unsigned int mstid)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									MSTP_DBUS_METHOD_CFG_VLAN_ON_MST);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&vid,
									DBUS_TYPE_UINT32,&mstid,							 									 		
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
   	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret; 
}

int dcli_stp_config_digest_snp(unsigned int port_index, unsigned int isEnable)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	unsigned int op_ret = 0;
	
	
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,
									RSTP_DBUS_OBJPATH,
									RSTP_DBUS_INTERFACE,
									MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP);

    dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &port_index,
								DBUS_TYPE_UINT32, &isEnable,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_stp_config_digest_string(char *digest_str)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int str_size = 0;
	unsigned int op_ret = 0;

	str_size = strlen(digest_str);
	if (str_size >= 32 + 1) {
		return STP_RETURN_CODE_MST_DIGEST_STRING_ILLEGAL;
	}
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,
									RSTP_DBUS_OBJPATH,
									RSTP_DBUS_INTERFACE,
									MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT);

    dbus_message_append_args(query,
								DBUS_TYPE_STRING, &digest_str,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_enable_stp_on_one_port_to_protocol
(
	unsigned int port_index,
	unsigned int enable,
	int lkState,
	unsigned int speed
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;
	int op_ret = 0;
	int port_link = lkState;
	int port_speed = speed;
	

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_PORT_STP_ENABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_UINT32,&port_link,
							 DBUS_TYPE_UINT32,&port_speed,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		printf("REPLY IS NULL.\n");
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_INT32,&op_ret,
			DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		printf("get args failed.\n");
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;

}

int dcli_enable_stp_on_one_port_to_npd
(
	unsigned int mode,
	unsigned int port_index,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;
	int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_INT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}

 int dcli_get_one_port_index
(
	unsigned int port_index,
	unsigned int* netif_index
) 
{
	DBusMessage *query, *reply;
    
	DBusError err;
	int op_ret = 0;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;
}



/**********************************************************************************
 *  dcli_stp_set_stpid_to_npd
 *
 *	DESCRIPTION:
 * 		bind vlan id  and specifed stpid
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		CMD_SUCCESS
 *		CMD_WARNING
 *
 **********************************************************************************/
/*fixed*/
int dcli_stp_set_stpid_to_npd
(
	unsigned short vid,
	unsigned int mstid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int op_ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								MSTP_DBUS_METHOD_CFG_VLAN_ON_MST);
	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&vid,							 								 		
									DBUS_TYPE_UINT32,&mstid,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_unref(reply);
	return op_ret;  
}
int dcli_stp_set_port_prio_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORTPRIO);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			printf("execute command error\n");
		}
		
	} else {
		printf("Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_pathcost_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			printf("execute command error\n");
		}
		
	} else {
		printf("Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_edge_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_EDGE);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			printf("execute command error\n");
		}
		
	} else {
		printf("Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_p2p_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_P2P);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			printf("execute command error\n");
		}
		
	} else {
		printf("Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_nonstp_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_NONSTP);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			printf("execute command error\n");
		}
		
	} else {
		printf("Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


unsigned int dcli_get_one_port_admin_state
(
	unsigned int port_index,
	unsigned int *admin_state
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;

	unsigned int op_ret = 0;

    unsigned int enable = 0;
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE_ADMIN_STATE);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&port_index,
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(stpReply,&stpIter);			
	dbus_message_iter_get_basic(&stpIter, &op_ret);
	if(STP_RETURN_CODE_ERR_NONE == op_ret )
	{
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter,&enable);
		*admin_state = enable;		
	}
	dbus_message_unref(stpReply);
	return op_ret;
}

/*fixed*/
unsigned int dcli_get_next_one_port_info
(
	unsigned int port_index,
	unsigned int *port_state,
	PORT_T* port
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter  stpIter_array;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;

	int n,op_ret;
	unsigned char port_prio = 0;
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE_NEXTPORT);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32, &(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter, &op_ret);
	if(STP_RETURN_CODE_ERR_NONE == op_ret )
	{
        
	    dbus_message_iter_next(&stpIter);
		dbus_message_iter_recurse(&stpIter,&stpIter_array);

		dbus_message_iter_recurse(&stpIter_array,&stpIter_struct);
	    dbus_message_iter_get_basic(&stpIter_struct, &(port->port_index));

        dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port_prio));
		port->port_prio = port_prio;
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->operPCost));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->role));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->adminEnable));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,  &(port->operPointToPointMac));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->operEdge));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->portPrio.root_path_cost));			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct, &(port->portPrio.design_port));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);
		
		dbus_message_iter_get_basic(&stpIter_sub_struct, &(port->portPrio.design_bridge.prio));
		dbus_message_iter_next(&stpIter_sub_struct);
		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct, &(port->portPrio.design_bridge.addr[n]));
			dbus_message_iter_next(&stpIter_sub_struct);
		}
		dbus_message_unref(stpReply);
		return op_ret;
	}
	else 
	{
		dbus_message_unref(stpReply);
		return op_ret;
	}
}
int dcli_get_stp_port_num(unsigned int port_index)
{
	int ret = 0;
	unsigned int port_state = 0;
	PORT_T port;
	int stp_port_num = 0;
	memset(&port, 0, sizeof(PORT_T));
	port.port_index = port_index;
	while(1)
    {
        ret = dcli_get_next_one_port_info(port.port_index, &port_state, &port);
        if(STP_RETURN_CODE_ERR_NONE != ret)
            break;
		stp_port_num++;
	}
	return stp_port_num;
}
int dcli_get_mstp_port_num(void)
{
	int mstid = 0;
	int port_index = 0;
	int ret = 0;
	UID_STP_PORT_STATE_T portInfo;
	memset(&portInfo, 0, sizeof(UID_STP_PORT_STATE_T));
	unsigned short revision = 0;
	STPM_T this;
	char vlan_str[256];
	char pname[20];
	int mstp_port_count = 0;
	
	memset(&portInfo, 0, sizeof(UID_STP_PORT_STATE_T));
	memset(&this, 0, sizeof(STPM_T));
	memset(pname, 0, 20);
	memset(vlan_str, 0, 256);
 	
	for (mstid = MIN_MST_ID; mstid < MAX_MST_ID; mstid++)
	{
		ret = dcli_get_br_self_info(mstid, pname, &revision, &this, vlan_str);
		if (0 != ret)
			continue;
		port_index = 0;
		while(1)
		{
			ret = dcli_get_mstp_next_one_port_info(mstid, port_index, &port_index, &portInfo);
		    if(STP_RETURN_CODE_ERR_NONE != ret) 
				break;
			else
				mstp_port_count++;
		}
	}
    return mstp_port_count;
}
/*fixed*/
unsigned int dcli_get_one_port_info
(
	unsigned int port_index,
	unsigned int *port_state,
	PORT_T* port
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter  stpIter_array;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;


	int n,ret;

	/*printf("port index %d\n",port_index);*/
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter, &ret);
	dbus_message_iter_next(&stpIter);
	if(STP_RETURN_CODE_ERR_NONE == ret )
	{
		/*		
			Array of Port Infos.
			port no
			port prio
			port role
			port State
			port link
			port p2p
			port edge
			port Desi bridge
			port Dcost
			port D-port
		*/
		dbus_message_iter_recurse(&stpIter,&stpIter_array);

		dbus_message_iter_recurse(&stpIter_array,&stpIter_struct);
	
		dbus_message_iter_get_basic(&stpIter_struct, &(port->port_prio));
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->operPCost));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->role));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->adminEnable));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->operPointToPointMac));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->operEdge));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct, &(port->portPrio.root_path_cost));			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct, &(port->portPrio.design_port));

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);


		dbus_message_iter_get_basic(&stpIter_sub_struct, &(port->portPrio.design_bridge.prio));
		dbus_message_iter_next(&stpIter_sub_struct);

		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct, &(port->portPrio.design_bridge.addr[n]));
			dbus_message_iter_next(&stpIter_sub_struct);
		}
	}
	dbus_message_unref(stpReply);
	return ret;
}

unsigned int dcli_get_br_info
(
	STPM_T* stpm,
	unsigned int *root_br_portId
)
{
	DBusMessage *brQuery,*brReply;
	DBusError err;
	DBusMessageIter	 brIter,brIter_array,brIter_struct;

	int i = 0,op_ret = 1;

	brQuery = dbus_message_new_method_call(			\
						RSTP_DBUS_NAME,		\
						RSTP_DBUS_OBJPATH,	\
						RSTP_DBUS_INTERFACE, \
						RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO);

	dbus_error_init(&err);
	brReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, brQuery, -1, &err);
	dbus_message_unref(brQuery);
	if (NULL == brReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(brReply,&brIter);
	dbus_message_iter_get_basic(&brIter, &op_ret);
	dbus_message_iter_next(&brIter);
	if(STP_RETURN_CODE_ERR_NONE == op_ret)
	{

		dbus_message_iter_recurse(&brIter,&brIter_array);


		dbus_message_iter_recurse(&brIter_array,&brIter_struct);

		for(i = 0; i< 6; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootPrio.root_bridge.addr[i])); 
			dbus_message_iter_next(&brIter_struct);
		}
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootPrio.root_bridge.prio));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootPrio.root_path_cost));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, root_br_portId);
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootTimes.MaxAge));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootTimes.HelloTime));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->rootTimes.ForwardDelay));
		dbus_message_iter_next(&brIter_struct);
		for(i = 0; i< 6; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct, &(stpm->BrId.addr[i]));
			dbus_message_iter_next(&brIter_struct);
		}
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->BrId.prio));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->ForceVersion));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->BrTimes.MaxAge));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->BrTimes.HelloTime));
		dbus_message_iter_next(&brIter_struct);
		dbus_message_iter_get_basic(&brIter_struct, &(stpm->BrTimes.ForwardDelay));
		dbus_message_iter_next(&brIter_struct);
		
	}
	dbus_message_unref(brReply);
	return op_ret;
	
}

unsigned int dcli_get_brage_info
(
)
{
	DBusMessage *brQuery,*brReply;
	DBusError err;
	DBusMessageIter	 brIter,brIter_struct;

	unsigned char   root_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned char   design_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int      root_path_cost,design_br_version;
	unsigned short  root_br_prio,design_br_prio;
	unsigned short  root_br_portId;
	unsigned short  root_br_maxAge,design_br_maxAge;
	unsigned short  root_br_hTime,design_br_hTime;
	unsigned short  root_br_fdelay,design_br_fdelay;
	int i,ret;

	brQuery = dbus_message_new_method_call(			\
						RSTP_DBUS_NAME,		\
						RSTP_DBUS_OBJPATH,	\
						RSTP_DBUS_INTERFACE, \
						RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO);

	dbus_error_init(&err);
	brReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,brQuery,-1, &err);
	dbus_message_unref(brQuery);
	if (NULL == brReply) {
		/*vty_out(vty,"failed get slot count npdReply.\n");*/
		printf("failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			/*vty_out(vty,"%s raised: %s",err.name,err.message);*/
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(brReply);
		return CMD_WARNING;
	}

	dbus_message_iter_init(brReply,&brIter);
	dbus_message_iter_get_basic(&brIter, &ret);
	dbus_message_iter_next(&brIter);
	if(DCLI_STP_OK == ret )
	{

	dbus_message_iter_recurse(&brIter,&brIter_struct);

	/*display root bridge info*/
		/*vty_out(vty,"-----------------------SPANNING TREE information of STP domain 0------------------------------\n");

		//vty_out(vty,"Designated Root\t\t\t:  ");*/

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&root_br_mac[i]);
			/*vty_out(vty,"%02x:",root_br_mac[i]);*/
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		/*vty_out(vty,"%02x",design_br_mac[5]);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"\nDesignated Root Priority\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_prio);
		/*vty_out(vty,"%d\n",root_br_prio);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Designated Root Path Cost\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_path_cost);
		/*vty_out(vty,"%d\n",root_path_cost);*/
		dbus_message_iter_next(&brIter_struct);


		
		/*vty_out(vty,"Root Port\t\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_portId);
		if(root_br_portId){
			;/*vty_out(vty,"%d\n",root_br_portId);*/
		}
		else{
			;/*vty_out(vty,"%s\n","none");*/
		}
		dbus_message_iter_next(&brIter_struct);


		/*vty_out(vty,"Root Max Age");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_maxAge);
		/*vty_out(vty,"%6d\t",root_br_maxAge);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Hello Time");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_hTime);
		/*vty_out(vty,"%5d\t\t",root_br_hTime);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Forward Delay");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_fdelay);
		/*vty_out(vty,"%5d\n",root_br_fdelay);*/
		dbus_message_iter_next(&brIter_struct);

		/*display self-bridge info*/

		/*vty_out(vty,"\nBridge ID Mac Address\t\t:  ");*/

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
			/*vty_out(vty,"%02x:",design_br_mac[i]);*/
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		/*vty_out(vty,"%02x",design_br_mac[5]);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"\nBridge ID Priority\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_prio);
		/*vty_out(vty,"%d\n",design_br_prio);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Bridge ID ForceVersion\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_version);
		/*vty_out(vty,"%d\n",design_br_version);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Bridge Max Age");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_maxAge);
		/*vty_out(vty,"%5d\t",design_br_maxAge);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Hello Time");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_hTime);
		/*vty_out(vty,"%5d\t\t",design_br_hTime);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Forward Delay");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_fdelay);
		/*vty_out(vty,"%5d\n",design_br_fdelay);*/
		dbus_message_iter_next(&brIter_struct);
		dbus_message_unref(brReply);
		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(brReply);
		return ret;
	}
	else
	{
		dbus_message_unref(brReply);
		return ret;
	}
	
}



int dcli_get_cist_info(STPM_T* cist_stpm, unsigned int *netif_index)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter;

	unsigned int i,op_ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_CIST_INFO);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if(STP_RETURN_CODE_ERR_NONE == op_ret) {

    	for(i = 0; i < 6; i++)
    	{
    		dbus_message_iter_get_basic(&iter,&(cist_stpm->rootPrio.root_bridge.addr[i]));
    		dbus_message_iter_next(&iter);
    	}
		dbus_message_iter_get_basic(&iter,&(cist_stpm->rootPrio.root_bridge.prio));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(cist_stpm->rootPrio.root_path_cost));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,netif_index);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(cist_stpm->rootTimes.MaxAge));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(cist_stpm->rootTimes.HelloTime));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(cist_stpm->rootTimes.ForwardDelay));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(cist_stpm->rootTimes.RemainingHops));
	}
	dbus_message_unref(reply);
	return op_ret;
}


int dcli_get_msti_info
(
	int 	  mstid,
	STPM_T*   this,
	unsigned int *netif_index
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter;
	
	unsigned int i,op_ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_MSTI_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);
	if(STP_RETURN_CODE_ERR_NONE == op_ret) 
	{
		for(i = 0; i< 6; i++)
		{
			dbus_message_iter_get_basic(&iter, &(this->rootPrio.region_root_bridge.addr[i]));
			dbus_message_iter_next(&iter);
		}
		dbus_message_iter_get_basic(&iter, &(this->rootPrio.region_root_bridge.prio));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(this->rootPrio.region_root_path_cost));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, netif_index);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(this->rootTimes.MaxAge));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(this->rootTimes.HelloTime));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(this->rootTimes.ForwardDelay));
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(this->rootTimes.RemainingHops));
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_get_br_self_info(
	int mstid,
	char *pname,
	unsigned short *revision,
	STPM_T* this,
	char *vlan_str
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter;
	unsigned int count = 0;
	int i = 0, ret=0;
	char *tmp_pname = NULL;
	char *tmp_vlan_str = NULL;
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_SELF_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	if(STP_RETURN_CODE_ERR_NONE == ret) {

		dbus_message_iter_get_basic(&iter,&tmp_pname);
		memcpy(pname, tmp_pname, 20);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, revision);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&count);
		this->vlan_map.ulcount = count;
		dbus_message_iter_next(&iter);
		
        dbus_message_iter_get_basic(&iter, &tmp_vlan_str);
		memcpy(vlan_str, tmp_vlan_str, 256);
		dbus_message_iter_next(&iter);
		
		for(i = 0; i< 6; i++)
		{
			dbus_message_iter_get_basic(&iter, &(this->BrId.addr[i]));
			dbus_message_iter_next(&iter);
		}

		dbus_message_iter_get_basic(&iter, &(this->BrId.prio));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &(this->ForceVersion));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &(this->BrTimes.MaxAge));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &(this->BrTimes.HelloTime));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &(this->BrTimes.ForwardDelay));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &(this->BrTimes.RemainingHops));

	}
	dbus_message_unref(reply);
	return ret;	
	
}

int dcli_get_mstp_one_port_info
(
	int 		  mstid,
	unsigned int  port_index,
	UID_STP_PORT_STATE_T *portInfo
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	int ret;
	int n  = 0;
	unsigned char   port_prio;

	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								MSTP_DBUS_METHOD_GET_PORT_INFO);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&mstid,
					 DBUS_TYPE_UINT32,&(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(stpReply);
		return STP_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter,&ret);
    if(0 != ret)
    {
        dbus_message_unref(stpReply);
        return STP_RETURN_CODE_ERR_GENERAL;
    }
	dbus_message_iter_next(&stpIter);
	if(STP_RETURN_CODE_ERR_NONE == ret)
	{
	/*		
		Array of Port Infos.
		port no
		port prio
		port role
		port State
		port link
		port p2p
		port edge
		port Desi bridge
		port Dcost
		port D-port
	*/

		dbus_message_iter_get_basic(&stpIter, &port_prio);
		portInfo->port_prio = port_prio;
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->path_cost));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->role));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->state));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->linkState));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->oper_point2point));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->oper_edge));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_cost));			

		dbus_message_iter_next(&stpIter);			
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_port));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_bridge.prio));
		dbus_message_iter_next(&stpIter);
		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter,&(portInfo->designated_bridge.addr[n]));
			dbus_message_iter_next(&stpIter);
		}						
	}
	dbus_message_unref(stpReply);
	return ret;		
}

int dcli_get_mstp_next_one_port_info
(
	int 		  mstid,
	unsigned int  port_index,
	unsigned int *next_port_index,
	UID_STP_PORT_STATE_T *portInfo
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	int ret;
    int n;
	unsigned char   port_prio;
    unsigned int     ret_port_index;

	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								MSTP_DBUS_METHOD_GET_NEXTPORT_INFO);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&mstid,
					 DBUS_TYPE_UINT32,&(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter,&ret);
    if(0 != ret)
    {
        dbus_message_unref(stpReply);
        return STP_RETURN_CODE_ERR_GENERAL;
    }
	dbus_message_iter_next(&stpIter);
	if(STP_RETURN_CODE_ERR_NONE == ret)
	{
	/*		
		Array of Port Infos.
		port no
		port prio
		port role
		port State
		port link
		port p2p
		port edge
		port Desi bridge
		port Dcost
		port D-port
	*/

        dbus_message_iter_get_basic(&stpIter, &ret_port_index);
        *next_port_index = ret_port_index;
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &port_prio);
		portInfo->port_prio = port_prio;
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->path_cost));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->role));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->state));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->linkState));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->oper_point2point));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->oper_edge));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_cost));			

		dbus_message_iter_next(&stpIter);			
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_port));

		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_bridge.prio));
		dbus_message_iter_next(&stpIter);
		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter, &(portInfo->designated_bridge.addr[n]));
			dbus_message_iter_next(&stpIter);
		}
	}
	dbus_message_unref(stpReply);
	return ret;		
}
/**************************************************************************
*
*	command func
*
*
***************************************************************************/

int dcli_set_bridge_max_age
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int max_age
)
{
}

int dcli_set_bridge_hello_time
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int htime
)
{
}

int dcli_set_bridge_forward_delay
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int fdelay
)
{
}

/*fixed*/
int dcli_set_bridge_force_version
(
	unsigned int fversion
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret = 0;
	unsigned int value = fversion;
	
	query = dbus_message_new_method_call(
										RSTP_DBUS_NAME,				\
										RSTP_DBUS_OBJPATH,			\
										RSTP_DBUS_INTERFACE,		\
										RSTP_DBUS_METHOD_CONFIG_FORVERSION);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return STP_RETURN_CODE_ERR_GENERAL;
	}
    if (!(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)))
	{
        if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return STP_RETURN_CODE_ERR_GENERAL;
	} 
		
	dbus_message_unref(reply);
	return ret;
}

int dcli_set_bridge_max_hops
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int maxhops
)
{

}

int dcli_set_bridge_default_value
(
	DCLI_STP_RUNNING_MODE mode
)
{

}

int dcli_set_port_priority
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{

}



