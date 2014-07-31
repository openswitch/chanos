/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_SFLOW
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_sflow.h"

db_table_t *sflow_intfs_db = NULL;
hash_table_index_t *sflow_intfs_hash = NULL;

db_table_t *sflow_agt_db = NULL;
array_table_index_t *sflow_agt_array = NULL;

db_table_t *sflow_clt_db = NULL;
array_table_index_t *sflow_clt_array = NULL;

unsigned int sflow_intf_hash_key(void *data)
{
    sflow_intf_db_t *sflow_intf = (sflow_intf_db_t*)data;
	unsigned int netif_index = sflow_intf->netif_index;
	int hash_index = 0;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
    return hash_index % MAX_ETH_GLOBAL_INDEX;
}

unsigned int sflow_intf_hash_cmp(void *data1, void *data2)
{
	return 1;
}

long sflow_intf_handle_update(void * newdata, void *olddata);
long sflow_intf_handle_insert(void *newdata);

int npd_sflow_init(void)
{
	int ret;

    create_dbtable("sflow_port", MAX_ETH_GLOBAL_INDEX, sizeof(struct sflow_intf_db_s),
        &sflow_intf_handle_update, NULL, &sflow_intf_handle_insert, 
        NULL, NULL, NULL, NULL, 
        NULL, NULL, DB_SYNC_ALL, &sflow_intfs_db);

    dbtable_create_hash_index("sflow_netif_index", sflow_intfs_db, MAX_ETH_GLOBAL_INDEX, 
          &sflow_intf_hash_key, &sflow_intf_hash_cmp,
          &sflow_intfs_hash);

	create_dbtable( "sflow_agent_index", AGENT_NUM, sizeof(struct sflow_agt_db_s),
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
				NULL,
				NULL,
			    DB_SYNC_ALL,
			    &sflow_agt_db);
	ret = dbtable_create_array_index("agent", sflow_agt_db, &sflow_agt_array);

	create_dbtable( "sflow_collector_index", COLLECTOR_NUM, sizeof(struct sflow_clt_db_s),
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
				NULL,
				NULL,
			    DB_SYNC_ALL,
			    &sflow_clt_db);
	ret = dbtable_create_array_index("ollector", sflow_clt_db, &sflow_clt_array);


	//register_netif_notifier(&poe_netif_notifier);
	return NPD_SUCCESS;
}

long sflow_intf_handle_insert(void *newdata)
{
	sflow_intf_db_t *sflow_intf = (sflow_intf_db_t*)newdata;
	int ret = 0, retval = 0;

	ret = nam_sflow_port_dir_enable_set(sflow_intf->netif_index, sflow_intf->is_enable);
	if (0 != ret)
		retval = -1;
	
	ret = nam_sflow_port_dir_ratio_set(sflow_intf->netif_index, sflow_intf->sampling_rate);
	if (0 != ret)
		retval = -1;

	return retval;
}

long sflow_intf_handle_update(void * newdata, void *olddata)
{
	sflow_intf_db_t *new_sflow_intf = (sflow_intf_db_t *)newdata;
    sflow_intf_db_t *old_sflow_intf = (sflow_intf_db_t *)olddata;
    int ret = 0, retval = 0;

	if (new_sflow_intf->is_enable != old_sflow_intf->is_enable) {
		ret = nam_sflow_port_dir_enable_set(new_sflow_intf->netif_index, new_sflow_intf->is_enable);
		if (0 != ret)
			retval = -1;
	}
	
	if (new_sflow_intf->sampling_rate != old_sflow_intf->sampling_rate) {
		ret = nam_sflow_port_dir_ratio_set(new_sflow_intf->netif_index, new_sflow_intf->sampling_rate);
		if (0 != ret)
			retval = -1;
	}

	return retval;
}

