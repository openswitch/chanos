
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_AAA
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <time.h>     
#include <sys/time.h>

#include "command.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"

#include "lib/netif_index.h"
#include "dbus/asd/ASDDbusDef.h"
#include "dbus/wcpss/ACDbusDef.h"
#include "sysdef/npd_sysdef.h"
#include "man_wtp.h"
#include "man_sta.h"

extern	DBusConnection *dcli_dbus_connection;


int man_asd_sta_set_wlan_extern_balance(unsigned char wlan_id,unsigned char type, unsigned int index  )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&type,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("%% failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_unref(reply);

	return ret; 
}


int man_asd_channel_get_next_access_time(unsigned int index, struct dcli_channel_access_time_s *dcli_channel_access_element)
{
	DBusMessage     *query, *reply;
	DBusError       err;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
    DBusMessageIter iter_struct;
	int             ret;
	unsigned char   i=0;
    unsigned char   num;
	unsigned char   BUSNAME[PATH_LEN];
	unsigned char   OBJPATH[PATH_LEN];
	unsigned char   INTEFACENAME[PATH_LEN];
    

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME);
	dbus_error_init(&err);
    dbus_message_append_args(query,
                        DBUS_TYPE_BYTE, &(dcli_channel_access_element->channel),
                        DBUS_TYPE_INVALID
                        );
    
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
	if(ret == 0 )
     {		
		    dbus_message_iter_next(&iter);	
            
			dbus_message_iter_get_basic(&iter,&(dcli_channel_access_element->channel));
		
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_get_basic(&iter,&(dcli_channel_access_element->sta_num));
		
			dbus_message_iter_next(&iter);

			dbus_message_iter_get_basic(&iter,&(dcli_channel_access_element->StaTime));
		}

	dbus_message_unref(reply);
	
	return ret;
	
}


int man_asd_sta_show_radio_info(unsigned int index ,unsigned int wtp_id ,struct  dcli_radio_info_s *radio_info, unsigned int *num1)
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
    DBusMessageIter iter_struct;
    unsigned int radionum = 0;
    unsigned int ret;
	int i;

  
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_UINT32,&radio_info->radioid,
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
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radionum));
        *num1 = radionum;
        
		dbus_message_iter_next(&iter);
		
			dbus_message_iter_get_basic(&iter,&(radio_info->radioid));

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->acc_tms));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_tms));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->repauth_tms));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_success_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_fail_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_invalid_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_timeout_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_refused_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->auth_others_num));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_req_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_resp_num));
	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_invalid_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_timeout_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_refused_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->assoc_others_num));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_request_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_success_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_invalid_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_timeout_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_refused_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->reassoc_others_num));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->identify_request_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->identify_success_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->abort_key_error_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->abort_invalid_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->abort_timeout_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->abort_refused_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->abort_others_num));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->deauth_request_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->deauth_user_leave_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->deauth_ap_unable_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->deauth_abnormal_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->deauth_others_num));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->disassoc_request_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->disassoc_user_leave_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->disassoc_ap_unable_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->disassoc_abnormal_num));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->disassoc_others_num));

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->rx_mgmt_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->tx_mgmt_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->rx_ctrl_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->tx_ctrl_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->rx_data_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->tx_data_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->rx_auth_pkts));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(radio_info->tx_auth_pkts));

	}
   
	dbus_message_unref(reply);
    
	return ret;
}



int man_asd_sta_show_wlan_info(unsigned int index,struct dcli_wlan_info_s  *show_wlan_info)
{	
	
	DBusMessage 		*query, *reply; 
	DBusMessageIter  	iter;
	DBusError 			err;
	unsigned int ret;
	int i;

	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&show_wlan_info->wlan_id,
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
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->rx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->tx_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->rx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->tx_bytes));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_resp_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->sta_assoced_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->reassoc_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->reassoc_success_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_req_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_resp_interim));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_wlan_info->assoc_success_interim));
		
    }
    
	dbus_message_unref(reply);
    
	return ret;
}


int man_asd_sta_show_wtp_info(unsigned int index,struct dcli_wtp_info_s *wtp_info )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	int i;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_info->wtp_id,
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
	if(ret == 0){	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->wtp_total_online_time));	/*	xm0703*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->num_assoc_failure));	/*	xm0703*/
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->acc_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->repauth_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_resp_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_others_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->identify_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->identify_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_key_error_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_others_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_others_num));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_data_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_data_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_auth_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_auth_pkts));


	}

    	dbus_message_unref(reply);

	return ret;
}

