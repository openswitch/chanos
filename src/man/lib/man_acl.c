
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* dcli_acl.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		CLI definition for ACL module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.103 $	
*******************************************************************************/
#ifdef HAVE_ACL
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include <dbus/npd/npd_dbus_def.h>
#include "man_str_parse.h"
#include "man_acl.h"


extern	DBusConnection *config_dbus_connection;

int dcli_str2ulong(char* str, unsigned int* Value)
{
	char* endptr = NULL;
    
	if (NULL == str) 
	{
		return FALSE;
	}
	
	*Value= strtoul(str, &endptr, 10);
	if ('\0' != endptr[0])
	{
		return FALSE;
	}
    
	return TRUE;	
}

int dcli_date_legal_check(int year, int month, int day)
{
    int leep_year = 0;
    year += 1900;
    month += 1;
    if(year < 1900)
        return TIME_RANGE_RETURN_DATE_ILLEGAL;
    if(year%100 == 0)
    {
        if(year%400 == 0)
        {
            leep_year = 1;
        }
    }
    else if(year%4 == 0)
    {
        leep_year = 1;
    }
    else
    {
        leep_year = 0;
    }

    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(day > 31 || day < 1)
                return TIME_RANGE_RETURN_DATE_ILLEGAL;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            if(day < 1 || day > 30)
                return TIME_RANGE_RETURN_DATE_ILLEGAL;
            break;
        case 2:
            if(leep_year)
            {
                if(day > 29 || day < 1)
                    return TIME_RANGE_RETURN_DATE_ILLEGAL;
            }
            else
            {
                if(day > 28 || day < 1)
                    return TIME_RANGE_RETURN_DATE_ILLEGAL;
            }
            break;
    }
    return TIME_RANGE_RETURN_DATE_LEGAL;
}

int map_name_legal_check(char* str,unsigned int len)
{
	int i;
	int ret = -1;
	char c = 0;
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len >= 31){
		ret = -1;
		return ret;
	}

	c = str[0];
	if(	(c=='_')||
		(c<='z'&&c>='a')||
		(c<='Z'&&c>='A')
	  ){
		ret = 0;
	}
	else {
		return -1;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_')
		    ){
			continue;
		}
		else {
			ret = -1;
			break;
		}
	}
	
	return ret;
}

int dcli_acl_check_format_name(const char* str)
{
	int ni = 0;
	int nLen = 0;
	char sName[32];

	nLen = strlen(str);
	if(nLen == 0 || nLen > 31)
	{
		return 1;
	}

	memset(sName, 0, 32);
	strncpy(sName, str, nLen);
	for (ni = 0; ni < nLen; ni++)
	{
		if ((sName[ni] == '_')
            || (sName[ni] >= 'a' && sName[ni] <= 'z')
            || (sName[ni] >= 'A' && sName[ni] <= 'Z')
            || (sName[ni] >= '0' && sName[ni] <= '9'))
		{
			continue;
		}
		else
		{
			return 1;
		}
	}
    
	if (ni == nLen)
	{
		return 0;
	}
	
	return 1;
}

int dcli_acl_parse_ethertype_arg(const char* arg)
{
	char*	endptr = NULL;
	int		value = 0;

	if ('0' == arg[0] && (('x' == arg[1]) || ('X' == arg[1])) && ('\0' != arg[2]))
	{
		value = strtoul(arg, &endptr, 16);
	}
	else
	{
		value = strtoul(arg, &endptr, 10);
	}
    
	if ('\0' != *endptr)
	{
		return -1;
	}

	if (value > 0xffff)
	{
		return -1;
	}

	return value;
}

int dcli_acl_parse_value_arg(const char* arg)
{
	char*	endptr = NULL;
	int		value = 0;

    if (NULL == arg) 
	{
		return -1;
	}

    value = strtoul(arg, &endptr, 10);

	if ('\0' != *endptr)
	{
		return -1;
	}

	return value;
}

int dcli_acl_parse_portno_to_eth_g_index(const char* arg, unsigned int* value)
{
	unsigned int	eth_g_index = 0;
	
	eth_g_index = parse_str_to_eth_index((char*)arg);
	if (0 == eth_g_index) 
	{
		return -1;
	}
	*value = eth_g_index;

	return 0;
}

