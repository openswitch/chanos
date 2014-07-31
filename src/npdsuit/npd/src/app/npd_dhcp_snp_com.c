
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dhcp_snp_com.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		dhcp snooping common for NPD module.
*
* DATE:
*		06/04/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.8 $	
*******************************************************************************/
#ifdef HAVE_DHCP_SNP
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_dhcp_snp_com.h"

/*********************************************************
*	global variable define											*
**********************************************************/
unsigned int dhcp_snp_global_no =0 ;

extern hash_table_index_t *npd_dhcp_snp_dbhash_mac_index;

extern hash_table_index_t *npd_dhcp_snp_status_item_index;
extern array_table_index_t *npd_dhcp_snp_global_status_index;

/*********************************************************
*	extern variable												*
**********************************************************/
extern unsigned int dhcp_relay_max_hops;

unsigned int npd_dhcp_snp_global_cfg_get
(
	struct NPD_DHCP_SNP_GLOBAL_STATUS *user
);

int npd_dhcp_snp_modify_port_by_trunk(struct NPD_DHCP_SNP_GLOBAL_STATUS* entry, int netif_index);

static char* npd_dhcp_packet_type_str[10] =
{
    "DHCP-RESERVE",
    "DHCP-DISCOVER",
    "DHCP-OFFER",
    "DHCP-REQUEST",
    "DHCP-DECLINE",
    "DHCP-ACK",
    "DHCP-NAK",
    "DHCP-RELEASE",
    "DHCP-INFORM",
    "DHCP-UNKNOWN"
};
 

void npd_dhcp_snp_port_event
(
	unsigned int  netif_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
)
{
    int array_port_index = 0;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT item;
    struct NPD_DHCP_SNP_GLOBAL_STATUS entry;
    
	syslog_ax_dhcp_snp_dbg("npd_dhcp_snp_port_event");
	memset(&item,0,sizeof(item));
    memset(&entry, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
	item.global_port_ifindex = netif_index;
    
	switch(event)
	{
		case PORT_NOTIFIER_LINKDOWN_E:
        {
			syslog_ax_dhcp_snp_dbg("the port netif_index %d has been deleted!\n",netif_index);				
			break;
		}
		case PORT_NOTIFIER_LINKUP_E:
		case PORT_NOTIFIER_L2CREATE:
			break;
		case PORT_NOTIFIER_L2DELETE:
	    case PORT_NOTIFIER_DELETE:
        {
			if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
			{
                unsigned short vlan_id = 0;
				vlan_id = (unsigned short)npd_netif_vlan_get_vid(netif_index);
/*              if (DHCP_SNP_RETURN_CODE_EN_VLAN == npd_dhcp_snp_check_vlan_status(vlan_id))    */
                if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                {
                    if (NPD_VBMP_MEMBER(entry.vlan_admin_status, vlan_id))
                    {
                        NPD_VBMP_VLAN_REMOVE(entry.vlan_admin_status, vlan_id);
                        npd_dhcp_snp_global_cfg_set(&entry);
                        npd_dhcp_snp_lease_delete_by_vlan(vlan_id, NPD_DHCP_SNP_BIND_TYPE_DYNAMIC);
                    	npd_dhcp_snp_lease_delete_by_vlan(vlan_id, NPD_DHCP_SNP_BIND_TYPE_STATIC);
        				syslog_ax_dhcp_snp_dbg("delete bind table on vlan %d delete !\n", vlan_id);
                    }
                }
			}
			else 
			{
                if ((NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
                    && (PORT_NOTIFIER_L2DELETE == event))
                {
                    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                    {
                        array_port_index = netif_array_index_from_ifindex(netif_index);
                        entry.switch_port_control_count[array_port_index] = 0;
                        npd_dhcp_snp_global_cfg_set(&entry);
                    }
                }
				npd_dhcp_snp_lease_delete_by_port(netif_index,NPD_DHCP_SNP_BIND_TYPE_DYNAMIC);	
				npd_dhcp_snp_lease_delete_by_port(netif_index,NPD_DHCP_SNP_BIND_TYPE_STATIC);	
				syslog_ax_dhcp_snp_dbg("the port netif_index %d has been deleted!\n",netif_index);
				npd_dhcp_snp_status_item_delete(&item);	
			}
			break;
	    }
		default:
        {
	        break; 
		}
	}
	syslog_ax_dhcp_snp_dbg("npd_dhcp_snp_port_event end!");
}

void npd_dhcp_snp_relate_event
(
    unsigned int netif_index,
	unsigned int  sub_netif_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
)
{
    int array_port = 0;
    int array_port_index = 0;
	unsigned int vid ;
	unsigned int ret = 0;
	unsigned int netif_type = 0;
	unsigned int sub_netif_type = 0;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT item;
    struct NPD_DHCP_SNP_GLOBAL_STATUS entry;
	memset(&item,0,sizeof(item));
    memset(&entry, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
	netif_type = npd_netif_type_get(netif_index);
	sub_netif_type = npd_netif_type_get(sub_netif_index);
	item.global_port_ifindex = sub_netif_index;
    
	switch (event)
	{
		case PORT_NOTIFIER_JOIN:
        {
			if (netif_type == NPD_NETIF_VLAN_TYPE)
			{
                if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                {
                    array_port_index = netif_array_index_from_ifindex(sub_netif_index);
                    vid = npd_netif_vlan_get_vid(netif_index);
                    if (NPD_VBMP_MEMBER(entry.vlan_admin_status, vid))
                    {
                        entry.switch_port_control_count[array_port_index]++;

                        if (NPD_NETIF_TRUNK_TYPE == sub_netif_type)
                        {
                            npd_dhcp_snp_modify_port_by_trunk(&entry, sub_netif_index);
                        }
                        npd_dhcp_snp_global_cfg_set(&entry);
                    }
                }
			}
			else if ((netif_type == NPD_NETIF_TRUNK_TYPE) && (sub_netif_type == NPD_NETIF_ETH_TYPE))
			{
				npd_dhcp_snp_status_item_delete(&item);
                if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                {
                    array_port = netif_array_index_from_ifindex(netif_index);
                    if (entry.switch_port_control_count[array_port] > 0)
                    {
                        array_port_index = netif_array_index_from_ifindex(sub_netif_index);
                        entry.switch_port_control_count[array_port_index] = entry.switch_port_control_count[array_port];
                        npd_dhcp_snp_global_cfg_set(&entry);
                    }
                }
			}
			break;
		}
		case PORT_NOTIFIER_LEAVE:
        {
			syslog_ax_dhcp_snp_dbg("delete %d items for port %d from binding table!\n",ret,sub_netif_index);
			
			if (netif_type == NPD_NETIF_VLAN_TYPE)
			{
                vid = npd_netif_vlan_get_vid(netif_index);
				ret = npd_dhcp_snp_lease_delete_by_vlan_port(vid, sub_netif_index,NPD_DHCP_SNP_BIND_TYPE_DYNAMIC);
				ret = npd_dhcp_snp_lease_delete_by_vlan_port(vid, sub_netif_index,NPD_DHCP_SNP_BIND_TYPE_STATIC);
                if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                {
                    if (NPD_VBMP_MEMBER(entry.vlan_admin_status, vid))
                    {
                        array_port_index = netif_array_index_from_ifindex(sub_netif_index);
                        if (entry.switch_port_control_count[array_port_index] > 0)
                        {
                            entry.switch_port_control_count[array_port_index]--;
                        }

                        if (NPD_NETIF_TRUNK_TYPE == sub_netif_type)
                        {
                            npd_dhcp_snp_modify_port_by_trunk(&entry, sub_netif_index);
                        }

                        npd_dhcp_snp_global_cfg_set(&entry);
                    }
                }
			}
            else if ((netif_type == NPD_NETIF_TRUNK_TYPE) && (sub_netif_type == NPD_NETIF_ETH_TYPE))
            {
                if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&entry))
                {
                    array_port_index = netif_array_index_from_ifindex(sub_netif_index);
                    entry.switch_port_control_count[array_port_index] = 0;
                    npd_dhcp_snp_global_cfg_set(&entry);
                }
            }
			break;
		}
		default:
        {
			break; 
		}
	}
}


netif_event_notifier_t npd_dhcp_snp_notifier =
{
    .netif_event_handle_f = &npd_dhcp_snp_port_event,
    .netif_relate_handle_f = &npd_dhcp_snp_relate_event
};

/**********************************************************************************
 * npd_dhcp_snp_enable
 *		set DHCP_Snooping enable global status, and init 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_snp_init()
{
	int ret;
	syslog_ax_dhcp_snp_dbg("dhcp snp init!\n");
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
	ret = npd_dhcp_snp_tbl_initialize();
	ret = npd_dhcp_snp_status_item_table_initialize();
	ret = npd_dhcp_snp_global_status_initialize();
	
    register_netif_notifier(&npd_dhcp_snp_notifier);	
	sprintf(user.dhcp_snp_opt82_remoteid_str, "%.2x%.2x%.2x%.2x%.2x%.2x",
	   PRODUCT_MAC_ADDRESS[0], PRODUCT_MAC_ADDRESS[1], PRODUCT_MAC_ADDRESS[2], PRODUCT_MAC_ADDRESS[3],
	   PRODUCT_MAC_ADDRESS[4], PRODUCT_MAC_ADDRESS[5]);
	user.dhcp_snp_opt82_format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX;
	user.dhcp_snp_opt82_fill_format_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT;
	user.dhcp_snp_opt82_remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
    if (0 != npd_dhcp_snp_self_sock_init())
    {
        syslog_ax_dhcp_snp_dbg("dhcp snp self sock init failed!\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }
	nam_thread_create("NpdDhcpSnp", npd_dhcp_snp_timer_thread, NULL,NPD_TRUE,NPD_FALSE);
	if (0 != dbtable_array_insert(npd_dhcp_snp_global_status_index,&dhcp_snp_global_no, &user))
	{
		syslog_ax_dhcp_snp_dbg("dhcp snp init failed!\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	
	syslog_ax_dhcp_snp_dbg("dhcp snp init finish!\n");
	return DHCP_SNP_RETURN_CODE_OK;
}


unsigned int npd_dhcp_snp_global_cfg_get
(
	struct NPD_DHCP_SNP_GLOBAL_STATUS *user
)
{
    return (0 == dbtable_array_get(npd_dhcp_snp_global_status_index, dhcp_snp_global_no, user)) \
        ? DHCP_SNP_RETURN_CODE_OK : DHCP_SNP_RETURN_CODE_ERROR;
}

unsigned int npd_dhcp_snp_global_cfg_set
(
	struct NPD_DHCP_SNP_GLOBAL_STATUS *user
)
{
    return (0 == dbtable_array_update(npd_dhcp_snp_global_status_index, dhcp_snp_global_no, NULL, user)) \
        ? DHCP_SNP_RETURN_CODE_OK : DHCP_SNP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 * npd_dhcp_snp_check_global_status
 *		check DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_ENABLE_GBL		- global status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	- global status is disable
 **********************************************************************************/
unsigned int npd_dhcp_snp_check_global_status()
{
	
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
    {
        syslog_ax_dhcp_snp_dbg("user.dhcp_snp_enable = %d\n", user.dhcp_snp_enable);
		if (user.dhcp_snp_enable == NPD_DHCP_SNP_ENABLE)
		{
			return DHCP_SNP_RETURN_CODE_ENABLE_GBL;
		}
		else
        {
			return DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
		}
    }
    else
    {
        return DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
    }

    return DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
}


/**********************************************************************************
 * npd_dhcp_snp_get_global_status
 *		get DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned char *status
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_snp_get_global_status
(
	unsigned char *status
)
{

	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
    {
        *status = user.dhcp_snp_enable;
    }
    else
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }

    return DHCP_SNP_RETURN_CODE_OK;
}

unsigned int npd_dhcp_information_snp_opt82_status_get
(
	unsigned char *snp_opt82_status
)
{
	struct NPD_DHCP_SNP_GLOBAL_STATUS user_snp;
    
	memset(&user_snp, 0, sizeof(user_snp));
    
    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user_snp))
	{
		*snp_opt82_status = user_snp.dhcp_snp_opt82_enable;
	}
    else
	{
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_check_vlanl_status
 *		check DHCP_Snooping enable/disable status on special vlan
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_EN_VLAN			- vlan status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_EN_VLAN		- vlan status is disable
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	- check fail, vlan not exist
 **********************************************************************************/

unsigned int npd_dhcp_snp_check_vlan_status
(
	unsigned short vlanId
)
{
	struct vlan_s *vlanNode = NULL;
    struct NPD_DHCP_SNP_GLOBAL_STATUS global_cfg;
    
	memset(&global_cfg, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
    
	vlanNode = npd_find_vlan_by_vid(vlanId);
	if (NULL == vlanNode)
	{
		syslog_ax_dhcp_snp_dbg("the vlan has not exist.\n");
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}
	else
    {
        if(vlanNode)
        {
            free(vlanNode);
        }
        
        if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&global_cfg))
        {
            syslog_ax_dhcp_snp_dbg("To get global item failed!\n");
            return DHCP_SNP_RETURN_CODE_ERROR;
        }
        
		if (NPD_VBMP_MEMBER(global_cfg.vlan_admin_status, vlanId))
		{
			syslog_ax_dhcp_snp_dbg("the vlan has enable dhcpsnp.\n");
			return DHCP_SNP_RETURN_CODE_EN_VLAN;
		}
		else
		{
			syslog_ax_dhcp_snp_dbg("the vlan has disable dhcpsnp.\n");
			return DHCP_SNP_RETURN_CODE_NOT_EN_VLAN;
		}
	}
    
    return DHCP_SNP_RETURN_CODE_OK;
}