int man_asd_sta_show_bss_info(unsigned int index,struct dcli_wtp_info_s *wtp_info )
{
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
    DBusMessageIter iter_struct;
	DBusError err;
	unsigned int ret;
	int bss_num;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID_BSS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_info->wtp_id,
							 DBUS_TYPE_UINT32,&wtp_info->bss_num,
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

	if(ret == 0){

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->wlan_id));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->BSSIndex));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->acc_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_tms));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->repauth_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_fail_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_refused_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->auth_others_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_req_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_resp_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_timeout_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->assoc_others_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_success_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_invalid_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->reassoc_others_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->identify_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->identify_success_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_key_error_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_invalid_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_timeout_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_refused_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->abort_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->deauth_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_request_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_user_leave_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_ap_unable_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_abnormal_num));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->disassoc_others_num));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_mgmt_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_ctrl_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_data_pkts));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_data_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->rx_auth_pkts));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_info->tx_auth_pkts));
		//	dbus_message_iter_next(&iter);	
		//	dbus_message_iter_get_basic(&iter,&(wtp_info->bss_total_past_online_time));
		//	dbus_message_iter_next(&iter);	
		//	dbus_message_iter_get_basic(&iter,&(wtp_info->bss_total_present_online_time));
	}
	dbus_message_unref(reply);

	return ret;
}

int man_asd_sta_show_collect_info_of_all_wtp(unsigned int index , struct dcli_collect_info_of_all_wtp_s *collect_info_of_all_wtp, unsigned int *wtp_num1)
{	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
    DBusMessageIter iter_struct;

	DBusError err;
	unsigned int ret;
	unsigned int wtp_num = 0;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP);

	dbus_error_init(&err);

    dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&collect_info_of_all_wtp->wtp_id,
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

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wtp_num));
    *wtp_num1 = wtp_num;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);
	
	if((ret == 0)&&(wtp_num !=0)){	
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(collect_info_of_all_wtp->wtp_id));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(collect_info_of_all_wtp->assoc_req_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(collect_info_of_all_wtp->reassoc_request_num));
			
			dbus_message_iter_next(&iter_array);		
	}
		
	dbus_message_unref(reply);
    
	return ret;
}


int man_asd_sta_extend_show_sta(unsigned char *mac , unsigned int index, struct dcli_extend_show_sta_s *extend_show_sta)
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	unsigned char mac1[MAC_LEN];
	unsigned int ret;
    
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
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
	if(ret == 0){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->in_addr));
        strncpy(extend_show_sta->ip, extend_show_sta->in_addr, 16);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->snr));	
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->rr));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->tr));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->tp));	

		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->rx_bytes));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->tx_bytes));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->rx_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->tx_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->rtx));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->rtx_pkts));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->err_pkts));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->StaTime));	

		#if 0/*wapi mib*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wapi_version));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->controlled_port_status));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->selected_unicast_cipher[0]));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->selected_unicast_cipher[1]));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->selected_unicast_cipher[2]));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->selected_unicast_cipher[3]));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wpi_replay_counters));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wpi_decryptable_errors));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wpi_mic_errors));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_sign_errors));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_hmac_errors));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_auth_res_fail));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_discard));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_timeout));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_format_errors));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_cert_handshake_fail));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_unicast_handshake_fail));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_show_sta->wai_multi_handshake_fail));	
		#endif
	}	

    dbus_message_unref(reply);
	
	return ret;
    
}

int man_asd_sta_extend_show_wtp_sta(unsigned int wtp_id, unsigned int index ,struct  dcli_extend_show_wtp_sta_s   *extend_wtp_sta)
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);



	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
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
	if(ret == 0){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->acc_tms));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->auth_tms));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->repauth_tms));
				
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->deny_num));

    }

    dbus_message_unref(reply);
    
	return ret;

}

