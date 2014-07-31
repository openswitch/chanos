/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_switch_port.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD by SWITCH PORT module.
*
* DATE:
*		06/21/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.256 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*
  Network Platform Daemon Ethernet Port Management
*/
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_switch_port.h"

#define MAX_PORT_ISOLATE_GROUP 1


db_table_t *switch_ports_db = NULL;
array_table_index_t *switch_ports = NULL;
hash_table_index_t *switch_ports_hash = NULL;

db_table_t *isolate_ports_db = NULL;
array_table_index_t *isolate_ports = NULL;
unsigned int isolate_group_global_no = 0;

extern hash_table_index_t *mac_vlan_hash;
extern hash_table_index_t *subnet_vlan_hash;
extern hash_table_index_t *proto_vlanport_hash;
#ifdef HAVE_QINQ
extern hash_table_index_t *vlan_eline_vlan_hash;
#endif
extern unsigned int g_tpid ;
extern unsigned int g_inner_tpid;


void npd_switchport_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
    int type = npd_netif_type_get(netif_index);
	unsigned short vid = 0;
	
    if(type != NPD_NETIF_VLAN_TYPE)
    {
		return;
    }

    vid = npd_netif_vlan_get_vid(netif_index);
    switch(evt)
    {
    case PORT_NOTIFIER_DELETE:
	
		// delete associated protocol vlan
		{
			int ret;
		    proto_vlan_port_t item;
		    memset(&item, 0, sizeof(proto_vlan_port_t));

	        while(1)
	        {
	       		item.vid = vid;
				ret = dbtable_hash_head(proto_vlanport_hash, &item,  
					&item, vlan_assoc_protocol_vlanid_show_filter);
				if (0 != ret)
					break;
				ret = dbtable_hash_delete(proto_vlanport_hash, &item,&item);
	        }
		}
		
		//delete associated mac
	    {
			int ret;
		    macbase_vlan_t item;
		    macbase_vlan_t item_out;
		    memset(&item, 0, sizeof(macbase_vlan_t));
		    memset(&item_out, 0, sizeof(macbase_vlan_t));	 
			item.vid = vid;
			
		    ret = dbtable_hash_head(mac_vlan_hash,  
				&item, &item_out, &vlan_assoc_mac_show_filter);
		    while (0 == ret)  
		    {
				memcpy(item.mac, item_out.mac, sizeof(item_out.mac));
		    	ret = dbtable_hash_delete(mac_vlan_hash, &item_out, &item_out);
				if (0 != ret)
					break;
				ret = nam_vlan_mac_tbl_index_free(item.tbl_index);
				
			    ret = dbtable_hash_head(mac_vlan_hash,  
					&item, &item_out, &vlan_assoc_mac_show_filter);	
			}     
	    }		

		//delete associated subnet-vlan
        {
            int ret;
			subnetbase_vlan_t item;
			subnetbase_vlan_t item_out;
		    memset(&item, 0, sizeof(subnetbase_vlan_t));
		    memset(&item_out, 0, sizeof(subnetbase_vlan_t));
			
		    item.ipaddr = 0;
		    item.mask = 0;

			item.vid = vid;
            ret = dbtable_hash_head(subnet_vlan_hash, 
					&item, &item_out, &vlan_assoc_subnet_show_filter);
			
            while(0 == ret)
            {
				item.ipaddr = item_out.ipaddr;
				item.mask = item_out.mask;
				ret = dbtable_hash_delete(subnet_vlan_hash, &item_out, &item_out);
				if (0 != ret)
					break;
				ret = nam_vlan_subnet_tbl_index_free(item.tbl_index);
				
				ret = dbtable_hash_head(subnet_vlan_hash, 
					&item, &item_out, vlan_assoc_subnet_show_filter);
			}
        }		
#ifdef HAVE_QINQ
		// delete associated e-line
		{
			int ret;
			vlan_eline_db_entry_t item;
			vlan_eline_db_entry_t item_out;
		    memset(&item, 0, sizeof(vlan_eline_db_entry_t));
		    memset(&item_out, 0, sizeof(vlan_eline_db_entry_t));
			
			item.outer_vid = vid;
            ret = dbtable_hash_head(vlan_eline_vlan_hash, 
					&item, &item_out, npd_vlan_eline_outervlan_filter);
			
            while(0 == ret)
            {
				item.outer_vid = vid;
				ret = dbtable_hash_delete(vlan_eline_vlan_hash, &item_out, &item_out);
				if (0 != ret)
					break;
				ret = dbtable_hash_head(vlan_eline_vlan_hash, 
						&item, &item_out, npd_vlan_eline_outervlan_filter);
			}
		}
#endif
        break;

    default:
        break;
    }

    return;
}

void npd_switchport_relate_event(
    unsigned int father_netif_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
	int ret = 0;
	switch(event)
	{
	case PORT_NOTIFIER_JOIN:
		if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(father_netif_index))
		{
			if(NPD_NETIF_ETH_TYPE != npd_netif_type_get(netif_index))
				return;
#ifdef HAVE_PORT_ISOLATE		
			struct port_isolate_group_s group_default = {0};
			unsigned int array_port = 0;
			int trunkId = 0;
			array_port = eth_port_array_index_from_ifindex(netif_index);
			ret = dbtable_array_get(isolate_ports, isolate_group_global_no, &group_default);
			if (ret != VLAN_RETURN_CODE_ERR_NONE)
			{
				return;
			}
			trunkId = npd_netif_trunk_get_tid(father_netif_index);
			if (NPD_PBMP_MEMBER(group_default.isolate_trunks, trunkId) 
				&& !NPD_PBMP_MEMBER(group_default.isolate_ports, array_port))
			{
				NPD_PBMP_PORT_ADD(group_default.isolate_ports, array_port);
				ret = dbtable_array_update(isolate_ports, isolate_group_global_no, NULL, &group_default);				
			}			
#endif
#ifdef HAVE_PRIVATE_VLAN
			{
				pvlan_primary_t pvlan_primary = {0};
				unsigned int primary_vlan = 0;
				unsigned int array_port = 0;
				unsigned int pvlan_port_type = 0;
				
				npd_pvlan_netif_type_get(father_netif_index, &pvlan_port_type);
				if (!pvlan_port_type)
				{
					return ;
				}
				npd_vlan_port_pvid_get(father_netif_index, (unsigned short *)&primary_vlan);	
				memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
				pvlan_primary.vid = primary_vlan;
				ret = dbtable_sequence_search(
					pvlan_primary_vid_sequence, primary_vlan, (void * )&pvlan_primary);
			    if(0 != ret)
			    {
					return ;
			    }
				array_port = netif_array_index_from_ifindex(netif_index);
				NPD_PBMP_PORT_ADD(pvlan_primary.ports, array_port);
				ret = dbtable_sequence_update(pvlan_primary_vid_sequence,
					 					primary_vlan, NULL, (void * )&pvlan_primary);											
			}
#endif
		}
		break;
	case PORT_NOTIFIER_LEAVE:
		if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(father_netif_index))
		{
			if(NPD_NETIF_ETH_TYPE != npd_netif_type_get(netif_index))
				return;
#ifdef HAVE_PORT_ISOLATE			
			struct port_isolate_group_s group_default = {0};
			unsigned int array_port = 0;
			int trunkId = 0;
			array_port = eth_port_array_index_from_ifindex(netif_index);
			ret = dbtable_array_get(isolate_ports, isolate_group_global_no, &group_default);
			if (ret != VLAN_RETURN_CODE_ERR_NONE)
			{
				return;
			}

			trunkId = npd_netif_trunk_get_tid(father_netif_index);
			if (NPD_PBMP_MEMBER(group_default.isolate_trunks, trunkId) 
				&& NPD_PBMP_MEMBER(group_default.isolate_ports, array_port))			
			{
				NPD_PBMP_PORT_REMOVE(group_default.isolate_ports, array_port);
				ret = dbtable_array_update(isolate_ports, isolate_group_global_no, NULL, &group_default);				
			}		
#endif
#ifdef HAVE_PRIVATE_VLAN
			{
				pvlan_primary_t pvlan_primary = {0};
				unsigned int primary_vlan = 0;
				unsigned int array_port = 0;
				unsigned int pvlan_port_type = 0;
				
				npd_pvlan_netif_type_get(father_netif_index, &pvlan_port_type);
				if (!pvlan_port_type)
				{
					return ;
				}

				npd_vlan_port_pvid_get(father_netif_index, (unsigned short *)&primary_vlan);	
				memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
				pvlan_primary.vid = primary_vlan;
				ret = dbtable_sequence_search(
					pvlan_primary_vid_sequence, primary_vlan, (void * )&pvlan_primary);
			    if(0 != ret)
			    {
					return ;
			    }
				array_port = netif_array_index_from_ifindex(netif_index);
				NPD_PBMP_PORT_REMOVE(pvlan_primary.ports, array_port);
				ret = dbtable_sequence_update(pvlan_primary_vid_sequence,
					 					primary_vlan, NULL, (void * )&pvlan_primary);											
			}
#endif
		}		
		break;
    default:
        break;
		
	}
}

