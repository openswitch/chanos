
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_AAA
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"
#include "sysdef/returncode.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef.h"
#include "man/man_security.h"
#include "man/man_radius.h"

extern DBusConnection *dcli_dbus_connection;

int dcli_radius_clear_radius_statistics(int radius_id)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	DBusMessage *query; 
	int ret;
    int i=0;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_CLEAR_RADIUS_STATISTICS);													
	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic(  &iter,
									 DBUS_TYPE_UINT32,
									 &radius_id);		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}




#if 0
DEFUN(	config_radius_authorization_network_cmd_func,
	  		config_radius_authorization_network_cmd,
		    "authorization network radius",
		    "Config authorization\n"
		    "Config network authorization\n"
		    "Authorization radius\n"
	)	
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	DBusError err;
	int ret,ret1 ;
    int i=0;
	DBusMessage *query; 
	int stat = ENABLE;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;	
	
	dbus_error_init(&err);
	

	/*now we  send port_index-vlan_id-security to asd .*/

	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTHORIZATION_NETWORK_RADIUS);
	dbus_message_append_args( query,
							  DBUS_TYPE_INT32,&stat,
							  DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dcli_process_dbus_result(vty, ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS; 
	
}	

DEFUN(	config_radius_no_authorization_network_cmd_func,
	  		config_radius_no_authorization_network_cmd,
		    "no authorization network radius",
		    NO_STR
		    "Config authorization\n"
		    "Config network authorization\n"
		    "Authorization radius\n"
	)	
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	DBusError err;
	int ret,ret1 ;
    int i=0;
	DBusMessage *query; 
	int stat = DISABLE;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;	
	
	dbus_error_init(&err);
	

	/*now we  send port_index-vlan_id-security to asd .*/

	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTHORIZATION_NETWORK_RADIUS);
	dbus_message_append_args( query,
							  DBUS_TYPE_INT32,&stat,
							  DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dcli_process_dbus_result(vty, ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS; 
	
}	

#endif

int dcli_radius_conf_accounting_mode(int radius_id, char status)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_BYTE,&status,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);
	return ret;
}

int dcli_radius_config_attribute_def
(
	int radius_id, 
	unsigned char isAdd,
	unsigned int attr_flag,
	unsigned char attr_type, 
	unsigned char attr_subtype, 
	unsigned char attr_format, 
	unsigned char attr_mode,
	unsigned char attr_usage,
	unsigned char *attr_value
)
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret = ASD_DBUS_SUCCESS;
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_ATTR_DEF);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_BYTE,&isAdd,
							 DBUS_TYPE_UINT32,&attr_flag,
							 DBUS_TYPE_BYTE,&attr_type,
							 DBUS_TYPE_BYTE,&attr_subtype,
							 DBUS_TYPE_BYTE,&attr_format,
							 DBUS_TYPE_BYTE,&attr_mode,
							 DBUS_TYPE_BYTE,&attr_usage,
							 DBUS_TYPE_STRING,&attr_value,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}

int dcli_radius_config_attribute_4_ipaddr(int radius_id, int status, char *host_ip)
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret = ASD_DBUS_SUCCESS;
	
	if (strlen(host_ip) && Check_IP_Format(host_ip)){
		return ASD_IP_ADDR_INVALID;
	}
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_ATTR_4);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&status,
							 DBUS_TYPE_STRING,&host_ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}


int dcli_radius_config_server_ip_addr(int radius_id, int port, char *ip, char *shared_secret, char *server_name, int type)
{		
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret;	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	
	if(port > 65535){
		return ASD_SECURITY_PARAMETER_OUT_OF_RANGE;
	}	
	
	ret = Check_IP_Format(ip);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}

	ret = dcli_str_check(shared_secret, PASSWD_TYPE_STR, MAX_PASSWD_LEN, MIN_PASSWD_LEN);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}	

	
	ret = dcli_str_check(server_name, NAME_TYPE_STR, MAX_NAME_LEN, MIN_NAME_LEN);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	if(type == RADIUS_ACCOUNTING_TYPE){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ACCT);
	}else if(type == RADIUS_AUTHENTICATION_TYPE)	{	
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTH);
	}
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&port,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_STRING,&server_name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}



int dcli_radius_config_server_key_by_ip(int radius_id, char *ip, char *shared_secret, int type)
{	
				
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int ret;
	
	ret = Check_IP_Format(ip);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}

	ret = dcli_str_check(shared_secret, PASSWD_TYPE_STR, MAX_PASSWD_LEN, MIN_PASSWD_LEN);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}	
	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_BY_IP);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);
	return ret;
}


