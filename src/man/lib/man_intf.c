
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *

* dcli_intf.c
*
*
* CREATOR:
*       zhengzw@autelan.com
*
* DESCRIPTION:
*       CLI definition for L3 interface module.
*
* DATE:
*       02/21/2008
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.126 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "dbus/npd/npd_dbus_def.h"
#include "vtysh/vtysh.h"
#include "man_str_parse.h"


#ifdef HAVE_NPD_IPV6
#include "man_ipv6.h"
#endif //HAVE_NPD_IPV6

#include "man_intf.h"



extern DBusConnection *config_dbus_connection;


char vlan_eth_port_ifname [INTERFACE_NAMSIZ] = {0};
char dcli_vty_ifname[INTERFACE_NAMSIZ+1] = {0};

int boot_flag;
struct vtysh_client *vtysh_client_dcli = NULL;
void (*vtyshell_add_entry)(char *show_str);
void (*vtyshell_add_buf)(char *show_buf);
int (*vtyshell_client_execute)(struct vtysh_client *vclient, const char *line, FILE *fp);
int (*vtysh_get_sigint_flag)();

void vtysh_add_show_string(char *showStr) 
{

	(*vtyshell_add_entry)(showStr);
}

void vtysh_add_show_buf(char *showStr)
{
    (*vtyshell_add_buf)(showStr);
}

int vtyshell_get_sigint_flag()
{
	return (*vtysh_get_sigint_flag)();
}

int vtysh_dcli_client_execute (struct vtysh_client *vclient, const char *line, FILE *fp)
{
	return (*vtyshell_client_execute)(vclient, line, fp);
}
void dcli_set_reference(
    int boot_flag_set,
    void (*add_entry)(char *str),
    void (*add_buf)(char *str),
    int (*client_execute)(struct vtysh_client *vclient, const char *line, FILE *fp),
    int (*get_sigint_flag)(),
    struct vtysh_client *vtysh_client_l
    )
{
    boot_flag = boot_flag_set;
    vtyshell_add_entry = add_entry;
    vtyshell_add_buf = add_buf;
	vtysh_client_dcli = vtysh_client_l;
	vtyshell_client_execute = client_execute;
	vtysh_get_sigint_flag = get_sigint_flag;
}

int client_execute_send
(
	char *line,
	FILE *fp,
	unsigned char flag
)
{
	int i = 0;
	int cmd_stat = 0;

	while(vtysh_client_dcli[i].name)
    {
        if((vtysh_client_dcli[i].flag & flag) &&
			(vtysh_client_dcli[i].fd != -1))
        {
        	cmd_stat = vtysh_dcli_client_execute(&vtysh_client_dcli[i], line, fp);

            if (cmd_stat == 1||cmd_stat == -1)
                break;
        }
		i++;
    }

	return cmd_stat;
}

int eth_switchport_exist_check
(
	unsigned int netif_index
)
{
	unsigned int ret = 0;
	unsigned int op_ret = 0;
	unsigned int type = 0;

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH ,	\
							    NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SWITCHPORT_EXIST);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&netif_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return INTERFACE_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
        dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_SWITCHPORT_MODE;
	}
    dbus_message_unref(reply);
	return INTERFACE_RETURN_CODE_SUCCESS;
}

int dcli_create_intf(unsigned int netif_index, unsigned int *ifindex)
{
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    unsigned int netif_index_t = netif_index;

    DBusMessage *query, *reply;
    DBusError err;
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_CREATE_INTF);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&netif_index_t,
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
                              DBUS_TYPE_UINT32,ifindex,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		op_ret = INTERFACE_RETURN_CODE_ERROR;
    }
    dbus_message_unref(reply);
	return op_ret;
}
int dcli_delete_intf(unsigned int netif_index)
{
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    unsigned int netif_index_t = netif_index;
	int ret = 0;
		
    DBusMessage *query, *reply;
    DBusError err;
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_DELETE_INTF);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&netif_index_t,
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
		op_ret = INTERFACE_RETURN_CODE_ERROR;
    }
    dbus_message_unref(reply);
	return op_ret;
}
#ifndef HAVE_ZEBRA
int man_intf_add_ipv4_addr
(
    unsigned char* ifname,
    unsigned int ipv4_addr,
    unsigned int ipv4_mask
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	unsigned int ipv4_addr_t = ipv4_addr;
	unsigned int ipv4_mask_t = ipv4_mask;
	char ipv4_str[20], mask_str[20], command_str[128];
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_ADD_IPV4_ADDR);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &ipv4_addr_t,
							 DBUS_TYPE_UINT32, &ipv4_mask_t,
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
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
		else
		{
			unsigned char *my_ipv4 = ipv4_str;
			unsigned char *my_mask = mask_str;
			memset(ipv4_str, 0, 20);
			memset(mask_str, 0, 20);
			memset(command_str, 0, 128);
			ip_long2str(ipv4_addr_t, &my_ipv4);
			ip_long2str(ipv4_mask_t, &my_mask);
			sprintf(command_str, "sudo /sbin/ifconfig %s %s netmask %s\n", ifname, ipv4_str, mask_str);
			system(command_str);
		}
	}

	return op_ret;
}

