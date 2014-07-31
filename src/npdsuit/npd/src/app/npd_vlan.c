/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_vlan.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		API used in NPD for VLAN module.
*
* DATE:
*		02/21/2010	
*
*UPDATE:
*04/26/2010              zhengzw@autelan.com          Unifying netif index formate with vlan and port-channel
*05/10/2010              zhengzw@autelan.com          Using DB.
*06/11/2010              zhengzw@autelan.com          L3 interface supported.
*08/12/2010              chengjun@autelan.com           Add switchport bitmap.
*09/27/2010              chengjun@autelan.com           Add Q-in-Q supporting.
*  FILE REVISION NUMBER:
*  		$Revision: 1.186 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_vlan.h"

db_table_t *vlan_db = NULL;
sequence_table_index_t *g_vlans = NULL;

db_table_t *macvlan_db = NULL;
hash_table_index_t *mac_vlan_hash = NULL;
sequence_table_index_t *mac_vlan_vid_sequence = NULL;

db_table_t *subnetvlan_db = NULL;
hash_table_index_t *subnet_vlan_hash = NULL;
sequence_table_index_t *subnet_vlan_vid_sequence = NULL;

db_table_t *proto_vlanport_db = NULL;
hash_table_index_t *proto_vlanport_hash = NULL;

db_table_t *proto_vlan_db = NULL;
array_table_index_t *proto_vlan_array = NULL;
#ifdef HAVE_QINQ
db_table_t *vlan_xlate_table_db = NULL;
hash_table_index_t *vlan_xlate_table_hash = NULL;

#define VLAN_XLATE_NETIF_HASH_KEY_MAX 256
hash_table_index_t *vlan_xlate_table_netif_hash = NULL;

#define VLAN_ELINE_VLAN_HASH_KEY_MAX 256
db_table_t *vlan_eline_table_db = NULL;
array_table_index_t *vlan_eline_array = NULL;
hash_table_index_t *vlan_eline_vlan_hash = NULL;
hash_table_index_t *vlan_eline_netif_hash = NULL;

db_table_t         *npd_vlan_table_qinqcfg = NULL;
array_table_index_t *npd_vlan_qinq_index = NULL;
#endif

unsigned int g_tpid = 0x8100;
unsigned int g_inner_tpid = 0x8100;

unsigned int vlan_qinq_global_no = 0;

int vlan_mtu = NPD_VLAN_MTU_VALUE;
int bcast_ratelimit = NPD_FALSE;
unsigned char g_vlan_egress_filter = NPD_TRUE;


#ifdef HAVE_PRIVATE_VLAN
#define NPD_PVLAN_PRIMARY_SIZE	4
#define NPD_PVLAN_ISOLATE_SIZE	32

enum NPD_PVLAN_VLAN_TYPE
{
	PVLAN_VLAN_TYPE_NORMAL = 0,
	PVLAN_VLAN_TYPE_PRIMARY,
	PVLAN_VLAN_TYPE_ISOLATE,
	PVLAN_VLAN_TYPE_COMMUNITY,
	PVLAN_VLAN_TYPE_MAX
};


db_table_t *pvlan_primary_db = NULL;
sequence_table_index_t *pvlan_primary_vid_sequence = NULL;

db_table_t *pvlan_isolate_db = NULL;
sequence_table_index_t *pvlan_isolate_vid_sequence = NULL;

unsigned int npd_vlan_get_assoc_primary_vid
(
	unsigned int secondary_vlan
);
unsigned int npd_vlan_pvlan_delete_by_id(unsigned int vid);
unsigned int npd_vlan_pvlan_port_config(
	unsigned int netif_index,
	unsigned short vlan_id,
	unsigned char is_tagged,
	unsigned char is_add,
	unsigned int *pvlan_type
	
);


unsigned int npd_vlan_pvlan_promis_port_del(
	unsigned int netif_index, 
	unsigned int primary_vlan
);

unsigned int npd_vlan_pvlan_isolate_port_del(
	unsigned int netif_index, 
	unsigned int isolate_vlan
);



extern int nam_pvlan_associate(unsigned int primary_vlan, unsigned int secondary_vlan);
extern int nam_pvlan_no_associate(unsigned int primary_vlan, unsigned int secondary_vlan);
extern int nam_pvlan_isolate_port_set(unsigned int netif_index, unsigned int isolate_vid, int enable);
extern int nam_pvlan_promis_port_set(unsigned int netif_index, unsigned int primary_vid, int enable);
extern int npd_netif_free_all_pvlan(unsigned int netif_index);

#endif

#ifdef HAVE_PORT_ISOLATE
extern int isolate_group_global_no;
extern array_table_index_t *isolate_ports;
#endif
	


/*以下的函数在非database的updata/insert/delete处理函数之外
  作为访问数据库的必须调用函数，以防止因为数据库不一致导致的
  问题*/
void npd_key_database_lock()
{
	dbtable_sequence_lock(g_vlans);
	dbtable_sequence_lock(g_eth_ports);
	dbtable_sequence_lock(g_trunks);
    dbtable_array_lock(switch_ports);
}

void npd_key_database_unlock()
{
    dbtable_array_unlock(switch_ports);
	dbtable_sequence_unlock(g_trunks);
	dbtable_sequence_unlock(g_eth_ports);
	dbtable_sequence_unlock(g_vlans);
}

void npd_vlan_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    struct vlan_s *vlan = malloc(sizeof(struct vlan_s));
    int type = npd_netif_type_get(netif_index);
    unsigned int vid;
    int ret;
    npd_vbmp_t vbmp;

    if((NULL == switch_port) || (NULL == vlan))
    {
        npd_syslog_err("% ERROR: SYSTEM resourse exhausts\n");
        goto error;
    }

    if((type != NPD_NETIF_ETH_TYPE)
        &&(type != NPD_NETIF_TRUNK_TYPE) 
#ifdef HAVE_CAPWAP_ENGINE		
        &&(type != NPD_NETIF_WIRELESS_TYPE)
#endif //HAVE_CAPWAP_ENGINE		
        )
        goto error;
    memset(switch_port, 0, sizeof(switch_port_db_t));
    switch_port->global_port_ifindex = netif_index;
    switch(evt)
    {
    case PORT_NOTIFIER_L2CREATE:
        npd_netif_allow_vlan(netif_index, DEFAULT_VLAN_ID, FALSE, TRUE);
        break;
    case PORT_NOTIFIER_L2DELETE:
#ifdef HAVE_QINQ		
        npd_netif_free_all_xlate(netif_index);
#endif
#ifdef HAVE_PRIVATE_VLAN
        npd_netif_free_all_pvlan(netif_index);
#endif
        npd_netif_free_allvlan(netif_index);
        /*second free for avoid add netif_index to default*/
        npd_netif_free_allvlan(netif_index);
        break;
    case PORT_NOTIFIER_LINKUP_E:
        ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
        if(0 != ret)
        {
            npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
            goto error;
        }
	
        switch_port->link_state = PORT_LINK_UP;
        dbtable_array_update(switch_ports, switch_port->switch_port_index, 
             NULL, switch_port);
        vbmp = switch_port->allow_tag_vlans;
        NPD_VBMP_OR(vbmp, switch_port->allow_untag_vlans);
        NPD_VBMP_ITER(vbmp, vid)
        {
            vlan->vid = vid;
            ret = dbtable_sequence_search(g_vlans, vid, vlan);
            if(0 != ret)
            {
                continue;
            }
            if(vlan->link_status == PORT_LINK_DOWN)
            {
                vlan->link_status = PORT_LINK_UP;
                dbtable_sequence_update(g_vlans, vid, NULL, vlan);
                netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_LINKUP_E);
            }
        }
        break;
    case PORT_NOTIFIER_INSERT:
        ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
        if(0 != ret)
        {
            npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
            goto error;
        }
	
        switch_port->state = ONLINE_INSERT;
        dbtable_array_update(switch_ports, switch_port->switch_port_index,
            NULL, switch_port);
		
        break;
    case PORT_NOTIFIER_REMOVE:
        ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
        if(0 != ret)
        {
            npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
            goto error;
        }
	
        switch_port->link_state = PORT_LINK_DOWN;
        switch_port->state = ONLINE_REMOVE;
        dbtable_array_update(switch_ports, switch_port->switch_port_index, 
             NULL, switch_port);
		
        vbmp = switch_port->allow_tag_vlans;
        NPD_VBMP_OR(vbmp, switch_port->allow_untag_vlans);
        NPD_VBMP_ITER(vbmp, vid)
        {
            unsigned int port;
            vlan->vid = vid;
            ret = dbtable_sequence_search(g_vlans, vid, vlan);
            if(0 != ret)
            {
                continue;
            }
            if(vlan->link_status == PORT_LINK_UP)
            {
                int link_status = PORT_LINK_DOWN;
                npd_pbmp_t pbmp;
                
                NPD_PBMP_ASSIGN(pbmp,vlan->tag_ports);
                NPD_PBMP_OR(pbmp,vlan->untag_ports);
                NPD_PBMP_ITER(pbmp, port)
                {
                    ret = npd_switch_port_link_state_get(port, &link_status);
                    if(-1 == ret)
                        continue;
                    else if(link_status == PORT_LINK_UP)
                        break;
                }
                vlan->link_status = link_status;
                dbtable_sequence_update(g_vlans, vid, NULL, vlan);
                if(vlan->link_status == PORT_LINK_DOWN)
                    netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_LINKDOWN_E);
            }
        }
        
        break;
    case PORT_NOTIFIER_LINKDOWN_E:
        ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
        if(0 != ret)
        {
            npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
            goto error;
        }
	
        switch_port->link_state = PORT_LINK_DOWN;
        dbtable_array_update(switch_ports, switch_port->switch_port_index, 
             NULL, switch_port);
		
        vbmp = switch_port->allow_tag_vlans;
        NPD_VBMP_OR(vbmp, switch_port->allow_untag_vlans);
        NPD_VBMP_ITER(vbmp, vid)
        {
            unsigned int port;
            vlan->vid = vid;
            ret = dbtable_sequence_search(g_vlans, vid, vlan);
            if(0 != ret)
            {
                continue;
            }
            if(vlan->link_status == PORT_LINK_UP)
            {
                int link_status = PORT_LINK_DOWN;
                npd_pbmp_t pbmp;
                
                NPD_PBMP_ASSIGN(pbmp,vlan->tag_ports);
                NPD_PBMP_OR(pbmp,vlan->untag_ports);
                NPD_PBMP_ITER(pbmp, port)
                {
                    ret = npd_switch_port_link_state_get(port, &link_status);
                    if(-1 == ret)
                        continue;
                    else if(link_status == PORT_LINK_UP)
                        break;
                }
                vlan->link_status = link_status;
                dbtable_sequence_update(g_vlans, vid, NULL, vlan);
                if(vlan->link_status == PORT_LINK_DOWN)
                    netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_LINKDOWN_E);
            }
        }
        
        break;
    default:
        break;
    }

error:
    if(vlan)
        free(vlan);
    if(switch_port)
        free(switch_port);
    return;
}

void npd_vlan_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    struct vlan_s *vlan = malloc(sizeof(struct vlan_s));
    int type = npd_netif_type_get(netif_index);
    unsigned long vlan_type = npd_netif_type_get(vlan_index);
    unsigned int vid = npd_netif_vlan_get_vid(vlan_index);
    int ret;
    int link_status;

    if(NPD_NETIF_VLAN_TYPE != vlan_type)
        goto over;
    if((NULL == switch_port) || (NULL == vlan))
    {
        npd_syslog_err("% Error: SYSTEM resourse exhausts\n");
        goto over;
    }

    if((type != NPD_NETIF_ETH_TYPE)
        &&(type != NPD_NETIF_TRUNK_TYPE)
#ifdef HAVE_CAPWAP_ENGINE		
        &&(type != NPD_NETIF_WIRELESS_TYPE)
#endif //HAVE_CAPWAP_ENGINE		
		)
    {
        goto over;
    }
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
        goto over;
    }
    vlan->vid = vid;
    ret = dbtable_sequence_search(g_vlans, vid, vlan);
    if(0 != ret)
    {
        npd_syslog_dbg("% Can not get vlan entity for vid %d", vid);
        goto over;
    }

    link_status = switch_port->link_state;    
    if((vlan->link_status == PORT_LINK_DOWN)
        && (event == PORT_NOTIFIER_JOIN))
    {
        if(link_status == PORT_LINK_UP)
        {
            vlan->link_status = PORT_LINK_UP;
            dbtable_sequence_update(g_vlans, vid, NULL, vlan);
            netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_LINKUP_E);
        }
    }
    else if((vlan->link_status == PORT_LINK_UP)
        && (event == PORT_NOTIFIER_LEAVE))
    {
        unsigned int port;
        npd_pbmp_t pbmp;
        
        link_status = PORT_LINK_DOWN;
        NPD_PBMP_ASSIGN(pbmp, vlan->tag_ports);
        NPD_PBMP_OR(pbmp,vlan->untag_ports);
        NPD_PBMP_ITER(pbmp, port)
        {
            ret = npd_switch_port_link_state_get(port, &link_status);
            if(-1 == ret)
                continue;
            else if(link_status == PORT_LINK_UP)
                break;
        }
        if(vlan->link_status != link_status)
        {
            vlan->link_status = link_status;
            dbtable_sequence_update(g_vlans, vid, NULL, vlan);
        }
        if(vlan->link_status == PORT_LINK_DOWN)
            netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_LINKDOWN_E);
    }
	else if (vlan->forward_mode != FORWARD_BRIDGING 
		&& (event == PORT_NOTIFIER_LEAVE))
	{
#ifdef HAVE_QINQ
		vlan_eline_db_entry_t entry = {0};
		vlan_eline_db_entry_t entry_out = {0};

		entry.outer_vid = vlan->vid;
		entry.netif_index_first = netif_index;
		entry.netif_index_second = netif_index;

		ret = dbtable_hash_head(vlan_eline_vlan_hash, 
			&entry, &entry_out, npd_vlan_eline_netif_outer_filter);
		while (ret == 0)
		{
			memcpy(&entry, &entry_out, sizeof(vlan_eline_db_entry_t));
			entry.netif_index_first = netif_index;
			entry.netif_index_second = netif_index;
			
			if (entry_out.netif_index_first == netif_index)
			{
				entry_out.netif_index_first = 0;
			}

			if (entry_out.netif_index_second == netif_index)
			{
				entry_out.netif_index_second = 0;
			}
			
			ret = dbtable_hash_update(vlan_eline_vlan_hash, NULL, &entry_out);
			if (ret != 0)
			{
				break;
			}
			ret = dbtable_hash_next(vlan_eline_vlan_hash, 
				&entry, &entry_out, npd_vlan_eline_netif_outer_filter);
		}
#endif		
	}
over:
	if(switch_port)
	{
		free(switch_port);
	}
	if(vlan)
	{
		free(vlan);
	}
}

netif_event_notifier_t vlan_netif_notifier =
{
    .netif_event_handle_f = &npd_vlan_notify_event,
    .netif_relate_handle_f = &npd_vlan_relate_event
};

/**********************************************************************************
 *  npd_create_vlan_by_vid
 *
 *	DESCRIPTION:
 * 		Create static vlan node by VLAN ID
 * 		Vlan node should be have not been created before.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		pointer to vlan being created.
 *
 **********************************************************************************/
struct vlan_s* npd_create_vlan_by_vid
(
	unsigned short vlanId
)
{
	struct vlan_s* node = NULL;
    if((1 > vlanId) || (NPD_MAX_VLAN_ID < vlanId))
        return NULL;
	node = (struct vlan_s *)malloc(sizeof(struct vlan_s));
	if(NULL == node) {
		 syslog_ax_vlan_err("create vlan by vid memory alloc fail!\n");
		return NULL;
	}
	memset(node,0,sizeof(struct vlan_s));
	node->vid = vlanId;
	node->isStatic = NPD_TRUE;
    node->fdb_learning_mode = TRUE;
	node->fdb_limit = -1;
	npd_key_database_lock();
    dbtable_sequence_insert(g_vlans, vlanId, node);
    netif_notify_event(npd_netif_vlan_index(vlanId), PORT_NOTIFIER_CREATE);
	npd_key_database_unlock();
	return node;
}

/**********************************************************************************
 *  npd_create_vlan_by_vid_dynamic
 *
 *	DESCRIPTION:
 * 		Create a dynamic vlan node by VLAN ID
 * 		Vlan node should be have not been created before.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		pointer to vlan being created.
 *
 **********************************************************************************/
struct vlan_s* npd_create_vlan_by_vid_dynamic
(
	unsigned short vlanId
)
{
	char vlanName[ALIAS_NAME_SIZE] = {0};
	struct vlan_s* node = NULL;
    if((1 > vlanId) || (NPD_MAX_VLAN_ID < vlanId))
        return NULL;
    sprintf(vlanName, "VLAN%.4d", vlanId);
	node = (struct vlan_s *)malloc(sizeof(struct vlan_s));
	if(NULL == node) {
		 syslog_ax_vlan_err("create vlan by vid memory alloc fail!\n");
		return NULL;
	}
	memset(node,0,sizeof(struct vlan_s));
	strncpy(node->name , vlanName ,NPD_VLAN_IFNAME_SIZE );
	node->vid = vlanId;
	node->isStatic = NPD_FALSE;
    node->fdb_learning_mode = TRUE;
	node->fdb_limit = -1;
	npd_key_database_lock();
    dbtable_sequence_insert(g_vlans, vlanId, node);
    netif_notify_event(npd_netif_vlan_index(vlanId), PORT_NOTIFIER_CREATE);
	npd_key_database_unlock();
	return node;
}

/**********************************************************************************
 *  npd_delete_vlan_by_vid
 *
 *	DESCRIPTION:
 * 		Delete vlan node by VLAN ID
 * 		Vlan node should be have been created before.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_VLAN_ERR_NONE - if no error occurred
 *		NPD_VLAN_NOTEXISTS - if vlan does not exists
 *		
 *
 **********************************************************************************/
int npd_delete_vlan_by_vid
(
	unsigned short vlanId
)
{
	struct vlan_s* node = malloc(sizeof(struct vlan_s));
    unsigned int switch_port_index;
    unsigned int eth_g_index;
    unsigned int Tagged = 1,UnTagged = 0;
	int retval = VLAN_RETURN_CODE_ERR_NONE;
    int ret;

    if(NULL == node)
        return COMMON_RETURN_CODE_NO_RESOURCE;
#ifdef HAVE_PRIVATE_VLAN	
	ret = npd_vlan_pvlan_delete_by_id(vlanId);
	if (ret != 0)
	{
		free(node);
		return ret;
	}
#endif
		
    node->vid = vlanId;
	npd_key_database_lock();
	retval = dbtable_sequence_search(g_vlans, vlanId, node);

    if(0 != retval){
	 	retval = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	}
	else 
    {
		if(node->isAutoCreated){
			retval = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		}
		else
        {
            NPD_PBMP_ITER(node->untag_ports, switch_port_index)
            {
                ret = npd_switch_port_netif_index(switch_port_index, &eth_g_index);
                if(-1 == ret)
                    continue;
                retval = npd_netif_free_vlan(eth_g_index, vlanId, UnTagged);
            }
            
            NPD_PBMP_ITER(node->tag_ports, switch_port_index)
            {
                ret = npd_switch_port_netif_index(switch_port_index, &eth_g_index);
                if(-1 == ret)
                    continue;
                retval = npd_netif_free_vlan(eth_g_index, vlanId, Tagged);
            }
            
            netif_notify_event(npd_netif_vlan_get_index(vlanId),
                PORT_NOTIFIER_DELETE);
            netif_app_notify_event(npd_netif_vlan_get_index(vlanId), PORT_NOTIFIER_DELETE, NULL, 0);
		    dbtable_sequence_delete(g_vlans, vlanId, (void*)node, (void*)node);
		}
	}
	npd_key_database_unlock();
	free(node);
	return retval;
}

/**********************************************************************************
 *  npd_create_vlan_by_name
 *
 *	DESCRIPTION:
 * 		Create a static vlan node by VLAN ID
 * 		Vlan node should be have not been created before.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		pointer to vlan being created.
 *
 **********************************************************************************/
struct vlan_s* npd_create_vlan_by_name
(
	char *name,
	unsigned int vid
)
{
	struct vlan_s* node = NULL;
    int ret;
    
	if(NULL == name) {
		return  NULL;
	}
	node = (struct vlan_s *)malloc(sizeof(struct vlan_s));
	if(NULL == node) {
		 syslog_ax_vlan_err("error:null memory allocated!\n");
		return NULL;
	}
	memset(node,0,sizeof(struct vlan_s));
	strncpy(node->name , name ,NPD_VLAN_IFNAME_SIZE );
    node->vid = vid;
	node->isStatic = NPD_TRUE;
    node->fdb_learning_mode = TRUE;
	node->fdb_limit = -1;
	npd_key_database_lock();
    ret = dbtable_sequence_insert(g_vlans, vid, node);

    if(-1 == ret)
    {
        syslog_ax_vlan_err("Can not create vlan %d\n", vid);
		npd_key_database_unlock();
        free(node);
        return NULL;
    }
    netif_notify_event(npd_netif_vlan_index(vid), PORT_NOTIFIER_CREATE);
	npd_key_database_unlock();			
	return node;
}

/**********************************************************************************
 *  npd_find_vlan_by_vid
 *
 *	DESCRIPTION:
 * 		Check out if specified vlan has been created or not
 * 		VLAN ID used as an index to find vlan structure.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL  - if parameters given are wrong
 *		vlanNode	- if vlan has been created before 
 *		
 **********************************************************************************/
struct vlan_s *npd_find_vlan_by_vid
(
	unsigned short vlanId
)
{
	struct vlan_s* node = NULL;
    int ret;
    
	node = (struct vlan_s *)malloc(sizeof(struct vlan_s));
	if(NULL == node) {
		 syslog_ax_vlan_err("create vlan by vid memory alloc fail!\n");
		return NULL;
	}
	memset(node,0,sizeof(struct vlan_s));
	node->vid = vlanId;
    ret = dbtable_sequence_search(g_vlans, vlanId, node);
    if(0 != ret)
    {
        free(node);
        return NULL;
    }
    else
	    return node;
}


/**********************************************************************************
 *  npd_find_vlan_by_name
 *
 *	DESCRIPTION:
 * 		Check out if specified vlan has been created or not
 * 		name is used to compare with each vlan exists.
 *
 *	INPUT:
 *		name - vlan name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		pointer to vlan found.
 *
 **********************************************************************************/
struct vlan_s* npd_find_vlan_by_name
(
	char *name
)
{
	struct vlan_s* node = NULL;
    int ret;
	int iter = 0;

	node = (struct vlan_s *)malloc(sizeof(struct vlan_s));
	if(NULL == node) {
		 syslog_ax_vlan_err("create vlan by vid memory alloc fail!\n");
		return NULL;
	}

	for(iter = 0;iter < NPD_VLAN_NUMBER_MAX; iter++) {
        node->vid = iter;
		ret = dbtable_sequence_search(g_vlans, iter, node);
        if(0 != ret)
            continue;
        if(!strcmp(name, node->name))
            return node;
	}

	free(node);
	return NULL;
}

unsigned short npd_vlan_get_id_by_name( char *name)
{
	struct vlan_s* node = NULL;
    unsigned short vid = 0;

	node = npd_find_vlan_by_name( name);
	if(NULL == node) {
		 syslog_ax_vlan_err("create vlan by vid memory alloc fail!\n");
		return 0;
	}

	vid = node->vid;
	free(node);

	return vid;
}

int npd_put_vlan(struct vlan_s *vlan)
{
	struct vlan_s vlanNode;
    int ret;
    if(vlan == NULL)
    {
		return 0;
    }
	vlanNode.vid = vlan->vid;
    ret = dbtable_sequence_search(g_vlans, vlan->vid, &vlanNode);
    if(0 != memcmp(vlan, &vlanNode, sizeof(struct vlan_s))) {
    	ret = dbtable_sequence_update(g_vlans, vlan->vid, NULL, vlan);
    }
    free(vlan);
    return ret;
}

int create_default_vlan(void)
{
	struct vlan_s *vlan_1;
	char	vlanName[ALIAS_NAME_SIZE] = "DEFAULT";
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

	vlan_1 = npd_create_vlan_by_vid(DEFAULT_VLAN_ID);
	if( vlan_1 != NULL ) {
	    strcpy(vlan_1->name, vlanName);
    	vlan_1->isAutoCreated = TRUE;
        vlan_1->fdb_learning_mode = TRUE;
		npd_put_vlan(vlan_1);
	}
	else {
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}
	nam_asic_vlan_entry_cpu_del(DEFAULT_VLAN_ID);
	return ret ;
}


int create_port_l3intf_vlan(void)
{
	struct vlan_s *vlan_max = NULL;
	char	vlanName[ALIAS_NAME_SIZE] = "vlan4095";
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

	vlan_max = npd_create_vlan_by_vid(NPD_PORT_L3INTF_VLAN_ID);
	if( vlan_max != NULL ) {
	    strcpy(vlan_max->name, vlanName);
    	vlan_max->isAutoCreated = TRUE;
        vlan_max->fdb_learning_mode = TRUE;
		vlan_max->pvlan_type = TRUE;
    	npd_put_vlan(vlan_max);
	}
	else {
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}

    nam_fdb_na_msg_l3portvlan_set(NPD_PORT_L3INTF_VLAN_ID, 1);
	return ret ;
}


/**********************************************************************************
 *
 * init global vlan structure:NPD_VLAN_NUMBER_MAX pointers point to detailde vlan structure.
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
unsigned int vlandb_index(unsigned int index)
{
    return index;
}

unsigned int vlandb_key(void* data)
{
    struct vlan_s * vlan = (struct vlan_s*)data;

    return vlan->vid;
}

int vlandb_cmp(void *data1, void *data2)
{
    struct vlan_s * vlan1 = (struct vlan_s*)data1;
    struct vlan_s * vlan2 = (struct vlan_s*)data2;

    return (vlan1->vid == vlan2->vid);
}

long vlandb_handle_insert(void *data);

long vlandb_handle_delete(void *data);

long vlandb_handle_update(void *newdata, void *olddata);


unsigned int macvlan_hash_mac(void *in)
{
	unsigned int key = 0; /*for hash key calculate*/
	unsigned int p = 16777619;
	unsigned char mac[MAC_ADDR_LEN] = {0};
	unsigned int tmpData = 0;
	int i = 0;
	macbase_vlan_t *data = (macbase_vlan_t *)in;
	
	if(NULL == data) {
		npd_syslog_err("npd mac based vlan make key null pointers error.");
		return ~0UI;
	}
	memcpy(mac,data->mac,MAC_ADDR_LEN);
	for(i = 0;i<6;i++){
        tmpData = mac[i];
		key = (key^tmpData)*p;
		key += key<<13;
		key ^= key>>7;
		key += key<<3;
		key ^= key>>17;
		key += key<<5;
	}
	key %= (NPD_MACBASE_VLAN_SIZE);
	
	return key;    
}


unsigned int subnetvlan_hash_ip(void *in)
{
    subnetbase_vlan_t *data = (subnetbase_vlan_t *)in;	
    unsigned int ipAddr = data->ipaddr & data->mask;
    unsigned int key;
		
	key = ((ipAddr>>22)& 0x07FF) + ((ipAddr>>11)& 0x07FF) + ((ipAddr)& 0x07FF);

	key %= NPD_SUBNETBASE_VLAN_SIZE;
	
	return key;    
}

