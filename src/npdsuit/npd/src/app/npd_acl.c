
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_acl.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		API used in NPD dbus process for ACL module
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.105 $	
*******************************************************************************/
#ifdef HAVE_ACL
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"


extern array_table_index_t* service_policy_index;
extern int eth_port_array_index_to_ifindex(unsigned int netif_index);

extern char* acl_match_show_running_config(char* showStr, int* safe_len);
extern char* acl_action_show_running_config(char* showStr, int* safe_len);
extern int npd_class_map_have_dst_src_ip6(int index);


enum
{
	ACL_GSWITCH_ACLSERVICE_ENABLE = 0x1,
	ACL_GSWITCH_DIFFSERV_ENABLE = 0x2,
	ACL_GSWITCH_VOICEVLAN_ENABLE = 0x4,
	ACL_GSWITCH_SERVICEALL_ENABLE = 0x7
};

db_table_t *npd_acl_global_parm_dbtbl = NULL;

array_table_index_t  *npd_acl_global_parm_arr_index = NULL;


DBusMessage* npd_dbus_acl_rule_show_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	int		total_len = 0;
	int 	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE;
	char*	showStr = NULL;
	char*	cursor = NULL;

	DBusMessage*	reply;
	DBusMessageIter	iter;

	showStr = (char*)malloc(NPD_ACL_RULE_SHOWRUN_CFG_SIZE * 10);
	if(NULL == showStr) 
	{
		syslog_ax_acl_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_ACL_RULE_SHOWRUN_CFG_SIZE * 10);
	cursor = showStr;
	{
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
		cursor = showStr + total_len;
		cursor = acl_rule_show_running_config(cursor, &safe_len);

	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

#ifdef __cplusplus
}
#endif

#endif