int man_intf_del_ipv4_addr
(
    unsigned char* ifname,
    unsigned int ipv4_addr,
    unsigned int ipv4_mask
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	unsigned int ipv4_addr_t = ipv4_addr;
	unsigned int ipv4_mask_t = ipv4_mask;
	char command_str[128];
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_DEL_IPV4_ADDR);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &ipv4_addr_t,
							 DBUS_TYPE_UINT32, &ipv4_mask_t,
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
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
		else
		{
			memset(command_str, 0, 128);
			sprintf(command_str, "sudo /sbin/ifconfig %s 0.0.0.0\n", ifname);
			system(command_str);
		}
	}

	return op_ret;
}
#endif

#ifdef HAVE_DHCP_SNP
int dcli_intf_bind_set
(
    unsigned int state,
    unsigned int netif_index
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    char line[80];
	unsigned int i = 0;
	
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTERFACE_SOUCE_GUARD_SERVICE);
    dbus_error_init(&err);
    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &state,
                             DBUS_TYPE_UINT32, &netif_index,
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
        return IPSG_RETURN_CODE_ERROR;
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
        return IPSG_RETURN_CODE_ERROR;
    }
	else
	{
	   	dbus_message_unref(reply);
	}

	return op_ret;
}

int config_ip_source_guard_entry
(
	unsigned int ipno,
	unsigned int eth_g_index,
	unsigned short vid,
	unsigned char mac[6],
	int isAdd
)
{
    DBusMessage *query, *reply;
    DBusError err;
    /*variables*/
    unsigned int ret = 0;
    unsigned int int_slot_no = 0,int_port_no = 0;
    unsigned int ipmaskLen = 32;
    /*parse argcs*/

    /*appends and gets*/
    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,                   \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_SOUCE_GUARD_ENTRY);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&eth_g_index,
                             DBUS_TYPE_UINT32,&ipno,
                             DBUS_TYPE_UINT16, &vid,
                             DBUS_TYPE_BYTE, &mac[0],
                             DBUS_TYPE_BYTE, &mac[1],
                             DBUS_TYPE_BYTE, &mac[2],
                             DBUS_TYPE_BYTE, &mac[3],
                             DBUS_TYPE_BYTE, &mac[4],
                             DBUS_TYPE_BYTE, &mac[5],
                             DBUS_TYPE_UINT32,&isAdd,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("failed get reply.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return IPSG_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		ret = IPSG_RETURN_CODE_ERROR;
    }

    dbus_message_unref(reply);
    return ret;
}

int man_ip_sg_show_interface_status(unsigned int* ptr_sg_flag_buf)
{
	unsigned int		ret = 0;
    unsigned int        length = 0;
    unsigned int*       ptr_sg_buf = NULL;
	DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
	
	query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_SOUCE_GUARD_SHOW_PORT);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return IPSG_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
        	DBUS_TYPE_UINT32, &ret,
        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_sg_buf, &length,
        	DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return IPSG_RETURN_CODE_ERROR;
	}

    if (0 == ret)
    {
	    memcpy(ptr_sg_flag_buf, ptr_sg_buf, (sizeof(unsigned int) * (MAX_ETH_GLOBAL_INDEX + 1)));
    }

	dbus_message_unref(reply);

	return ret; 
}
#endif

int dcli_intf_shutdown_set
(
    unsigned int intf_ctrl_state,
    unsigned char* ifname,
    FILE *fp

)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    char line[80];
	unsigned int i = 0;
	
    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_SHUTDOWN_SET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &intf_ctrl_state,
                             DBUS_TYPE_STRING, &ifname,
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
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}