unsigned int macvlan_sequence_index(unsigned int vid)
{
	unsigned int key = 0; /*for hash key calculate*/
	
	key = (vid)& 0xFFF ;
	
	key %= (NPD_MACBASE_VLAN_SIZE);
	
	return key;
}

unsigned int subnetvlan_sequence_index(unsigned int vid)
{
	unsigned int key = 0; /*for hash key calculate*/
	
	key = (vid)& 0xFFF ;
	
	key %= (NPD_SUBNETBASE_VLAN_SIZE);
	
	return key;
}

unsigned int macvlan_vid_key(void * in)
{
	macbase_vlan_t *data1 = (macbase_vlan_t *)in;
    return (data1->vid);
}

unsigned int subnetvlan_vid_key(void * in)
{
	subnetbase_vlan_t *data1 = (subnetbase_vlan_t *)in;
    return (data1->vid);
}

unsigned int macvlan_cmp_mac(void * in1, void * in2)
{
	macbase_vlan_t *data1 = (macbase_vlan_t *)in1;
	macbase_vlan_t *data2 = (macbase_vlan_t *)in2;
    return (0 == memcmp(data1->mac, data2->mac, sizeof(data1->mac)));
}

int macvlan_cmp_vid(void * in1, void * in2)
{
	macbase_vlan_t *data1 = (macbase_vlan_t *)in1;
	macbase_vlan_t *data2 = (macbase_vlan_t *)in2;
    return (data1->vid == data2->vid);
}

unsigned int subnetvlan_cmp_ip(void * in1, void * in2)
{
	subnetbase_vlan_t *data1 = (subnetbase_vlan_t *)in1;
	subnetbase_vlan_t *data2 = (subnetbase_vlan_t *)in2;	
    return ((data1->ipaddr&data1->mask) == (data2->ipaddr&data2->mask));
}

int subnetvlan_cmp_vid(void * in1, void * in2)
{
	subnetbase_vlan_t *data1 = (subnetbase_vlan_t *)in1;
	subnetbase_vlan_t *data2 = (subnetbase_vlan_t *)in2;		
    return (data1->vid == data2->vid);
}

long macvlandb_handle_insert(void *data)
{
    macbase_vlan_t *item = (macbase_vlan_t *)data;
    int ret;

    ret = nam_vlan_assoc_mac(item->vid, item->mac, item->tbl_index);
    npd_syslog_dbg("Assoc mac return code %d\n", ret);
    return ret;
}

long macvlandb_handle_delete(void *data)
{
    macbase_vlan_t *item = (macbase_vlan_t *)data;
    int ret;

    ret = nam_vlan_deassoc_mac(item->vid, item->mac, item->tbl_index);
    return ret;
    
}

unsigned int protovlandb_hash(void *data)
{
    proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
    unsigned int key = 0;

    key = (proto_vlan->proto_group_index & 0xf)<< 6;
    key += proto_vlan->netif_index % 64;

    key = key % PROTO_VLAN_PORT_TABLE_SIZE;
    return key;
}

unsigned int protovlandb_cmp(void *data1, void *data2)
{
    proto_vlan_port_t *proto_vlan1 = (proto_vlan_port_t*)data1;
    proto_vlan_port_t *proto_vlan2 = (proto_vlan_port_t*)data2;

    return ((proto_vlan1->netif_index == proto_vlan2->netif_index)
              && (proto_vlan1->proto_group_index == proto_vlan2->proto_group_index));
}

int protovlandb_index(unsigned int index)
{
    return index;
}

int protovlandb_key(void *data)
{
    proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
    return proto_vlan->proto_group_index;
}

int protovlandb_seq_cmp(void *data1, void *data2)
{
    proto_vlan_port_t *proto_vlan1 = (proto_vlan_port_t*)data1;
    proto_vlan_port_t *proto_vlan2 = (proto_vlan_port_t*)data2;

    return  (proto_vlan1->proto_group_index == proto_vlan2->proto_group_index);
}

long protovlandb_handle_insert(void *data)
{
    proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
    protobase_vlan_t proto_vlan_base;
	unsigned int netif_index;
    unsigned int array_port;
	unsigned int eth_g_index;
    int ret, all_ret;

    ret = dbtable_array_get(proto_vlan_array, proto_vlan->proto_group_index, 
                   &proto_vlan_base);

	netif_index = proto_vlan->netif_index;
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		struct trunk_s node = {0};

		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = nam_proto_vlan_port_add(eth_g_index, 
				proto_vlan->proto_group_index, 
				proto_vlan->vid, 
	        	proto_vlan_base.ether_frame, 
	        	proto_vlan_base.eth_type);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
	{
	    all_ret = nam_proto_vlan_port_add(proto_vlan->netif_index, 
			proto_vlan->proto_group_index, 
			proto_vlan->vid, 
	        proto_vlan_base.ether_frame, 
	        proto_vlan_base.eth_type);		
	}

	{
	    port_driver_t *driver 
			 = port_driver_get(netif_index);

		if(driver==NULL)
			return all_ret;
		
		if(driver->port_vlan_filter)
		{
		    all_ret = (*driver->port_vlan_filter)(netif_index, 0);/*NONE*/
		}	
		else
		{
			all_ret = NPD_FAIL;
		}
	}

    npd_syslog_dbg("Assoc protocol return code %d\n", all_ret);
    return all_ret;
}

long protovlandb_handle_delete(void *data)
{
    proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
    protobase_vlan_t proto_vlan_base;
	unsigned int netif_index;
    unsigned int array_port;
	unsigned int eth_g_index;
    int ret, all_ret;

    ret = dbtable_array_get(proto_vlan_array, proto_vlan->proto_group_index, 
                   &proto_vlan_base);
    
	netif_index = proto_vlan->netif_index;
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		struct trunk_s node = {0};

		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = nam_proto_vlan_port_del(eth_g_index, 
				proto_vlan->proto_group_index, 
				proto_vlan->vid, 
	        	proto_vlan_base.ether_frame, 
	        	proto_vlan_base.eth_type);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
	{
	    all_ret = nam_proto_vlan_port_del(proto_vlan->netif_index, 
			proto_vlan->proto_group_index, 
			proto_vlan->vid, 
	        proto_vlan_base.ether_frame, 
	        proto_vlan_base.eth_type);		
	}
	
	{
	    port_driver_t *driver 
			 = port_driver_get(netif_index);
		
		if(driver==NULL)
			return all_ret;
		
		if(driver->port_vlan_filter)
		{
		    all_ret = (*driver->port_vlan_filter)(netif_index, 3);/*INGRESS and EGRESS*/
		}	
		else
		{
			all_ret = NPD_FAIL;
		}
	}
	

    npd_syslog_dbg("Assoc protocol return code %d\n", all_ret);
    return all_ret;        
}

long protobasedb_handle_insert(void *data)
{
	protobase_vlan_t* proto_base = (protobase_vlan_t*)data;
    int all_ret;
	
	all_ret = nam_proto_class_add(proto_base->proto_group_index, 
		proto_base->ether_frame, proto_base->eth_type);

	return all_ret;
}

long protobasedb_handle_delete(void *data)
{
	protobase_vlan_t* proto_base = (protobase_vlan_t*)data;
    int all_ret;
	
	all_ret = nam_proto_class_del(proto_base->proto_group_index, 
		proto_base->ether_frame, proto_base->eth_type);

	return all_ret;
}


long subnetvlandb_handle_insert(void *data)
{
    subnetbase_vlan_t *item = (subnetbase_vlan_t *)data;
    int ret;

    ret = nam_vlan_assoc_subnet(item->vid, item->ipaddr, item->mask, item->tbl_index);
    npd_syslog_dbg("Assoc subnet return code %d\n", ret);

    return ret;
}

long subnetvlandb_handle_delete(void *data)
{
    subnetbase_vlan_t *item = (subnetbase_vlan_t *)data;
    int ret;

    ret = nam_vlan_deassoc_subnet(item->vid, item->ipaddr, item->mask, item->tbl_index);

    return ret;
}
#ifdef HAVE_QINQ
unsigned int vlan_xlate_db_hash(void *data)
{
    unsigned int key;
    vlan_xlate_db_entry_t *entry = (vlan_xlate_db_entry_t *)data;

	key = (entry->netif_index % 32) << 8;
	key += (entry->xlate_type & 0xf) << 4;
	key += (entry->ingress_outer_start_vid % 2) << 3;
	key += (entry->ingress_inner_start_vid % 2) << 2;
	key += (entry->egress_outer_vid % 2) << 1;
	key += (entry->egress_inner_vid % 2) ;
	key %= VLAN_XLATE_TABLE_SIZE;
	

   
    return key;
    
}

unsigned int vlan_xlate_db_cmp(void *data, void *data2)
{
    vlan_xlate_db_entry_t *entry1 = (vlan_xlate_db_entry_t *)data;
    vlan_xlate_db_entry_t *entry2 = (vlan_xlate_db_entry_t *)data2;

	if (entry1->netif_index == entry2->netif_index &&
		entry1->xlate_type == entry2->xlate_type &&
		entry1->ingress_inner_start_vid == entry2->ingress_inner_start_vid &&
		entry1->ingress_inner_vid_num == entry2->ingress_inner_vid_num &&
		entry1->ingress_outer_start_vid == entry2->ingress_outer_start_vid &&
		entry1->ingress_outer_vid_num == entry2->ingress_outer_vid_num &&
		entry1->egress_outer_vid == entry2->egress_outer_vid &&
		entry1->egress_inner_vid == entry2->egress_inner_vid)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

unsigned int vlan_xlate_db_netif_hash(void *data)
{
    int key;
    vlan_xlate_db_entry_t *entry = (vlan_xlate_db_entry_t *)data;

    if(npd_netif_type_get(entry->netif_index) == NPD_NETIF_ETH_TYPE)
        key = eth_port_array_index_from_ifindex(entry->netif_index) << 20;
    else if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(entry->netif_index))
        key  = npd_netif_trunk_get_tid(entry->netif_index);

    key %= VLAN_XLATE_NETIF_HASH_KEY_MAX;

    return key;
    
}

int vlan_xlate_db_netif_cmp(void *data1, void *data2)
{
    vlan_xlate_db_entry_t *entry1 = (vlan_xlate_db_entry_t *)data1;
    vlan_xlate_db_entry_t *entry2 = (vlan_xlate_db_entry_t *)data2;

    return (entry1->netif_index == entry2->netif_index);
}

long vlan_xlate_db_handle_insert(void *data)
{
    vlan_xlate_db_entry_t *entry = (vlan_xlate_db_entry_t *)data;
    int ret;

    ret = nam_vlan_xlate_entry_add(entry);

    return ret;
}

long vlan_xlate_db_handle_delete(void *data)
{
    vlan_xlate_db_entry_t *entry = (vlan_xlate_db_entry_t *)data;
    int ret;

    ret = nam_vlan_xlate_entry_del(entry);

    return ret;
    
}

unsigned int vlan_eline_db_vlan_hash(void *data)
{
    int key;
    vlan_eline_db_entry_t *entry = (vlan_eline_db_entry_t *)data;

	key = entry->outer_vid & 0xfff;
	key += (entry->inner_vid & 0xfff) << 12;
	
    key %= VLAN_ELINE_VLAN_HASH_KEY_MAX;

    return key;
}

unsigned int vlan_eline_vlan_cmp(void *data1, void *data2)
{
    vlan_eline_db_entry_t *entry1 = (vlan_eline_db_entry_t *)data1;
    vlan_eline_db_entry_t *entry2 = (vlan_eline_db_entry_t *)data2;

	if (entry1->outer_vid == entry2->outer_vid && 
		entry1->inner_vid == entry2->inner_vid)
	{
		return TRUE;
	}
	else
		return FALSE;
}

long vlan_eline_db_handle_update(void * newdata, void *olddata)
{
    vlan_eline_db_entry_t *new = (vlan_eline_db_entry_t*)newdata;
    vlan_eline_db_entry_t *old = (vlan_eline_db_entry_t*)olddata;
    int ret = 0;

	if (new->netif_index_first != 0 && 
		new->netif_index_second != 0)
	{
		ret = nam_vlan_eline_entry_add(new);
	}

	if (old->netif_index_first != 0 &&
		old->netif_index_second != 0)
	{
		if (new->netif_index_first == 0 ||
			new->netif_index_second == 0)
		{
			ret = nam_vlan_eline_entry_del(old);
		}
	}

    return ret;
}

long npd_vlan_qinqcfg_handle_update(void *newItem, void *oldItem )
{
	npd_vlan_qinq_t * new = (npd_vlan_qinq_t *)newItem;
	npd_vlan_qinq_t * old= (npd_vlan_qinq_t *)oldItem;
	int ret, all_ret = 0;
	
	if (new->inner_tpid != old->inner_tpid)
	{
		ret = nam_vlan_inner_tpid_set(new->inner_tpid);
		if (ret != 0)
			all_ret = ret;
	}

	return all_ret;
}

long npd_vlan_qinqcfg_handle_insert(void *newItem)
{
	npd_vlan_qinq_t * new = (npd_vlan_qinq_t *)newItem;
	int ret, all_ret = 0;


	if (new->inner_tpid != 0x8100)
	{
		ret = nam_vlan_inner_tpid_set(new->inner_tpid);
		if (ret != 0)
			all_ret = ret;
	}

	return all_ret;
	
}
#endif


#ifdef HAVE_PRIVATE_VLAN

int pvlan_primary_cmp_vid(void * in1, void * in2)
{
	pvlan_primary_t *data1 = (pvlan_primary_t *)in1;
	pvlan_primary_t *data2 = (pvlan_primary_t *)in2;		
    return (data1->vid == data2->vid);
}

unsigned int pvlan_primary_vid_key(void * in)
{
	pvlan_primary_t *data1 = (pvlan_primary_t *)in;
    return (data1->vid);
}

unsigned int pvlan_primary_sequence_index(unsigned int vid)
{
	unsigned int key = 0; /*for hash key calculate*/
	
	key = (vid)& 0xFFF ;
	
	key %= (NPD_PVLAN_PRIMARY_SIZE);
	
	return key;
}


int pvlan_primarydb_handle_ntoh(void *data)
{
    pvlan_primary_t *item = (pvlan_primary_t *)data;
	
	item->vid = ntohl(item->vid);
	NPD_VBMP_VLAN_NTOH(item->secondary_vlans);
	NPD_PBMP_PORT_NTOH(item->ports);

	return 0;
}

int pvlan_primarydb_handle_hton(void *data)
{
    pvlan_primary_t *item = (pvlan_primary_t *)data;
	
	item->vid = htonl(item->vid);
	NPD_VBMP_VLAN_HTON(item->secondary_vlans);
	NPD_PBMP_PORT_HTON(item->ports);

	return 0;
}

long pvlan_primarydb_handle_update(void *new_data, void *old_data)
{
	pvlan_primary_t* old_item = (pvlan_primary_t *)old_data;
	pvlan_primary_t* new_item = (pvlan_primary_t *)new_data;
    int all_ret = 0;

	npd_pbmp_t port_bmp;
	unsigned int promis_port;
	unsigned int netif_index;


	#if 0 
    NPD_VBMP_ASSIGN(vlan_bmp, new_item->secondary_vlans);
    NPD_VBMP_XOR(vlan_bmp, old_item->secondary_vlans);
    NPD_VBMP_ITER(vlan_bmp, second_vid)
    {
		if (NPD_VBMP_MEMBER(new_item->secondary_vlans, second_vid))
		{
			//nam_pvlan_associate(new_item->vid, second_vid);
		}
		else
		{
			nam_pvlan_no_associate(new_item->vid, second_vid);
		}
	}
	#endif
	
	NPD_PBMP_ASSIGN(port_bmp, new_item->ports);
	NPD_PBMP_XOR(port_bmp, old_item->ports);	
	NPD_PBMP_ITER(port_bmp, promis_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(promis_port);
		if (NPD_PBMP_MEMBER(new_item->ports, promis_port))
		{
			nam_pvlan_promis_port_set(netif_index, new_item->vid, NPD_TRUE);
		}
		else
		{
			nam_pvlan_promis_port_set(netif_index, new_item->vid, NPD_FALSE);
		}		
	}
	

	return all_ret;
}


long pvlan_primarydb_handle_insert(void *data)
{
    pvlan_primary_t *item = (pvlan_primary_t *)data;
    int ret= 0;
	unsigned int promis_port;
	unsigned int netif_index;

	NPD_PBMP_ITER(item->ports, promis_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(promis_port);
		nam_pvlan_promis_port_set(netif_index, item->vid, NPD_TRUE);
	}
	

    npd_syslog_dbg("Assoc private return code %d\n", ret);

    return ret;
}

long pvlan_primarydb_handle_delete(void *data)
{
    pvlan_primary_t *item = (pvlan_primary_t *)data;
    int ret = 0;

	unsigned int promis_port = 0;
	unsigned int netif_index = 0;


	NPD_PBMP_ITER(item->ports, promis_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(promis_port);
		nam_pvlan_promis_port_set(netif_index, item->vid, NPD_FALSE);
	}
	

    return ret;
}


/* isolate vlan control */
int pvlan_isolate_cmp_vid(void * in1, void * in2)
{
	pvlan_isolate_t *data1 = (pvlan_isolate_t *)in1;
	pvlan_isolate_t *data2 = (pvlan_isolate_t *)in2;		
    return (data1->vid == data2->vid);
}

unsigned int pvlan_isolate_vid_key(void * in)
{
	pvlan_isolate_t *data1 = (pvlan_isolate_t *)in;
    return (data1->vid);
}

unsigned int pvlan_isolate_sequence_index(unsigned int vid)
{
	unsigned int key = 0; /*for hash key calculate*/
	
	key = (vid)& 0xFFF ;
	
	key %= (NPD_PVLAN_ISOLATE_SIZE);
	
	return key;
}


int pvlan_isolatedb_handle_ntoh(void *data)
{
    pvlan_isolate_t *item = (pvlan_isolate_t *)data;
	
	item->vid= ntohl(item->vid);
	item->primary_vid= ntohl(item->primary_vid);	
	NPD_PBMP_PORT_NTOH(item->ports);	

	return 0;
}

int pvlan_isolatedb_handle_hton(void *data)
{
    pvlan_isolate_t *item = (pvlan_isolate_t *)data;

	item->vid= htonl(item->vid);
	item->primary_vid= htonl(item->primary_vid);	
	NPD_PBMP_PORT_HTON(item->ports);	

	return 0;
}

long pvlan_isolatedb_handle_update(void *new_data, void *old_data)
{
	pvlan_isolate_t* old_item = (pvlan_isolate_t *)old_data;
	pvlan_isolate_t* new_item = (pvlan_isolate_t *)new_data;
    int all_ret = 0;
	npd_pbmp_t port_bmp;
	unsigned int isolate_port;
	unsigned int netif_index;

	if (new_item->primary_vid != old_item->primary_vid)
	{
		if (new_item->primary_vid)
		{
			nam_pvlan_associate(new_item->primary_vid, new_item->vid);	
		}
		else
		{
			nam_pvlan_no_associate(old_item->primary_vid, old_item->vid);	
		}
		
	}


	NPD_PBMP_ASSIGN(port_bmp, new_item->ports);
	NPD_PBMP_XOR(port_bmp, old_item->ports);	
	NPD_PBMP_ITER(port_bmp, isolate_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(isolate_port);
		if (NPD_PBMP_MEMBER(new_item->ports, isolate_port))
		{
			nam_pvlan_isolate_port_set(netif_index, new_item->vid, NPD_TRUE);
		}
		else
		{
			nam_pvlan_isolate_port_set(netif_index, new_item->vid, NPD_FALSE);
		}		
	}
	
	return all_ret;
}


long pvlan_isolatedb_handle_insert(void *data)
{
    pvlan_isolate_t *item = (pvlan_isolate_t *)data;
    int ret = 0;
	unsigned int isolate_port = 0;
	unsigned int netif_index = 0;

	if (item->primary_vid)
	{
		nam_pvlan_associate(item->primary_vid, item->vid);
	}


	NPD_PBMP_ITER(item->ports, isolate_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(isolate_port);
		nam_pvlan_isolate_port_set(netif_index, item->vid, NPD_TRUE);
	}


    return ret;
}

long pvlan_isolatedb_handle_delete(void *data)
{
    pvlan_isolate_t *item = (pvlan_isolate_t *)data;
    int ret = 0;
	unsigned int isolate_port = 0;
	unsigned int netif_index = 0;

	if (item->primary_vid)
	{
		nam_pvlan_no_associate(item->primary_vid, item->vid);
	}


	NPD_PBMP_ITER(item->ports, isolate_port)
	{
		netif_index = (unsigned int )netif_array_index_to_ifindex(isolate_port);
		nam_pvlan_isolate_port_set(netif_index, item->vid, NPD_FALSE);
	}

    return ret;
}

#endif

int vlandb_handle_ntoh(void * data)
{
	struct vlan_s *vlan = (struct vlan_s *)data;

	vlan->vid = ntohl(vlan->vid);
	vlan->g_ifindex = ntohl(vlan->g_ifindex);
	vlan->fdb_limit = ntohl(vlan->fdb_limit);
	vlan->mtu = ntohl(vlan->mtu);
	vlan->fdb_learning_mode = ntohl(vlan->fdb_learning_mode);
	vlan->link_status = ntohl(vlan->link_status);
	vlan->isAutoCreated = ntohl(vlan->isAutoCreated);
	vlan->pvlan_type = ntohl(vlan->pvlan_type);
	vlan->IsEnable = ntohl(vlan->IsEnable);
	vlan->groupId = ntohl(vlan->groupId);
	vlan->EgrGroupId = ntohl(vlan->EgrGroupId);
	vlan->isStatic = ntohl(vlan->isStatic);
	vlan->forward_mode = ntohl(vlan->forward_mode);
	NPD_PBMP_PORT_NTOH(vlan->untag_ports);
	NPD_PBMP_PORT_NTOH(vlan->tag_ports);
/*	NPD_PBMP_PORT_NTOH(vlan->dhcpMbrEnDis); */

	return 0;
}

int vlandb_handle_hton(void * data)
{
	struct vlan_s *vlan = (struct vlan_s *)data;

	vlan->vid = htonl(vlan->vid);
	vlan->g_ifindex = htonl(vlan->g_ifindex);
	vlan->fdb_limit = htonl(vlan->fdb_limit);
	vlan->mtu = htonl(vlan->mtu);
	vlan->fdb_learning_mode = htonl(vlan->fdb_learning_mode);
	vlan->link_status = htonl(vlan->link_status);
	vlan->isAutoCreated = htonl(vlan->isAutoCreated);
	vlan->pvlan_type = htonl(vlan->pvlan_type);
	vlan->IsEnable = htonl(vlan->IsEnable);
	vlan->groupId = htonl(vlan->groupId);
	vlan->EgrGroupId = htonl(vlan->EgrGroupId);
	vlan->isStatic = htonl(vlan->isStatic);
	vlan->forward_mode = htonl(vlan->forward_mode);	
	NPD_PBMP_PORT_HTON(vlan->untag_ports);
	NPD_PBMP_PORT_HTON(vlan->tag_ports);
/*	NPD_PBMP_PORT_HTON(vlan->dhcpMbrEnDis); */

	return 0;
}

int macvlandb_handle_ntoh(void *data)
{	
    macbase_vlan_t *item = (macbase_vlan_t *)data;
	item->vid = ntohl(item->vid);
	item->tbl_index = ntohl(item->tbl_index);

	return 0;
}

int macvlandb_handle_hton(void *data)
{	
	macbase_vlan_t *item = (macbase_vlan_t *)data;
	item->vid = htonl(item->vid);
	item->tbl_index = htonl(item->tbl_index);

	return 0;
}


int subnetvlandb_handle_ntoh(void *data)
{
    subnetbase_vlan_t *item = (subnetbase_vlan_t *)data;
	item->vid = ntohl(item->vid);
	/*ip and mask need to be kept in network endian*/
	/*
    item->ipaddr = ntohl(item->ipaddr);
    item->mask = ntohl(item->mask);
    */
	item->tbl_index = ntohl(item->tbl_index);	

	return 0;
}

int subnetvlandb_handle_hton(void *data)
{
	subnetbase_vlan_t *item = (subnetbase_vlan_t *)data;
	item->vid = htonl(item->vid);
	item->ipaddr = htonl(item->ipaddr);
	item->mask = htonl(item->mask);
	item->tbl_index = htonl(item->tbl_index);	

	return 0;
}


int protovlandb_handle_ntoh(void *data)
{
    proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
	proto_vlan->proto_group_index = ntohl(proto_vlan->proto_group_index);
	proto_vlan->vid = ntohs(proto_vlan->vid);
	proto_vlan->netif_index = ntohl(proto_vlan->netif_index);

	return 0;
}

int protovlandb_handle_hton(void *data)
{
	proto_vlan_port_t *proto_vlan = (proto_vlan_port_t*)data;
	proto_vlan->proto_group_index = htonl(proto_vlan->proto_group_index);
	proto_vlan->vid = htons(proto_vlan->vid);
	proto_vlan->netif_index = htonl(proto_vlan->netif_index);

	return 0;
}


int protobasedb_handle_ntoh(void *data)
{
	protobase_vlan_t* proto_base = (protobase_vlan_t*)data;
	proto_base->proto_group_index = ntohl(proto_base->proto_group_index);
	proto_base->ether_frame = ntohl(proto_base->ether_frame);
	proto_base->eth_type = ntohs(proto_base->eth_type);
	
	return 0;
}

int protobasedb_handle_hton(void *data)
{
	protobase_vlan_t* proto_base = (protobase_vlan_t*)data;
	proto_base->proto_group_index = htonl(proto_base->proto_group_index);
	proto_base->ether_frame = htonl(proto_base->ether_frame);
	proto_base->eth_type = htons(proto_base->eth_type);
	
	return 0;
}

#ifdef HAVE_QINQ
int vlan_xlate_db_handle_ntoh(void *data)
{
	vlan_xlate_db_entry_t *vlanXlate = (vlan_xlate_db_entry_t *)data;

	vlanXlate->netif_index = ntohl(vlanXlate->netif_index);
	vlanXlate->xlate_type = ntohl(vlanXlate->xlate_type);
	vlanXlate->priority = ntohl(vlanXlate->priority);
	vlanXlate->ingress_outer_start_vid = ntohl(vlanXlate->ingress_outer_start_vid);
	vlanXlate->ingress_outer_vid_num = ntohl(vlanXlate->ingress_outer_vid_num);
	vlanXlate->ingress_inner_start_vid = ntohl(vlanXlate->ingress_inner_start_vid);
	vlanXlate->ingress_inner_vid_num = ntohl(vlanXlate->ingress_inner_vid_num);
	vlanXlate->egress_outer_vid = ntohl(vlanXlate->egress_outer_vid);
	vlanXlate->egress_inner_vid = ntohl(vlanXlate->egress_inner_vid);

	return 0;
}

int vlan_xlate_db_handle_hton(void *data)
{
	vlan_xlate_db_entry_t *vlanXlate = (vlan_xlate_db_entry_t *)data;

	vlanXlate->netif_index = htonl(vlanXlate->netif_index);
	vlanXlate->xlate_type = htonl(vlanXlate->xlate_type);
	vlanXlate->priority = htonl(vlanXlate->priority);
	vlanXlate->ingress_outer_start_vid = htonl(vlanXlate->ingress_outer_start_vid);
	vlanXlate->ingress_outer_vid_num = htonl(vlanXlate->ingress_outer_vid_num);
	vlanXlate->ingress_inner_start_vid = htonl(vlanXlate->ingress_inner_start_vid);
	vlanXlate->ingress_inner_vid_num = htonl(vlanXlate->ingress_inner_vid_num);
	vlanXlate->egress_outer_vid = htonl(vlanXlate->egress_outer_vid);
	vlanXlate->egress_inner_vid = htonl(vlanXlate->egress_inner_vid);

	return 0;
}


