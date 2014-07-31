
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
#include "dbus/npd/npd_dbus_def.h"
#include "lib/netif_index.h"
#include "man/man_security.h"


extern DBusConnection *dcli_dbus_connection;
char ASD_DBUS_BUSNAME[PATH_LEN] = "aw.asd";
char ASD_DBUS_OBJPATH[PATH_LEN] = "/aw/asd";
char ASD_DBUS_INTERFACE[PATH_LEN] = "aw.asd";
char ASD_DBUS_STA_OBJPATH[PATH_LEN] = "/aw/asd/sta";
char ASD_DBUS_STA_INTERFACE[PATH_LEN] = "aw.asd.sta";
char ASD_DBUS_SECURITY_OBJPATH[PATH_LEN] = "/aw/asd/security";
char ASD_DBUS_SECURITY_INTERFACE[PATH_LEN]= "aw.asd.security";	
char ASD_DBUS_AC_GROUP_OBJPATH[PATH_LEN] = "/aw/asd";
char ASD_DBUS_AC_GROUP_INTERFACE[PATH_LEN] = "aw.asd";

int Check_IP_Format(char* str){
	unsigned int ipAddr;

	ipAddr = lib_get_ip_from_string(str);
	if(ipAddr == -1)
		return ASD_IP_ADDR_INVALID;
	
	ipAddr = ntohl(ipAddr);
	/* legal ip address must not be broadcast,0,or multicast,*/
	/* or 169.254.x.x or 127.0.0.1*/
	if(	(0==ipAddr) ||
		(0xFFFFFFFF == ipAddr)||
		(0x7F000001 == ipAddr ) ||
		(0xE0 == ((ipAddr >> 24)&0xF0)))
	{
		return ASD_IP_ADDR_INVALID;	
	}
	
	return ASD_DBUS_SUCCESS;

}


void ReInitDbusPath(int index, char * path, char * newpath)
{
	int len;
	sprintf(newpath,"%s%d",path,index);	
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	/*else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio");			
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}*/
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
}

int asd_get_if_array_index(unsigned int netif_index)
{
	return netif_array_index_from_ifindex(netif_index);
#if 0	
	int netif_type;
	int array_index = -1;

	netif_type = npd_netif_type_get(netif_index);

	if(NPD_NETIF_ETH_TYPE == netif_type){
		int port_array_index = 0;
		port_array_index = eth_port_array_index_from_ifindex(netif_index);
	
		if(port_array_index >= port_max_num){
			return -1;
		}
		array_index = port_array_index;
	}
	else if(NPD_NETIF_TRUNK_TYPE == netif_type){
		int trunk_id = 0;
		trunk_id = npd_netif_trunk_get_tid(netif_index);

		if(trunk_id >= trunk_max_num){
			return -1;
		}

		array_index = port_max_num + trunk_id;
	}	

	return array_index;
#endif	
}


int dcli_str_check(const char *str, STRING_TYPE_T type, int max__len, int min_len)
{
	int i, len;
	const char *chr;

	chr = str;
	len = strlen(chr);
	for(i =0; i< len; i++){
		if(!isalpha(chr[i]) && !isalnum(chr[i]) && (chr[i] != '_')){
			return ASD_STR_CONTAINS_INVALID_CHARACTER;
		}
	}

	if(!strcmp(str, NULL_STRING)){
		return 0;
	}

	if(len > max__len){
		return ASD_STR_IS_TOO_LONG;
	}
	if(len < min_len){
		return ASD_STR_IS_TOO_SHORT;
	}
	
	return 0;
}

int dcli_check_reply(DBusMessage *reply, DBusError err)
{
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}	
	return 0;
}
int dcli_security_policy_check( unsigned int security_type, unsigned int encription_type, unsigned char *key)
{
    int ret = 0;
    unsigned int len;
    
    
    if((security_type == WPA_P)||(security_type == WPA2_P)||(security_type == WAPI_PSK))
	{
		len = strlen(key);
		if((len < 8)||(len > SECURITY_KEY_LEN -1)){
			ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
		}
	}
	else if(((security_type == OPEN)&&(encription_type == WEP))||(security_type == SHARED)){
		len = strlen(key);
		if((len != 5)&&(len != 13)&&(len != 16)){
			ret = ASD_SECURITY_KEY_LEN_NOT_PERMIT;
		}
	}	
	return ret;
}