int npd_dhcp_snp_traversal_vlan_endis
(
	unsigned int *endis
)
{
    int ret = 0;
	unsigned int vlan_id = 0;
    struct NPD_DHCP_SNP_GLOBAL_STATUS global_cfg;
    
	memset(&global_cfg, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    ret = npd_dhcp_snp_global_cfg_get(&global_cfg);
    if (DHCP_SNP_RETURN_CODE_OK == ret)
    {
        NPD_VBMP_ITER(global_cfg.vlan_admin_status, vlan_id)
        {
            endis[0]++;
            endis[endis[0]] = vlan_id;
        }
    }

    return ret;
}

unsigned int npd_dhcp_snp_set_port_trust_mode
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned int trust_mode
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT itemA;
	memset(&itemA,0,sizeof(itemA));

    itemA.global_port_ifindex = g_eth_index;

    syslog_ax_dhcp_snp_dbg("npd dhcp-snooping: g_eth_index=0x%02x, trust_mode=%d\n", g_eth_index, trust_mode);
    
	if (DHCP_SNP_RETURN_CODE_ERROR == npd_dhcp_snp_status_item_find(&itemA))
	{
		itemA.opt82_strategy = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE;
		itemA.opt82_circuitid = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT;
        itemA.global_port_ifindex = g_eth_index;
    	itemA.trust_mode = trust_mode;

		return npd_dhcp_snp_status_item_insert(&itemA);
	}
	else 
	{
		if(itemA.trust_mode == trust_mode)
		{
			return DHCP_SNP_RETURN_CODE_ALREADY_SET;
		}	
		else
		{
            itemA.trust_mode = trust_mode;
            return npd_dhcp_snp_status_item_refresh(&itemA, &itemA);
		}
	}
}

/**********************************************************************************
 * npd_dhcp_snp_get_option
 *		get an option with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL			- not get the option
 *		not NULL 		- string of the option
 **********************************************************************************/
void *npd_dhcp_snp_get_option
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int over = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned int curr = NPD_DHCP_SNP_OPTION_FIELD;
	unsigned char *optionptr = NULL;

	syslog_ax_dhcp_snp_dbg("get option %x.\n", code);

	if (!packet) {
		syslog_ax_dhcp_snp_err("get option error, parameter is null\n");
		return NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done) 
	{
		if ((i >= length)
            || (((DHCP_PADDING != optionptr[i + NPD_DHCP_SNP_OPT_CODE])
                    && (DHCP_END != optionptr[i + NPD_DHCP_SNP_OPT_CODE]))
                    && (i + 2 >= length)))
		{
			syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
			return NULL;
		}
		if ((optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
            && (DHCP_PADDING != optionptr[i + NPD_DHCP_SNP_OPT_CODE])
                    && (DHCP_END != optionptr[i + NPD_DHCP_SNP_OPT_CODE]))
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
				return NULL;
			}

			syslog_ax_dhcp_snp_dbg("get option %x success.\n", code);
			return optionptr + i + 2;
		}			
		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length) 
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return NULL;
				}
				over = optionptr[i + NPD_DHCP_SNP_OPT_DATA];
				i += optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD) 
				{
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD) 
				{
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else
				{
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
                break;
		}
	}
	return NULL;
}

unsigned int npd_dhcp_snp_get_interface_info
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *ports_array,
	unsigned int index
)
{
	unsigned int ret = 0;
	if(index == 0)
	{
		if(ports_array->global_port_ifindex == 0)
		{
			ret = dbtable_hash_head(npd_dhcp_snp_status_item_index, ports_array, ports_array, NULL);
		}
		else
		{
			ret = dbtable_hash_next(npd_dhcp_snp_status_item_index, ports_array, ports_array, NULL);
		}
	}
	else
	{
		ports_array->global_port_ifindex = index;
		ret = dbtable_hash_search(npd_dhcp_snp_status_item_index, ports_array, NULL, ports_array);
	}
	return ret;

}

unsigned int npd_dhcp_snp_wrapsum(unsigned int sum)
{
	sum = ~sum & 0xFFFF;

	return htons(sum);
}

unsigned int npd_dhcp_snp_in_csum 
(
    unsigned char *buf,
    unsigned nbytes,
    unsigned int sum
)
{
    unsigned i;

    for (i = 0; i < (nbytes & ~1U); i += 2)
    {
        sum += (unsigned short) ntohs(*((unsigned short *)(buf + i)));
        if (sum > 0xFFFF)
        {
            sum -= 0xFFFF;
        }
    }	

    if (i < nbytes)
    {
        sum += buf [i] << 8;
        if (sum > 0xFFFF)
        {
            sum -= 0xFFFF;
        }
    }

    return sum;
}

inline void npd_dhcp_snp_make_dhcp_nak(NPD_DHCP_SNP_USER_ITEM_T* entry, char* buf, int* length)
{
    /* DHCP Magic cookie. */
#define DHCP_OPTIONS_COOKIE	"\143\202\123\143"
    int ni = 0;
    unsigned char data = 0;
    char* ptr = NULL;
    struct __raw
    {
        unsigned char  op;		/* 0: Message opcode/type */
        unsigned char  htype;	/* 1: Hardware addr type (net/if_types.h) */
        unsigned char  hlen;		/* 2: Hardware addr length */
        unsigned char  hops;		/* 3: Number of relay agent hops from client */
        unsigned int xid;		/* 4: Transaction ID */
        unsigned short secs;		/* 8: Seconds since client started looking */
        unsigned short flags;	/* 10: Flag bits */
        struct in_addr ciaddr;	/* 12: Client IP address (if already in use) */
        struct in_addr yiaddr;	/* 16: Client IP address */
        struct in_addr siaddr;	/* 18: IP address of next server to talk to */
        struct in_addr giaddr;	/* 20: DHCP relay agent IP address */
        unsigned char chaddr[16];	/* 24: Client hardware address */
        char sname[64];	/* 40: Server name */
        char file[128];	/* 104: Boot filename */
        unsigned int cookie;
    };
    struct __option
    {
        char code;
        char length;
        char data[0];
    };
    struct __raw* packet = (struct __raw* )buf;
    struct __option* option = NULL;

    memset(packet, 0, sizeof(*packet));
    packet->op = NPD_DHCP_BOOTREPLY;
    packet->htype = 1;
    packet->hlen = 6;
    memcpy(packet->chaddr, entry->chaddr, packet->hlen);
    packet->hops = 0;
    packet->xid = entry->xid;
    packet->secs = 0;
    if (!entry->relay_ip)
    {
        packet->flags = htons(0x8000);
    }
    else
    {
        packet->flags = 0;
    }
    memcpy(&packet->giaddr, &entry->relay_ip, sizeof(unsigned int));
    memcpy(&packet->cookie, DHCP_OPTIONS_COOKIE, 4);

    ptr = buf + sizeof(struct __raw);

    option = (struct __option*)ptr;

    option->code = DHCP_MESSAGE_TYPE;
    option->length = 1;
    data = (unsigned char)NPD_DHCP_NAK;
    memcpy(option->data, &data, option->length);

    ptr = (char*)option;
    option = (struct __option* )(ptr + sizeof(struct __option) + option->length);

    option->code = DHCP_SERVER_ID;
    option->length = 4;
    memcpy(option->data, &entry->server_id, option->length);

    ptr = (char*)option;
    option = (struct __option* )(ptr + sizeof(struct __option) + option->length);

    option->code = DHCP_END;
    option->length = 0;

    ptr = (char*)option;
    ptr++;  /* skip option-code-end */
    ni =  ptr - buf;

    for (ni = ni % 4; (ni != 0) && (ni < 4); ni++)
    {
        *ptr = 0;
        ptr++;
    }
    *length +=  ptr - buf;

    return ;
}