int vlan_eline_db_handle_ntoh(void *data)
{	
    vlan_eline_db_entry_t *vlanEline = (vlan_eline_db_entry_t*)data;
	vlanEline->eline_id = ntohl(vlanEline->eline_id);
	vlanEline->eline_type = ntohl(vlanEline->eline_type);
	vlanEline->inner_vid = ntohl(vlanEline->inner_vid);
	vlanEline->outer_vid = ntohl(vlanEline->outer_vid);
	vlanEline->netif_index_first = ntohl(vlanEline->netif_index_first);
	vlanEline->netif_index_second = ntohl(vlanEline->netif_index_second);

	return 0;	
}

int vlan_eline_db_handle_hton(void *data)
{	
	vlan_eline_db_entry_t *vlanEline = (vlan_eline_db_entry_t*)data;
	vlanEline->eline_id = htonl(vlanEline->eline_id);
	vlanEline->eline_type = htonl(vlanEline->eline_type);
	vlanEline->inner_vid = htonl(vlanEline->inner_vid);
	vlanEline->outer_vid = htonl(vlanEline->outer_vid);
	vlanEline->netif_index_first = htonl(vlanEline->netif_index_first);
	vlanEline->netif_index_second = htonl(vlanEline->netif_index_second);

	return 0;	
}


int npd_vlan_qinqcfg_handle_ntoh(void *data)
{	
	npd_vlan_qinq_t * vlanQinq = (npd_vlan_qinq_t *)data;
	vlanQinq->tpid = ntohs(vlanQinq->tpid);	
	vlanQinq->inner_tpid = ntohs(vlanQinq->inner_tpid);

	return 0;
}

int npd_vlan_qinqcfg_handle_hton(void *data)
{	
	npd_vlan_qinq_t * vlanQinq = (npd_vlan_qinq_t *)data;
	vlanQinq->tpid = htons(vlanQinq->tpid); 
	vlanQinq->inner_tpid = htons(vlanQinq->inner_tpid);

	return 0;
}
#endif

void npd_init_vlan(void)
{
    char name[16];
#ifdef HAVE_QINQ    
    struct npd_vlan_qinq_s npd_vlan_qinq_default = {0};
#endif

	nam_vlan_init();
	
    sprintf(name, "VLAN_DB");

    /*port based vlan db*/
    create_dbtable(name, CHASSIS_VLAN_RANGE_MAX, sizeof(struct vlan_s),
        &vlandb_handle_update, NULL, &vlandb_handle_insert, &vlandb_handle_delete,
        NULL, NULL, NULL, vlandb_handle_ntoh, vlandb_handle_hton, DB_SYNC_ALL, &vlan_db);
    if(NULL == vlan_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    dbtable_create_sequence_index("vlan_id", vlan_db, CHASSIS_VLAN_RANGE_MAX, 
        &vlandb_index, &vlandb_key, &vlandb_cmp, &g_vlans);
    if(NULL == g_vlans)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    /*mac based vlan db*/
    sprintf(name, "MACVLAN_DB");
    create_dbtable(name, NPD_MACBASE_VLAN_SIZE, sizeof(macbase_vlan_t),
        NULL, NULL, &macvlandb_handle_insert, &macvlandb_handle_delete, NULL,
        NULL, NULL, macvlandb_handle_ntoh, macvlandb_handle_hton, DB_SYNC_ALL, &macvlan_db);
    if(NULL == macvlan_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_hash_index("mac", macvlan_db, NPD_MACBASE_VLAN_SIZE, &macvlan_hash_mac,
        &macvlan_cmp_mac, &mac_vlan_hash);
    if(NULL == mac_vlan_hash)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_sequence_index("vlan_id", macvlan_db, NPD_MACBASE_VLAN_SIZE, &macvlan_sequence_index,
        &macvlan_vid_key, &macvlan_cmp_vid, &mac_vlan_vid_sequence);
    if(NULL == mac_vlan_vid_sequence)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    /*subnet based vlan db*/
    sprintf(name, "SUBNETVLAN_DB");
    create_dbtable(name, NPD_SUBNETBASE_VLAN_SIZE, sizeof(subnetbase_vlan_t),
        NULL, NULL, &subnetvlandb_handle_insert, &subnetvlandb_handle_delete,
        NULL, NULL, NULL, subnetvlandb_handle_ntoh, subnetvlandb_handle_hton, DB_SYNC_ALL, &subnetvlan_db);
    if(NULL == subnetvlan_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_hash_index("subnetvlanip", subnetvlan_db, NPD_SUBNETBASE_VLAN_SIZE, &subnetvlan_hash_ip,
        &subnetvlan_cmp_ip, &subnet_vlan_hash);
    if(NULL == subnet_vlan_hash)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_sequence_index("subnetvlanvid", subnetvlan_db, NPD_SUBNETBASE_VLAN_SIZE, 
        &subnetvlan_sequence_index, &subnetvlan_vid_key, &subnetvlan_cmp_vid, &subnet_vlan_vid_sequence);
    if(NULL == subnet_vlan_vid_sequence)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    
         

    /*protocol based vlan db*/
    sprintf(name, "PROTOVLAN_DB");
    create_dbtable(name, PROTO_VLAN_PORT_TABLE_SIZE, sizeof(proto_vlan_port_t),
        NULL, NULL, &protovlandb_handle_insert, &protovlandb_handle_delete,
        NULL, NULL, NULL, protovlandb_handle_ntoh, protovlandb_handle_hton, DB_SYNC_ALL, &proto_vlanport_db);
    if(NULL == proto_vlanport_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_hash_index("netif&protocol", proto_vlanport_db, PROTO_VLAN_PORT_TABLE_SIZE, 
        &protovlandb_hash, &protovlandb_cmp, &proto_vlanport_hash);
    if(NULL == proto_vlanport_hash)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
	/* 
    dbtable_create_sequence_index("protocol", proto_vlanport_db, 
        16, protovlandb_index, protovlandb_key, protovlandb_seq_cmp, 
        &proto_vlanport_seq);
    */
    
    sprintf(name, "PROTOVLAN_BASE");
    create_dbtable(name, PROTO_VLAN_TABLE_SIZE, sizeof(protobase_vlan_t),
        NULL, NULL, protobasedb_handle_insert, protobasedb_handle_delete,
        NULL, NULL, NULL, protobasedb_handle_ntoh, protobasedb_handle_hton, DB_SYNC_ALL, &proto_vlan_db);
    if(NULL == proto_vlan_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_array_index("protocol", proto_vlan_db,  &proto_vlan_array);
    if(NULL == proto_vlan_array)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
#ifdef HAVE_QINQ
    sprintf(name, "VLAN_XLATE_DB");
    create_dbtable(name, VLAN_XLATE_TABLE_SIZE, sizeof(vlan_xlate_db_entry_t),
        NULL, NULL, &vlan_xlate_db_handle_insert, &vlan_xlate_db_handle_delete,
        NULL, NULL, NULL, vlan_xlate_db_handle_ntoh, vlan_xlate_db_handle_hton, DB_SYNC_ALL, &vlan_xlate_table_db);
    if(NULL == vlan_xlate_table_db)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_hash_index("vlanxlate", vlan_xlate_table_db, VLAN_XLATE_TABLE_SIZE, &vlan_xlate_db_hash,
        &vlan_xlate_db_cmp, &vlan_xlate_table_hash);
    if(NULL == vlan_xlate_table_hash)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
    dbtable_create_hash_index("xlatenetif", vlan_xlate_table_db, VLAN_XLATE_NETIF_HASH_KEY_MAX,
        &vlan_xlate_db_netif_hash, &vlan_xlate_db_cmp, &vlan_xlate_table_netif_hash);
    if(NULL == vlan_xlate_table_netif_hash)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    sprintf(name, "VLAN_ELINE_DB");
    create_dbtable(name, VLAN_ELINE_TABLE_SIZE, sizeof(vlan_eline_db_entry_t),
        &vlan_eline_db_handle_update, NULL, NULL, NULL,
        NULL, NULL, NULL, vlan_eline_db_handle_ntoh, vlan_eline_db_handle_hton, DB_SYNC_ALL, &vlan_eline_table_db);
    if(NULL == vlan_eline_table_db)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
	
    dbtable_create_array_index("eline", vlan_eline_table_db,  &vlan_eline_array);
    if(NULL == vlan_eline_array)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
	
    dbtable_create_hash_index("elinevlan", vlan_eline_table_db, VLAN_ELINE_VLAN_HASH_KEY_MAX,
        &vlan_eline_db_vlan_hash, &vlan_eline_vlan_cmp, &vlan_eline_vlan_hash);
    if(NULL == vlan_eline_vlan_hash)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

	/* qinq configure */
	ret = create_dbtable( "QinqConfTbl", 1, sizeof(struct npd_vlan_qinq_s),\
					npd_vlan_qinqcfg_handle_update, 
					NULL,
					npd_vlan_qinqcfg_handle_insert, 
					NULL,
					NULL, 
					NULL,
					NULL, npd_vlan_qinqcfg_handle_ntoh, npd_vlan_qinqcfg_handle_hton, 
					DB_SYNC_ALL,
					&npd_vlan_table_qinqcfg);
    if(NULL == npd_vlan_table_qinqcfg)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

	ret = dbtable_create_array_index("qinq_cfg", npd_vlan_table_qinqcfg, &npd_vlan_qinq_index);
    if(NULL == npd_vlan_table_qinqcfg)
    {
        syslog_ax_vlan_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
#endif	
#ifdef HAVE_PRIVATE_VLAN

    sprintf(name, "PVLAN_PRIMARY_DB");
    create_dbtable(name, NPD_PVLAN_PRIMARY_SIZE, sizeof(pvlan_primary_t),
        pvlan_primarydb_handle_update, NULL, NULL, pvlan_primarydb_handle_insert,
        pvlan_primarydb_handle_delete, NULL, NULL, pvlan_primarydb_handle_ntoh, 
        pvlan_primarydb_handle_hton, DB_SYNC_ALL, &pvlan_primary_db);
    if(NULL == pvlan_primary_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    dbtable_create_sequence_index("pvlan_primary_seqindex", pvlan_primary_db, NPD_PVLAN_PRIMARY_SIZE, 
        &pvlan_primary_sequence_index, &pvlan_primary_vid_key, 
        &pvlan_primary_cmp_vid, &pvlan_primary_vid_sequence);
    if(NULL == pvlan_primary_vid_sequence)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    sprintf(name, "PVLAN_ISOLATE_DB");
    create_dbtable(name, NPD_PVLAN_ISOLATE_SIZE, sizeof(pvlan_isolate_t),
        &pvlan_isolatedb_handle_update, NULL, &pvlan_isolatedb_handle_insert, 
        &pvlan_isolatedb_handle_delete,NULL, NULL, NULL, pvlan_isolatedb_handle_ntoh, 
        pvlan_isolatedb_handle_hton, DB_SYNC_ALL, &pvlan_isolate_db);
    if(NULL == pvlan_isolate_db)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }

    dbtable_create_sequence_index("pvlan_isolate_seqindex", pvlan_isolate_db, NPD_PVLAN_ISOLATE_SIZE, 
        &pvlan_isolate_sequence_index, &pvlan_isolate_vid_key, 
        &pvlan_isolate_cmp_vid, &pvlan_isolate_vid_sequence);
    if(NULL == pvlan_isolate_vid_sequence)
    {
        npd_syslog_err("ERROR: Can not get resource to initialize vlan database\n");
        exit(1);
    }
	
#endif


    register_netif_notifier(&vlan_netif_notifier);

    create_default_vlan();
    create_port_l3intf_vlan();
#ifdef HAVE_QINQ
	npd_vlan_qinq_default.tpid = g_tpid;
	npd_vlan_qinq_default.inner_tpid = g_inner_tpid;
	ret = dbtable_array_insert(npd_vlan_qinq_index, &vlan_qinq_global_no, &npd_vlan_qinq_default);
	if(ret != 0)
	{
		syslog_ax_fdb_err("Insert fdb default configuration failed.\n");
        exit(1);
	}
#endif	
}

/* check a vlan interface node has exists in the list or not*/
/* we use vlanId as a index for searching for the node*/
/**********************************************************************************
 *  npd_check_vlan_real_exist
 *
 *	DESCRIPTION:
 * 		Check out if specified vlan has been created or not
 * 		VLAN ID used as an index to find vlan structure.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		TRUE	- if vlan has been created before 
 *		FALSE	- if vlan doesn't exist,or hasn't been created before
 *
 **********************************************************************************/
unsigned int npd_check_vlan_real_exist
(
	unsigned short vlanId
)
{
    struct vlan_s * vlan = NULL;
    int ret;

    vlan = npd_find_vlan_by_vid(vlanId);

    if(NULL != vlan)
        ret = TRUE;
    else
        ret = FALSE;

	if(vlan)
	    free(vlan);

    return ret;
}

/* check a vlan interface node has exists in the list or not*/
/* we use vlanId as a index for searching for the node*/
/**********************************************************************************
 *  npd_check_vlan_exist
 *
 *	DESCRIPTION:
 * 		Check out if specified vlan has been created or not
 * 		VLAN ID used as an index to find vlan structure.
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		VLAN_RETURN_CODE_VLAN_EXISTS	- if vlan has been created before 
 *		VLAN_RETURN_CODE_VLAN_NOT_EXISTS	- if vlan doesn't exist,or hasn't been created before or is created automaically
 *
 **********************************************************************************/
unsigned int npd_check_vlan_exist
(
	unsigned short vlanId
)
{
    struct vlan_s * vlan = NULL;
    int ret;

    vlan = npd_find_vlan_by_vid(vlanId);

    if(NULL != vlan)
    {
        ret = TRUE;
    }        
    else
	{
        ret = FALSE;
    }
	
	if(vlan)
	    free(vlan);

    return ret;
}
unsigned int npd_check_user_vlan_exist
(
	unsigned short vlanId
)
{
    struct vlan_s * vlan = NULL;
    int ret;

    vlan = npd_find_vlan_by_vid(vlanId);

    if(NULL != vlan)
    {
        if(FALSE == vlan->isAutoCreated)
            ret = TRUE;
        else
            ret = FALSE;
    }        
    else
	{
        ret = FALSE;
    }
	
	if(vlan)
	    free(vlan);

    return ret;
}

unsigned short npd_get_valid_vlan_id()
{
	unsigned short i = 0;
    int ret;
    struct vlan_s *vlan = malloc(sizeof(*vlan));

    if(NULL == vlan)
        return 0;
	for(i = 1; i < NPD_VLAN_NUMBER_MAX; i++)
	{
        vlan->vid = i;
        ret = dbtable_sequence_search(g_vlans, i, vlan);
	    if(0 != ret)
	    {
            free(vlan);
			return i;
	    }
        
	}
    free(vlan);
	return 0;
}

/**********************************************************************************
 *  npd_check_vlan_status
 *
 *	DESCRIPTION:
 * 		Check out if specified vlan is UP or DOWN
 *
 *	INPUT:
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		VLAN_STATE_UP_E  - if vlan is up
 *		VLAN_STATE_DOWN_E	- if vlan is down
 *		-NPD_VLAN_NOTEXISTS	- if vlan doesn't exist,or hasn't been created before
 *
 **********************************************************************************/
int npd_check_vlan_status
(
	unsigned short vlanId
)
{
	struct vlan_s 	*vlanNode = NULL;
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

    vlanNode = npd_find_vlan_by_vid(vlanId);
	if( NULL == vlanNode ) {
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	}
	else{
		if(VLAN_STATE_UP_E == vlanNode->link_status) {
			ret = VLAN_STATE_UP_E;
		}
		else {
			ret = VLAN_STATE_DOWN_E;
		}
	    free(vlanNode);
	}

	return ret;
}

int npd_netif_vlan_member(
    unsigned int netif_index,
    unsigned int vid,
    int isTaged
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret;

    if(vid > 4095)
    {
		ret = FALSE;
        goto error;
    }
	
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = FALSE;
        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
        goto error;
    }
    
   if(isTaged)
    {
        if(NPD_VBMP_MEMBER(switch_port->allow_tag_vlans, vid))
        {
            ret = TRUE;
        }
        else
            ret = FALSE;
    }
    else
    {
        if(NPD_VBMP_MEMBER(switch_port->allow_untag_vlans, vid))
        {
            ret = TRUE;
        }
        else 
            ret = FALSE;
    }

error:
    if(switch_port)
        free(switch_port);
    return ret;
}


int npd_netif_vlan_access_mode(
    unsigned int netif_index,
    int mode
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret;
    int count;

    if(NULL == switch_port)
        return COMMON_RETURN_CODE_NO_RESOURCE;
    switch_port->global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
        goto error;
    }

    if(SWITCH_PORT_MODE_ACCESS == mode)
    {
        NPD_VBMP_COUNT(switch_port->allow_tag_vlans,count);
        if(count > 0)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto error;
        }
            
        NPD_VBMP_COUNT(switch_port->allow_untag_vlans, count);
        if(count > 1)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto error;
        }
    }
    if(SWITCH_PORT_MODE_TRUNK == mode)
    {           
        NPD_VBMP_COUNT(switch_port->allow_untag_vlans, count);
        if(count > 1)
        {
            ret = VLAN_RETURN_CODE_PORT_TRUNK_MODE_CONFLICT;
            goto error;
        }
    }

	
    switch_port->vlan_access_mode = mode;
    ret = dbtable_hash_update(switch_ports_hash, NULL, switch_port);
    
error:
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);
    return ret;
}

int npd_netif_free_vlan(
    unsigned int netif_index,
    unsigned int vid,
    int isTagged
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    struct vlan_s *vlan = malloc(sizeof(struct vlan_s));
    int ret;
	int orig_learn = 0;

    if((!switch_port) || (!vlan))
    {
        if(switch_port)
            free(switch_port);
        if(vlan)
            free(vlan);
        return COMMON_RETURN_CODE_NO_RESOURCE;
    }
	
    npd_key_database_lock();
	npd_fdb_learning_mode_get(netif_index, &orig_learn);
	if(npd_startup_end)
	{
	    npd_fdb_learning_mode_set(netif_index, 0);
	}

    vlan->vid = vid;
    ret = dbtable_sequence_search(g_vlans, vid, vlan);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto error;
    }
    
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
        goto error;
    }
	if (vlan->pvlan_type && (switch_port->vlan_private_mode != SWITCH_PORT_PRIVLAN_PROMI))
	{
		if (switch_port->vlan_private_mode == SWITCH_PORT_PRIVLAN_COMMUNITY && 
			(1 >= npd_vlan_is_port_in_privlan(switch_port->global_port_ifindex)))
		{
			switch_port->vlan_private_mode = SWITCH_PORT_PRIVLAN_PROMI;
		}
		else if (switch_port->vlan_private_mode == SWITCH_PORT_PRIVLAN_ISOLATED)
		{
			switch_port->vlan_private_mode = SWITCH_PORT_PRIVLAN_PROMI;
		}
		ret = dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
	}
	
    if(isTagged)
    {
        if(!NPD_PBMP_MEMBER(vlan->tag_ports, switch_port->switch_port_index))
        {
            ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
            goto error;
        }
        NPD_VBMP_VLAN_REMOVE(switch_port->allow_tag_vlans, vid);
        NPD_PBMP_PORT_REMOVE(vlan->tag_ports, switch_port->switch_port_index);
        dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
        dbtable_sequence_update(g_vlans, vid, NULL, vlan);
        netif_notify_relate_event(npd_netif_vlan_index(vid), 
            switch_port->global_port_ifindex, PORT_NOTIFIER_LEAVE);
        netif_app_notify_relate_event(npd_netif_vlan_index(vid),
            switch_port->global_port_ifindex, PORT_NOTIFIER_LEAVE, NULL, 0);
    }
    else
    {
        if(!NPD_PBMP_MEMBER(vlan->untag_ports, switch_port->switch_port_index))
        {
            ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
            goto error;
        }
        NPD_VBMP_VLAN_REMOVE(switch_port->allow_untag_vlans, vid);
        NPD_PBMP_PORT_REMOVE(vlan->untag_ports, switch_port->switch_port_index);
        dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
        dbtable_sequence_update(g_vlans, vid, NULL, vlan);
        netif_notify_relate_event(npd_netif_vlan_index(vid), 
            switch_port->global_port_ifindex, PORT_NOTIFIER_LEAVE);
        netif_app_notify_relate_event(npd_netif_vlan_index(vid),
            switch_port->global_port_ifindex, PORT_NOTIFIER_LEAVE, NULL, 0);
		
		ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
		if(0 != ret)
	    {
	        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
	        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
	        goto error;
	    }
        if(switch_port->pvid == vid)
        {
            if(vid != DEFAULT_VLAN_ID)
            {
        		if(NPD_PBMP_MEMBER(switch_port->allow_tag_vlans, DEFAULT_VLAN_ID))
        		{/* if the default vlan has this port with tagged, remove it defaultly*/
					NPD_VBMP_VLAN_REMOVE(switch_port->allow_tag_vlans, DEFAULT_VLAN_ID);
				}
                vlan->vid = DEFAULT_VLAN_ID;
                dbtable_sequence_search(g_vlans, DEFAULT_VLAN_ID, vlan);
                switch_port->pvid = DEFAULT_VLAN_ID;
                NPD_VBMP_VLAN_ADD(switch_port->allow_untag_vlans, DEFAULT_VLAN_ID);
                NPD_PBMP_PORT_ADD(vlan->untag_ports, switch_port->switch_port_index);
                dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
                dbtable_sequence_update(g_vlans, DEFAULT_VLAN_ID, NULL, vlan);
                netif_notify_relate_event(npd_netif_vlan_index(DEFAULT_VLAN_ID), 
                    switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN);
                netif_app_notify_relate_event(npd_netif_vlan_index(DEFAULT_VLAN_ID),
                    switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN, NULL, 0);
            }  
            else
            {
                switch_port->pvid = 0;
                dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
            }
        }
    }

    ret = VLAN_RETURN_CODE_ERR_NONE;

error:
	if(npd_startup_end)
	{
	    npd_fdb_learning_mode_set(netif_index, orig_learn);
	}
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);
    if(vlan)
        free(vlan);
    return ret;       
}
    
int npd_netif_allow_vlan(
    unsigned int netif_index,
    unsigned int vid,
    int isTaged,
    int pvid_set
    )
{
	unsigned char remove_form_default_vlan_flag = 0;
	int orig_learn = 0;
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    struct vlan_s *vlan = malloc(sizeof(struct vlan_s));
    int ret;

    if((!switch_port) || (!vlan))
    {
        if(switch_port)
            free(switch_port);
        if(vlan)
            free(vlan);
        return COMMON_RETURN_CODE_NO_RESOURCE;
    }

    npd_key_database_lock();
	npd_fdb_learning_mode_get(netif_index, &orig_learn);
	if(npd_startup_end)
	{
	    npd_fdb_learning_mode_set(netif_index, 0);
	}
	
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        npd_syslog_dbg("% Can not get switch port entity for port index %d", netif_index);
        goto error;
    }

    vlan->vid = vid;
    ret = dbtable_sequence_search(g_vlans, vid, vlan);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto error;
    }
    syslog_ax_vlan_dbg("Switch port related NETIF is %d\n", switch_port->switch_port_index);

	if (vlan->forward_mode == FORWARD_SINGLE_XC)
	{
		npd_pbmp_t pbmp;
		int count;
		
		NPD_PBMP_ASSIGN(pbmp, vlan->untag_ports);
		NPD_PBMP_OR(pbmp, vlan->tag_ports);
		NPD_PBMP_COUNT(pbmp, count);
		if (count >= 2)
		{
			ret = VLAN_RETURN_CODE_PORT_ELINE_FULL;
			goto error;
		}
	}
			
    if(isTaged)
    {
        if(NPD_PBMP_MEMBER(vlan->untag_ports, switch_port->switch_port_index))
        {
            syslog_ax_vlan_dbg("Switch port has been added this vlan with untagged mode\n");
            ret = VLAN_RETURN_CODE_PORT_TAG_CONFLICT;
            goto error;
        }
        else if(NPD_PBMP_MEMBER(vlan->tag_ports, switch_port->switch_port_index))
        {
            syslog_ax_vlan_dbg("Switch port has been added this vlan with tagged mode\n");
            ret = VLAN_RETURN_CODE_PORT_EXISTS;
            goto error;
        }
        if(SWITCH_PORT_MODE_ACCESS == switch_port->vlan_access_mode)
        {
            ret = VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT;
            goto error;
        }
        NPD_VBMP_VLAN_ADD(switch_port->allow_tag_vlans, vid);
        NPD_PBMP_PORT_ADD(vlan->tag_ports, switch_port->switch_port_index);
        dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
        dbtable_sequence_update(g_vlans, vid, NULL, vlan);
        netif_notify_relate_event(npd_netif_vlan_index(vid), 
            switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN);
        netif_app_notify_relate_event(npd_netif_vlan_index(vid),
            switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN, NULL, 0);
    }
    else
    {
        if(NPD_PBMP_MEMBER(vlan->tag_ports, switch_port->switch_port_index))
        {
            syslog_ax_vlan_dbg("Switch port has been added this vlan with tagged mode\n");
            ret = VLAN_RETURN_CODE_PORT_TAG_CONFLICT;
            goto error;
        }
        else if(NPD_PBMP_MEMBER(vlan->untag_ports, switch_port->switch_port_index))
        {
            syslog_ax_vlan_dbg("Switch port has been added this vlan with untagged mode\n");
            ret = VLAN_RETURN_CODE_PORT_EXISTS;
            goto error;
        }
        if(switch_port->pvid == 0)
        {
            if(vid == DEFAULT_VLAN_ID)
            {
                switch_port->pvid = vid;
            }
            else
            {
                ret = VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT;
                goto error;
            }
        }
        else if((switch_port->pvid == DEFAULT_VLAN_ID)
                  && pvid_set)
        {
        	remove_form_default_vlan_flag = 1;
            switch_port->pvid = vid;
        }
        else if(switch_port->vlan_access_mode != SWITCH_PORT_MODE_HYBRID)
        {
            npd_syslog_dbg("NETIF pvid is %d\n", switch_port->pvid);
            ret = VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT;
            goto error;
        }
        
        NPD_VBMP_VLAN_ADD(switch_port->allow_untag_vlans, vid);
        NPD_PBMP_PORT_ADD(vlan->untag_ports, switch_port->switch_port_index);
        dbtable_array_update(switch_ports, switch_port->switch_port_index, NULL, switch_port);
        dbtable_sequence_update(g_vlans, vid, NULL, vlan);
		if (remove_form_default_vlan_flag)
		{
             npd_netif_free_vlan(netif_index, DEFAULT_VLAN_ID, FALSE);
		}
        netif_notify_relate_event(npd_netif_vlan_index(vid), 
            switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN);
        netif_app_notify_relate_event(npd_netif_vlan_index(vid),
            switch_port->global_port_ifindex, PORT_NOTIFIER_JOIN, NULL, 0);
    }

    ret = VLAN_RETURN_CODE_ERR_NONE;

error:
	if(npd_startup_end)
	{
	    npd_fdb_learning_mode_set(netif_index, orig_learn);
	}
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);
    if(vlan)
        free(vlan);

    return ret;
}