netif_event_notifier_t switchport_netif_notifier =
{
    .netif_event_handle_f = &npd_switchport_notify_event,
    .netif_relate_handle_f = &npd_switchport_relate_event
};



extern port_driver_t eth_switchport_driver;
extern port_driver_t trunk_switchport_driver;
port_driver_t *type_driver[] =
{
    &eth_switchport_driver,
    &trunk_switchport_driver
};
    
port_driver_t * port_driver_get(unsigned int netif_index)
{
    int i;
    for(i = 0; i < sizeof(type_driver)/sizeof(type_driver[0]); i++)
    {
        if (type_driver[i]->type == npd_netif_type_get(netif_index))
            return type_driver[i];
    }
    return NULL;
}

unsigned int switch_port_hash_key(void *data)
{
    switch_port_db_t *switch_port = (switch_port_db_t*)data;
	unsigned int netif_index = switch_port->global_port_ifindex;
	int hash_index = 0;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
    return hash_index%MAX_SWITCHPORT_PER_SYSTEM;
}

unsigned int switch_port_hash_cmp(void *data1, void *data2)
{
    switch_port_db_t *switch_port1 = (switch_port_db_t*)data1;
    switch_port_db_t *switch_port2 = (switch_port_db_t*)data2;

    return (switch_port1->global_port_ifindex 
                  == switch_port2->global_port_ifindex);
}
long switch_port_handle_delete(void *newdata);
long switch_port_handle_update(void * newdata, void *olddata);
long switch_port_handle_insert(void *newdata);

int switch_port_handle_ntoh(void *data)
{	
    switch_port_db_t *switchPort = (switch_port_db_t*)data;
	switchPort->switch_port_index = ntohl(switchPort->switch_port_index);
	switchPort->global_port_ifindex = ntohl(switchPort->global_port_ifindex);
	switchPort->fdb_limit = ntohl(switchPort->fdb_limit);
	switchPort->pvid = ntohl(switchPort->pvid);
	switchPort->inner_pvid = ntohl(switchPort->inner_pvid);
	switchPort->default_pri = ntohl(switchPort->default_pri);
	switchPort->default_inner_pri = ntohl(switchPort->default_inner_pri);
	switchPort->link_state = ntohl(switchPort->link_state);
	switchPort->stp_flag = ntohl(switchPort->stp_flag);
	switchPort->vlan_infilter_mode = ntohl(switchPort->vlan_infilter_mode);
	switchPort->vlan_efilter_mode = ntohl(switchPort->vlan_efilter_mode);
	switchPort->fdb_learning_mode = ntohl(switchPort->fdb_learning_mode);
	switchPort->vlan_access_mode = ntohl(switchPort->vlan_access_mode);
	switchPort->vlan_private_mode = ntohl(switchPort->vlan_private_mode);
	switchPort->subnet_vlan_flag = ntohl(switchPort->subnet_vlan_flag);
	switchPort->mac_vlan_flag = ntohl(switchPort->mac_vlan_flag);
	switchPort->prefer_subnet = ntohl(switchPort->prefer_subnet);
	switchPort->access_qinq = ntohl(switchPort->access_qinq);
	switchPort->qinq_drop_miss = ntohl(switchPort->qinq_drop_miss);
	switchPort->tpid = ntohs(switchPort->tpid);
	switchPort->inner_tpid = ntohs(switchPort->inner_tpid);
	switchPort->state = ntohl(switchPort->state);
    //switchPort->igmp_enable = ntohl(switchPort->igmp_enable);
	NPD_VBMP_VLAN_NTOH(switchPort->allow_tag_vlans);
	NPD_VBMP_VLAN_NTOH(switchPort->allow_untag_vlans);
#if 0
	NPD_VBMP_VLAN_NTOH(switchPort->allow_dhcp_vlans);
#endif
#ifdef HAVE_PORT_ISOLATE	
	switchPort->port_isolate = ntohl(switchPort->port_isolate);
#endif
	//NPD_VBMP_VLAN_NTOH(switchPort->trust_in_vlan);

	return 0;
}

int switch_port_handle_hton(void *data)
{	
    switch_port_db_t *switchPort = (switch_port_db_t*)data;
	switchPort->switch_port_index = htonl(switchPort->switch_port_index);
	switchPort->global_port_ifindex = htonl(switchPort->global_port_ifindex);
	switchPort->fdb_limit = htonl(switchPort->fdb_limit);
	switchPort->pvid = htonl(switchPort->pvid);
	switchPort->inner_pvid = htonl(switchPort->inner_pvid);
	switchPort->default_pri = htonl(switchPort->default_pri);
	switchPort->default_inner_pri = htonl(switchPort->default_inner_pri);
	switchPort->link_state = htonl(switchPort->link_state);
	switchPort->stp_flag = htonl(switchPort->stp_flag);
	switchPort->vlan_infilter_mode = htonl(switchPort->vlan_infilter_mode);
	switchPort->vlan_efilter_mode = htonl(switchPort->vlan_efilter_mode);
	switchPort->fdb_learning_mode = htonl(switchPort->fdb_learning_mode);
	switchPort->vlan_access_mode = htonl(switchPort->vlan_access_mode);
	switchPort->vlan_private_mode = htonl(switchPort->vlan_private_mode);
	switchPort->subnet_vlan_flag = htonl(switchPort->subnet_vlan_flag);
	switchPort->mac_vlan_flag = htonl(switchPort->mac_vlan_flag);
	switchPort->prefer_subnet = htonl(switchPort->prefer_subnet);
	switchPort->access_qinq = htonl(switchPort->access_qinq);
	switchPort->qinq_drop_miss = htonl(switchPort->qinq_drop_miss);
	switchPort->tpid = htons(switchPort->tpid);
	switchPort->inner_tpid = htons(switchPort->inner_tpid);
	switchPort->state = htonl(switchPort->state);	
    //switchPort->igmp_enable = htonl(switchPort->igmp_enable);
	NPD_VBMP_VLAN_HTON(switchPort->allow_tag_vlans);
	NPD_VBMP_VLAN_HTON(switchPort->allow_untag_vlans);
#if 0    
	NPD_VBMP_VLAN_HTON(switchPort->allow_dhcp_vlans);
#endif
#ifdef HAVE_PORT_ISOLATE		
	switchPort->port_isolate = htonl(switchPort->port_isolate);
#endif
	//NPD_VBMP_VLAN_HTON(switchPort->trust_in_vlan);

	return 0;
}

long islate_group_handle_delete(void *newdata);
long islate_group_handle_update(void * newdata, void *olddata);
long islate_group_handle_insert(void *newdata);

int islate_group_handle_ntoh(void *data)
{	
	port_isolate_group_t* isolate_group = (port_isolate_group_t *)data;
	NPD_PBMP_PORT_NTOH(isolate_group->isolate_ports);
	NPD_PBMP_PORT_NTOH(isolate_group->isolate_trunks);
	
	return 0;
}

int islate_group_handle_hton(void *data)
{	
	port_isolate_group_t* isolate_group = (port_isolate_group_t *)data;
	NPD_PBMP_PORT_HTON(isolate_group->isolate_ports);
	NPD_PBMP_PORT_HTON(isolate_group->isolate_trunks);

	return 0;
}


struct switch_port_db_s * npd_get_switch_port_by_index
(
	unsigned int eth_g_index
)
{
	struct switch_port_db_s *portInfo = NULL;
	int ret;

	portInfo = malloc(sizeof(struct switch_port_db_s));

	if (NULL == portInfo)
		return NULL;

	portInfo->global_port_ifindex = eth_g_index;

	ret = dbtable_hash_search(switch_ports_hash, portInfo, NULL, portInfo);

	if (0 != ret)
	{
		free(portInfo);
		return NULL;
	}

	return portInfo;
}