DBusMessage * npd_dbus_sflow_config_port_endis(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int is_enable = 0;	
	unsigned int netif_index = 0;
	int ret = 0;
	sflow_intf_db_t npd_sflow_cfg_get = {0}; 

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error, \
								DBUS_TYPE_UINT32, &netif_index, \
								DBUS_TYPE_UINT32, &is_enable, \
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

   	npd_sflow_cfg_get.netif_index = netif_index;
	ret = dbtable_hash_search(sflow_intfs_hash, &npd_sflow_cfg_get, NULL, &npd_sflow_cfg_get);
	
	if(ret != 0) { 
		if(is_enable == 0)
			goto retcode;
		
		npd_sflow_cfg_get.is_enable = is_enable;
		npd_sflow_cfg_get.sampling_rate = SFLOW_DEFAULT_SAMPLING_RATE;
		
		ret = dbtable_hash_insert(sflow_intfs_hash, &npd_sflow_cfg_get);
		if(ret != 0) {
			ret = SFLOW_RETURN_CODE_INSERT_FAIL;
        	goto retcode;
		}
    }
	else {
		npd_sflow_cfg_get.is_enable = is_enable;
		ret = dbtable_hash_update(sflow_intfs_hash, &npd_sflow_cfg_get, &npd_sflow_cfg_get);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_UPDATE_FAIL;
        	goto retcode;
    	}
	}
	
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}

DBusMessage * npd_dbus_sflow_config_samplrate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int rate = 0; 
	unsigned int netif_index = 0;
	int ret = 0;
	
	sflow_intf_db_t npd_sflow_cfg_get;
	memset(&npd_sflow_cfg_get, 0, sizeof(sflow_intf_db_t));
	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error, \
								DBUS_TYPE_UINT32, &netif_index, \
								DBUS_TYPE_UINT32, &rate, \
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error)) {
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	npd_sflow_cfg_get.netif_index = netif_index;
		
	ret = dbtable_hash_search(sflow_intfs_hash, &npd_sflow_cfg_get, NULL, &npd_sflow_cfg_get);
	if(ret != 0) {
		npd_sflow_cfg_get.netif_index = netif_index;
		npd_sflow_cfg_get.is_enable = 0;
		npd_sflow_cfg_get.sampling_rate = rate;
		ret = dbtable_hash_insert(sflow_intfs_hash, &npd_sflow_cfg_get);
		if(ret != 0) {
				ret = SFLOW_RETURN_CODE_INSERT_FAIL;
        		goto retcode;
		}
	}
	else {
		npd_sflow_cfg_get.sampling_rate = rate;
		ret = dbtable_hash_update(sflow_intfs_hash, &npd_sflow_cfg_get, &npd_sflow_cfg_get);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_UPDATE_FAIL;
        	goto retcode;
    	}	
	}
	
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}

DBusMessage * npd_dbus_sflow_config_agent(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned long agt_ip = 0; 
	unsigned int agt_index = 0;
	int ret = 0;
	
	sflow_agt_db_t npd_sflow_agent;
	memset(&npd_sflow_agent, 0, sizeof(sflow_agt_db_t));

	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &agt_ip,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

	ret = dbtable_array_get(sflow_agt_array, agt_index, &npd_sflow_agent);
	if(ret != 0) {	
		npd_sflow_agent.agt_ip = agt_ip;
		npd_sflow_agent.agt_index = agt_index;
		ret = dbtable_array_insert_byid(sflow_agt_array, agt_index, &npd_sflow_agent);
		if(ret != 0){  
			ret = SFLOW_RETURN_CODE_INSERT_FAIL;
        	goto retcode;
    	}	
	}
	else {
		npd_sflow_agent.agt_ip = agt_ip;
		npd_sflow_agent.agt_index = agt_index;
		ret = dbtable_array_update(sflow_agt_array, npd_sflow_agent.agt_index, NULL, &npd_sflow_agent);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_UPDATE_FAIL;
        	goto retcode;
    	}	
	}
		
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}