int npd_netif_free_alluntag_vlan(
    unsigned int netif_index
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret;
    unsigned int vid;

    if(!switch_port)
        return COMMON_RETURN_CODE_NO_RESOURCE;
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    

    NPD_VBMP_ITER(switch_port->allow_untag_vlans, vid)
    {
        ret = npd_netif_free_vlan(netif_index, vid, FALSE);
        if(0 != ret)
        {
            goto error;
        }
    }


    ret = VLAN_RETURN_CODE_ERR_NONE;

error:
    if(switch_port)
        free(switch_port);
    return ret;
    
}
  
int npd_netif_free_alltag_vlan(
    unsigned int netif_index
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret;
    unsigned int vid;

    if(!switch_port)
        return COMMON_RETURN_CODE_NO_RESOURCE;
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    

    NPD_VBMP_ITER(switch_port->allow_tag_vlans, vid)
    {
        ret = npd_netif_free_vlan(netif_index, vid, TRUE);
        if(0 != ret)
        {
            goto error;
        }
    }


    ret = VLAN_RETURN_CODE_ERR_NONE;

error:
    if(switch_port)
        free(switch_port);
    return ret;
    
}
  
int npd_netif_free_allvlan(
    unsigned int netif_index
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret;
    unsigned int vid;

    if(!switch_port)
        return COMMON_RETURN_CODE_NO_RESOURCE;
	syslog_ax_vlan_dbg("Will free all vlan for netif 0x%x.\n",netif_index);
    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    
    NPD_VBMP_ITER(switch_port->allow_tag_vlans, vid)
    {
        ret = npd_netif_free_vlan(netif_index, vid, TRUE);
        if(0 != ret)
        {
	        syslog_ax_vlan_dbg("Free tagged vlan %d for netif 0x%x failed.\n", vid, netif_index);
            goto error;
        }
		else
		{
	        syslog_ax_vlan_dbg("Free tagged vlan %d for netif 0x%x OK.\n", vid, netif_index);
		}
    }

    NPD_VBMP_ITER(switch_port->allow_untag_vlans, vid)
    {
        ret = npd_netif_free_vlan(netif_index, vid, FALSE);
        if(0 != ret)
        {
	        syslog_ax_vlan_dbg("Free untagged vlan %d for netif 0x%x failed.\n", vid, netif_index);
            goto error;
        }
		else
		{
	        syslog_ax_vlan_dbg("Free untagged vlan %d for netif 0x%x OK.\n", vid, netif_index);
		}
    }


    ret = VLAN_RETURN_CODE_ERR_NONE;

error:
    if(switch_port)
        free(switch_port);
    return ret;
    
}

unsigned int npd_vlan_port_pvid_get(
    unsigned int netif_index,
    unsigned short *pvid
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret = VLAN_RETURN_CODE_ERR_NONE;

    if(NULL == switch_port)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }

    switch_port->global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL,  (void*)switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    *pvid = switch_port->pvid;

error:
    if(switch_port)
        free(switch_port);

    return ret;

}

unsigned int npd_vlan_netif_pvid_set(
    unsigned int netif_index,
    unsigned short pvid
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret = VLAN_RETURN_CODE_ERR_NONE;

    if(NULL == switch_port)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }

    switch_port->global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL,  (void*)switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    switch_port->pvid = pvid;
    ret = dbtable_array_update(switch_ports, switch_port->switch_port_index,
        NULL, switch_port);
error:
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);

    return ret;

}

unsigned int npd_vlan_netif_inner_pvid_set(
    unsigned int netif_index,
    unsigned short pvid
    )
{
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    int ret = VLAN_RETURN_CODE_ERR_NONE;

    if(NULL == switch_port)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }

    switch_port->global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL,  (void*)switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto error;
    }
    switch_port->inner_pvid = pvid;
    ret = dbtable_array_update(switch_ports, switch_port->switch_port_index,
        NULL, switch_port);
error:
	npd_key_database_unlock();
    if(switch_port)
        free(switch_port);

    return ret;

}

/**********************************************************************************
 *  npd_vlan_check_port_membership
 *
 *	DESCRIPTION:
 * 		check out if a port is an untagged or tagged port member of specified vlan
 *
 *	INPUT:
 *		vlanId - vlan id
 *		eth_g_index - global ethernet port index
 *		isTagged - untagged or tagged port member
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - if port is a member of vlan
 *		NPD_FALSE - if port is not a member of vlan
 *		
 *
 **********************************************************************************/
unsigned int npd_vlan_check_port_membership
(
	unsigned short vlanId,
	unsigned int   eth_g_index,
	unsigned char  isTagged
)
{
    int ret;
    ret = npd_netif_vlan_member(eth_g_index, vlanId, isTagged);
    return ret;
}


unsigned int npd_vlan_if_has_member(unsigned short	vlanId)
{
    struct vlan_s *vlan = malloc(sizeof(struct vlan_s));
    int ret;

    if(!vlan)
        return NPD_FALSE;
    vlan->vid = vlanId;    
    ret = dbtable_sequence_search(g_vlans, vlanId, vlan);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        if(vlan)
            free(vlan);
		return NPD_FALSE;
    }
	if(NPD_PBMP_IS_NULL(vlan->tag_ports) && NPD_PBMP_IS_NULL(vlan->untag_ports))
	{
        if(vlan)
            free(vlan);
        return NPD_FALSE;
	}
    if(vlan)
        free(vlan);
    return NPD_TRUE;
}


/**********************************************************************************
 *  npd_vlan_check_contain_port
 *
 *	DESCRIPTION:
 * 		check out if a port is an untagged or tagged port member of specified vlan
 *
 *	INPUT:
 *		vlanId - vlan id
 *		eth_g_index - global ethernet port index
 *	
 *	OUTPUT:
 *		isTagged - ethernet port is untagged or tagged port member of vlan
 *
 * 	RETURN:
 *		NPD_TRUE  - if port is a member of vlan
 *		NPD_FALSE - if port is not a member of vlan
 *		
 *
 **********************************************************************************/
unsigned int npd_vlan_check_contain_port
(
	unsigned short vlanId,
	unsigned int   eth_g_index,
	unsigned char  *isTagged
)
{
	if(NPD_TRUE == npd_netif_vlan_member(eth_g_index,vlanId,NPD_FALSE)) {
		*isTagged = NPD_FALSE;
		return NPD_TRUE;
	}
	else if(NPD_TRUE == npd_netif_vlan_member(eth_g_index,vlanId,NPD_TRUE)) {
		*isTagged = NPD_TRUE;
		return NPD_TRUE;
	}
	else
		return NPD_FALSE;
}


/**********************************************************************************
 *  npd_vlan_check_trunk_membership
 *
 *	DESCRIPTION:
 * 		check out if a trunk is an untagged or tagged trunk member of specified vlan
 *
 *	INPUT:
 *		trunkId - trunk id
 *		vlanId -- vlan id
 *		isTagged - untagged or tagged port member
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - if trunk is a member of vlan
 *		NPD_FALSE - if trunk is not a member of vlan
 *		
 *
 **********************************************************************************/
unsigned int npd_vlan_check_trunk_membership
(
	unsigned short vlanId,
	unsigned short trunkId,
	unsigned char  isTagged
)
{
    int ret;

    ret = npd_netif_vlan_member(npd_netif_trunk_get_index(trunkId), vlanId, isTagged);
    return ret;
}


/**********************************************************************************
 *  npd_vlan_check_contain_trunk
 *
 *	DESCRIPTION:
 * 		check out if a trunk is an untagged or tagged member of specified vlan
 *
 *	INPUT:
 *		trunkId - trunk id
 *		vlanId - vlan id
 *	
 *	OUTPUT:
 *		isTagged - trunk is untagged or tagged port member of vlan
 *
 * 	RETURN:
 *		NPD_TRUE  - if trunk is a member of vlan
 *		NPD_FALSE - if trunk is not a member of vlan
 *		
 *
 **********************************************************************************/
unsigned int npd_vlan_check_contain_trunk
(
	unsigned short vlanId,
	unsigned short trunkId,
	unsigned char  *isTagged
)
{
    unsigned int trunk_index = npd_netif_trunk_index(trunkId);
	if(NPD_TRUE == npd_netif_vlan_member(trunk_index,vlanId,NPD_FALSE)) {
		*isTagged = NPD_FALSE;
		return NPD_TRUE;
	}
	else if(NPD_TRUE == npd_netif_vlan_member(trunk_index,vlanId,NPD_TRUE)) {
		*isTagged = NPD_TRUE;
		return NPD_TRUE;
	}
	else
		return NPD_FALSE;
}

unsigned int npd_vlan_check_contain_netif
(
	unsigned short vlanId,
	unsigned int netif_index,
	unsigned char  *isTagged
)
{
	if(NPD_TRUE == npd_netif_vlan_member(netif_index,vlanId,NPD_FALSE)) {
		*isTagged = NPD_FALSE;
		return NPD_TRUE;
	}
	else if(NPD_TRUE == npd_netif_vlan_member(netif_index,vlanId,NPD_TRUE)) {
		*isTagged = NPD_TRUE;
		return NPD_TRUE;
	}
	else
		return NPD_FALSE;
}


/**********************************************************************************
 *
 * create vlan by VLAN ID on both SW and HW side
 *
 *
 *	INPUT:
 *		vlanId - vlan id
 *		name - vlan alias name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_VLAN_ERR_GENERAL - if error occurred when create vlan in SW side 
 *		NPD_VLAN_ERR_HW - if error occurred when create vlan in HW side
 *		
 *
 **********************************************************************************/
unsigned int npd_vlan_activate
(
	unsigned short vlanId,
	char *name
)
{
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

	struct vlan_s *vlanNode;
	ret = npd_check_vlan_real_exist(vlanId);
	/*vlan exsits*/
	if(ret == TRUE) {
    	npd_key_database_lock();
		vlanNode = npd_find_vlan_by_vid(vlanId);
		if (NULL == vlanNode) {
			 syslog_ax_vlan_err("npd_vlan_find error.\n");
			ret = VLAN_RETURN_CODE_ERR_GENERAL; 
		}
		else{
			ret = VLAN_RETURN_CODE_VLAN_EXISTS;
			if(vlanNode->isAutoCreated){
				vlanNode->isAutoCreated = FALSE;				
				memcpy(vlanNode->name,name,NPD_VLAN_IFNAME_SIZE);
				ret = VLAN_RETURN_CODE_ERR_NONE;
			}
            if((vlanId == vlanNode->vid) && (strcmp(name,vlanNode->name)==0))
            {
                ret = VLAN_RETURN_CODE_ERR_NONE;
            }
		}
        
		if(vlanNode)
	        npd_put_vlan(vlanNode);
		npd_key_database_unlock();
		return ret;   /*NPD return HERE!!! steps into next CMD node*/
	}
	/*vlan NOT exists*/
	else {
		vlanNode = npd_find_vlan_by_name(name);
		if(NULL != vlanNode) {
			ret = VLAN_RETURN_CODE_NAME_CONFLICT;
            free(vlanNode);
			return ret;
		}
		
		vlanNode = npd_create_vlan_by_name(name, vlanId);
		if (NULL == vlanNode) { 
			 syslog_ax_vlan_err("vlan create error.\n");
			return VLAN_RETURN_CODE_ERR_GENERAL;  
		}
        free(vlanNode);
	}
	return ret;
}

int vlan_mtu_set(unsigned int vid, int mtu)
{
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

	struct vlan_s *vlanNode;
    npd_key_database_lock();
    vlanNode = npd_find_vlan_by_vid(vid);
    if(NULL == vlanNode)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto error;
    }

    vlanNode->mtu = mtu;

error:
	npd_key_database_unlock();
    if(vlanNode)
        npd_put_vlan(vlanNode);
    return ret;
}

unsigned int npd_fdb_number_vlan_set
(
    unsigned short vlanId,
    int number
)
{
	unsigned int ret = 0;
	struct vlan_s *vlanNode = NULL;

	ret = npd_check_vlan_exist(vlanId);
	if(NPD_TRUE!= ret){/*NPD_VLAN_EXIST == 13*/
	   npd_syslog_err("fdb:The vlan %d input does not exist \n",vlanId);
	   return FDB_RETURN_CODE_GENERAL;
	}
    npd_key_database_lock();
	vlanNode = npd_find_vlan_by_vid(vlanId);
	if(vlanNode != NULL) {
		if(vlanNode->fdb_limit != number)
		{
    	    vlanNode->fdb_limit = number;		
    		npd_put_vlan(vlanNode);
        	if(number >= 0)
        	{
        	    npd_fdb_dynamic_entry_del_by_vlan(vlanId);
        	}
		}
	}
	npd_key_database_unlock();
	return VLAN_RETURN_CODE_ERR_NONE;
}


unsigned int npd_fdb_number_vlan_set_check 
(
    unsigned short vlanId,
    unsigned int *number
)

{
	unsigned int ret = 0;
    struct vlan_s *vlanNode = NULL;
	
	ret = npd_check_vlan_exist(vlanId);
	if(NPD_TRUE != ret){/*NPD_VLAN_EXISTS == 13*/
	   return FALSE;
	}


	vlanNode = npd_find_vlan_by_vid(vlanId);

	if( vlanNode != NULL ) 
		*number = vlanNode->fdb_limit;
	
	if(*number >= 0){
	   ret =  TRUE;
	}
    else{
	   ret = FALSE;
	}

	if( vlanNode )
		free(vlanNode);

	return ret;
}


unsigned int npd_vlan_autolearn_set
(
    unsigned short vlanId,
    unsigned int  autolearn

)
{
    unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

    struct vlan_s *vlanNode;

	npd_key_database_lock();
    vlanNode = npd_find_vlan_by_vid(vlanId);
    if(NULL == vlanNode)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto error;
    }

    vlanNode->fdb_learning_mode = autolearn;

error:
	npd_key_database_unlock();
    if(vlanNode)
        npd_put_vlan(vlanNode);
    
	return ret;
} 

int npd_vlan_assoc_mac_set(unsigned char isAdd, unsigned char *mac, unsigned short cur_vid)
{
	int ret;
	macbase_vlan_t item;
	
	item.vid = cur_vid;
	memcpy(item.mac, mac, MAC_ADDR_LEN);
    ret = dbtable_hash_search(mac_vlan_hash, &item, NULL, &item);
    if(isAdd)
    {
        if(0 == ret)
        {
            npd_syslog_dbg("VLAN %d has assocated with the MAC", item.vid,
                item.mac[0], item.mac[1], item.mac[2], item.mac[3], item.mac[4], item.mac[5]);
			if (cur_vid == item.vid)
			{
				ret = VLAN_RETURN_CODE_MACBASE_EXIST;
			}
			else
			{
				ret = VLAN_RETURN_CODE_MACBASE_OTHER_EXIST;
			}	
        }
        else
        {
			ret = nam_vlan_mac_tbl_index_alloc(&(item.tbl_index));
			if (ret == 0)
				ret = dbtable_hash_insert(mac_vlan_hash, &item);
    	}
    }
    else
    {
        npd_syslog_dbg("VLAN delete with MAC %.2x-%.2x-%.2x-%.2x-%.2x-%.2x",
                item.mac[0], item.mac[1], item.mac[2], item.mac[3], item.mac[4], item.mac[5]);
        if(0 != ret || cur_vid != item.vid)
        {
            ret = COMMON_SUCCESS;
        }
        else
        {
			ret = dbtable_hash_delete(mac_vlan_hash, &item, &item);
			if (ret == 0)
				ret = nam_vlan_mac_tbl_index_free(item.tbl_index);
		}
            
    }

	return ret;
}

long vlandb_handle_insert(void *data)
{
    struct vlan_s *vlan = (struct vlan_s *)data;
    int ret, all_ret = 0;

    npd_syslog_dbg("%s %d: Create vlan (%d) at NAM layer.\r\n", 
				__func__, __LINE__, vlan->vid);
    ret = nam_asic_vlan_entry_active(SYS_PRODUCT_TYPE, vlan->vid);

	if (0 != ret)
	{
		all_ret = -1;
		npd_syslog_dbg("%s %d: Create vlan (%d) failed\r\n", 
				__func__, __LINE__, vlan->vid);
	}
	
    if(vlan->fdb_limit >= 0)
    {
        ret = nam_fdb_limit4vlan((unsigned short)vlan->vid, vlan->fdb_limit);
        if(ret)
		{
			all_ret = -1;
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "nam_fdb_limit4vlan");
		}
	
    }
	
    {
        ret = nam_fdb_na_msg_vlan_set(vlan->vid, vlan->fdb_learning_mode);
		if(ret)
		{
			all_ret = -1;
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
		}
		
    }
#if 0
	{
		ret = nam_set_igmp_enable(vlan->vid, vlan->igmpSnpState);
        if(ret)
		{
			all_ret = -1;
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
		}
	}
#endif
	{
		ret = nam_vlan_cross_connect_set(vlan->vid, vlan->forward_mode);
		if(ret)
		{
			all_ret = -1;
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
		}
	}

	{
		ret = nam_vlan_private_set(vlan->vid, vlan->pvlan_type);
		if(ret)
		{
			all_ret = -1;
			npd_syslog_dbg("ret is not 0: %s(%d): %s", 
				__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
		}
	}	

    return all_ret;
}

long vlandb_handle_delete(void *data)
{
    struct vlan_s *vlan = (struct vlan_s *)data;
    int ret;

	npd_syslog_dbg("vlan delete: delete vid %d", vlan->vid);
	
    ret = nam_asic_vlan_entry_delete(vlan->vid);
    return ret;
}

long vlandb_handle_update(void *newdata, void *olddata)
{
    struct vlan_s *vlan_new = (struct vlan_s*)newdata;
    struct vlan_s *vlan_old = (struct vlan_s*)olddata;
    int ret = 0, all_ret = 0;

/*hardware operation finish in switchport*/
#if 0
    if(!NPD_PBMP_EQ(vlan_new->tag_ports, vlan_old->tag_ports))
    {
        int switch_port_index;
        npd_pbmp_t pbmp = NPD_PBMP_XOR(vlan_new->tag_ports, vlan_old->tag_ports);
        NPD_PBMP_ITER(pbmp, switch_port_index)
        {
            
        }
    }

    if(!NPD_PBMP_EQ(vlan_new->untag_ports, vlan_old->untag_ports))
    {
        int switch_port_index;
        npd_pbmp_t pbmp = NPD_PBMP_XOR(vlan_new->tag_ports, vlan_old->tag_ports);
        NPD_PBMP_ITER(pbmp, switch_port_index)
        {
        }
    }
#endif
    if(vlan_new->fdb_limit != vlan_old->fdb_limit)
    {
        ret = nam_fdb_limit4vlan((unsigned short)vlan_new->vid, vlan_new->fdb_limit);
        if(ret)
            all_ret = -1;

		if (-1 == all_ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n", 
				__FILE__, __LINE__, "nam_fdb_limit4vlan");
		}
	
    }

    if(vlan_new->fdb_learning_mode != vlan_old->fdb_learning_mode)
    {
        ret = nam_fdb_na_msg_vlan_set(vlan_new->vid, vlan_new->fdb_learning_mode);
        if(ret)
            all_ret = -1;

		if (-1 == all_ret)
		{
			npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n", 
				__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
		}
		
    }
    
#if 0
	if( vlan_new->igmpSnpState != vlan_old->igmpSnpState ) 
	{
		ret = nam_set_igmp_enable(vlan_new->vid, vlan_new->igmpSnpState);
        if(0 != ret)
            all_ret = -1;
	}
	if( vlan_new->dhcpSnpEnDis != vlan_old->dhcpSnpEnDis )
	{
		ret = nam_set_UdpBcPkt_enable(vlan_new->vid, vlan_new->dhcpSnpEnDis);
        if(0 != ret)
            all_ret = -1;
	}

	if( !NPD_PBMP_EQ(vlan_new->igmpMbrEnDis, vlan_old->igmpMbrEnDis))
	{		
		npd_pbmp_t pbmp;
		NPD_PBMP_ASSIGN(pbmp, vlan_new->igmpMbrEnDis);
		NPD_PBMP_XOR(pbmp, vlan_old->igmpMbrEnDis);

		NPD_PBMP_ITER(pbmp, switch_port_index)
		{	
			netif_index = eth_port_array_index_to_ifindex( switch_port_index );
			
			if( NPD_PBMP_MEMBER(vlan_new->igmpMbrEnDis, switch_port_index) ) {				
				if(0 != nam_asic_igmp_trap_set_by_devport(vlan_new->vid, netif_index,TRUE)){
					all_ret = -1;
					
					if (-1 == all_ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
					}
					
				}
			}
			else {				
				if(0 != nam_asic_igmp_trap_set_by_devport(vlan_new->vid, netif_index,FALSE)){
					all_ret = -1;

					if (-1 == all_ret)
					{
						npd_syslog_dbg("ret is not 0: %s(%d): %s", 
							__FILE__, __LINE__, "nam_fdb_na_msg_vlan_set");
					}
					
				}
			}
			
		}

	}
#endif
	if (vlan_new->forward_mode != vlan_old->forward_mode)
	{
		ret = nam_vlan_cross_connect_set(
			vlan_new->vid, vlan_new->forward_mode);
		if (ret != 0)
			all_ret = -1;
	}

	if (vlan_new->pvlan_type != vlan_old->pvlan_type)
	{
		ret = nam_vlan_private_set(
			vlan_new->vid, vlan_new->pvlan_type);
		if (ret != 0)
			all_ret = -1;
	}
	
	return all_ret;    
}

void npd_save_vlan_cfg(char* buf,int bufLen)
{
	char *showStr = buf,*cursor = NULL;
	int i = 0;
	struct vlan_s* node = NULL;
    char tmpBuf[256] = {0};
    int length = 0;
	int start_vid = 0;
	int end_vid = 0;
	int vid;
	int vid_str_len = 0;
	int vid_allow_count = 0;
	npd_vbmp_t bmp;
	
	cursor = showStr;
    *cursor = 0;
    NPD_VBMP_CLEAR(bmp);
    for(i = 2; i < NPD_MAX_VLAN_ID; i++) {
		node = npd_find_vlan_by_vid(i);
		if(NULL != node)
		{
			NPD_VBMP_VLAN_ADD(bmp, i);
            free(node);
		}
    }


    NPD_VBMP_ITER(bmp, vid)
    {
        if(start_vid == 0)
        {
            start_vid = vid;
            end_vid = vid;
            vid_allow_count++;
        }
        else if((vid == end_vid+1)
                 && (vid_allow_count <= 199))
        {
            end_vid = vid;
            vid_allow_count++;
        }
        else
        {
            if(0 == vid_str_len)
                sprintf(tmpBuf, "vlan ");
            if(start_vid == end_vid)
            {
                sprintf(tmpBuf, "%s%d", tmpBuf, start_vid);
            }
            else
            {
                sprintf(tmpBuf, "%s%d-%d", tmpBuf, start_vid, end_vid);
            }
            vid_str_len = strlen(tmpBuf);
            if((vid_str_len < 50) && (vid_allow_count <= 199))
                sprintf(tmpBuf, "%s,", tmpBuf);
            else
            {
                sprintf(tmpBuf, "%s\n", tmpBuf);
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
                vid_str_len = 0;
                vid_allow_count = 0;
            }
            start_vid = vid;
            end_vid = vid;
            vid_allow_count++;
        }
    }

    if(start_vid != 0)
    {
		int need_exit = 0;
        if(0 == vid_str_len)
             sprintf(tmpBuf, "vlan ");
        if(start_vid == end_vid)
        {
            sprintf(tmpBuf, "%s%d\n", tmpBuf, start_vid);
			if(0 == vid_str_len)
			{
			    need_exit = 1;
			}
        }
        else
        {
            sprintf(tmpBuf, "%s%d-%d\n", tmpBuf, start_vid, end_vid);
        }
        length += strlen(tmpBuf);
        if(length < bufLen)
            strcat(showStr, tmpBuf);
        vid_str_len = 0;
        vid_allow_count = 0;
		if(need_exit == 1)
		{
            sprintf(tmpBuf, " exit\n");
            length += strlen(tmpBuf);
            if(length < bufLen)
                strcat(showStr, tmpBuf);
		}
    }
	
	for(i = 2; i < NPD_MAX_VLAN_ID; i++) {
	
		node = npd_find_vlan_by_vid(i);
		if(NULL != node) {
            char default_name[50] = {0};
			int enter = 0;

			sprintf(default_name, "VLAN%.4d", i);
			if( 0 != strcmp(default_name, node->name))
			{
				enter = 1;
                sprintf(tmpBuf, "vlan %d\n", i);
                length += strlen(tmpBuf);
                if(length < bufLen)
                {
                    strcat(showStr, tmpBuf);
                }
                sprintf(tmpBuf, " name %s\n", node->name);
                length += strlen(tmpBuf);
                if(length < bufLen)
                {
                    strcat(showStr, tmpBuf);
                }
			}
#ifdef HAVE_PRIVATE_VLAN
			if (node->pvlan_type)
			{
				if (enter == 0)
				{
					enter = 1;
                    sprintf(tmpBuf, "vlan %d\n", i);
                    length += strlen(tmpBuf);
                    if(length < bufLen)
                    {
                        strcat(showStr, tmpBuf);
                    }
				}
				if (node->pvlan_type == PVLAN_VLAN_TYPE_PRIMARY)
				{
	                sprintf(tmpBuf, " pvlan primary\n");
	                length += strlen(tmpBuf);
	                if(length < bufLen)
	                    strcat(showStr, tmpBuf);					
				}
				else if (node->pvlan_type == PVLAN_VLAN_TYPE_ISOLATE)
				{
					unsigned int assoc_prim_vid = 0;
					assoc_prim_vid = npd_vlan_get_assoc_primary_vid(node->vid);
	                sprintf(tmpBuf, " pvlan mapping primary-vlan %d\n", assoc_prim_vid);
	                length += strlen(tmpBuf);
	                if(length < bufLen)
	                    strcat(showStr, tmpBuf);					

				}
			}
#endif
			#if 0
			
            if(node->pvlan_type)
            {
				if(enter == 0)
				{
					enter = 1;
                    sprintf(tmpBuf, "vlan %d\n", i);
                    length += strlen(tmpBuf);
                    if(length < bufLen)
                    {
                        strcat(showStr, tmpBuf);
                    }
				}
                sprintf(tmpBuf, "private-vlan\n");
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
            }
			#endif

            if(node->fdb_limit >= 0)
            {
				if(enter == 0)
				{
					enter = 1;
                    sprintf(tmpBuf, "vlan %d\n", i);
                    length += strlen(tmpBuf);
                    if(length < bufLen)
                    {
                        strcat(showStr, tmpBuf);
                    }
				}
                sprintf(tmpBuf, "mac-address-table limit %d\n", node->fdb_limit);
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
            }
            {
                int ret;
			    macbase_vlan_t item;
			    macbase_vlan_t item_out;
			    memset(&item, 0, sizeof(macbase_vlan_t));
			    memset(&item_out, 0, sizeof(macbase_vlan_t));	
				
                item.vid = node->vid;
				ret = dbtable_hash_head(mac_vlan_hash,  
					&item, &item_out, &vlan_assoc_mac_show_filter);	
				if(0 == ret)
				{
    				if(enter == 0)
    				{
    					enter = 1;
                        sprintf(tmpBuf, "vlan %d\n", i);
                        length += strlen(tmpBuf);
                        if(length < bufLen)
                        {
                            strcat(showStr, tmpBuf);
                        }
    				}
				}
                while(0 == ret)
                {
                    sprintf(tmpBuf, "associate mac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 
                                    item_out.mac[0], item_out.mac[1], item_out.mac[2],
                                    item_out.mac[3], item_out.mac[4], item_out.mac[5]);
                    length += strlen(tmpBuf);
                    if(length < bufLen)
                        strcat(showStr, tmpBuf);
					memcpy(item.mac, item_out.mac, sizeof(item_out.mac));
                    ret = dbtable_hash_next(mac_vlan_hash,  
						&item, &item_out, &vlan_assoc_mac_show_filter);
                }
            }
            {
                int ret;
				subnetbase_vlan_t item;
    			subnetbase_vlan_t item_out;
			    memset(&item, 0, sizeof(subnetbase_vlan_t));
			    memset(&item_out, 0, sizeof(subnetbase_vlan_t));
				
			    item.ipaddr = 0;
			    item.mask = 0;

				item.vid = node->vid;
                ret = dbtable_hash_head(subnet_vlan_hash, 
							&item, &item_out, &vlan_assoc_subnet_show_filter);
				if(0 == ret)
				{
    				if(enter == 0)
    				{
    					enter = 1;
                        sprintf(tmpBuf, "vlan %d\n", i);
                        length += strlen(tmpBuf);
                        if(length < bufLen)
                        {
                            strcat(showStr, tmpBuf);
                        }
    				}
				}
                while(0 == ret)
                {
                    char ip[32];
					int makelen;
                    lib_get_string_from_ip(ip, item_out.ipaddr);
					lib_get_masklen_from_mask(item_out.mask, &makelen);
                    sprintf(tmpBuf, "associate subnet %s/%d\n", ip, makelen);
                    length += strlen(tmpBuf);
                    if(length < bufLen)
                        strcat(showStr, tmpBuf);

					item.ipaddr = item_out.ipaddr;
					item.mask = item_out.mask;
        			ret = dbtable_hash_next(subnet_vlan_hash,  
							&item, &item_out, &vlan_assoc_subnet_show_filter);					
                }
            }
#ifdef HAVE_QINQ			
			/* eline */
			if (node->forward_mode != FORWARD_BRIDGING)
			{
				int ret;
				vlan_eline_db_entry_t item;
    			vlan_eline_db_entry_t item_out;
			    memset(&item, 0, sizeof(vlan_eline_db_entry_t));
			    memset(&item_out, 0, sizeof(vlan_eline_db_entry_t));
				
			    item.eline_id= 0;
				item.outer_vid = node->vid;
                ret = dbtable_hash_head(vlan_eline_vlan_hash, 
							&item, &item_out, &npd_vlan_eline_outervlan_filter);
				if(0 == ret)
				{
    				if(enter == 0)
    				{
    					enter = 1;
                        sprintf(tmpBuf, "vlan %d\n", i);
                        length += strlen(tmpBuf);
                        if(length < bufLen)
                        {
                            strcat(showStr, tmpBuf);
                        }
    				}
				}
                while(0 == ret)
                {
					switch(item_out.eline_type)
					{
						case VLAN_ELINE_TYPE_SINGLE:
							length += sprintf(tmpBuf, "e-line %d\n", item_out.eline_id);
							break;
						case VLAN_ELINE_TYPE_DOUBLE:
							length += sprintf(tmpBuf, "e-line %d inner vlan %d\n", 
								item_out.eline_id, item_out.inner_vid);
							break;
					}					
                    if(length < bufLen)
                        strcat(showStr, tmpBuf);
					memcpy(&item, &item_out, sizeof(vlan_eline_db_entry_t));
        			ret = dbtable_hash_next(vlan_eline_vlan_hash,  
							&item, &item_out, &npd_vlan_eline_outervlan_filter);					
                }
			}
#endif			
			if(enter == 1)
			{
                sprintf(tmpBuf, "exit\n");
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
			}
            free(node);
		}
	}
   	sprintf(tmpBuf, "\n");
    length += strlen(tmpBuf);
    if(length < bufLen)
        strcat(showStr, tmpBuf);

    {
        protobase_vlan_t proto_vlan;
        int ret;
        for(i = 0; i <= NPD_MAX_PROTO_VLAN_ID; i++)
        {
            char frame[15];
            char ether_type[15];
            ret = dbtable_array_get(proto_vlan_array, i, &proto_vlan);
            if(0 == ret)
            {
                switch(proto_vlan.ether_frame)
                {
                    case VLAN_PROTOCOL_FRAME_ETHER2:
                        sprintf(frame, "ethernetii");
                        break;
                    case VLAN_PROTOCOL_FRAME_LLC:
                        sprintf(frame, "llc");
                        break;
                    case VLAN_PROTOCOL_FRAME_SNAP:
                        sprintf(frame, "snap");
                        break;
                     default:
                        sprintf(frame," ");
                        break;
                }
                switch(proto_vlan.eth_type)
                {
                    case 0x800:
                        sprintf(ether_type, "ipv4");
                        break;
                    case 0x86dd:
                        sprintf(ether_type, "ipv6");
                        break;
                    case 0x8137:
                        sprintf(ether_type, "ipx");
                        break;
                    case 0x806:
                        sprintf(ether_type, "arp");
                        break;
                    default:
                        sprintf(ether_type, "%d", proto_vlan.eth_type);
                        break;
                }
                sprintf(tmpBuf, "protocol-vlan %d %s %s\n", i, frame, ether_type);
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
                
            }
        }
	}
#ifdef HAVE_QINQ	
	/* vlan qinq set */
	{
		if (g_tpid != 0x8100)
		{
			length += sprintf(tmpBuf, "qinq tpid 0x%x\n", g_tpid);
            if(length < bufLen)
                strcat(showStr, tmpBuf);						
		}
		
		if (g_inner_tpid != 0x8100)
		{
			length += sprintf(tmpBuf, "qinq inner-tpid 0x%x\n", g_inner_tpid);
            if(length < bufLen)
                strcat(showStr, tmpBuf);			
		}
	}
#endif
}