int man_asd_sta_extend_show_bss_sta(unsigned int wtp_id,unsigned int index , struct  dcli_extend_show_wtp_sta_s   *extend_wtp_sta)
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);



	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_EXTEND_SHOW_BSS_STA_STALIST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[0],
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[1],
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[2],
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[3],
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[4],
							 DBUS_TYPE_BYTE,&extend_wtp_sta->mac[5],	
							 DBUS_TYPE_UINT32,&extend_wtp_sta->bssnum,
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
	if(ret == 0){	
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[0]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[1]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[2]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[3]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[4]));	

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mac[5]));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->mode));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->channel));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->rssi));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->nRate));	

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->isPowerSave));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->isQos));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->snr));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->rr));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->tr));	
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->tp));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->rx_pkts));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->tx_pkts));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->rtx));	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->rtx_pkts)); 
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&(extend_wtp_sta->err_pkts)); 
	
	    }
  
	dbus_message_unref(reply);
    
	return ret;
}

int man_asd_sta_kick_sta(char *mac, unsigned int index)
{
	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned char 	mac1[MAC_LEN];
	unsigned int 	ret;
	
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	
	mac1[0] = (unsigned char)mac[0];
	mac1[1] = (unsigned char)mac[1];	
	mac1[2] = (unsigned char)mac[2];	
	mac1[3] = (unsigned char)mac[3];	
	mac1[4] = (unsigned char)mac[4];	
	mac1[5] = (unsigned char)mac[5];	
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_KICKSTA);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac1[0],
							 DBUS_TYPE_BYTE,&mac1[1],
							 DBUS_TYPE_BYTE,&mac1[2],
							 DBUS_TYPE_BYTE,&mac1[3],
							 DBUS_TYPE_BYTE,&mac1[4],
						   	 DBUS_TYPE_BYTE,&mac1[5],
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

int man_asd_sta_show_sta(unsigned char *mac, unsigned int index ,struct dcli_show_sta_s *show_sta)
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned char mac1[MAC_LEN];

	unsigned int ret;
    
    unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&mac[0],
							 DBUS_TYPE_BYTE,&mac[1],
							 DBUS_TYPE_BYTE,&mac[2],
							 DBUS_TYPE_BYTE,&mac[3],
							 DBUS_TYPE_BYTE,&mac[4],
						   	 DBUS_TYPE_BYTE,&mac[5],
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
	if(ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->WlanID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->Radio_G_ID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->BSSIndex));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->Security_ID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->sta_flags));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->pae_state));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->backend_state));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->StaTime));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->sta_traffic_limit)); 
					
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(show_sta->sta_send_traffic_limit));	
		
	}	
    
	dbus_message_unref(reply);

	return ret;
}

int man_asd_sta_show_wireless_sta_summary(unsigned int index ,struct dcli_sta_summary_s *sta_summary, unsigned int *wlanid_len1,unsigned int *wtpid_len1)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array,iter_array1;
	DBusError err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;	
	unsigned int wlanid_len=0;
    unsigned int wtpid_len=0;
	int i=0;

	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, ASD_DBUS_STA_METHOD_STA_SUMMARY);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
		
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}
        
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

        if(0 == ret){
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&sta_summary->total_wireless);

    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&sta_summary->local_roam_count);
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&sta_summary->total_unconnect_count);
    	
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&wlanid_len);
            *wlanid_len1 = wlanid_len;

            dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&wtpid_len);
            *wtpid_len1 = wtpid_len;
        }
        
		dbus_message_unref(reply);
        
		return ret;
}


int man_asd_sta_show_wlan_sta_summary(unsigned int index , struct dcli_sta_summary_s *sta_summary)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;	


	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, ASD_DBUS_STA_METHOD_WLAN_STA_SUMMARY);
	dbus_error_init(&err);

    dbus_message_append_args(query,
				  DBUS_TYPE_UINT32,&sta_summary->wlan_id,
				  DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
		
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

        if(0 == ret)
        {
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&sta_summary->tem_wlanid);
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&sta_summary->tem_wlanid_num);
            
            dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&sta_summary->wlan_id);
        }
		
		dbus_message_unref(reply);
        
		return ret;
}

int man_asd_sta_show_wtp_sta_summary(unsigned int index ,struct dcli_sta_summary_s *sta_summary)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array,iter_array1;
	DBusError err;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;	


	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, ASD_DBUS_STA_METHOD_WTP_STA_SUMMARY);
	dbus_error_init(&err);

    dbus_message_append_args(query,
			      DBUS_TYPE_UINT32,&sta_summary->wtp_id,
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

        if(0 == ret){

            dbus_message_iter_next(&iter);	
    		
    		dbus_message_iter_get_basic(&iter,&sta_summary->tem_wtpid);
    		
    		dbus_message_iter_next(&iter);

            dbus_message_iter_get_basic(&iter,&sta_summary->tem_wtpid_num);
            
            dbus_message_iter_next(&iter);

            dbus_message_iter_get_basic(&iter,&sta_summary->wtp_id);
        }
        
		dbus_message_unref(reply);
        
		return ret;
}