inline void npd_dhcp_snp_make_ethernet_hdr(NPD_DHCP_SNP_USER_ITEM_T* entry, char* buf)
{
    struct ether_header* ether = (struct ether_header *)buf;

    memcpy(ether->ether_dhost, entry->chaddr, sizeof(ether->ether_dhost));
    memcpy(ether->ether_shost, SYS_PRODUCT_BASEMAC, sizeof (ether->ether_shost));
    ether->ether_type = htons(ETHERTYPE_IP);

    return ;
}

inline void npd_dhcp_snp_make_ip_udp_hdr(int length, NPD_DHCP_SNP_USER_ITEM_T* entry, char* buf)
{
#define l3_hdr_size    (sizeof(struct iphdr) + sizeof(struct udphdr))
    struct iphdr* ip = (struct iphdr*)buf;
    struct udphdr* udp = (struct udphdr*)(buf + sizeof(struct iphdr));

    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0x10;
    ip->tot_len = htons(sizeof(*ip) + sizeof(*udp) + length);
    ip->id = 0;
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = 0x11;
    ip->check = 0;
    memcpy(&ip->saddr, &entry->server_id, 4);
    if (entry->relay_ip)
    {
        memcpy(&ip->daddr, &entry->relay_ip, 4);
    }
    else
    {
        ip->daddr = 0xffffffff;
    }

    ip->check = npd_dhcp_snp_wrapsum(npd_dhcp_snp_in_csum((unsigned char *)ip, sizeof(*ip), 0));

    udp->source = htons(NPD_DHCPR_SERVER_PORT);
    if (entry->relay_ip)
    {
        udp->dest = htons(NPD_DHCPR_SERVER_PORT);
    }
    else
    {
        udp->dest = htons(NPD_DHCPR_CLIENT_PORT);
    }
    udp->len = htons(sizeof(*udp) + length);
    udp->check = 0;

	udp->check = npd_dhcp_snp_wrapsum(npd_dhcp_snp_in_csum ((unsigned char *)udp, sizeof(*udp),
			   npd_dhcp_snp_in_csum (buf + l3_hdr_size, length, 
				     npd_dhcp_snp_in_csum ((unsigned char *)&ip->saddr,
					       2 * sizeof(ip->saddr),
					       0x11 + (unsigned int)ntohs (udp->len)))));

    return ;
}

void npd_dhcp_snp_send_nak_to_client(NPD_DHCP_SNP_USER_ITEM_T* entry)
{
#define l2_hdr_size       (sizeof(struct ethhdr))
#define l2_l3_hdr_size    (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr))
    unsigned char is_tagged = 0;
    int length = 0;
    char buffer[512 + NPD_PKT_RESERVED_LEN];
    char* buf = &buffer[NPD_PKT_RESERVED_LEN];

    memset(buffer, 0, sizeof(buffer));
#ifdef HAVE_DHCP_RELAY
    if (0 != ntohl(entry->relay_ip))
    {
        npd_dhcp_snp_make_dhcp_nak(entry, buf, &length);
        npd_dhcpr_send_to_agent(buf, length, entry->relay_ip, htons(NPD_DHCPR_SERVER_PORT));
    }
    else
#endif
    {
        npd_dhcp_snp_make_ethernet_hdr(entry, buf);
        npd_dhcp_snp_make_dhcp_nak(entry, buf + l2_l3_hdr_size, &length);
        npd_dhcp_snp_make_ip_udp_hdr(length, entry, buf + l2_hdr_size);
        
        npd_vlan_check_contain_netif(entry->vlanId, entry->ifindex, &is_tagged);
		nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_IP_UDP_DHCP_E, 
											entry->ifindex, 
											entry->vlanId, 
											(unsigned int)is_tagged, 
											buf, 
											length + l2_l3_hdr_size);
    }
}

unsigned int npd_dhcp_snp_send_to_server
(
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netifIndex,
	unsigned short vid,
	int relay_is_enable
)
{
	unsigned int agent_count = 0;
 	NPD_DHCP_MESSAGE_T *data = NULL;
	syslog_ax_dhcp_snp_dbg("npd_dhcp_snp_send_to_server: vid %d, ifindex 0x%x bufflen %d", vid, netifIndex, buffLen);
			
	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

#ifdef HAVE_DHCP_RELAY
    if (0 == relay_is_enable)  /*not enable dhcp relay on interface, so broadcast on vlan*/
#endif
	{
		syslog_ax_dhcp_snp_dbg("start flood dhcp broadcast packet in vlan %d.\n", vid);

		npd_packet_tx_broadcast_exclude_netif(NAM_PACKET_TYPE_IP_UDP_DHCP_E, vid, packetBuffs, buffLen,netifIndex);
		return DHCP_SNP_RETURN_CODE_OK;
	}
#ifdef HAVE_DHCP_RELAY        
	else 
	{
		if( data->hops > dhcp_relay_max_hops )
		{
			syslog_ax_dhcp_snp_dbg("dhcp relay to server: hops %d beyond limits", data->hops);
	 		return DHCP_SNP_RETURN_CODE_PKT_DROP;
		}
		data->hops++;

		agent_count = npd_dhcpr_relay_server_send(packetBuffs, buffLen, netifIndex, vid);

		syslog_ax_dhcp_snp_dbg("Relay dhcp to %d servers.\n", agent_count);
		if( agent_count == 0 ) /*broadcast the packet in VLAN if there is no helper */
		{
			syslog_ax_dhcp_snp_dbg("start flood dhcp broadcast packet in vlan %d.\n", vid);

			npd_packet_tx_broadcast_exclude_netif(NAM_PACKET_TYPE_IP_UDP_DHCP_E, vid, packetBuffs, buffLen,netifIndex);
		}
	}	
#endif		

	return DHCP_SNP_RETURN_CODE_PKT_DROP;
}


unsigned int npd_dhcp_snp_send_to_client
(
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int eth_g_index,
	unsigned short vid
)
{
	int ret = 0;

	char zeroMac[6]={0};

	struct ethhdr *ethHdr = NULL;
	struct iphdr  *ipHdr = NULL;
	struct udphdr *udpHdr = NULL;
	NPD_DHCP_MESSAGE_T *data = NULL;
	NPD_DHCP_SNP_USER_ITEM_T userInfo;
	unsigned int output_index =0 ;
	unsigned short vlanId = 0;

	unsigned char endis = 0;
	unsigned char isTagged = 0;
	unsigned char isBroadcast = 0;
	unsigned int flag = FALSE;
	
	syslog_ax_dhcp_snp_dbg("npd dhcp snp send to client: vid %d, ifindex 0x%x bufflen %d", vid, eth_g_index, buffLen);

	ethHdr = (struct ethhdr *)packetBuffs;
	ipHdr  = (struct iphdr *)(packetBuffs + sizeof(struct ethhdr));
	udpHdr = (struct udphdr *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr));
	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

	if(!memcmp(data->chaddr, zeroMac, MAC_ADDR_LEN) )
	{
		syslog_ax_dhcp_snp_dbg("npd dhcp snp send to client: no client Hardware address");
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
	}

	memset(&userInfo, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	memcpy(userInfo.chaddr, data->chaddr, MAC_ADDR_LEN);

	if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_get_global_status(&endis))
	{
		if ((endis == NPD_DHCP_SNP_ENABLE) && (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_tbl_item_find(&userInfo)))
		{
			syslog_ax_dhcp_snp_dbg("npd dhcp snp send to client: find user for pkt");
			vlanId = userInfo.vlanId;
			output_index = userInfo.ifindex;
			ret = 1;
		}
        else
		{
			ret = 0;
		}
	}
    else
	{
		syslog_ax_dhcp_snp_dbg("get dhcp snooping global table faile");
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
	}
    
#ifdef HAVE_DHCP_RELAY
	flag = npd_dhcpr_check_global_status() ;
#endif
	if((ntohl(data->giaddr) == 0) ||  (flag!=TRUE))
	{
		if(ret == 0)
		{
			vlanId = vid;	
			isBroadcast = 1;
		}
	}
	else if((ntohl(data->giaddr) != 0) && (flag == TRUE))
	{				
#ifdef HAVE_DHCP_RELAY	
		if(DHCP_SNP_RETURN_CODE_OK != npd_dhcp_relay_message_replace(packetBuffs, &isBroadcast,&output_index,&buffLen, &vlanId))
		{
			syslog_ax_dhcp_snp_dbg("relay message replace faile!");
			return DHCP_SNP_RETURN_CODE_PKT_DROP;			
		}
#endif
	}
	
	if( 1 == isBroadcast )
	{				

		ret = npd_packet_tx_broadcast_exclude_netif(NAM_PACKET_TYPE_IP_UDP_DHCP_E, 
														vlanId, 
														packetBuffs, 
														buffLen,
														eth_g_index);

		syslog_ax_dhcp_snp_dbg("broadcast msg for user: %02x:%02x:%02x:%02x:%02x:%02x on vlanid %d, ret %x\n",
					data->chaddr[0], data->chaddr[1], data->chaddr[2],
					data->chaddr[3], data->chaddr[4], data->chaddr[5],
					vlanId, ret);
	}
	else {
		
		npd_vlan_check_contain_netif(vlanId,output_index, &isTagged);
		ret = nam_packet_tx_unicast_by_netif( NAM_PACKET_TYPE_IP_UDP_DHCP_E, 
											output_index, 
											vlanId, 
											isTagged, 
											packetBuffs, 
											buffLen);
		syslog_ax_dhcp_snp_dbg("unicast msg for user: %02x:%02x:%02x:%02x:%02x:%02x on vlanid %d, port 0x%x, ret %x ,isTagged %x	buffLen %d\n",
							data->chaddr[0], data->chaddr[1], data->chaddr[2],
							data->chaddr[3], data->chaddr[4], data->chaddr[5],
							vlanId, output_index, ret,isTagged, buffLen);			
		
	}	

	return DHCP_SNP_RETURN_CODE_PKT_DROP;			
}
	