/*********************************************************************************************
 *			NPD dbus operation part
 *	
 *********************************************************************************************/
DBusMessage * npd_dbus_check_vlan_exist
(
	DBusConnection *conn, 
 	DBusMessage *msg, 
 	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter iter;
	unsigned int op_ret = 0;
	unsigned short vlan_id = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT16, &vlan_id,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return NULL;
	}

	op_ret = npd_check_vlan_exist(vlan_id);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);
	return reply;
		
}


DBusMessage * npd_dbus_vlan_create_vlan_entry_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter = {0};
	unsigned short	vlanId = 1;
	char *vlanName = NULL;
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err = {0};


	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_STRING,&vlanName,
							 DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (!CHASSIS_VLANID_ISLEGAL(vlanId)) {
		ret = ETHPORT_RETURN_CODE_NO_SUCH_VLAN;
	}
	else {
		/* call npd_vlan_interface_active, to do somthing as BELOW:*/
		ret = npd_vlan_activate(vlanId,vlanName);
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	return reply;
	
}
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vlanId;
	unsigned int	ret = 0,op_ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (!CHASSIS_VLANID_ISLEGAL(vlanId)) {
		ret = ETHPORT_RETURN_CODE_NO_SUCH_VLAN;
	}
	else {
        ret = npd_check_user_vlan_exist(vlanId);
		if (FALSE == ret) {
			op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS; 
		}
		
		else{  
			op_ret = VLAN_RETURN_CODE_ERR_NONE;      
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	
	return reply;
	
}
/*enter vlan configure node*/
DBusMessage * npd_dbus_vlan_config_layer2_vname
(	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vlanId = 0;
	char*	vlanName = NULL;
	struct vlan_s*	vlanNode = NULL;
	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING,&vlanName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_vlan_dbg("To set or create a vlan named %s",vlanName);
	vlanNode = npd_find_vlan_by_name(vlanName);
	if(NULL == vlanNode)
	{
		vlanId = npd_get_valid_vlan_id();
		if(vlanId == 0)
		{
    		op_ret = COMMON_RETURN_CODE_NO_RESOURCE;  
		}
		else
		{
    		op_ret = npd_vlan_activate(vlanId,vlanName);
    		if (VLAN_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			syslog_ax_vlan_err("npd_dbus101:: add vlan vid %d\n",vlanId);
    		}
    		else
    		{
    			op_ret = VLAN_RETURN_CODE_ERR_NONE;   
    		}
		}
	}
	else {
		vlanId = vlanNode->vid;
		if(1 == vlanId) { 
			op_ret = VLAN_RETURN_CODE_BADPARAM;
		}/*default vlan can NOT config*/
		else if (4095 == vlanId) {
			op_ret = VLAN_RETURN_CODE_PORT_L3_INTF;
		}
		else if(vlanNode->isAutoCreated){
			op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		}
		else 
			op_ret = VLAN_RETURN_CODE_ERR_NONE;
        free(vlanNode);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16,
									 &vlanId);
	
	return reply;
	
}

DBusMessage * npd_dbus_vlan_update_name
(	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vlanId = 0;
	char*	vlanName = NULL;
	struct vlan_s*	vlanNode = NULL;
	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_STRING,&vlanName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_vlan_dbg("To change the name of vlan %d to %s", vlanId, vlanName);
	npd_key_database_lock();
	vlanNode = npd_find_vlan_by_name(vlanName);
	if(NULL == vlanNode)
	{
		vlanNode = npd_find_vlan_by_vid(vlanId);
		if(vlanNode == NULL)
		{
    		op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;  
		}
		else
		{
            strcpy(vlanNode->name, vlanName);
            npd_put_vlan(vlanNode);
		}
	}
	else {
		if (vlanNode->vid == vlanId)
		{
			op_ret = VLAN_RETURN_CODE_ERR_NONE;
		}
		else
		{
        	op_ret = VLAN_RETURN_CODE_NAME_CONFLICT;
		}
	}
    npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	
	return reply;
	
}


DBusMessage *npd_dbus_vlan_config_port_add_del
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char	isAdd = 0, isTagged = 0;
	unsigned short	vlanId = 0;	
    unsigned int pvid_set;

	unsigned int	netif_index = 0;
	unsigned int 	ret = VLAN_RETURN_CODE_ERR_NONE;
	unsigned int 	pvlan_type = 0;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_BYTE,&isAdd,
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_BYTE,&isTagged,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_UINT32,&pvid_set,
							DBUS_TYPE_INVALID))) {
		syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
#ifdef HAVE_PRIVATE_VLAN
	ret = npd_vlan_pvlan_port_config(netif_index, vlanId, isTagged, isAdd, &pvlan_type);	
	if (pvlan_type)
	{
		goto answer;
	}
#endif	
    if(isAdd)
    {	
        if((vlanId == DEFAULT_VLAN_ID)
            && (isTagged == FALSE))
            ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
        else
            ret = npd_netif_allow_vlan(netif_index, 
                vlanId, isTagged, pvid_set);
    }
    else
    {		
        if((vlanId == DEFAULT_VLAN_ID)
            && (isTagged == FALSE))
            ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
        else 
            ret = npd_netif_free_vlan(netif_index, 
                vlanId, isTagged);
    }

answer:	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage *npd_dbus_vlan_config_netif_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char	isAdd = 0, isTagged = 0;
	unsigned short	vlanId = 0;	
    switch_port_db_t switch_port = {0};

	unsigned int	netif_index = 0;
	unsigned int 	ret = VLAN_RETURN_CODE_ERR_NONE;
	unsigned int 	pvlan_type = 0;
	/*enum module_id_e module_type;*/
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_BYTE,&isAdd,
							DBUS_TYPE_UINT32,&netif_index,
							DBUS_TYPE_BYTE,&isTagged,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID))) {
		syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(DEFAULT_VLAN_ID == vlanId)
    {
        ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
        goto retCode;
    }       

    switch_port.global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
        goto retCode;
    }

#ifdef HAVE_PRIVATE_VLAN
	ret = npd_vlan_pvlan_port_config(netif_index, vlanId, isTagged, isAdd, &pvlan_type);	
	if (pvlan_type)
	{
		goto retCode;
	}
#endif

	if(isAdd)
	{		
        if(switch_port.vlan_access_mode != SWITCH_PORT_MODE_HYBRID)
        {
            ret = npd_netif_allow_vlan(netif_index,vlanId, FALSE, TRUE);
        }
        else
        {
            if(!NPD_VBMP_MEMBER(switch_port.allow_untag_vlans, vlanId))
            {
                ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
                goto retCode;
            }
            npd_netif_free_vlan(netif_index, DEFAULT_VLAN_ID, FALSE);
            ret = npd_vlan_netif_pvid_set(netif_index, vlanId);
        }  
    }
    else
    {		
        if(switch_port.vlan_access_mode != SWITCH_PORT_MODE_HYBRID)
        {
            ret = npd_netif_free_vlan(netif_index,vlanId, FALSE);
        }
        else
        {
            if(!NPD_VBMP_MEMBER(switch_port.allow_untag_vlans, vlanId))
            {
                ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
                goto retCode;
            }
            ret = npd_netif_allow_vlan(netif_index, DEFAULT_VLAN_ID, FALSE, TRUE);
            ret = npd_vlan_netif_pvid_set(netif_index, DEFAULT_VLAN_ID);
        }  
    }
retCode:    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
#ifdef HAVE_QINQ
DBusMessage *npd_dbus_vlan_config_netif_inner_pvid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char	isAdd = 0;
	unsigned short	vlanId = 0;	

	unsigned int	eth_g_index = 0;
	unsigned int 	ret = VLAN_RETURN_CODE_ERR_NONE;
	/*enum module_id_e module_type;*/
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_BYTE,&isAdd,
							DBUS_TYPE_UINT32,&eth_g_index,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID))) {
		syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(isAdd)
	{
       	ret = npd_vlan_netif_inner_pvid_set(eth_g_index, vlanId);
    }
    else
    {
		vlanId = DEFAULT_VLAN_ID;
       	ret = npd_vlan_netif_inner_pvid_set(eth_g_index, vlanId);
    }
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
#endif
DBusMessage * npd_dbus_vlan_get_vname_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	iter;
	DBusError err;
	char *vlanName;
	struct vlan_s*	vlanNode = NULL;
	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING,&vlanName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    vlanNode = npd_find_vlan_by_name(vlanName);
    
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
    if(NULL == vlanNode)
        op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&op_ret);

	if( VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->vid));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &vlanName);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->link_status));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->mtu));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->fdb_learning_mode));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->fdb_limit));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->isAutoCreated));
	}
	
    if(vlanNode)
        free(vlanNode);

    return reply;
}


DBusMessage * npd_dbus_vlan_get_base_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	iter;
	DBusError err;
	unsigned short	vlanId = 0;
	struct vlan_s*	vlanNode = NULL;
	int	op_ret = VLAN_RETURN_CODE_ERR_NONE;
	char *name = malloc(ALIAS_NAME_SIZE+1);

	if(name == NULL)
	{
		return NULL;
	}
	memset(name, 0, ALIAS_NAME_SIZE+1);
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_INVALID))) {
		syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(name);
		return NULL;
	}

    vlanNode = npd_find_vlan_by_vid(vlanId);
	if( vlanNode == NULL ) {
		op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&op_ret);

	if( VLAN_RETURN_CODE_ERR_NONE == op_ret) {
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->vid));	
	    memcpy(name, vlanNode->name, ALIAS_NAME_SIZE);
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &(name));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->link_status));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->mtu));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->fdb_learning_mode));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->fdb_limit));	
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &(vlanNode->isAutoCreated));
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_INT32,
										 &(vlanNode->pvlan_type));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32,
										&(vlanNode->isStatic));

		syslog_ax_vlan_dbg("vlan get base info: vid %d name %s linkstate %d, ret %d\n", vlanNode->vid, vlanNode->name, \
																	vlanNode->link_status, op_ret);
	}
	else {
		syslog_ax_vlan_dbg("vlan get base info: vid %d not exists\n", vlanId); 
	}
	
    if(vlanNode)
		free(vlanNode);

	free(name);
	
    return reply;
}

DBusMessage * npd_dbus_vlan_get_next_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned short	vlanId = 0;
	struct vlan_s vlan;
	int ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	memset(&vlan, 0, sizeof(struct vlan_s));

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
		return NULL;
	}

	syslog_ax_vlan_dbg("To get the next vid of vlan %d\n", vlanId);
    if(0 == vlanId)
        vlanId = -1;
	ret = dbtable_sequence_traverse_next(g_vlans, vlanId, &vlan);
	if(0 != ret || 4095 <= vlan.vid) {
		vlanId = 0;
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else {
		vlanId = vlan.vid;
	}

	syslog_ax_vlan_dbg("The next vid is %d.\n", vlanId);
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16,
									 &vlanId);
	
	return reply;

}


DBusMessage * npd_dbus_vlan_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned short	vlanId = 0;
	unsigned char isTagged = 0, type = 0;
	struct vlan_s*	vlanNode = NULL;
	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE,trunkId = 0;
	npd_pbmp_t      mbr,pbmp;
	int i;
	
	/*Read from SW Record or vlan entry table in HW*/
	memset(&mbr ,0 ,sizeof(npd_pbmp_t));
	memset(&pbmp ,0 ,sizeof(npd_pbmp_t));
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
						DBUS_TYPE_UINT16,&vlanId,
						DBUS_TYPE_BYTE, &isTagged,
						DBUS_TYPE_BYTE, &type,
						DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_vlan_dbg("To get vlan member: vlan %d, tag mode: %d, member type: %d.\n", vlanId, isTagged, type);

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if(NULL == vlanNode)
		op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&op_ret);

	if(VLAN_RETURN_CODE_ERR_NONE == op_ret )
	{
        unsigned int switch_port_index;
        unsigned int netif_index;
        unsigned int array_index;
        int ret;

        NPD_PBMP_ASSIGN(pbmp, (isTagged?vlanNode->tag_ports: vlanNode->untag_ports));
        NPD_PBMP_ITER(pbmp, switch_port_index)
        {
            ret = npd_switch_port_netif_index(switch_port_index, &netif_index);
            if(-1 == ret)
                continue;

            if(type == npd_netif_type_get(netif_index))
            {  
                if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
                {
                    trunkId = npd_netif_trunk_get_tid(netif_index);
                    NPD_PBMP_PORT_ADD(mbr, trunkId);
                     
                }
#ifdef HAVE_CAPWAP_ENGINE                
                else if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
                {
                	unsigned int bss_index = 0;
                    bss_index = npd_netif_get_bssindex(netif_index);
                    NPD_PBMP_PORT_ADD(mbr, bss_index);
                     
                }
#endif //HAVE_CAPWAP_ENGINE                
                else
                {
                    array_index = eth_port_array_index_from_ifindex(netif_index);
                    NPD_PBMP_PORT_ADD(mbr, array_index);
                } 
                  
                 
            }
        }       
	    for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
	    {
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 (unsigned int *)(&mbr) + i );
	    }	
	}

	if(vlanNode)
		free(vlanNode);
	
	return reply;
}


DBusMessage * npd_dbus_vlan_delete_vlan_entry_one(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vlanId = 0;
	unsigned int	ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    ret = npd_delete_vlan_by_vid(vlanId);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	
	return reply;
	
}

DBusMessage * npd_dbus_vlan_delete_vlan_entry_vname(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int	ret = VLAN_RETURN_CODE_ERR_NONE;    
	DBusError err;

	
	/* syslog_ax_vlan_dbg("Entering delete vlan one!\n");*/
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING,&vlanName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	vlanId = npd_vlan_get_id_by_name(vlanName);
    ret = npd_delete_vlan_by_vid(vlanId);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	return reply;
	
}

DBusMessage* npd_dbus_config_switchport_protect(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    int protect = 0;
    unsigned int netif_index = 0;
    unsigned int ret = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_UINT32,&protect,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	
    if(protect == 1)
    {
		protect = 2;/*protect mode*/
    }
	else
	{
		if(protect == 4)
		{
			protect = 4;
		}
		else
		{
		    protect = 1;/*learning and forwarding*/
		}
	}
    ret = npd_fdb_learning_mode_set(netif_index, protect);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}


DBusMessage * npd_dbus_vlan_config_switchport_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int netif_index;
    int mode;
	int ret  = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_UINT32,&mode,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = npd_netif_vlan_access_mode(netif_index, mode);
	if(0 == ret) {
		ret = VLAN_RETURN_CODE_ERR_NONE;
	}

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}


DBusMessage * npd_dbus_vlan_config_assoc_mac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    int isAdd;
	int ret  = 0;
	DBusError err;
	int cur_vid = 0;
	unsigned char mac[6];
    

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &isAdd,
								DBUS_TYPE_UINT32,&cur_vid,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],						
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    ret = npd_vlan_assoc_mac_set(isAdd, (char*)mac, cur_vid);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}
unsigned int vlan_assoc_mac_show_filter(void *in, void* out)
{
    macbase_vlan_t *item_a = (macbase_vlan_t*)in;
    macbase_vlan_t *item_b = (macbase_vlan_t*)out;

    if(item_a->vid == item_b->vid)
        return TRUE;
    else
        return FALSE;
}
DBusMessage * npd_dbus_vlan_show_assoc_mac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    macbase_vlan_t item;
    macbase_vlan_t item_out;
	int ret  = 0;
    int  opt_ret = VLAN_RETURN_CODE_MACBASE_EXIST;
	DBusError err;
    memset(&item, 0, sizeof(macbase_vlan_t));
    memset(&item_out, 0, sizeof(macbase_vlan_t));


	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&item.vid,
								DBUS_TYPE_BYTE,&item.mac[0],
								DBUS_TYPE_BYTE,&item.mac[1],
								DBUS_TYPE_BYTE,&item.mac[2],
								DBUS_TYPE_BYTE,&item.mac[3],
								DBUS_TYPE_BYTE,&item.mac[4],
								DBUS_TYPE_BYTE,&item.mac[5],
								
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    if(item.mac[0] == 0 && item.mac[1] == 0 && item.mac[2] == 0 && item.mac[3] == 0 && item.mac[4] == 0 && item.mac[5] == 0)
    {
        ret = dbtable_hash_head(mac_vlan_hash,  &item, &item_out, &vlan_assoc_mac_show_filter);
    }
    else
    {
        ret = dbtable_hash_next(mac_vlan_hash,  &item, &item_out, &vlan_assoc_mac_show_filter);
    }
    
    if(0 != ret)
    {
        opt_ret = 0; 
    }
	else
	{
    	npd_syslog_dbg("VLAN assocated with MAC %.2x-%.2x-%.2x-%.2x-%.2x-%.2x",
            item_out.mac[0], item_out.mac[1], item_out.mac[2], item_out.mac[3], item_out.mac[4], item_out.mac[5]);	
	}


	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[0]);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[1]);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[2]);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[3]);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[4]);
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &item_out.mac[5]);
	return reply;
	
}


DBusMessage * npd_dbus_vlan_config_assoc_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    subnetbase_vlan_t item;
    int isAdd;
	int ret  = 0;
	int cur_vid = 0;
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &isAdd,
								DBUS_TYPE_UINT32,&cur_vid,
								DBUS_TYPE_UINT32,&item.ipaddr,
								DBUS_TYPE_UINT32,&item.mask,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	item.vid = cur_vid;
    ret = dbtable_hash_search(subnet_vlan_hash, &item, NULL, &item);
    if(isAdd)
    {
        if(0 == ret)
        {
			if (cur_vid == item.vid)
			{
				ret = VLAN_RETURN_CODE_SUBNETBASE_EXIST;
			}
			else
			{
				ret = VLAN_RETURN_CODE_SUBNETBASE_OTHER_EXIST;
			}				
        }
        else
        {
			ret = nam_vlan_subnet_tbl_index_alloc((&item.tbl_index));
			if (0 == ret)
	            ret = dbtable_hash_insert(subnet_vlan_hash, &item);
    	}
    }
    else
    {
        if(0 != ret || cur_vid != item.vid)
        {
            ret = COMMON_SUCCESS;
        }
        else
        {
			ret = dbtable_hash_delete(subnet_vlan_hash, &item, &item);
			if (0 == ret)
				ret = nam_vlan_subnet_tbl_index_free(item.tbl_index);
		}
            
    }

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}
unsigned int vlan_assoc_subnet_show_filter(void *in, void* out)
{
    subnetbase_vlan_t *item_a = (subnetbase_vlan_t*)in;
    subnetbase_vlan_t *item_b = (subnetbase_vlan_t*)out;

    if(item_a->vid == item_b->vid)
        return TRUE;
    else
        return FALSE;
}

unsigned int vlan_assoc_protocol_group_show_filter(void *in, void* out)
{
    proto_vlan_port_t *item_a = (proto_vlan_port_t*)in;
    proto_vlan_port_t *item_b = (proto_vlan_port_t*)out;

    if(item_a->proto_group_index== item_b->proto_group_index)
        return TRUE;
    else
        return FALSE;
}

unsigned int vlan_assoc_protocol_vlanid_show_filter(void *in, void* out)
{
    proto_vlan_port_t *item_a = (proto_vlan_port_t*)in;
    proto_vlan_port_t *item_b = (proto_vlan_port_t*)out;

    if(item_a->vid == item_b->vid)
    {
        return TRUE;		
	}
    else
        return FALSE;
}

unsigned int vlan_assoc_protocol_netif_show_filter(void *in, void* out)
{
    proto_vlan_port_t *item_a = (proto_vlan_port_t*)in;
    proto_vlan_port_t *item_b = (proto_vlan_port_t*)out;

    if(item_a->netif_index== item_b->netif_index)
    {
        return TRUE;		
	}
    else
        return FALSE;
}


DBusMessage * npd_dbus_vlan_show_assoc_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    subnetbase_vlan_t item;
    subnetbase_vlan_t item_out;
    unsigned int index =0;
    unsigned int ipaddr[128] = {0};
    unsigned int ipmask[128] = {0};
	int ret  = 0;
    int opt_ret  = 0;
    int i = 0;
	DBusError err;
    
	dbus_error_init(&err);
	item.ipaddr = 0;
    item.mask = 0;
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &item.vid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        return NULL;
	}
    while(1)
    {
        ret = dbtable_hash_next(subnet_vlan_hash,  &item, &item_out, 
                    &vlan_assoc_subnet_show_filter);
        if(0 != ret)
        {
            npd_syslog_dbg("(sub_net_vlan) debug: BREAK");
            break;
        }
        opt_ret = VLAN_RETURN_CODE_SUBNETBASE_EXIST;
        ipaddr[index] = item.ipaddr = item_out.ipaddr;
        ipmask[index] = item.mask = item_out.mask;
        index++;
    }
 
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &opt_ret);
    
    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &index);
    for(i = 0; i < index; i++)
    {
        
        dbus_message_iter_append_basic (&iter,
      								     DBUS_TYPE_UINT32,
      								     &ipaddr[i]);
        dbus_message_iter_append_basic (&iter,
      								     DBUS_TYPE_UINT32,
      								     &ipmask[i]);
    }
	return reply;      
	
}


int vlan_private_show_filter(void *in, void* out)
{
    struct vlan_s *item_b = (struct vlan_s *)out;

    if(item_b->pvlan_type)
        return TRUE;
    else
        return FALSE;
}

DBusMessage * npd_dbus_privlan_get_next_vid
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned short	vlanId = 0;
	struct vlan_s vlan;
	int ret = VLAN_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vlanId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
		return NULL;
	}

	syslog_ax_vlan_dbg("To get the next private vlan of vlan %d\n", vlanId);
    if(0 == vlanId)
    {
		vlanId = 1;
    }
	
	memset(&vlan, 0, sizeof(struct vlan_s));

	vlan.vid = vlanId;
	while(ret == 0 && !vlan.pvlan_type)
	{
		ret = dbtable_sequence_traverse_next(g_vlans, vlan.vid, &vlan);
	}
	
	if(0 != ret || 4095 <= vlan.vid) {
		vlanId = 0;
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}
	else {
		vlanId = vlan.vid;
	}

	syslog_ax_vlan_dbg("The next private vlan is vlan %d\n", vlanId);
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16,
									 &vlanId);
	
	return reply;

}


