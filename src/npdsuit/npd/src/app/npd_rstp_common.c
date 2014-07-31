/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_rstp_common.c
*
*
*CREATOR:
*	chengjun@autelan.com
*
*DESCRIPTION:
*	APIs used in NPD for rstp process.
*
*DATE:
*	03/01/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.52 $		
*******************************************************************************/
#ifdef HAVE_BRIDGE_STP
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/protocol/stp_api.h"
#include "npd_rstp_common.h"

db_table_t *ports_stp_db = NULL;
hash_table_index_t *ports_stp = NULL;

db_table_t *vlan_stp_db = NULL;
sequence_table_index_t *vlans_stp = NULL;


int npd_stp_msg_handler(char *msg, int len);

int npd_mstp_vlan_stpid_bind
(
	unsigned short vid,
	unsigned int 	mstid	
);
int npd_cmd_write_ro_rstp
(
	NPD_CMD_STC		*npdCmdPtr,
	unsigned int	cmdLen
);



/*******************************************************************************
 * npd_rstp_link_change
 *
 * DESCRIPTION:
 *   	when link up or down ,send info to stp
 *
 * INPUTS:
 * 	NONE
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	TRUE   	- on success
 * 	FALSE	- otherwise
 *
 *
 ********************************************************************************/
int npd_rstp_link_change
(
	unsigned int netif_index,
	enum PORT_NOTIFIER_ENT	event
)
{
	unsigned char stp_enable = 0;
	int duplex_mode= 0;
	int ret = 0;
	int speed = 0;
    struct stp_info_db_s stpInfo;
    NPD_CMD_STC		npdCmd;
    int i;
	int temp_stpen = 0;

	syslog_ax_rstp_dbg("rstp handle link change: port_index 0x%x, event %d\n", netif_index, event );
    memset(&stpInfo, 0, sizeof(stpInfo));
    if((npd_netif_type_get(netif_index) != NPD_NETIF_ETH_TYPE)
        && (npd_netif_type_get(netif_index) != NPD_NETIF_TRUNK_TYPE))
    {
        syslog_ax_rstp_dbg("Only handle ethernet and trunk netif.\n");
        return -1;
    }

	if(PORT_NOTIFIER_LINKUP_E == event)
		stp_enable = 1;
	else if((PORT_NOTIFIER_LINKDOWN_E == event)
             || (PORT_NOTIFIER_REMOVE == event))
		stp_enable = 0;
    else 
        goto error;

    
    stpInfo.port_index = netif_index;
    stpInfo.mstid = 0;
    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
    if(0 != ret)
    {
        syslog_ax_rstp_dbg("Netif does not enable\n");
        return -1;
    }
    ret = npd_netif_speed(netif_index,&speed);
	/*get the duplex mode*/
	ret = npd_netif_duplex_mode(netif_index,&duplex_mode);
    
	npdCmd.type = LINK_CHANGE_EVENT_E;
	npdCmd.length = sizeof(NPD_RSTP_INTF_OP_PARAM_T);
	npdCmd.cmdData.cmdLink.portIdx = netif_index;
	npdCmd.cmdData.cmdLink.speed = speed;
	npdCmd.cmdData.cmdLink.duplex_mode = duplex_mode;

    temp_stpen = stpInfo.stp_en;
    stpInfo.stp_en= stp_enable;
	/*此段要考虑防止trunk在增加接口的时候出现状态被强制成discard问题*/
    if((stp_enable) && (!temp_stpen) && (stpInfo.state != NAM_STP_PORT_STATE_DISABLE_E))
           stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
    dbtable_hash_update(ports_stp, NULL, &stpInfo);
    npdCmd.cmdData.cmdLink.event = stp_enable;

    for(i = 1; i < 256; i++)
    {
        stpInfo.port_index = netif_index;
        stpInfo.mstid = i;
        ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
        if(0 == ret)
        {
            if((stpInfo.smartlink_flag == 1)||(stpInfo.erpp_flag == 1))
                continue;
            if(stpInfo.stp_en != stp_enable)
            {
                stpInfo.stp_en= stp_enable;
                if((stp_enable) && (stpInfo.state != NAM_STP_PORT_STATE_DISABLE_E))
                    stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
            }
            dbtable_hash_update(ports_stp, NULL, &stpInfo);
        }
        
    }
	/*config protocol*/
	 syslog_ax_rstp_dbg("npd_rstp_link_change :: link state %d, port %d ,speed %x,duplex_mode %d config protocol\n",
	 event,netif_index,speed, duplex_mode);
	if(( npd_cmd_write_ro_rstp(&npdCmd,sizeof(NPD_CMD_STC)))!=-1){
		 syslog_ax_rstp_dbg("send Success\n");
	}
	else{
		 syslog_ax_rstp_err("del ERROR\n");
		return -1;
	}
	return 0;
error:
    return -1;
}

void npd_smart_link_stp_flag
(
	unsigned int netif_index,
	unsigned int mstid,
	unsigned int is_enable,
	unsigned int stp_port_status
)
{
    struct stp_info_db_s stpInfo;

    memset(&stpInfo, 0, sizeof(stpInfo));
    
    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;
    
    if (0 == dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo))
    {
        stpInfo.smartlink_flag = is_enable;
        stpInfo.state = stp_port_status;
        (void)dbtable_hash_update(ports_stp, NULL, &stpInfo);
    }

    memset(&stpInfo, 0, sizeof(stpInfo));
    stpInfo.port_index = netif_index;
    stpInfo.mstid = 0;
    
    if (0 == dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo))
    {
        stpInfo.smartlink_flag = is_enable;
        stpInfo.state = stp_port_status;
        (void)dbtable_hash_update(ports_stp, NULL, &stpInfo);
    }

    return ;
}

void npd_erpp_stp_flag
(
	unsigned int netif_index,
	unsigned int mstid,
	unsigned int is_enable,
	unsigned int stp_port_status
)
{
    struct stp_info_db_s stpInfo;

    memset(&stpInfo, 0, sizeof(stpInfo));
    
    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;
    
    if (0 == dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo))
    {
        stpInfo.erpp_flag = is_enable;
        stpInfo.state = stp_port_status;
        (void)dbtable_hash_update(ports_stp, NULL, &stpInfo);
    }

    memset(&stpInfo, 0, sizeof(stpInfo));
    stpInfo.port_index = netif_index;
    stpInfo.mstid = 0;
    
    if (0 == dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo))
    {
        stpInfo.erpp_flag = is_enable;
        stpInfo.state = stp_port_status;
        (void)dbtable_hash_update(ports_stp, NULL, &stpInfo);
    }

    return ;
}

void npd_rstp_switchover_event(event)
{
    NPD_CMD_STC		npdCmd = {0};

	syslog_ax_rstp_dbg("rstp handle switchover\n");

	npdCmd.type = STP_SWITCHOVER;
	npdCmd.length = 0;
	if(( npd_cmd_write_ro_rstp(&npdCmd,sizeof(NPD_CMD_STC)))!=-1){
		 syslog_ax_rstp_dbg("send Success\n");
		return ;
	}
	else{
		 syslog_ax_rstp_err("del ERROR\n");
		return ;
	}

	return ;
}