DBusMessage * npd_dbus_sflow_config_collector(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned long clt_ip = 0; 
	unsigned int port = 0;
	unsigned int clt_index = 0;
	int ret = 0;
	
	sflow_clt_db_t npd_sflow_collector;
	memset(&npd_sflow_collector, 0, sizeof(sflow_clt_db_t));
	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &clt_ip,
								DBUS_TYPE_UINT32, &port,
								DBUS_TYPE_UINT32, &clt_index,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	npd_sflow_collector.clt_index = clt_index - 1;

	ret = dbtable_array_get(sflow_clt_array, npd_sflow_collector.clt_index, &npd_sflow_collector);
	if(ret != 0) {	
		npd_sflow_collector.clt_ip = clt_ip;
		npd_sflow_collector.port = port;
		ret = dbtable_array_insert_byid(sflow_clt_array, npd_sflow_collector.clt_index , &npd_sflow_collector);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_INSERT_FAIL;
        	goto retcode;
    	}	
	}
	else {
		npd_sflow_collector.clt_ip = clt_ip;
		npd_sflow_collector.port = port;
		ret = dbtable_array_update(sflow_clt_array, npd_sflow_collector.clt_index, NULL, &npd_sflow_collector);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_UPDATE_FAIL;
        	goto retcode;
    	}	
	}
		
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}

static char* npd_sflow_agent_show_running_config(char *showStr, int *safe_len)
{
	int totalLen = 0;
	unsigned int agt_index = 0;
	sflow_agt_db_t	npd_sflow_agent;
	int ret;
	char agt_ip[20] = {'\0'};
	char *ipv4 = agt_ip;
	
	memset(&npd_sflow_agent, 0, sizeof(sflow_agt_db_t));
	
	ret = dbtable_array_get(sflow_agt_array, agt_index, &npd_sflow_agent);
	if(ret == 0) {
	    lib_get_string_from_ip(ipv4, (int)npd_sflow_agent.agt_ip);
		totalLen += sprintf(showStr, "sflow agent ip %s\n", agt_ip);
	}
	
	*safe_len = totalLen;
	return showStr;
}

static char* npd_sflow_collector_show_running_config(char *showStr, int *safe_len)
{
	int totalLen = 0;
	int i;
	int ret;
	char tmp[128] = {'\0'};
	char clt_ip[20] = {'\0'};
	sflow_clt_db_t npd_sflow_clt;
	char *ipv4 = clt_ip;
	
	memset(&npd_sflow_clt, 0, sizeof(sflow_clt_db_t));
	
	for(i = 0; i < COLLECTOR_NUM; i++) {
		if(*safe_len < (totalLen + 100))
			break;
		
		bzero(tmp, sizeof(tmp));
   		ret = dbtable_array_get(sflow_clt_array, i, &npd_sflow_clt);
    	if(ret == 0) {
			lib_get_string_from_ip(ipv4, (int)npd_sflow_clt.clt_ip);
        	totalLen += sprintf(tmp, "sflow collector %u ip %s port %u\n", \
								i + 1, clt_ip, npd_sflow_clt.port);
			strcat(showStr, tmp);
		}	
	}

	*safe_len = totalLen;
	return showStr;
}

static char* npd_sflow_port_show_running_config(char *showStr, int *safe_len)
{
	int totalLen = 0;
	int ret;
	sflow_intf_db_t npd_sflow_intf;
	char name[128] = {'\0'};
	char tmp[128] = {'\0'};
	char entry[128] = {'\0'};
	char exit[16] = {'\0'};
	
	memset(&npd_sflow_intf, 0, sizeof(sflow_intf_db_t));
	
	ret = dbtable_hash_head(sflow_intfs_hash, &npd_sflow_intf, &npd_sflow_intf, NULL); 
    while(0 == ret) {
		if (*safe_len < (totalLen + 100))
			break;

		bzero(name, sizeof(name));
		bzero(tmp, sizeof(tmp));
		bzero(entry, sizeof(entry));
		bzero(exit, sizeof(exit));
		
		npd_netif_index_to_user_fullname(npd_sflow_intf.netif_index, entry);
		totalLen += sprintf(name, "interface %s\n", entry);
		strcat(showStr, name);

		if(npd_sflow_intf.is_enable == 1) {
			totalLen += sprintf(tmp, " sflow enable\n");
			strcat(showStr, tmp);
			if(npd_sflow_intf.sampling_rate != SFLOW_DEFAULT_SAMPLING_RATE) {
				bzero(tmp, sizeof(tmp));
				totalLen += sprintf(tmp, " sflow sampling-rate %u\n", npd_sflow_intf.sampling_rate);
    			strcat(showStr, tmp);
			}

			totalLen += sprintf(exit, " exit\n");
			strcat(showStr, exit);
		}
		else {
			bzero(tmp, sizeof(tmp));
			totalLen += sprintf(tmp, " sflow sampling-rate %u\n", npd_sflow_intf.sampling_rate);
    		strcat(showStr, tmp);
			totalLen += sprintf(exit, " exit\n");
			strcat(showStr, exit);
    	}
		ret = dbtable_hash_next(sflow_intfs_hash, &npd_sflow_intf, &npd_sflow_intf, NULL);		
	}

	*safe_len = totalLen;
	return showStr;
}