DBusMessage * npd_dbus_privlan_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned short	vlanId = 0;
	unsigned char pri_mode = 0, type = 0;

	struct vlan_s*	vlanNode = NULL;
	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE,trunkId = 0;
	npd_pbmp_t      mbr,pbmp;
	int i;
	
	/*Read from SW Record or vlan entry table in HW*/
	memset(&mbr ,0 ,sizeof(npd_pbmp_t));
	memset(&pbmp ,0 ,sizeof(npd_pbmp_t));
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
						DBUS_TYPE_UINT16,&vlanId,
						DBUS_TYPE_BYTE, &pri_mode,			
						DBUS_TYPE_BYTE, &type,
						DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_vlan_dbg("get vlan port member: vlan %d private mode %d type %d\n", vlanId, pri_mode, type);

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if(NULL == vlanNode)
		op_ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
					DBUS_TYPE_UINT32,&op_ret);

	if(VLAN_RETURN_CODE_ERR_NONE == op_ret )
	{
        unsigned int switch_port_index;
        unsigned int netif_index;
        unsigned int array_index;
        switch_port_db_t switch_port = {0};
        int ret;
	
        NPD_PBMP_ASSIGN(pbmp, vlanNode->tag_ports);
        NPD_PBMP_OR(pbmp, vlanNode->untag_ports);
        NPD_PBMP_ITER(pbmp, switch_port_index)
        {
            ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port );

            if(-1 == ret)
                continue;
            
            netif_index =   switch_port.global_port_ifindex;
              
	    
            if(pri_mode == switch_port.vlan_private_mode &&
				type == npd_netif_type_get(netif_index))
            {  
                if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
                {
                    
                    trunkId = npd_netif_trunk_get_tid(netif_index);
                    NPD_PBMP_PORT_ADD(mbr, trunkId);
                     
                }
                else
                {
                    array_index = eth_port_array_index_from_ifindex(netif_index);
                    NPD_PBMP_PORT_ADD(mbr, array_index);
                } 
            }
        }       
	    for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
	    {
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 (unsigned int *)(&mbr) + i );
	    }	
	}

	if(vlanNode)
		free(vlanNode);
	
	return reply;
}

DBusMessage * npd_dbus_vlan_config_private(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    struct vlan_s vlan;
    unsigned int vid;
    int isEnable;
	int ret  = 0;
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &vid,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    vlan.vid = vid;    
	npd_key_database_lock();
    ret = dbtable_sequence_search(g_vlans, vid, &vlan);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto answer;
    }
    {
        int count;
        NPD_PBMP_COUNT(vlan.tag_ports, count);
        if(count)
        {
            ret = VLAN_RETURN_CODE_PORT_EXISTS;
            goto answer;
        }
        NPD_PBMP_COUNT(vlan.untag_ports, count);
        if(count)
        {
            ret = VLAN_RETURN_CODE_PORT_EXISTS;
            goto answer;
        }

        vlan.pvlan_type = isEnable;
        ret = dbtable_sequence_update(g_vlans, vid, NULL, &vlan);
    }

answer:
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

int npd_vlan_is_port_in_privlan(unsigned int netif_index)
{
	switch_port_db_t switch_port;
	npd_vbmp_t bmp;
	int pri_cnt = 0;
	int ret;
	int vlan;
	
    switch_port.global_port_ifindex = netif_index;    
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        return 0;
    }

	
	NPD_VBMP_ASSIGN(bmp, switch_port.allow_untag_vlans);
	NPD_VBMP_OR(bmp, switch_port.allow_tag_vlans);
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
        	pri_cnt++;
        }
	}
	
	return pri_cnt;
}

DBusMessage * npd_dbus_netif_config_private(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    struct switch_port_db_s switch_port;
    unsigned int netif_index;
    int mode;
	int ret  = 0;
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&mode,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch_port.global_port_ifindex = netif_index; 
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
        goto answer;
    }
    if((switch_port.vlan_access_mode != SWITCH_PORT_MODE_ACCESS)
        &&(mode == SWITCH_PORT_PRIVLAN_ISOLATED))
    {
        ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
        goto answer;
    }

	if (0 == npd_vlan_is_port_in_privlan(netif_index))
	{
		ret = VLAN_RETURN_CODE_VLAN_NOT_PRIVLAN;
		goto answer;
	}
	//first delete isolation between port
	if ((switch_port.vlan_private_mode != SWITCH_PORT_PRIVLAN_PROMI)
		&& (mode != SWITCH_PORT_PRIVLAN_PROMI))
	{
		switch_port.vlan_private_mode = SWITCH_PORT_PRIVLAN_PROMI;
        dbtable_array_update(switch_ports, switch_port.switch_port_index, 
            NULL, &switch_port);
		
	}
	
    {
        switch_port.vlan_private_mode = mode;
        dbtable_array_update(switch_ports, switch_port.switch_port_index, 
            NULL, &switch_port);
    }

answer:
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

#ifdef HAVE_PORT_ISOLATE
DBusMessage * npd_dbus_netif_config_isolate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    struct switch_port_db_s switch_port;
    unsigned int netif_index;
    unsigned int array_port;
	
    unsigned int isolate;
	int ret  = 0;
	DBusError err;
	struct port_isolate_group_s group_default = {0};
	npd_pbmp_t isolate_pbmp ;
	npd_pbmp_t temp_pbmp ;
	
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&isolate,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch_port.global_port_ifindex = netif_index;  
    npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
        goto answer;
    }
	ret = dbtable_array_get(isolate_ports, isolate_group_global_no, &group_default);
	if (ret != VLAN_RETURN_CODE_ERR_NONE)
	{
		goto answer;
	}

	NPD_PBMP_CLEAR(isolate_pbmp);
	NPD_PBMP_CLEAR(temp_pbmp);

    array_port = netif_array_index_from_ifindex(netif_index);

    NPD_PBMP_PORT_ADD(isolate_pbmp, array_port);

/*	
	if (NPD_NETIF_ETH_TYPE ==  npd_netif_type_get(netif_index))
	{
		array_port = eth_port_array_index_from_ifindex(netif_index);
		NPD_PBMP_PORT_ADD(isolate_pbmp, array_port);
	}
	else if (NPD_NETIF_TRUNK_TYPE ==  npd_netif_type_get(netif_index))
	{
		struct trunk_s node = {0};
		unsigned int eth_g_index = 0;
		
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		NPD_PBMP_ITER(node.ports, array_port)
		{
			NPD_PBMP_PORT_ADD(isolate_pbmp, array_port);
		}
		if (isolate)
		{
			NPD_PBMP_PORT_ADD(group_default.isolate_trunks, node.trunk_id);
		}
		else
		{
			NPD_PBMP_PORT_REMOVE(group_default.isolate_trunks, node.trunk_id);			
		}
	}
*/

	NPD_PBMP_ASSIGN(temp_pbmp, group_default.isolate_ports);
	NPD_PBMP_AND(temp_pbmp, isolate_pbmp); /* test if there is have  ports in group*/
	if (isolate)
	{		
		if (NPD_PBMP_NOT_NULL(group_default.isolate_ports)
			&& NPD_PBMP_NOT_NULL(temp_pbmp)) 
		{
			ret = VLAN_RETURN_CODE_PORT_ISOLATED;
			goto answer;
			/* this the port has been configured */
		}
		NPD_PBMP_OR(group_default.isolate_ports, isolate_pbmp);
#ifdef HAVE_PORT_ISOLATE		
		switch_port.port_isolate = TRUE;
#endif
	}	
	else
	{
		if (NPD_PBMP_IS_NULL(temp_pbmp)) 
		{
			ret = VLAN_RETURN_CODE_PORT_NOT_ISOLATED;
			goto answer;
			/* this the port is not' in this */
		}
		NPD_PBMP_REMOVE(group_default.isolate_ports, isolate_pbmp);		
#ifdef HAVE_PORT_ISOLATE		
		switch_port.port_isolate = FALSE;
#endif
	}
	ret = dbtable_array_update(isolate_ports, isolate_group_global_no, NULL, &group_default);
    ret = dbtable_hash_update(switch_ports_hash, NULL, &switch_port);
	

answer:
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}


DBusMessage * npd_dbus_isolate_group_get_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned short	groupId = 0;
	unsigned char type = 0;
	struct port_isolate_group_s group_default = {0};

	unsigned int	op_ret = VLAN_RETURN_CODE_ERR_NONE;
	npd_pbmp_t      mbr,pbmp;
	int i;
	
	/*Read from SW Record or vlan entry table in HW*/
	memset(&mbr ,0 ,sizeof(npd_pbmp_t));
	memset(&pbmp ,0 ,sizeof(npd_pbmp_t));
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
						DBUS_TYPE_UINT16,&groupId,
						DBUS_TYPE_BYTE, &type,
						DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	op_ret = dbtable_array_get(isolate_ports, isolate_group_global_no, &group_default);
	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
					DBUS_TYPE_UINT32,&op_ret);

	if(VLAN_RETURN_CODE_ERR_NONE == op_ret )
	{
        unsigned int switch_port_index;
        switch_port_db_t switch_port = {0};
        int ret;

		if (type == NPD_NETIF_ETH_TYPE)
		{
	        NPD_PBMP_ASSIGN(pbmp, group_default.isolate_ports);
			NPD_PBMP_ITER(pbmp, switch_port_index)
			{
	            ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port );
	            if(-1 == ret)
	                continue;
				NPD_PBMP_PORT_ADD(mbr, switch_port_index);
			}
		}
		else if (type == NPD_NETIF_TRUNK_TYPE)
		{
			NPD_PBMP_ASSIGN(mbr, group_default.isolate_trunks);
		}
	    for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
	    {
		    dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 (unsigned int *)(&mbr) + i );
	    }	
	}
	
	return reply;
}
#endif


#ifdef HAVE_PRIVATE_VLAN

unsigned int npd_vlan_get_assoc_primary_vid
(
	unsigned int secondary_vlan
)
{
	pvlan_isolate_t pvlan_isolate = {0};
	unsigned int ret = 0;

	pvlan_isolate.vid = secondary_vlan;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, secondary_vlan, (void *)&pvlan_isolate);
	if (ret == 0)
	{
		return pvlan_isolate.primary_vid;
	}
	else
	{
		return NPD_FALSE;
	}
}


unsigned int npd_pvlan_netif_type_get(
	unsigned int netif_index,
	unsigned int* pvlan_port_type
	)
{
    struct switch_port_db_s switch_port;
	unsigned int ret  = 0;

    switch_port.global_port_ifindex = netif_index; 
	
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
        goto answer;
    }

	*pvlan_port_type = switch_port.vlan_private_mode;	

answer:
	npd_key_database_unlock();	

	return ret;
}
unsigned int npd_pvlan_netif_type_set(
	unsigned int netif_index,
	unsigned int pvlan_port_type
	)
{
    struct switch_port_db_s switch_port;
	unsigned int ret  = 0;

    switch_port.global_port_ifindex = netif_index; 
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
        goto answer;
    }
	
    switch_port.vlan_private_mode = pvlan_port_type;
    ret = dbtable_array_update(switch_ports, switch_port.switch_port_index, 
        NULL, &switch_port);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}

answer:
	npd_key_database_unlock();	

	return ret;
}

unsigned int npd_pvlan_netif_type_unset(
	unsigned int netif_index
)
{
    struct switch_port_db_s switch_port;
	unsigned int ret  = 0;

    switch_port.global_port_ifindex = netif_index; 
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
        goto answer;
    }
	
    switch_port.vlan_private_mode = SWITCH_PORT_PRIVLAN_NORMAL;
    ret = dbtable_array_update(switch_ports, switch_port.switch_port_index, 
        NULL, &switch_port);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}

answer:
	npd_key_database_unlock();	

	return ret;
}

unsigned int npd_vlan_pvlan_type_get(
	unsigned int vid,
	unsigned int* pvlan_type
)
{
	struct vlan_s * vlan = NULL;
	unsigned int ret = 0;
	
	if (pvlan_type == NULL)
	{
		ret = VLAN_RETURN_CODE_BADPARAM;
		goto answer;
	}
	
	vlan = npd_find_vlan_by_vid(vid);
	if (vlan == NULL)
	{
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		goto answer;
	}
	*pvlan_type = vlan->pvlan_type;
	
answer:
	if (vlan != NULL)
		free(vlan);
		
	return ret;	
}

unsigned int npd_vlan_pvlan_type_set(
	unsigned int vid,
	unsigned int pvlan_type
	)
{
	struct vlan_s * vlan = NULL;
	unsigned int ret = 0;

	npd_key_database_lock();
	vlan = npd_find_vlan_by_vid(vid);
	if (vlan == NULL)
	{
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		goto answer;
	}
	
	if (!NPD_PBMP_IS_NULL(vlan->untag_ports) || 
		!NPD_PBMP_IS_NULL(vlan->tag_ports))
	{
		ret = VLAN_RETURN_CODE_PORT_EXISTS;
		goto answer;
	}
	if (PVLAN_VLAN_TYPE_NORMAL != vlan->pvlan_type)
	{
		ret = VLAN_RETURN_CODE_IS_PRIVATE_VLAN;
		goto answer;
	}	
	vlan->pvlan_type = pvlan_type;
	
	ret = dbtable_sequence_update(g_vlans, vid, NULL, vlan);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}
	
answer:
	if (vlan != NULL)
		free(vlan);
	
	npd_key_database_unlock();
	
	return ret;	
}

unsigned int npd_vlan_pvlan_type_unset(
	unsigned int vid	
)
{
	struct vlan_s * vlan = NULL;
	unsigned int ret = 0;	

	npd_key_database_lock();
	vlan = npd_find_vlan_by_vid(vid);
	if (vlan == NULL)
	{
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		goto answer;
	}
	
	if (PVLAN_VLAN_TYPE_NORMAL == vlan->pvlan_type)
	{
		ret = VLAN_RETURN_CODE_NOT_PRIVATE_VLAN;
		goto answer;
	}	
	vlan->pvlan_type = PVLAN_VLAN_TYPE_NORMAL;
	
	ret = dbtable_sequence_update(g_vlans, vid, NULL, vlan);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}
	
answer:
	if (vlan != NULL)
		free(vlan);
	
	npd_key_database_unlock();
	
	return ret;	
}

unsigned int npd_vlan_pvlan_primary_add(unsigned int vid)
{
	pvlan_primary_t pvlan_primary = {0};

	unsigned int ret = 0;
	
	ret = npd_vlan_pvlan_type_set(vid, PVLAN_VLAN_TYPE_PRIMARY);
	if (0 != ret)
	{
		return ret;
	}

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = vid;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, vid, (void * )&pvlan_primary);	
	if (0 == ret)
	{
		return ret;
	}
	/* This is need judge the primary vlan count */
	
	ret = dbtable_sequence_insert(
		pvlan_primary_vid_sequence, vid, (void * )&pvlan_primary);	
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}

	return ret;
}

unsigned int npd_vlan_pvlan_primary_del(unsigned int vid)
{
	pvlan_primary_t pvlan_primary = {0};
	pvlan_primary_t ret_pvlan_primary;
	unsigned int ret = 0;

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = vid;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, vid, (void * )&pvlan_primary);	
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}
	if (!NPD_VBMP_IS_NULL(pvlan_primary.secondary_vlans))
	{
		ret = VLAN_RETURN_CODE_PRIMARY_VLAN_HAS_ASSOC;
		return ret;
	}

	if (!NPD_PBMP_IS_NULL(pvlan_primary.ports))
	{
		unsigned int array_port = 0;
		unsigned int netif_index = 0;
		unsigned int op_ret = 0;
		
		/* free to isolate vlan for promis port */
		NPD_PBMP_ITER(pvlan_primary.ports, array_port)
		{
			netif_index = netif_array_index_to_ifindex(array_port);
			op_ret |= npd_vlan_pvlan_promis_port_del(netif_index, vid);
		}
	}

	
	/* This is need judge the primary vlan count */
	
	ret = dbtable_sequence_delete(pvlan_primary_vid_sequence, 
			vid, (void * )&pvlan_primary, (void * )&ret_pvlan_primary);	
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}
	
	ret = npd_vlan_pvlan_type_unset(vid);

	return ret;

}


unsigned int npd_vlan_pvlan_isolate_assoc_add(
	unsigned int primary_vid,
	unsigned int isolate_vid
)
{
	unsigned int ret = 0;
	unsigned int prim_pvlan_type = 0;
	unsigned int assoc_prim_vid = 0;
	unsigned int array_port = 0;
	unsigned int netif_index = 0;
	unsigned int op_ret = 0;
	
	pvlan_primary_t pvlan_primary ;
	pvlan_isolate_t pvlan_isolate ;

	ret = npd_vlan_pvlan_type_get(primary_vid, &prim_pvlan_type);
	if (0 != ret )
	{
		return ret;
	}
	if (prim_pvlan_type != PVLAN_VLAN_TYPE_PRIMARY)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}
	
	ret = npd_vlan_pvlan_type_set(isolate_vid, PVLAN_VLAN_TYPE_ISOLATE);
	if (0 != ret)
	{
		return ret;
	}

	assoc_prim_vid = npd_vlan_get_assoc_primary_vid(isolate_vid);
	if (assoc_prim_vid != 0)
	{
		if (assoc_prim_vid != primary_vid)
		{
			ret = VLAN_RETURN_CODE_SECOND_VLAN_ASSOC_OTHER; 
		}
		else
		{
			//ret = VLAN_RETURN_CODE_SECOND_VLAN_HAS_ASSOC;
		}		
		return ret;
	}	

	/* update the primary vlan association */
	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = primary_vid;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, primary_vid, (void * )&pvlan_primary);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}

	NPD_VBMP_VLAN_ADD(pvlan_primary.secondary_vlans, isolate_vid);
	ret = dbtable_sequence_update(pvlan_primary_vid_sequence, 
						primary_vid, NULL, (void *)&pvlan_primary);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}	


	/* add allow to isolate vlan for promis port */
	NPD_PBMP_ITER(pvlan_primary.ports, array_port)
	{
		netif_index = netif_array_index_to_ifindex(array_port);
		op_ret |= npd_netif_allow_vlan(netif_index, isolate_vid, FALSE, FALSE);
	}
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}
	

	/* insert the isolate vlan */
	memset(&pvlan_isolate, 0, sizeof(pvlan_isolate_t));
	pvlan_isolate.vid = isolate_vid;
	pvlan_isolate.primary_vid = primary_vid;
	ret = dbtable_sequence_insert(
		pvlan_isolate_vid_sequence, isolate_vid, (void *)&pvlan_isolate);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}
	
	return ret;	
}

unsigned int npd_vlan_pvlan_isolate_assoc_del(
	unsigned int primary_vid,
	unsigned int isolate_vid
)
{
	unsigned int ret = 0;
	unsigned int prim_pvlan_type = 0;
	unsigned int isolate_pvlan_type = 0;
	
	unsigned int assoc_prim_vid = 0;
	pvlan_primary_t pvlan_primary ;
	pvlan_isolate_t pvlan_isolate ;
	pvlan_isolate_t ret_pvlan_isolate ;
	unsigned int netif_index = 0;
	unsigned int array_port = 0;
	unsigned int op_ret = 0;
	
	ret = npd_vlan_pvlan_type_get(primary_vid, &prim_pvlan_type);
	if (0 != ret )
	{
		return ret;
	}
	if (prim_pvlan_type != PVLAN_VLAN_TYPE_PRIMARY)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}
	ret = npd_vlan_pvlan_type_get(isolate_vid, &isolate_pvlan_type);
	if (0 != ret )
	{
		return ret;
	}	
	assoc_prim_vid = npd_vlan_get_assoc_primary_vid(isolate_vid);
	if (assoc_prim_vid == 0)
	{
		ret = VLAN_RETURN_CODE_SECOND_VLAN_NO_ASSOC;
		return ret;
	}
	else if (assoc_prim_vid != primary_vid)
	{
		ret = VLAN_RETURN_CODE_SECOND_VLAN_ASSOC_OTHER;
		return ret;
	}

	/* delete the isolate vlan */
	memset(&pvlan_isolate, 0, sizeof(pvlan_isolate_t));
	pvlan_isolate.vid = isolate_vid;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, isolate_vid, (void * )&pvlan_isolate);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_NO_ISOLATE_VLAN;
		return ret;
	}	
	/* if isolate vlan has port , remove them */
	if (!NPD_PBMP_IS_NULL(pvlan_isolate.ports))
	{
		unsigned int array_port = 0;
		unsigned int netif_index = 0;
		NPD_PBMP_ITER(pvlan_isolate.ports, array_port)
		{
			netif_index = netif_array_index_to_ifindex(array_port);
			npd_vlan_pvlan_isolate_port_del(netif_index, isolate_vid);
		}
	}	
	ret = dbtable_sequence_delete(pvlan_isolate_vid_sequence,
				isolate_vid, (void *)&pvlan_isolate, (void *)&ret_pvlan_isolate);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}		
	

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = primary_vid;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, primary_vid, (void * )&pvlan_primary);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}	

	/* free to isolate vlan for promis port */
	NPD_PBMP_ITER(pvlan_primary.ports, array_port)
	{
		netif_index = netif_array_index_to_ifindex(array_port);
		op_ret |= npd_netif_free_vlan(netif_index, isolate_vid, FALSE);
	}
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}	

	/* update the primary vlan association */
	NPD_VBMP_VLAN_REMOVE(pvlan_primary.secondary_vlans, isolate_vid);
	ret = dbtable_sequence_update(pvlan_primary_vid_sequence, 
						primary_vid, NULL, (void *)&pvlan_primary);
	if (0 != ret)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}		

	ret = npd_vlan_pvlan_type_unset(isolate_vid);

	return ret;
}


unsigned int npd_vlan_pvlan_promis_port_add(
	unsigned int netif_index, 
	unsigned int primary_vlan
)
{

	pvlan_primary_t pvlan_primary = {0};

	unsigned int second_vid ;
	unsigned short port_pvid = 0;
	unsigned int ret  = 0;	
	unsigned int op_ret  = 0;	

	unsigned int array_port = 0;

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = primary_vlan;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, primary_vlan, (void * )&pvlan_primary);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret ;
    }
	npd_vlan_port_pvid_get(netif_index, &port_pvid);	
	if (port_pvid != DEFAULT_VLAN_ID)
	{
		ret = VLAN_RETURN_CODE_PORT_PVID_NOT_DEFAULT;
		return ret;
	}
	
	ret = npd_netif_vlan_access_mode(netif_index, SWITCH_PORT_MODE_HYBRID);
	if (ret != 0)
	{
		return ret;
	}
	
	NPD_VBMP_ITER(pvlan_primary.secondary_vlans, second_vid)
	{
		op_ret |= npd_netif_allow_vlan(netif_index, second_vid, FALSE, FALSE);
	}
	op_ret |= npd_netif_allow_vlan(netif_index, primary_vlan, FALSE, FALSE);
	op_ret |= npd_vlan_netif_pvid_set(netif_index, primary_vlan);
	op_ret |= npd_netif_free_vlan(netif_index, DEFAULT_VLAN_ID, FALSE);

	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}

	if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
	{
		array_port = netif_array_index_from_ifindex(netif_index);
		NPD_PBMP_PORT_ADD(pvlan_primary.ports, array_port);
	}
	else if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
	    struct trunk_s node = {0};
		
	    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
	    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		if (0 != ret)
		{
			ret = VLAN_RETURN_CODE_ERR_HW;
			return ret;			
		}
		NPD_PBMP_OR(pvlan_primary.ports, node.ports);
	}
	
	ret = dbtable_sequence_update(pvlan_primary_vid_sequence,
		 					primary_vlan, NULL, (void * )&pvlan_primary);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
    }

	ret = npd_pvlan_netif_type_set(netif_index, SWITCH_PORT_PRIVLAN_PROMI);
	
	
	return ret ;
}

unsigned int npd_vlan_pvlan_promis_port_del(
	unsigned int netif_index, 
	unsigned int primary_vlan
)
{
	pvlan_primary_t pvlan_primary = {0};

	unsigned int second_vid ;
	unsigned short port_pvid = 0;
	unsigned int ret  = 0;	
	unsigned int op_ret  = 0;	

	unsigned int array_port = 0;

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = primary_vlan;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, primary_vlan, (void * )&pvlan_primary);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
    }
	npd_vlan_port_pvid_get(netif_index, &port_pvid);	
	if (port_pvid != primary_vlan)
	{
		ret = VLAN_RETURN_CODE_PORT_PVID_NOT_PVLAN;
		return ret;
	}

	ret = npd_pvlan_netif_type_unset(netif_index);
	if (0 != ret)
	{
		return ret;
	}

	if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
	{
		array_port = netif_array_index_from_ifindex(netif_index);
		NPD_PBMP_PORT_REMOVE(pvlan_primary.ports, array_port);
	}
	else if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
	    struct trunk_s node = {0};
	    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
	    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		if (0 != ret)
		{
			ret = VLAN_RETURN_CODE_ERR_HW;
			return ret;			
		}
		NPD_PBMP_REMOVE(pvlan_primary.ports, node.ports);
	}
	
	array_port = netif_array_index_from_ifindex(netif_index);
	NPD_PBMP_PORT_REMOVE(pvlan_primary.ports, array_port);
	ret = dbtable_sequence_update(pvlan_primary_vid_sequence,
		 					primary_vlan, NULL, (void * )&pvlan_primary);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
		return ret;
    }
	
    op_ret |= npd_netif_allow_vlan(netif_index, DEFAULT_VLAN_ID, FALSE, TRUE);
    op_ret |= npd_vlan_netif_pvid_set(netif_index, DEFAULT_VLAN_ID);	
	op_ret |= npd_netif_free_vlan(netif_index, primary_vlan, FALSE);
	
	NPD_VBMP_ITER(pvlan_primary.secondary_vlans, second_vid)
	{
		op_ret |= npd_netif_free_vlan(netif_index, second_vid, FALSE);
	}		
	op_ret |= npd_netif_vlan_access_mode(netif_index, SWITCH_PORT_MODE_TRUNK);
	
	/* port mode dont't do nothing */	
	
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}
	return ret;
}



unsigned int npd_vlan_pvlan_isolate_port_add(
	unsigned int netif_index, 
	unsigned int isolate_vlan
)
{
	pvlan_isolate_t pvlan_isolate = {0};
    unsigned int primary_vlan;

	unsigned int array_port = 0;
	unsigned short port_pvid = 0;
	
	unsigned int ret  = 0;
	unsigned int op_ret  = 0;

	memset(&pvlan_isolate, 0, sizeof(pvlan_isolate));
	pvlan_isolate.vid = isolate_vlan;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, isolate_vlan, (void * )&pvlan_isolate);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_NO_ISOLATE_VLAN;
		return ret;
    }	
	
	npd_vlan_port_pvid_get(netif_index, &port_pvid);	
	if (port_pvid != DEFAULT_VLAN_ID)
	{
		ret = VLAN_RETURN_CODE_PORT_PVID_NOT_DEFAULT;
		return ret;
	}

	primary_vlan = pvlan_isolate.primary_vid;	

	/* set isolate port pvid and port mode */

	op_ret |= npd_netif_vlan_access_mode(netif_index, SWITCH_PORT_MODE_HYBRID);
	op_ret |= npd_netif_allow_vlan(netif_index, primary_vlan, FALSE, FALSE);		
	
	op_ret |= npd_netif_allow_vlan(netif_index, isolate_vlan, FALSE, FALSE);
	op_ret |= npd_vlan_netif_pvid_set(netif_index, isolate_vlan);
	op_ret |= npd_netif_free_vlan(netif_index, DEFAULT_VLAN_ID, FALSE);
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}

	array_port = netif_array_index_from_ifindex(netif_index);	
	NPD_PBMP_PORT_ADD(pvlan_isolate.ports, array_port);	
	ret = dbtable_sequence_update(pvlan_isolate_vid_sequence,
								isolate_vlan, NULL, (void *)&pvlan_isolate);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
		return ret;
	}	

	ret = npd_pvlan_netif_type_set(netif_index, SWITCH_PORT_PRIVLAN_ISOLATED);
	
	return ret;
}