unsigned int npd_dhcp_snp_packet_tx_hook
(
	int packetType,
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netif_index,
	unsigned char isTagged,
	unsigned short vid
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int trust_mode = NPD_DHCP_SNP_PORT_MODE_TRUST;
	NPD_DHCP_MESSAGE_T *data = NULL;
    unsigned char* p_message_type = NULL;
	unsigned char status = DHCP_SNP_RETURN_CODE_OK;
	/* send packet */

	syslog_ax_dhcp_snp_dbg("DHCP TX Hook:start dhcp snp packet rx process vlanid %d netif_index %d.\n", vid, netif_index);

	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

    if (NPD_DHCP_BOOTREQUEST == data->op)
    {
        syslog_ax_dhcp_snp_dbg("DHCP TX Hook:DHCP-Snooping hook just execute the reply packet, which come from CPU!");
        return ret;
    }

	npd_dhcp_snp_get_global_status(&status);

   	p_message_type = (unsigned char *)npd_dhcp_snp_get_option(data, NPD_DHCP_MESSAGE_TYPE);
    if (p_message_type == NULL)
    {
		syslog_ax_dhcp_snp_dbg("%% DHCP TX Hook:Unknown dhcp option-53 or bootp packet. Drop it.\r\n");
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
    }
	/*if(*p_message_type == NPD_DHCP_ACK)*/
	{
        /*Maybe station move */
		ret = DHCP_SNP_RETURN_CODE_MAC_ILEGAL;
	}
	if (status != NPD_DHCP_SNP_ENABLE)
	{
		syslog_ax_dhcp_snp_dbg("DHCP TX Hook:DHCP Snooping not enabled global.\n");
		return ret;
	}

	if (npd_dhcp_snp_check_vlan_status(vid) == DHCP_SNP_RETURN_CODE_EN_VLAN)
	{
		if( DHCP_SNP_RETURN_CODE_OK != npd_dhcp_packet_record_user_info(vid, netif_index, data, *p_message_type, trust_mode) )
		{
			syslog_ax_dhcp_snp_dbg("DHCP TX Hook:receive dhcp reply packet from untrusted interface 0x%x, discard\n", netif_index);
			return DHCP_SNP_RETURN_CODE_PKT_DROP;
		}
	}

	return ret; 
}

/**********************************************************************************
 * npd_dhcp_snp_packet_rx_process()
 *	DESCRIPTION:
 *		receive packets for DHCP Snooping rx 
 *
 *	INPUTS:
 *		unsigned long numOfBuff		- number of buffer
 *		unsigned char *packetBuffs[]	- array of buffer
 *		unsigned long buffLen[]			- array of buffer length
 *		unsigned int interfaceId			- port Or TrunkId has been transfer to eth_g_index
 *		unsigned char isTagged			- tag flag of port
 *		unsigned short vid				- vlanid
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		GT_FAIL			- build fail
 *		GT_NO_RESOURCE	- alloc memory error
 *		GT_BAD_SIZE		- packet length is invalid
 *		GT_OK			- build ok
 *		totalLen			- receive packet length
***********************************************************************************/
unsigned int npd_dhcp_snp_packet_rx_process
(
	int packetType,
	unsigned char *packetBuffs,
	unsigned long* buffLen,
	unsigned int netif_index,
	unsigned char isTagged,
	unsigned short vid
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int trust_mode = NPD_DHCP_SNP_PORT_MODE_NOTRUST;
	NPD_DHCP_MESSAGE_T *data = NULL;
	unsigned char* p_message_type = NULL;

    int npd_option_enable = 0;
    int npd_snoop_enable = 0;
    int npd_relay_enable = 0;
    struct NPD_DHCP_SNP_GLOBAL_STATUS snp_entry;
    struct npd_dhcp_relay_global_status relay_entry;
        
	memset(&snp_entry, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
    memset(&relay_entry, 0, sizeof(struct npd_dhcp_relay_global_status));

	syslog_ax_dhcp_snp_dbg("Start dhcp snp packet rx process vlanid %d netif_index %d.\n", vid, netif_index);

	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

    if (0 == memcmp(data->chaddr, SYS_PRODUCT_BASEMAC, 6))  /* local dhcp-client packet! */
    {
        return DHCP_SNP_RETURN_CODE_OK;
    }
    
	p_message_type = (unsigned char *)npd_dhcp_snp_get_option(data, NPD_DHCP_MESSAGE_TYPE);
    if (p_message_type == NULL)
    {
		syslog_ax_dhcp_snp_dbg("%% Unknown dhcp option-53 or bootp packet. Drop it.\r\n");
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
    }
    else
    {
        if ((*p_message_type > 0) && (*p_message_type < NPD_DHCP_UNKNOWN))
        {
            syslog_ax_dhcp_snp_dbg("________get %s packet!\n", npd_dhcp_packet_type_str[*p_message_type]);
        }
        else
        {
            syslog_ax_dhcp_snp_dbg("________get %s packet, packet type-code = 0x%2x!\n", \
                                            npd_dhcp_packet_type_str[NPD_DHCP_UNKNOWN], *p_message_type);
        }
    }


    if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&snp_entry))
    {
        return DHCP_SNP_RETURN_CODE_PKT_DROP;
    }

	if ((NPD_DHCP_SNP_ENABLE == snp_entry.dhcp_snp_enable)
        && (NPD_VBMP_MEMBER(snp_entry.vlan_admin_status, vid)))
/*        && (DHCP_SNP_RETURN_CODE_EN_VLAN == npd_dhcp_snp_check_vlan_status(vid))) */
	{
		npd_snoop_enable = 1;
        if (NPD_DHCP_SNP_OPT82_ENABLE == snp_entry.dhcp_snp_opt82_enable)
        {
            npd_option_enable = 0x1;
        }
	}

#ifdef HAVE_DHCP_RELAY
    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&relay_entry))
	{
		if (relay_entry.dhcp_relay_endis == NPD_DHCP_RELAY_ENABLE)
		{
            int lo_netif_index = 0;

            if (vid != NPD_PORT_L3INTF_VLAN_ID)
        	{
        		lo_netif_index = npd_netif_vlan_index(vid);
        	}
        	else
        	{
        		lo_netif_index = netif_index;
        	}

            if (npd_dhcpr_check_intf_status(lo_netif_index))
            {
                npd_relay_enable = 1;
                if (NPD_DHCP_RELAY_OPT82_ENABLE == relay_entry.dhcp_relay_opt82_enable)
                {
                    npd_option_enable |= 0x2;
                }
            }
            
            if ((0 != ntohl(data->giaddr)) && (NPD_DHCP_BOOTREPLY == data->op))      /* XXX: is dangerous */
            {
                npd_relay_enable = 1;
                npd_option_enable |= 0x2;
            }
		}
	}
#endif

    if (!(npd_snoop_enable | npd_relay_enable))
    {
        return DHCP_SNP_RETURN_CODE_OK;
    }

	if (npd_snoop_enable)
	{
        trust_mode = NPD_DHCP_SNP_PORT_MODE_NOTRUST;
        npd_dhcp_snp_get_trust_mode(vid, netif_index, &trust_mode);
        
		if (NPD_DHCP_BOOTREPLY == data->op)
		{
			if (trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST)
			{
				syslog_ax_dhcp_snp_dbg("receive dhcp reply packet from untrusted interface %d, discard\n", netif_index);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
		}
		
		if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_packet_record_user_info(vid, netif_index, data, *p_message_type, trust_mode))
		{
			syslog_ax_dhcp_snp_dbg("receive dhcp reply packet from untrusted interface 0x%x, discard\n", netif_index);
			return DHCP_SNP_RETURN_CODE_PKT_DROP;
		}
	}

    if (0 != npd_option_enable)
    {
    	if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_packet_information_process
                    (vid, netif_index, isTagged, packetBuffs, buffLen, npd_option_enable))
    	{
    		syslog_ax_dhcp_snp_dbg("fail to process option for DHCP packet, discard\n");
    		return DHCP_SNP_RETURN_CODE_PKT_DROP;
    	}
	
    }

    syslog_ax_dhcp_snp_dbg("start dhcp snp packet rx process ret = %d without option-82.\n",ret);
    if (NPD_DHCP_BOOTREPLY == data->op)
	{
		ret = npd_dhcp_snp_send_to_client(packetBuffs, *buffLen, netif_index, vid);
	}
	else if (NPD_DHCP_BOOTREQUEST == data->op)
	{
		ret = npd_dhcp_snp_send_to_server(packetBuffs, *buffLen, netif_index, vid, npd_relay_enable);
	}
	syslog_ax_dhcp_snp_dbg("end dhcp snp packet rx process ret = %d.\n", ret);

	return ret;	
}

int npd_dhcp_snp_modify_port_by_trunk(struct NPD_DHCP_SNP_GLOBAL_STATUS* entry, int netif_index)
{
    int array_index = 0;
    int array_port_tindex = 0;
    struct trunk_s trunk;

    memset(&trunk, 0, sizeof(struct trunk_s));
    array_index = trunk_array_index_from_ifindex(netif_index);
    
    trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
    if (0 == dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk))
    {
        NPD_PBMP_ITER(trunk.ports, array_port_tindex)
        {
            entry->switch_port_control_count[array_port_tindex] = entry->switch_port_control_count[array_index];
        }
    }
    else
    {
        syslog_ax_dhcp_snp_dbg("Get trunk struct failed!\n");
        return ARP_RETURN_CODE_ERROR;
    }
    return 0;
}