int	dcli_acl_service_enable(unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &is_enable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_no_acl_rule(unsigned int rule_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_DELETE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &rule_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_create_ingress_group(unsigned int dir_info, unsigned int group_num, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &group_num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_no_ingress_group(unsigned int dir_info, unsigned int group_num, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_DELETE_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &group_num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_unbind_ingress_group(unsigned int dir_info, unsigned int g_index, unsigned int group_num, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ETHPORTS_OBJPATH,
										NPD_DBUS_ETHPORTS_INTERFACE,
										NPD_DBUS_ETHPORTS_METHOD_UNBIND_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_UINT32, &group_num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_ingress_enable(unsigned int dir_info, unsigned int g_index, unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ETHPORTS_OBJPATH,
										NPD_DBUS_ETHPORTS_INTERFACE,
										NPD_DBUS_ETHPORTS_METHOD_CONFIG_ACL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_UINT32, &is_enable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_bind_ingress_group(unsigned int dir_info, unsigned int g_index, unsigned int group_num, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ETHPORTS_OBJPATH,
										NPD_DBUS_ETHPORTS_INTERFACE,
										NPD_DBUS_ETHPORTS_METHOD_BIND_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_UINT32, &group_num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_add_delete_rule_ro_group(unsigned int dir_info, unsigned int op_flag, unsigned int acl_group_index, unsigned int rule_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_ADD_ACL_TO_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &op_flag,
							 DBUS_TYPE_UINT32, &acl_group_index,
							 DBUS_TYPE_UINT32, &rule_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_acl_service_show(unsigned int* is_enable)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_ACL_OBJPATH,
											NPD_DBUS_ACL_INTERFACE,
											NPD_DBUS_METHOD_SHOW_ACL_SERVICE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, is_enable,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_acl_eth_ingress_acl_group_show(unsigned int dir_info, unsigned int node_flag, unsigned int g_index, unsigned int* op_ret, struct xgress_group_g* gress_grp)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;
	unsigned int	ni = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ETHPORTS_OBJPATH,
										NPD_DBUS_ETHPORTS_INTERFACE,
										NPD_DBUS_ETHPORTS_METHOD_SHOW_BIND_ACL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &node_flag,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (0 == node_flag)
	{
		dbus_message_iter_init(reply, &iter);
		dbus_message_iter_get_basic(&iter, op_ret);

		if (ACL_RETURN_CODE_SUCCESS == *op_ret)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter, &gress_grp->g_index);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter, &gress_grp->group_index);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &gress_grp->acl_count);
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter, &iter_array);

			for (ni = 0; ni < gress_grp->acl_count; ni++) 
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array, &iter_struct);			
				dbus_message_iter_get_basic(&iter_struct, &gress_grp->rule_index[ni]);
				dbus_message_iter_next(&iter_array); 
			}
		}
	} 
	
	dbus_message_unref(reply);

	return TRUE;
}

int dcli_acl_ingress_group_show(unsigned int dir_info, unsigned int group_index, unsigned int* op_ret, struct xgress_group_g* gress_grp_g, unsigned int* next_index)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;
	unsigned int	current_index = 0;
	unsigned int	next_index_en = *next_index;;
	unsigned int	ni = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_SHOW_ACL_GROUP_INDEX);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dir_info,
							 DBUS_TYPE_UINT32, &group_index,
							 DBUS_TYPE_UINT32, &next_index_en,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (0 == dir_info)
	{
		dbus_message_iter_init(reply, &iter);
		dbus_message_iter_get_basic(&iter, op_ret);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &current_index);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, next_index);

		if (ACL_RETURN_CODE_SUCCESS == *op_ret)
		{
			gress_grp_g->group_index = current_index;
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &gress_grp_g->acl_count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &gress_grp_g->port_count);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter, &iter_array);

			for (ni = 0; ni < gress_grp_g->port_count; ni++) 
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct, &gress_grp_g->group_bind_port[ni]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_next(&iter_array);
			}

			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (ni = 0; ni < gress_grp_g->acl_count; ni++) 
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct, &gress_grp_g->rule_index[ni]);
				dbus_message_iter_next(&iter_array); 
			}
		}
	} 
	
	dbus_message_unref(reply);

	return TRUE;
}