void npd_rstp_vlan_event(
    unsigned int vid,
    enum PORT_NOTIFIER_ENT event
    )
{
    struct vlan_s *vlan;
    struct switch_port_db_s switch_port;
    unsigned int switch_port_index;
    npd_pbmp_t bmp;
    switch(event)
    {
    case PORT_NOTIFIER_CREATE:
        npd_mstp_add_vlan_on_mst(vid);
        npd_mstp_vlan_stpid_bind(vid, 0);
        /*此时不会存在端口*/
#if 0		
        vlan = npd_find_vlan_by_vid(vid);
        /*
        add port to mstp;
        */
        if(NULL == vlan)
            return;
        NPD_PBMP_ASSIGN(bmp, vlan->tag_ports);
        NPD_PBMP_OR(bmp, vlan->untag_ports);
        NPD_PBMP_ITER(bmp, switch_port_index)
        {
            int ret;
            switch_port.switch_port_index = switch_port_index;
            ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port);
            if(0 != ret)
                continue;
            npd_mstp_add_port(vid, switch_port.global_port_ifindex);
        }
        
        free(vlan);
#endif		
        break;
    case PORT_NOTIFIER_DELETE:
        npd_mstp_del_vlan_on_mst(vid);
        vlan = npd_find_vlan_by_vid(vid);
        if(NULL == vlan)
            return;

        NPD_PBMP_ASSIGN(bmp, vlan->tag_ports);
        NPD_PBMP_OR(bmp, vlan->untag_ports);
        NPD_PBMP_ITER(bmp, switch_port_index)
        {
            int ret;
            switch_port.switch_port_index = switch_port_index;
            ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port);
            if(0 != ret)
                continue;
            npd_mstp_del_port(vid, switch_port.global_port_ifindex);
        }
        {
            vlan_stp_t temp;
            temp.vid = vid;
            int ret;

            ret = dbtable_sequence_head(vlans_stp, vid, &temp);
            while(0 == ret)
            {
                dbtable_sequence_delete(vlans_stp, vid, &temp, &temp);
                ret = dbtable_sequence_head(vlans_stp, vid, &temp);
            }
        }
        /*ports_stp会在relate事件中进行处理*/
        free(vlan);
        break;
    default:
        break;
    }
}

unsigned int ports_stp_cmp_port(struct stp_info_db_s *exist,
	                   struct stp_info_db_s  *new)
{
	if(exist->port_index == new->port_index)
		return TRUE;
	else
		return FALSE;
}

void npd_rstp_netif_event(
	unsigned int netif_index,
	enum PORT_NOTIFIER_ENT	event,
	char *private, int len
    )
{
    unsigned int type = npd_netif_type_get(netif_index);
    vlan_stp_t vlan_stp = {0};
    int ret;

    ret = dbtable_sequence_traverse_next(vlans_stp, -1, &vlan_stp);
    if(0 != ret)
        return;
    

    if(NPD_GLOBAL_NETIF_INDEX == netif_index)
    {
        switch(event)
        {
            case NOTIFIER_SWITCHOVER:
                npd_rstp_switchover_event(event);
                break;
            default:
                break;
        }
        return;
    }

    switch(type)
    {
    case NPD_NETIF_VLAN_TYPE:
        npd_rstp_vlan_event(npd_netif_vlan_get_vid(netif_index), event);
        break;
    case NPD_NETIF_ETH_TYPE:
    case NPD_NETIF_TRUNK_TYPE:
        {
            switch(event)
            {
            case PORT_NOTIFIER_L2CREATE:
                if(dbtable_hash_count(ports_stp) != 0)
                {
                    int link_status;
                    int ret;
                    ret = npd_check_netif_status(netif_index, &link_status);
                    if(0 == ret)
                    {
                        struct stp_info_db_s stpInfo = {0};
                        stpInfo.port_index = netif_index;
                	    stpInfo.stp_en = link_status;
                        stpInfo.mstid = 0;
                        stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
                        stpInfo.port_index_count++;
                        ret = dbtable_hash_insert(ports_stp, &stpInfo);
                    }  
                    npd_mstp_add_port(0, netif_index);
                    if(link_status)
                       npd_rstp_link_change(netif_index, PORT_NOTIFIER_LINKUP_E);
                }
                break;
            case PORT_NOTIFIER_L2DELETE:
                npd_mstp_del_port(0, netif_index);
				/*此处还是需要删除该端口相关的所有stp的信息，relate事件在此之后才能到达*/
                if(dbtable_hash_count(ports_stp) != 0)
                {
                    struct stp_info_db_s stpInfo = {0};
					int i;

                    for(i = 1; i < 256; i++)
                    {
                        stpInfo.port_index = netif_index;
                        stpInfo.mstid = i;
                        ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
                        if(0 == ret)
                        {
                            dbtable_hash_delete(ports_stp, &stpInfo, &stpInfo);
                        }
                        
                    }					

                    stpInfo.port_index = netif_index;
                    stpInfo.mstid = 0;
                    dbtable_hash_delete(ports_stp, &stpInfo, &stpInfo);
                }
                break;
            default:
                npd_rstp_link_change(netif_index, event);
                break;
                
            }
        }
        break;
    default:
        break;
    }
    return;
    
}

void npd_rstp_relate_handle(
    unsigned int father_index,
    unsigned int son_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
    unsigned short vid;
    vlan_stp_t vlan_stp = {0};
    int ret;

    ret = dbtable_sequence_traverse_next(vlans_stp, -1, &vlan_stp);
    if(0 != ret)
        return;


    /*change lag bandwidth*/
    if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(father_index))
    {
        int link_status = npd_check_trunk_status(father_index);
        if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(son_index))
        {
            if(link_status)
                npd_rstp_link_change(father_index, PORT_NOTIFIER_LINKUP_E);
            else
                npd_rstp_link_change(father_index, PORT_NOTIFIER_LINKDOWN_E);
        }
        return;
    }
    
    /*only handle vlan add port or trunk event*/
    if(NPD_NETIF_VLAN_TYPE != npd_netif_type_get(father_index))
        return;

    if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(son_index))
       && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(son_index)))
       return;

    vid = npd_netif_vlan_get_vid(father_index);

    switch(event)
    {
        case PORT_NOTIFIER_JOIN:
        {
            vlan_stp_t temp;
            temp.vid = vid;
            int ret;

            ret = dbtable_sequence_search(vlans_stp, vid, &temp);
            if(0 == ret)
            {
                if(temp.mstid != 0)
                {
                    int link_status;
                    ret = npd_check_netif_status(son_index, &link_status);
                    if(0 == ret)
                    {
                        struct stp_info_db_s stpInfo = {0};
                        stpInfo.port_index = son_index;
                        stpInfo.mstid = temp.mstid;
                        stpInfo.port_index_count = 0;
                        ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
						if(0 != ret)
						{
							stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
						}
                	    stpInfo.stp_en = link_status;
                        stpInfo.port_index_count++;
                        ret = dbtable_hash_insert(ports_stp, &stpInfo);
                    }                    
                }
            }
        }
        npd_mstp_add_port(vid, son_index);

        
        break;
    case PORT_NOTIFIER_LEAVE:
        npd_mstp_del_port(vid, son_index);
        {
            vlan_stp_t temp;
            temp.vid = vid;
            int ret;

            ret = dbtable_sequence_search(vlans_stp, vid, &temp);
            if(0 == ret)
            {
                if(temp.mstid != 0)
                {
                    int link_status;
                    ret = npd_check_netif_status(son_index, &link_status);
                    if(0 == ret)
                    {
                        struct stp_info_db_s stpInfo;
                        stpInfo.mstid = temp.mstid;
                        stpInfo.port_index = son_index;
                        ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
                        if(0 == ret)
                        {
                            if(stpInfo.port_index_count == 1)
                            {
                                ret = dbtable_hash_delete(ports_stp, &stpInfo, &stpInfo);
                            }
                            else
                            {
                                stpInfo.port_index_count--;
                                dbtable_hash_update(ports_stp, NULL, &stpInfo);
                            }
                        }
                    }                    
                }
            }
        }
        break;
    default:
        break;
    }
    return;
        
}
    