unsigned int npd_dhcp_snp_vlan_enable_set(unsigned char is_enable, unsigned short vlan_id)
{
    int array_port = 0;
    int netif_index = 0;
    int netif_type = 0;
    unsigned int ret = 0;
    struct vlan_s vlan;
    struct NPD_DHCP_SNP_GLOBAL_STATUS global_cfg;

    memset(&global_cfg, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    ret = npd_dhcp_snp_global_cfg_get(&global_cfg);

    if (DHCP_SNP_RETURN_CODE_OK == ret)
    {
        if (global_cfg.dhcp_snp_enable == NPD_DHCP_SNP_ENABLE)
        {
            memset(&vlan, 0, sizeof(struct vlan_s));

            vlan.vid = vlan_id;
            if (0 != dbtable_sequence_search(g_vlans, vlan.vid, &vlan))
            {
                syslog_ax_dhcp_snp_dbg("Get vlan struct failed!\n");
                return DHCP_SNP_RETURN_CODE_ERROR;
            }
        
            if (NPD_VBMP_MEMBER(global_cfg.vlan_admin_status, vlan_id))
            {
                if (NPD_DHCP_SNP_ENABLE == is_enable)
                {
                    ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
                }
                else
                {
                    npd_dhcp_snp_lease_delete_by_vlan(vlan_id, NPD_DHCP_SNP_BIND_TYPE_DYNAMIC);
                    
                    NPD_VBMP_VLAN_REMOVE(global_cfg.vlan_admin_status, vlan_id);
                    NPD_PBMP_ITER(vlan.untag_ports, array_port)
                    {
                        if (global_cfg.switch_port_control_count[array_port] > 0)
                        {
                            global_cfg.switch_port_control_count[array_port]--;
                            netif_index = netif_array_index_to_ifindex(array_port);
                            netif_type = npd_netif_type_get(netif_index);
                        
                            if (NPD_NETIF_TRUNK_TYPE == netif_type)
                            {
                                if (0 != npd_dhcp_snp_modify_port_by_trunk(&global_cfg, netif_index))
                                {
                                    syslog_ax_dhcp_snp_dbg("Modify port by trunk failed!\n");
                                    return ARP_RETURN_CODE_ERROR;
                                }
                            }
                        }
                    }
                    
                    NPD_PBMP_ITER(vlan.tag_ports, array_port)
                    {
                        if (global_cfg.switch_port_control_count[array_port] > 0)
                        {
                            global_cfg.switch_port_control_count[array_port]--;
                            netif_index = netif_array_index_to_ifindex(array_port);
                            netif_type = npd_netif_type_get(netif_index);
                        
                            if (NPD_NETIF_TRUNK_TYPE == netif_type)
                            {
                                if (0 != npd_dhcp_snp_modify_port_by_trunk(&global_cfg, netif_index))
                                {
                                    syslog_ax_dhcp_snp_dbg("Modify port by trunk failed!\n");
                                    return ARP_RETURN_CODE_ERROR;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (NPD_DHCP_SNP_DISABLE == is_enable)
                {
                    ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
                }
                else
                {
                    NPD_VBMP_VLAN_ADD(global_cfg.vlan_admin_status, vlan_id);

                    NPD_PBMP_ITER(vlan.untag_ports, array_port)
                    {
                        global_cfg.switch_port_control_count[array_port]++;
                        netif_index = netif_array_index_to_ifindex(array_port);
                        netif_type = npd_netif_type_get(netif_index);
                        
                        if (NPD_NETIF_TRUNK_TYPE == netif_type)
                        {
                            if (0 != npd_dhcp_snp_modify_port_by_trunk(&global_cfg, netif_index))
                            {
                                syslog_ax_dhcp_snp_dbg("Modify port by trunk failed!\n");
                                return ARP_RETURN_CODE_ERROR;
                            }
                        }
                    }
                    
                    NPD_PBMP_ITER(vlan.tag_ports, array_port)
                    {
                        global_cfg.switch_port_control_count[array_port]++;

                        netif_index = netif_array_index_to_ifindex(array_port);
                        netif_type = npd_netif_type_get(netif_index);
                        
                        if (NPD_NETIF_TRUNK_TYPE == netif_type)
                        {
                            if (0 != npd_dhcp_snp_modify_port_by_trunk(&global_cfg, netif_index))
                            {
                                syslog_ax_dhcp_snp_dbg("Modify port by trunk failed!\n");
                                return ARP_RETURN_CODE_ERROR;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
        }
    }
    else
    {
        syslog_ax_dhcp_snp_err("DHCP snp get global configure error!\n");
    }
	
	if (DHCP_SNP_RETURN_CODE_OK == ret)
	{
		ret = npd_dhcp_snp_global_cfg_set(&global_cfg);
	}
    
    return ret;
}

/**********************************************************************************
 * npd_dbus_dhcp_snp_enable_global_status
 *		set DHCP-Snooping enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_enable_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_INIT_0;	
	struct NPD_DHCP_SNP_GLOBAL_STATUS global_cfg;
	memset(&global_cfg, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcp_snp_global_cfg_get(&global_cfg);

    if (DHCP_SNP_RETURN_CODE_OK == ret)
    {
        if (global_cfg.dhcp_snp_enable == isEnable)
        {
            syslog_ax_dhcp_snp_err("DHCP SNP ALREADY SET %s", isEnable ? "enable" : "disable");
    		ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
        }
        else
        {
            global_cfg.dhcp_snp_enable = isEnable;

            if (0 == isEnable)
            {
                npd_dhcp_snp_tbl_destroy();
            }

            ret = npd_dhcp_snp_global_cfg_set(&global_cfg);
            if (DHCP_SNP_RETURN_CODE_OK != ret)
            {
                syslog_ax_dhcp_snp_err("DHCP snp set global configure error!\n");
            }
        }
    }
    else
    {
        syslog_ax_dhcp_snp_err("DHCP snp get global configure error!\n");
    }

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 


DBusMessage * npd_dbus_dhcp_snp_check_vlan_status(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
 	unsigned int status[4095];
    unsigned int* ptr_status = status;
	memset(status ,0,sizeof(status));
	
	ret = npd_dhcp_snp_traversal_vlan_endis(status);

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
	{
		npd_syslog_err("tracking dbus set error!\n");
		return reply;
	}

    dbus_message_append_args(reply,
							DBUS_TYPE_UINT32,
							&ret,
							
							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_status,
							4095,
							
							DBUS_TYPE_INVALID);
	return reply;
}

DBusMessage * npd_dbus_dhcp_snp_check_op82_status(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
    unsigned char*  tmp = NULL;

	memset(&user, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	syslog_ax_dhcp_snp_dbg("start check global status!\n ");

	if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&user))
	{
		syslog_ax_dhcp_snp_dbg("get global status faile.\n");
		ret =  DHCP_SNP_RETURN_CODE_ERROR;		
	}
    else
	{
		ret = DHCP_SNP_RETURN_CODE_OK;
	}

	syslog_ax_dhcp_snp_dbg("ret = %d.\n", ret);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic(&iter,
									 DBUS_TYPE_UINT32, &ret);	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.dhcp_snp_opt82_fill_format_type));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.dhcp_snp_opt82_format_type));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.dhcp_snp_opt82_remoteid_type));
    tmp = user.dhcp_snp_opt82_remoteid_str;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &tmp);

	return reply;

}

DBusMessage * npd_dbus_dhcp_snp_check_op82_global_status_get(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;

	memset(&user, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	syslog_ax_dhcp_snp_dbg("start check global status!\n ");

	if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&user))
	{
		syslog_ax_dhcp_snp_dbg("get global status faile.\n");
		ret =  DHCP_SNP_RETURN_CODE_ERROR;		
	}
    else
	{
		ret = DHCP_SNP_RETURN_CODE_OK;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic(&iter,
									 DBUS_TYPE_UINT32, &ret);	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.dhcp_snp_enable));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &(user.dhcp_snp_opt82_enable));

	return reply;

}

DBusMessage * npd_dbus_dhcp_snp_check_op82_interface_status(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

 	unsigned int ifindex = DHCP_SNP_RETURN_CODE_OK;
	unsigned int mode = DHCP_SNP_RETURN_CODE_OK;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT show_trust_ports;	
	memset(&show_trust_ports,0,sizeof(show_trust_ports));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
							DBUS_TYPE_UINT32, &ifindex,						
							DBUS_TYPE_UINT32, &mode,						
							DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	show_trust_ports.global_port_ifindex = ifindex ;
		
	syslog_ax_dhcp_snp_dbg("start check global dhcp snp!\n");		
	ret = npd_dhcp_snp_get_interface_info(&show_trust_ports,mode);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	if(ret == DHCP_SNP_RETURN_CODE_OK)
	{

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_trust_ports.global_port_ifindex));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_trust_ports.trust_mode));
	}
	
	return reply;
}

DBusMessage * npd_dbus_dhcp_snp_op82_port_status_get(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

 	unsigned int ifindex = DHCP_SNP_RETURN_CODE_OK;
	unsigned int mode = DHCP_SNP_RETURN_CODE_OK;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT show_trust_ports;	
	memset(&show_trust_ports,0,sizeof(show_trust_ports));

	unsigned char *circuit_temp = NULL;
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
							DBUS_TYPE_UINT32, &ifindex,						
							DBUS_TYPE_UINT32, &mode,						
							DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	show_trust_ports.global_port_ifindex = ifindex ;
		
	syslog_ax_dhcp_snp_dbg("start check global dhcp snp!\n");
		
	ret = npd_dhcp_snp_get_interface_info(&show_trust_ports,mode);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
    
	if(ret == DHCP_SNP_RETURN_CODE_OK)
	{

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_trust_ports.global_port_ifindex));

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_trust_ports.opt82_strategy));

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_trust_ports.opt82_circuitid));
		
		circuit_temp = show_trust_ports.opt82_circuitid_str;
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_STRING,
										&circuit_temp);
	}
	
	return reply;
}


DBusMessage * npd_dbus_dhcp_snp_check_vlan_status_by_vid(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanid = 0;
 	unsigned int flag = 0;
	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanid,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	flag = npd_dhcp_snp_check_vlan_status(vlanid);
	if(DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == flag){
		syslog_ax_dhcp_snp_dbg("the vlan is not exist.\n");
		ret = DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;		
	}else{
		ret = DHCP_SNP_RETURN_CODE_OK;
	}
		
	syslog_ax_dhcp_snp_dbg("ret = 0x%x.\n",ret);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &flag);

	return reply;

}

/**********************************************************************************
 * npd_dbus_dhcp_snp_enable_vlan_status
 *		enable/disable DHCP-Snooping on special vlan
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_enable_vlan_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_DISABLE;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcp_snp_vlan_enable_set(isEnable, vlanId);
    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 


/**********************************************************************************
 * npd_dbus_dhcp_snp_config_port
 *		config DHCP-Snooping trust mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_config_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned int trust_mode = NPD_DHCP_SNP_PORT_MODE_INVALID;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_UINT32, &trust_mode,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		syslog_ax_dhcp_snp_dbg("config DHCP-Snooping trust mode %s on port %d\n",
						(trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
						((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
						((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" : "unknow")),
						 eth_g_index);
		
        switch (npd_netif_type_get(eth_g_index))
        {
            case NPD_NETIF_ETH_TYPE:
            {
		        npd_key_database_lock();
    			ethPort = (struct eth_port_s*)npd_get_port_by_index(eth_g_index);
    			if (NULL == ethPort)
    			{
    				syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
    				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
    			}
    			else if (-1 != ethPort->trunkid)
    			{
    				/*port is member of trunk*/
    				syslog_ax_dhcp_snp_err(" the port is member of trunk	\n");
    				ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
    			}
    			else
                {
					ret = npd_dhcp_snp_set_port_trust_mode(vlanId, eth_g_index, tagMode, trust_mode);
					syslog_ax_dhcp_snp_err("npd_dhcp_snp_set_port_trust_mode.\n");
					if (DHCP_SNP_RETURN_CODE_OK != ret)
					{
						syslog_ax_dhcp_snp_err("trust_mode already set!\n");
					}
			    }
				npd_key_database_unlock();

                break;
    		}
            case NPD_NETIF_TRUNK_TYPE:
            {
		        npd_key_database_lock();
    			ret = npd_dhcp_snp_set_port_trust_mode(vlanId, eth_g_index, tagMode, trust_mode);
    			syslog_ax_dhcp_snp_err("npd_dhcp_snp_set_port_trust_mode.\n");
    			if (DHCP_SNP_RETURN_CODE_OK != ret)
    			{
    				syslog_ax_dhcp_snp_err("trust_mode already set!\n");
    			}
				npd_key_database_unlock();
                break;
    		}
            default :
            {
                break;
            }
        }
	}
	else
    {
        ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}
    
	if (ethPort)
	{
		free(ethPort);
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
	
} 

/**********************************************************************************
 * npd_dbus_dhcp_snp_show_bind_table
 *		show DHCP-Snooping bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_show_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	
	int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	struct NPD_DHCP_SNP_USER_ITEM_S show_items;

	
	memset(&show_items,0,sizeof(show_items));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &show_items.chaddr[0],
								DBUS_TYPE_BYTE, &show_items.chaddr[1],
								DBUS_TYPE_BYTE, &show_items.chaddr[2],
								DBUS_TYPE_BYTE, &show_items.chaddr[3],
								DBUS_TYPE_BYTE, &show_items.chaddr[4],
								DBUS_TYPE_BYTE, &show_items.chaddr[5],
								DBUS_TYPE_UINT16, &show_items.vlanId,
								DBUS_TYPE_INVALID))){
		syslog_ax_dhcp_snp_dbg("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_dbg("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_dbg("return error caused dbus.\n");
		return NULL;
	}
	
	
	record_count = dbtable_hash_count(npd_dhcp_snp_dbhash_mac_index);

	syslog_ax_dhcp_snp_dbg("start npd_dbus_dhcp_snp_show_bind_table\n");
	ret = npd_dhcp_snp_bind_show(&show_items);
	
	syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	if( ret == DHCP_SNP_RETURN_CODE_OK )
	{
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&record_count);
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_items.bind_type));

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[0]));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[1]));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[2]));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[3]));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[4]));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE,
										&(show_items.chaddr[5]));
		
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_items.ip_addr));

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT16,
										&(show_items.vlanId));
		
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_items.ifindex));

		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(show_items.lease_time));
	}
	return reply;
}



/**********************************************************************************
 * npd_dbus_dhcp_snp_enable_opt82
 *		enable/disable DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_enable_opt82
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_INIT_0;	
	unsigned char opt82_status = NPD_DHCP_SNP_OPT82_DISABLE;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
    if (npd_dhcp_snp_check_global_status() == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = npd_dhcp_snp_get_opt82_status(&opt82_status);
		if (ret != DHCP_SNP_RETURN_CODE_OK) {
			syslog_ax_dhcp_snp_err("check DHCP-Snooping option82 error\n");
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}else {
			syslog_ax_dhcp_snp_dbg("check DHCP-Snooping option82 status %s.\n",
								opt82_status ? "enable" : "disable");
			
			if (opt82_status == isEnable) {
				syslog_ax_dhcp_snp_err("DHCP-Snooping option82 status already set %s\n",
									opt82_status ? "enable" : "disable");
				ret = DHCP_SNP_RETURN_CODE_OPTION82_SNP_ALREADY_SET;
			}else {
				if (isEnable) {
					ret = npd_dhcp_snp_opt82_enable();
				}else {
					ret = npd_dhcp_snp_opt82_disable();
				}
			}
		}
	}
	else { 
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * npd_dbus_dhcp_snp_set_opt82_format_type
 *		set storage format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_set_opt82_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID;	
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &format_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
    {
    	if (user.dhcp_snp_opt82_format_type == format_type)
    	{
    		ret = DHCP_SNP_RETURN_CODE_OPTION82_ATTRIBUTE_ALREADY_SET;
    	}
        else
        {
    		user.dhcp_snp_opt82_format_type = format_type;
            ret = npd_dhcp_snp_global_cfg_set(&user);
        }
    }
    else
    {
        ret = DHCP_SNP_RETURN_CODE_ERROR;
    }

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 




/**********************************************************************************
 * npd_dbus_dhcp_snp_set_opt82_format_type
 *		set fill format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_set_opt82_fill_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char fill_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID;	
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &fill_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
	{
		if (user.dhcp_snp_opt82_fill_format_type == fill_type)
		{
			ret = DHCP_SNP_RETURN_CODE_OPTION82_ATTRIBUTE_ALREADY_SET;
		}
        else
		{
			user.dhcp_snp_opt82_fill_format_type = fill_type;
			ret = npd_dhcp_snp_global_cfg_set(&user);
		}
    }
    else
    {
        ret = DHCP_SNP_RETURN_CODE_ERROR;
    }
    
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
} 

/**********************************************************************************
 * npd_dbus_dhcp_snp_set_opt82_remoteid_content
 *		set remote-id type and content of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_set_opt82_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;	
	char *remoteid_str = NULL;
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &remoteid_type,
								DBUS_TYPE_STRING, &remoteid_str,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
	{
        switch (remoteid_type)
        {
    		case NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC:
    		{
    			/* set default value for remote-id content */
    			user.dhcp_snp_opt82_remoteid_type = remoteid_type;
    			memset(user.dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
    	       	sprintf(user.dhcp_snp_opt82_remoteid_str, "%.2x%.2x%.2x%.2x%.2x%.2x",
    	       	   PRODUCT_MAC_ADDRESS[0], PRODUCT_MAC_ADDRESS[1], PRODUCT_MAC_ADDRESS[2], PRODUCT_MAC_ADDRESS[3],
    	       	   PRODUCT_MAC_ADDRESS[4], PRODUCT_MAC_ADDRESS[5]);
                break;
    		}
    		case NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME:
    		{
    			/* set system name for remote-id content */
    			user.dhcp_snp_opt82_remoteid_type = remoteid_type;
    			memset(user.dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
    			memcpy(user.dhcp_snp_opt82_remoteid_str, (unsigned char*)PRODUCT_SYSTEM_NAME, strlen(PRODUCT_SYSTEM_NAME));/* 24 is SYSINFO_PRODUCT_NAME */
                break;
    		}
    		case NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR:
    		{
    			/* set user-defined string for remote-id content */
    			user.dhcp_snp_opt82_remoteid_type = remoteid_type;
    			memset(user.dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
    			/*memcpy(dhcp_snp_opt82_remoteid_str, remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);*/
    			memcpy(user.dhcp_snp_opt82_remoteid_str, remoteid_str, strlen(remoteid_str));
                break;
    		}
    		default :
    		{
    			syslog_ax_dhcp_snp_err("set ERROR remote-id type %d of DHCP-Snooping option82.\n", remoteid_type);
    			ret = DHCP_SNP_RETURN_CODE_ERROR;
                break;
    		}
        }

        if (DHCP_SNP_RETURN_CODE_OK == ret)
        {
            ret = npd_dhcp_snp_global_cfg_set(&user);
        }
	}
    
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
} 

/**********************************************************************************
 * npd_dbus_dhcp_snp_set_opt82_port_strategy
 *		set DHCP-Snooping option 82 strategy mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82 strategy already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 * 
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_set_opt82_port_strategy
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char strategy_mode = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
//								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_BYTE, &strategy_mode,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
    
    switch (npd_netif_type_get(eth_g_index))
    {
        case NPD_NETIF_ETH_TYPE:
        {
		    npd_key_database_lock();
			ethPort = (struct eth_port_s*)npd_get_port_by_index(eth_g_index);
			if (NULL == ethPort)
			{
				syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
			}
			else if (-1 != ethPort->trunkid)
			{
				/*port is member of trunk*/
				ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
			}
			else
			{
				ret = npd_dhcp_snp_set_opt82_port_strategy(vlanId, eth_g_index, tagMode, strategy_mode);
				if (DHCP_SNP_RETURN_CODE_OK != ret)
				{
					syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
				}
			}
		    npd_key_database_unlock();

            break;
		}
        case NPD_NETIF_TRUNK_TYPE:
        {
		    npd_key_database_lock();
			ret = npd_dhcp_snp_set_opt82_port_strategy(vlanId, eth_g_index, tagMode, strategy_mode);
			if (DHCP_SNP_RETURN_CODE_OK != ret)
			{
				syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
			}
			npd_key_database_unlock();

            break;
		}
        default :
        {
            break;
        }
    }

	if (ethPort)
	{
		free(ethPort);
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/**********************************************************************************
 * npd_dbus_dhcp_snp_set_opt82_port_circuitid_content
 *		set circuit-id type and content of  DHCP-Snooping option82 on special port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82  circuit-id mode already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 *		DHCP_SNP_RETURN_CODE_ERROR					- fail
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_set_opt82_port_circuitid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char circuitid_type = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;	
	char *circuitid_str = NULL;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
//								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_BYTE, &circuitid_type,
								DBUS_TYPE_STRING, &circuitid_str,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    switch (npd_netif_type_get(eth_g_index))
	{
		case NPD_NETIF_ETH_TYPE:
		{
		    npd_key_database_lock();
			ethPort = (struct eth_port_s*)npd_get_port_by_index(eth_g_index);
			if (NULL == ethPort)
			{
				syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
			}
			else if (-1 != ethPort->trunkid)
			{
				/*port is member of trunk*/
				ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
			}
			else
			{
				ret = npd_dhcp_snp_set_opt82_port_circuitid(vlanId, eth_g_index, tagMode,
														circuitid_type, circuitid_str);
				if (DHCP_SNP_RETURN_CODE_OK != ret)
				{
					syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
				}
			}
		    npd_key_database_unlock();
            break;
		}
		case NPD_NETIF_TRUNK_TYPE:
		{
		    npd_key_database_lock();
		    ret = npd_dhcp_snp_set_opt82_port_circuitid(vlanId, eth_g_index, tagMode,
													circuitid_type, circuitid_str);
		    if (DHCP_SNP_RETURN_CODE_OK != ret)
		    {
				syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
			}
		    npd_key_database_unlock();
            break;
		}
        default :
        {
            break;
        }
	}
    
	if (ethPort)
	{
		free(ethPort);
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * npd_dbus_dhcp_snp_add_binding
 *		add dhcp-snooping binding item to bind table
 *		inner interface in dhcp snooping, for save content of bindint table
 *		by command "write" before reboot
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_add_del_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	unsigned char macAddr[NPD_DHCP_SNP_MAC_ADD_LEN] = {0};
	unsigned long ip      = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned int lease_time = NPD_DHCP_SNP_INIT_0;
	unsigned int bind_type = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_USER_ITEM_T user,item;
	unsigned int opt_type = NPD_DHCP_SNP_BINDING_OPERATE_TYPE_INVALID;	

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &macAddr[0],
								DBUS_TYPE_BYTE, &macAddr[1],
								DBUS_TYPE_BYTE, &macAddr[2],
								DBUS_TYPE_BYTE, &macAddr[3],
								DBUS_TYPE_BYTE, &macAddr[4],
								DBUS_TYPE_BYTE, &macAddr[5],
								DBUS_TYPE_UINT32, &ip,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_UINT32, &opt_type,
								DBUS_TYPE_UINT32, &lease_time,
								DBUS_TYPE_UINT32, &bind_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

    if (macAddr[0] | macAddr[1] | macAddr[2] | macAddr[3]| macAddr[4] | macAddr[5])
    {
    	ret = npd_dhcp_snp_check_global_status();
    	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
    	{
    		/*********************************************************************
    		 *	the vlan must exist,
    		 *	not concerned about enabled/disabled DHCP Snooping on the vlan
    		 *********************************************************************/
    		syslog_ax_dhcp_snp_dbg("vlan id %d ,ret %d\n",vlanId,ret);

    		ret = npd_dhcp_snp_check_vlan_status(vlanId);

    		syslog_ax_dhcp_snp_dbg("vlan id %d ,ret %d\n",vlanId,ret);
    		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
    			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
    		}
    		else
    		{	/* add DHCP Snooping dynamic binding item to binding table */
    		    char ip_str[32];
    			syslog_ax_dhcp_snp_dbg("add DHCP-Snooping binding item to binding table.");
                lib_get_string_from_ip(ip_str, ip);
    			syslog_ax_dhcp_snp_dbg("MAC %02x:%02x:%02x:%02x:%02x:%02x IP %s",
    									macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
    									ip_str);
    			syslog_ax_dhcp_snp_dbg("vlan %d eth_g_index %d lease-time %d bind-type %d(0:dynamic 1:static)\n",
    									vlanId, eth_g_index, lease_time, bind_type);

    			if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
    				/*check port membership*/
    				syslog_ax_dhcp_snp_dbg("the port is not member of vlan!\n");
    				ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
    			}
    			else {
    				/* step 1: fill DHCP Snooping table item */
    				user.bind_type = bind_type;
    				user.ip_addr = ip;
    				memcpy(user.chaddr, macAddr, NPD_DHCP_SNP_MAC_ADD_LEN);
    				user.vlanId = vlanId;
    				user.ifindex = eth_g_index;
    				user.lease_time = lease_time;
    				memcpy(&item,&user,sizeof(user));
    				
    				/* step 2: add */
    				ret = npd_dhcp_snp_tbl_item_find(&user);	
    				if (NPD_DHCP_SNP_BINDING_OPERATE_TYPE_ADD == opt_type)
    				{
    					if (DHCP_SNP_RETURN_CODE_OK == ret) {
    						if(user.bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC)
    						{
    							syslog_ax_dhcp_snp_dbg("the item already set static!\n");
    							ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;					
    						}
    						if(user.bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC)
    						{
                                item.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
    							if(!npd_dhcp_snp_tbl_refresh_bind(&user,&item)){
    								syslog_ax_dhcp_snp_dbg("refresh the binding table success!\n");
    								ret = DHCP_SNP_RETURN_CODE_OK;
    							}else{
    								syslog_ax_dhcp_snp_dbg("refresh the binding table faile!\n");
    								ret = DHCP_SNP_RETURN_CODE_ERROR;
    							}							
    						}
    					}
    					else {
                            user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
                            
    						if(0 == npd_dhcp_snp_tbl_static_binding_insert(user)){
    							syslog_ax_dhcp_snp_dbg("insert the binding table seccess!\n");
    							ret = DHCP_SNP_RETURN_CODE_OK;
    						}
    						else{
    							syslog_ax_dhcp_snp_err("insert the binding table error!\n");
    							ret = DHCP_SNP_RETURN_CODE_ERROR;
    						}
    					}
    				}
    				else if (NPD_DHCP_SNP_BINDING_OPERATE_TYPE_DEL == opt_type)
    				{
    					if (DHCP_SNP_RETURN_CODE_OK == ret) {
    						if((user.vlanId == item.vlanId)&&
    							(user.ifindex == item.ifindex)&&
    							(user.ip_addr == item.ip_addr)&&
    							(!memcmp(&(user.chaddr),&(item.chaddr),NPD_DHCP_SNP_MAC_ADD_LEN)))
    						{
    							if(!npd_dhcp_snp_tbl_static_binding_delete(user)){
    								syslog_ax_dhcp_snp_dbg("DEL SUCCESS!\n");
    								ret = DHCP_SNP_RETURN_CODE_OK;
    							}
    							  else{
    								syslog_ax_dhcp_snp_dbg("DEL FAILE!\n");
    								ret = DHCP_SNP_RETURN_CODE_ERROR;
    							}
    						}
    						else
    						{
    							syslog_ax_dhcp_snp_dbg("The binding table item compare faile!\n");
    							ret = DHCP_SNP_RETURN_CODE_ERROR;
    						}
    					}
    					else {
    						syslog_ax_dhcp_snp_err("DELETE ERROR  NO ITEM!\n",opt_type);
    						ret = DHCP_SNP_RETURN_CODE_ERROR;
    					}
    				}
    				else {
    					ret = DHCP_SNP_RETURN_CODE_NOT_FOUND;
    				}
    			}
    		}
    	}
    	else {
    		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
    	}
    }
    else
    {
        ret = DHCP_SNP_RETURN_CODE_MAC_ILEGAL;
    }
    
	syslog_ax_dhcp_snp_dbg("ret = %d\n ",ret);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
		return reply;
}