int dcli_asd_config_security_type(unsigned int security_ID, unsigned int type,unsigned int index)
{	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&security_ID,
							 DBUS_TYPE_UINT32,&type,
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


int dcli_asd_config_encryption_type(unsigned int security_id, unsigned int type, unsigned int index)
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&security_id,
							 DBUS_TYPE_UINT32,&type,
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


int dcli_asd_config_security_profile_key(unsigned int security_id, unsigned char input_type_of_key,unsigned char *key, unsigned int index)
{
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_KEY);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&security_id,
							 DBUS_TYPE_STRING,&key,
							 DBUS_TYPE_BYTE,&input_type_of_key,
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

int dcli_asd_config_system_auth_control(int stat)
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	int i=0;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;	
	
	dbus_error_init(&err);


	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_DOT1X_SYSTEM_AUTH_CONTROL);
														
	dbus_message_append_args(query, 
							DBUS_TYPE_UINT32,&stat,
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



int dcli_asd_config_authentication_mode(int authentication_mode)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret;
	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AUTHENTICATION_MODE);
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&authentication_mode);
	
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

int dcli_asd_reauthenticate_interface(int *netif_index, int p_count)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	DBusMessage *query; 

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int ret;
    int i=0;


	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_REAUTHENTICATE_PORT);
		
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&p_count);
	
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	for(i = 0; i < p_count; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					   DBUS_TYPE_UINT32,
					  &(netif_index[i]));

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container (&iter, &iter_array);

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

int dcli_asd_clear_dot1x_statistics(int *port_index, int p_count)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessage *query; 
	DBusError err;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret;
    int i=0;	
	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_CLEAR_DOT1X_STATISTICS);
		
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&p_count);
	
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	for(i = 0; i < p_count; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					   DBUS_TYPE_UINT32,
					  &(port_index[i]));
		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container (&iter, &iter_array);
	
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

int dcli_asd_config_dot1x_control(unsigned int netif_index, unsigned char stat)
{
	int ret;

	int i=0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	
	
	/*now send  slot-port-stat to asd */
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE);
	
	dbus_message_iter_init_append (query, &iter);

    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& netif_index);
		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_BYTE,
										&stat);
				
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


int dcli_asd_config_max_req(unsigned int netif_index,  int count)
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];


	index = 0;	
	/*send this to the asd(take wid as example first)*/
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_DOT1X_MAX_REQUEST);
														
				
	dbus_message_iter_init_append (query, &iter);

    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										& netif_index);
		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&count);				
				
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



int dcli_asd_config_max_user(unsigned int netif_index,  int count)
{	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned int *ifList = NULL;

	/*send this to the asd(take wid as example first)*/
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_DOT1X_MAX_USER);
														
				
	dbus_message_iter_init_append (query, &iter);
    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);
		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&count);				
		
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


int dcli_asd_config_reauthentication(unsigned int netif_index,  int stat)
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_PORT_REAUTHENTICATION);
														
	dbus_message_iter_init_append(query, &iter);
    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);	
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&stat);

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


int dcli_asd_config_guest_vlan_period(unsigned int netif_index,int period)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	int ret;

	
	if(period<1||period>65535)
	{
		return ASD_SECURITY_PARAMETER_OUT_OF_RANGE;
	}
	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_GUEST_VLAN_PERIOD);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);	
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);

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


int dcli_asd_config_reauth_period(unsigned int netif_index,  int period)
{
	int ret;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);	
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);

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


int dcli_asd_config_quiet_period(unsigned int netif_index,  int period)
{

	int ret;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);	
    
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


int dcli_asd_config_tx_period(unsigned int netif_index, int period)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;
	
	if(period<0||period>65535)
	{
		return ASD_SECURITY_PARAMETER_OUT_OF_RANGE;
	}	

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_TX_PERIOD);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
    dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);	
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);
					
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


int dcli_asd_config_supp_timeout(unsigned int netif_index,  int period)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;
	

	if(period<0||period>65535)
	{
		return ASD_SECURITY_PARAMETER_OUT_OF_RANGE;
	}

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SUPP_TIMEOUT);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);	
					
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


int dcli_asd_config_server_timeout(unsigned int netif_index,  int period)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	

	if(period<0||period>65535)
	{
		return ASD_SECURITY_PARAMETER_OUT_OF_RANGE;
	}

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SERV_TIMEOUT);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&period);	
	
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

int dcli_asd_config_mac_auth_bypass(unsigned int netif_index,  char isEnable)
{
	int ret;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_MAC_AUTH_BYPASS);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&netif_index);
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_BYTE,
										&isEnable);	
	
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