netif_event_notifier_t stp_netif_notifier =
{
    .netif_event_handle_f = &npd_rstp_netif_event,
    .netif_relate_handle_f = &npd_rstp_relate_handle
};

unsigned int npd_stp_hash_key(void * data)
{
    stp_info_db_t *stp_info = (stp_info_db_t*)data;
    unsigned int key;
    NPD_NETIF_INDEX_U eth_ifindex;

	eth_ifindex.netif_index = stp_info->port_index;

    key = (stp_info->mstid&0xf)<<6;    
    key += ((eth_ifindex.common_if.other>>22)&0x3)<<4;
    key += ((eth_ifindex.common_if.other>>14)&0xf);

    return key%(MSTP_PORT_MST_DB_SIZE/8);
}

unsigned int npd_stp_hash_cmp(void* data1, void* data2)
{
    stp_info_db_t *stp_info1 = (stp_info_db_t*)data1;
    stp_info_db_t *stp_info2 = (stp_info_db_t*)data2;


    return ((stp_info1->port_index == stp_info2->port_index)
              && (stp_info1->mstid == stp_info2->mstid));
    
}

unsigned int vlan_stp_index(unsigned int index)
{
    return index;
}

unsigned int vlan_stp_key(void *data)
{
    struct vlan_stp_s *vlan_stp = (struct vlan_stp_s*)data;

    return vlan_stp->vid;
}

int vlan_stp_cmp(void *data1, void *data2)
{
    struct vlan_stp_s *vlan_stp1 = (struct vlan_stp_s*)data1;
    struct vlan_stp_s *vlan_stp2 = (struct vlan_stp_s*)data2;

    return (vlan_stp1->vid == vlan_stp2->vid);
}
/**********************************************************************************
 *  npd_init_port_stp
 *
 *	DESCRIPTION:
 * 		this routine initialize all ethernet ports
 *
 *	INPUT:
 *		NULL
 *		
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
long npd_stp_info_update(void *newdata, void *olddata);
long npd_stp_info_insert(void *data);
long npd_stp_info_delete(void *data);
long npd_vlan_stp_insert(void *data);
long npd_vlan_stp_delete(void *data);
long npd_vlan_stp_update(void *new, void *old);

void npd_init_stp(void)
{
    char name[16];

    strcpy(name, "PORT_STP_DB");
    create_dbtable(name, MSTP_PORT_MST_DB_SIZE, sizeof(stp_info_db_t),
        npd_stp_info_update, NULL, npd_stp_info_insert, npd_stp_info_delete, 
        NULL, NULL, NULL, NULL, NULL, DB_SYNC_ALL, &ports_stp_db);
    if(NULL == ports_stp_db)
        return;
    dbtable_create_hash_index("mstid&netif_index", ports_stp_db, MSTP_PORT_MST_DB_SIZE/8, 
        &npd_stp_hash_key, &npd_stp_hash_cmp, &ports_stp);
    if(NULL == ports_stp)
	{
		syslog_ax_eth_port_dbg("memory alloc error for eth port stp init!!!\n");
		return;
	}

    strcpy(name, "VLAN_STP_DB");
    create_dbtable(name, CHASSIS_VLAN_RANGE_MAX-2, sizeof(vlan_stp_t),
        &npd_vlan_stp_update, NULL, &npd_vlan_stp_insert, &npd_vlan_stp_delete, 
        NULL, NULL, NULL, NULL, NULL, DB_SYNC_ALL, &vlan_stp_db);
    if(NULL == vlan_stp_db)
        return;
    
    dbtable_create_sequence_index("vlan_id", vlan_stp_db, CHASSIS_VLAN_RANGE_MAX-2, &vlan_stp_index,
        &vlan_stp_key, &vlan_stp_cmp, &vlans_stp);
    if(NULL == vlans_stp)
        return;
    register_netif_notifier(&stp_netif_notifier);

    nam_asic_stp_init();
	return;
}

long npd_stp_info_insert(void *data)
{
    struct stp_info_db_s *new_stp_port 
          = (stp_info_db_t*)data;
    int ret = 0;

    syslog_ax_rstp_dbg("Enable stp port %x \n", 
        new_stp_port->port_index);
    if(NPD_NETIF_TRUNK_TYPE 
         == npd_netif_type_get(new_stp_port->port_index))
    {
        unsigned int array_port;
        struct trunk_s trunk = {0};

        trunk.trunk_id = npd_netif_trunk_get_tid(new_stp_port->port_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;
        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            unsigned int eth_index;
            eth_index = eth_port_array_index_to_ifindex(array_port);
            if(new_stp_port->mstid == 0)
                ret = nam_asic_stp_port_enable(eth_index, new_stp_port->stp_en);
        }
    }
    else if(NPD_NETIF_ETH_TYPE
         == npd_netif_type_get(new_stp_port->port_index))
    {      
      if(new_stp_port->mstid == 0)
          ret = nam_asic_stp_port_enable(new_stp_port->port_index, new_stp_port->stp_en);
    }
    return 0;
}

long npd_stp_info_delete(void *data)
{
    struct stp_info_db_s *new_stp_port 
          = (stp_info_db_t*)data;
    int ret = 0;

    syslog_ax_rstp_dbg("Disable stp port %x \n", 
        new_stp_port->port_index);

    if(NPD_NETIF_TRUNK_TYPE 
         == npd_netif_type_get(new_stp_port->port_index))
    {
        unsigned int array_port;
        struct trunk_s trunk = {0};

        trunk.trunk_id = npd_netif_trunk_get_tid(new_stp_port->port_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;
        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            unsigned int eth_index;
            eth_index = eth_port_array_index_to_ifindex(array_port);
			/*此处，如果mstid为0，则说明stp协议已经禁止，
			  可以设置为disable, 如果mstid不为零，因为VLAN还没有从mstid删除,
			  如果设置成disable，有可能会导致暂时的环路*/
			if(new_stp_port->mstid != 0)
		        ret = nam_stp_state_set(new_stp_port->port_index, 
                      new_stp_port->mstid, NAM_STP_PORT_STATE_DISCARD_E);       
			else
		        ret = nam_stp_state_set(new_stp_port->port_index, 
                      new_stp_port->mstid, NAM_STP_PORT_STATE_DISABLE_E);       
            if(new_stp_port->mstid == 0)
                ret = nam_asic_stp_port_enable(eth_index, 0);
        }
    }
    else if(NPD_NETIF_ETH_TYPE
         == npd_netif_type_get(new_stp_port->port_index))
    {    
    	/*此处，如果mstid为0，则说明stp协议已经禁止，
    	  可以设置为disable, 如果mstid不为零，因为VLAN还没有从mstid删除,
    	  如果设置成disable，有可能会导致暂时的环路*/
    	if(new_stp_port->mstid != 0)
            ret = nam_stp_state_set(new_stp_port->port_index, 
                  new_stp_port->mstid, NAM_STP_PORT_STATE_DISCARD_E);       
    	else
            ret = nam_stp_state_set(new_stp_port->port_index, 
                  new_stp_port->mstid, NAM_STP_PORT_STATE_DISABLE_E);       
        if(new_stp_port->mstid == 0)
            ret = nam_asic_stp_port_enable(new_stp_port->port_index, 0);
    }

    return 0;
}