int dcli_asd_show_wireless_sta_list(unsigned char *mac_addr, unsigned int bss_index,  wireless_user_info_s *wireless_user_info)
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;
	unsigned char *ip_addr = NULL, *staID = 0;


	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WIRELESS_STALIST);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&bss_index,
						 DBUS_TYPE_BYTE,&mac_addr[0],
						 DBUS_TYPE_BYTE,&mac_addr[1],
						 DBUS_TYPE_BYTE,&mac_addr[2],
						 DBUS_TYPE_BYTE,&mac_addr[3],
						 DBUS_TYPE_BYTE,&mac_addr[4],
						 DBUS_TYPE_BYTE,&mac_addr[5],
						 DBUS_TYPE_INVALID);
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
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Security_ID));	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->WlanID));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->BSSIndex));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Radio_G_ID));	
	
	wireless_user_info->WTPID = wireless_user_info->Radio_G_ID/L_RADIO_NUM;

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ip_addr);
	strncpy(wireless_user_info->ip_addr, ip_addr, IP_LENGTH);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[0]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[1]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[2]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[3]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[4]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[5]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&staID);	
	memset(wireless_user_info->id, 0, NAME_LEN+1);
	strncpy(wireless_user_info->id, staID, NAME_LEN);
	dbus_message_unref(reply);

	return ret;
	
}	

int dcli_asd_show_wired_sta_list(wired_user_info_s *wired_user_info)
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;
	unsigned char *ch = NULL;
	

	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WIREd_STALIST);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&wired_user_info->netif_index,
						 DBUS_TYPE_UINT16,&wired_user_info->vlan_id,
						 DBUS_TYPE_BYTE,&wired_user_info->mac[0],
						 DBUS_TYPE_BYTE,&wired_user_info->mac[1],
						 DBUS_TYPE_BYTE,&wired_user_info->mac[2],
						 DBUS_TYPE_BYTE,&wired_user_info->mac[3],
						 DBUS_TYPE_BYTE,&wired_user_info->mac[4],
						 DBUS_TYPE_BYTE,&wired_user_info->mac[5],
						 DBUS_TYPE_INVALID);
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

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->netif_index));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->vlan_id));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[0]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[1]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[2]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[3]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[4]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wired_user_info->mac[5]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ch);
	strncpy(wired_user_info->ip_addr, ch, IP_LENGTH -1);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ch);	
	memset(wired_user_info->id, 0, NAME_LEN+1);
	strncpy(wired_user_info->id, ch, NAME_LEN);

	dbus_message_unref(reply);
	return ret;
}

int dcli_asd_show_wireless_sta_list_by_wlan_id(int wlan_id, wireless_user_info_s *wireless_user_info)
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;
	unsigned char *ip_addr = 0;


	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WIRELESS_STALIST_BY_WLAN);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&wireless_user_info->BSSIndex,
						 DBUS_TYPE_UINT32,&wlan_id,
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[0],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[1],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[2],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[3],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[4],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[5],
						 DBUS_TYPE_INVALID);
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
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Security_ID));	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->WlanID));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->BSSIndex));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Radio_G_ID));	
	
	wireless_user_info->WTPID = wireless_user_info->Radio_G_ID/L_RADIO_NUM;

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ip_addr);
	memcpy(wireless_user_info->ip_addr, ip_addr, IP_LENGTH);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[0]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[1]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[2]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[3]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[4]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[5]));	
	
	dbus_message_unref(reply);

	return ret;
	
}	


int dcli_asd_show_wireless_sta_list_by_wtpid(int wtp_id, wireless_user_info_s *wireless_user_info)
{	
	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	unsigned int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	unsigned int ret;
	unsigned char *ip_addr = 0;


	index = 0;
	ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_WIRELESS_STALIST_BY_WTP);

	dbus_message_append_args(query,
						 DBUS_TYPE_UINT32,&wireless_user_info->BSSIndex,
						 DBUS_TYPE_UINT32,&wtp_id,
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[0],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[1],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[2],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[3],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[4],
						 DBUS_TYPE_BYTE,&wireless_user_info->mac[5],
						 DBUS_TYPE_INVALID);
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
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Security_ID));	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->WlanID));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->BSSIndex));	
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->Radio_G_ID));	
	
	wireless_user_info->WTPID = wireless_user_info->Radio_G_ID/L_RADIO_NUM;

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ip_addr);
	memcpy(wireless_user_info->ip_addr, ip_addr, IP_LENGTH);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[0]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[1]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[2]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[3]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[4]));	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(wireless_user_info->mac[5]));	
	
	dbus_message_unref(reply);

	return ret;
	
}	