void npd_init_switch_ports(void)
{
    char name[16];

    strcpy(name, "SWITCH_PORT_DB");
    create_dbtable(name, MAX_SWITCHPORT_PER_SYSTEM, sizeof(struct switch_port_db_s),
        &switch_port_handle_update, NULL, &switch_port_handle_insert, 
        &switch_port_handle_delete, NULL, NULL, NULL, 
        switch_port_handle_ntoh, switch_port_handle_hton, DB_SYNC_ALL, &switch_ports_db);

    dbtable_create_array_index("array_index", switch_ports_db, &switch_ports);
	if(NULL == switch_ports)
	{
		syslog_ax_eth_port_dbg("memory alloc error for eth port init!!!\n");
		return;
	}

    dbtable_create_hash_index("netif_index", switch_ports_db, MAX_SWITCHPORT_PER_SYSTEM, 
          &switch_port_hash_key, &switch_port_hash_cmp,
          &switch_ports_hash);

#ifdef HAVE_PORT_ISOLATE
    struct port_isolate_group_s group_default;
    int ret;
    strcpy(name, "PORT_ISOLATE_DB");
    create_dbtable(name, MAX_PORT_ISOLATE_GROUP, sizeof(struct port_isolate_group_s),
        &islate_group_handle_update, NULL, &islate_group_handle_insert, 
        NULL, NULL, NULL, NULL, 
        islate_group_handle_ntoh, islate_group_handle_hton, DB_SYNC_ALL, &isolate_ports_db);

	dbtable_create_array_index("array_index", isolate_ports_db, &isolate_ports);
	if(NULL == switch_ports)
	{
		syslog_ax_eth_port_dbg("memory alloc error for eth port init!!!\n");
		return;
	}

    NPD_PBMP_CLEAR(group_default.isolate_ports);
    NPD_PBMP_CLEAR(group_default.isolate_trunks);
	
	ret = dbtable_array_insert(isolate_ports, &isolate_group_global_no, &group_default);
	if(ret != 0)
	{
		syslog_ax_fdb_err("Insert isolate port group default configuration failed.\n");
		return;
	}
#endif

    register_netif_notifier(&switchport_netif_notifier);
	return;
}


/**********************************************************************************
 *  npd_create_switch_port
 *
 *	DESCRIPTION:
 * 		this routine create ethernet port with <slot,port> and 
 *		attach port info to global data structure.
 *
 *	INPUT:
 *		global_index - 
 *		name - switchport name, ethernet or trunk
 *		link_state - switchport link state
 *		
 *	
 *	OUTPUT:
 *		index - switchport index in DB, auto assigned by DB
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
void npd_create_switch_port
(
	unsigned int global_index,
	char *name,
	unsigned int *index,	
	int link_state
	
) 
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int array_index;

    if(NULL == switch_port)
    {
        npd_syslog_err("% ERROR: SYSTEM resource exhausts\n");
        return;
    }
	memset( switch_port, 0, sizeof(switch_port_db_t));
    switch_port->global_port_ifindex = global_index;
    strncpy((char*)switch_port->switch_port_name, name, 30);
    switch_port->link_state = link_state;
    switch_port->vlan_access_mode = SWITCH_PORT_MODE_TRUNK;
    switch_port->tpid = g_tpid;
	switch_port->inner_tpid = g_inner_tpid;
	switch_port->vlan_private_mode = SWITCH_PORT_PRIVLAN_PROMI;
	switch_port->inner_pvid = DEFAULT_VLAN_ID;
    switch_port->fdb_learning_mode = 1;
    switch_port->fdb_limit = -1;

    array_index = netif_array_index_from_ifindex(global_index);
    switch_port->switch_port_index = array_index;
    
	syslog_ax_eth_port_dbg("create switch port for netif 0x%x \n", global_index);

    dbtable_array_insert_byid(switch_ports, array_index, (void*)switch_port);
    
	syslog_ax_eth_port_dbg("Switch port index %d for netif 0x%x\n", *index, global_index);

    *index = array_index;
	free(switch_port);

	return;
}

void npd_delete_switch_port
(
    unsigned int index
) 
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));

    if(NULL == switch_port)
        return ;
    dbtable_array_delete(switch_ports, index, (void*)switch_port);
    free(switch_port);
	return;
	
}

unsigned int npd_switch_port_get_vlan_mode
(
    unsigned int netif_index, int *mode
) 
{
	unsigned int ret = NPD_SUCCESS;
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));

    if(NULL == switch_port)
        return NPD_FAIL;
	
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port,
         NULL, switch_port);
	if(ret == 0)
	{
		*mode = switch_port->vlan_access_mode;
	}
    free(switch_port);
	return ret;
	
}

int npd_switch_port_vlan_filter_set(
    unsigned int switch_index,
    int mode
    )
{
	unsigned int ret = NPD_SUCCESS;

    switch_port_db_t *portInfo = malloc(sizeof(switch_port_db_t));

    if(NULL == portInfo)
        return -1;
    npd_key_database_lock();
    ret = dbtable_array_get(switch_ports, switch_index, (void*)portInfo);
	if(0 != ret) {
		syslog_ax_eth_port_err("npd eth port set pvid port %#0x null",switch_index);
        free(portInfo);
		npd_key_database_unlock();
		return NPD_FAIL;
	}

	portInfo->vlan_infilter_mode = mode;
    ret = dbtable_array_update(switch_ports, switch_index, NULL, (void*)portInfo);
	ret = NPD_SUCCESS;
    npd_key_database_unlock();
    free(portInfo);

	return ret;
    
}


int npd_switch_port_isolate_set(switch_port_db_t* src_port, switch_port_db_t* dst_port)
{
    port_driver_t *driver ;
        
	int ret = NPD_SUCCESS;

	if (src_port->switch_port_index== dst_port->switch_port_index) 
	{
		return NPD_SUCCESS;
	}

	driver = port_driver_get(dst_port->global_port_ifindex);
	if(driver == NULL)
		return ret;
	
    switch(dst_port->vlan_private_mode)
    {
    case SWITCH_PORT_PRIVLAN_PROMI:
		{
			if(driver->port_isolate_del)
			{
				(*driver->port_isolate_del)(src_port->global_port_ifindex,
                     	dst_port->global_port_ifindex);
			}
		}
        break;
    case SWITCH_PORT_PRIVLAN_COMMUNITY:
		{
            if(src_port->vlan_private_mode == SWITCH_PORT_PRIVLAN_ISOLATED)
            {
				if(driver->port_isolate_add == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->port_isolate_add)(src_port->global_port_ifindex,
                            dst_port->global_port_ifindex);
				}
            }
			else
			{
				if(driver->port_isolate_del == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->port_isolate_del)(src_port->global_port_ifindex,
                            dst_port->global_port_ifindex);
				}
				
			}

			if (0 != ret)
			{
				npd_syslog_dbg("ret is not 0: %s(%d): %s", 
					__FILE__, __LINE__, "*driver->port_isolate_add");
			}
		}
        break;
    case SWITCH_PORT_PRIVLAN_ISOLATED:
		{
            if(src_port->vlan_private_mode != SWITCH_PORT_PRIVLAN_PROMI)
            {
				if(driver->port_isolate_add == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->port_isolate_add)(src_port->global_port_ifindex,
                            dst_port->global_port_ifindex);
				}
            }
			else
			{
				if(driver->port_isolate_del == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->port_isolate_del)(src_port->global_port_ifindex,
                            dst_port->global_port_ifindex);
				}				
			}

			if (0 != ret)
			{
				npd_syslog_dbg("ret is not 0: %s(%d): %s", 
					__FILE__, __LINE__, "*driver->port_isolate_add");
			}
		}
        break;
    default:
        break;
    }	

	return ret;
}

long switch_port_handle_delete(void *newdata)
{
    switch_port_db_t *new = (switch_port_db_t*)newdata;
    int ret = 0, retval = 0;

    port_driver_t *driver 
         = port_driver_get(new->global_port_ifindex);

	if(driver==NULL)
		return retval;

    syslog_ax_vlan_dbg("Switch port %d delete\n", new->switch_port_index);
    
	if(driver->fdb_delete_by_port == NULL)
	{
		retval = NPD_FAIL;
	}
	else
	{
        ret = (*driver->fdb_delete_by_port)(new->global_port_ifindex);
        if(0 != ret)
            retval = ret;
    }
	return retval;
}

long switch_port_handle_insert(void *newdata)
{
    switch_port_db_t *new = (switch_port_db_t*)newdata;
    int ret = 0, retval = 0;

    port_driver_t *driver 
         = port_driver_get(new->global_port_ifindex);

    syslog_ax_vlan_dbg("Switch port %d insert\n", new->switch_port_index);

	if( driver == NULL )
		return 0;
		
    {
        ret = (*driver->fdb_learning_mode)(new->global_port_ifindex, new->fdb_learning_mode);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->fdb_learning_mode");
		}
        if(new->fdb_learning_mode == 0 || (new->fdb_learning_mode == 2 && npd_startup_end))        
            ret = (*driver->fdb_delete_by_port)(new->global_port_ifindex);
    }

    if(new->fdb_limit >= 0)
    {
        ret = (*driver->fdb_limit_set)(new->global_port_ifindex, new->fdb_limit);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->fdb_limit_set");
		}
        ret = (*driver->fdb_delete_by_port)(new->global_port_ifindex);
    }

    if(new->pvid)
    {
        ret =  (*driver->set_pvid)(new->global_port_ifindex, new->pvid);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->set_pvid");
		}
		
    }

	if(new->inner_pvid)
	{
        ret =  (*driver->set_inner_pvid)(new->global_port_ifindex, new->inner_pvid);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->set_inner_pvid");
		}
	}

	/* because all ports be placed in default vlan(VLAN 1) in the beginning. 
	    so we must remove it  */
	if (!NPD_VBMP_MEMBER(new->allow_untag_vlans, DEFAULT_VLAN_ID))
	{
        syslog_ax_vlan_dbg("Operate vlan %d remove port 0x%x untagged\n",
            DEFAULT_VLAN_ID, new->global_port_ifindex);
        ret = (*driver->remove_vlan)(new->global_port_ifindex, DEFAULT_VLAN_ID, FALSE);

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->remove_vlan");
		}			
	}

    {
        int vlan;

        NPD_VBMP_ITER(new->allow_tag_vlans, vlan)
        {
            {
                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x tagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->allow_vlan)(new->global_port_ifindex, vlan, TRUE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->allow_vlan");
				}
				
            }

            if(0 != ret)
                retval = ret;
        }
    }
    
    {
        int vlan;

        NPD_VBMP_ITER(new->allow_untag_vlans, vlan)
        {
            {
                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x untagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->allow_vlan)(new->global_port_ifindex, vlan, FALSE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->allow_vlan");
				}
				
            }
 
            if(0 != ret)
                retval = ret;
            
        }
    }