/**********************************************************************************
 * npd_dbus_dhcp_snp_show_running_global_config
 *		DHCP Snooping show running global config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_show_running_global_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	unsigned char *showStr = NULL;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_dbg("DHCP Snooping show running global config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/**************************************** 
	  * save DHCP Snooping global config information
	  ***************************************/
	npd_dhcp_snp_save_global_cfg(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	if(showStr)
	    free(showStr);
	showStr = NULL;

	return reply;
}

/**********************************************************************************
 * npd_dhcp_snp_save_global_cfg
 *		get string of DHCP Snooping show running global config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void npd_dhcp_snp_save_global_cfg
(
	unsigned char *buf,
	unsigned int bufLen
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&user))
    {
        return ;
    }

	syslog_ax_dhcp_snp_dbg("enter into npd_dhcp_snp_save_global_cfg !\n");

	if (NULL == buf || bufLen == 0) {
		syslog_ax_dhcp_snp_dbg("DHCP Snooping show running global config, parameter is null error\n");
		return ;
	}

	showStr = (char *)buf;
	current = showStr;

	ret = npd_dhcp_snp_get_global_status(&status);
	if(DHCP_SNP_RETURN_CODE_OK == ret )
	{
		if (NPD_DHCP_SNP_ENABLE == status)
		{
			if ((length + sizeof("dhcp-snooping enable\n")) < bufLen)
			{
				length += sprintf(current, "dhcp-snooping enable\n");
				current = showStr + length;
			}
		}
		
		if (NPD_DHCP_SNP_OPT82_ENABLE == user.dhcp_snp_opt82_enable) 
		{
			if ((length + sizeof("dhcp-snooping information enable\n")) < bufLen) 
			{
				length += sprintf(current, "dhcp-snooping information enable\n");
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == user.dhcp_snp_opt82_format_type) 
		{
			if ((length + sizeof("dhcp information option format ascii\n")) < bufLen) 
			{
				length += sprintf(current, "dhcp information option format ascii\n");
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT != user.dhcp_snp_opt82_fill_format_type)
		{
			if ((length + sizeof("dhcp information option mode standard\n")) < bufLen)
			{
				length += sprintf(current, "dhcp information option mode standard\n");
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC != user.dhcp_snp_opt82_remoteid_type)
		{
			if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME == user.dhcp_snp_opt82_remoteid_type)
			{
				if ((length + sizeof("dhcp information option remote-id sysname\n")) < bufLen)
				{
					length += sprintf(current, "dhcp information option remote-id sysname\n");
					current = showStr + length;
				}
			}
			else if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR == user.dhcp_snp_opt82_remoteid_type)
			{
				if ((length + sizeof("dhcp information option remote-id string \n") + 64) < bufLen)
				{
					length += sprintf(current, "dhcp information option remote-id string %s\n", user.dhcp_snp_opt82_remoteid_str);
					current = showStr + length;
				}
			}
		}
	}
	else
	{
		syslog_ax_dhcp_snp_dbg("get global struct faile\n");
	}
    
	return ;
} 
/**********************************************************************************
 * npd_dbus_dhcp_snp_show_running_vlan_config
 *		DHCP Snooping show running vlan config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_show_running_vlan_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	unsigned char *showStr = NULL;
	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_GLOBAL_CFG_LEN);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_GLOBAL_CFG_LEN);

	/**************************************** 
	  * save DHCP Snooping vlan config information
	  ***************************************/
	npd_dhcp_snp_save_vlan_cfg(showStr, NPD_DHCP_SNP_RUNNING_GLOBAL_CFG_LEN);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	    showStr = NULL;

	return reply;
}