int dcli_voice_vlan_enable(unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_SWITCH_VOICE_VLAN);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &is_enable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_voice_vlan_config(unsigned int cmd_name_flag, unsigned int vlan_index, unsigned int g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_VOICE_VLAN_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &cmd_name_flag,
							 DBUS_TYPE_UINT32, &vlan_index,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_no_voive_vlan(unsigned int g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_NO_VOICE_VLAN_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_voice_vlan_trust(char trust_flag, unsigned int g_index, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_VOICE_VLAN_TRUST_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &trust_flag,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_voice_vlan_remark_dot1p(unsigned int dot1p_value, unsigned int g_index,  unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_VOICE_VLAN_SET_DOT1P_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &dot1p_value,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_voice_vlan_service_show(unsigned int* is_enable)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_ACL_OBJPATH,
											NPD_DBUS_ACL_INTERFACE,
											NPD_DBUS_ACL_SHOW_VOICE_VLAN_SERVICE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, is_enable,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_voice_vlan_show(unsigned int g_index, unsigned int* op_ret, unsigned int* current_index, struct voice_vlan_s* svoice_vlan)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	unsigned int	next_index_en = *current_index;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_VOICE_VLAN_SHOW_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &next_index_en,
							 DBUS_TYPE_UINT32, &g_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_UINT32, current_index,
		DBUS_TYPE_UINT32, &svoice_vlan->global_enable,
		DBUS_TYPE_BYTE, &svoice_vlan->is_enable,
		DBUS_TYPE_BYTE, &svoice_vlan->is_trust,
		DBUS_TYPE_BYTE, &svoice_vlan->cmd_arg_pri,
		DBUS_TYPE_BYTE, &svoice_vlan->cmd_arg_cosq,
		DBUS_TYPE_BYTE, &svoice_vlan->cmd_arg_dscp,
		DBUS_TYPE_BYTE, &svoice_vlan->cmd_name_flag,
		DBUS_TYPE_UINT32, &svoice_vlan->cmd_arg_value,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_diffserv_enable(unsigned int is_enable, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_CONFIG_DIFFSERV);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &is_enable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int	dcli_diffserv_service_show(unsigned int* is_enable)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError err;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_ACL_OBJPATH,
											NPD_DBUS_ACL_INTERFACE,
											NPD_DBUS_METHOD_SHOW_DIFFSERV_SERVICE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, is_enable,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	return TRUE;
}
int dcli_diffserv_class_map_match_all(char* class_map_name, char ipv6_flag, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_CLASS_MAP_MATCH_ALL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &class_map_name,
							DBUS_TYPE_BYTE, &ipv6_flag, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_diffserv_no_class_map_match_all(char* class_map_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_NO_CLASS_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &class_map_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_diffserv_class_map_match_all_rename(char* class_map_old, char* class_map_new, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_CLASS_MAP_RENAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &class_map_old,
							DBUS_TYPE_STRING, &class_map_new,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_diffserv_match_config(char* map_name, char* match_name, char* data, char* mask, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_MATCH_ADD);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_STRING, &match_name,
								DBUS_TYPE_STRING, &data,
								DBUS_TYPE_STRING, &mask,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}

		return FALSE;
	 }
	
	if (!dbus_message_get_args( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);

	return TRUE;
}

int dcli_diffserv_no_match_config(char* map_name, char* match_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_MATCH_DELETE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_STRING, &match_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);

	return TRUE;	
}

int dcli_diffserv_policy_map(char* policy_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_POLICY_MAP_IN);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &policy_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}

		dbus_message_unref(reply);
		return FALSE;	
	}
	dbus_message_unref(reply);

	return TRUE;	
}

int dcli_diffserv_no_policy_map(char* policy_map, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError	err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_NO_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;	
}

int dcli_diffserv_policy_map_rename(char* policy_name_old, char* policy_map_new, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError	err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_POLICY_MAP_RENAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &policy_name_old,
								DBUS_TYPE_STRING, &policy_map_new,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;	
}

int dcli_diffserv_action_config(char* policy_name, char* action_name, char* data, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError	err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_POLICY_ACTION);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &policy_name,
								DBUS_TYPE_STRING, &action_name,
								DBUS_TYPE_STRING, &data, 
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_no_action_config(char* policy_name, char* action_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_DELETE_POLICY_ACTION);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &policy_name,
								DBUS_TYPE_STRING, &action_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		return FALSE;
	 }
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;	
}