#if 0
	{
        int vlan;

		NPD_VBMP_ITER(new->allow_dhcp_vlans, vlan)
		{	
			if(driver->dhcp_trap_set == NULL)
			{
				ret = NPD_FAIL;
			}
			else
			{
                ret = (*driver->dhcp_trap_set)(vlan, new->global_port_ifindex, TRUE);
			}
            if(0 != ret)
			{
				retval = -1;
				
				if (-1 == retval)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "TRAP SET");
				}
				
			}
		}
	}
    {
		if(new->igmp_enable)
		{
			if(driver->igmp_trap_set == NULL)
            {
				ret = NPD_FAIL;
            }
			else
			{
                ret = (*driver->igmp_trap_set)(0, new->global_port_ifindex, TRUE);
			}
            if(0 != ret)
			{
				retval = -1;
				
				if (-1 == retval)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
				}
				
			}
		}
    }
 #endif

 #ifndef HAVE_PRIVATE_VLAN
    if(new->vlan_private_mode)
    {
        npd_vbmp_t bmp;
        int vlan;
		
        NPD_VBMP_ASSIGN(bmp, new->allow_untag_vlans);
        NPD_VBMP_OR(bmp, new->allow_tag_vlans);
        NPD_VBMP_ITER(bmp, vlan)
        {
            struct vlan_s vlan_loop;
            int vlan_ret;

            vlan_loop.vid = vlan;
            vlan_ret = dbtable_sequence_search(g_vlans, vlan, &vlan_loop);
            if(0 != vlan_ret)
            {
                continue;
            }

            if(vlan_loop.pvlan_type)
            {
                int port;
				npd_pbmp_t bmp_port;
				
				NPD_PBMP_ASSIGN(bmp_port, vlan_loop.untag_ports);
        		NPD_PBMP_OR(bmp_port, vlan_loop.tag_ports);
                NPD_PBMP_ITER(bmp_port, port)
                {
                    switch_port_db_t switch_port_egress;
					
                    ret = dbtable_array_get(switch_ports, port, &switch_port_egress);
					if (0 != ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "dbtable_array_get");
					}
					
					ret = npd_switch_port_isolate_set(&switch_port_egress,new);
					if (0 != ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "npd_switch_port_isolate_set");
					}
						
					
					ret = npd_switch_port_isolate_set(new, &switch_port_egress);
					if (0 != ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "npd_switch_port_isolate_set");
					}
						 
                }
            }
        }
    }
#endif

#ifdef HAVE_QINQ
    if(new->access_qinq)
    {
		if(driver->access_qinq_enable == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
            ret = (*driver->access_qinq_enable)(new->global_port_ifindex, new->access_qinq);
		}
        if(0 != ret)
            retval = ret;
		
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "access_qinq_enable");
		}
    }

    if(new->qinq_drop_miss)
    {
		if(driver->qinq_drop_miss_enable == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
            ret = (*driver->qinq_drop_miss_enable)(new->global_port_ifindex, new->qinq_drop_miss);
		}
        if(0 != ret)
            retval = ret;
		
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "qinq_drop_miss_enable");
		}		
    }

	if (new->tpid != 0x8100)
	{
		if (driver->tpid_set == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
			ret = (*driver->tpid_set)(new->global_port_ifindex, new->tpid);
		}

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "tpid_set");
		}		
	}	
#endif
	/* 
	if (new->inner_tpid != 0x8100)
	{
		if (driver->inner_tpid_set == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
			ret = (*driver->inner_tpid_set)(new->global_port_ifindex, new->inner_tpid);
		}
	}	
	*/
	

    {
		if(driver->subnet_vlan_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->subnet_vlan_enable)(new->global_port_ifindex, new->subnet_vlan_flag);
            if(0 != ret)
                retval = ret;
			else
			{
        		if(new->mac_vlan_flag || new->subnet_vlan_flag)
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 0);/*NONE*/
        			}
        		}
        		else
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 3);/*INGRESS and EGRESS*/
        			}
        		}
				retval = ret;
			}
		}
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "subnet_vlan_enable");
		}				
    }

    {
		if(driver->mac_vlan_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->mac_vlan_enable)(new->global_port_ifindex, new->mac_vlan_flag);
            if(0 != ret)
                retval = ret;
			else
			{
        		if(new->mac_vlan_flag || new->subnet_vlan_flag)
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 0);/*NONE*/
        			}
        		}
        		else
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 3);/*INGRESS and EGRESS*/
        			}
        		}
				retval = ret;
			}
		}
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "mac_vlan_enable");
		}				
    }

    {
		if(driver->prefer_subnet_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->prefer_subnet_enable)(new->global_port_ifindex, new->prefer_subnet);
            if(0 != ret)
                retval = ret;
		}
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "prefer_subnet_enable");
		}						
    }
	if(driver->vlan_mode_set == NULL)
	{
		retval = NPD_FAIL;
	}
	else
	{
		ret = (*driver->vlan_mode_set)(new->global_port_ifindex, new->vlan_access_mode);
		if(0 != ret)
			retval = ret;
	}
    return retval;
    
}

long switch_port_handle_update(void * newdata, void *olddata)
{
    switch_port_db_t *new = (switch_port_db_t*)newdata;
    switch_port_db_t *old = (switch_port_db_t*)olddata;
    int ret = 0, retval = 0;


    port_driver_t *driver 
         = port_driver_get(new->global_port_ifindex);

	if(driver==NULL)
		return retval;
	
    syslog_ax_vlan_dbg("Switch port %d update\n", new->switch_port_index);

    if(new->fdb_learning_mode != old->fdb_learning_mode)
    {
        ret = (*driver->fdb_learning_mode)(new->global_port_ifindex, new->fdb_learning_mode);
        if((0 == new->fdb_learning_mode) || (2 == new->fdb_learning_mode && npd_startup_end))
            ret = (*driver->fdb_delete_by_port)(new->global_port_ifindex);
    }

    if(new->fdb_limit != old->fdb_limit)
    {
        ret = (*driver->fdb_limit_set)(new->global_port_ifindex, new->fdb_limit);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->fdb_limit_set");
		}
/*
        ret = (*driver->fdb_delete_by_port)(new->global_port_ifindex);
*/
    }

    if(new->pvid != old->pvid)
    {
        ret =  (*driver->set_pvid)(new->global_port_ifindex, new->pvid);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->set_pvid");
		}

    }

    if(new->inner_pvid != old->inner_pvid)
    {
        ret =  (*driver->set_inner_pvid)(new->global_port_ifindex, new->inner_pvid);
        if(0 != ret)
            retval = ret;

		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "*driver->set_inner_pvid");
		}

    }


    if(!NPD_VBMP_EQ(new->allow_tag_vlans , old->allow_tag_vlans))
    {
        npd_vbmp_t bmp;
        int vlan;

        NPD_VBMP_ASSIGN(bmp, new->allow_tag_vlans);
        NPD_VBMP_XOR(bmp, old->allow_tag_vlans);
        NPD_VBMP_ITER(bmp, vlan)
        {
            if(NPD_VBMP_MEMBER(new->allow_tag_vlans, vlan))
            {
                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x tagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->allow_vlan)(new->global_port_ifindex, vlan, TRUE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->allow_vlan");
				}
				
            }
            else
            {
                syslog_ax_vlan_dbg("Operate vlan %d remove port 0x%x tagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->remove_vlan)(new->global_port_ifindex, vlan, TRUE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->remove_vlan");
				}
				
            }
            if(0 != ret)
                retval = ret;
        }
    }
    
    if(!NPD_VBMP_EQ(new->allow_untag_vlans , old->allow_untag_vlans))
    {
        npd_vbmp_t bmp;
        int vlan;

        NPD_VBMP_ASSIGN(bmp, new->allow_untag_vlans);
        NPD_VBMP_XOR(bmp, old->allow_untag_vlans);
        NPD_VBMP_ITER(bmp, vlan)
        {
            if(NPD_VBMP_MEMBER(new->allow_untag_vlans, vlan))
            {
                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x untagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->allow_vlan)(new->global_port_ifindex, vlan, FALSE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->allow_vlan");
				}
				
            }
            else
            {
                syslog_ax_vlan_dbg("Operate vlan %d remove port 0x%x untagged\n",
                    vlan, new->global_port_ifindex);
                ret = (*driver->remove_vlan)(new->global_port_ifindex, vlan, FALSE);

				if (0 != ret)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "*driver->remove_vlan");
				}				
            }
            if(0 != ret)
                retval = ret;            
        }
    }