int man_asd_add_sta_MAC_filter_list(unsigned char *name, unsigned int index)
{
	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned int 	ret;
	unsigned char 	BUSNAME[PATH_LEN];
	unsigned char 	OBJPATH[PATH_LEN];
	unsigned char 	INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
  
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_ADD_MAC_FILTER_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_filter_list_add_MAC(unsigned char *mac, unsigned char *filter_list_name, unsigned int index)
{
	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned int 	ret;
	unsigned char 	BUSNAME[PATH_LEN];
	unsigned char 	OBJPATH[PATH_LEN];
	unsigned char 	INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
  
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_FILTER_LIST_ADD_MAC);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&filter_list_name,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_filter_list_del_MAC(unsigned char *mac,unsigned char *filter_list_name,unsigned int index)
{
	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned int 	ret;
	unsigned char 	BUSNAME[PATH_LEN];
	unsigned char 	OBJPATH[PATH_LEN];
	unsigned char 	INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
  
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_FILTER_LIST_DEL_MAC);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&filter_list_name,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_del_sta_MAC_filter_list(unsigned char *name, unsigned int index)
{
	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned int 	ret;
	unsigned char 	BUSNAME[PATH_LEN];
	unsigned char 	OBJPATH[PATH_LEN];
	unsigned char 	INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
  
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_DEL_MAC_FILTER_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_sta_wlan_del_MAC_list(unsigned char *mac, unsigned char wlan_id ,	unsigned char list_type, unsigned int index )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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


int man_asd_sta_wlan_use_MAC_list(unsigned char wlan_id,unsigned char *name,unsigned char list_type, unsigned int index )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_sta_wlan_unuse_MAC_list(unsigned char wlan_id,unsigned char list_type, unsigned int index )
{

	DBusMessage 		*query, *reply;	
	DBusMessageIter		iter;
	DBusError 			err;
	unsigned int 	ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WLAN_UNUSE_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&wlan_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_sta_wtp_add_MAC_list(unsigned char *mac, unsigned int wtp_id, unsigned char list_type, unsigned int index )
{

	DBusMessage 		*query, *reply;	
	DBusMessageIter	 	iter;
	DBusError 			err;
	unsigned int ret;
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_sta_wtp_del_MAC_list(unsigned char *mac, unsigned int wtp_id ,unsigned char list_type ,unsigned int index)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	unsigned int ret;
    
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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


int man_asd_sta_wtp_use_MAC_list(unsigned int wtp_id,	unsigned char list_type, unsigned int index )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
		
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_id,
							DBUS_TYPE_BYTE,&list_type,
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


int man_asd_sta_bss_add_MAC_list(unsigned char *mac,unsigned int radio_id , unsigned char bss_id , unsigned char list_type, unsigned int index )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;	
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
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

int man_asd_sta_bss_del_MAC_list(unsigned char *mac,unsigned int radio_id , unsigned char bss_id , unsigned char list_type, unsigned int index)
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_BYTE,&list_type,
							DBUS_TYPE_BYTE,&mac[0],
							DBUS_TYPE_BYTE,&mac[1],
							DBUS_TYPE_BYTE,&mac[2],
							DBUS_TYPE_BYTE,&mac[3],
							DBUS_TYPE_BYTE,&mac[4],
							DBUS_TYPE_BYTE,&mac[5],
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


int man_asd_sta_bss_use_MAC_list(unsigned int radio_id , unsigned char bss_id , unsigned char list_type, unsigned int index )
{

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_BYTE,&list_type,
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

int man_asd_sta_show_mac_filter_list_acl(unsigned char *name,int filter_list_id,unsigned int index ,struct dcli_MAC_filter_list *dcli_filter_list )
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	iter_array;
    DBusError err;
	unsigned int ret;
	int i=0, j=0;
	unsigned char addr[MAX_FILTER_LIST_MAC_NUMBER*MAC_LEN] = {0};
    unsigned char *strcp = NULL;
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,	\
							ASD_DBUS_STA_METHOD_SHOW_MAC_FILTER_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_UINT32,&filter_list_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("show WLAN %d MAC list failed get reply.\n",dcli_filter_list->wlan_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

    if(ASD_DBUS_SUCCESS == ret)
    {
    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&strcp);	
		strncpy(dcli_filter_list->list_name, strcp, DEFAULT_LEN);
		
    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(dcli_filter_list->mac_num));

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(dcli_filter_list->maclist_id));

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(dcli_filter_list->wlan_num));        

    	dbus_message_iter_next(&iter);
    	dbus_message_iter_recurse(&iter,&iter_array);
    	for (i = 0; i < dcli_filter_list->wlan_num; i++) 
    	{
    		DBusMessageIter iter_struct;
    		dbus_message_iter_recurse(&iter_array,&iter_struct);
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->used_wlan_id[i]);
    		dbus_message_iter_next(&iter_array);
    	}
        
        dbus_message_iter_next(&iter);
    	dbus_message_iter_recurse(&iter,&iter_array);
    	for (i = 0; i < dcli_filter_list->mac_num; i++) 
    	{
    		DBusMessageIter iter_struct;
    		dbus_message_iter_recurse(&iter_array,&iter_struct);
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[0]);
    		dbus_message_iter_next(&iter_struct); 
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[1]);
    		dbus_message_iter_next(&iter_struct); 
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[2]);
    		dbus_message_iter_next(&iter_struct); 
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[3]);
    		dbus_message_iter_next(&iter_struct); 
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[4]);
    		dbus_message_iter_next(&iter_struct); 
    		dbus_message_iter_get_basic(&iter_struct, &dcli_filter_list->macaddr_list[i].addr[5]);
    		dbus_message_iter_next(&iter_array); 
    	}

    }
    
    dbus_message_unref(reply);
  
	return ret; 
}