long npd_stp_info_update(void *newdata, void *olddata)
{
    struct stp_info_db_s *new_stp_port 
          = (stp_info_db_t*)newdata;
    struct stp_info_db_s *old_stp_port
         = (stp_info_db_t *)olddata;
    int ret = 0;

    syslog_ax_rstp_dbg("Update stp port %x \n", 
        new_stp_port->port_index);
    
    if(new_stp_port->stp_en != old_stp_port->stp_en)
    {
        if(new_stp_port->stp_en)
        {
            if(NPD_NETIF_TRUNK_TYPE 
                 == npd_netif_type_get(new_stp_port->port_index))
            {
                unsigned int array_port;
                struct trunk_s trunk = {0};

                trunk.trunk_id = npd_netif_trunk_get_tid(new_stp_port->port_index);
                ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
                if(0 != ret)
                    return 0;
                NPD_PBMP_ITER(trunk.ports, array_port)
                {
                    unsigned int eth_index;
                    eth_index = eth_port_array_index_to_ifindex(array_port);                    
                    if(new_stp_port->mstid == 0)
                        ret = nam_asic_stp_port_enable(eth_index, 1);
                }
            }
            else if(NPD_NETIF_ETH_TYPE
                 == npd_netif_type_get(new_stp_port->port_index))
            {
                  if((new_stp_port->mstid == 0)&&(new_stp_port->smartlink_flag == 0)&&(new_stp_port->erpp_flag == 0))
                      ret = nam_asic_stp_port_enable(new_stp_port->port_index, 1);
                #if 0
              if(new_stp_port->mstid == 0)
                  ret = nam_asic_stp_port_enable(new_stp_port->port_index, 1);
              #endif
/*            nam_stp_port_bpdu_enable(devNum, portNum, 1);*/
            }
        }
        else
        {
            if(NPD_NETIF_TRUNK_TYPE 
                 == npd_netif_type_get(new_stp_port->port_index))
            {
                unsigned int array_port;
                struct trunk_s trunk = {0};

                trunk.trunk_id = npd_netif_trunk_get_tid(new_stp_port->port_index);
                ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
                if(0 != ret)
                    return 0;
                NPD_PBMP_ITER(trunk.ports, array_port)
                {
                    unsigned int eth_index;
                    eth_index = eth_port_array_index_to_ifindex(array_port);                    
                    if(new_stp_port->mstid == 0)
                        ret = nam_asic_stp_port_enable(eth_index, 0);
                }
            }
            else if(NPD_NETIF_ETH_TYPE
                 == npd_netif_type_get(new_stp_port->port_index))
            {
                
              if(new_stp_port->mstid == 0)
                  ret = nam_asic_stp_port_enable(new_stp_port->port_index, 0);
/*            nam_stp_port_bpdu_enable(devNum, portNum, 1);*/
            }        
        }
    }

    {
            if(NPD_NETIF_TRUNK_TYPE 
                 == npd_netif_type_get(new_stp_port->port_index))
            {
                unsigned int array_port;
                struct trunk_s trunk = {0};

                trunk.trunk_id = npd_netif_trunk_get_tid(new_stp_port->port_index);
                ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
                if(0 != ret)
                    return 0;
                NPD_PBMP_ITER(trunk.ports, array_port)
                {
                    unsigned int eth_index;
                    eth_index = eth_port_array_index_to_ifindex(array_port);

                    ret = nam_stp_state_set(eth_index, new_stp_port->mstid, new_stp_port->state);
                }
            }
            else if(NPD_NETIF_ETH_TYPE
                 == npd_netif_type_get(new_stp_port->port_index))
            {
              ret = nam_stp_state_set(new_stp_port->port_index, new_stp_port->mstid, new_stp_port->state);
/*            nam_stp_port_bpdu_enable(devNum, portNum, 1);*/
            } 
    }
    return 0;
}

long npd_vlan_stp_update(void *new, void *old)
{
    vlan_stp_t *vlan_stp = (vlan_stp_t*)new;
    vlan_stp_t *old_vlan_stp = (vlan_stp_t*)old;
    int ret = 0;

    syslog_ax_rstp_dbg("Insert vlan %d mstid %d \n", 
        vlan_stp->vid, vlan_stp->mstid);

    ret = nam_stp_vlan_unbind_stg(0, old_vlan_stp->vid, old_vlan_stp->mstid);
    ret = nam_stp_vlan_bind_stg(vlan_stp->vid, vlan_stp->mstid);
    return 0;
}
long npd_vlan_stp_insert(void *data)
{
    vlan_stp_t *vlan_stp = (vlan_stp_t*)data;
    int ret = 0;

    syslog_ax_rstp_dbg("Insert vlan %d mstid %d \n", 
        vlan_stp->vid, vlan_stp->mstid);

    ret = nam_stp_vlan_bind_stg(vlan_stp->vid, vlan_stp->mstid);
    return 0;
}

long npd_vlan_stp_delete(void *data)
{
    vlan_stp_t *vlan_stp = (vlan_stp_t*)data;
    int ret= 0;

    syslog_ax_rstp_dbg("Delete vlan %d mstid %d \n", 
        vlan_stp->vid, vlan_stp->mstid);

    ret = nam_stp_vlan_unbind_stg(0, vlan_stp->vid, vlan_stp->mstid);
    return 0;
    
}

/*local variables definition begin */
struct	sockaddr_un 	npd_addr;       /*local addr*/	
struct	sockaddr_un		rstp_addr;   	/*remote addr*/
int		ser_fd = 0;



int npd_netif_config_stp
(
	unsigned int	netif_index,
	enum stp_running_mode mode,
	unsigned int	state
)
{
	struct stp_info_db_s stpInfo = {0};
    int ret;
    int link_status;

    stpInfo.mstid = 0;
    stpInfo.port_index = netif_index;
    {
        int i;
        ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
        if(0 != ret)
            return STP_RETURN_CODE_RSTP_NOT_ENABLED;
        if(!state)
            stpInfo.state = NAM_STP_PORT_STATE_DISABLE_E;
        else
            stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
        ret = dbtable_hash_update(ports_stp, &stpInfo, &stpInfo);
        for(i = 1; i < 256; i++)
        {
            stpInfo.port_index = netif_index;
            stpInfo.mstid = i;
            ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);
            if(0 == ret)
            {
                if(!state)
                    stpInfo.state = NAM_STP_PORT_STATE_DISABLE_E;
                else
                    stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
                dbtable_hash_update(ports_stp, NULL, &stpInfo);
            }
            
        }
        if(state)
        {
            ret = npd_check_netif_status(netif_index, &link_status);
            if(link_status)
                npd_rstp_link_change(netif_index, PORT_NOTIFIER_LINKUP_E);
        }
            
        return STP_RETURN_CODE_ERR_NONE;
    }
