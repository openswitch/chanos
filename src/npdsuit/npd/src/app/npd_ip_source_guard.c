/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_DHCP_SNP
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_ip_source_guard.h"

extern sequence_table_index_t *g_eth_ports;

db_table_t         *npd_source_guard_dbtbl = NULL;
hash_table_index_t *npd_sg_haship_index = NULL;

tbl_index_t* npd_sg_dynamic_index = NULL;
tbl_index_t* npd_sg_static_index = NULL;


int npd_sg_entry_destroy_by_ifindex(unsigned int netif_index, unsigned int type) ;
int npd_netif_source_guard_set(unsigned netif_index, int state);


int npd_sg_alloc_static_index(int* sg_index)
{
    int ret = 0;
    int index = 0;

    if (NULL == sg_index)
    {
        return IPSG_RETURN_CODE_ERROR;
    }
    
    if (0 != *sg_index)
    {
        index = *sg_index;
        ret = nam_index_get(npd_sg_static_index, index);
    }
    else
    {
        ret = nam_index_alloc(npd_sg_static_index, &index);
        if (0 == ret)
        {
            *sg_index = index;
        }
    }
    
    return ((0 == ret) ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_alloc_dynamic_index(int* sg_index)
{
    int ret = 0;
    int index = 0;

    if (NULL == sg_index)
    {
        return IPSG_RETURN_CODE_ERROR;
    }
    
    if (0 != *sg_index)
    {
        index = *sg_index - NPD_SG_DYNAMIC_ENTRY_OFFSET;
        ret = nam_index_get(npd_sg_dynamic_index, index);
    }
    else
    {
        ret = nam_index_alloc(npd_sg_dynamic_index, &index);
        if (0 == ret)
        {
            *sg_index = index + NPD_SG_DYNAMIC_ENTRY_OFFSET;
        }
    }

    return ((0 == ret) ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_free_static_index(int sg_index)
{
    return ((0 == nam_index_free(npd_sg_dynamic_index, sg_index)) \
        ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_free_dynamic_index(int sg_index)
{
    return ((0 == nam_index_free(npd_sg_dynamic_index, sg_index - NPD_SG_DYNAMIC_ENTRY_OFFSET)) \
        ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

void npd_ip_sg_event
(
    unsigned int netif_index,
	enum PORT_NOTIFIER_ENT event,
	char* data,
	int datalen
)
{
	switch(event)
    {
        case PORT_NOTIFIER_L2DELETE:
        {
            if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
            {
                npd_netif_source_guard_set(netif_index, 0);
                npd_sg_entry_destroy_by_ifindex(netif_index, SOURCE_GUARD_STATIC);
            }
        	break;
        }
    	default:
        {
        	break;
    	}
    }
    
	return ;
}

void npd_ip_sg_relate_event
(
    unsigned int father_index,
	unsigned int  son_ifindex,
	enum PORT_RELATE_ENT event,
	char* data,
	int datalen
)
{
	return ;    
}

netif_event_notifier_t npd_ip_sg_notifier =
{
    .netif_event_handle_f = &npd_ip_sg_event,
    .netif_relate_handle_f = &npd_ip_sg_relate_event
};

int npd_sg_index_init()
{
    int index = 0;
    npd_sg_dynamic_index = nam_index_create(NPD_SG_ENTRY_NUM + 1);
	npd_sg_static_index = nam_index_create(NPD_SG_ENTRY_NUM + 1);
    npd_sg_alloc_static_index(&index);      /* XXX: SG avoid index 0 */
    npd_sg_alloc_dynamic_index(&index);     /* XXX: SG avoid index 256 */

    register_netif_notifier(&npd_ip_sg_notifier);    

    return 0;
}

long npd_source_guard_handle_update(void *new, void *old)
{
    struct ip_source_guard_entry_s *entry_new = (struct ip_source_guard_entry_s *)new;
    struct ip_source_guard_entry_s *entry_old = (struct ip_source_guard_entry_s *)old;

    if (NPD_SUPPORT_SG_FILTER)
    {
        nam_source_guard_delete(entry_old);
        nam_source_guard_add(entry_new);
    }
    else
    {
        if (app_act_master_running())
        {
            return 0;
        }
        
        if ((entry_new->sg_index != entry_old->sg_index)
            || (entry_new->is_static != entry_old->is_static))
        {
            if ((entry_new->is_static == entry_old->is_static)
                && (entry_new->sg_index != entry_old->sg_index))
            {
                if (SOURCE_GUARD_STATIC == entry_new->is_static)
                {
                    npd_sg_free_static_index(entry_old->sg_index);
                    npd_sg_alloc_static_index(&(entry_new->sg_index));
                    
                }
                else
                {
                    npd_sg_free_dynamic_index(entry_old->sg_index);
                    npd_sg_alloc_dynamic_index(&(entry_new->sg_index));
                }
            }
            else    /* !is_static */
            {
                if ((SOURCE_GUARD_STATIC == entry_new->is_static)
                    && (SOURCE_GUARD_DYNAMIC == entry_old->is_static))
                {
                    npd_sg_free_dynamic_index(entry_old->sg_index);
                    npd_sg_alloc_static_index(&(entry_new->sg_index));
                }
            }
        }
    }
    
    return 0;
}

long npd_source_guard_handle_insert(void *data)
{
    struct ip_source_guard_entry_s *entry = (struct ip_source_guard_entry_s *)data;

    if (NPD_SUPPORT_SG_FILTER)
    {
        nam_source_guard_add(entry);
    }
    else
    {
        if (app_act_master_running())
        {
            return 0;
        }
        
        if (SOURCE_GUARD_STATIC == entry->is_static)
        {
            npd_sg_alloc_static_index(&(entry->sg_index));
        }
        else
        {
            npd_sg_alloc_dynamic_index(&(entry->sg_index));
        }
    }
        
    return 0;
}

long npd_source_guard_handle_delete(void *data)
{
    struct ip_source_guard_entry_s *entry = (struct ip_source_guard_entry_s *)data;

    if (NPD_SUPPORT_SG_FILTER)
    {
        nam_source_guard_delete(entry);
    }
    else
    {
        if (app_act_master_running())
        {
            return 0;
        }
        if (SOURCE_GUARD_STATIC == entry->is_static)
        {
            npd_sg_free_static_index(entry->sg_index);
        }
        else
        {
            npd_sg_free_dynamic_index(entry->sg_index);
        }
    }
    
    return 0;
}

int npd_source_guard_handle_ntoh(void *data)
{
	struct ip_source_guard_entry_s *item = (struct ip_source_guard_entry_s *)data;

    item->ifIndex = ntohl(item->ifIndex);
    item->ipAddr = ntohl(item->ipAddr);
    item->is_static   = ntohl(item->is_static);
    item->vid    = ntohs(item->vid);
    item->sg_index   = ntohl(item->sg_index);
	return 0;
}

int npd_source_guard_handle_hton(void *data)
{
	struct ip_source_guard_entry_s *item = (struct ip_source_guard_entry_s *)data;

    item->ifIndex = htonl(item->ifIndex);
    item->ipAddr = htonl(item->ipAddr);
    item->is_static   = htonl(item->is_static);
    item->vid = htons(item->vid);
    item->sg_index   = htonl(item->sg_index);
	return 0;
}

unsigned int npd_sg_key_generate
(
	void *data
)
{
	unsigned int key = 0;

	struct ip_source_guard_entry_s *item = (struct ip_source_guard_entry_s *)data;

	if (NULL == item)
    {
    	syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.\r\n");
    	return ~0UI;
    }

    key = jhash_1word(item->ipAddr, 0x35798642);
/*    key %= (NPD_ARPSNP_TABLE_SIZE/16);  */
    key %= NPD_SG_MAX_TABLE_SIZE/16;

	return key;
}

unsigned int npd_source_guard_compare
(
	void *data1,
	void *data2
)
{
	unsigned int equal = TRUE;
	struct ip_source_guard_entry_s *itemA = (struct ip_source_guard_entry_s *)data1;
	struct ip_source_guard_entry_s *itemB  = (struct ip_source_guard_entry_s *)data2;

	if ((NULL==itemA) || (NULL==itemB))
    {
    	syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.\r\n");
    	return FALSE;
    }
        
	if (itemA->ipAddr != itemB->ipAddr) 
    {   /* ip address*/
    	equal = FALSE;
    }

	return equal;
}

int npd_ip_source_guard_init()
{
    int ret = 0;

    ret = create_dbtable("ipsourceguardtbl", NPD_SG_MAX_TABLE_SIZE, sizeof(struct ip_source_guard_entry_s),
                	npd_source_guard_handle_update, 
                	NULL,
                	npd_source_guard_handle_insert, 
                	npd_source_guard_handle_delete,
                	NULL,
                	NULL, 
                	NULL, 
                	npd_source_guard_handle_ntoh,
                	npd_source_guard_handle_hton,
                	DB_SYNC_ALL,
                    &(npd_source_guard_dbtbl));
	if (0 != ret)
    {
    	syslog_ax_arpsnooping_err("create npd ip-source-guard database fail\n");
    	return NPD_FAIL;
    }
                                                            /* NPD_ARPSNP_TABLE_SIZE/16 */
	ret = dbtable_create_hash_index("ipsghash", npd_source_guard_dbtbl,NPD_SG_MAX_TABLE_SIZE/16, npd_sg_key_generate,
                                                	npd_source_guard_compare, &npd_sg_haship_index);
	if (0 != ret)
    {
    	syslog_ax_arpsnooping_err("create npd ip-source-guard hash fail\n");
    	return NPD_FAIL;
    }

    npd_sg_index_init();
    
    return ret;
}

int npd_sg_entry_get(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_search(npd_sg_haship_index, entry, NULL, entry)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_entry_set(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_update(npd_sg_haship_index, NULL, entry)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_entry_remove(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_delete(npd_sg_haship_index, entry, NULL)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_entry_insert(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_insert(npd_sg_haship_index, entry)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_entry_list_head(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_head(npd_sg_haship_index, NULL, entry, NULL)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

int npd_sg_entry_list_next(struct ip_source_guard_entry_s* entry)
{
    return ((0 == dbtable_hash_next(npd_sg_haship_index, entry, entry, NULL)) \
       ? IPSG_RETURN_CODE_SUCCESS : IPSG_RETURN_CODE_ERROR);
}

void create_sg_map()
{
    class_map_index_t class_map = {{0}};
    policy_map_index_t policy_map = {{0}};
   
    //default sg for permit
    class_map_create(SG_DEFAULT_PERMIT);
    class_map_find_by_name(SG_DEFAULT_PERMIT, &class_map);
    class_map_add_match(&class_map, "protocol", "17", "0xffff");
    class_map_add_match(&class_map, "dstl4port", "68", "0xffff");
    policy_map_create(SG_DEFAULT_PERMIT);
    policy_map_class(SG_DEFAULT_PERMIT, SG_DEFAULT_PERMIT);
    policy_map_add_set(&policy_map, "drop", "0");
    policy_map_find_by_name(SG_DEFAULT_PERMIT, &policy_map);
 
    //default sg for deny
    class_map_create(SG_DEFAULT_DENY);
    policy_map_create(SG_DEFAULT_DENY);
    class_map_find_by_name(SG_DEFAULT_DENY, &class_map);
    class_map_add_match(&class_map, "destination-address mac", "00:00:00:00:00:00", "00:00:00:00:00:00");
    policy_map_class(SG_DEFAULT_DENY, SG_DEFAULT_DENY);
    policy_map_find_by_name(SG_DEFAULT_DENY, &policy_map);
    policy_map_add_set(&policy_map, "drop", "1");  
}

int npd_sg_enable(int netif_index)
{
    class_map_index_t class_map = {{0}};   
    int ret = class_map_find_by_name(SG_DEFAULT_PERMIT, &class_map);
    if(ret == CLASSMAP_RETURN_CODE_NOTEXIST)
    {
        create_sg_map();
    }   
    service_policy_create(SG_DEFAULT_PERMIT, ACL_DIRECTION_INGRESS_E, netif_index);
    service_policy_create(SG_DEFAULT_DENY, ACL_DIRECTION_INGRESS_E, netif_index);   

    return 0;
}

int npd_sg_disable(int netif_index)
{
    service_policy_destroy(SG_DEFAULT_PERMIT, ACL_DIRECTION_INGRESS_E, netif_index);
    service_policy_destroy(SG_DEFAULT_DENY, ACL_DIRECTION_INGRESS_E, netif_index);

    return 0;
}

int npd_sg_add_entry(int add,struct ip_source_guard_entry_s *item,int sg_index)
{
    class_map_index_t class_map = {{0}};
    policy_map_index_t policy_map = {{0}};
    int ret = 0;
    char name[32]={0};
    char drop_name[32] = {0};
    char ip_str[32]={0};
    char mac_str[32]={0};
    char mac_mask[32] = {0};
    char vlan_id[8]={0};
    unsigned int netifindex = 0;
    sprintf(name,"SG_%d",sg_index);
    sprintf(drop_name, "SG_%d", (SG_VLAN_DROP_ENTRY_OFFSET + sg_index));
    if(add)
    {
#if 0
        //default sg for permit 
        npd_sg_enable(item->ifIndex);
#endif
        /*create permit acl for port and deploy it*/
        class_map_create(name);
        class_map_find_by_name(name, &class_map);
        sprintf(mac_str,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", item->mac[0], item->mac[1], item->mac[2], item->mac[3], item->mac[4],item->mac[5]);
        memcpy(mac_mask, "ff:ff:ff:ff:ff:ff", strlen("ff:ff:ff:ff:ff:ff"));
        if(!strcmp(mac_str,"00:00:00:00:00:00"))
        {
            memcpy(mac_mask,"00:00:00:00:00:00", strlen("00:00:00:00:00:00"));
        }
        else
        {
            class_map_add_match(&class_map, "source-address mac", mac_str, mac_mask);
        }
        memset(vlan_id, 0, sizeof(vlan_id));
        sprintf(vlan_id, "%d", item->vid);
        class_map_add_match(&class_map, "outer-vlan", vlan_id, "0xfff");
        lib_get_string_from_ip(ip_str, item->ipAddr);
        class_map_add_match(&class_map, "srcip",ip_str, "0xffff");
        policy_map_create(name);
        policy_map_class(name, name);
        policy_map_find_by_name(name, &policy_map);
        policy_map_add_set(&policy_map, "drop", "0");      
        service_policy_create(name, ACL_DIRECTION_INGRESS_E, item->ifIndex);

        /*create drop acl for vlan and deploy it*/
        ret = class_map_create(drop_name);
        ret = class_map_find_by_name(drop_name, &class_map);
        sprintf(mac_str,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", item->mac[0], item->mac[1], item->mac[2], item->mac[3], item->mac[4],item->mac[5]);
        memcpy(mac_mask, "ff:ff:ff:ff:ff:ff", strlen("ff:ff:ff:ff:ff:ff"));
        if(!strcmp(mac_str,"00:00:00:00:00:00"))
        {
            memcpy(mac_mask,"00:00:00:00:00:00", strlen("00:00:00:00:00:00"));
        }
        else
        {
            ret = class_map_add_match(&class_map, "source-address mac", mac_str, mac_mask);
        }
        netifindex = npd_netif_vlan_index(item->vid);
        lib_get_string_from_ip(ip_str, item->ipAddr);
        ret = class_map_add_match(&class_map, "srcip",ip_str, "0xffff");
        ret = policy_map_create(drop_name);
        ret = policy_map_class(drop_name, drop_name);
        ret = policy_map_find_by_name(drop_name, &policy_map);
        ret = policy_map_add_set(&policy_map, "drop", "1");      
        ret = service_policy_create(drop_name, ACL_DIRECTION_INGRESS_E, netifindex);
        printf("vlan service_policy_create = %d\n", ret);
    }
    else
    {        
        service_policy_destroy(name, ACL_DIRECTION_INGRESS_E, item->ifIndex);
        policy_map_delete(name);
        class_map_delete(name);

        /*delete drop acl from vlan*/
        netifindex = npd_netif_vlan_index(item->vid);
        service_policy_destroy(drop_name, ACL_DIRECTION_INGRESS_E, netifindex);
        policy_map_delete(drop_name);
        class_map_delete(drop_name);
    }

    return 0;
}


int npd_sg_entry_destroy_by_ifindex(unsigned int netif_index, unsigned int type) 
{
    struct ip_source_guard_entry_s item;
    struct ip_source_guard_entry_s item_del;
    int ret;
    char map_name[16];
    int del_flag = 0;
    unsigned int ifindex = 0;

    memset(&item, 0, sizeof(struct ip_source_guard_entry_s));
    
    ret = npd_sg_entry_list_head(&item);
    while (IPSG_RETURN_CODE_SUCCESS == ret)
    {
        if ((netif_index == item.ifIndex)
            && (type == item.is_static))
        {
            if (!NPD_SUPPORT_SG_FILTER)
            {
                memset(map_name, 0, 16);
                sprintf(map_name, "SG_%d", item.sg_index);
                service_policy_destroy(map_name, ACL_DIRECTION_INGRESS_E, netif_index);
                policy_map_delete(map_name);
                class_map_delete(map_name);

                memset(map_name, 0, 16);
                sprintf(map_name, "SG_%d", (SG_VLAN_DROP_ENTRY_OFFSET + item.sg_index));
                ifindex = npd_netif_vlan_index(item.vid);
                service_policy_destroy(map_name, ACL_DIRECTION_INGRESS_E, ifindex);
                policy_map_delete(map_name);
                class_map_delete(map_name);
            }
            del_flag = 1;
            memcpy(&item_del, &item, sizeof(struct ip_source_guard_entry_s));
        }
        ret = npd_sg_entry_list_next(&item);
        if (del_flag)
        {
            npd_sg_entry_remove(&item_del);
            del_flag = 0;
        }
    }
    
    return IPSG_RETURN_CODE_SUCCESS;
}

int npd_netif_source_guard_set(unsigned netif_index, int state)
{
    struct eth_port_s *eth_port;

    eth_port = npd_get_port_by_index(netif_index);

    if (NULL == eth_port)
    {
        return IPSG_RETURN_CODE_ERROR;
    }

    if (eth_port->ip_sg == state)
    {
        free(eth_port);
        return IPSG_RETURN_CODE_ALREADY_SET;
    }

    if (!state)
    {
        npd_sg_entry_destroy_by_ifindex(netif_index, SOURCE_GUARD_DYNAMIC);
    }
        
    if (!NPD_SUPPORT_SG_FILTER)
    {
        if (state)
        {
            npd_sg_enable(netif_index);
        }
        else
        {
            npd_sg_disable(netif_index);
        }
    }

    eth_port->ip_sg = state;
    npd_put_port(eth_port);

    return IPSG_RETURN_CODE_SUCCESS;
}

int npd_sg_get_by_entry(struct ip_source_guard_entry_s *sg_entry)
{
    struct ip_source_guard_entry_s item;
    int ret = 0;
    
    ret = npd_sg_entry_list_head(&item);
    while (IPSG_RETURN_CODE_SUCCESS == ret)
    {
        if ((sg_entry->ipAddr == item.ipAddr)
            && (sg_entry->ifIndex == item.ifIndex))
        {
            memcpy(sg_entry, &item, sizeof(struct ip_source_guard_entry_s));
            return IPSG_RETURN_CODE_SUCCESS;
        }
        ret = npd_sg_entry_list_next(&item);
    }
    
    return IPSG_RETURN_CODE_NOT_EXIST;
}

int npd_ip_sg_show_port_enable(unsigned int* sg_flag_buf)
{
    int ret = 0;
    struct eth_port_s eth_port;

    memset(&eth_port, 0, sizeof(struct eth_port_s));

    ret = dbtable_sequence_traverse_next(g_eth_ports, -1, &eth_port);
    while (0 == ret)
    {
        if (eth_port.ip_sg)
        {
            sg_flag_buf[0]++;
            sg_flag_buf[sg_flag_buf[0]] = eth_port.eth_port_ifindex;
        }
        ret = dbtable_sequence_traverse_next(g_eth_ports, eth_port.eth_port_ifindex, &eth_port);
    }
    
    return 0;
}

int npd_source_guard_entry_add
(
    unsigned int ipno, 
    unsigned int eth_g_index, 
    unsigned short vid,
    char* mac,
    int sg_type
)
{
    int ret = 0;
    struct ip_source_guard_entry_s item;
    struct ip_source_guard_entry_s item_original;

    memset(&item, 0, sizeof(struct ip_source_guard_entry_s));
    memset(&item_original, 0, sizeof(struct ip_source_guard_entry_s));

    item.ifIndex = eth_g_index;
    item.ipAddr = ipno;
    item.vid = vid;
    item.is_static = sg_type;
    memcpy(item.mac, mac, 6);

    item_original.ipAddr = ipno;

    if (IPSG_RETURN_CODE_SUCCESS == npd_sg_entry_get(&item_original))
    {
        if (SOURCE_GUARD_DYNAMIC == sg_type)    /* XXX: just for dynamic sg site */
        {
            if (SOURCE_GUARD_STATIC == item_original.is_static)
            {
                return IPSG_RETURN_CODE_SUCCESS;
            }
        }
        
        if (sg_type == item_original.is_static)
        {
            if (!NPD_SUPPORT_SG_FILTER)
            {
                npd_sg_add_entry(0, &item_original, item_original.sg_index);

                item.sg_index = item_original.sg_index;
                
                npd_sg_add_entry(1, &item, item.sg_index);
            }
            return npd_sg_entry_set(&item);
        }
        else if ((SOURCE_GUARD_DYNAMIC == item_original.is_static)
            && (SOURCE_GUARD_STATIC == sg_type))
        {
            if (!NPD_SUPPORT_SG_FILTER)
            {
                npd_sg_free_dynamic_index(item_original.sg_index);
                npd_sg_add_entry(0, &item_original, item_original.sg_index);
            }
            
            ret = npd_sg_entry_remove(&item_original);
            if (IPSG_RETURN_CODE_SUCCESS != ret)
            {
                return ret;
            }

            if (!NPD_SUPPORT_SG_FILTER)
            {
                if (IPSG_RETURN_CODE_SUCCESS != npd_sg_alloc_static_index(&(item.sg_index)))
                {
                    return IPSG_RETURN_CODE_OVER_MAX;
                }
                
                npd_sg_add_entry(1, &item, item.sg_index);
            }

            return npd_sg_entry_insert(&item);
        }
    }
    else
    {
        if (!NPD_SUPPORT_SG_FILTER)
        {
            if (SOURCE_GUARD_STATIC == sg_type)
            {
                item.sg_index = 0;
                if (IPSG_RETURN_CODE_SUCCESS != npd_sg_alloc_static_index(&(item.sg_index)))
                {
                    return IPSG_RETURN_CODE_OVER_MAX;
                }
            }
            else
            {
                item.sg_index = 0;
                if (IPSG_RETURN_CODE_SUCCESS != npd_sg_alloc_dynamic_index(&(item.sg_index)))
                {
                    return IPSG_RETURN_CODE_OVER_MAX;
                }
            }
            
            if (0 != npd_sg_add_entry(1, &item, item.sg_index))
            {
                return IPSG_RETURN_CODE_ERROR;
            }
        }

        return npd_sg_entry_insert(&item);
    }
    
    return IPSG_RETURN_CODE_SUCCESS;
}

int npd_source_guard_entry_del(unsigned int ipno, unsigned int eth_g_index, int sg_type)
{
    int ret = IPSG_RETURN_CODE_SUCCESS;
    struct ip_source_guard_entry_s item;

    memset(&item, 0, sizeof(struct ip_source_guard_entry_s));

    item.ifIndex = eth_g_index;
    item.ipAddr = ipno;
    
    ret = npd_sg_get_by_entry(&item);
    if (IPSG_RETURN_CODE_SUCCESS == ret)
    {
        if ((SOURCE_GUARD_DYNAMIC == sg_type) && (SOURCE_GUARD_STATIC == item.is_static))
        {
            /* There is nothing business on static ip-source-guard
            * when dhcp-snooping binding item was remove
            */
            return IPSG_RETURN_CODE_SUCCESS;
        }

        if (!NPD_SUPPORT_SG_FILTER)
        {
            if (SOURCE_GUARD_DYNAMIC == item.is_static)
            {
                ret = npd_sg_free_dynamic_index(item.sg_index);
            }
            else
            {
                ret = npd_sg_free_static_index(item.sg_index);
            }

            npd_sg_add_entry(0, &item, item.sg_index);
        }

        ret = npd_sg_entry_remove(&item);
    }
    
    return ret;
}

DBusMessage * npd_dbus_interface_sg_service(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = IPSG_RETURN_CODE_SUCCESS;
    unsigned int netif_index = 0;
	unsigned int state = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                 DBUS_TYPE_UINT32, &state,
                                 DBUS_TYPE_UINT32, &netif_index,
                                 DBUS_TYPE_INVALID)))
    {
        syslog_ax_arpsnooping_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_arpsnooping_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
    
    ret = npd_netif_source_guard_set(netif_index, state);
        
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_source_guard_entry(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int ret = IPSG_RETURN_CODE_SUCCESS;
	unsigned int ipno = 0;
	unsigned short vlanId = 0;
	unsigned int eth_g_index = 0;
	unsigned int isAdd = FALSE;
    unsigned char mac[6] = {0};

	dbus_error_init(&err);
    
	if (!(dbus_message_get_args ( msg, &err,
                 DBUS_TYPE_UINT32,&eth_g_index,
                 DBUS_TYPE_UINT32,&ipno,
                 DBUS_TYPE_UINT16, &vlanId,
                 DBUS_TYPE_BYTE, &mac[0],
                 DBUS_TYPE_BYTE, &mac[1],
                 DBUS_TYPE_BYTE, &mac[2],
                 DBUS_TYPE_BYTE, &mac[3],
                 DBUS_TYPE_BYTE, &mac[4],
                 DBUS_TYPE_BYTE, &mac[5],
                 DBUS_TYPE_UINT32,&isAdd,
                 DBUS_TYPE_INVALID))) {
    	syslog_ax_arpsnooping_err("Unable to get input args ");
    	if (dbus_error_is_set(&err)) {
        	syslog_ax_arpsnooping_err("%s raised: %s",err.name,err.message);
        	dbus_error_free(&err);
        }
    	return NULL;
    }

    if (isAdd)
    {
        ret = npd_source_guard_entry_add(ipno, eth_g_index, vlanId, mac, SOURCE_GUARD_STATIC);
    }
    else
    {
        ret = npd_source_guard_entry_del(ipno, eth_g_index, SOURCE_GUARD_STATIC);
    }
    
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
                                     DBUS_TYPE_UINT32,
                                     &ret);                                     
	return reply;                                 

}

DBusMessage * npd_dbus_ip_sg_port_show_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply = NULL;
    unsigned int ret = 0;
    unsigned int length = MAX_ETH_GLOBAL_INDEX + 1;
    unsigned int sg_flag_buf[MAX_ETH_GLOBAL_INDEX + 1];
    unsigned int* ptr_sg_buf = sg_flag_buf;

    memset(sg_flag_buf, 0, sizeof(sg_flag_buf));
    ret = npd_ip_sg_show_port_enable(sg_flag_buf);

    reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
    {
    	npd_syslog_err("NEW method dbus failed!\n");
    	return reply;
    }

    dbus_message_append_args(reply,
                        	DBUS_TYPE_UINT32,
                            &ret,
                            
                        	DBUS_TYPE_ARRAY,
                        	DBUS_TYPE_UINT32,
                            &ptr_sg_buf,
                        	length,
                            
                        	DBUS_TYPE_INVALID);
	return reply;
}


DBusMessage * npd_dbus_source_guard_entry_show_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	char *buffer = NULL;
     /* XXX: one ip-source-guard item will be 64 character */
	unsigned int buffsize = 64 * NPD_SG_MAX_TABLE_SIZE;
    struct ip_source_guard_entry_s item = {0};
    int ret;
    char* sg_type[2] = {"Static", "Dynamic"};
    unsigned char zero_mac[6] = {0, 0, 0, 0, 0, 0};
    int len;
    char *curr;

	buffer = (char*)malloc(buffsize);
	if(NULL == buffer) {
    	return NULL;
    }
	memset(buffer,0,buffsize);

    curr = buffer;
    
    memset(&item, 0, sizeof(struct ip_source_guard_entry_s));
    ret = npd_sg_entry_list_head(&item);
    while (IPSG_RETURN_CODE_SUCCESS == ret)
    {
        if( 0 != memcmp(item.mac, zero_mac, sizeof(zero_mac)))
        {
            char name[50];
            char ip_str[20];
 
            parse_eth_index_to_name(item.ifIndex, name);
            lib_get_string_from_ip(ip_str, item.ipAddr);
            len = sprintf(curr, "%-8s ip-source-guard %-10s %-15s %-4d %.02x:%.02x:%.02x:%.02x:%.02x:%.02x\n",
                    (item.is_static == SOURCE_GUARD_DYNAMIC ? sg_type[1] : sg_type[0]),
                    name, ip_str, item.vid, item.mac[0], item.mac[1], item.mac[2], item.mac[3],
                    item.mac[4], item.mac[5]);
        }
        else
        {
            char name[50];
            char ip_str[20];
 
            parse_eth_index_to_name(item.ifIndex, name);
            lib_get_string_from_ip(ip_str, item.ipAddr);
            len = sprintf(curr, "%-8s ip-source-guard %-10s %-15s %-4d\n",
                    (item.is_static == SOURCE_GUARD_DYNAMIC ? sg_type[1] : sg_type[0]),
                    name, ip_str, item.vid);
            
            
        }
        curr+=len;
        ret = npd_sg_entry_list_next(&item);
    }
    

	reply = dbus_message_new_method_return(msg);
    
	dbus_message_iter_init_append (reply, &iter);
    
	dbus_message_iter_append_basic (&iter,
                                     DBUS_TYPE_STRING,
                                     &buffer); 
                                 
	free(buffer);
	buffer = NULL;

	return reply;                             

}


DBusMessage * npd_dbus_source_guard_entry_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	char *buffer = NULL;
     /* XXX: one ip-source-guard item will be 64 character */
	unsigned int buffsize = 64 * NPD_SG_MAX_TABLE_SIZE;
    struct ip_source_guard_entry_s item = {0};
    int ret;
    unsigned char zero_mac[6] = {0};
    int len;
    char *curr;

	buffer = (char*)malloc(buffsize);
	if(NULL == buffer) {
    	return NULL;
    }
	memset(buffer,0,buffsize);

    curr = buffer;
    ret = npd_sg_entry_list_head(&item);
    while (IPSG_RETURN_CODE_SUCCESS == ret)
    {
        if (SOURCE_GUARD_DYNAMIC == item.is_static)
        {
            ret = npd_sg_entry_list_next(&item);
            continue;
        }
        
        if( 0 != memcmp(item.mac, zero_mac, sizeof(zero_mac)))
        {
            char name[50];
            char ip_str[20];
 
            parse_eth_index_to_name(item.ifIndex, name);
            lib_get_string_from_ip(ip_str, item.ipAddr);
            len = sprintf(curr, "ip-source-guard %s %s %d %.02x:%.02x:%.02x:%.02x:%.02x:%.02x\n",
                    name, ip_str, item.vid, item.mac[0], item.mac[1], item.mac[2], item.mac[3],
                    item.mac[4], item.mac[5]);
        }
        else
        {
            char name[50];
            char ip_str[20];
 
            parse_eth_index_to_name(item.ifIndex, name);
            lib_get_string_from_ip(ip_str, item.ipAddr);
            len = sprintf(curr, "ip-source-guard %s %s %d\n", name, ip_str, item.vid);
        }
        curr+=len;
        ret = npd_sg_entry_list_next(&item);
    }
    

	reply = dbus_message_new_method_return(msg);
    
	dbus_message_iter_init_append (reply, &iter);
    
	dbus_message_iter_append_basic (&iter,
                                     DBUS_TYPE_STRING,
                                     &buffer); 
                                 
	free(buffer);
	buffer = NULL;

	return reply;                             

}
#endif