int dcli_asd_config_guest_vlan(unsigned int *netif_index, int num ,unsigned short vlan_id)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret;
	int i=0;
	int op_ret;
	
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME, OBJPATH, INTERFACE, ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_GUEST_VLAN);
															
	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic(  &iter,
									 DBUS_TYPE_UINT16,
									 &vlan_id);		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&num);
	
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);


	for(i = 0; i <num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &netif_index[i]);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;


}



int dcli_asd_config_auth_fail_vlan(unsigned int *netif_index, int num, unsigned short vlan_id)
{	
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i=0;
	
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call( BUSNAME, OBJPATH, INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_DOT1X_PORT_AUTH_FAIL_VLAN);
															
	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic(  &iter,
									 DBUS_TYPE_UINT16,
									 &vlan_id);		
	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&num);
	
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);


	for(i = 0; i <num; i++){			
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &netif_index[i]);

		dbus_message_iter_close_container (&iter_array, &iter_struct);

	}
				
	dbus_message_iter_close_container (&iter, &iter_array);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;
}

int dcli_asd_show_authentication_mode(int *authentication_mode)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret;
    *authentication_mode = -1;
	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_AUTHENTICATION_MODE);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(0 == ret){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,authentication_mode);
	}
	dbus_message_unref(reply);
	
	return ret; 
	
}


int dcli_asd_show_dot1x_summary(dot1x_summary_type *dot1x_summary_info)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int ret;
	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_SUMMARY);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(0 == ret){
		memset(dot1x_summary_info, 0, sizeof(*dot1x_summary_info));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dot1x_summary_info->sys_auth_control);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dot1x_summary_info->authentication_mode);
	}
	dbus_message_unref(reply);
	
	return ret; 
	
}

int dcli_asd_show_dot1x(int netif_index, asd_show_dot1x_type *dot1x_show_info, int *next_netif_index)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	DBusMessage *query; 
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int ret = 0;

	index= 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DOT1X_GET_NEXT_IFINDEX);
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
										&netif_index);	
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
	
	dbus_message_iter_get_basic(&iter, &netif_index);
	
	if(netif_index <=0){
		dbus_message_unref(reply);
		ret = ASD_DBUS_ERROR;
		return ret;
	}
	*next_netif_index = netif_index;
	dbus_message_unref(reply);	
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_INFORMATION);
	dbus_message_iter_init_append (query, &iter);
	
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
										&netif_index);	
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
	
	dbus_message_iter_get_basic(&iter, &dot1x_show_info->security_type);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_info->encryption_type);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_info->port_dot1x_ctrl);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_info->eap_reauth_ctrl);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_info->mac_auth_bypass);
	dbus_message_iter_next(&iter);
	
	ret = npd_netif_index_to_user_fullname(netif_index, dot1x_show_info->name);
	dbus_message_unref(reply);	
	return ret; 
	
}



int dcli_asd_show_dot1x_timer(int netif_index, asd_dot1x_timer_type *dot1x_show_timer_info, int *next_netif_index)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	DBusMessage *query; 
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int index = 0;
	int ret = 0;

	/*now we  send port_index-vlan_id-security to asd .*/
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_DOT1X_GET_NEXT_IFINDEX);
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
										&netif_index);	
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
	
	dbus_message_iter_get_basic(&iter, &netif_index);
	
	if(netif_index <=0){
		dbus_message_unref(reply);
		ret = ASD_DBUS_ERROR;
	}
	*next_netif_index = netif_index;
	dbus_message_unref(reply);	
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_TIMER_INFORMATION);
	dbus_message_iter_init_append (query, &iter);
	
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
										&netif_index);	
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
	
	dbus_message_iter_get_basic(&iter,&dot1x_show_timer_info->eap_reauth_period);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_timer_info->guest_vlan_period);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_timer_info->quiet_period);
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_timer_info->tx_period);
	dbus_message_iter_next(&iter);
 	
 	dbus_message_iter_get_basic(&iter, &dot1x_show_timer_info->supp_timeout);
 	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &dot1x_show_timer_info->serv_timeout);
	dbus_message_iter_next(&iter);
	
	dbus_message_unref(reply);
	
	ret = npd_netif_index_to_user_fullname(netif_index, dot1x_show_timer_info->name);
	
	return ret;
	
}