/*    
    else
    {
        ret = npd_check_netif_status(netif_index, &link_status);
        if(0 == ret)
        {
    	    stpInfo.stp_en = link_status;
            stpInfo.mode = mode;
    		stpInfo.prio = DEF_PORT_PRIO;
    		stpInfo.pathcost = ADMIN_PORT_PATH_COST_AUTO;
        	stpInfo.edge = DEF_ADMIN_EDGE;
        	stpInfo.p2p = DEF_P2P;
        	stpInfo.nonstp = DEF_ADMIN_NON_STP;
            stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
            ret = dbtable_hash_insert(ports_stp, &stpInfo);
            if(link_status)
                npd_rstp_link_change(netif_index, PORT_NOTIFIER_LINKUP_E);
        }
    }
*/    
    return STP_RETURN_CODE_ERR_NONE;
}


int	npd_rstp_msg_init()
{
	memset(&rstp_addr,0,sizeof(rstp_addr));
	memset(&npd_addr,0,sizeof(npd_addr));

	if((ser_fd = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		 syslog_ax_rstp_err("create npd to rstp socket fail\n");
		return -1;
	}
	rstp_addr.sun_family = AF_LOCAL;
	strcpy(rstp_addr.sun_path, "/tmp/NpdCmd_CLIENT");
	
	npd_addr.sun_family = AF_LOCAL;
	strcpy(npd_addr.sun_path,"/tmp/NpdCmd_SERVER");
				

    unlink(npd_addr.sun_path);

	if(bind(ser_fd, (struct sockaddr *)&npd_addr, sizeof(npd_addr)) == -1) 
	{
		 syslog_ax_rstp_err("npd to rstp socket created but failed when bind\n");
		return -1;
	}

	chmod(npd_addr.sun_path, 0777);

	npd_app_msg_socket_register(ser_fd, "stpMsg",npd_stp_msg_handler, sizeof(NPD_CMD_STC));

	return 0;	
	
}

 int npd_cmd_write_ro_rstp
(
	NPD_CMD_STC		*npdCmdPtr,
	unsigned int	cmdLen
)
{
	int	rc,byteSend = 0;

	while(cmdLen != byteSend)
	{
        if(ser_fd == 0)
            return -1;
		rc = sendto(ser_fd,npdCmdPtr,cmdLen,0,
							(struct sockaddr *)&rstp_addr, sizeof(rstp_addr));
		if(rc < 0)
		{
			if(errno == EINTR)
			{
				npd_syslog_err("sendto fail \n");
				continue; 
			}
			else
			{
				 syslog_ax_rstp_err("npd send command to rstp write fail\n");
				return -1;
			}
		}
		byteSend += rc;
	}

	return byteSend;
	
}


int npd_mstp_enable_port(unsigned int port_index)
{
		NPD_CMD_STC	 npdCmdPtr ={
		.type = PORT_ENABLE_E,
		.length  = sizeof(NPD_RSTP_INTF_OP_PARAM_T),
		.cmdData.cmdIntf.vid = 0,
		.cmdData.cmdIntf.portIdx = port_index
	};
	
	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("enable ERROR\n");
		return -1;
	}
		
}

int npd_mstp_disable_port(unsigned int port_index)
{
		NPD_CMD_STC	 npdCmdPtr ={
		.type = PORT_DISABLE_E,
		.length  = sizeof(NPD_RSTP_INTF_OP_PARAM_T),
		.cmdData.cmdIntf.vid = 0,
		.cmdData.cmdIntf.portIdx = port_index
	};
	
	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("disable ERROR\n");
		return -1;
	}
		
}

int npd_mstp_add_port(unsigned short vid,unsigned int index)
{
    int speed;
    int duplex_mode;
	NPD_CMD_STC	 npdCmdPtr ={
		.type = INTERFACE_ADD_E,
		.length  = sizeof(NPD_RSTP_INTF_OP_PARAM_T),
		.cmdData.cmdIntf.vid = vid,
		.cmdData.cmdIntf.portIdx = index
	};

	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(index))
		return 0;
    npd_netif_speed(index,&speed);
	/*get the duplex mode*/
	npd_netif_duplex_mode(index,&duplex_mode);

    npdCmdPtr.cmdData.cmdIntf.speed = speed;
    npdCmdPtr.cmdData.cmdIntf.duplex_mode = duplex_mode;
	
	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("add ERROR\n");
		return -1;
	}
		
}

int npd_mstp_add_ports(unsigned short vid,npd_pbmp_t ports)
{
	NPD_CMD_STC	 npdCmdPtr ={
		.type = INTERFACE_ADD_ALL_E,
		.length  = sizeof(NPD_RSTP_INTF_OP_PARAM_T) + sizeof(npd_pbmp_t),
		.cmdData.cmdIntf.vid = vid,
		.cmdData.cmdIntf.portIdx = 0
	};
	char *buf = malloc(sizeof(NPD_CMD_STC)+sizeof(npd_pbmp_t));
    if(buf == NULL)
    {
		return -1;
    }
	memcpy(buf, &npdCmdPtr, sizeof(NPD_CMD_STC));
	memcpy(buf+sizeof(NPD_CMD_STC), &ports, sizeof(npd_pbmp_t));

	if(( npd_cmd_write_ro_rstp((NPD_CMD_STC *)buf,sizeof(NPD_CMD_STC)+sizeof(npd_pbmp_t)))!=-1){
		free(buf);
		return 0;
	}
	else{
		syslog_ax_rstp_err("add ERROR\n");
		free(buf);
		return -1;
	}
		
}


int npd_mstp_del_port(unsigned short vid,unsigned int index)
{
    int speed;
    int duplex_mode;
	NPD_CMD_STC	 npdCmdPtr ={
		.type = INTERFACE_DEL_E,
		.length  = sizeof(NPD_RSTP_INTF_OP_PARAM_T),
		.cmdData.cmdIntf.vid = vid,
		.cmdData.cmdIntf.portIdx = index
	};
	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(index))
		return 0;
    npd_netif_speed(index,&speed);
	/*get the duplex mode*/
	npd_netif_duplex_mode(index,&duplex_mode);

    npdCmdPtr.cmdData.cmdIntf.speed = speed;
    npdCmdPtr.cmdData.cmdIntf.duplex_mode = duplex_mode;
	
	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("del ERROR\n");
		return -1;
	}
		
}

int npd_mstp_add_vlan_on_mst(unsigned short vid)
{
	NPD_CMD_STC	 npdCmdPtr ={
		.type = VLAN_ADD_ON_MST_E,
		.length  = sizeof(NPD_MSTP_VLAN_OP_PARAM_T),
		.cmdData.cmdVlan.vid = vid
	};

	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("add vlan ERROR\n");
		return -1;
	}
}

