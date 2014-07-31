
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAVE_PORTAL
#include <stdio.h>
#include <string.h>

#include <dbus/dbus.h>
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/asd/ASDDbusDef.h"
#include "dbus/wcpss/ACDbusDef.h"
#include "vtysh/vtysh.h"

#include "command.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "lib/netif_index.h"

#include "man_wtp.h"
#include "man_portal.h"
#include "man_security.h"
#include "man_str_parse.h"

extern DBusConnection *config_dbus_connection;
extern DBusConnection *dcli_dbus_connection;

int man_portal_config_enable(unsigned char isEnable)
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_PORTAL);
	
	dbus_message_iter_init_append (query, &iter);

	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_BYTE,
										& isEnable);
		
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

int man_portal_conf_get(dcli_asd_portal_info *info)
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int num = 0, i;
	char *url;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_GET_PORTAL);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	
	dbus_message_unref(query);
	
	if((ret =dcli_check_reply(reply, err))){
		return ret;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ASD_RETURN_CODE_SUCCESS == ret)
	{
		dbus_message_iter_next(&iter);    
    	dbus_message_iter_get_basic(&iter,&info->enable);
		
		dbus_message_iter_next(&iter);    
    	dbus_message_iter_get_basic(&iter,&num);

		for(i=0;i<num;i++)
		{
			dbus_message_iter_next(&iter);    
    		dbus_message_iter_get_basic(&iter,&info->srv[i].enable);

			dbus_message_iter_next(&iter);    
    		dbus_message_iter_get_basic(&iter,&info->srv[i].ipaddr);

			dbus_message_iter_next(&iter);    
    		dbus_message_iter_get_basic(&iter,&info->srv[i].port);

			dbus_message_iter_next(&iter);    
    		dbus_message_iter_get_basic(&iter,&url);
			strncpy(info->srv[i].URL, url, 256);
		}
	}
	dbus_message_unref(reply);
	return ret;

}

int man_portal_sta_get_by_ip(portal_user_info_s *station, unsigned int *next_addr)
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char *ch;

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_GET_PORTAL_STA);
	
	dbus_message_iter_init_append (query, &iter);

	dbus_message_iter_append_basic (	&iter,
										DBUS_TYPE_UINT32,
										&station->ipaddr);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret != CMD_SUCCESS){
		return ret;
	}

	if(ASD_DBUS_SUCCESS == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&station->state); 
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ch); 
		memset(station->id, 0, NAME_LEN+1);
		strncpy(station->id, ch, NAME_LEN);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(station->ipaddr));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(station->netif_index));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,next_addr);			
	}
	
	return ret;
}

int man_portal_intf_set_portal_server
(
    unsigned char* ifname,
    unsigned int portal_ctrl,
    unsigned int portal_srv_id,
    unsigned int* ipaddr_v4
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    unsigned int ipv4;

    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,      \
                                         NPD_DBUS_INTF_OBJPATH,  \
                                         NPD_DBUS_INTF_INTERFACE, \
                                         NPD_DBUS_INTF_SET_PORTAL_SERVER);                               
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &portal_ctrl,
							 DBUS_TYPE_UINT32, &portal_srv_id,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return INTERFACE_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&op_ret,
                              DBUS_TYPE_UINT32,&ipv4,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }
    *ipaddr_v4 = ipv4;

	return op_ret;
}

int man_portal_show_portal_intf(unsigned int* netif_index, unsigned int* server)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0, i=0, num=0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    DBusMessageIter	 iter;
    
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,      \
                                         NPD_DBUS_INTF_OBJPATH,  \
                                         NPD_DBUS_INTF_INTERFACE, \
                                         NPD_DBUS_INTF_SHOW_PORTAL_INTERFACE);                               
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return INTERFACE_RETURN_CODE_ERROR;
    }

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(NPD_DBUS_SUCCESS == op_ret){		
		
   		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);

        for(i=0;i<num;i++)
        {
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &netif_index[i]);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &server[i]);
        }
	}        

	return op_ret;
}