int dcli_intf_proxyarp_set
(
    unsigned char* ifname,
    unsigned char proxy_ary
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	
    query = dbus_message_new_method_call(
			                NPD_DBUS_BUSNAME,           \
			                NPD_DBUS_INTF_OBJPATH,          \
			                NPD_DBUS_INTF_INTERFACE,                    \
			                NPD_DBUS_INTF_METHOD_INTF_PROXY_ARP_SET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_BYTE, &proxy_ary,
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
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}

int man_conf_urpf_set
(
    unsigned int urpf_strict
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	
    query = dbus_message_new_method_call(
			                NPD_DBUS_BUSNAME,           \
			                NPD_DBUS_INTF_OBJPATH,          \
			                NPD_DBUS_INTF_INTERFACE,                    \
			                NPD_DBUS_INTF_METHOD_CONF_URPF_SET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &urpf_strict,
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
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}

int man_conf_urpf_get
(
    unsigned int *urpf_strict
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	
    query = dbus_message_new_method_call(
			                NPD_DBUS_BUSNAME,           \
			                NPD_DBUS_INTF_OBJPATH,          \
			                NPD_DBUS_INTF_INTERFACE,                    \
			                NPD_DBUS_INTF_METHOD_CONF_URPF_GET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &urpf_strict,
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
                              DBUS_TYPE_UINT32,urpf_strict,
                              DBUS_TYPE_INVALID))
    
    {
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}

int man_intf_urpf_set
(
    unsigned char* ifname,
    unsigned int urpf_check
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	
    query = dbus_message_new_method_call(
			                NPD_DBUS_BUSNAME,           \
			                NPD_DBUS_INTF_OBJPATH,          \
			                NPD_DBUS_INTF_INTERFACE,                    \
			                NPD_DBUS_INTF_METHOD_INTF_URPF_SET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &urpf_check,
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
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}

int config_ipmc_enable
(
    unsigned char* ifname,
    int enable
)
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
	
    query = dbus_message_new_method_call(
			                NPD_DBUS_BUSNAME,           \
			                NPD_DBUS_INTF_OBJPATH,          \
			                NPD_DBUS_INTF_INTERFACE,                    \
			                NPD_DBUS_INTF_METHOD_CONF_IPMC_SET);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &enable,
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
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    	dbus_message_unref(reply);
        return INTERFACE_RETURN_CODE_ERROR;
    }
	else
	{
	   	dbus_message_unref(reply);
		if(INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
	        return op_ret;
		}
	}

	return op_ret;
}


int dcli_l3intf_get(struct dcli_l3intf_ctrl *ifctrl)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int op_ret = 0,i = 0,j = 0;
	unsigned int ifIndex;
	unsigned char *interfaceName;
	unsigned char proxyarpFlag = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_L3INTF_GET);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &(ifctrl->netif_index),
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return INTERFACE_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(NPD_DBUS_SUCCESS == op_ret){ 	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->netif_index);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&interfaceName);
		memset(ifctrl->ifname,0,INTERFACE_NAMSIZ);
		strncpy(ifctrl->ifname, interfaceName, strlen(interfaceName));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->proxy_arp);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->attribute);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->bind);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->ipmc);
		
	}
	
	return op_ret;
}


int dcli_l3intf_get_next(struct dcli_l3intf_ctrl *ifctrl)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int op_ret = 0,i = 0,j = 0;
	unsigned int ifIndex;
	unsigned char *interfaceName;
	unsigned char proxyarpFlag = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_L3INTF_GET_NEXT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &(ifctrl->netif_index),
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return INTERFACE_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(NPD_DBUS_SUCCESS == op_ret){		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->netif_index);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&interfaceName);
		memset(ifctrl->ifname,0,INTERFACE_NAMSIZ);
		strncpy(ifctrl->ifname, interfaceName, strlen(interfaceName));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->proxy_arp);		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifctrl->attribute);	
	}
	
	return op_ret;
}