int dcli_diffserv_class_to_policy(char* class_map, char* policy_map, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_CONFIG_CLASS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &class_map,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_no_class_from_policy(char* class_map, char* policy_map, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError	err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_CONFIG_NO_CLASS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &class_map,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_service_policy(unsigned int g_index, char* policy_map, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_SERVICE_POLICY_IN);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &g_index,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_service_policy_all(char* policy_map, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_SERVICE_POLICY_ALL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_no_service_policy(unsigned int g_index, char* policy_map, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_NO_SERVICE_POLICY_IN);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &g_index,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;	
}

int dcli_diffserv_no_service_policy_all(char* policy_map, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_NO_SERVICE_POLICY_ALL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &policy_map,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return FALSE;
	 }

	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	
	dbus_message_unref(reply);
	return TRUE;	
}

int dcli_diffserv_match_cmap_list_show(unsigned int* name_count, char* show_str)
{
	char*			tmp = NULL;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_CMAP_LIST);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, name_count);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);	
	dbus_message_unref(reply);

	return TRUE; 
}

int dcli_diffserv_match_pmap_list_show(unsigned int* name_count, char* show_str)
{
	char*			tmp = NULL;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_PMAP_LIST);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, name_count);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
    
	dbus_message_unref(reply);

	return TRUE; 
}

int dcli_diffserv_cmap_name_show(char* cp_map_name, unsigned int* cp_map_length, char** cp_map_info)
{
	char*			temp_info;
	unsigned int	uni = 0;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_CMAP_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &cp_map_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, cp_map_length);
	for (uni = 0; uni < *cp_map_length; uni++)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &temp_info);
		strncpy(cp_map_info[uni], temp_info, 64);
	}

	dbus_message_unref(reply);
	return TRUE; 
}

int dcli_diffserv_pmap_name_show(char* cp_map_name, unsigned int* cp_map_length, char** cp_map_info)
{
	char*			temp_info;
	unsigned int	uni = 0;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_PMAP_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &cp_map_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, cp_map_length);
	for (uni = 0; uni < *cp_map_length; uni++)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &temp_info);
		strncpy(cp_map_info[uni], temp_info, 33);
	}

	dbus_message_unref(reply);
	return TRUE; 
}

int dcli_diffserv_service_policy_show
(   
    unsigned int g_index, 
    unsigned int* op_ret, 
    unsigned int* name_count, 
    char* show_str, 
    unsigned int flag
)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
    char*           tmp = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_SERVICE_POLICY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &g_index,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, name_count);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
	dbus_message_unref(reply);

	return TRUE; 
}

int dcli_time_range_info_search_name(char *name,unsigned int *op_ret)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_TIME_RANGE_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}

    dbus_message_unref(reply);
    return TRUE;	
}

int dcli_time_range_info_create(char *name,char *start_time,char *end_time,unsigned int flag,unsigned int *op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_TIME_RANGE_ARG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_STRING, &start_time,
								DBUS_TYPE_STRING, &end_time,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}

	dbus_message_unref(reply);
	return TRUE;

}

int dcli_is_acl_associate_time_range_info(char *map_name,unsigned int *op_ret)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_ASSOCIATE_TIME_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}

    dbus_message_unref(reply);
    return TRUE;	
}

int dcli_no_time_range_name(char *time_range_name,unsigned int *op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_NO_TIME_RANGE_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &time_range_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_no_acl_time_range_associate(char* map_name, char* time_range_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_NO_ACL_TIME_RANGE_ASSOCIATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_STRING, &time_range_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_show_time_range_name(unsigned char* time_range_name, char** time_range_info,unsigned int* op_ret, unsigned int *count)
{
	char*			temp_info;
	unsigned int	uni = 0;
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_TIME_RANGE_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &time_range_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, op_ret);

    if(*op_ret == 0)
    	for (uni = 0; uni < 8; uni++)
    	{
    		dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter, &temp_info);
    		strcpy(time_range_info[uni], temp_info);
    	}

	dbus_message_unref(reply);
	return TRUE; 
}
int dcli_show_time_range_bind(unsigned char* time_range_name, char* acl_bind_show_str)
{
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    char *          tmp = NULL;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_TIME_RANGE_BIND);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &time_range_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(acl_bind_show_str, tmp, SHOW_SERVICE_SIZE);
	dbus_message_unref(reply);
	return TRUE; 
}
int dcli_show_time_range(char* show_str)
{
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    char*           tmp = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_TIME_RANGE);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
    
	dbus_message_unref(reply);

	return TRUE; 
}