int npd_mstp_del_vlan_on_mst(unsigned short vid)
{
	NPD_CMD_STC	 npdCmdPtr ={
		.type = VLAN_DEL_ON_MST_E,
		.length  = sizeof(NPD_MSTP_VLAN_OP_PARAM_T),
		.cmdData.cmdVlan.vid = vid
	};
	
	npdCmdPtr.cmdData.cmdVlan.untagbmp[0] = 0;
	npdCmdPtr.cmdData.cmdVlan.untagbmp[1] =0;
	npdCmdPtr.cmdData.cmdVlan.tagbmp[0] = 0;
	npdCmdPtr.cmdData.cmdVlan.tagbmp[1] = 0;

	if(( npd_cmd_write_ro_rstp(&npdCmdPtr,sizeof(NPD_CMD_STC)))!=-1){
		syslog_ax_rstp_dbg("send success\n");
		return 0;
	}
	else{
		syslog_ax_rstp_err("del vlan ERROR\n");
		return -1;
	}
	
}
int npd_mstp_get_port_state
(
    unsigned short vid,
    unsigned int port_index,
    NAM_RSTP_PORT_STATE_E*  state 
)
{
	struct stp_info_db_s* stp_port = NULL;
    struct vlan_stp_s *vlan_stp = NULL;
	int ret = 0;

    stp_port = malloc(sizeof(struct stp_info_db_s));
    vlan_stp = malloc(sizeof(vlan_stp_t));

    if((NULL == stp_port) || (NULL == vlan_stp))
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
		goto error;
    }

    memset(vlan_stp, 0, sizeof(*vlan_stp));
    vlan_stp->vid = vid;
    ret = dbtable_sequence_search(vlans_stp, vid, vlan_stp);
    if(0 != ret)
    {
        *state = NAM_STP_PORT_STATE_DISABLE_E;
        ret = STP_RETURN_CODE_MSTP_NOT_ENABLED;
        goto error;
    }
    
    stp_port->port_index = port_index;
    stp_port->mstid = vlan_stp->mstid;

    
    ret = dbtable_hash_search(ports_stp, stp_port, NULL, stp_port);

    if(0 != ret)
    {
        *state = NAM_STP_PORT_STATE_DISABLE_E;
    }
    else
    {
        *state = stp_port->state;
    }

error:
    if(vlan_stp)
        free(vlan_stp);
    if(stp_port)
        free(stp_port);

    return ret;
    
}


int npd_stp_msg_handler(char *msg, int len)
{
	int ret = 0;
	NPD_CMD_STC	*stpInfo = (NPD_CMD_STC	*)msg;
    struct stp_info_db_s stp_port;
	switch(stpInfo->type)
	{
		case STP_GET_SYS_MAC_E:
			npd_system_get_basemac(stpInfo->cmdData.cmdFdb.MacDa,6);
			if(npd_cmd_write_ro_rstp(stpInfo,sizeof(NPD_CMD_STC)) == -1)
			{
				syslog_ax_rstp_dbg("Failed to sync mac to rstp\n");
				return NPD_FAIL;
			}
			break;
		case STP_STATE_UPDATE_E:
			if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
				return NPD_FAIL;
			}
			stp_port.mstid = stpInfo->cmdData.cmdStp.mstid;
			stp_port.port_index = stpInfo->cmdData.cmdStp.portIdx;
			ret = dbtable_hash_search(ports_stp, &stp_port, NULL, &stp_port);
            if(0 != ret)
            {
                syslog_ax_rstp_dbg("Can not find stp_port from ports_stp, portindex is %d\n",
                                stpInfo->cmdData.cmdStp.portIdx);
                return NPD_FAIL;
            }
			switch(stpInfo->cmdData.cmdStp.portState)
			{
				case NAM_STP_PORT_STATE_DISCARD_E:
                    stp_port.state = NAM_STP_PORT_STATE_DISCARD_E;
					npd_key_database_lock();
                    dbtable_hash_update(ports_stp, NULL, &stp_port);
                    if(stp_port.stp_en)
                    {
                        netif_notify_event(stp_port.port_index, PORT_NOTIFIER_DISCARD);
                    }
					npd_key_database_unlock();
					break;
				case NAM_STP_PORT_STATE_LEARN_E:
					npd_key_database_lock();
                    stp_port.state = NAM_STP_PORT_STATE_LEARN_E;
                    dbtable_hash_update(ports_stp, NULL, &stp_port);
					npd_key_database_unlock();
                    break;
				case NAM_STP_PORT_STATE_FORWARD_E:
					npd_key_database_lock();
                    stp_port.state = NAM_STP_PORT_STATE_FORWARD_E;
                    dbtable_hash_update(ports_stp, NULL, &stp_port);
                    if(stp_port.stp_en)
                    {
                        netif_notify_event(stp_port.port_index, PORT_NOTIFIER_FORWARDING);
                    }
					npd_key_database_unlock();
					break;
				case NAM_STP_PORT_STATE_DISABLE_E:
					/*仅在bridge进程从一个stpm删除端口时调用，不必处理*/
					/*
                    stp_port.state = NAM_STP_PORT_STATE_DISABLE_E;
                    dbtable_hash_update(ports_stp, NULL, &stp_port);
                    */
                    break;
				default:
					break;
			}
			break;
		case STP_RECEIVE_TCN_E:
			if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
				return NPD_FAIL;
			}
            stp_port.mstid = stpInfo->cmdData.cmdStp.mstid;
            stp_port.port_index = stpInfo->cmdData.cmdStp.portIdx;
			ret = dbtable_hash_search(ports_stp, &stp_port, NULL, &stp_port);
            if(0 != ret)
            {
                return NPD_FAIL;
            }
			npd_key_database_lock();
            netif_notify_event(stp_port.port_index, PORT_NOTIFIER_STPTC);
			npd_key_database_unlock();
			break;
		default:
			syslog_ax_rstp_dbg("Unkown msg.\n");
			return NPD_FAIL;
	}
	return 0;
}

/*******************************************************************************
 * npd_mstp_vlan_stpid_bind
 *
 * DESCRIPTION:
 *   	vlan's stpid is mstid,while mstp running
 *
 * INPUTS:
 * 	NONE
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	TRUE   	- on success
 * 	FALSE	- otherwise
 *
 *
 ********************************************************************************/