int get_arp_next_info
(
	unsigned char flag,
	unsigned int arp_ifIndex,
	unsigned int arp_ipAddr,
	unsigned int arp_ipMask,
	ETHERADDR arp_macAddr,
	struct arp_info *info,
	unsigned int *count
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0,j = 0;
	unsigned int arpCount = 0;
	unsigned int ifIndex;
	unsigned int ipaddr = 0, ipmask = 0;
	unsigned char mac[6] = {0};
	unsigned char ipstring[16] = {0};
	unsigned char isValid = 0,isStatic = 0;
	unsigned char interfaceName[INTERFACE_NAMSIZ];
	unsigned int arpFlag = 0;
	unsigned int info_ifIndex = 0;
	unsigned int info_ipAddr = 0;
	unsigned char info_mac[6] = {0,0,0,0,0,0};
	unsigned int age_time = 0;
    memset(interfaceName, 0, ALIAS_NAME_SIZE);
    info_ifIndex = info->ifindex;
	info_ipAddr = info->ipAddr;
	memcpy(info_mac,info->mac,6);
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_SHOW_ARP_SPECIFY);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag,
							 DBUS_TYPE_UINT32, &arp_ifIndex,
							 DBUS_TYPE_UINT32, &arp_ipAddr,
							 DBUS_TYPE_UINT32, &arp_ipMask,
							 DBUS_TYPE_BYTE, &arp_macAddr.arEther[0],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[1],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[2],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[3],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[4],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[5],
							 DBUS_TYPE_UINT32, &info_ifIndex,
							 DBUS_TYPE_UINT32, &info_ipAddr,
							 DBUS_TYPE_BYTE, &info_mac[0],
                             DBUS_TYPE_BYTE, &info_mac[1],
                             DBUS_TYPE_BYTE, &info_mac[2],
                             DBUS_TYPE_BYTE, &info_mac[3],
                             DBUS_TYPE_BYTE, &info_mac[4],
                             DBUS_TYPE_BYTE, &info_mac[5],
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		return INTERFACE_RETURN_CODE_ERROR;
	}


	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);
	*count = arpCount;

	if(NPD_DBUS_SUCCESS == op_ret){
		memset(info,0,sizeof(struct arp_info));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ipaddr);
		info->ipAddr = ipaddr;
		dbus_message_iter_next(&iter);	
		
		for(j = 0; j< 6; j++) {
 			dbus_message_iter_get_basic(&iter,&mac[j]);
			dbus_message_iter_next(&iter);
		}
		
		memcpy(info->mac,mac,6);

		dbus_message_iter_get_basic(&iter, &arpFlag);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &ifIndex);
		dbus_message_iter_next(&iter);
		info->ifindex = ifIndex;
		if(!(arpFlag & ARPSNP_FLAG_DROP))
		{
			info->ifindex = ifIndex;
		}
		else
			info->ifindex = 0;

		dbus_message_iter_get_basic(&iter,&isStatic);
		dbus_message_iter_next(&iter);
		info->isStatic = isStatic;

		dbus_message_iter_get_basic(&iter,&isValid);
		dbus_message_iter_next(&iter);
		info->isValid = isValid;
		
		dbus_message_iter_get_basic(&iter,&age_time);
		dbus_message_iter_next(&iter);
		info->time = age_time;
		
	}
	dbus_message_unref(reply);
	return op_ret;
}

int clear_arp
(
	unsigned int eth_g_index,
	unsigned int ipAddr,
	unsigned int mask,
	ETHERADDR mac,
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_CLEAR_ARP_FOR_IFINDEX);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
    								DBUS_TYPE_UINT32,&eth_g_index,
    								DBUS_TYPE_UINT32,&ipAddr,
    								DBUS_TYPE_UINT32,&mask,
       							    DBUS_TYPE_BYTE, &mac.arEther[0],
                                    DBUS_TYPE_BYTE, &mac.arEther[1],
                                    DBUS_TYPE_BYTE, &mac.arEther[2],
                                    DBUS_TYPE_BYTE, &mac.arEther[3],
                                    DBUS_TYPE_BYTE, &mac.arEther[4],
                                    DBUS_TYPE_BYTE, &mac.arEther[5],
        							DBUS_TYPE_UINT32,&flag,
        							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = ARP_RETURN_CODE_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int create_ip_static_arp
(
	unsigned int ipno,
	ETHERADDR *macAddr,
	unsigned int eth_g_index
)
{
    DBusMessage *query, *reply;
    DBusError err;
    /*variables*/
    unsigned int ret = 0;
    unsigned int ipmaskLen = 32;
    /*parse argcs*/

    /*appends and gets*/
    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,                   \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_IP_STATIC_ARP);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&eth_g_index,
                             DBUS_TYPE_BYTE,&macAddr->arEther[0],
                             DBUS_TYPE_BYTE,&macAddr->arEther[1],
                             DBUS_TYPE_BYTE,&macAddr->arEther[2],
                             DBUS_TYPE_BYTE,&macAddr->arEther[3],
                             DBUS_TYPE_BYTE,&macAddr->arEther[4],
                             DBUS_TYPE_BYTE,&macAddr->arEther[5],
                             DBUS_TYPE_UINT32,&ipno,
                             DBUS_TYPE_UINT32,&ipmaskLen,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("failed get reply.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return ARP_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		ret = ARP_RETURN_CODE_ERROR;
    }

    dbus_message_unref(reply);
    return ret;
}

#ifdef HAVE_NPD_IPV6

int clear_neighbour
(
	unsigned int eth_g_index,
	man_ip6_addr *ipAddr,
	unsigned int v6_mask_len,
	ETHERADDR mac,
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_CLEAR_NEIGHBOUR_FOR_IFINDEX);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
    								DBUS_TYPE_UINT32,&eth_g_index,
    								DBUS_TYPE_UINT32,&ipAddr->u6_addr32[0],
    								DBUS_TYPE_UINT32,&ipAddr->u6_addr32[1],
    								DBUS_TYPE_UINT32,&ipAddr->u6_addr32[2],
    								DBUS_TYPE_UINT32,&ipAddr->u6_addr32[3],
    								DBUS_TYPE_UINT32,&v6_mask_len,
       							    DBUS_TYPE_BYTE, &mac.arEther[0],
                                    DBUS_TYPE_BYTE, &mac.arEther[1],
                                    DBUS_TYPE_BYTE, &mac.arEther[2],
                                    DBUS_TYPE_BYTE, &mac.arEther[3],
                                    DBUS_TYPE_BYTE, &mac.arEther[4],
                                    DBUS_TYPE_BYTE, &mac.arEther[5],
        							DBUS_TYPE_UINT32,&flag,
        							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NDISC_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		op_ret = NDISC_RETURN_CODE_ERROR;
	}
	
	dbus_message_unref(reply);
	return op_ret;
}