int man_asd_sta_show_wlan_maclist_acl(unsigned int index ,struct dcli_MAC_filter_list *wlan_MAC_list )
{	
	DBusMessage 		*query, *reply;
	DBusMessageIter  	iter;
	DBusError 			err;
	unsigned int ret;
	int i=0, j=0;
	unsigned char *strcp = NULL;
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,	\
							ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wlan_MAC_list->wlan_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show WLAN %d MAC list failed get reply.\n",wlan_MAC_list->wlan_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

    if(ASD_DBUS_SUCCESS == ret)
    {
    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->maclist_policy));
		
    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&strcp);	
		strncpy(wlan_MAC_list->list_name,strcp,DEFAULT_LEN);
		
    }
    dbus_message_unref(reply);
  
	return ret; 
}

int man_asd_sta_show_wlan_black_MAC(unsigned int index ,  struct dcli_wlan_MAC_list_s *wlan_MAC_list )
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;	
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];


	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,	\
							ASD_DBUS_STA_METHOD_SHOW_WLAN_BLACK_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wlan_MAC_list->wlan_id,
							DBUS_TYPE_UINT32,&wlan_MAC_list->maclist_acl,
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[0],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[1],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[2],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[3],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[4],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan %d mac list failed get reply.\n",wlan_MAC_list->wlan_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

    if(ASD_DBUS_SUCCESS == ret)
    {

    	dbus_message_iter_next(&iter);	

    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[0]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[1]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[2]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[3]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[4]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[5]));	
        
    }
	dbus_message_unref(reply);

	return ret; 
}

int man_asd_sta_show_wlan_white_MAC(unsigned int index ,  struct dcli_wlan_MAC_list_s *wlan_MAC_list )
{	
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
    DBusError err;
	unsigned int ret;
	
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,	\
							ASD_DBUS_STA_METHOD_SHOW_WLAN_WHITE_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wlan_MAC_list->wlan_id,
							DBUS_TYPE_UINT32,&wlan_MAC_list->maclist_acl,
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[0],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[1],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[2],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[3],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[4],
							DBUS_TYPE_BYTE,&wlan_MAC_list->mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan %d mac list failed get reply.\n",wlan_MAC_list->wlan_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
    if(ASD_DBUS_SUCCESS == ret){
    	dbus_message_iter_next(&iter);	

    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[0]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[1]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[2]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[3]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[4]));	

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter,&(wlan_MAC_list->mac[5]));	
    }
    	
	dbus_message_unref(reply);

	return ret; 
}