/**********************************************************************************
 * npd_dhcp_snp_save_vlan_cfg
 *		get string of DHCP Snooping show running vlan config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
 #define dhcp_snp_text "interface ethernet 01/01/01\n \
    dhcp-snooping trust\n \
    dhcp-snooping trust-nobind\n \
    dhcp information option strategy drop\n \
    dhcp information option strategy keep\n \
    dhcp information option circuit-id string \n \
    exit\n "
void npd_dhcp_snp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;
    char ifname_type[64];
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT show_trust_ports;
    struct NPD_DHCP_SNP_GLOBAL_STATUS gdhcp_snp_conf;
    struct npd_dhcp_relay_global_status gdhcp_relay_conf;

	memset(&show_trust_ports, 0, sizeof(struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT));
  	memset(&gdhcp_snp_conf,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));
    memset(&gdhcp_relay_conf, 0, sizeof(struct npd_dhcp_relay_global_status));
    memset(ifname_type, 0, sizeof(ifname_type));

	
	if (NULL == buf || 0 == bufLen) {
		syslog_ax_dhcp_snp_dbg("DHCP Snooping show running vlan config, parameter is null error\n");
		return ;
	}

    if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&gdhcp_snp_conf))
    {
        return ;
    }

#ifdef HAVE_DHCP_RELAY
    if (DHCP_RELAY_RETURN_CODE_SUCCESS != npd_dhcp_relay_global_status_get(&gdhcp_relay_conf))
    {
        return ;
    }
#endif
		
	showStr = (char *)buf;
	current = showStr;
	syslog_ax_dhcp_snp_dbg("enter !");

    if (NPD_DHCP_SNP_ENABLE == gdhcp_snp_conf.dhcp_snp_enable)
    {
        NPD_VBMP_ITER(gdhcp_snp_conf.vlan_admin_status, vlanId)
        {
            if ((length + sizeof("vlan 0000\n dhcp-snooping enable\n exit\n ")) < bufLen)
            {
                length += sprintf(current, "vlan %d\n dhcp-snooping enable\n exit\n", vlanId);
                current = showStr+ length;
        	}
            else
            {
                return ;
            }
        }
    }

	ret = dbtable_hash_head(npd_dhcp_snp_status_item_index, NULL, &show_trust_ports, NULL);
	while(0 == ret)
	{
        if ((NPD_DHCP_SNP_ENABLE == gdhcp_snp_conf.dhcp_snp_enable)
            || (NPD_DHCP_SNP_OPT82_ENABLE == gdhcp_snp_conf.dhcp_snp_opt82_enable)
#ifdef HAVE_DHCP_RELAY
            || (NPD_DHCP_RELAY_OPT82_ENABLE == gdhcp_relay_conf.dhcp_relay_opt82_enable)
#endif
            )
        {
            /* information option circuit-id has 64 char */
            if ((length + sizeof(dhcp_snp_text) + 1 + 64) >= bufLen)
            {
                return ;
            }

    		/* trust */
            npd_netif_index_to_user_fullname(show_trust_ports.global_port_ifindex, ifname_type);
			
			length += sprintf(current, "interface %s\n", ifname_type);
			current = showStr + length;
                
            if (NPD_DHCP_SNP_ENABLE == gdhcp_snp_conf.dhcp_snp_enable)
            {
    			if (NPD_DHCP_SNP_PORT_MODE_TRUST == show_trust_ports.trust_mode)
    			{
					length += sprintf(current, " dhcp-snooping trust\n");
					current = showStr + length;
    			}
    			else if (NPD_DHCP_SNP_PORT_MODE_NOBIND == show_trust_ports.trust_mode)
    			{
					length += sprintf(current, " dhcp-snooping trust-nobind\n");
					current = showStr + length;
    			}
            }

            if ((NPD_DHCP_SNP_OPT82_ENABLE == gdhcp_snp_conf.dhcp_snp_opt82_enable)
#ifdef HAVE_DHCP_RELAY
            || (NPD_DHCP_RELAY_OPT82_ENABLE == gdhcp_relay_conf.dhcp_relay_opt82_enable)
#endif
                )
            {
        		if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE != show_trust_ports.opt82_strategy)
        		{
        			if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP == show_trust_ports.opt82_strategy)
        			{
    					length += sprintf(current, " dhcp information option strategy drop\n");
    					current = showStr + length;
        			}
                    else if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP == show_trust_ports.opt82_strategy)
        			{
    					length += sprintf(current, " dhcp information option strategy keep\n");
    					current = showStr + length;
        			}
        		}
        			/* option 82 Circuit-ID */
        		if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT != show_trust_ports.opt82_circuitid)
        		{
    				length += sprintf(current, " dhcp information option circuit-id string %s\n",
    								show_trust_ports.opt82_circuitid_str);
    				current = showStr + length;
        		}
        	}

			length += sprintf(current, " exit\n");
			current = showStr+ length;
        }
		memset(ifname_type, 0, sizeof(ifname_type));	
		ret = dbtable_hash_next(npd_dhcp_snp_status_item_index, &show_trust_ports, &show_trust_ports, NULL);
	}	

	return;
}