int dcli_diffserv_vmap_name_show(char* map_name, char * show_str)
{
	DBusMessageIter	iter;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    char *          tmp = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_SHOW_VMAP_NAME);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &map_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &tmp);
    if((NULL != show_str) && (NULL != tmp))
    {
        strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
    }
	dbus_message_unref(reply);
	return TRUE; 
}
int dcli_diffserv_match_l4port_range(
     char *map_name, 
     char *match_name, 
     unsigned int operation, 
     unsigned int protocol,
     unsigned int port0,
     unsigned int port1,
     int *op_ret
     )
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_MATCH_L4PORT_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_STRING, &match_name,
								DBUS_TYPE_UINT32, &operation,
								DBUS_TYPE_UINT32, &protocol,
								DBUS_TYPE_UINT32, &port0,
								DBUS_TYPE_UINT32, &port1,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}

		return FALSE;
	 }
	
	if (!dbus_message_get_args( reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);

	return TRUE;
}
int	dcli_check_policy_route_support(unsigned int* is_support)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_ACL_OBJPATH,
											NPD_DBUS_ACL_INTERFACE,
											NPD_DBUS_ACL_POLICY_ROUTE_SUPPORT);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, is_support,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_acl_group_config(char* acl_group_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_no_acl_group_config(char* acl_group_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_NO_ACL_GROUP_CONFIG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_acl_group_add_desp(char* acl_group_name, char * description, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_ADD_DESP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_STRING, &description,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_acl_group_delete_desp(char* acl_group_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_DELETE_DESP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_acl_group_add_rule(char* acl_group_name, char * map_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_ADD_RULE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_STRING, &map_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}
int dcli_diffserv_acl_group_delete_rule(char* acl_group_name, char * map_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_DELETE_RULE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &acl_group_name,
							DBUS_TYPE_STRING, &map_name,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return FALSE;
	}
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return FALSE;
	}
	dbus_message_unref(reply);
	
	return TRUE;
}

int dcli_diffserv_acl_group_deploy(unsigned int g_index, char* group_name, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_DEPLOY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &g_index,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &group_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}

int dcli_diffserv_acl_group_undeploy(unsigned int g_index, char* group_name, int dir, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_UNDEPLOY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &g_index,
								DBUS_TYPE_UINT32, &dir,
								DBUS_TYPE_STRING, &group_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}
int dcli_diffserv_acl_group_undeploy_all(char* group_name, unsigned int* op_ret)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_ACL_METHOD_ACL_GROUP_UNDEPLOY_ALL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &group_name,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("%% Failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}
	
	if (!dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, op_ret,
		DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		
		dbus_message_unref(reply);
		return FALSE;	
	}
	
	dbus_message_unref(reply);
	return TRUE;
}
int dcli_diffserv_acl_group_show_detail
(    
    char *group_name,  
    char* show_str, 
    unsigned int *op_ret
)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
    char*           tmp = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_METHOD_ACL_GROUP_SHOW_DETAIL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &group_name,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, op_ret);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
	dbus_message_unref(reply);

	return TRUE; 
}
int dcli_diffserv_acl_group_intf_show
(    
    unsigned int  netif_index,  
    char* show_str, 
    unsigned int *op_ret
)
{
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
    char*           tmp = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										   NPD_DBUS_ACL_OBJPATH,
										   NPD_DBUS_ACL_INTERFACE,
										   NPD_DBUS_ACL_METHOD_ACL_GROUP_INTF_SHOW);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return FALSE;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, op_ret);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter, &tmp);
    strncpy(show_str, tmp, SHOW_SERVICE_SIZE);
	dbus_message_unref(reply);

	return TRUE; 
}
#ifdef __cplusplus
}
#endif
#endif