#if 0
    if(!NPD_VBMP_EQ(new->allow_dhcp_vlans , old->allow_dhcp_vlans))
    {
        npd_vbmp_t pbmp;
        int vlan;
        
		NPD_PBMP_ASSIGN(pbmp, new->allow_dhcp_vlans);
		NPD_PBMP_XOR(pbmp, old->allow_dhcp_vlans);

		NPD_PBMP_ITER(pbmp, vlan)
		{				
			if( NPD_PBMP_MEMBER(new->allow_dhcp_vlans, vlan) ) {	

				if(driver->dhcp_trap_set == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->dhcp_trap_set)(vlan, new->global_port_ifindex, TRUE);
				}
                if(0 != ret)
				{
					retval = -1;
					
					if (-1 == retval)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
					}
					
				}
			}
			else 
            {
				if(driver->dhcp_trap_set == NULL)
				{
					ret = NPD_FAIL;
				}
				else
				{
                    ret = (*driver->dhcp_trap_set)(vlan, new->global_port_ifindex, FALSE);
				}
				if(0 != ret)
                {
					retval = -1;

					if (-1 == retval)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
					}
					
				}
			}
			
		}
    }

    if(new->igmp_enable != old->igmp_enable)
    {
        npd_vbmp_t pbmp;
        int vlan;
		if(new->igmp_enable)
		{
			if(driver->igmp_trap_set == NULL)
            {
				ret = NPD_FAIL;
            }
			else
			{
                ret = (*driver->igmp_trap_set)(vlan, new->global_port_ifindex, TRUE);
			}
            if(0 != ret)
			{
				retval = -1;
				
				if (-1 == retval)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
				}
				
			}
		}
		else
		{
			if(driver->igmp_trap_set == NULL)
			{
				ret = NPD_FAIL;
			}
			else
			{
                ret = (*driver->igmp_trap_set)(vlan, new->global_port_ifindex, FALSE);
			}
			if(0 != ret)
            {
				retval = -1;

				if (-1 == retval)
				{
					npd_syslog_dbg("ret is not 0: %s(%d): %s", 
						__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
				}
				
			}
		}
    }
#endif

#ifndef HAVE_PRIVATE_VLAN
    if(new->vlan_private_mode != old->vlan_private_mode)
    {
        npd_vbmp_t bmp;
        int vlan;
		
        NPD_VBMP_ASSIGN(bmp, new->allow_untag_vlans);
        NPD_VBMP_OR(bmp, new->allow_tag_vlans);
        NPD_VBMP_ITER(bmp, vlan)
        {
            struct vlan_s vlan_loop;
            int vlan_ret;

            vlan_loop.vid = vlan;
            vlan_ret = dbtable_sequence_search(g_vlans, vlan, &vlan_loop);
            if(0 != vlan_ret)
            {
                continue;
            }

            if(vlan_loop.pvlan_type)
            {
                int port;
				npd_pbmp_t bmp_port;
				
				NPD_PBMP_ASSIGN(bmp_port, vlan_loop.untag_ports);
        		NPD_PBMP_OR(bmp_port, vlan_loop.tag_ports);
                NPD_PBMP_ITER(bmp_port, port)
                {
                    switch_port_db_t switch_port_egress;
					
                    ret = dbtable_array_get(switch_ports, port, &switch_port_egress);
					if (0 != ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "dbtable_array_get");
					}
					
					ret = npd_switch_port_isolate_set(&switch_port_egress,new);
					
					ret = npd_switch_port_isolate_set(new, &switch_port_egress);
                         
                }
            }
        }
    }
#endif

#ifdef HAVE_QINQ
    if(new->access_qinq != old->access_qinq)
    {
		if(driver->access_qinq_enable == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
            ret = (*driver->access_qinq_enable)(new->global_port_ifindex, new->access_qinq);
		}
        if(0 != ret)
            retval = ret;
    }

    if(new->qinq_drop_miss != old->qinq_drop_miss)
    {
		if(driver->qinq_drop_miss_enable == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
            ret = (*driver->qinq_drop_miss_enable)(new->global_port_ifindex, new->qinq_drop_miss);
		}
        if(0 != ret)
            retval = ret;
    }

	if (new->tpid != old->tpid)
	{
		if (driver->tpid_set == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
			ret = (*driver->tpid_set)(new->global_port_ifindex, new->tpid);
		}
	}

	/* 
	if (new->inner_tpid != old->inner_tpid)
	{
		if (driver->inner_tpid_set == NULL)
		{
			ret = NPD_FAIL;
		}
		else
		{
			ret = (*driver->inner_tpid_set)(new->global_port_ifindex, new->inner_tpid);
		}
	}
	*/
#endif
    if(new->subnet_vlan_flag != old->subnet_vlan_flag)
    {
		if(driver->subnet_vlan_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->subnet_vlan_enable)(new->global_port_ifindex, new->subnet_vlan_flag);
            if(0 != ret)
                retval = ret;
			else
			{
        		if(new->mac_vlan_flag || new->subnet_vlan_flag)
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 0);/*NONE*/
        			} 
        		}
        		else
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 3);/*INGRESS and EGRESS*/
        			}
        		}
				retval = ret;
			}
		}
    }

    if(new->mac_vlan_flag != old->mac_vlan_flag)
    {
		if(driver->mac_vlan_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->mac_vlan_enable)(new->global_port_ifindex, new->mac_vlan_flag);
            if(0 != ret)
                retval = ret;
			else
			{
        		if(new->mac_vlan_flag || new->subnet_vlan_flag)
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 0);/*NONE*/
        			}
        		}
        		else
        		{
        			if(driver->port_vlan_filter)
        			{
        			    ret = (*driver->port_vlan_filter)(new->global_port_ifindex, 3);/*INGRESS and EGRESS*/
        			}
        		}
				retval = ret;
			}
		}
    }

    if(new->prefer_subnet != old->prefer_subnet)
    {
		if(driver->prefer_subnet_enable == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
            ret = (*driver->prefer_subnet_enable)(new->global_port_ifindex, new->prefer_subnet);
            if(0 != ret)
                retval = ret;
		}
    }

	if(new->vlan_access_mode != old->vlan_access_mode)
	{
		if(driver->vlan_mode_set == NULL)
		{
			retval = NPD_FAIL;
		}
		else
		{
			ret = (*driver->vlan_mode_set)(new->global_port_ifindex, new->vlan_access_mode);
			if(0 != ret)
				retval = ret;
		}
	}
    return retval;

}



long npd_switch_port_isolate_add(switch_port_db_t* src_port, switch_port_db_t* dst_port)
{
	port_driver_t * driver = NULL;
	int ret = 0;
	/* add the egress to iter */
	driver = port_driver_get(src_port->global_port_ifindex);
	if(driver == NULL)
		return 0;
	
	if(driver->port_isolate_add)
	{
        ret = (*driver->port_isolate_add)(src_port->global_port_ifindex,
                dst_port->global_port_ifindex);
	}

	/* add the iter to egress */
	driver = port_driver_get(dst_port->global_port_ifindex);
	if(driver == NULL)
		return 0;
	
	if(driver->port_isolate_add)
	{
        ret = (*driver->port_isolate_add)(dst_port->global_port_ifindex, 
			src_port->global_port_ifindex);
	}
    return 0;
}