DBusMessage* npd_dbus_sflow_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage *reply;
	DBusMessageIter	iter;
	int	 total_len = 0;
	int  safe_len = 72 * 1024;
	char *showStr = NULL;
	char *cursor = NULL;

	showStr = (char*)malloc(72 * 1024);
	if(NULL == showStr) 
		return NULL;
	memset(showStr, 0, (72 * 1024));

	cursor = showStr;
	cursor = npd_sflow_agent_show_running_config(cursor, &safe_len);

	total_len += safe_len;
	safe_len = (72 * 1024) - total_len;
	cursor = showStr + total_len;
	cursor = npd_sflow_collector_show_running_config(cursor, &safe_len);

	total_len += safe_len;
	safe_len = (72 * 1024) - total_len;
	cursor = showStr + total_len;
	cursor = npd_sflow_port_show_running_config(cursor, &safe_len);


	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

DBusMessage * npd_dbus_sflow_no_agent(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int del_flag; 
	int ret = 0;
	unsigned int agt_index = 0;
	sflow_agt_db_t npd_sflow_agent;
	memset(&npd_sflow_agent, 0, sizeof(sflow_agt_db_t));
	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &del_flag,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	if(del_flag != 1)
		goto retcode;

	ret = dbtable_array_get(sflow_agt_array, agt_index, &npd_sflow_agent);
	if(ret != 0) {	
		op_ret = SFLOW_RETURN_CODE_NO_SUCH_INTERFACE;
		goto retcode;
	}
	else {
		ret = dbtable_array_delete(sflow_agt_array, agt_index, &npd_sflow_agent);
		if(ret != 0) {
			ret = SFLOW_RETURN_CODE_DELEATE_FAIL;
        	op_ret = ret;
		}
		op_ret = ret;
	}
		
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}

DBusMessage * npd_dbus_sflow_no_collector(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int clt_index = 0;
	int ret = 0;
	
	sflow_clt_db_t npd_sflow_collector;
	memset(&npd_sflow_collector, 0, sizeof(sflow_clt_db_t));
	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &clt_index,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	npd_sflow_collector.clt_index = clt_index - 1;

	ret = dbtable_array_get(sflow_clt_array, npd_sflow_collector.clt_index, &npd_sflow_collector);
	if(ret != 0) {	
		op_ret = SFLOW_RETURN_CODE_NO_SUCH_INTERFACE;
		goto retcode;
	}
	else {
		ret = dbtable_array_delete(sflow_clt_array, npd_sflow_collector.clt_index, &npd_sflow_collector);
		if(ret != 0) {
			ret = SFLOW_RETURN_CODE_DELEATE_FAIL;
        	op_ret = ret;
		}
		op_ret = ret;
	}
		
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}

DBusMessage * npd_dbus_sflow_no_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int rate = 0; 
	unsigned int netif_index = 0;
	int ret = 0;
	
	sflow_intf_db_t npd_sflow_cfg_get;
	memset(&npd_sflow_cfg_get, 0, sizeof(sflow_intf_db_t));
	
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32, &rate,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	npd_sflow_cfg_get.netif_index = netif_index;
		
	ret = dbtable_hash_search(sflow_intfs_hash, &npd_sflow_cfg_get, NULL, &npd_sflow_cfg_get);
	if(ret != 0) {	
		ret = SFLOW_RETURN_CODE_NO_SUCH_INTERFACE;
		goto retcode;
	}

	if(npd_sflow_cfg_get.is_enable != 1) {	
		ret = SFLOW_RETURN_CODE_IS_DISABLE;
		goto retcode;
	}
	else {
		npd_sflow_cfg_get.sampling_rate = rate;
		ret = dbtable_hash_update(sflow_intfs_hash, &npd_sflow_cfg_get, &npd_sflow_cfg_get);
		if(ret != 0) {  
			ret = SFLOW_RETURN_CODE_UPDATE_FAIL;
        	goto retcode;
    	}	
	}
	
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	return reply;
}