unsigned int npd_vlan_pvlan_isolate_port_del(
	unsigned int netif_index, 
	unsigned int isolate_vlan
)
{
	pvlan_isolate_t pvlan_isolate = {0};
    unsigned int primary_vlan;

	unsigned int array_port = 0;
	unsigned short port_pvid = 0;
	
	unsigned int ret  = 0;
	unsigned int op_ret  = 0;

	memset(&pvlan_isolate, 0, sizeof(pvlan_isolate));
	pvlan_isolate.vid = isolate_vlan;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, isolate_vlan, (void * )&pvlan_isolate);
    if(0 != ret)
    {
		ret = VLAN_RETURN_CODE_NO_ISOLATE_VLAN;
		return ret;
    }
	npd_vlan_port_pvid_get(netif_index, &port_pvid);		
	if (port_pvid != isolate_vlan)
	{
		ret = VLAN_RETURN_CODE_PORT_PVID_NOT_DEFAULT;
		return ret;
	}

	/* set isolate port pvid and port mode */

	array_port = netif_array_index_from_ifindex(netif_index);	
	NPD_PBMP_PORT_REMOVE(pvlan_isolate.ports, array_port);	
	ret = dbtable_sequence_update(pvlan_isolate_vid_sequence,
								isolate_vlan, NULL, (void *)&pvlan_isolate);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
		return ret;
	}

	primary_vlan = pvlan_isolate.primary_vid;	
    op_ret |= npd_netif_allow_vlan(netif_index, DEFAULT_VLAN_ID, FALSE, TRUE);
    op_ret |= npd_vlan_netif_pvid_set(netif_index, DEFAULT_VLAN_ID);	
	op_ret |= npd_netif_free_vlan(netif_index, isolate_vlan, FALSE);	
	op_ret |= npd_netif_free_vlan(netif_index, primary_vlan, FALSE);
	op_ret |= npd_netif_vlan_access_mode(netif_index, SWITCH_PORT_MODE_TRUNK);


	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
		return ret;
	}

	ret = npd_pvlan_netif_type_unset(netif_index);
	
	return ret;
}

unsigned int npd_vlan_pvlan_port_config(
	unsigned int netif_index,
	unsigned short vlan_id,
	unsigned char is_tagged,
	unsigned char is_add,
	unsigned int *pvlan_type
	
)
{
	unsigned int 	ret = VLAN_RETURN_CODE_ERR_NONE;
	if (pvlan_type == NULL)
		return VLAN_RETURN_CODE_BADPARAM;

	ret = npd_vlan_pvlan_type_get(vlan_id, pvlan_type);
	if (ret != 0)
		return ret;

	if (*pvlan_type == PVLAN_VLAN_TYPE_NORMAL)
		return 0;
	
	if (*pvlan_type && is_tagged)
	{
		ret = VLAN_RETURN_CODE_PVLAN_CONFIG_NOT_ALLOWED;
		return ret;
	}

	if (*pvlan_type == PVLAN_VLAN_TYPE_PRIMARY)
	{
		if (is_add)
		{
			ret = npd_vlan_pvlan_promis_port_add(netif_index,  vlan_id);
		}
		else
		{
			ret = npd_vlan_pvlan_promis_port_del(netif_index,  vlan_id);
		}		
	}
	else if (*pvlan_type == PVLAN_VLAN_TYPE_ISOLATE)
	{
		if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
		{
			ret = VLAN_RETURN_CODE_PVLAN_TRUNK_NOT_SUPPORT;
			return ret;
		}
		
		if (is_add)
		{
			ret = npd_vlan_pvlan_isolate_port_add(netif_index,  vlan_id);
		}
		else
		{
			ret = npd_vlan_pvlan_isolate_port_del(netif_index,  vlan_id);
		}		
	}	

	return ret;	
}

unsigned int npd_vlan_pvlan_delete_primary_vlan(unsigned int vid)
{
	pvlan_primary_t pvlan_primary = {0};
	unsigned int ret  = 0;
	unsigned int op_ret  = 0;
	
	unsigned int netif_index;
	unsigned int array_port = 0;

	npd_pbmp_t pbm;
	

	memset(&pvlan_primary, 0, sizeof(pvlan_primary_t));
	pvlan_primary.vid = vid;
	ret = dbtable_sequence_search(
		pvlan_primary_vid_sequence, vid, (void * )&pvlan_primary);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_IS_NOT_PRIMARY_VLAN;
		return ret;
	}

	if (!NPD_VBMP_IS_NULL(pvlan_primary.secondary_vlans))
	{
		ret = VLAN_RETURN_CODE_PRIMARY_VLAN_HAS_ASSOC;
		return ret;
	}
	
	NPD_PBMP_ASSIGN(pbm, pvlan_primary.ports);	
	NPD_PBMP_ITER(pbm, array_port)
	{
		netif_index = netif_array_index_to_ifindex(array_port);
		op_ret |= npd_vlan_pvlan_promis_port_del(netif_index, vid);
	}
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
	}
	return ret;
}

unsigned int npd_vlan_pvlan_delete_isolate_vlan(unsigned int vid)
{
	pvlan_isolate_t pvlan_isolate = {0};

	unsigned int netif_index;
	unsigned int array_port = 0;
	
	unsigned int ret  = 0;
	unsigned int op_ret  = 0;

	npd_pbmp_t pbm;
	
	memset(&pvlan_isolate, 0, sizeof(pvlan_isolate));
	pvlan_isolate.vid = vid;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, vid, (void * )&pvlan_isolate);
    if(0 != ret)
    {
		//ret = VLAN_RETURN_CODE_IS_NOT_ISOLATE_VLAN;
		return ret;
    }
	NPD_PBMP_ASSIGN(pbm, pvlan_isolate.ports);
	NPD_PBMP_ITER(pbm, array_port)
	{
		netif_index = netif_array_index_to_ifindex(array_port);		
		op_ret |= npd_vlan_pvlan_isolate_port_del(netif_index, vid);
	}
	if (op_ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_HW;
		return ret;
	}
	ret = npd_vlan_pvlan_isolate_assoc_del(pvlan_isolate.primary_vid, vid);
	
	return ret;
}

unsigned int npd_vlan_pvlan_delete_by_id(unsigned int vid)
{
	unsigned int ret = 0;
	unsigned int pvlan_type = 0;
	
	ret = npd_vlan_pvlan_type_get(vid, &pvlan_type);
	if (0 != ret)
		return ret;
	
	if (pvlan_type == PVLAN_VLAN_TYPE_PRIMARY)
	{
		ret = npd_vlan_pvlan_delete_primary_vlan(vid);
	}
	else if (pvlan_type == PVLAN_VLAN_TYPE_ISOLATE)
	{
		ret = npd_vlan_pvlan_delete_isolate_vlan(vid);
	}

	return ret;
}

int npd_netif_free_all_pvlan(unsigned int netif_index)
{
	unsigned int pvlan_port_type;
	unsigned int pvid = 0;
	
	npd_pvlan_netif_type_get(netif_index, &pvlan_port_type);
	if (!pvlan_port_type)
	{
		return -1;
	}
	npd_vlan_port_pvid_get(netif_index, (unsigned short *)&pvid);	
	if (pvlan_port_type == SWITCH_PORT_PRIVLAN_PROMI)
	{
		npd_vlan_pvlan_promis_port_del(netif_index, pvid);
	}
	else if (pvlan_port_type == SWITCH_PORT_PRIVLAN_ISOLATED)
	{
		npd_vlan_pvlan_isolate_port_del(netif_index, pvid);
	}
    return 0;
}


DBusMessage * npd_dbus_pvlan_primary_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	DBusError err;
	
    unsigned int primary_vlan= 0;
    unsigned int enable = 0;
	unsigned int ret  = 0;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &primary_vlan,
								DBUS_TYPE_UINT32, &enable,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (enable)
	{
		ret = npd_vlan_pvlan_primary_add(primary_vlan);
	}
	else
	{
		ret = npd_vlan_pvlan_primary_del(primary_vlan);
	}
	

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}




DBusMessage * npd_dbus_pvlan_primary_associate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	
	DBusError err;

	unsigned int ret = 0;
	unsigned int primary_vlan = 0;
	unsigned int isolate_vlan = 0;
	unsigned int assoc = 0;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &primary_vlan,
								DBUS_TYPE_UINT32,&isolate_vlan,
								DBUS_TYPE_UINT32,&assoc,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (assoc)
	{
		ret = npd_vlan_pvlan_isolate_assoc_add(primary_vlan, isolate_vlan);
	}
	else
	{
		ret = npd_vlan_pvlan_isolate_assoc_del(primary_vlan, isolate_vlan);
	}
	
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	return reply;
	
}




DBusMessage * npd_dbus_pvlan_promis_port_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    unsigned int primary_vlan;
	unsigned int netif_index;
	unsigned int is_add;	
	unsigned int ret  = 0;
	
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&primary_vlan,
								DBUS_TYPE_UINT32,&is_add,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (is_add)
	{
		ret = npd_vlan_pvlan_promis_port_add(netif_index, primary_vlan);
	}
	else
	{
		ret = npd_vlan_pvlan_promis_port_del(netif_index, primary_vlan);
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	return reply;
	
}



DBusMessage * npd_dbus_pvlan_isolate_port_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;

	unsigned int is_add;
	unsigned int isolate_vlan;
	unsigned int netif_index = 0;	
	unsigned int ret  = 0;
	
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_UINT32,&isolate_vlan,
								DBUS_TYPE_UINT32,&is_add,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (is_add)
	{
		ret = npd_vlan_pvlan_isolate_port_add(netif_index, isolate_vlan);
	}
	else
	{
		ret = npd_vlan_pvlan_isolate_port_del(netif_index, isolate_vlan);
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	return reply;
	
}


DBusMessage * npd_dbus_pvlan_get_next_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	pvlan_isolate_t pvlan_isolate = {0};
    unsigned int isolate_vlan;

	int ret  = 0;
	
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&isolate_vlan,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if (isolate_vlan == 0)
	{
		ret = dbtable_sequence_traverse_next(
			pvlan_isolate_vid_sequence, -1, (void * )&pvlan_isolate);		
	}
	else
	{
		pvlan_isolate.vid = isolate_vlan;
		ret = dbtable_sequence_traverse_next(
			pvlan_isolate_vid_sequence, isolate_vlan, (void * )&pvlan_isolate);				
	}
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &pvlan_isolate.vid);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &pvlan_isolate.primary_vid);
	
	return reply;
	
}

DBusMessage * npd_dbus_pvlan_get_member(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	pvlan_isolate_t pvlan_isolate = {0};
	unsigned int isolate_vlan;
	
	int ret  = 0;
	int i = 0;
	npd_pbmp_t      mbr,pbmp;	
	unsigned int array_index;
	unsigned int netif_index;
	unsigned int type;
	unsigned int trunk_id;
	
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&isolate_vlan,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	memset(&mbr ,0 ,sizeof(npd_pbmp_t));
	memset(&pbmp ,0 ,sizeof(npd_pbmp_t));	

	pvlan_isolate.vid = isolate_vlan;
	ret = dbtable_sequence_search(
		pvlan_isolate_vid_sequence, isolate_vlan, (void * )&pvlan_isolate);	
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}
	

	NPD_PBMP_ASSIGN(pbmp, pvlan_isolate.ports);
	NPD_PBMP_ITER(pbmp, array_index)
	{
		netif_index = netif_array_index_to_ifindex(array_index);
		if (type != npd_netif_type_get(netif_index))
			continue;

        if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
        {
            trunk_id = npd_netif_trunk_get_tid(netif_index);
            NPD_PBMP_PORT_ADD(mbr, trunk_id);             
        }
		else if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
		{
			NPD_PBMP_PORT_ADD(mbr, array_index);
		}
	}		

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
    for(i = 0; i < (sizeof(npd_pbmp_t)/4); i++)
    {
	    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 (unsigned int *)(&mbr) + i );
    }	
	
	return reply;
	
}

DBusMessage * npd_dbus_pvlan_get_next_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	pvlan_primary_t pvlan_primary = {0};
    unsigned int primary_vlan;
	unsigned int ret  = 0;
	int i = 0;
	npd_vbmp_t      vmbr;	
	
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&primary_vlan,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	

	if (primary_vlan == 0)
	{
		ret = dbtable_sequence_traverse_next(
			pvlan_primary_vid_sequence, -1, (void * )&pvlan_primary);		
	}
	else
	{
		pvlan_primary.vid = primary_vlan;
		ret = dbtable_sequence_traverse_next(
			pvlan_primary_vid_sequence, primary_vlan, (void * )&pvlan_primary);				
	}
	
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
	}
	NPD_VBMP_ASSIGN(vmbr, pvlan_primary.secondary_vlans);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &pvlan_primary.vid);
	
    for(i = 0; i < (sizeof(npd_vbmp_t)/4); i++)
    {
	    dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 (unsigned int *)(&vmbr) + i );
    }	
		
	return reply;	
}

#endif

DBusMessage * npd_dbus_vlan_config_protovlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    protobase_vlan_t protovlan;
    proto_vlan_port_t proto_vlanport;
    protobase_vlan_t item;
    unsigned char isAdd;
    unsigned int index;
	int ret  = 0;
	DBusError err;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_UINT32,&protovlan.ether_frame,
							 	DBUS_TYPE_UINT32,&index,
							 	DBUS_TYPE_UINT16,&protovlan.eth_type,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_key_database_lock();
	
	/* resoleve protovlan be covered */
	protovlan.proto_group_index = index;
    if(isAdd)
    {
        ret = dbtable_array_get(proto_vlan_array, index, &item);
        if(0 == ret)
        {
            ret = VLAN_RETURN_CODE_VLAN_PROTO_EXISTS;
            goto answer;
        }

        ret = dbtable_array_insert_byid(proto_vlan_array, index, &protovlan);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_ERR_GENERAL;
            goto answer;
        }
    }
    else
    {

        /*delete all protocol vlan with group index "index" on port*/
        while(1)
        {
            proto_vlanport.proto_group_index = index;
			ret = dbtable_hash_head(proto_vlanport_hash, &proto_vlanport,  
				&proto_vlanport, vlan_assoc_protocol_group_show_filter);
			if (0 != ret)
				break;
			ret = dbtable_hash_delete(proto_vlanport_hash, &proto_vlanport,
                                      &proto_vlanport);
        }
        
        ret = dbtable_array_get(proto_vlan_array, index, &item);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_VLAN_PROTO_NOEXISTS;
            goto answer;
        }

        ret = dbtable_array_delete(proto_vlan_array, index, &protovlan);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_ERR_GENERAL;
            goto answer;
        }
    }

    
answer:
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
    
    
}

DBusMessage * npd_dbus_vlan_config_protovlan_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    proto_vlan_port_t protovlan;
    unsigned int isAdd;
    unsigned int index;
    unsigned int netif_index;
    unsigned short vlanId;
	int ret  = 0;
	DBusError err;
	switch_port_db_t switch_port;
    protobase_vlan_t item;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 	DBUS_TYPE_UINT32,&isAdd,
							 	DBUS_TYPE_UINT32,&netif_index,
							 	DBUS_TYPE_UINT32,&index,
							 	DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	memset(&switch_port, 0, sizeof(switch_port_db_t));
    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
		goto answer;
    }
	
    protovlan.netif_index = netif_index;
    protovlan.proto_group_index = index;
    protovlan.vid = vlanId;

    if(FALSE == npd_check_user_vlan_exist(vlanId))
    {
        ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
        goto answer;
    }
	
	ret = dbtable_array_get(proto_vlan_array, index, &item);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_VLAN_PROTO_NOEXISTS;
        goto answer;
    }
	
    ret = dbtable_hash_search(proto_vlanport_hash, &protovlan, NULL, &protovlan);
    
    if(isAdd)
    {
        if(0 == ret)
        {
            ret = VLAN_RETURN_CODE_VLAN_PROTO_EXISTS;
            goto answer;
        }

        ret = dbtable_hash_insert(proto_vlanport_hash, &protovlan);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_ERR_GENERAL;
            goto answer;
        }
    }
    else
    {
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_VLAN_PROTO_NOEXISTS;
            goto answer;
        }

		if (protovlan.vid != vlanId)
		{
            ret = VLAN_RETURN_CODE_VLAN_PROTO_NOMATCH;
            goto answer;
		}
		
        ret = dbtable_hash_delete(proto_vlanport_hash, &protovlan, &protovlan);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_ERR_GENERAL;
            goto answer;
        }
    }

    
answer:
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;


}

/*npd_dbus_vlan_show_one_port_pvid*/

/*original*/
DBusMessage * npd_dbus_vlan_show_one_port_pvid(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	
	DBusMessage* reply;
	DBusMessageIter		iter;
	unsigned int	ret = VLAN_RETURN_CODE_ERR_NONE;
	unsigned short pvid = 0;
	unsigned int	eth_g_index = 0;
	DBusError err;


	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = npd_vlan_port_pvid_get(eth_g_index, &pvid);
	if(VLAN_RETURN_CODE_ERR_NONE == ret) {
		ret = VLAN_RETURN_CODE_ERR_NONE;	
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16, 
									 &pvid);
	
	return reply;
}


DBusMessage * npd_dbus_vlan_show_egress_filter(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage* 	reply = NULL;
	DBusMessageIter	iter = {0};
	DBusError 		err;
	unsigned int	Isable;
	syslog_ax_acl_dbg("show vlan egress filter\n");

	dbus_error_init(&err);
	if(NPD_TRUE == g_vlan_egress_filter) {
	   	syslog_ax_acl_dbg("vlan egress filter is enabled");		   
	}
	else {
		syslog_ax_acl_dbg("vlan egress filter is disabled");	
	}
	Isable = g_vlan_egress_filter;
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&Isable);
	return reply;	
}


DBusMessage * npd_dbus_vlan_config_vlan_egress_filter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int isable;
	int ret  = 0;
	DBusError err;
	unsigned char dev = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&isable,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = nam_config_vlan_egress_filter(dev,isable);
	if(NPD_SUCCESS != ret) {
		 syslog_ax_vlan_err("nam_config_vlan_egress_filter() fail");
	}
	else {
		g_vlan_egress_filter = isable;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_vlan_filter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int filterType = 0xff,enDis = NPD_FALSE;
	int ret  = 0;
	unsigned short vlanId = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_UINT32,&filterType,
								DBUS_TYPE_UINT32,&enDis,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_vlan_dbg("To %s vlan filter with type %d.\n", (1 == enDis)?"enalbe":"disable", filterType);
	ret = nam_config_vlan_filter(vlanId,filterType,enDis);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

#ifdef HAVE_QINQ
unsigned int vlan_xlate_db_exist_filter(void * in1, void * in2)
{
	vlan_xlate_db_entry_t *entry = (vlan_xlate_db_entry_t *)in1; 
	vlan_xlate_db_entry_t *input = (vlan_xlate_db_entry_t *)in2;
		
    if((entry->netif_index == input->netif_index)
        && (entry->xlate_type == input->xlate_type)
        && (entry->egress_inner_vid == input->egress_inner_vid)
        && (entry->egress_outer_vid == input->egress_outer_vid)
        && (entry->priority == input->priority)
        )
        
    {
        if(((entry->ingress_inner_start_vid >= input->ingress_inner_start_vid)
            &&(entry->ingress_inner_start_vid <= input->ingress_inner_start_vid + entry->ingress_inner_vid_num -1))
            ||((entry->ingress_inner_start_vid <= input->ingress_inner_start_vid)
                &&(entry->ingress_inner_start_vid + entry->ingress_inner_vid_num -1 
                                      >= input->ingress_inner_start_vid)))
        {
            if(((entry->ingress_outer_start_vid >= input->ingress_outer_start_vid)
                &&(entry->ingress_outer_start_vid <= input->ingress_outer_start_vid + entry->ingress_outer_vid_num -1))
                ||((entry->ingress_outer_start_vid <= input->ingress_outer_start_vid)
                    &&(entry->ingress_outer_start_vid + entry->ingress_outer_vid_num - 1
                                      >= input->ingress_outer_start_vid)))
                return TRUE;
        }
    }
    return FALSE;
}



unsigned int npd_vlan_xlate_inner_range_show_filter(void * in, void * out)
{
    vlan_xlate_db_entry_t *entry1 = (vlan_xlate_db_entry_t*)in;
    vlan_xlate_db_entry_t *entry2 = (vlan_xlate_db_entry_t*)out;

	if (entry1->netif_index != entry2->netif_index)
	{
		return FALSE;
	}
	
	if (entry1->ingress_inner_vid_num > 1 && 
		entry2->ingress_inner_vid_num > 1)
	{
		return TRUE;
	}

	return FALSE;
}


unsigned int npd_vlan_xlate_outer_range_show_filter(void * in, void * out)
{
    vlan_xlate_db_entry_t *entry1 = (vlan_xlate_db_entry_t*)in;
    vlan_xlate_db_entry_t *entry2 = (vlan_xlate_db_entry_t*)out;

	if (entry1->netif_index != entry2->netif_index)
	{
		return FALSE;
	}

	if (entry1->ingress_outer_vid_num > 1 && 
		entry2->ingress_outer_vid_num > 1)
	{
		return TRUE;
	}

	return FALSE;
}

#define XLATE_VLAN_RANGE_MAX 	8

int npd_vlan_xlate_range_check(vlan_xlate_db_entry_t * entry)
{
	vlan_xlate_db_entry_t entry_out ;
	vlan_xlate_db_entry_t entry_find ;
	
	int ret = VLAN_RETURN_CODE_ERR_NONE;
	int op_ret = 0;
	int start_vid = 0;
	int min_vid = 0;
	int max_vid = 0;
	int range_count = 0;
	
	if(entry->ingress_inner_vid_num <= 1 &&
		entry->ingress_outer_vid_num <= 1)
	{
		return VLAN_RETURN_CODE_ERR_NONE;
	}
	
	memset(&entry_find, 0, sizeof(vlan_xlate_db_entry_t));
	memset(&entry_out, 0, sizeof(vlan_xlate_db_entry_t));
	
	/* check outer range */
	if (entry->ingress_outer_vid_num > 1)
	{
		start_vid = entry->ingress_outer_start_vid;
		entry_find.netif_index = entry->netif_index;
		entry_find.ingress_outer_vid_num = entry->ingress_outer_vid_num;
		
		op_ret = dbtable_hash_head(vlan_xlate_table_hash, 
			&entry_find, &entry_out, npd_vlan_xlate_outer_range_show_filter);

		while (op_ret == 0)
		{
			range_count++;
			min_vid = entry_out.ingress_outer_start_vid;
			max_vid = entry_out.ingress_outer_start_vid 
				+ entry_out.ingress_outer_vid_num - 1;

			if (min_vid <= start_vid && start_vid <= max_vid)
			{
				ret = VLAN_RETURN_CODE_XLATE_OUTER_RANGE_OVERLAP;
				goto retcode;
			}
			memcpy(&entry_find, &entry_out, sizeof(vlan_xlate_db_entry_t));
			op_ret = dbtable_hash_next(vlan_xlate_table_hash, 
				&entry_find, &entry_out, npd_vlan_xlate_outer_range_show_filter);
		}
		
	}
	if (range_count >= XLATE_VLAN_RANGE_MAX)
	{
		ret = VLAN_RETURN_CODE_XLATE_OUTER_RANGE_FULL;
		goto retcode;
	}
	range_count = 0;

	/* check inner range */
	
	memset(&entry_find, 0, sizeof(vlan_xlate_db_entry_t));
	memset(&entry_out, 0, sizeof(vlan_xlate_db_entry_t));
	
	if (entry->ingress_inner_vid_num > 1)
	{
		start_vid = entry->ingress_inner_start_vid;
		entry_find.netif_index = entry->netif_index;
		entry_find.ingress_inner_vid_num = entry->ingress_inner_vid_num;
		
		op_ret = dbtable_hash_head(vlan_xlate_table_hash, 
			&entry_find, &entry_out, npd_vlan_xlate_inner_range_show_filter);

		while (op_ret == 0)
		{
			range_count++;
			min_vid = entry_out.ingress_inner_start_vid;
			max_vid = entry_out.ingress_inner_start_vid 
				+ entry_out.ingress_inner_vid_num - 1;

			if (min_vid <= start_vid && start_vid <= max_vid)
			{
				ret = VLAN_RETURN_CODE_XLATE_INNER_RANGE_OVERLAP;
				goto retcode;
			}
			memcpy(&entry_find, &entry_out, sizeof(vlan_xlate_db_entry_t));
			op_ret = dbtable_hash_next(vlan_xlate_table_hash, 
				&entry_find, &entry_out, npd_vlan_xlate_inner_range_show_filter);	
		}
		
	}

	if (range_count >= XLATE_VLAN_RANGE_MAX)
	{
		ret = VLAN_RETURN_CODE_XLATE_INNER_RANGE_FULL;
		goto retcode;
	}
		
retcode:
	return ret;

}
	
DBusMessage * npd_dbus_vlan_config_xlate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isAdd;
    int isEgress;
    vlan_xlate_db_entry_t entry = {0};
    switch_port_db_t switch_port;
        
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isAdd,
								DBUS_TYPE_UINT32,&isEgress,
                                DBUS_TYPE_UINT32,&entry.netif_index,
								DBUS_TYPE_UINT32,&entry.xlate_type,
								DBUS_TYPE_UINT32,&entry.ingress_outer_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_outer_vid_num,
								DBUS_TYPE_UINT32,&entry.ingress_inner_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_inner_vid_num,
								DBUS_TYPE_UINT32,&entry.priority,
								DBUS_TYPE_UINT32,&entry.egress_outer_vid,
								DBUS_TYPE_UINT32,&entry.egress_inner_vid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch_port.global_port_ifindex = entry.netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto retcode;
        }
    }

    if(isAdd)
    {
		
		ret = npd_vlan_xlate_range_check(&entry);
		if (VLAN_RETURN_CODE_ERR_NONE!= ret)
		{
			goto retcode;
		}
		
        ret = dbtable_hash_traversal_key(vlan_xlate_table_hash,  0, &entry, 
            &vlan_xlate_db_exist_filter, NULL);
        if(0 == ret)
        {
            ret = dbtable_hash_insert(vlan_xlate_table_hash, &entry);
        }
        else
            ret = VLAN_RETURN_CODE_XLATE_CONFILCT;
    }
    else
    {
        ret = dbtable_hash_traversal_key(vlan_xlate_table_hash,  0, &entry, 
            &vlan_xlate_db_exist_filter, NULL);
		if (ret > 0)
		{
			ret = dbtable_hash_delete(vlan_xlate_table_hash, &entry, &entry);
		}
		else
			ret = VLAN_RETURN_CODE_XLATE_NO_EXIST;
                    
    }
retcode:    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

unsigned int vlan_xlate_netif_show_filter(void *in, void* out)
{
    vlan_xlate_db_entry_t *entry1 = (vlan_xlate_db_entry_t*)in;
    vlan_xlate_db_entry_t *entry2 = (vlan_xlate_db_entry_t*)out;

    if(entry1->netif_index == entry2->netif_index)
        return TRUE;
    else
        return FALSE;
}