long npd_switch_port_isolate_del(switch_port_db_t* src_port, switch_port_db_t* dst_port)
{
	port_driver_t * driver = NULL;
	int ret = 0;
	/* add the egress to iter */
	driver = port_driver_get(dst_port->global_port_ifindex);
	if(driver == NULL)
		return 0;
	
	if(driver->port_isolate_del)
	{
        ret = (*driver->port_isolate_del)(src_port->global_port_ifindex,
                dst_port->global_port_ifindex);
	}

	/* add the iter to egress */
	driver = port_driver_get(src_port->global_port_ifindex);
	if(driver == NULL)
		return 0;
	
	if(driver->port_isolate_del)
	{
        ret = (*driver->port_isolate_del)(dst_port->global_port_ifindex, 
			src_port->global_port_ifindex);
	}	
    return 0;
}

long islate_group_handle_delete(void *newdata)
{
	return 0;
}

long islate_group_handle_insert(void *newdata)
{
	port_isolate_group_t* new_group = (port_isolate_group_t *)newdata;
	unsigned int iter_port = 0;
	unsigned int array_port = 0;
	
	NPD_PBMP_ITER(new_group->isolate_ports, array_port)
	{

		/*
	    ret = dbtable_array_get(switch_ports, array_port, &switch_port_egress);
		if (0 != ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "dbtable_array_get");
			
			continue;
		}
		*/
		NPD_PBMP_ITER(new_group->isolate_ports, iter_port)
		{
			if (iter_port == array_port)
			{
				continue;
			}
			/*
		    ret = dbtable_array_get(switch_ports, iter_port, &switch_port_iter);
			if (0 != ret)
			{
				npd_syslog_dbg("ret is not 0: %s(%d): %s", 
					__FILE__, __LINE__, "dbtable_array_get");
				
				continue;
			}
			*/
			//npd_switch_port_isolate_add(&switch_port_egress, &switch_port_iter);
			npd_port_isolate_add(
				netif_array_index_to_ifindex(array_port), 
				netif_array_index_to_ifindex(iter_port));
			npd_port_isolate_add(
				netif_array_index_to_ifindex(iter_port),
				netif_array_index_to_ifindex(array_port));
							
		}		
	}
		
	return 0;
}
long islate_group_handle_update(void * newdata, void *olddata)
{
	port_isolate_group_t* new_group = (port_isolate_group_t *)newdata;
	port_isolate_group_t* old_group = (port_isolate_group_t *)olddata;
	npd_pbmp_t bmp;
	unsigned int array_port = 0;
	unsigned int iter_port = 0;
	
    NPD_PBMP_ASSIGN(bmp, new_group->isolate_ports);
    NPD_PBMP_XOR(bmp, old_group->isolate_ports);
    NPD_PBMP_ITER(bmp, array_port)
    {
		if (NPD_PBMP_MEMBER(new_group->isolate_ports, array_port)) 
		{
			/* add the isolate between ports  and new port*/
			NPD_PBMP_ITER(new_group->isolate_ports, iter_port)
			{
				if (iter_port == array_port)
				{
					continue;
				}			
				npd_port_isolate_add(
					netif_array_index_to_ifindex(array_port), 
					netif_array_index_to_ifindex(iter_port));
				npd_port_isolate_add(
					netif_array_index_to_ifindex(iter_port),
					netif_array_index_to_ifindex(array_port));
			}
		}
		else if (NPD_PBMP_MEMBER(old_group->isolate_ports, array_port))
		{
			/* del the isolate between ports and old port */
			NPD_PBMP_ITER(old_group->isolate_ports, iter_port)
			{
				if (iter_port == array_port)
				{
					continue;
				}			
				npd_port_isolate_del(
					netif_array_index_to_ifindex(array_port), 
					netif_array_index_to_ifindex(iter_port));
				npd_port_isolate_del(
					netif_array_index_to_ifindex(iter_port),
					netif_array_index_to_ifindex(array_port));
				
			}			
		}
		else
		{
			continue;
		}
		
		
	}

	
	return 0;
}

unsigned int npd_switch_port_link_state_get(
    unsigned int switch_port_index,
    int *link_state
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret = 0;

    if(NULL == switch_port)
        return COMMON_RETURN_CODE_NO_RESOURCE;

    ret = dbtable_array_get(switch_ports, switch_port_index, switch_port);
    if(-1 == ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }

    *link_state = switch_port->link_state;

error:
    if(switch_port)
        free(switch_port);
    return ret;
    
}
    

 
unsigned int npd_fdb_number_port_set_check 
(
    unsigned int eth_g_index,
    unsigned int* number
)

{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    unsigned int ret;

    if(NULL == switch_port)
        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;

    switch_port->global_port_ifindex = eth_g_index;
    ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port,
         NULL, switch_port);
    if(0 != ret)
        goto error;
    
    *number = switch_port->fdb_limit;

    if(*number >= 0){
      /* npd_syslog_dbg("The port-based FDB protected has been set: slotNum: %d,portNum: %d, fdbLimit: %d,fdbCount: %d  \n ",slotNum,portNum,*number,switchNode->fdbCount);*/
	   ret = TRUE;
	}
    else{
       /*npd_syslog_dbg("The port-based FDB protected has not been set: slotNum: %d,portNum: %d, fdbnumber: %d,fdbCount: %d \n ",slotNum,portNum,*number,switchNode->fdbCount);*/
	   ret = FALSE;
	}

error:
    if(switch_port)
        free(switch_port);
	return ret;

}


unsigned int npd_fdb_number_port_set
(
    unsigned int eth_g_index,
    unsigned int number
)
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    unsigned int ret = 0;

    if(NULL == switch_port)
        ret = FDB_RETURN_CODE_HW_NOT_SUPPORT;

    switch_port->global_port_ifindex = eth_g_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port,
         NULL, switch_port);
    if(0 != ret)
    {
		ret = FDB_RETURN_CODE_SIWTCHPORT_NOT_EXIST;
        goto error;
    }
    if(switch_port->fdb_limit != number)
    {
        switch_port->fdb_limit = number;
    
        ret = dbtable_array_update(switch_ports, switch_port->switch_port_index, 
            NULL, switch_port);
    	if(number != 0)
    	{
            npd_fdb_dynamic_entry_del_by_port(eth_g_index);
    	}
    }
error:
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);
	return ret;
}
/*status : 0                      not learning and discard unknown SA
                   1                      auto learning and forward 
                   2                      protect mode. CPU learning and discard packet when mac limit full
                   3                      mac auth mode. CPU learning and discard packet when mac un-authed
                   4                      sticky mode
*/
unsigned int npd_fdb_learning_mode_set
(
    unsigned int eth_g_index,
    unsigned int mode
)
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    unsigned int ret;

    if(NULL == switch_port)
    {
        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
		return ret;
	}

    switch_port->global_port_ifindex = eth_g_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port,
         NULL, switch_port);
    if(0 != ret)
        goto error;
        
    switch_port->fdb_learning_mode = mode;

    ret = dbtable_array_update(switch_ports, switch_port->switch_port_index, 
        NULL, switch_port);
    if(mode == 0 || mode == 2)
	{
		npd_fdb_dynamic_entry_del_by_port(eth_g_index);
    }
	else if(mode == 4)
	{
		npd_fdb_dynamic_entry_sticky_by_netif(eth_g_index);
	}
error:
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);
	return ret;
}

unsigned int npd_fdb_learning_mode_get
(
    unsigned int eth_g_index,
    unsigned int *mode
)
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    unsigned int ret;

    if(NULL == switch_port)
        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;

    switch_port->global_port_ifindex = eth_g_index;
    ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port,
         NULL, switch_port);
    if(0 != ret)
        goto error;
        
    *mode = switch_port->fdb_learning_mode;

error:
    if(switch_port)
        free(switch_port);
	return ret;
}

unsigned int npd_module_learning_mode_get
(
    unsigned int netif_index,
    unsigned int learn_module,
    unsigned int *mode
)
{
   	int ret = 0;
	ret = DB_TABLE_RETURN_CODE_ENTRY_NOT_EXIST;
	ret = npd_fdb_learning_mode_get(netif_index, mode);
	/* only judge it for switchport. other will be succeeed*/
	if (DB_TABLE_RETURN_CODE_ENTRY_NOT_EXIST == ret)
	{
		*mode = 1;
		return 0;
	}
	return ret;	
}