int get_neigh_next_info
(
	unsigned char flag,
	struct neigh_info *info,
	unsigned int *count
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;

	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_METHOD_SHOW_NEIGH_SPECIFY);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag,
							 DBUS_TYPE_UINT32, &info->ifindex,
							 DBUS_TYPE_UINT32, &info->ipAddr.u6_addr32[0],
							 DBUS_TYPE_UINT32, &info->ipAddr.u6_addr32[1],
							 DBUS_TYPE_UINT32, &info->ipAddr.u6_addr32[2],
							 DBUS_TYPE_UINT32, &info->ipAddr.u6_addr32[3],
							 DBUS_TYPE_UINT32, &info->mask_len,
							 DBUS_TYPE_BYTE, &info->mac[0],
                             DBUS_TYPE_BYTE, &info->mac[1],
                             DBUS_TYPE_BYTE, &info->mac[2],
                             DBUS_TYPE_BYTE, &info->mac[3],
                             DBUS_TYPE_BYTE, &info->mac[4],
                             DBUS_TYPE_BYTE, &info->mac[5],
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		return INTERFACE_RETURN_CODE_ERROR;
	}


	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,count);

	if(NPD_DBUS_SUCCESS == op_ret){
		unsigned char j = 0;
		
		//memset(info,0,sizeof(*info));

		for(j = 0; j< 4; j++) {
			dbus_message_iter_next(&iter);	
 			dbus_message_iter_get_basic(&iter,&info->ipAddr.u6_addr32[j]);
		}
		
		for(j = 0; j< 6; j++) {
			dbus_message_iter_next(&iter);
 			dbus_message_iter_get_basic(&iter,&info->mac[j]);
		}
		

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &info->flags);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &info->ifindex);
		if(info->flags & NDISCSNP_FLAG_DROP)
		{
			info->ifindex = 0;
		}

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&info->isStatic);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&info->isValid);
		
	}
	dbus_message_unref(reply);
	return op_ret;
}


int create_ipv6_static_neigh
(
	man_ip6_addr *neigh_ip,
	ETHERADDR *macAddr,
	unsigned int eth_g_index
)
{
    DBusMessage *query, *reply;
    DBusError err;
    /*variables*/
    unsigned int ret = 0;
    unsigned int int_slot_no = 0,int_port_no = 0;
    unsigned int ipmaskLen = 128;
    /*parse argcs*/

    /*appends and gets*/
    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,                   \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_IPV6_STATIC_NEIGH);
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&eth_g_index,
                             DBUS_TYPE_BYTE,&macAddr->arEther[0],
                             DBUS_TYPE_BYTE,&macAddr->arEther[1],
                             DBUS_TYPE_BYTE,&macAddr->arEther[2],
                             DBUS_TYPE_BYTE,&macAddr->arEther[3],
                             DBUS_TYPE_BYTE,&macAddr->arEther[4],
                             DBUS_TYPE_BYTE,&macAddr->arEther[5],
                             DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[0],
                             DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[1],
                             DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[2],
                             DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[3],
                             DBUS_TYPE_UINT32,&ipmaskLen,
                             DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("failed get reply.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return ARP_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		ret = ARP_RETURN_CODE_ERROR;
    }

    dbus_message_unref(reply);
    return ret;
}


int no_ip_static_neighbour
(
	unsigned int eth_g_index,
	man_ip6_addr *neigh_ip,
	unsigned int mask_len,
	ETHERADDR macAddr,
	unsigned int flag
)
{
    DBusMessage *query, *reply;
    DBusError err;
    unsigned int ret = 0;
    

    /*appends and gets*/
    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,                   \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_NO_IP_STATIC_NEIGH);
    dbus_error_init(&err);
    dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[0],
								DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[1],
								DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[2],
								DBUS_TYPE_UINT32,&neigh_ip->u6_addr32[3],
								DBUS_TYPE_UINT32,&mask_len,
       							DBUS_TYPE_BYTE, &macAddr.arEther[0],
                                DBUS_TYPE_BYTE, &macAddr.arEther[1],
                                DBUS_TYPE_BYTE, &macAddr.arEther[2],
                                DBUS_TYPE_BYTE, &macAddr.arEther[3],
                                DBUS_TYPE_BYTE, &macAddr.arEther[4],
                                DBUS_TYPE_BYTE, &macAddr.arEther[5],
								DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("failed get reply.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NDISC_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		ret = NDISC_RETURN_CODE_ERROR;
    }

    dbus_message_unref(reply);
    return ret;
}