int man_asd_sta_show_wtp_maclist_acl(unsigned int index , struct dcli_wtp_MAC_list_s *wtp_MAC_list)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array,iter_struct;
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,		\
							ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_MAC_list->wtp_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp %d mac list failed get reply.\n",wtp_MAC_list->wtp_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

        if(ASD_DBUS_SUCCESS == ret)
        {
    	
    	    dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->maclist_acl));	
    		
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->num[MAC_LIST_MODE_BLACK]));	
    		
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->num[MAC_LIST_MODE_WHITE]));	
        }
	
	dbus_message_unref(reply);

	return ret;	
}

int man_asd_sta_show_wtp_black_MAC(unsigned int index ,  struct dcli_wtp_MAC_list_s *wtp_MAC_list)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
		
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,		\
							ASD_DBUS_STA_METHOD_SHOW_WTP_BLACK_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_MAC_list->wtp_id,
							DBUS_TYPE_UINT32,&wtp_MAC_list->maclist_acl,
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[0],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[1],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[2],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[3],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[4],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp %d mac list failed get reply.\n",wtp_MAC_list->wtp_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}
	
    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&ret);

        if(ASD_DBUS_SUCCESS == ret)
        {
        	dbus_message_iter_next(&iter);	

        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[0]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[1]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[2]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[3]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[4]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[5]));	
        }

	
	dbus_message_unref(reply);

	return ret;	
}

int man_asd_sta_show_wtp_white_MAC(unsigned int index ,  struct dcli_wtp_MAC_list_s *wtp_MAC_list)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
		
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,		\
							ASD_DBUS_STA_METHOD_SHOW_WTP_WHITE_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&wtp_MAC_list->wtp_id,
							DBUS_TYPE_UINT32,&wtp_MAC_list->maclist_acl,
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[0],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[1],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[2],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[3],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[4],
							DBUS_TYPE_BYTE,&wtp_MAC_list->mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wtp %d mac list failed get reply.\n",wtp_MAC_list->wtp_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}
	
    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&ret);
        if(ASD_DBUS_SUCCESS == ret)
        {
        	dbus_message_iter_next(&iter);	

        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[0]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[1]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[2]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[3]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[4]));	

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&(wtp_MAC_list->mac[5]));	
        }

	
	dbus_message_unref(reply);

	return ret;	
}


int man_asd_sta_show_radio_bss_maclist_acl(unsigned int index ,unsigned int radio_id, unsigned char bss_id, struct dcli_radio_bss_MAC_list_s *radio_bss_MAC_list)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
    DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
    
	DBusError err;
	unsigned int ret;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
		
	
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME ,\
							ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		    printf("show radio %d bss %d wlan mac list failed get reply.\n",radio_id,bss_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}
    
        dbus_message_iter_init(reply,&iter);
        dbus_message_iter_get_basic(&iter,&ret);

        if(0 == ret)
        {
            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->maclist_acl));	

            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->num[MAC_LIST_MODE_BLACK]));	

            dbus_message_iter_next(&iter);	
            dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->num[MAC_LIST_MODE_WHITE]));	
        }
        dbus_message_unref(reply);
       
	return ret;	
}

int man_asd_sta_show_bss_MAC(unsigned int index ,unsigned int radio_id, unsigned char bss_id, struct dcli_radio_bss_MAC_list_s *radio_bss_MAC_list)
{	
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int ret;

	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);
		
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME ,\
							ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&radio_id,
							DBUS_TYPE_BYTE,&bss_id,
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[0],
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[1],
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[2],
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[3],
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[4],
							DBUS_TYPE_BYTE,&radio_bss_MAC_list->mac[5],
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show radio %d bss %d wlan mac list failed get reply.\n",radio_id,bss_id);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ASD_DBUS_FAIL_TO_GET_REPLY;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

    if(0 == ret){
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[0]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[1]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[2]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[3]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[4]));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(radio_bss_MAC_list->mac[5]));	
				
	}
	
	dbus_message_unref(reply);

	return ret;	
}