/**********************************************************************************
 * npd_dbus_dhcp_snp_show_running_save_bind_table
 *		DHCP Snooping show running save bind table
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR			- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_snp_show_running_save_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/******************************************** 
	  * save DHCP Snooping bind table information
	  *******************************************/
	npd_dhcp_snp_save_bind_table(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, &en_dis);
	syslog_ax_dhcp_snp_dbg("DHCP Snooping service %s\n", en_dis ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

/**********************************************************************************
 * npd_dhcp_snp_save_bind_table
 *		get string of DHCP Snooping show running save bind table
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void npd_dhcp_snp_save_bind_table
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	unsigned char port_name[20];
	struct NPD_DHCP_SNP_USER_ITEM_S show_items;
	
	/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */

	if (NULL == buf || NULL == enDis) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, parameter is null error\n");
		return ;
	}

	memset(&show_items, 0, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	showStr = (char *)buf;
	current = showStr;

	ret = npd_dhcp_snp_get_global_status(&status);
	if (NPD_DHCP_SNP_ENABLE == status)
	{
		*enDis = NPD_DHCP_SNP_ENABLE;

		while(0 == npd_dhcp_snp_bind_show(&show_items))
		{
		    char ip_str[32];
            if (NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == show_items.bind_type){
                continue;
            }
			lib_get_string_from_ip(ip_str, show_items.ip_addr);
			if(npd_netif_type_get(show_items.ifindex) == NPD_NETIF_ETH_TYPE)
			{
				parse_eth_index_to_name(show_items.ifindex, port_name);
				if ((length + 95) < bufLen) 
				{
					if(show_items.lease_time ==NPD_DHCP_SNP_LEASE_INFINITE)
					{
						length += sprintf(current, "dhcp-snooping binding %s %02x:%02x:%02x:%02x:%02x:%02x %d %s\n",
										ip_str, \
										show_items.chaddr[0], show_items.chaddr[1], \
										show_items.chaddr[2], show_items.chaddr[3], \
										show_items.chaddr[4], show_items.chaddr[5], \
										show_items.vlanId,port_name);
					}
					else {
						length += sprintf(current, "dhcp-snooping binding %s %02x:%02x:%02x:%02x:%02x:%02x %d %s %d\n",
										ip_str, \
										show_items.chaddr[0], show_items.chaddr[1], \
										show_items.chaddr[2], show_items.chaddr[3], \
										show_items.chaddr[4], show_items.chaddr[5], \
										show_items.vlanId,port_name,show_items.lease_time);
					}
					current = showStr + length;
				}	
			}
			else if(npd_netif_type_get(show_items.ifindex) == NPD_NETIF_TRUNK_TYPE)
			{
				if ((length + 110) < bufLen) 
				{
					if(show_items.lease_time ==NPD_DHCP_SNP_LEASE_INFINITE)
					{
						length += sprintf(current, "dhcp-snooping binding %s %02x:%02x:%02x:%02x:%02x:%02x %d port-channel %d\n",
										ip_str, \
										show_items.chaddr[0], show_items.chaddr[1], \
										show_items.chaddr[2], show_items.chaddr[3], \
										show_items.chaddr[4], show_items.chaddr[5], \
										(int)show_items.vlanId,(int)npd_netif_trunk_get_tid(show_items.ifindex));
					}
					else
					{
						length += sprintf(current, "dhcp-snooping binding %s %02x:%02x:%02x:%02x:%02x:%02x %d port-channel %d %d\n",
										ip_str, \
										show_items.chaddr[0], show_items.chaddr[1], \
										show_items.chaddr[2], show_items.chaddr[3], \
										show_items.chaddr[4], show_items.chaddr[5], \
										(int)show_items.vlanId,(int)npd_netif_trunk_get_tid(show_items.ifindex), \
										show_items.lease_time);
					}
					current = showStr + length;
				}	
			}
		}
	}
	else {
		*enDis = NPD_DHCP_SNP_DISABLE;
	}

	return;
} 

unsigned int npd_dhcp_information_checksum
(
	struct iphdr* ip,
	struct udphdr* udp
)
{
	unsigned int dest = NPD_DHCP_SNP_INIT_0;
	unsigned int source = NPD_DHCP_SNP_INIT_0;
	unsigned short check = NPD_DHCP_SNP_INIT_0;
	struct iphdr tempip ;

	memcpy(&tempip, ip, sizeof(struct iphdr));
	source = ip->saddr;
	dest = ip->daddr;
	check = udp->check;
	udp->check = 0;
	memset(ip, 0, sizeof(struct iphdr));
	
	ip->protocol = IPPROTO_UDP;
	ip->saddr = source;
	ip->daddr = dest;
	ip->tot_len = udp->len; /* cheat on the psuedo-header */
	if (check) {
		udp->check = npd_dhcp_snp_checksum(ip, ntohs(udp->len) + sizeof(struct iphdr)); 
	}		
	memcpy(ip, &tempip, sizeof(struct iphdr));
	ip->check = 0;
	ip->check = npd_dhcp_snp_checksum(ip, sizeof(struct iphdr));
	
	syslog_ax_dhcp_snp_dbg("npd_dhcp_information_checksum ipcheck =  %d ,udpcheck= %d !\n",ip->check,udp->check);
	return 0;

}

#ifdef __cplusplus
	}
#endif
#endif