int man_ndp_timeout_set
(
	unsigned int timeout,
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SET_NDP_AGETIME);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &timeout,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);

	return ret;
}


int man_ndp_timeout_get
(
	unsigned int *agetime
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int timeout = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SHOW_NDP_AGETIME);

	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &timeout,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}
	else{
        if(ARP_RETURN_CODE_SUCCESS == ret){
            *agetime = timeout;
		}
	}
   

	dbus_message_unref(reply);
	return ret;
}

#endif //HAVE_NPD_IPV6

int no_ip_static_arp
(
	unsigned int eth_g_index,
	unsigned int ipno,
	unsigned int ipmask,
	ETHERADDR macAddr,
	unsigned int flag
)
{
    DBusMessage *query, *reply;
    DBusError err;

    unsigned int ret = 0;

    /*appends and gets*/
    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,                   \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,        \
                                         NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP);
    dbus_error_init(&err);
    dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_UINT32,&ipno,
								DBUS_TYPE_UINT32,&ipmask,
       							DBUS_TYPE_BYTE, &macAddr.arEther[0],
                                DBUS_TYPE_BYTE, &macAddr.arEther[1],
                                DBUS_TYPE_BYTE, &macAddr.arEther[2],
                                DBUS_TYPE_BYTE, &macAddr.arEther[3],
                                DBUS_TYPE_BYTE, &macAddr.arEther[4],
                                DBUS_TYPE_BYTE, &macAddr.arEther[5],
								DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block(config_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("failed get reply.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return ARP_RETURN_CODE_ERROR;
    }

    if (!dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&ret,
                              DBUS_TYPE_INVALID))
    
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
		ret = ARP_RETURN_CODE_ERROR;
    }

    dbus_message_unref(reply);
    return ret;
}

int transform_dyntostatic_arp
(
	unsigned char flag,
	unsigned int arp_ifIndex,
	unsigned int arp_ipAddr,
	unsigned int arp_ipMask,
	ETHERADDR arp_macAddr

)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,		\
								NPD_DBUS_INTF_DYNTOSTATIC_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &flag,
							 DBUS_TYPE_UINT32, &arp_ifIndex,
							 DBUS_TYPE_UINT32, &arp_ipAddr,
							 DBUS_TYPE_UINT32, &arp_ipMask,
							 DBUS_TYPE_BYTE, &arp_macAddr.arEther[0],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[1],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[2],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[3],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[4],
                             DBUS_TYPE_BYTE, &arp_macAddr.arEther[5],
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}


	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_unref(reply);

	return op_ret;
}

int arp_timeout_set
(
	unsigned int timeout,
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SET_ARP_AGETIME);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &timeout,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);

	return ret;
}


int arp_timeout_get
(
	unsigned int *agetime
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int timeout = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SHOW_ARP_AGETIME);

	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &timeout,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}
	else{
        if(ARP_RETURN_CODE_SUCCESS == ret){
            *agetime = timeout;
		}
	}
   

	dbus_message_unref(reply);
	return ret;
}

int man_arp_drop_set
(
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SET_ARP_DROP);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);

	return ret;
}

int man_arp_drop_get
(
	unsigned int *flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	unsigned int drop_flag = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_INTF_METHOD_SHOW_ARP_DROP);

	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_UINT32, &drop_flag,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}
	else{
        if(ARP_RETURN_CODE_SUCCESS == ret){
            *flag = drop_flag;
		}
	}
   

	dbus_message_unref(reply);
	return ret;
}


int create_intf
(
	char *ifname,	
	FILE *fp
)
{
	size_t sl;
    int ret = 0;
    unsigned int netif_index = 0;
    unsigned int ifindex = 0;
	char line[80] = {0};
    if ((sl = strlen(ifname)) > INTERFACE_NAMSIZ)
    {
        return INTERFACE_RETURN_CODE_IFNAME_LENGTH_ERR;
    }
    if(strncmp("mng", ifname, strlen("mng"))== 0){
		sprintf(dcli_vty_ifname, "%s", ifname);
    }
    else if(strncmp("ctrl", ifname, strlen("ctrl"))== 0)
    {
        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
    }
    else if(strcmp("loopback0", ifname)== 0)
    {
        sprintf(dcli_vty_ifname, "lo");
    }
    else if(strncmp("pimreg", ifname, strlen("pimreg"))== 0)
    {
        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
    }
    else if(strncmp("sit", ifname, strlen("sit"))== 0)
    {
        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
    }
    else if(strncmp("ip6tnl", ifname, strlen("ip6tnl"))== 0)
    {
        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
    }
	else
	{
        netif_index = parse_intf_name_to_netif_index(ifname);
    
        if (netif_index == 0)
        {
            return INTERFACE_RETURN_CODE_IFNAME_NOT_ALLOWED;
        }
		if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
		{
			ret = eth_switchport_exist_check(netif_index);
			if(0 != ret)
			{
				return ret;
			}
		}
		if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
		{
			ret = eth_switchport_exist_check(netif_index);
			if(0 != ret)
			{
				return ret;
			}
		}
        ret = dcli_create_intf(netif_index, &ifindex);
        if (ret != 0)
        {
            return ret;
        }
    	
        dcli_netif_name_convert_to_interface_name(ifname, dcli_vty_ifname);
	}

    sprintf(line,"interface %s",dcli_vty_ifname);

    ret = client_execute_send(line,fp,VTYSH_INTERFACE);
	if(0 != ret)
	{
		return INTERFACE_RETURN_CODE_ERROR;
	}

	return ret;
}