int npd_netif_speed(
    unsigned int netif_index,
    int *speed)
{
    int ret;
    port_driver_t *driver;
    driver = port_driver_get(netif_index);

    if(NULL == driver)
        return -1;

    ret = (*driver->port_speed)(netif_index, speed);
    return ret;
}

int npd_netif_duplex_mode(
    unsigned int netif_index,
    int *duplex_mode
    )
{
    int ret;
    port_driver_t *driver;
    driver = port_driver_get(netif_index);

    if(NULL == driver)
        return -1;

    ret = (*driver->port_duplex_mode)(netif_index, duplex_mode);
    return ret;
}

int npd_check_netif_status(
    unsigned int netif_index,
    int *link_status
    )
{
    int type;
    type = npd_netif_type_get(netif_index);

    switch(type)
    {
    case NPD_NETIF_ETH_TYPE:
        *link_status = npd_check_eth_port_status(netif_index);
        return 0;
    case NPD_NETIF_TRUNK_TYPE:
        *link_status = npd_check_trunk_status(npd_netif_trunk_get_tid(netif_index));
        return 0;
    case NPD_NETIF_VLAN_TYPE:
        *link_status = npd_check_vlan_status(npd_netif_vlan_get_vid(netif_index));
        return 0;
    default:
        *link_status = 1;
        return 0;
    }
    return 0;
}

unsigned int npd_switch_port_netif_index(
    unsigned int switch_port_index,
    unsigned int *netif_index
    )
{
	switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
	*netif_index = 0;
	
	if(NULL == switch_port) {
		return -1;
	}
	
	dbtable_array_get(switch_ports, switch_port_index, (void*)switch_port);
	if( switch_port != 0) {
		*netif_index = switch_port->global_port_ifindex;
		free(switch_port);
		return 0;
	}
    free(switch_port);
	return -1;
}

int npd_save_switch_port_igmp_snp_cfg(void *data, char* buf,int *bufLen)
{
    return 0;
}

int npd_check_netif_switch_mode(
    unsigned int netif_index)
{
	int netif_type = -1;
	int trunk_id = -1;
	struct trunk_s trunk;

	netif_type = npd_netif_type_get(netif_index);
	if(NPD_NETIF_ETH_TYPE == netif_type){
		return npd_check_port_switch_mode(netif_index);
	}
	else if(NPD_NETIF_TRUNK_TYPE == netif_type){
		trunk_id = npd_netif_trunk_get_tid(netif_index);
		if(npd_find_trunk(trunk_id, &trunk)){
			return FALSE;
		}
		return (PORT_SWITCH_PORT == trunk.forward_mode? TRUE:FALSE);
	}

	return FALSE;
   
}

extern hash_table_index_t *vlan_xlate_table_netif_hash;

#ifdef HAVE_PRIVATE_VLAN
enum NPD_PVLAN_PORT_TYPE
{
	PVLAN_PORT_TYPE_NORMAL = 0,
	PVLAN_PORT_TYPE_PROMISCUOUS,
	PVLAN_PORT_TYPE_ISOLATE,
	PVLAN_PORT_TYPE_MAX		
};

#endif

int npd_switch_port_show_running(
    unsigned int switch_port_index,
    char *showStr,
    int size
    )
{
    switch_port_db_t switch_port;
    vlan_t vlan;
    char tempbuf[256];
    int len = 0;
    int ret;
    int start_vid = 0,  end_vid = 0;
    int vid;
    int vid_str_len = 0;
    int vid_allow_count = 0;

    ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port);
    if(ret != 0)
        return -1;
	/* Assume the primary vlan before isolate vlan in private vlan, so this feature need be improved */

    if(switch_port.vlan_access_mode != SWITCH_PORT_MODE_TRUNK)
    {
        char *str[3] = {"access", "trunk", "hybrid"};
        sprintf(tempbuf, " switchport mode %s\n", str[switch_port.vlan_access_mode]);
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }
    NPD_VBMP_ITER(switch_port.allow_untag_vlans, vid)
    {
        memset(&vlan, 0, sizeof(vlan));
        vlan.vid = vid;
        ret = dbtable_sequence_search(g_vlans, vid, &vlan);
        if(0 != ret)
            continue;
        if(vid == DEFAULT_VLAN_ID)
		{
			continue;
		}
        if((vid == switch_port.pvid)
            && (switch_port.vlan_access_mode != SWITCH_PORT_MODE_HYBRID))
            continue;
#ifdef HAVE_PRIVATE_VLAN
		if ((switch_port.vlan_private_mode == SWITCH_PORT_PRIVLAN_PROMI) &&
			(vlan.pvlan_type > 1)) /* PRIMARY VLAN */
		{
			continue;
		}

		if ((switch_port.vlan_private_mode == SWITCH_PORT_PRIVLAN_ISOLATED) &&
			(vlan.pvlan_type != 2)) /* ISOLATE VLAN */
		{
			continue;
		}
		
#endif

        if(start_vid == 0)
        {
            start_vid = vid;
            end_vid = vid;
            vid_allow_count++;
        }
        else if((vid == end_vid+1)
                 && vlan.isStatic
                 && (vid_allow_count < 199))
        {
            end_vid = vid;
            vid_allow_count++;
        }
        else
        {
            if(0 == vid_str_len)
                sprintf(tempbuf, " switchport access allowed vlan ");
            if(start_vid == end_vid)
            {
                sprintf(tempbuf, "%s%d", tempbuf, start_vid);
            }
            else
            {
                sprintf(tempbuf, "%s%d-%d", tempbuf, start_vid, end_vid);
            }
            vid_str_len = strlen(tempbuf);
            if((vid_str_len < 50) && (vid_allow_count < 199))
                sprintf(tempbuf, "%s,", tempbuf);
            else
            {
                sprintf(tempbuf, "%s\n", tempbuf);
                len += strlen(tempbuf);
                if(len < size)
                    strcat(showStr, tempbuf);
    		    ret = 1;
                vid_str_len = 0;
                vid_allow_count = 0;
            }

            
            if(vlan.isStatic)
            {
                start_vid = vid;
                end_vid = vid;
                vid_allow_count++;
            }
            else
            {
                start_vid = end_vid = 0;
            }
        }
    }

    if(start_vid != 0)
    {
        if(0 == vid_str_len)
             sprintf(tempbuf, " switchport access allowed vlan ");
        if(start_vid == end_vid)
        {
            sprintf(tempbuf, "%s%d\n", tempbuf, start_vid);
        }
        else
        {
            sprintf(tempbuf, "%s%d-%d\n", tempbuf, start_vid, end_vid);
        }
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
        vid_str_len = 0;
        vid_allow_count = 0;

    }
    start_vid = 0;
    NPD_VBMP_ITER(switch_port.allow_tag_vlans, vid)
    {
        memset(&vlan, 0, sizeof(vlan));
        vlan.vid = vid;
        ret = dbtable_sequence_search(g_vlans, vid, &vlan);
        if(0 != ret)
            continue;
		if(vid == DEFAULT_VLAN_ID)
		{
			continue;
		}
        if(start_vid == 0)
        {
            start_vid = vid;
            end_vid = vid;
            vid_allow_count++;
        }
        else if((vid == end_vid+1)
                  && (vlan.isStatic)
                 && (vid_allow_count < 199))
        {
            end_vid = vid;
            vid_allow_count++;
        }
        else
        {
            if(0 == vid_str_len)
                sprintf(tempbuf, " switchport trunk allowed vlan ");
            if(start_vid == end_vid)
            {
                sprintf(tempbuf, "%s%d", tempbuf,start_vid);
            }
            else
            {
                sprintf(tempbuf, "%s%d-%d", tempbuf, start_vid, end_vid);
            }
            vid_str_len = strlen(tempbuf);
            if((vid_str_len < 50) && (vid_allow_count < 199))
                sprintf(tempbuf, "%s,", tempbuf);
            else
            {
                sprintf(tempbuf, "%s\n", tempbuf);
                len += strlen(tempbuf);
                if(len < size)
                    strcat(showStr, tempbuf);
    		    ret = 1;
                vid_str_len = 0;
                vid_allow_count = 0;
            }
            if(vlan.isStatic)
            {
                start_vid = vid;
                end_vid = vid;
                vid_allow_count++;
            }
            else
            {
                start_vid = end_vid = 0;
            }
        }
    }

    if(start_vid != 0)
    {
        if(0 == vid_str_len)
                sprintf(tempbuf, " switchport trunk allowed vlan ");
        if(start_vid == end_vid)
        {
            sprintf(tempbuf, "%s%d\n", tempbuf, start_vid);
        }
        else
        {
            sprintf(tempbuf, "%s%d-%d\n", tempbuf, start_vid, end_vid);
        }
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
        vid_str_len = 0;

    }
    if(switch_port.pvid != 0 && switch_port.pvid != DEFAULT_VLAN_ID)
    {
        sprintf(tempbuf, " switchport pvid %d\n", switch_port.pvid);
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }
    if(switch_port.inner_pvid != 0 && switch_port.inner_pvid != DEFAULT_VLAN_ID)
    {
        sprintf(tempbuf, " switchport inner pvid %d\n", switch_port.inner_pvid);
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }
	
    if(switch_port.fdb_limit != -1)
    {
        sprintf(tempbuf, " switchport mac-address-table limit %d\n", switch_port.fdb_limit);
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }

    if(switch_port.fdb_learning_mode == 2)
    {
        sprintf(tempbuf, " switchport protect\n");
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
        
    }
    if(switch_port.fdb_learning_mode == 4)/*PORT_SECURITY_STICKY*/
    {
        sprintf(tempbuf, " switchport mac-address-table sticky\n");
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
        
    }
    if(switch_port.mac_vlan_flag)
    {
        if(!switch_port.prefer_subnet)
        {
            sprintf(tempbuf, " switchport mac-based-vlan preferred\n");
        }
        else
        {
            sprintf(tempbuf, " switchport mac-based-vlan\n");
        }
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }

    if(switch_port.subnet_vlan_flag)
    {
        if(switch_port.prefer_subnet)
        {
            sprintf(tempbuf, " switchport subnet-based-vlan preferred\n");
        }
        else
        {
            sprintf(tempbuf, " switchport subnet-based-vlan\n");
        }
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }
#ifndef HAVE_PRIVATE_VLAN
	if (switch_port.vlan_private_mode != SWITCH_PORT_PRIVLAN_PROMI)
	{
		if (switch_port.vlan_private_mode == SWITCH_PORT_PRIVLAN_ISOLATED)
		{
			len += sprintf(tempbuf, " switchport private-vlan isolated\n");
		}
		else if (switch_port.vlan_private_mode == SWITCH_PORT_PRIVLAN_COMMUNITY)
		{
			len += sprintf(tempbuf, " switchport private-vlan community\n");
		}
		
        if(len < size)
            strcat(showStr, tempbuf);	
		ret = 1;
	}