int npd_mstp_vlan_stpid_bind
(
	unsigned short vid,
	unsigned int 	mstid	
)
{
	int ret = 0;
    struct vlan_stp_s vlan_stp = {0};
    unsigned int vlan_id = vid;

    /*这个函数不必在向bridge进程发送端口增加，bridge进程会自行处理*/
    vlan_stp.vid = vid;
    ret = dbtable_sequence_search(vlans_stp, vlan_id, &vlan_stp);
    if(0 == ret)
    {
        if(0 != vlan_stp.mstid)
        {
            struct vlan_s vlan = {0};
            npd_pbmp_t pbmp;
            unsigned int switch_port;

            vlan.vid = vid;
            ret = dbtable_sequence_search(g_vlans, vid, &vlan);
            NPD_PBMP_ASSIGN(pbmp, vlan.untag_ports);
            NPD_PBMP_OR(pbmp, vlan.tag_ports);
            NPD_PBMP_ITER(pbmp, switch_port)
            {
                unsigned int netif_index;
                stp_info_db_t stp_info = {0};
                
                npd_switch_port_netif_index(switch_port, &netif_index);
            	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
            		continue;
                stp_info.port_index = netif_index;
                stp_info.mstid = vlan_stp.mstid;
                ret = dbtable_hash_search(ports_stp, &stp_info, NULL, &stp_info);
                if(0 == ret)
                {
                    if(stp_info.port_index_count > 1)
                    {
						/*一个port属于多个vlan,多个vlan属于一个mstid的时候处理vlan的计数*/
                        stp_info.port_index_count--;
                        dbtable_hash_update(ports_stp, NULL, &stp_info);
                    }
                    else
                    {
                        dbtable_hash_delete(ports_stp, &stp_info, &stp_info);
                    }
                }
            }
        }
    }
    
    vlan_stp.vid = vid;
    vlan_stp.mstid = mstid;

    ret = dbtable_sequence_insert(vlans_stp, vlan_id, &vlan_stp);

    if(0 != mstid)
    {
        struct vlan_s vlan = {0};
        int ret2 = 0;
        npd_pbmp_t pbmp;
        unsigned int switch_port;

        vlan.vid = vid;
        ret2 = dbtable_sequence_search(g_vlans, vid, &vlan);
        NPD_PBMP_ASSIGN(pbmp, vlan.untag_ports);
        NPD_PBMP_OR(pbmp, vlan.tag_ports);
        NPD_PBMP_ITER(pbmp, switch_port)
        {
            unsigned int netif_index;
            stp_info_db_t stp_info = {0};            
            stp_info_db_t stp_info_temp = {0};
            int link_status;
            
            ret2 = npd_check_netif_status(netif_index, &link_status);
			netif_index = netif_array_index_to_ifindex(switch_port);
        	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
        		continue;
            stp_info.port_index = netif_index;
            stp_info.mstid = mstid;
            ret = dbtable_hash_search(ports_stp, &stp_info, NULL, &stp_info);
			stp_info.stp_en = link_status;
			
			if(ret == 0)
			{
				/*已经作为其他的VLAN属于此mstid,仅仅增加端口计数,其他不变*/
			    stp_info.port_index_count++;
				ret = dbtable_hash_insert(ports_stp, &stp_info_temp);
			}
			else
			{
			    /*此端口第一次属于此mstid,初始化端口计数， state设置为BLOCKING*/
			    stp_info.port_index_count = 1;
				stp_info.state = NAM_STP_PORT_STATE_DISCARD_E;
                ret = dbtable_hash_insert(ports_stp, &stp_info);
			}
        }
    }   
    return ret;
    
}

/*******************************************************************************
 * npd_stp_vlan_stpid_bind_init
 *
 * DESCRIPTION:
 *   	init vlan's stpid to 0
 *
 * INPUTS:
 * 	NONE
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	TRUE   	- on success
 * 	FALSE	- otherwise
 *
 *
 ********************************************************************************/
int npd_stp_vlan_stpid_bind_init(void)
{
	unsigned short i = 0;
	unsigned int stpId = 0,ret = 0;
    struct vlan_s *vlan;
    npd_pbmp_t bmp;
    unsigned int switch_port_index;
    struct switch_port_db_s switch_port = {0};

    /*add all ports to stp protocol cist, 
       先初始化好底层，避免在此过程中bridge进程发送的状态数据无法找到数据结构*/
    for(i = 0; i < dbtable_array_totalcount(switch_ports); i++)
    {
        int link_status;
        ret = dbtable_array_get(switch_ports, i, &switch_port);
        if(0 != ret)
            continue;
    	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(switch_port.global_port_ifindex))
    		continue;
        ret = npd_check_netif_status(switch_port.global_port_ifindex, &link_status);
        if(0 == ret)
        {
            struct stp_info_db_s stpInfo = {0};
            stpInfo.port_index = switch_port.global_port_ifindex;
    	    stpInfo.stp_en = link_status;
            stpInfo.mstid = 0;
            stpInfo.state = NAM_STP_PORT_STATE_DISCARD_E;
            ret = dbtable_hash_insert(ports_stp, &stpInfo);
            npd_mstp_add_port(0, switch_port.global_port_ifindex);
            if(link_status)
                npd_rstp_link_change(switch_port.global_port_ifindex, PORT_NOTIFIER_LINKUP_E);
        }  
    }
 
    /*firstly inform stp protocol to add exist vlan*/
	for(i = 1; i < 4094; i++) {
        vlan = npd_find_vlan_by_vid(i);
        /*
        add port to mstp;
        */
        if(NULL == vlan)
            continue;
        ret = npd_mstp_vlan_stpid_bind(i, stpId);
        npd_mstp_add_vlan_on_mst(i);
        NPD_PBMP_ASSIGN(bmp, vlan->tag_ports);
        NPD_PBMP_OR(bmp, vlan->untag_ports);
        NPD_PBMP_ITER(bmp, switch_port_index)
        {
            int ret;
            switch_port.switch_port_index = switch_port_index;
            ret = dbtable_array_get(switch_ports, switch_port_index, &switch_port);
            if(0 != ret)
                continue;
        	if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(switch_port.global_port_ifindex))
        		continue;
            npd_mstp_add_port(i, switch_port.global_port_ifindex);
        }
        free(vlan);
	}
    /*在初始化阶段，需要留出充足的时间，此处以后还是需要优化，用较少的通信完成STP vlan和端口关系通知*/
	if(!npd_startup_end)
        sleep(10);
    else
		sleep(1);
	return 0;
}


DBusMessage * npd_dbus_ethports_interface_config_stp(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int 	netif_index = 0;
	int ret = 0;
	unsigned int	stp_enable;
	DBusError err;
	unsigned int mode = STP_MODE;
	NPD_RECV_INFO  stpInfo;
	memset(&stpInfo,0,sizeof(stpInfo));
	
	syslog_ax_rstp_dbg("Entering config ethport stp!!!!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&mode,
		DBUS_TYPE_UINT32,&stp_enable,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_rstp_dbg("index %#0x %s",netif_index,stp_enable ? "enable":"disable");
	ret = npd_netif_config_stp(netif_index,mode,stp_enable);

	if(NPD_SUCCESS != ret) {
		syslog_ax_rstp_err("npd eth port %#x config stp fail",netif_index);
	}

		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &ret);
	

	return reply;
}

DBusMessage *npd_dbus_stp_set_stpid_for_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	unsigned short vid = 0;
	unsigned int mstid = 0;
	int ret = NPD_SUCCESS;

	syslog_ax_rstp_dbg("Entering stp set stpid!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&vid,
		DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_INVALID))) {
		syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_rstp_dbg("bind stpid %d to vlanid %d\n",mstid,vid);
    npd_key_database_lock();
	ret = npd_mstp_vlan_stpid_bind(vid,mstid);
    npd_key_database_unlock();
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply; 
}


DBusMessage * npd_dbus_stp_set_port_pathcost(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusError err;
	struct stp_info_db_s stpInfo;
	unsigned int netif_index = 0,value = 0,mstid = 0;
	int ret = NPD_SUCCESS;

	 syslog_ax_rstp_dbg("Entering stp set port pathcost!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;

    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);

    if(-1 == ret)
    {
        ret = STP_RETURN_CODE_PORT_NOT_ENABLED;
        goto error;
    }
    stpInfo.pathcost = value;
	if(mstid > 0)
	   stpInfo.mode = MST_MODE;
	ret = dbtable_hash_update(ports_stp, NULL, &stpInfo);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
error:
	return reply; 
}