int delete_intf
(
	char *ifname,
	unsigned short vlanID,
	FILE *fp
)
{
	int ret = 0;
    unsigned int netif_index = 0;
    char line[80];
	if(0 == vlanID)
	{
	    netif_index = parse_intf_name_to_netif_index(ifname);
	    if (netif_index == 0)
	    {
			if(strncmp("ctrl", ifname, strlen("ctrl")) == 0)
	        {
		        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
	        }
			
			if(strcmp("loopback0", ifname) == 0)
	        {
	            return INTERFACE_RETURN_CODE_IFNAME_NOT_ALLOWED;
	        }
			if(strncmp("pimreg", ifname, strlen("pimreg")) == 0)
	        {
		        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
	        }
			if(strncmp("sit", ifname, strlen("sit")) == 0)
	        {
		        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
	        }

			if(strncmp("ip6tnl", ifname, strlen("ip6tnl")) == 0)
	        {
		        return INTERFACE_RETURN_CODE_IFNAME_RESERVED;
	        }


			
	        sprintf(line,"no interface %s",dcli_vty_ifname);
	    	ret = client_execute_send(line,fp,VTYSH_INTERFACE);
	        if(ret != 0)
	        {
	            return INTERFACE_RETURN_CODE_ERROR;
	        }
			return ret;
	    }
	}
	else
	{
	    netif_index = npd_netif_vlan_get_index(vlanID);

	    if (netif_index == 0)
	    {
	        return INTERFACE_RETURN_CODE_VLAN_NOTEXIST;
	    }
	}
		
    ret = dcli_delete_intf(netif_index);

    return ret;

}

int config_switchport
(
    char *ifname,
	int mode,
	unsigned int *ifindex
)
{
    int ret = 0;
    char netif_name[IF_NAMESIZE+1];
	unsigned int netif_index = 0;

    dcli_interface_name_convert_to_netif_name(ifname, netif_name);

    netif_index = parse_intf_name_to_netif_index(netif_name);
    if (netif_index == 0)
    {
        return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
    }

    if((NPD_NETIF_ETH_TYPE != npd_netif_type_get(netif_index)) 
	&& (NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(netif_index)))
    {
        return INTERFACE_RETURN_CODE_NOT_SWITCHPORT;
    }
    ret = dcli_delete_intf(netif_index);

    if (ret != 0)
    {
        return ret;
    }
	*ifindex = netif_index;
	mode =ETH_PORT_FUNC_BRIDGE;
	return 0;//dcli_eth_port_mode_config(netif_index,mode);
}

#ifdef HAVE_ARP_INSPECTION
int config_arp_inspection_service
(
	unsigned int isEnable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int ret = 0;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   printf("failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   printf("%s raised: %s", err.name, err.message);
		   dbus_error_free(&err);
	   }
	   return ARP_RETURN_CODE_ERROR;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}   

	dbus_message_unref(reply);

	return ret;
}

int config_arp_inspection_vlan_service
(
	unsigned short vlanId,
	unsigned char isEnable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	/*get vlanid */
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_VLAN_ARP_INSPECTION);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return ret;
}

int set_arp_inspection_trust
(
	unsigned int eth_g_index,
	unsigned int trust_mode
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_TRUST);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &trust_mode,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return ret;
}

int set_arp_inspection_validate_type
(
	char *typestr,
	unsigned int flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned int check_type = 0;
	unsigned int allowzero = 0;
	
	/*get vlanid */
	if (!strncmp(typestr, "src-mac", strlen(typestr))) {
		check_type = 1;
	}
	else if(!strncmp(typestr, "dst-mac", strlen(typestr))) {
		check_type = 2;
	}
	else if(!strncmp(typestr, "ip", strlen(typestr))) {
		check_type = 3;
	}	
	else if(!strncmp(typestr, "allow-zero-ip", strlen(typestr))) {
		check_type = 4;
		allowzero = 1;		
	}	

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_VALIDATE_TYPE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &check_type,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_UINT32, &allowzero,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (!dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return ret;
}

int get_arp_inspection_info
(
	struct arp_inspection_info *info
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned int flag = 0;
	unsigned char status = 0;
	unsigned int allowzero = 0;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_GLOBAL);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_BYTE,   &status,
							DBUS_TYPE_UINT32, &allowzero,
							DBUS_TYPE_INVALID))
	{			
		if (ARP_RETURN_CODE_SUCCESS == ret) {
			info->flag = flag;
			info->status = status;
			info->allowzero = allowzero;
		}
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return ret;
}