DBusMessage * npd_dbus_vlan_show_xlate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	int ret  = 0;
    vlan_xlate_db_entry_t entry = {0};
    switch_port_db_t switch_port;
        
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&entry.netif_index,
								DBUS_TYPE_UINT32,&entry.xlate_type,
								DBUS_TYPE_UINT32,&entry.ingress_outer_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_outer_vid_num,
								DBUS_TYPE_UINT32,&entry.ingress_inner_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_inner_vid_num,	
								DBUS_TYPE_UINT32,&entry.priority,
								DBUS_TYPE_UINT32,&entry.egress_outer_vid,
								DBUS_TYPE_UINT32,&entry.egress_inner_vid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch_port.global_port_ifindex = entry.netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto retcode;
        }
    }

    {
        ret = dbtable_hash_next(vlan_xlate_table_netif_hash,  &entry, &entry, 
            &vlan_xlate_netif_show_filter);
        if(0 != ret)
        {
            ret = VLAN_RETURN_CODE_XLATE_NOMORE;
        }
    }
retcode:    
	reply = dbus_message_new_method_return(msg);

    dbus_message_append_args(reply, 
                                DBUS_TYPE_UINT32,&ret,
                                DBUS_TYPE_UINT32,&entry.netif_index,
								DBUS_TYPE_UINT32,&entry.xlate_type,
								DBUS_TYPE_UINT32,&entry.ingress_outer_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_outer_vid_num,
								DBUS_TYPE_UINT32,&entry.ingress_inner_start_vid,
								DBUS_TYPE_UINT32,&entry.ingress_outer_vid_num,
								DBUS_TYPE_UINT32,&entry.priority,
								DBUS_TYPE_UINT32,&entry.egress_outer_vid,
								DBUS_TYPE_UINT32,&entry.egress_inner_vid,
								DBUS_TYPE_INVALID);
	
	return reply;
	
}

DBusMessage * npd_dbus_netif_xlate_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    vlan_xlate_db_entry_t entry = {0};
    switch_port_db_t switch_port;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
    int len;
    char tempBuf[128] = {0};
    char outerstr[64] = {0};
    char innerstr[64] = {0};
    char eouterstr[64] = {0};
    char einnerstr[64] = {0};
    int first_flag = TRUE;

	DBusError err;

	syslog_ax_vlan_dbg("Entering show vlan xlate ...\n");
	showStr = (char*)malloc(NPD_ETHPORT_SHOWRUN_CFG_SIZE);
	if(NULL == showStr)
	{
		syslog_ax_vlan_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,NPD_ETHPORT_SHOWRUN_CFG_SIZE);
	cursor = showStr;
    totalLen = NPD_ETHPORT_SHOWRUN_CFG_SIZE;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&entry.netif_index,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(showStr);
		return NULL;
	}
    switch_port.global_port_ifindex = entry.netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto retcode;
        }
    }

    {
        char name[20] = {0};
        npd_netif_index_to_user_fullname(entry.netif_index, name);

        while(1)
        {
            ret = dbtable_hash_next(vlan_xlate_table_netif_hash,  &entry, &entry, 
                &vlan_xlate_netif_show_filter);
            if(0 != ret)
            {
                break;
            }
            if(first_flag)
            {
                len += sprintf(tempBuf, "interface %s\n", name);
                if(len < totalLen)
                    strcat(showStr, tempBuf);
                first_flag = FALSE;
            }

            if(entry.ingress_outer_start_vid != 0)
                sprintf(outerstr, "outer vlan %d", entry.ingress_outer_start_vid);
            if(entry.ingress_outer_vid_num != 0)
                sprintf(outerstr, "%s-%d", outerstr, 
                          entry.ingress_outer_start_vid+entry.ingress_outer_vid_num);
            if(entry.ingress_inner_start_vid != 0)
                sprintf(innerstr, "inner vlan %d", entry.ingress_inner_start_vid);
            if(entry.ingress_inner_vid_num != 0)
                sprintf(innerstr, "%s-%d", innerstr, 
                          entry.ingress_inner_start_vid+entry.ingress_inner_vid_num);
            if(entry.egress_outer_vid)
                sprintf(eouterstr, "outer vlan %d", entry.egress_outer_vid);
            if(entry.egress_inner_vid)
                sprintf(einnerstr, "inner vlan %d", entry.egress_inner_vid);
            switch(entry.xlate_type)
            {
                case XLATE_POP_INNER:
                    len += sprintf(tempBuf, "switchport egress %s %s pop inner\n", outerstr, innerstr);
                    break;
                case XLATE_POP_OUTER:
                    len += sprintf(tempBuf, "switchport egress %s %s pop outer\n", outerstr, innerstr);
                    break;
                case XLATE_POP_BOTH:
                    len += sprintf(tempBuf, "switchport egress %s %s pop 2\n", outerstr, innerstr);
                    break;
                case XLATE_PUSH_OUTER:
                    len += sprintf(tempBuf, "switchport ingress %s %s push %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_PUSH_INNER:
                    len += sprintf(tempBuf, "switchport ingress %s %s push %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_PUSH_BOTH:
                    len += sprintf(tempBuf, "switchport ingress push %s %s\n", eouterstr, einnerstr);
                    break;
                case XLATE_REWRITE_OUTER:
                    len += sprintf(tempBuf, "switchport ingress %s %s translate %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_REWRITE_INNER:
                    len += sprintf(tempBuf, "switchport ingress %s %s translate %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_REWRITE_BOTH:
                    len += sprintf(tempBuf, "switchport ingress %s %s translate %s %s\n", outerstr, innerstr, eouterstr, einnerstr);
                    break;
                case XLATE_E_REWRITE_OUTER:
                    len += sprintf(tempBuf, "switchport egress %s %s translate %s\n", outerstr, innerstr, eouterstr);
                    break;
                case XLATE_E_REWRITE_INNER:
                    len += sprintf(tempBuf, "switchport egress %s %s translate %s\n", outerstr, innerstr, einnerstr);
                    break;
                case XLATE_E_REWRITE_BOTH:
                    len += sprintf(tempBuf, "switchport egress %s %s translate %s %s\n", outerstr, innerstr, eouterstr, einnerstr);
                    break;
                default :
                    break;
                   
            }
            
            if(len < totalLen)
                strcat(showStr, tempBuf);
            
        }
        if(FALSE == first_flag)
        {
            len += sprintf(tempBuf, "exit\n");
            if(len < totalLen)
                strcat(showStr, tempBuf);
        }
    }
retcode:    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
	
}

int npd_netif_free_all_xlate(unsigned int netif_index)
{
    vlan_xlate_db_entry_t entry = {0};
    switch_port_db_t switch_port;
    int ret;

    entry.netif_index = netif_index;
    while(1)
    {
        ret = dbtable_hash_next(vlan_xlate_table_netif_hash,  &entry, &entry, 
            &vlan_xlate_netif_show_filter);
        if(0 != ret)
        {
            break;
        }
        dbtable_hash_delete(vlan_xlate_table_hash, &entry, &entry);
    }
    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
		npd_key_database_unlock();
        return NPD_SUCCESS;
    }
    switch_port.access_qinq = FALSE;
    switch_port.qinq_drop_miss = FALSE;
    ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
           NULL, &switch_port);
	npd_key_database_unlock();
    if(0 != ret)
        ret = VLAN_RETURN_CODE_ERR_HW;
    return ret;
}

int npd_vlan_qinq_netif_config_check(unsigned int netif_index)
{
	int op_ret = 0;  
    vlan_eline_db_entry_t entry_out = {0};
	/* xlate  */
	{
		vlan_xlate_db_entry_t entry = {0};
	    entry.netif_index = netif_index;
	    op_ret = dbtable_hash_head_key(vlan_xlate_table_netif_hash,  &entry, &entry, 
	        &vlan_xlate_netif_show_filter);
		if (op_ret == 0)
		{
			return op_ret;
		}
	}
	/* eline */
	{
        vlan_eline_db_entry_t entry = {0};
		entry.netif_index_first = netif_index;
		entry.netif_index_second = netif_index;
        op_ret = dbtable_hash_head(vlan_eline_vlan_hash,  &entry, &entry_out, 
            &npd_vlan_eline_netif_filter);
		if (op_ret == 0)
		{
			return op_ret;	
		}		
	}
	return op_ret;
}

DBusMessage * npd_dbus_vlan_config_netif_qinq(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    unsigned int isEnable;
    unsigned int netif_index;
    switch_port_db_t switch_port;
    
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if (!isEnable)
	{
		ret = npd_vlan_qinq_netif_config_check(netif_index);
		if (ret == 0) /* 0 indicate that there other configuration need qinq */
		{
			ret = VLAN_RETURN_CODE_PORT_QINQ_CONFIG_EXIST;
			goto retcode;
		}		
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        switch_port.access_qinq = isEnable;
        ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
               NULL, &switch_port);
        if(0 != ret)
            ret = VLAN_RETURN_CODE_ERR_HW;
    }
	npd_key_database_unlock();

retcode:    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}
#endif

DBusMessage * npd_dbus_vlan_config_netif_subnet(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isEnable;
    unsigned int netif_index;
    int preferred;
    switch_port_db_t switch_port;
    
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&preferred,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        switch_port.subnet_vlan_flag = isEnable;
        switch_port.prefer_subnet = preferred;
        ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
               NULL, &switch_port);
        if(0 != ret)
            ret = VLAN_RETURN_CODE_ERR_HW;
    }
    npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_netif_mac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isEnable;
    unsigned int netif_index;
    int preferred;
    switch_port_db_t switch_port;
    
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config MAC-BASED vlan ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&preferred,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        switch_port.mac_vlan_flag = isEnable;
        switch_port.prefer_subnet = !preferred;
        ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
               NULL, &switch_port);
        if(0 != ret)
            ret = VLAN_RETURN_CODE_ERR_HW;
    }
    npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}
#ifdef HAVE_QINQ
DBusMessage * npd_dbus_vlan_config_netif_qinq_miss(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isEnable;
    unsigned int netif_index;
    switch_port_db_t switch_port;
    
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
        else
        {
            switch_port.qinq_drop_miss = isEnable;
            ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
                   NULL, &switch_port);
            if(0 != ret)
                ret = VLAN_RETURN_CODE_ERR_HW;
        }
    }
    npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_netif_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isSet;
    unsigned int netif_index;
    unsigned short tpid;
    switch_port_db_t switch_port;
    
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isSet,
								DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_UINT16,&tpid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
        else
        {
            if(isSet)
            {
                if(switch_port.tpid != 0x8100)
                {
                    ret = VLAN_RETURN_CODE_PORT_TPID_CONFLICT;
                    goto retCode;
                }
                else
                    switch_port.tpid = tpid;
            }
            else
            {
                if(!isSet)
                {
                    if(switch_port.tpid != tpid)
                    {
                        ret = VLAN_RETURN_CODE_PORT_TPID_CONFLICT;
                        goto retCode;
                    }
                    else
                        switch_port.tpid = 0x8100;
                }
            }
            
            //switch_port.tpid = tpid;
            ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
                   NULL, &switch_port);
            if(0 != ret)
                ret = VLAN_RETURN_CODE_ERR_HW;
        }
    }
retCode:    
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_netif_inner_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    int isSet;
    unsigned int netif_index;
    unsigned short inner_tpid;
    switch_port_db_t switch_port;
    
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isSet,
								DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_UINT16,&inner_tpid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    switch_port.global_port_ifindex = netif_index;
	npd_key_database_lock();
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
        else
        {
            if(isSet)
            {
                if(switch_port.inner_tpid != 0x8100)
                {
                    ret = VLAN_RETURN_CODE_PORT_TPID_CONFLICT;
                    goto retCode;
                }
                else
                    switch_port.inner_tpid = inner_tpid;
            }
            else
            {
                if(!isSet)
                {
                    if(switch_port.inner_tpid != inner_tpid)
                    {
                        ret = VLAN_RETURN_CODE_PORT_TPID_CONFLICT;
                        goto retCode;
                    }
                    else
                        switch_port.inner_tpid = 0x8100;
                }
            }
            
            //switch_port.tpid = tpid;
            ret = dbtable_array_update(switch_ports, switch_port.switch_port_index,
                   NULL, &switch_port);
            if(0 != ret)
                ret = VLAN_RETURN_CODE_ERR_HW;
        }
    }
retCode:    
	npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_qinq_global_tpid(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    unsigned short tpid;
	unsigned int isSet;
	unsigned int isOuter;
    struct npd_vlan_qinq_s npd_vlan_qinq_set;
	    
	DBusError err;

	syslog_ax_vlan_dbg("Entering config qinq ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isSet,
                                DBUS_TYPE_UINT32,&isOuter,                              
								DBUS_TYPE_UINT16,&tpid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = dbtable_array_get(npd_vlan_qinq_index, vlan_qinq_global_no, &npd_vlan_qinq_set);

	/* config qinq inner tpid */
	if (!isSet)
	{
		tpid = 0x8100;
	}

	if (isOuter)
	{
		npd_vlan_qinq_set.tpid= tpid;
		g_tpid = tpid;
	}
	else
	{
		npd_vlan_qinq_set.inner_tpid = tpid;
		g_inner_tpid = tpid;
	}	

	
	ret = dbtable_array_update(npd_vlan_qinq_index, vlan_qinq_global_no, &npd_vlan_qinq_set, &npd_vlan_qinq_set);
	if (isOuter)
	{
		int i;
		int switch_port_count = dbtable_array_totalcount(switch_ports);
		int ret, all_ret = 0;
		switch_port_db_t switch_port = {0};

	    npd_key_database_lock();
		
		for (i = 0; i < switch_port_count; i++)
		{
			ret = dbtable_array_get(switch_ports, i, &switch_port);
			if (ret != 0)
			{
				all_ret = VLAN_RETURN_CODE_ERR_HW;
				continue;
			}
			switch_port.tpid = g_tpid;
			ret = dbtable_array_update(switch_ports, i, NULL, &switch_port);
			if (ret != 0)
			{
				all_ret = VLAN_RETURN_CODE_ERR_HW;
				continue;
			}
		}	
		npd_key_database_unlock();
		
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

#endif



DBusMessage * npd_dbus_vlan_egress_filter_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	char *showStr = NULL, *cursor = NULL;
	int totalLen = 0;

	showStr = (char*)malloc(NPD_VLAN_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		 syslog_ax_vlan_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,NPD_VLAN_RUNNING_CFG_MEM);
	
	cursor = showStr;

	 if(g_vlan_egress_filter == NPD_FALSE){
			totalLen += sprintf(cursor,"config vlan egress-filter disable\n");
			cursor = showStr + totalLen;			
	 }

	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr); 
	free(showStr);
	showStr = NULL;
	return reply;
}

#ifdef HAVE_QINQ
unsigned int npd_vlan_eline_outervlan_filter(void *in, void* out)
{
    vlan_eline_db_entry_t *entry1 = (vlan_eline_db_entry_t*)in;
    vlan_eline_db_entry_t *entry2 = (vlan_eline_db_entry_t*)out;

    if(entry1->outer_vid== entry2->outer_vid)
        return TRUE;
    else
        return FALSE;
}

unsigned int npd_vlan_eline_netif_filter(void *in, void* out)
{
    vlan_eline_db_entry_t *entry1 = (vlan_eline_db_entry_t*)in;
    vlan_eline_db_entry_t *entry2 = (vlan_eline_db_entry_t*)out;

	if (entry1->netif_index_first == entry2->netif_index_first ||
		entry1->netif_index_first == entry2->netif_index_second ||
		entry1->netif_index_second == entry2->netif_index_first ||
		entry1->netif_index_second == entry2->netif_index_second )
	{
		return TRUE;
	}
	else
		return FALSE;
}

unsigned int npd_vlan_eline_netif_outer_filter(void *in, void* out)
{
    vlan_eline_db_entry_t *entry1 = (vlan_eline_db_entry_t*)in;
    vlan_eline_db_entry_t *entry2 = (vlan_eline_db_entry_t*)out;

	if (entry1->outer_vid== entry2->outer_vid)
	{		
		if (entry1->netif_index_first == entry2->netif_index_first ||
			entry1->netif_index_first == entry2->netif_index_second ||
			entry1->netif_index_second == entry2->netif_index_first ||
			entry1->netif_index_second == entry2->netif_index_second )
		{
			
			return TRUE;
		}
		else
			return FALSE;		
	}
	else
		return FALSE;
}

int npd_vlan_forward_mode_set(
	unsigned int vlan_id,
	unsigned int isAdd,
	vlan_eline_db_entry_t * entry
	)
{
	int ret  = VLAN_RETURN_CODE_ERR_NONE;
    struct vlan_s * vlan = NULL;
	vlan_eline_db_entry_t* entry_out = NULL;
	
    vlan = npd_find_vlan_by_vid(vlan_id);
    if(NULL == vlan)
    {
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		goto retcode;
    }        
	entry_out = malloc(sizeof(vlan_eline_db_entry_t));
	if (entry_out == NULL)
	{
		ret = VLAN_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	if (isAdd)
	{
		if (vlan->forward_mode == FORWARD_SINGLE_XC)
		{
			ret = VLAN_RETURN_CODE_ELINE_VLAN_CONFILCT;
			goto retcode;
		}

		if (vlan->forward_mode == FORWARD_DOUBLE_XC 
				&& entry->eline_type == VLAN_ELINE_TYPE_SINGLE )
		{
			ret = VLAN_RETURN_CODE_ELINE_CONFILCT;
			goto retcode;
		}

		if (entry->eline_type == VLAN_ELINE_TYPE_SINGLE)
		{
			vlan->forward_mode = FORWARD_SINGLE_XC;
		}
		else
		{
			vlan->forward_mode = FORWARD_DOUBLE_XC;
		}
		ret = dbtable_sequence_update(g_vlans, vlan->vid, NULL, vlan);
		if (ret != 0)
		{
			ret = VLAN_RETURN_CODE_ERR_HW;
			goto retcode;
		}

	}
	else
	{
		if (vlan->forward_mode == FORWARD_DOUBLE_XC)
		{
			ret = dbtable_hash_head(vlan_eline_vlan_hash, entry, entry_out, 
					npd_vlan_eline_outervlan_filter);
			if (ret != 0 || entry_out->inner_vid == entry->inner_vid)
			{
				vlan->forward_mode = FORWARD_BRIDGING;
			}
		}
		else
		{
			vlan->forward_mode = FORWARD_BRIDGING;
		}
		
		ret = dbtable_sequence_update(g_vlans, vlan->vid, NULL, vlan);
		if (ret != 0)
		{
			goto retcode;
		}	
	}
	
retcode:
	if(vlan)
	    free(vlan);
	if(entry_out)
		free(entry_out);

	return ret;	
}

DBusMessage * npd_dbus_vlan_config_eline(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
    unsigned int isAdd;
    vlan_eline_db_entry_t entry = {0};
	int ret  = 0;
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config vlan xlate ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isAdd,
                                DBUS_TYPE_UINT32,&entry.eline_id,
                                DBUS_TYPE_UINT32,&entry.eline_type,
                                DBUS_TYPE_UINT32,&entry.outer_vid,
                                DBUS_TYPE_UINT32,&entry.inner_vid,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = npd_check_vlan_exist(entry.outer_vid);
	if (ret == FALSE)
	{
		ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
		goto retcode;
	}
	
	ret = dbtable_array_get(vlan_eline_array, entry.eline_id, &entry);
	if(isAdd)
    {
		if (ret != 0)
		{
			ret = npd_vlan_if_has_member(entry.outer_vid);
			if (ret == NPD_TRUE)
			{
				ret = VLAN_RETURN_CODE_PORT_EXISTS;
				goto retcode;
			}

			ret = npd_vlan_forward_mode_set(entry.outer_vid, isAdd, &entry);
			if (ret != 0)
			{
				goto retcode;
			}
			ret = dbtable_array_insert_byid(vlan_eline_array, entry.eline_id, &entry);
		}	
        else
            ret = VLAN_RETURN_CODE_ELINE_CONFILCT;
    }
    else
    {
		if (ret == 0)
		{	
			ret = dbtable_array_delete(vlan_eline_array, entry.eline_id, &entry);
			if (ret != 0)
			{
				ret = VLAN_RETURN_CODE_ERR_HW;
				goto retcode;
			}
			ret = npd_vlan_forward_mode_set(entry.outer_vid, isAdd, &entry);
		}
		else
			ret = VLAN_RETURN_CODE_ELINE_NO_EXIST;
    }
retcode:

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}

DBusMessage * npd_dbus_vlan_config_netif_eline(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret  = 0;
    unsigned int isEnable;
    unsigned int eline_id;
	unsigned int netif_index;
    vlan_eline_db_entry_t entry = {0};
    switch_port_db_t switch_port;
	unsigned char isTagged;
        
	DBusError err;

	syslog_ax_vlan_dbg("Entering config vlan xlate ...\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                                DBUS_TYPE_UINT32,&isEnable,
                                DBUS_TYPE_UINT32,&eline_id,    
                                DBUS_TYPE_UINT32,&netif_index,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_vlan_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_vlan_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch_port.global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
    {
        ret = VLAN_RETURN_CODE_PORT_NOT_SWTICHMODE;
    }
    else
    {
        if(TRUE != switch_port.access_qinq)
        {
            ret = VLAN_RETURN_CODE_PORT_ACCESS_MODE_CONFLICT;
            goto retcode;
        }
    }

	ret = dbtable_array_get(vlan_eline_array, eline_id, &entry);
	if (ret != 0)
	{
		ret = VLAN_RETURN_CODE_ELINE_NO_EXIST;
		goto retcode;
	}

	ret = npd_vlan_check_contain_netif(entry.outer_vid, switch_port.global_port_ifindex, &isTagged);
	if (ret != NPD_TRUE)
	{
		ret = VLAN_RETURN_CODE_PORT_NOTEXISTS;
		goto retcode;
	}
	
    if(isEnable)
    {
		if (entry.netif_index_first == netif_index ||
			entry.netif_index_second == netif_index)
		{
			ret = VLAN_RETURN_CODE_PORT_ELINE_CONFLICT;
			goto retcode;
		}
		
		if (entry.netif_index_first != 0)
		{
			if (entry.netif_index_second != 0)
			{
				ret = VLAN_RETURN_CODE_PORT_ELINE_FULL;
				goto retcode;				
			}
			else
			{
				entry.netif_index_second = netif_index;
			}
		}

		if (entry.netif_index_first == 0)
 		{
 			 entry.netif_index_first = netif_index;
		}
    }
    else
    {
		if (entry.netif_index_first != netif_index &&
			entry.netif_index_second != netif_index)
		{
			ret = VLAN_RETURN_CODE_PORT_ELINE_NO_PORT;
			goto retcode;
		}
		
		if (entry.netif_index_first == netif_index) 
		{
			entry.netif_index_first = 0;
		}
		else if (entry.netif_index_second == netif_index)
		{
			entry.netif_index_second = 0;
		}
    }
	
	ret = dbtable_array_update(vlan_eline_array, eline_id, NULL, &entry);
	if (0 != ret)
		ret = VLAN_RETURN_CODE_ERR_HW;
		
	
retcode:    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
	
}
#endif

DBusMessage * npd_dbus_vlan_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *showStr = NULL;

	showStr = (char*)malloc(NPD_VLAN_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		 syslog_ax_vlan_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,NPD_VLAN_RUNNING_CFG_MEM);
	/*save vlan cfg*/
	npd_save_vlan_cfg(showStr,NPD_VLAN_RUNNING_CFG_MEM);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}


int npd_check_vlan_private_primary(unsigned short vlanid)
{
	return NPD_FALSE;
}
int npd_check_vlan_rspn(unsigned short vlanid)
{
	return NPD_FALSE;
}
int npd_check_vlan_voice(unsigned short vlanid)
{
	return NPD_FALSE;
}

DBusMessage * npd_dbus_vlan_check_port_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
		DBusMessage* reply;
		DBusError err;
		DBusMessageIter  iter;
		int ret = VLAN_RETURN_CODE_ERR_NONE;
		
		unsigned short vlanid;
		unsigned long netif_index;
		int state;
				
			
		dbus_error_init(&err);
		
		dbus_message_iter_init(msg,&iter);
		dbus_message_iter_get_basic(&iter,&vlanid);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&netif_index);	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&state);	
		if(vlanid != 0){
			ret = npd_check_user_vlan_exist(vlanid);
			if ( NPD_FALSE == ret){
				syslog_ax_vlan_dbg("vlan %d doesn't exist\n", vlanid );
				ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
				goto reply;
			}
			if ( npd_check_vlan_private_primary(vlanid)){
				syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
				ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
				goto reply;
			}
			if ( npd_check_vlan_rspn(vlanid)){
				syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
				ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
				goto reply;
			}
			if ( npd_check_vlan_voice(vlanid)){
				syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
				ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
				goto reply;
			}
		}
		
#ifdef HAVE_AAA
		ret = npd_asd_check_vlan_valid(netif_index, vlanid, state);
		if(ret){
			syslog_ax_vlan_dbg("VLAN %d invalid\r\n", vlanid);
			goto reply;
		}
#endif		
		if(npd_check_netif_switch_mode(netif_index) == NPD_FALSE){
			syslog_ax_vlan_dbg("port %d is not switch port\n", netif_index );
			ret = ETHPORT_RETURN_CODE_UNSUPPORT;
			goto reply;
		}
		if(npd_vlan_check_port_membership(vlanid, netif_index, 1)== NPD_TRUE){
			syslog_ax_vlan_dbg("port %d is the tag member of vlan %d\n", netif_index, vlanid );
			ret = VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT;
			goto reply;
		}
reply:		
		reply = dbus_message_new_method_return(msg);
			
		dbus_message_iter_init_append(reply, &iter);
			
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		return reply;	

}



DBusMessage * npd_dbus_vlan_set_port_auth_fail_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = VLAN_RETURN_CODE_ERR_NONE;
	
	unsigned short vlanid;
	unsigned long netif_index;
	int state;
			
		
	dbus_error_init(&err);
	
	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&vlanid);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&netif_index);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);	
	if(vlanid != 0){
		ret = npd_check_user_vlan_exist(vlanid);
		if ( NPD_FALSE == ret){
			syslog_ax_vlan_dbg("vlan %d doesn't exist\n", vlanid );
			ret = VLAN_RETURN_CODE_VLAN_NOT_EXISTS;
			goto reply;
		}
		if ( npd_check_vlan_private_primary(vlanid)){
			syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
			ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
			goto reply;
		}
		if ( npd_check_vlan_rspn(vlanid)){
			syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
			ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
			goto reply;
		}
		if ( npd_check_vlan_voice(vlanid)){
			syslog_ax_vlan_dbg("vlan %d is private vlan\n", vlanid );
			ret = VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED;
			goto reply;
		}
	}
	
#ifdef HAVE_AAA
	ret = npd_asd_check_vlan_valid(netif_index, vlanid, state);
	if(ret){
		syslog_ax_vlan_dbg("VLAN %d invalid\r\n", vlanid);
		goto reply;
	}
#endif		
	if(npd_check_netif_switch_mode(netif_index) == NPD_FALSE){
		syslog_ax_vlan_dbg("port %d is not switch port\n", netif_index );
		ret = ETHPORT_RETURN_CODE_UNSUPPORT;
		goto reply;
	}
	if(npd_vlan_check_port_membership(vlanid, netif_index, 1)== NPD_TRUE){
		syslog_ax_vlan_dbg("port %d is the tag member of vlan %d\n", netif_index, vlanid );
		ret = VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT;
		goto reply;
	}
reply:		
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	return reply;	

}


#ifdef __cplusplus
}
#endif