DBusMessage * npd_dbus_stp_set_port_prio(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusError err;
	struct stp_info_db_s stpInfo;
	unsigned int netif_index = 0,value = 0,mstid = 0;
	int ret = NPD_SUCCESS;

	 syslog_ax_rstp_dbg("Entering stp set port pathcost!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;

    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);

    if(-1 == ret)
    {
        ret = STP_RETURN_CODE_PORT_NOT_ENABLED;
        goto error;
    }
    stpInfo.prio = value;
	if(mstid > 0)
	   stpInfo.mode = MST_MODE;
	ret = dbtable_hash_update(ports_stp, NULL, &stpInfo);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
error:
	return reply; 
}
DBusMessage * npd_dbus_stp_set_port_nonstp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusError err;
	struct stp_info_db_s stpInfo;
	unsigned int netif_index = 0,value = 0,mstid = 0;
	int ret = NPD_SUCCESS;

	 syslog_ax_rstp_dbg("Entering stp set port pathcost!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;

    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);

    if(-1 == ret)
    {
        ret = STP_RETURN_CODE_PORT_NOT_ENABLED;
        goto error;
    }
    stpInfo.nonstp = value;
	if(mstid > 0)
	   stpInfo.mode = MST_MODE;
	ret = dbtable_hash_update(ports_stp, NULL, &stpInfo);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
error:
	return reply; 
}

DBusMessage * npd_dbus_stp_set_port_p2p(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusError err;
	struct stp_info_db_s stpInfo;
	unsigned int netif_index = 0,value = 0,mstid = 0;
	int ret = NPD_SUCCESS;

	 syslog_ax_rstp_dbg("Entering stp set port pathcost!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;

    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);

    if(-1 == ret)
    {
        ret = STP_RETURN_CODE_PORT_NOT_ENABLED;
        goto error;
    }
    stpInfo.p2p = value;
	if(mstid > 0)
	   stpInfo.mode = MST_MODE;
	ret = dbtable_hash_update(ports_stp, NULL, &stpInfo);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);

error:    
	return reply; 
}

DBusMessage * npd_dbus_stp_set_port_edge(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply = NULL;
	DBusError err;
	struct stp_info_db_s stpInfo;
	unsigned int netif_index = 0,value = 0,mstid = 0;
	int ret = NPD_SUCCESS;

	 syslog_ax_rstp_dbg("Entering stp set port pathcost!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&mstid,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    stpInfo.port_index = netif_index;
    stpInfo.mstid = mstid;

    ret = dbtable_hash_search(ports_stp, &stpInfo, NULL, &stpInfo);

    if(-1 == ret)
    {
        ret = STP_RETURN_CODE_PORT_NOT_ENABLED;
        goto error;
    }
    stpInfo.edge = value;
	if(mstid > 0)
	   stpInfo.mode = MST_MODE;
	ret = dbtable_hash_update(ports_stp, NULL, &stpInfo);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
error:    
	return reply; 
}


DBusMessage * npd_dbus_stp_show_running_cfg(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	return NULL;
}

DBusMessage * npd_dbus_stp_all_vlans_bind_to_stpid(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;
	int ret = NPD_SUCCESS;
	unsigned int enable = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args (msg,&err,
						DBUS_TYPE_UINT32,&enable,
						DBUS_TYPE_INVALID))) {
		syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(1 == enable){
            npd_key_database_lock();
			ret = npd_stp_vlan_stpid_bind_init();
            npd_key_database_unlock();
		    syslog_ax_rstp_dbg("npd_stp_vlan_stpid_bind_init value is %d\n",ret);
		}
	else if(0 == enable)
	{
        npd_key_database_lock();
		npd_stp_struct_init();
        npd_key_database_unlock();
	}
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage * npd_dbus_stp_get_port_link_state(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;
	int lk = 0;
	int ret = NPD_SUCCESS;
	unsigned int netif_index = 0;

	
	 syslog_ax_rstp_dbg("Entering get stp port link state!\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = npd_check_netif_status(netif_index, &lk);

    
	syslog_ax_rstp_dbg("STP get port links status %s\n",lk ? "up" : "down");
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_UINT32,&lk,
							 DBUS_TYPE_INVALID);
	
	return reply;
}


DBusMessage * npd_dbus_stp_get_port_speed(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;

	int ret = NPD_SUCCESS;
	unsigned int netif_index = 0;
	int speed = 0;
	
	 syslog_ax_rstp_dbg("Entering get stp port link state!\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = npd_netif_speed(netif_index, &speed);
	
	/*printf("npd_dbus 6338 >> lkstae %s\n",lk ? "up" : "down");*/
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INT32,&speed,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage * npd_dbus_stp_get_port_duplex_mode(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;

	int ret = NPD_SUCCESS;
	unsigned int netif_index = 0;
	int duplex_mode = 0;
	
	syslog_ax_rstp_dbg("Entering get stp port link state!\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&netif_index,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret =  npd_netif_duplex_mode(netif_index, &duplex_mode);

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INT32,&duplex_mode,
							 DBUS_TYPE_INVALID);
	
	return reply;
}


DBusMessage * npd_dbus_rstp_get_one_port_index(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned char	slot_no,port_no;
	unsigned int 	eth_g_index = 0;
	int ret;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_BYTE,&slot_no,
		DBUS_TYPE_BYTE,&port_no,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_rstp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = NPD_DBUS_ERROR_NO_SUCH_PORT;
	
	if (CHASSIS_SLOTNO_ISLEGAL(slot_no)) {
		if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no,port_no)) {
			eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no,port_no);
			ret = NPD_DBUS_SUCCESS;
		}
	}
		
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &eth_g_index);
	return reply;
}

int npd_netif_travesal_delete
(
    hash_table_index_t *hash,
	void *data,
    unsigned int flag
)
{
    int ret;
    struct stp_info_db_s *stpInfo = (struct stp_info_db_s *)data;
	
    syslog_ax_rstp_dbg("Delete netif %x, mstid %d\n", 
        stpInfo->port_index, stpInfo->mstid);
    
    ret = dbtable_hash_delete(hash, stpInfo, stpInfo);
    return ret;
}

int npd_vlan_travesal_delete
(
    sequence_table_index_t *sequence,
	void *data,
    unsigned int flag
)
{
    int ret;
    struct vlan_stp_s *vlan_stp = (struct vlan_stp_s *)data;
	
    ret = dbtable_sequence_delete(sequence, vlan_stp->vid, vlan_stp, vlan_stp);
    return ret;
}

int npd_stp_struct_init(){
    int ret;

    ret = dbtable_hash_traversal(ports_stp, 0, NULL, NULL, npd_netif_travesal_delete);
    syslog_ax_rstp_dbg("Delete %d ports stp\n", ret);

    
    ret = dbtable_sequence_traversal(vlans_stp, 0, NULL, NULL, npd_vlan_travesal_delete);
    syslog_ax_rstp_dbg("Delete %d vlans stp\n", ret);
 
	return 0;
}

#ifdef __cplusplus
}
#endif
#endif