int get_arp_inspection_vlan_service_status
(
	unsigned short vlanid,
	unsigned char *flag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0;
	unsigned char status = 0;
	/*get vlanid */

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_VLAN_BY_VID);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_BYTE, &status,
							DBUS_TYPE_INVALID))
	{			
		if (ARP_RETURN_CODE_SUCCESS == ret) {
			*flag = status;
		}
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		ret = ARP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return ret;
}

int get_arp_inspection_vlan_next
(
	unsigned short *vlanid
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = 0;
	unsigned short vid = *vlanid;
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_VLAN);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vid,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &vid);
	if(ARP_RETURN_CODE_SUCCESS == ret){	
		*vlanid = vid;
	}

	dbus_message_unref(reply);	
	return ret;
}

int get_arp_inspection_trust_next
(
	unsigned int *eth_g_index
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = 0;
	unsigned int g_index = *eth_g_index;
	/*get vlanid */
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_INTF_OBJPATH,		   \
							   NPD_DBUS_INTF_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION_SHOW_TRUST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &g_index,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}


	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &g_index);

	if(ARP_RETURN_CODE_SUCCESS == ret)
	{
		*eth_g_index = g_index;
	}
	dbus_message_unref(reply);	
	return ret;
}

int man_ip_arp_inspection_statistics
(
    unsigned int* un_ais_num,
    unsigned int un_ais_valid[],
    unsigned int un_ais_index[],
    unsigned int un_ais_permit[],
    unsigned int un_ais_deny[]
)
{
    DBusMessage*	query = NULL;
	DBusMessage*	reply = NULL;
	DBusError		err;
    int ni = 0;
    unsigned int    length = 0;
    unsigned int*   ptr_ais_valid = NULL;
    unsigned int*   ptr_ais_index = NULL;
    unsigned int*   ptr_ais_permit = NULL;
    unsigned int*   ptr_ais_deny = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_INTF_OBJPATH,
                                         NPD_DBUS_INTF_INTERFACE,
                                         NPD_DBUS_CONFIG_ARP_INSPECTION_STATISTICS);
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
		return -1;
	}
    
	if (!dbus_message_get_args(reply, &err,
            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_ais_valid, &length,
            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_ais_index, &length,
        	DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_ais_permit, &length,
            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &ptr_ais_deny, &length,
        	DBUS_TYPE_INVALID))
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%% %s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return -1;
	}

    for (ni = 0; ni < length; ni++)
    {
        un_ais_valid[ni] = ptr_ais_valid[ni];
        un_ais_index[ni] = ptr_ais_index[ni];
        un_ais_permit[ni] = ptr_ais_permit[ni];
        un_ais_deny[ni] = ptr_ais_deny[ni];
    }
    
    *un_ais_num = length;
    
	dbus_message_unref(reply);
	
	return 0;
}

int man_clear_ip_arp_inspection_statistics()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter iter;

	unsigned int ret = 0;

	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_INTF_OBJPATH,
							   NPD_DBUS_INTF_INTERFACE,
							   NPD_DBUS_CONFIG_ARP_INSPECTION_CLEAR);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (config_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return ARP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);
	return ret;
}
#endif      /* end of HAVE_ARP_INSPECTION macro */

/********check vlan L2 or L3****************************/
int dcli_vlan_intf_check(unsigned short vlan_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError error;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME, \
										NPD_DBUS_INTF_OBJPATH, \
										NPD_DBUS_INTF_INTERFACE, \
										NPD_DBUS_INTF_METHOD_VLAN_INTF_CHECK);
	dbus_error_init(&error);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlan_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(config_dbus_connection, query, -1, &error);
	dbus_message_unref(query);

	if (NULL == reply)
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return INTERFACE_RETURN_CODE_UNKNOWN_ERROR;
	}
	if (!(dbus_message_get_args(reply, &error,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		dbus_message_unref(reply);
		return INTERFACE_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);	
	return op_ret;
}

#if 0
int delete_ip_address(char* ifname, char* ipstring)
{
	struct interface *ifp = if_get_by_vty_index(ifname);

	if(NULL == ifp)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

	return ip_address_install (ifp, ifname, NULL, NULL);
}
#endif