struct WtpStaInfo* show_sta_info_of_all_wtp(DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{

	wireless_user_info_s 	wireless_user_info;
	man_ap_conf_t ap_conf;
	unsigned int 	wireless_sta_num = 0;
	unsigned int 	wired_sta_num = 0; 
	unsigned int 	bss_index = 0, man_ret = 0;
	unsigned char 	mac_addr[MAC_LEN] = {0};
	unsigned int    varlen = 0;

	struct WtpStaInfo * StaHead = NULL;
	struct WtpStaInfo * StaNode = NULL;
	struct WtpStaInfo * StaTail = NULL;

	memset(&wireless_user_info, 0, sizeof(wireless_user_info));
	while(1){	
		bss_index = wireless_user_info.BSSIndex;
		memcpy(mac_addr, wireless_user_info.mac, MAC_LEN);
		
		man_ret = dcli_asd_show_wireless_sta_list(mac_addr, bss_index, &wireless_user_info);
		if(man_ret != CMD_SUCCESS) break;

		man_ret = man_wid_wtp_show_specify_wtp(wireless_user_info.WTPID, &ap_conf);
		if(man_ret != CMD_SUCCESS) break;

		if((StaNode=(struct WtpStaInfo*)malloc(sizeof(struct WtpStaInfo)))==NULL)
		{
			dcli_free_wtp_sta_info_head(StaHead);
			*ret=MALLOC_ERROR;		
			return NULL;
		}

		memset(StaNode,0,sizeof(struct WtpStaInfo));
		if(StaHead==NULL){
			StaHead=StaNode;
			StaTail=StaNode;
		}
		else{
			StaTail->next=StaNode;
			StaTail=StaNode;
		}

		if((StaNode->wtpMacAddr=(char*)malloc(20))==NULL)
		{
			dcli_free_wtp_sta_info_head(StaHead);
			*ret=MALLOC_ERROR;		
			return NULL;
		}
		memset(StaNode->wtpMacAddr, 0, 20);
		sprintf(StaNode->wtpMacAddr,"%02X:%02X:%02X:%02X:%02X:%02X",ap_conf.WTPMAC[0],ap_conf.WTPMAC[1],
														ap_conf.WTPMAC[2],ap_conf.WTPMAC[3],
														ap_conf.WTPMAC[4],ap_conf.WTPMAC[5]);

		
		if((StaNode->wtpTerminalMacAddr=(char*)malloc(20))==NULL)
		{
			dcli_free_wtp_sta_info_head(StaHead);
			*ret=MALLOC_ERROR;		
			return NULL;
		}
		memset(StaNode->wtpTerminalMacAddr, 0, 20);
		sprintf(StaNode->wtpTerminalMacAddr,"%02X:%02X:%02X:%02X:%02X:%02X",wireless_user_info.mac[0],wireless_user_info.mac[1],
														wireless_user_info.mac[2],wireless_user_info.mac[3],
														wireless_user_info.mac[4],wireless_user_info.mac[5]);

		StaNode->wtpBelongAPID = wireless_user_info.WTPID;
		StaNode->wtpStaIp = lib_get_ip_from_string(wireless_user_info.ip_addr);
		*wtp_num++;
		
	}


	return StaHead;	
}
int man_asd_sta_logoff(
	unsigned int type,
	unsigned int netif_index,
	unsigned short vlanid,
	unsigned int ipaddr,
	unsigned char *mac
)
{
	DBusMessage 		*query, *reply; 
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int	ret;
	int index = 0;
	
	unsigned char BUSNAME[PATH_LEN];
	unsigned char OBJPATH[PATH_LEN];
	unsigned char INTEFACENAME[PATH_LEN];

	ReInitDbusPath(index,ASD_DBUS_BUSNAME, BUSNAME);
	ReInitDbusPath(index,ASD_DBUS_STA_OBJPATH, OBJPATH);
	ReInitDbusPath(index,ASD_DBUS_STA_INTERFACE, INTEFACENAME);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTEFACENAME,ASD_DBUS_SECURITY_METHOD_FORCE_LOGOFF_STA);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &type,
							 DBUS_TYPE_UINT32, &netif_index,
							 DBUS_TYPE_UINT16, &vlanid,
							 DBUS_TYPE_UINT32, &ipaddr,
							 DBUS_TYPE_BYTE, &mac[0],
							 DBUS_TYPE_BYTE, &mac[1],
							 DBUS_TYPE_BYTE, &mac[2],
							 DBUS_TYPE_BYTE, &mac[3],
							 DBUS_TYPE_BYTE, &mac[4],
							 DBUS_TYPE_BYTE, &mac[5],
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

#endif