int dcli_asd_show_dot1x_detail(int netif_index, asd_dot1x_detail_type *dot1x_detail_info)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;
	int ret;
	char buff[PATH_LEN];
	char *key;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	


	if( npd_netif_index_to_user_fullname(netif_index, dot1x_detail_info->name)){
		return ASD_FAIL_TO_GET_INTERFACE_NAME;
	}
	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_DETAIL_INFORMATION);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
									&netif_index);	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dot1x_detail_info->security_type);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->encryption_type);
	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->RadiusID);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->keyInputType);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &key);
	strncpy(dot1x_detail_info->SecurityKey, key, sizeof(dot1x_detail_info->SecurityKey)-1);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->port_dot1x_ctrl);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->eap_reauth_ctrl);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->reauth_period);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->guest_vlan_period);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->vlan_state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->auth_state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->guest_vlan_id);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->auth_fail_vlan_id);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->quiet_Period);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->tx_period);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->supp_timeout);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->serv_timeout);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->max_retrans);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->max_users);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->extensible_auth);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->pre_auth);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &dot1x_detail_info->mac_auth_bypass);
	
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 
	
}



int dcli_asd_show_dot1x_statistics(int netif_index, asd_dot1x_statistics_type *statistics_info)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;
	int ret;
	int i = 0;
	
	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	
	/*now we  send port_index-vlan_id-security to asd .*/
	index = 0;	
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	
	if( npd_netif_index_to_user_fullname(netif_index, statistics_info->name)){
		return ASD_FAIL_TO_GET_INTERFACE_NAME;
	}
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_DOT1X_PORT_STATISTICS_INFORMATION);

	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic (	&iter,
									DBUS_TYPE_UINT32,
									&netif_index);	
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret != ASD_DBUS_SUCCESS){
		return ret;
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&statistics_info->eapol_frames_recv);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eapol_frames_trans);
	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eapol_start_frames_recv);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eapol_logoff_frames_recv);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->last_eapol_frame_version);

	//memcpy(statistics_info->last_eapol_src_mac, mac_addr, MAC_LEN);
	for(i=0;i<MAC_LEN;i++){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &statistics_info->last_eapol_src_mac[i]);
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eap_resp_id_frame_recv);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eap_resp_frame_recv);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eap_req_id_frame_trans);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eap_req_frame_trans);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->invalid_eapol_frame_recv);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &statistics_info->eap_len_err_frames_recv);

	dbus_message_unref(reply);	
	
	return ret; 
	
}


int dcli_asd_config_add_local_user(int boot_flag, unsigned char *username, unsigned char *password)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	int i;

    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(0,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_OBJPATH, OBJPATH);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_INTERFACE, INTEFACENAME);

	dbus_error_init(&err);
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_SECURITY_METHOD_ADD_LOCAL_USER);

	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&boot_flag,
							 DBUS_TYPE_STRING,&username,
							 DBUS_TYPE_STRING,&password,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
    if (NULL == reply) {
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	return ret; 

}


int dcli_asd_config_delete_local_user(unsigned char *username)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	int i;

    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(0,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_OBJPATH, OBJPATH);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_INTERFACE, INTEFACENAME);

	dbus_error_init(&err);
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_SECURITY_METHOD_DELETE_LOCAL_USER);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&username,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
    if (NULL == reply) {
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	return ret; 

}


int dcli_asd_show_dot1x_local_user(unsigned char *qlocal_user ,struct  dcli_local_user *local_user)
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;


    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(0,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_OBJPATH, OBJPATH);
	ReInitDbusPath(0,ASD_DBUS_SECURITY_INTERFACE, INTEFACENAME);

	dbus_error_init(&err);
    query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_SECURITY_SHOW_LOCAL_USER);

	dbus_message_append_args(query,
                             DBUS_TYPE_STRING,&qlocal_user,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
    if (NULL == reply) {
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
    if(ret == 0)
    {
        dbus_message_iter_next(&iter);    
    	dbus_message_iter_get_basic(&iter,&qlocal_user);	
        strcpy(local_user->username,  qlocal_user);
        
    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(qlocal_user));
        strcpy(local_user->password,qlocal_user);

    }
	
	dbus_message_unref(reply);

	return ret; 

}


#ifdef HAVE_CAPWAP_ENGINE
int dcli_asd_wlan_isolation_configuration(unsigned int wlan_id, unsigned int status)
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
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
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WLAN_ISOLATION_STATUS);
														
	dbus_message_append_args(query, 
							DBUS_TYPE_UINT32,&wlan_id,
							DBUS_TYPE_UINT32,&status,
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




int man_asd_get_wlan_isolation_configuration(unsigned int wlan_id, unsigned int *isolation)
{

	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusMessage *query; 
	DBusError err;
	int ret;

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	
	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_GET_WLAN_ISOLATION_CONFIGURATION);
	
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&wlan_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ASD_RETURN_CODE_SUCCESS == ret){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,isolation);
	}
	dbus_message_unref(reply);
	
	return ret; 
	
}
#endif



#endif