int dcli_radius_config_server_key_by_name(int radius_id, char *name, char *shared_secret, int type)
{	
				
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int ret;

	
	ret = dcli_str_check(shared_secret, PASSWD_TYPE_STR, MAX_PASSWD_LEN, MIN_PASSWD_LEN);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_BY_NAME);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_STRING,&shared_secret,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);
	return ret;
}

int dcli_radius_config_server_name_by_ip(int radius_id, char *ip, char *name, int type)
{	
				
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int ret;
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_RADIUS_SERVER_NAME_BY_IP);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);
	return ret;
}

int dcli_radius_del_server_by_ip(int radius_id, char *ip, int type)
{		
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;	

	
	ret = Check_IP_Format(ip);
	if(ret != ASD_DBUS_SUCCESS){
		return ASD_IP_ADDR_INVALID;
	}
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DEL_RADIUS_SERVER_BY_IP);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	
	return ret;
}

int dcli_radius_del_server_by_name(int radius_id, char *name, int type)
{		
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;	

	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DEL_RADIUS_SERVER_BY_NAME);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	
	return ret;
}

#if 0
DEFUN(	config_radius_server_msgauth_cmd_func,
	  		config_radius_server_msgauth_cmd,
		    "radius server msgauth IPADDR",
		    "Config radius\n"
		    "Radius server\n"
		    "Message authenticator\n"
		    CONFIG_IPADDR_STR
		    )   
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned int type;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;


		
	type = ENABLE;			
	char ip_addr[16] = {};	
	int port;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;

	ret = Check_IP_Format((char*)argv[0]);
	if(ret != ASD_DBUS_SUCCESS){
		//vty_out(vty,"%% <error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	memcpy(ip_addr, argv[0], sizeof(ip_addr));	
	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_MSGAUTH);
	/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &type,
							 DBUS_TYPE_STRING, &ip_addr,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return CMD_FAILURE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	
	dcli_process_dbus_result(vty, ret);	
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(	config_radius_no_server_msgauth_cmd_func,
	  		config_radius_no_server_msgauth_cmd,
		    "no radius server msgauth IPADDR",
		    NO_STR
		    "Config radius\n"
		    "Radius server\n"
		    "Message authenticator\n"
		    CONFIG_IPADDR_STR
		    )   
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	unsigned int type;
	char ip_addr[16] = {};	
	int port;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	type = DISABLE;		
	ret = Check_IP_Format((char*)argv[0]);
	if(ret != ASD_DBUS_SUCCESS){
		//vty_out(vty,"%% <error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	memcpy(ip_addr, argv[0], sizeof(ip_addr));
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_MSGAUTH);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &type,
							 DBUS_TYPE_STRING, &ip_addr,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return CMD_FAILURE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	
	dcli_process_dbus_result(vty, ret);	
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif


int dcli_radius_config_primary_server_by_ip(int radius_id, char *ip_addr, int type)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	
	int index = 0;			
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ret = Check_IP_Format(ip_addr);
	if(ret != ASD_DBUS_SUCCESS){
		return ASD_IP_ADDR_INVALID;
	}
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_PRIMARY_BY_IP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &radius_id,
							 DBUS_TYPE_STRING, &ip_addr,
							 DBUS_TYPE_UINT32, &type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}


int dcli_radius_config_primary_server_by_name(int radius_id, char *name, int type)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	
	int index = 0;			
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_PRIMARY_BY_NAME);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &radius_id,
							 DBUS_TYPE_STRING, &name,
							 DBUS_TYPE_UINT32, &type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}



int dcli_radius_config_server_retransmit(int radius_id, int times)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_RETRANSMIT);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &radius_id,
							 DBUS_TYPE_UINT32, &times,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	
	return ret;
}


int dcli_radius_config_server_timeout(int radius_id, int seconds)
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret = ASD_DBUS_SUCCESS;

	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_RADIUS_SERVER_TIMEOUT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &radius_id,
							 DBUS_TYPE_UINT32, &seconds,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	
	return ret;
}