int man_portal_set_client_bypass(unsigned int enable,unsigned int ipaddr_v4)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;

    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,     \
									     NPD_DBUS_ASD_OBJPATH,  \
									     NPD_DBUS_ASD_INTERFACE, \
									     NPD_DBUS_ASD_SET_PORTAL_CLIENT_BYPASS);                              
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32, &enable,
							 DBUS_TYPE_UINT32, &ipaddr_v4,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return INTERFACE_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&op_ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }

	return op_ret;
}    

int man_portal_show_portal_bypass(unsigned int* bypass_ip)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0, i=0, num=0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    DBusMessageIter	 iter;

    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,     \
									     NPD_DBUS_ASD_OBJPATH,  \
									     NPD_DBUS_ASD_INTERFACE, \
									     NPD_DBUS_ASD_SHOW_PORTAL_CLIENT_BYPASS);      
                             
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return INTERFACE_RETURN_CODE_ERROR;
    }

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(NPD_DBUS_SUCCESS == op_ret)
    {       		
   		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);

        for(i=0;i<num;i++)
        {
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &bypass_ip[i]);
        }
	}        

	return op_ret;
}

int man_sta_new_portal_auth(int sta_ip, char *sta_id, char *sta_passwd)
{
	int sockfd, len;	
	int ret = -1;
	struct timeval recv_timeout;
	WIRED_TableMsg wMsg;	
	struct sockaddr_un local_man, asd_portal_addr;

	if((sockfd = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		return -1;
	}

	memset(&local_man, 0, sizeof(struct sockaddr_un));
	local_man.sun_family = AF_LOCAL;
	sprintf(local_man.sun_path, "/tmp/asd_req.%d", getpid());
	len = strlen(local_man.sun_path)+sizeof(local_man.sun_family);
	unlink(local_man.sun_path);

	memset(&asd_portal_addr, 0, sizeof(struct sockaddr_un));
	asd_portal_addr.sun_family = AF_LOCAL;
	sprintf(asd_portal_addr.sun_path, "/tmp/asd_portal_ctl");

	recv_timeout.tv_sec = 10;
	recv_timeout.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&recv_timeout, sizeof(recv_timeout));

	if(bind(sockfd , (struct sockaddr *)&local_man, len) == -1) 
	{
		goto handle_end;
	}
		
	memset(&wMsg, 0, sizeof(wMsg));
	wMsg.wType = STA_TYPE;
	wMsg.wOp = PORTALAUTH_STA;
	wMsg.u.wSTA.Sta_ip = sta_ip;
	strncpy(wMsg.u.wSTA.sta_id, sta_id, NAME_LEN);
	strncpy(wMsg.u.wSTA.sta_passwd, sta_passwd, NAME_LEN);
	if(sendto(sockfd, &wMsg, sizeof(wMsg), 0, (struct sockaddr *) &asd_portal_addr, sizeof(asd_portal_addr)) < 0){
		goto handle_end;
	}

	memset(&wMsg, 0, sizeof(wMsg));
	ret = recvfrom(sockfd, &wMsg, sizeof(wMsg), 0, NULL, NULL);
	if(ret <= 0)
	{
		ret = -1;
		goto handle_end;
	}
	
	ret = wMsg.result;  
	close(sockfd);
	return ret;
handle_end:
	close(sockfd);
	return ret;
	
}

int man_portal_set_bypass_user_iptables
(
    unsigned int ipv4_addr
)
{
    char command_str[128];
    unsigned char ipaddress[16] = {0};
    int status = 0;

    lib_get_string_from_ip(ipaddress, ipv4_addr);

    memset(command_str, 0, 128);
	sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -D PREROUTING -s %s -p tcp --dport 80 -j ACCEPT", ipaddress);
    status = system(command_str);
    
    memset(command_str, 0, 128);
	sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -I PREROUTING 1 -s %s -p tcp --dport 80 -j ACCEPT", ipaddress);
    status = system(command_str);

    return status;
}

int man_portal_del_bypass_user_iptables
(
    unsigned int ipv4_addr
)
{
    char command_str[128];
    unsigned char ipaddress[16] = {0};

    lib_get_string_from_ip(ipaddress, ipv4_addr);
    
    memset(command_str, 0, 128);
	sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -D PREROUTING -s %s -p tcp --dport 80 -j ACCEPT", ipaddress);

    int status = system(command_str);

    return status;
}
#endif
#ifdef __cplusplus
}
#endif