DBusMessage* npd_sflow_agent_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage *reply;
	DBusMessageIter	iter;
	int ret = -1;
	
	unsigned long agt_ip = -1;
	unsigned int agt_index = 0;
	
	sflow_agt_db_t npd_sflow_agent;

	memset(&npd_sflow_agent, 0, sizeof(sflow_agt_db_t));
		
	ret = dbtable_array_get(sflow_agt_array, agt_index, &npd_sflow_agent);
	if(ret == 0)
		agt_ip = npd_sflow_agent.agt_ip;

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &agt_ip);	

	return reply;
}

DBusMessage* npd_sflow_collector_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage *reply;
	DBusError error;
	DBusMessageIter	iter;
	unsigned int index;
	unsigned int clt_index; 
	unsigned long clt_ip;
	unsigned int clt_port;
	sflow_clt_db_t npd_sflow_clt;
	int ret = -1;
	
	memset(&npd_sflow_clt, 0, sizeof(sflow_clt_db_t));

	dbus_error_init(&error);
	if(!(dbus_message_get_args( msg, &error,
							    DBUS_TYPE_UINT32, &index,	
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error)) {
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		ret = SFLOW_RETURN_CODE_ERR_GENERAL;
	}

	ret = dbtable_array_get(sflow_clt_array, index - 1, &npd_sflow_clt);
	if(ret == 0) {
		clt_index = npd_sflow_clt.clt_index + 1;
		clt_ip = npd_sflow_clt.clt_ip;
		clt_port = npd_sflow_clt.port;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &clt_index);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &clt_ip);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &clt_port);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32,  &ret);

	return reply;
}

DBusMessage* npd_sflow_global_show_port(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage *reply;
	DBusMessageIter	iter;
	DBusError error;
	int ret = -1;
	int op_ret = -1;
	unsigned int num = 0;
	
	unsigned int netif_index = 0; 
	unsigned int is_enable = 0;
	unsigned int sampling_rate = 0;

	unsigned int ifindex = 0; 
	sflow_intf_db_t npd_sflow_intf;
	memset(&npd_sflow_intf, 0, sizeof(sflow_intf_db_t));

	dbus_error_init(&error);
	if(!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT32, &num,	
								DBUS_TYPE_UINT32, &ifindex,	
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error)) {
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		op_ret = -1;
		goto retcode;
	}
	
	if(num == 0) { //firtst port
		ret = dbtable_hash_head(sflow_intfs_hash, &npd_sflow_intf, &npd_sflow_intf, NULL);
		if(ret == 0) {
			netif_index = npd_sflow_intf.netif_index;
			sampling_rate = npd_sflow_intf.sampling_rate;
			is_enable = npd_sflow_intf.is_enable;
			op_ret = 0;
			goto retcode;
		}
		else {
			op_ret = -1; 
			goto retcode;
		}
	}
	else{ //not first port
		npd_sflow_intf.netif_index = ifindex;
		ret = dbtable_hash_search(sflow_intfs_hash, &npd_sflow_intf, NULL, &npd_sflow_intf);
		if(ret == 0) {
			ret = dbtable_hash_next(sflow_intfs_hash, &npd_sflow_intf, &npd_sflow_intf, NULL);
			if(ret == 0) {
				netif_index = npd_sflow_intf.netif_index;
				sampling_rate = npd_sflow_intf.sampling_rate;
				is_enable = npd_sflow_intf.is_enable;
				op_ret = 0; 
			}
			else {
				op_ret = -1; 
				goto retcode;
			}
		}
		else {
			op_ret = -1; 
			goto retcode;
		}	
	}

retcode:	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &netif_index);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &is_enable);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &sampling_rate);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32,  &op_ret);

	return reply;
}

#ifdef __cplusplus
}
#endif
#endif