int dcli_radius_show_radius(int radius_id, radius_info_type *radius_info)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter, iter_array;
	DBusError err;
	int index, attr_num = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	char *attribute_4_value;

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &radius_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_unref(reply);
		return ret;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->auth_count);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->acct_count);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->retrans);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->time_duration);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->account_disable);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_info->attribute_4_enable);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &attribute_4_value);
	strncpy(radius_info->attribute_4_value, attribute_4_value, sizeof(radius_info->attribute_4_value)-1);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &attr_num);

	if(attr_num)
	{
		int i = 0;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
				
		for (i = 0; i < attr_num; i++) {
			unsigned char *value;
			DBusMessageIter iter_struct;				
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_flag));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_type));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_subtype));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_format));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_mode));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(radius_info->attrDef[i].rad_attr_usage));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(value));
			strncpy(radius_info->attrDef[i].rad_attr_value, value, strlen(value));
		
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);
	return ret;
}



int dcli_radius_show_server(radius_server_info_type *radius_server_info, radius_server_info_type *radius_server_ret_info)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	DBusMessage *query; 
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;
	unsigned char *str_ptr;
	
	
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DOT1X_GET_NEXT_RADIUS_SERVER);
	dbus_message_iter_init_append (query, &iter);

	
	str_ptr = radius_server_info->ip_addr;
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_STRING,
										&str_ptr);	
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
										&radius_server_info->type);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter, &ret);
	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_unref(reply);
		return ret;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_ptr);
	memcpy(radius_server_ret_info->server_name, str_ptr, sizeof(radius_server_ret_info->server_name));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_ptr);
	memcpy(radius_server_ret_info->ip_addr, str_ptr, sizeof(radius_server_ret_info->ip_addr));

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_ptr);
	memcpy(radius_server_ret_info->domain, str_ptr, sizeof(radius_server_ret_info->domain));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_server_ret_info->type);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_server_ret_info->port);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_ptr);
	memcpy(radius_server_ret_info->secret, str_ptr, sizeof(radius_server_ret_info->secret));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &radius_server_ret_info->primary);
	
	dbus_message_unref(reply);

	return ret;

}

int man_radius_get_server_info(int radius_id, int type, int mode, char *filter, radius_serv_detail_info_type *serv_detail_info)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter, iter_array;
	DBusError err;

	unsigned int i;
	char *str_prt = NULL;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_CURR_RADIUS_SERVER_CONF);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_STRING,&filter,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret != ASD_DBUS_SUCCESS){
		dbus_message_unref(reply);
		return ret;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_prt);
	strncpy(serv_detail_info->server_name, str_prt, sizeof(serv_detail_info->server_name)-1);

    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_prt);
	strncpy(serv_detail_info->ip_addr, str_prt, IP_LENGTH-1);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &serv_detail_info->primary);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &serv_detail_info->retrans);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &serv_detail_info->time_duration);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &serv_detail_info->account_disable);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &serv_detail_info->attribute_4_enable);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &str_prt);
	strncpy(serv_detail_info->attribute_4_value, str_prt, sizeof(serv_detail_info->attribute_4_value)-1);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_radius_show_statistics_by_ip(int radius_id, int type, char *ip_addr, radius_statistics_type *radius_statis_info)
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned char *ch = NULL;
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_STATISTICS_BY_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&ip_addr,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&ch);
		strncpy(radius_statis_info->server_name, ch, PATH_LEN - 1);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_accepts);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_challenges);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_rejects);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->bad_authenticators);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->index);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->malformed_responses);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->packets_dropped);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->requests);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->responses);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->retransmissions);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->round_trip_time);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->timeouts);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->unknown_types);
		
	}
	else if (ret == ASD_RADIUS_NO_CONNECT){	
		memset(radius_statis_info, 0 ,sizeof(*radius_statis_info));
		ret = ASD_DBUS_SUCCESS;
	}
	
	dbus_message_unref(reply);
	
	return ret;
}


int dcli_radius_show_statistics_by_name(int radius_id, int type, char *name, radius_statistics_type *radius_statis_info)
{	

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = ASD_DBUS_SUCCESS;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	char *ch = NULL;
	
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_STATISTICS_BY_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radius_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == ASD_DBUS_SUCCESS){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&ch);
		strncpy(radius_statis_info->ip_addr, ch, IP_LENGTH-1);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_accepts);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_challenges);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->access_rejects);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->bad_authenticators);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->index);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->malformed_responses);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->packets_dropped);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->requests);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->responses);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->retransmissions);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->round_trip_time);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->timeouts);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&radius_statis_info->unknown_types);
	}
	else if(ASD_RADIUS_NO_CONNECT == ret){
		memset(radius_statis_info, 0, sizeof(*radius_statis_info));
		ret = ASD_DBUS_SUCCESS;
	}
	
	dbus_message_unref(reply);
	return ret;
}

#endif