#endif	
	
    if(switch_port.vlan_efilter_mode)
    {
    }

    if(switch_port.vlan_infilter_mode)
    {
    }
#ifdef HAVE_QINQ
    if(switch_port.access_qinq)
    {
        sprintf(tempbuf," switchport access qinq\n");
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
            
    }
    if(switch_port.qinq_drop_miss)
    {
        sprintf(tempbuf, " switchport qinq miss drop\n");
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;
    }
#endif	
#ifdef HAVE_PORT_ISOLATE	
	if (switch_port.port_isolate)
	{
        sprintf(tempbuf, " switchport isolate\n");
        len += strlen(tempbuf);
        if(len < size)
            strcat(showStr, tempbuf);
		ret = 1;		
	}
#endif	
	
#ifdef HAVE_QINQ	
    if(switch_port.access_qinq)
    {
		int op_ret;
        char outerstr[64] = {0};
        char innerstr[64] = {0};
        char eouterstr[64] = {0};
        char einnerstr[64] = {0};
        vlan_xlate_db_entry_t entry = {0};
        vlan_xlate_db_entry_t entry_out = {0};
		

        entry.netif_index = switch_port.global_port_ifindex;
        op_ret = dbtable_hash_head_key(vlan_xlate_table_netif_hash,  &entry, &entry_out, 
            &vlan_xlate_netif_show_filter);

        while(0 == op_ret)
        {
			memcpy(&entry, &entry_out, sizeof(vlan_xlate_db_entry_t));
			
            if(entry.ingress_outer_start_vid != 0)
                sprintf(outerstr, "outer vlan %d", entry.ingress_outer_start_vid);
            if(entry.ingress_outer_vid_num > 1)
                sprintf(outerstr, "%s-%d", outerstr, 
                          entry.ingress_outer_start_vid+entry.ingress_outer_vid_num-1);
            if(entry.ingress_inner_start_vid != 0)
                sprintf(innerstr, "inner vlan %d", entry.ingress_inner_start_vid);
            if(entry.ingress_inner_vid_num > 1)
                sprintf(innerstr, "%s-%d", innerstr, 
                          entry.ingress_inner_start_vid+entry.ingress_inner_vid_num-1);
            if(entry.egress_outer_vid)
                sprintf(eouterstr, "outer vlan %d", entry.egress_outer_vid);
            if(entry.egress_inner_vid)
                sprintf(einnerstr, "inner vlan %d", entry.egress_inner_vid);
            switch(entry.xlate_type)
            {
                case XLATE_POP_INNER:
                    len += sprintf(tempbuf, " switchport egress %s %s pop inner\n", outerstr, innerstr);
                    break;
                case XLATE_POP_OUTER:
                    len += sprintf(tempbuf, " switchport egress %s %s pop outer\n", outerstr, innerstr);
                    break;
                case XLATE_POP_BOTH:
                    len += sprintf(tempbuf, " switchport egress %s %s pop 2\n", outerstr, innerstr);
                    break;
                case XLATE_PUSH_OUTER:
                    len += sprintf(tempbuf, " switchport ingress %s %s push %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_PUSH_INNER:
                    len += sprintf(tempbuf, " switchport ingress %s %s push %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_PUSH_BOTH:
                    len += sprintf(tempbuf, " switchport ingress push %s %s\n", eouterstr, einnerstr);
                    break;
                case XLATE_REWRITE_OUTER:
                    len += sprintf(tempbuf, " switchport ingress %s %s translate %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_REWRITE_INNER:
                    len += sprintf(tempbuf, " switchport ingress %s %s translate %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_REWRITE_BOTH:
                    len += sprintf(tempbuf, " switchport ingress %s %s translate %s %s\n", outerstr, innerstr, eouterstr, einnerstr);
                    break;
                case XLATE_E_REWRITE_OUTER:
                    len += sprintf(tempbuf, " switchport egress %s %s translate %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_E_REWRITE_INNER:
                    len += sprintf(tempbuf, " switchport egress %s %s translate %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_E_REWRITE_BOTH:
                    len += sprintf(tempbuf, " switchport egress %s %s translate %s %s\n", outerstr, innerstr, eouterstr, einnerstr);
                    break;
                default :
                    break;
                   
            }
            
            if(len < size)
                strcat(showStr, tempbuf);
            ret = 1;

			
            op_ret = dbtable_hash_next_key(vlan_xlate_table_netif_hash,  &entry, &entry_out, 
                     &vlan_xlate_netif_show_filter);

        }
    }

	if (switch_port.access_qinq)
	{
		int op_ret;
        vlan_eline_db_entry_t entry = {0};
        vlan_eline_db_entry_t entry_out = {0};
		unsigned int netif_index = switch_port.global_port_ifindex;

		entry.netif_index_first = netif_index;
		entry.netif_index_second = netif_index;
		
        op_ret = dbtable_hash_head(vlan_eline_vlan_hash,  &entry, &entry_out, 
            &npd_vlan_eline_netif_filter);

		while (0 == op_ret)
		{
			len += sprintf(tempbuf," switchport e-line %d\n", entry_out.eline_id);
            if(len < size)
                strcat(showStr, tempbuf);
            ret = 1;

			memcpy(&entry, &entry_out, sizeof(vlan_eline_db_entry_t));
			entry.netif_index_first = entry.netif_index_second = netif_index;
			op_ret = dbtable_hash_next(vlan_eline_vlan_hash,  &entry, &entry_out, 
	            &npd_vlan_eline_netif_filter);	
		}
	}
#endif
	{
		proto_vlan_port_t protovlan;
		unsigned int g_ifindex;
		int i;
		int op_ret;
		

		npd_switch_port_netif_index(switch_port.switch_port_index, &g_ifindex);
		for(i = 0; i <= NPD_MAX_PROTO_VLAN_ID; i++)
		{
		    protovlan.netif_index = g_ifindex;
		    protovlan.proto_group_index = i;
		
		    op_ret = dbtable_hash_search(proto_vlanport_hash, &protovlan, NULL, &protovlan);
		    if(0 != op_ret)
		    {
		    	continue;
		    }		
			len += sprintf(tempbuf," switchport protocol-vlan %d vlan %d\n", 
				protovlan.proto_group_index, protovlan.vid);	
        	if(len < size)
            	strcat(showStr, tempbuf);		
			ret = 1;			
		}
	}
    return ret;
}

#ifdef __cplusplus
}
#endif

