#ifdef __cplusplus
extern "C"
{
#endif
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include "bcm/types.h"
#include "bcm/stat.h"
#include "bcm/switch.h"
#include "bcm/stack.h"
#include "bcm/trunk.h"
#include "bcm/vlan.h"
#include "bcm/port.h"
#include "bcm/mcast.h"
#include "bcm/ipmc.h"
#include "bcm/error.h"
#include "bcm/stg.h"
#include "soc/drv.h"
#include "sal/appl/sal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>

#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "nbm/nbm_api.h"
#include "npd_log.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "npd_database.h"
#include "npd_switch_port.h"
#include "board/ts_product_feature.h"

#define SDK_DIFF_TRUNK 121

extern unsigned long nam_asic_get_instance_num(void);


long tseries_linecard_local_conn_init(int product_type)
{
    int unit;
    int rv;
    int module_id, modnum;
    int port;
	int hash_control;

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

        /*set modid*/
        rv = bcm_stk_modid_count(unit, &modnum);
        
        rv = bcm_stk_my_modid_set(unit, module_id);
        if(BCM_E_NONE != rv)
        {
            npd_syslog_dbg("MODID set error, msg %s\n",
                bcm_errmsg(rv));
        }

        /*Then set forward between stack ports*/
        /*cpu only receive local module packets*/
        {
            bcm_pbmp_t allow_ports;
            BCM_PBMP_ASSIGN(allow_ports, PBMP_ALL(unit));
            BCM_PBMP_PORT_REMOVE(allow_ports, CMIC_PORT(unit));
			/*
            npd_syslog_dbg("%s %d port modid egress set: ports 0x%x, modid: %d, egress ports 0x%x \r\n", 
			    __func__, __LINE__, -1, -1, allow_ports.pbits[0]);
			    */

            rv = bcm_port_egress_set(unit, -1, -1, allow_ports);
            if(BCM_E_NONE != rv)
            {
                npd_syslog_err("Crossbar init: cpu port egress set error for all module, err %s\n",
                    bcm_errmsg(rv));
            }
			/*
            npd_syslog_dbg("%s %d port modid egress set: ports 0x%x, modid: %d, egress ports 0x%x \r\n", 
			    __func__, __LINE__, -1, module_id, PBMP_ALL(unit));
			    */
            rv = bcm_port_egress_set(unit, -1, module_id, PBMP_ALL(unit));
            if(BCM_E_NONE != rv)
            {
                npd_syslog_err("Crossbar init: cpu port egress set error for module %d err %s\n",
                    module_id,bcm_errmsg(rv));
            }
            if(modnum > 1)
            {
                rv = bcm_port_egress_set(unit, -1, module_id+1, PBMP_ALL(unit));
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Crossbar init: cpu port egress set error for module %d err %s\n",
                        module_id+1,bcm_errmsg(rv));
                }
            }

        }

    
        /*rtag7 port trunking load banlance id set*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field0, 
                0xfc0fff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field0, 
                0xfcf0ff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field0, 
                0xfe000f);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed0, sal_rand());
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed1, (unsigned int)-1 - sal_rand());
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField0Config, BCM_HASH_FIELD_CONFIG_CRC16XOR8);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField1Config, BCM_HASH_FIELD_CONFIG_CRC16XOR8);
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet0UnicastOffset, 0);/*hash A, 5bit*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet1UnicastOffset, 16); /*souce port lbn*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0UnicastOffset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1UnicastOffset, 16);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0NonUnicastOffset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1NonUnicastOffset, 16);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet0Offset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet1Offset, 16);
    	bcm_switch_control_port_set(unit, 0, bcmSwitchL2PortBlocking, 1);

		bcm_switch_control_port_get(unit, 0, bcmSwitchHashControl, &hash_control);
		hash_control |= BCM_HASH_CONTROL_MULTIPATH_L4PORTS;
		bcm_switch_control_port_set(unit, 0, bcmSwitchHashControl, hash_control);		

		PBMP_E_ITER(unit, port)
        {
			bcm_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
			bcm_port_control_set(unit, port, bcmPortControlTrunkHashSet, 1);				
        }
        PBMP_HG_ITER(unit, port)
        {
			bcm_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
			bcm_port_control_set(unit, port, bcmPortControlTrunkHashSet, 1);				
				
			//bcm_proxy_server_set(unit, port, BCM_PROXY_MODE_HIGIG, 1);
			bcm_port_control_set(unit, port, bcmPortControlL2Learn, 0x4);			
			bcm_port_control_set(unit, port, bcmPortControlL2Move, 0x4);
			bcm_port_control_set(unit, port, bcmPortControlForwardStaticL2MovePkt, 1);

			bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 0);
			bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 0);				

			bcm_port_flood_block_set(unit, port, CMIC_PORT(unit), 0x8);			
        }
    }
    return BCM_E_NONE;  
}

typedef struct bcm_conn_func_s
{
	int unit;  /*the number of PCI or Others bus*/
	int port; /*the port*/
	int peer_slot;
	int peer_board_type;
	int peer_switch_type;
	int peer_modid;    /*the destination modid*/
	int peer_unit;
	int peer_unit_port;
	int port_type;
}bcm_conn_func_t;

typedef struct _ts_board_conn_element_
{
	unsigned char local_dev;
	unsigned char local_port;
	unsigned char peer_mod;
	unsigned char peer_port;
	int peer_type;
	int peer_slot;
	unsigned short trunk_member;
	unsigned short tid;
	unsigned short is_dest_port;
	unsigned short is_src_port;
	unsigned short need_redirect;
	unsigned short redirect_from_port;
	int port_type;
}ts_board_conn_element;

ts_board_conn_element ts_board_conn[32];
/**********************************************************/
extern hash_table_index_t *switch_ports_hash;

int tseries_linecard_vlan_entry_del(unsigned char devNum, unsigned char portNum)
{
	int ret;
	unsigned int netif_index;
	switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));

	if(NULL == switch_port)
	{
        return NPD_FAIL;
	}

	ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
	if(ret == 0)
	{
	    struct eth_port_s *portInfo = NULL;
		unsigned int switchport_netif = netif_index;
		npd_key_database_lock();
	    portInfo = npd_get_port_by_index(netif_index);
		if(portInfo == NULL)
		{
		    npd_key_database_unlock();
			free(switch_port);
		    return 0;
		}
		if(portInfo->trunkid != -1)
		{
		    switchport_netif = npd_netif_trunk_get_index(portInfo->trunkid);
		}

		switch_port->global_port_ifindex = switchport_netif;
		ret = dbtable_hash_search(switch_ports_hash, (void*)switch_port, NULL, switch_port);
		if(ret == 0)
		{
			int vlan;
			bcm_pbmp_t pbmp, ut_pbmp;
			
			NPD_VBMP_ITER(switch_port->allow_tag_vlans, vlan)
	        {
	            {
	                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x tagged\n",
	                    vlan, switch_port->global_port_ifindex);
					BCM_PBMP_CLEAR(pbmp);
					BCM_PBMP_CLEAR(ut_pbmp);

					BCM_PBMP_PORT_ADD(pbmp, portNum);
					
	                ret = bcm_vlan_port_remove(devNum, vlan, pbmp);
					ret = bcm_vlan_port_add(devNum, vlan, pbmp, ut_pbmp);	
	            }
	        }

			NPD_VBMP_ITER(switch_port->allow_untag_vlans, vlan)
	        {
	            {
	                syslog_ax_vlan_dbg("Operate vlan %d add port 0x%x untagged\n",
	                    vlan, switch_port->global_port_ifindex);
	                					BCM_PBMP_CLEAR(pbmp);
					BCM_PBMP_CLEAR(pbmp);
					BCM_PBMP_CLEAR(ut_pbmp);

					BCM_PBMP_PORT_ADD(pbmp, portNum);
					BCM_PBMP_PORT_ADD(ut_pbmp, portNum);
					
	                ret = bcm_vlan_port_remove(devNum, vlan, pbmp);
					ret = bcm_vlan_port_add(devNum, vlan, pbmp, ut_pbmp);
	            }
	        }
		}
	    free(portInfo);
		npd_key_database_unlock();
	}
	free(switch_port);
	return 0;
}

int tseries_linecard_vlan_entry_add(unsigned char devNum, unsigned char portNum)
{
	int ret;
	unsigned short vlanId;
	bcm_pbmp_t pbmp, ut_pbmp;
	unsigned int netif_index;
	
	BCM_PBMP_CLEAR(pbmp);
	BCM_PBMP_CLEAR(ut_pbmp);
	ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
	if(ret == 0)
	{
		ret = npd_check_eth_port_status(netif_index);
		if(ret != -1)
		{
		    return 0;
		}
	}
	else
	{
	    return 0;
	}
		
	BCM_PBMP_PORT_ADD(pbmp, portNum);

	for (vlanId = 1; vlanId < 4095; vlanId++)
	{
	    ret = bcm_vlan_port_remove(devNum, vlanId, pbmp);
		ret = bcm_vlan_port_add(devNum, vlanId, pbmp, ut_pbmp);	
	}

	return 0;
}

int tseries_linecard_cscd_trunk_ports_del(unsigned char devNum, unsigned char portNum)
{
	int ret = 0;
	
	return ret;
}

void tseries_linecard_port_config(unsigned char devNum, unsigned char portNum, int portType)
{
	if (PLANE_PORTTYPE_USERETH == portType)
	{
		int ret = 0;
		unsigned int netif_index = 0;
		npd_syslog_dbg("NOPP Port Config: Port(%d, %d)\n", devNum, portNum);
		
    	ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
    	if(ret != 0)
    	{
    	    bcm_port_enable_set(devNum, portNum, 0);
    	}
		else
		{
            struct eth_port_s* portInfo = NULL;
		    npd_key_database_lock();
            portInfo = npd_get_port_by_index(netif_index);
    		if(portInfo == NULL)
    		{
    		    bcm_port_enable_set(devNum, portNum, 0);
    		}
			else
			{
			    if((portInfo->attr_bitmap & ETH_ATTR_ADMIN_STATUS) && (npd_startup_end))
			    {
			        bcm_port_enable_set(devNum, portNum, 1);
			    }
				else
				{
				    bcm_port_enable_set(devNum, portNum, 0);
				}
				free(portInfo);
			}
			npd_key_database_unlock();
		}
		
        bcm_port_ifg_set(devNum, portNum, 10000, BCM_PORT_DUPLEX_FULL, 8*8);

		bcm_port_encap_set(devNum, portNum, BCM_PORT_ENCAP_IEEE);

		bcm_port_control_set(devNum, portNum, bcmPortControlL2Learn, 0x5);  
		bcm_port_control_set(devNum, portNum, bcmPortControlL2Move, 0x5);  
		bcm_port_control_set(devNum, portNum, bcmPortControlForwardStaticL2MovePkt, 1);
	}
	else if (PLANE_PORTTYPE_STACK == portType)
	{
		npd_syslog_dbg("DSA Port Config: Port(%d, %d)\n", devNum, portNum);
		
		bcm_port_enable_set(devNum, portNum, 0);
		
        bcm_port_ifg_set(devNum, portNum, 12000, BCM_PORT_DUPLEX_FULL, 8*8);

		bcm_port_encap_set(devNum, portNum, BCM_PORT_ENCAP_HIGIG);

		bcm_port_control_set(devNum, portNum, bcmPortControlL2Learn, 0x4);  /* FWD */
		bcm_port_control_set(devNum, portNum, bcmPortControlL2Move, BCM_PORT_LEARN_SWITCH);  /* FWD */
		bcm_port_control_set(devNum, portNum, bcmPortControlForwardStaticL2MovePkt, 1);
	}
	else if (PLANE_PORTTYPE_STACKETH == portType)
	{
		npd_syslog_dbg("SDKDIFF Port Config: Port(%d, %d)\n", devNum, portNum);
		
		bcm_port_enable_set(devNum, portNum, 0);
		
        bcm_port_ifg_set(devNum, portNum, 12000, BCM_PORT_DUPLEX_FULL, 8*8);

		bcm_port_encap_set(devNum, portNum, BCM_PORT_ENCAP_IEEE);

		bcm_port_control_set(devNum, portNum, bcmPortControlL2Learn, 0x4);  /* FWD */
		bcm_port_control_set(devNum, portNum, bcmPortControlL2Move, BCM_PORT_LEARN_SWITCH);  /* FWD */
		bcm_port_control_set(devNum, portNum, bcmPortControlForwardStaticL2MovePkt, 1);
		
		BCM_PBMP_PORT_ADD(PBMP_ST_ALL(devNum), portNum);
		bcm_port_stp_set(devNum, portNum, BCM_STG_STP_FORWARD);
	}
}


int npd_get_modport_tid_by_global_index(unsigned int globle_index, int *tid, 
												unsigned char *mod, unsigned char *port)
{
	int ret = 0;
 	unsigned int eth_g_index[8];
	unsigned int eth_count = 0;
	int count;
	unsigned int peer_slot, peer_type;
	
	*tid = 0;
	if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(globle_index))
	{
	    *tid = npd_netif_trunk_get_tid(globle_index);
        if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
        {
			return 0;
        }
		if(*tid == SDK_DIFF_TRUNK)
		{
		    return -1;
		}
        ret = npd_trunk_member_port_index_get_all(*tid, eth_g_index, &eth_count);
		if(ret == NPD_TRUE)
		{
			for(count = 0; count < eth_count; count++)
			{
                ret = npd_get_modport_by_global_index(eth_g_index[count], mod, port);
                if (ret == 0)
                {
                    peer_slot = MODULE_2_SLOT_INDEX(*mod);
					if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
					{
						npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
						continue;
					}
					
					peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
					if(peer_type == 0)
					{
						npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
						continue;
					}
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
						*mod = 0;
						*port = 0;
						*tid = SDK_DIFF_TRUNK;
						return 0;
					}
                }
			}

			*mod = 0;
			*port = 0;
			return 0;
		}
		else
		{
		    *mod = 0;
			*port = 0;
		    return 0;
		}
	}
	else
	{
    	ret = npd_get_modport_by_global_index(globle_index, mod, port);
        if (0 != ret)
        {
            npd_syslog_dbg("eth_index %#x get asic port failed for ret %d\n",globle_index,ret);
            return -1;
        }
        if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
        {
			*tid = 0;
			return 0;
        }
		peer_slot = MODULE_2_SLOT_INDEX(*mod);
		if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
		{
			if(!MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(peer_slot))
			{
				*tid = 0;
				return 0;
			}
		}
		
		peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
		if(peer_type == 0)
		{
			*tid = 0;
			return 0;
		}

		if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
		{
			*tid = SDK_DIFF_TRUNK;
			return 0;
		}

		*tid = 0;
    }
	
	return 0;
}



long tseries_linecard_fullmesh_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    int unit;
    int rv;
    int tid = -1;
    int modid;
    int i, j;
    int my_type, my_slot;
    int unit_port;
    int plane_port;
    int peer_slot;
    int peer_port;
    int peer_type;
    int peer_unit = -1;
	int peer_modid = -1;
    int tid_base[2] = {0, 0};
    bcm_trunk_chip_info_t trunk_info;
    bcm_trunk_add_info_t trunk_data;
	
    memset(ts_board_conn, 0, sizeof(ts_board_conn));
    my_type = SYS_LOCAL_MODULE_TYPE;
    my_slot = SYS_LOCAL_MODULE_SLOT_INDEX;

    /*for full mesh, firstly delete all trunks*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_trunk_chip_info_get(unit, &trunk_info);
        for(tid = trunk_info.trunk_fabric_id_min;
                   tid <= trunk_info.trunk_fabric_id_max;
                   tid++)
        {
            rv = bcm_trunk_get(unit, tid, &trunk_data);
            if(BCM_E_NOT_FOUND != rv)
            {
                bcm_trunk_destroy(unit, tid);
            }
        }
    }

	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
        unit = PPAL_PLANE_2_UNIT(my_type, i);
        if(-1 == unit)
		{
            continue;
        }

        unit_port = PPAL_PLANE_2_PORT(my_type, i);

		memset(&ts_board_conn[i], 0, sizeof(ts_board_conn_element));
		ts_board_conn[i].local_dev = unit;
		ts_board_conn[i].local_port = unit_port;
		
        plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
               unit, unit_port);
        if(-1 == plane_port)
        {
            continue;
        }

		if(BOARD_INNER_CONN_PORT == plane_port)
		{
			/*board inner conn case*/
			npd_syslog_dbg("%s %d: port %d:%d is board inner conn port.\r\n", __func__, __LINE__, unit, unit_port);
			if(unit == 0)
			{
                ts_board_conn[i].peer_mod = 
					UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);
			}
			else
			{
                ts_board_conn[i].peer_mod = 
					UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
			}
			ts_board_conn[i].peer_slot = SYS_LOCAL_MODULE_SLOT_INDEX;
			ts_board_conn[i].peer_type = SYS_LOCAL_MODULE_TYPE;
			ts_board_conn[i].port_type = PLANE_PORTTYPE_STACK;
			continue;
		}
		
        peer_slot = SLOT_PORT_PEER_SLOT(my_slot, i);

        if(-1 == peer_slot)
        {
            continue;
        }
		
		if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
		{
			npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
			ts_board_conn[i].peer_slot = -1;
			continue;
		}
		
		peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
		if(peer_type == 0)
		{
			npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
			ts_board_conn[i].peer_slot = -1;
			continue;
		}
		peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
		peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);
		if(peer_unit == -1 && (SYS_MODULE_ISHAVEPP(peer_type)))
		{
			ts_board_conn[i].peer_slot = -1;
			ts_board_conn[i].peer_mod = -1;
			continue;
		}
		peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
		
		ts_board_conn[i].peer_type = peer_type;
		ts_board_conn[i].peer_slot = peer_slot;
		
		if(!SYS_MODULE_ISHAVEPP(peer_type))
		{
			/*No pp case*/
			npd_syslog_dbg("%s %d: Peer module type %d, is a Non-pp board.\r\n", __func__, __LINE__, peer_type);
			ts_board_conn[i].is_dest_port = 1;
			ts_board_conn[i].port_type = PLANE_PORTTYPE_USERETH;
			tseries_linecard_port_config(unit, unit_port, PLANE_PORTTYPE_USERETH);
		    tseries_linecard_vlan_entry_del(unit, unit_port);
			/**/
		}
		else if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
		{
			/*Different SDK type case*/
			npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
			ts_board_conn[i].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
			ts_board_conn[i].peer_port = peer_port;
			npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
				ts_board_conn[i].peer_mod, ts_board_conn[i].peer_port);
			ts_board_conn[i].port_type = PLANE_PORTTYPE_STACKETH;
			tseries_linecard_port_config(unit, unit_port, PLANE_PORTTYPE_STACKETH);
		    tseries_linecard_vlan_entry_add(unit, unit_port);
		}
		else
		{
			ts_board_conn[i].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
			ts_board_conn[i].peer_port = peer_port;
			npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
				ts_board_conn[i].peer_mod, ts_board_conn[i].peer_port);

			ts_board_conn[i].port_type = PLANE_PORTTYPE_STACK;
			tseries_linecard_port_config(unit, unit_port, PLANE_PORTTYPE_STACK);
		    tseries_linecard_vlan_entry_add(unit, unit_port);
		}
	}
    /*广播流量的路径*/
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
    	if(ts_board_conn[i].peer_slot == -1 || 
				ts_board_conn[i].peer_type == 0)
    	{
    		continue;
    	}
		for(j = 0; j < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); j++)
		{
			if(i == j)
			{
				continue;
			}
        	if(ts_board_conn[j].peer_slot == -1 || 
    				ts_board_conn[j].peer_type == 0)
        	{
        		continue;
        	}
			if(ts_board_conn[i].local_dev != ts_board_conn[j].local_dev)
			{
				continue;
			}
    		if(ts_board_conn[j].port_type == PLANE_PORTTYPE_USERETH)
    		{
				bcm_port_flood_block_set(ts_board_conn[i].local_dev, 
						ts_board_conn[i].local_port, ts_board_conn[j].local_port,
						0);
				continue;
    		}
			/* 对于NoPP的情况，不应该往下走*/
			if(ts_board_conn[i].port_type == PLANE_PORTTYPE_USERETH)
			{
				continue;
			}
			/*对于SDKDIFF的情况，不应该往下走*/
			if(ts_board_conn[i].port_type == PLANE_PORTTYPE_STACKETH)
			{
				continue;
			}
			if(ts_board_conn[i].peer_mod == ts_board_conn[j].peer_mod)
			{
				/*should be trunked*/
				ts_board_conn[i].trunk_member = 1;
				ts_board_conn[j].trunk_member = 1;
				if(ts_board_conn[i].tid == 0)
				{
					if(ts_board_conn[j].tid == 0)
					{
						ts_board_conn[i].tid = ts_board_conn[j].tid = trunk_info.trunk_fabric_id_min + tid_base[ts_board_conn[i].local_dev];
						tid_base[ts_board_conn[i].local_dev]++;
					}
					else
					{
						ts_board_conn[i].tid = ts_board_conn[j].tid;
					}
				}
				else
				{
				    ts_board_conn[j].tid = ts_board_conn[i].tid;
				}
			}
			else if(ts_board_conn[i].peer_slot == ts_board_conn[j].peer_slot)
			{
				int h = 0;
				int have_loop_conn = 0;
				int third_peer_port = 0;
				int third_unit = 0, third_unit_port = 0, third_peer_slot = 0, third_peer_unit = 0;
				for(h = 0; h < PPAL_PLANE_PORT_COUNT(ts_board_conn[j].peer_type); h++)
				{
					/*判断是否有内部连接端口*/
                    unit = PPAL_PLANE_2_UNIT(ts_board_conn[j].peer_type, h);
                    if(-1 == unit)
                        continue;
            
                    unit_port = PPAL_PLANE_2_PORT(ts_board_conn[j].peer_type, h);
                    plane_port = PPAL_PHY_2_PLANE(ts_board_conn[j].peer_type,
                           unit, unit_port);
                    if(-1 == plane_port)
                    {
                        continue;
                    }
					
            		if(BOARD_INNER_CONN_PORT == plane_port)
            		{
						npd_syslog_dbg("%s %d: Peer board have inner conn.\r\n", __func__, __LINE__);
						have_loop_conn = 1;
            		}
					/*为了防止双份广播报文,要判断环路存在*/
					else if(UNIT_2_MODULE(ts_board_conn[j].peer_type, ts_board_conn[j].peer_slot, unit, 0) 
						== ts_board_conn[j].peer_mod)
					{
						int k = 0;
						if(unit_port == ts_board_conn[j].peer_port)
						{
							continue;
						}
                        peer_slot = SLOT_PORT_PEER_SLOT(ts_board_conn[j].peer_slot, h);
                
                        if(-1 == peer_slot)
                            continue;
                		
                		if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
                		{
                			continue;
                		}
                		
                		peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
                		if(peer_type == 0)
                		{
                			continue;
                		}
                		peer_port = SLOT_PORT_PEER_PORT(peer_slot, h);
                		peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);
						if(peer_unit == -1)
						{
							continue;
						}
                		peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						if(peer_port == -1)
						{
							continue;
						}
						peer_modid = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
						for(k = 0; k < PPAL_PLANE_PORT_COUNT(peer_type); k++)
						{
                            third_unit = PPAL_PLANE_2_UNIT(peer_type, k);
                            if(-1 == third_unit)
                                continue;
                            if(peer_unit != third_unit)
                            {
								continue;
                            }
							
                            third_unit_port = PPAL_PLANE_2_PORT(peer_type, k);
							if(peer_port == third_unit_port)
							{
								continue;
							}
							
                            third_peer_slot = SLOT_PORT_PEER_SLOT(peer_slot, k);
                            if(third_peer_slot != ts_board_conn[j].peer_slot)
                            {
								continue;
                            }
							
                		    third_peer_port = SLOT_PORT_PEER_PORT(peer_slot, k);
                            third_peer_unit = PPAL_PLANE_2_UNIT(ts_board_conn[j].peer_type, third_peer_port);
							if(UNIT_2_MODULE(ts_board_conn[j].peer_type, ts_board_conn[j].peer_slot, third_peer_unit, 0)
								== ts_board_conn[i].peer_mod)
							{
								if(peer_modid < 
									UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, ts_board_conn[i].local_dev , 0))
								{
									npd_syslog_dbg("%s %d: Have loop.\r\n", __func__, __LINE__);
									have_loop_conn = 1;
								}
								break;
							}
						}
					}
				}
				if(have_loop_conn)
				{
					/*block broadcast/dlf*/
					bcm_port_flood_block_set(ts_board_conn[i].local_dev, 
						ts_board_conn[i].local_port, ts_board_conn[j].local_port,
						(BCM_PORT_FLOOD_BLOCK_BCAST|BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST|BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST));
				}
				else
				{
					bcm_port_flood_block_set(ts_board_conn[i].local_dev, 
						ts_board_conn[i].local_port, ts_board_conn[j].local_port,
						0);
				}
			}
			else
			{
				int h = 0;
				int have_conn = 0;
				for(h = 0; h < PPAL_PLANE_PORT_COUNT(ts_board_conn[j].peer_type); h++)
				{
					/*判断是否有端口与端口i对应的mod相连*/
                    unit = PPAL_PLANE_2_UNIT(ts_board_conn[j].peer_type, h);
                    if(-1 == unit)
                    {
                        continue;
                    }
            
                    unit_port = PPAL_PLANE_2_PORT(ts_board_conn[j].peer_type, h);
                    if(-1 == unit_port)
                    {
                        continue;
                    }
                    plane_port = PPAL_PHY_2_PLANE(ts_board_conn[j].peer_type,
                           unit, unit_port);
                    if(-1 == plane_port)
                    {
                        continue;
                    }
            
                    peer_slot = SLOT_PORT_PEER_SLOT(ts_board_conn[j].peer_slot, h);
            
                    if(-1 == peer_slot)
                    {
                        continue;
                    }
            		
            		if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
            		{
            			continue;
            		}
            		peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
            		if(peer_type == 0)
            		{
            			continue;
            		}
            		peer_port = SLOT_PORT_PEER_PORT(ts_board_conn[j].peer_slot, h);
            		peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);
					if(peer_unit == -1)
					{
						continue;
					}
            		
					peer_modid = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
					if(peer_modid == ts_board_conn[i].peer_mod)
					{
						have_conn = 1;
						break;
					}
				}
				if(have_conn)
				{
					/*block broadcast/dlf*/
					bcm_port_flood_block_set(ts_board_conn[i].local_dev, 
						ts_board_conn[i].local_port, ts_board_conn[j].local_port,
						(BCM_PORT_FLOOD_BLOCK_BCAST|BCM_PORT_FLOOD_BLOCK_UNKNOWN_UCAST|BCM_PORT_FLOOD_BLOCK_UNKNOWN_MCAST));
				}
				else
				{
					bcm_port_flood_block_set(ts_board_conn[i].local_dev, 
						ts_board_conn[i].local_port, ts_board_conn[j].local_port,
						0);
				}
			}
		}
	}

	/* 处理SDKDIFF 情况的端口*/
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
		if(ts_board_conn[i].peer_slot == -1 || 
			ts_board_conn[i].peer_type == 0)
		{
			continue;
		}
		if(ts_board_conn[i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
		{
			continue;
		}
		if(ts_board_conn[i].tid != 0)
		{
			continue;
		}
		/*NoPP的情况不往下走*/
		if(!SYS_MODULE_ISHAVEPP(ts_board_conn[i].peer_type))
		{
			continue;
		}
		if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, ts_board_conn[i].peer_type))
		{
			ts_board_conn[i].tid = SDK_DIFF_TRUNK;
			ts_board_conn[i].trunk_member = 1;
		}
	}

	npd_syslog_dbg("\r\nts_board_conn info \r\n\r\n");
	npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
				   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort  PortType\r\n");
	
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
		npd_syslog_dbg("%-10d", ts_board_conn[i].local_dev);
		npd_syslog_dbg("%-11d", ts_board_conn[i].local_port);
		npd_syslog_dbg("%-9d", ts_board_conn[i].peer_mod);
		npd_syslog_dbg("%-10d", ts_board_conn[i].peer_port);
		npd_syslog_dbg("%-10d", ts_board_conn[i].peer_type);
		npd_syslog_dbg("%-10d", ts_board_conn[i].peer_slot);
		npd_syslog_dbg("%-13d", ts_board_conn[i].trunk_member);
		npd_syslog_dbg("%-5d", ts_board_conn[i].tid);
		npd_syslog_dbg("%-10d", ts_board_conn[i].is_dest_port);
		npd_syslog_dbg("%-9d", ts_board_conn[i].is_src_port);
		npd_syslog_dbg("%-14d", ts_board_conn[i].need_redirect);
		npd_syslog_dbg("%-16d", ts_board_conn[i].redirect_from_port);
		npd_syslog_dbg("%-12d\r\n", ts_board_conn[i].port_type);
	}

	/*trunk*/
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
        int modport;
        int find_port = FALSE;
		if(ts_board_conn[i].trunk_member && ts_board_conn[i].tid)
		{
			if (BCM_E_NOT_FOUND == bcm_trunk_get(ts_board_conn[i].local_dev, ts_board_conn[i].tid, &trunk_data))
			{
                rv = bcm_trunk_create_id(ts_board_conn[i].local_dev, ts_board_conn[i].tid);

                if (rv < 0)
                {
                    npd_syslog_err("%s,%d:Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
                                       __func__, __LINE__, ts_board_conn[i].local_dev, ts_board_conn[i].tid, bcm_errmsg(rv));
                }

                bcm_trunk_add_info_t_init(&trunk_data);
                trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
                trunk_data.flags = BCM_TRUNK_FLAG_FAILOVER_ALL_LOCAL;
                trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
                trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
                /*trunk_data.psc = BCM_TRUNK_PSC_PORTFLOW;*/
                trunk_data.psc = BCM_TRUNK_PSC_SRCDSTMAC;
			}
            bcm_stk_modid_get(ts_board_conn[i].local_dev, &modid);
            modport = ts_board_conn[i].local_port;
            if (ts_board_conn[i].local_port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid += 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == modport)
                {
                    find_port = TRUE;
                    break;
                }
                trunk_data.tm[j] = modid;
            }
            if(find_port)
            {
				continue;
            }
            if (!find_port)
            {
                trunk_data.tp[trunk_data.num_ports] = modport;
                trunk_data.tm[trunk_data.num_ports] = modid;
                trunk_data.num_ports = trunk_data.num_ports + 1;
            }
            rv = bcm_trunk_set(ts_board_conn[i].local_dev, ts_board_conn[i].tid, &trunk_data);

            if (rv < 0)
            {
                npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
                                   ts_board_conn[i].local_dev, ts_board_conn[i].tid, bcm_errmsg(rv));
            }
		}
	}

	/* SDKDifferent trunk = 121 config */
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		memset(&trunk_data, 0, sizeof(trunk_data));
		if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, SDK_DIFF_TRUNK, &trunk_data))
		{
            rv = bcm_trunk_create_id(unit, SDK_DIFF_TRUNK);

            if (rv < 0)
            {
                npd_syslog_err("%s,%d:Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
                                   __func__, __LINE__, ts_board_conn[i].local_dev, ts_board_conn[i].tid, bcm_errmsg(rv));
            }

            bcm_trunk_add_info_t_init(&trunk_data);
            trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
            trunk_data.flags = BCM_TRUNK_FLAG_FAILOVER_ALL_LOCAL;
            trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
            trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
            trunk_data.psc = BCM_TRUNK_PSC_SRCDSTMAC;
		}
		for(i = 0; i < 4; i++)
		{
			if(ts_board_conn[i].peer_slot == -1 || 
			ts_board_conn[i].peer_type == 0)
			{
				continue;
			}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(ts_board_conn[i].peer_type))
			{
				continue;
			}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, ts_board_conn[i].peer_type))
			{
			    for(j = 0; j < PPAL_PLANE_PORT_COUNT(ts_board_conn[i].peer_type); j++)
			    {
			        if(j >= PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE))
			        {
			            break;
			        }
	                peer_slot = SLOT_PORT_PEER_SLOT(ts_board_conn[i].peer_slot, j);
					if(peer_slot == -1)
					{
					    continue;
					}
	    			if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
	    			{
	    				npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
	    				continue;
	    			}
	    			peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
	    			if(peer_type == 0)
	    			{
	    				npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
	    				continue;
	    			}
	    			/*NoPP的情况不往下走*/
					if(!SYS_MODULE_ISHAVEPP(peer_type))
		    		{
		    			continue;
		    		}
					
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, ts_board_conn[i].peer_type)
						&& !SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
					    int tmp = 0;
						int peer_mod = 0;
						int find_port = 0;
						
	        			peer_port = SLOT_PORT_PEER_PORT(ts_board_conn[i].peer_slot, j);
						if(peer_port == -1)
						{
						    continue;
						}
						
	        			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
	        			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);

						for(tmp = 0; tmp < trunk_data.num_ports; tmp++)
						{
							if(trunk_data.tp[tmp] == peer_port &&
								trunk_data.tm[tmp] == peer_mod)
							{
								find_port = 1;
								break;
							}
						}

						if(!find_port)
						{
							trunk_data.tp[trunk_data.num_ports] = peer_port;
		                	trunk_data.tm[trunk_data.num_ports] = peer_mod;
		                	trunk_data.num_ports = trunk_data.num_ports + 1;
						}
					}
			    }
			}
		}
		rv = bcm_trunk_set(unit, SDK_DIFF_TRUNK, &trunk_data);
	    if (rv < 0)
	    {
	        npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
	                           unit, SDK_DIFF_TRUNK, bcm_errmsg(rv));
	    }
	}
		
	/*unicast flow*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
    	int module_id = 0;
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                         SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);
    	for(peer_modid = 0; peer_modid < 6; peer_modid++)
    	{
			int conn_directed = 0;
            if(peer_modid == module_id)
            {
                continue;
            }
            for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
            {
            	if(ts_board_conn[i].peer_slot == -1 || 
        				ts_board_conn[i].peer_type == 0)
            	{
            		continue;
            	}
				if(ts_board_conn[i].local_dev != unit)
				{
                    continue;
				}
				/* 对于NoPP的情况，不应该往下走*/
				if(ts_board_conn[i].port_type == PLANE_PORTTYPE_USERETH)
				{
					continue;
				}
				/*对于SDKDIFF的情况，不应该往下走*/
				if(ts_board_conn[i].port_type == PLANE_PORTTYPE_STACKETH)
				{
					continue;
				}
    			bcm_stk_modport_delete(ts_board_conn[i].local_dev, peer_modid, ts_board_conn[i].local_port);
    			if(ts_board_conn[i].peer_mod == peer_modid)
    			{
                    bcm_stk_modport_add(ts_board_conn[i].local_dev, peer_modid, ts_board_conn[i].local_port);
					conn_directed = 1;
    			}
            }
			/*如果没有直接相连，找到中转的路径*/
			if(conn_directed == 0)
			{
				int have_conn = 0;
                for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
                {
    				if(ts_board_conn[i].local_dev != unit)
    				{
                        continue;
    				}
					
                	if(ts_board_conn[i].peer_slot == -1 || 
            				ts_board_conn[i].peer_type == 0)
                	{
                		continue;
                	}
        			if(ts_board_conn[i].peer_mod == peer_modid)
        			{
                        continue;
        			}
					/* 对于NoPP的情况，不应该往下走*/
					if(ts_board_conn[i].port_type == PLANE_PORTTYPE_USERETH)
					{
						continue;
					}
					
					/*对于SDKDIFF的情况，不应该往下走*/
					if(ts_board_conn[i].port_type == PLANE_PORTTYPE_STACKETH)
					{
						continue;
					}
					
					for(j = 0; j < PPAL_PLANE_PORT_COUNT(ts_board_conn[i].peer_type); j++)
    				{
    					/*判断是否有端口与端口i对应的mod相连*/
                        peer_unit = PPAL_PLANE_2_UNIT(ts_board_conn[i].peer_type, j);
                        if(-1 == peer_unit)
                            continue;
                
						if(UNIT_2_MODULE(ts_board_conn[i].peer_type, ts_board_conn[i].peer_slot, peer_unit, 0) != ts_board_conn[i].peer_mod)
						{
							continue;
						}
						
                        unit_port = PPAL_PLANE_2_PORT(ts_board_conn[i].peer_type, j);
						if(unit_port == -1)
						{
							continue;
						}
                        plane_port = PPAL_PHY_2_PLANE(ts_board_conn[i].peer_type,
                               peer_unit, unit_port);
                        if(-1 == plane_port)
                        {
                            continue;
                        }
                
                        peer_slot = SLOT_PORT_PEER_SLOT(ts_board_conn[i].peer_slot, j);
                
                        if(-1 == peer_slot)
                            continue;
                		
                		if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
                		{
                			continue;
                		}
                		peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
                		if(peer_type == 0)
                		{
                			continue;
                		}
                		peer_port = SLOT_PORT_PEER_PORT(ts_board_conn[i].peer_slot, j);
                		peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
                		if(peer_unit == -1)
                		{
							continue;
                		}
    					if(UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0) == peer_modid)
    					{
							bcm_stk_modport_add(ts_board_conn[i].local_dev, peer_modid, ts_board_conn[i].local_port);
							have_conn = 1;
							break;
    					}
    				}
					if(have_conn == 1)
					{
						break;
					}
                }
			}
    	}
    }
	
	/*enable all stacking ports*/
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
	{
    	if(ts_board_conn[i].peer_slot == -1 || 
				ts_board_conn[i].peer_type == 0)
    	{
    		continue;
    	}
		if(ts_board_conn[i].port_type != PLANE_PORTTYPE_USERETH)
		{
			bcm_port_enable_set(ts_board_conn[i].local_dev, ts_board_conn[i].local_port, 1);
		}
	}
	
    return BCM_E_NONE;      
   
}


int tseries_linecard_central_get_trunk_by_peer_slotunit(unsigned char unit,
						unsigned int slot_id, unsigned char peer_unit, int *trunkId)
{
	int rv = 0;
	bcm_trunk_chip_info_t trunk_info;
	int tid_id_min = 0, tid_id_max, tid_id_num, tid_cur;
	int tid_base, trunk_slot_base;
	int module_type;

	rv = bcm_trunk_chip_info_get(unit, &trunk_info);
	if (rv < 0)
	{
		npd_syslog_err("Crossbar init error: fail to get trunk chip info unit %d, error %s",
					   unit, bcm_errmsg(rv));
		return rv;
	}

	module_type = MODULE_TYPE_ON_SLOT_INDEX(SYS_CHASSIS_SLOT_NO2INDEX(slot_id));
	if(!module_type)
	{
		return -1;
	}
	
	tid_id_min = trunk_info.trunk_fabric_id_min;
	tid_id_max = trunk_info.trunk_fabric_id_max;
	tid_id_num = tid_id_max - tid_id_min + 1;

	
	tid_base = tid_id_num/SYS_CHASSIS_MASTER_SLOTNUM;
	if (tid_base <= 0)
	{
		tid_base = 1;
	}
	if (peer_unit >= tid_base)
	{
		return -1;
	}
		
	//trunk_slot_base = SYS_CHASSIS_SLOT_NO2INDEX(slot_id) - SYS_CHASSIS_MASTER_SLOT_INDEX(0);
	trunk_slot_base = 0;
	tid_base = trunk_slot_base * tid_base;
	tid_cur = tid_base + tid_id_min + peer_unit;
	
	*trunkId = tid_cur;

	return 0;	
}


long tseries_linecard_central_swap_extserv_trunk(int serv_slot)
{
	int rv = 0;
	int iter_slot = 0;
	int iter_moduletype = 0;
	int i = 0,j = 0, temp = 0;
	bcm_trunk_add_info_t  trunkEntry;
	int tid_cur;	
	
#define DATA_SWAP(a, b, tmp) \
    do{ \
		tmp=b; \
		b=a; \
		a=tmp; \
	}while(0)

	
	if (2 != nam_asic_get_instance_num())
		return 0;
	
		
	for (iter_slot = 0; iter_slot < CHASSIS_SLOT_COUNT; iter_slot++)
	{
		if (-1 != serv_slot && iter_slot != serv_slot)
			continue;
		
		iter_moduletype = MODULE_TYPE_ON_SLOT_INDEX(iter_slot);
		if(0 == iter_moduletype)
			continue;

		if(!SYS_MODULE_EXTERNAL_SERVICE(iter_moduletype) 
		   && !SYS_MODULE_INDEPENDENT_SERVICE(iter_moduletype))
		{
			continue;
		}

		if(SYS_MODULE_EXTERNAL_SERVICE(iter_moduletype)
			&& MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(iter_slot))
			continue;


		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(iter_slot), 0, &tid_cur);
		memset(&trunkEntry, 0, sizeof(trunkEntry));
		rv = bcm_trunk_get(0, tid_cur, &trunkEntry);	
		if (rv != BCM_E_NONE || trunkEntry.num_ports <= 0)
		{
			continue;
		}
		/* swap the trunkEntry */
		i = 0;
		j = trunkEntry.num_ports - 1;
		do {
			DATA_SWAP(trunkEntry.tm[i], trunkEntry.tm[j], temp);
			DATA_SWAP(trunkEntry.tp[i], trunkEntry.tp[j], temp);			
			i++;
			
			j--;
		}while(i<j);
		
		rv = bcm_trunk_set(1,tid_cur,&trunkEntry);	
		if (0 != rv)
		{ 
			npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
		}								
	}	
    return 0;

}
 

long tseries_linecard_central_system_indpnt_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
	int rv = 0;
	
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{			
		int i = 0,j = 0;
		bcm_trunk_add_info_t  trunkEntry;
		int tid_cur;
			
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_cur);

		rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
		if(BCM_E_NOT_FOUND == rv )
		{
			rv = bcm_trunk_create_id(unit, tid_cur);

			bcm_trunk_add_info_t_init(&trunkEntry);
			trunkEntry.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.mc_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.psc = BCM_TRUNK_PSC_SRCMAC;
		}
		
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			int peer_unit, peer_port, peer_module;
			int conn_slot, conn_port;
			int peer_moduletype, find_port = 0;

			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;

			conn_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			conn_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_slot || -1 == conn_port )
				continue;
			
			peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(conn_slot);
			if(0 == peer_moduletype)
				continue;

			/*
			if(MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(conn_slot))
				continue;
			*/


			peer_unit = PPAL_PLANE_2_UNIT(peer_moduletype, conn_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_moduletype, conn_port);
			peer_module = UNIT_2_MODULE(peer_moduletype, conn_slot, peer_unit, peer_port);

			for(j = 0; j < trunkEntry.num_ports; j++)
			{
				if((trunkEntry.tm[j] == peer_module) 
					&& (trunkEntry.tp[j] == peer_port))
					find_port = 1;
			}

			if(!find_port)
			{
				trunkEntry.tm[trunkEntry.num_ports] = peer_module;
				trunkEntry.tp[trunkEntry.num_ports] = peer_port;
				trunkEntry.num_ports += 1;
			}
		}
		rv = bcm_trunk_set(unit,tid_cur,&trunkEntry);	
		if (0 != rv)
		{ 
			npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
		}
	}	
	return NPD_SUCCESS;
	
}

long tseries_linecard_central_system_indpnt_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	int unit = 0;
	int rv = 0;
	bcm_trunk_add_info_t  trunkEntry;
	int tid_cur;
	
		
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
			
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(delete_slotid), unit, &tid_cur);
		
		rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
		if(BCM_E_NONE == rv )
		{
			rv = bcm_trunk_destroy(unit, tid_cur);
		}
	}		
	return NPD_SUCCESS;
}


long tseries_linecard_central_system_extserv_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
	int rv = 0;
	
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{			
		int i = 0,j = 0;
		bcm_trunk_add_info_t  trunkEntry;
		int tid_cur;
			
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_cur);

		rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
		if(BCM_E_NOT_FOUND == rv )
		{
			rv = bcm_trunk_create_id(unit, tid_cur);

			bcm_trunk_add_info_t_init(&trunkEntry);
			trunkEntry.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.mc_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
	        trunkEntry.psc = BCM_TRUNK_PSC_SRCMAC;
		}
		
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			int peer_unit, peer_port, peer_module;
			int conn_slot, conn_port;
			int peer_moduletype, find_port = 0;

			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;

			conn_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			conn_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_slot || -1 == conn_port )
				continue;
			
			peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(conn_slot);
			if(0 == peer_moduletype)
				continue;

			if(MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(conn_slot))
				continue;


			peer_unit = PPAL_PLANE_2_UNIT(peer_moduletype, conn_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_moduletype, conn_port);
			peer_module = UNIT_2_MODULE(peer_moduletype, conn_slot, peer_unit, peer_port);

			for(j = 0; j < trunkEntry.num_ports; j++)
			{
				if((trunkEntry.tm[j] == peer_module) 
					&& (trunkEntry.tp[j] == peer_port))
					find_port = 1;
			}

			if(!find_port)
			{
				trunkEntry.tm[trunkEntry.num_ports] = peer_module;
				trunkEntry.tp[trunkEntry.num_ports] = peer_port;
				trunkEntry.num_ports += 1;
			}
		}
		rv = bcm_trunk_set(unit,tid_cur,&trunkEntry);	
		if (0 != rv)
		{ 
			npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
		}
	}	

	tseries_linecard_central_swap_extserv_trunk(insert_slotid);
	
	return NPD_SUCCESS;
	
}

long tseries_linecard_central_system_extserv_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	int unit = 0;
	int rv = 0;
	bcm_trunk_add_info_t  trunkEntry;
	int tid_cur;
			
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
			
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(delete_slotid), unit, &tid_cur);
		
		rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
		if(BCM_E_NONE == rv )
		{
			rv = bcm_trunk_destroy(unit, tid_cur);
		}
	}		
	return NPD_SUCCESS;
}

long tseries_linecard_central_system_fabric_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    int unit;
    int port;
    int rv;
    int tid = -1;
    int module_id;
	int stk_modid;
	int peer_port; 
	int conn_port;
	int peer_moduletype;
	int conn_count;
	int e_i;
	int conn_i;

    bcm_trunk_add_info_t trunk_data;
    int tid_base;
	
	bcm_pbmp_t conn_pbmp;

	int plane_port;
	int peer_slot;
	int hg_port = 0;
	int e_port = 0;
	int panel_port = 0;
		
	int conn_unit;
	int conn_module;
	
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {	
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

		
		/* PHASE 1 - get the connected port bitmap */
		BCM_PBMP_CLEAR(conn_pbmp);
		PBMP_HG_ITER(unit, port)
		{
					
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,unit, port);
            if(-1 == plane_port)
                continue;
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,plane_port);
			if(-1 == peer_slot)
				continue;	
			if(insert_slotid != peer_slot)
				continue;
			BCM_PBMP_PORT_ADD(conn_pbmp, port);
		}
		/* PHASE 1 - end */
		
		/* PHASE 2 - Delete the exist Trunk */

		/* PHASE 2 - End */

		/* PHASE 3 - Set the Attribute of the Connected Port*/

		/* dont't do anything */
		
		/* PHASE 3 - End*/
		

		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

		/* 4.1 create the trunk */


		/* 4.2 add the connected port to the trunk  */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int find_port = FALSE;
			int j = 0;
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;

			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);
			
			rv = tseries_linecard_central_get_trunk_by_peer_slotunit(
				unit, SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), peer_unit, &tid_base);
			if (rv != 0)
			{
				continue;
			}
			if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
			{
				bcm_trunk_add_info_t_init(&trunk_data);
				trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.psc = BCM_TRUNK_PSC_SRCMAC;
				
				rv = bcm_trunk_create_id(unit, tid_base);
				if (rv < 0)
				{
					npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
									   unit, tid_base, bcm_errmsg(rv));
				}
			}		
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port &&
					trunk_data.tm[j] == trunk_modid)
                {
                    find_port = TRUE;
                    break;
                }
				trunk_data.tm[j] = module_id;
            }			
			if (!find_port)
			{
		        trunk_data.tp[trunk_data.num_ports] = trunk_port;
		        trunk_data.tm[trunk_data.num_ports] = trunk_modid;
				trunk_data.num_ports++;
				
				rv = bcm_port_enable_set(unit, port, 0);				
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);
                if (rv < 0)
                {
                    npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
                                       unit, tid, bcm_errmsg(rv));
                }				
			}
		}
		/* PHASE 4 - End */	
		
		/* PHASE 5 - Set Egress Table of connected port */
		/* Because the packet can't from this unit to other  */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			PBMP_HG_ITER(unit, hg_port)
			{
				rv = bcm_port_flood_block_set(unit, port, hg_port, BCM_PORT_FLOOD_BLOCK_ALL);
                if (rv < 0)
                {
                    npd_syslog_err("Crossbar init: fail to set block trunk port unit %d tid %d port %d \n err %s\r\n",
                                       unit, tid, port, bcm_errmsg(rv));
                }				
			}			
		}
		/* PHASE5 - END */

		/* PHASE 6 - Set Egress Table of connected port */

		/* 6.1 - get the exist trunk number */


		/* 6.2 -  set non't fabric slot modules path  */

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			for(stk_modid = 0; stk_modid < 64; stk_modid++)
			{
				if(stk_modid == module_id)
				{
					continue;
				}
				peer_slot = MODULE_2_SLOT_INDEX(stk_modid);
				/*
				if(peer_slot > CHASSIS_SLOT_COUNT)
					continue;
				*/
				if(SYS_CHASSIS_ISMASTERSLOT(peer_slot))
					continue;
				rv = bcm_stk_modport_add(unit, stk_modid, port);
			}
		}
		
		/* 6.3 -  set the peer module of connected port  */
		e_i = 0;
		conn_i = 0;
		
		conn_count = 0;
		BCM_PBMP_COUNT(conn_pbmp, conn_count);
		
		PBMP_ALL_ITER(unit, e_port)
		{
			panel_port = PHY_2_PANEL(unit,e_port);
			if (panel_port == -1)
				continue;
			e_i++;
			
			conn_i = 0;						
			BCM_PBMP_ITER(conn_pbmp, port)
			{
				conn_i++;
				int local_plane = 0;
				int peer_plane = 0;
				int peer_unit = 0;
				int peer_module = 0;

				/* because these port have inspect, so just trans it */
				local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
				peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
				peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);
				
				peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{									
		            /*hg ports between fabric and line use bcm_stk_modport_add*/
		            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
						__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
		            rv = bcm_stk_port_modport_add(unit, e_port, peer_module, port);
		            if(BCM_E_NONE != rv)
		            {
		                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
		                    peer_module, unit, port, bcm_errmsg(rv));
		            }	
									}
		
				peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
											peer_unit, TSERIES_PORT_PER_ASICMODULE);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{					
		            /*hg ports between fabric and line use bcm_stk_modport_add*/
		            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
						__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
		            rv = bcm_stk_port_modport_add(unit, e_port, peer_module, port);
		            if(BCM_E_NONE != rv)
		            {
		                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
		                    peer_module, unit, port, bcm_errmsg(rv));
		            }	
				}					
			}						
		}		

		/* set trunk override, then the hash of fabric trunk will be overrde by bitmap */
		e_i = 0;
		conn_i = 0;
		
		conn_count = 0;
		BCM_PBMP_COUNT(conn_pbmp, conn_count);		
		PBMP_ALL_ITER(unit, e_port)
		{
			panel_port = PHY_2_PANEL(unit,e_port);
			if (panel_port == -1)
				continue;
			e_i++;
			
			conn_i = 0;						
			BCM_PBMP_ITER(conn_pbmp, port)
			{
				conn_i++;
				int local_plane = 0;
				int peer_plane = 0;
				int peer_unit = 0;
				int peer_module = 0;

				/* because these port have inspect, so just trans it */
				local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
				peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
				peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);
				
				peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{														
					rv = bcm_trunk_override_ucast_set(unit, e_port, tid_base, peer_module, 1);				
				}
		
				peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
											peer_unit, TSERIES_PORT_PER_ASICMODULE);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{					
					rv = bcm_trunk_override_ucast_set(unit, e_port, tid_base, peer_module, 1);					
				}					
			}					
		}				
		
		/* PHASE 6 - End */

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 1);
		}
		
    }

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {	
		int i = 0,j = 0;
		bcm_trunk_add_info_t  trunkEntry;
		int tid_cur;
		
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{

			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;

			peer_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);	
			peer_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == peer_slot || -1 == peer_port)
				continue;


			peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(0 == peer_moduletype)
				continue;

			if(!SYS_MODULE_EXTERNAL_SERVICE(peer_moduletype) 
			   && !SYS_MODULE_INDEPENDENT_SERVICE(peer_moduletype))
			{
				continue;
			}

			if(SYS_MODULE_EXTERNAL_SERVICE(peer_moduletype)
				&& MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(peer_slot))
				continue;

			if(-1 == PPAL_PLANE_2_PORT(peer_moduletype, peer_port))
				continue;

			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(peer_slot), unit, &tid_cur);
			rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);							
			if(BCM_E_NOT_FOUND == rv )
			{
				rv = bcm_trunk_create_id(unit, tid_cur);

				bcm_trunk_add_info_t_init(&trunkEntry);
				trunkEntry.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
                trunkEntry.mc_index = BCM_TRUNK_UNSPEC_INDEX;
                trunkEntry.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
                trunkEntry.psc = BCM_TRUNK_PSC_SRCMAC;
			}					
			
			conn_unit = PPAL_PLANE_2_UNIT(insert_board_type, i);						
			conn_port = PPAL_PLANE_2_PORT(insert_board_type, i);
			conn_module = UNIT_2_MODULE(insert_board_type, insert_slotid, conn_unit, conn_port);

			for(j = 0; j < trunkEntry.num_ports; j++)
			{
				if((trunkEntry.tm[j] == conn_module) 
					&& (trunkEntry.tp[j] == conn_port))
					break;
			}
			if( j < trunkEntry.num_ports )
				continue;
			
			trunkEntry.tm[trunkEntry.num_ports] = conn_module;
			trunkEntry.tp[trunkEntry.num_ports] = conn_port;
			trunkEntry.num_ports += 1;
			
			rv = bcm_trunk_set(unit,tid_cur,&trunkEntry);	
			if (0 != rv)
			{ 
				npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
			}						
		}			
    }
	tseries_linecard_central_swap_extserv_trunk(-1);
	return 0;
}

long tseries_linecard_central_system_fabric_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
    int unit;
    int port;
    int rv;
    int tid = -1;
    int modid;
    int module_id;
	int stk_modid;
    int i;
    int j;
	int conn_i ;
	int e_i;
	int conn_count = 0;
	int peer_moduletype;

    bcm_trunk_add_info_t trunk_data;
    int tid_base;
	
	bcm_pbmp_t conn_pbmp;

	int modport;
	int plane_port;
	int peer_slot;
	int e_port = 0;
	int panel_port = 0;
		
	int fabric_unit;
	int fabric_port = 0;
	int fabric_module;
	
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {	
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);
		
		/* PHASE 1 - get the connected port bitmap */
		BCM_PBMP_CLEAR(conn_pbmp);
		PBMP_HG_ITER(unit, port)
		{
					
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,unit, port);
            if(-1 == plane_port)
                continue;
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,plane_port);
			if(-1 == peer_slot)
				continue;	
			if(delete_slotid != peer_slot)
				continue;	
			BCM_PBMP_PORT_ADD(conn_pbmp, port);
		}
		/* PHASE 1 - end */
		
		/* PHASE 2 - Delete port from the trunk */

		/* PHASE 2 - End */

		/* PHASE 3 - Set the Attribute of the Connected Port*/

		/* dont't do anything */
		
		/* PHASE 3 - End*/
		

		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

		/* 4.1 create the trunk */

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int i = 0;
			int j = 0;
			int find_index = -1;
			
			modid = module_id;
			modport = port;
			
            rv = bcm_trunk_find(unit, modid, modport, &tid_base); 
			if (0 != rv)
				continue;
			rv = bcm_trunk_get(unit, tid_base, &trunk_data);
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port)
                {
                    find_index = j;
                    break;
                }
				trunk_data.tm[j] = module_id;
            }			
			if(-1 != find_index)
			{			
		        for(i = find_index; i < trunk_data.num_ports-1; i++)
		        {
		            trunk_data.tp[i] = trunk_data.tp[i+1];
		            trunk_data.tm[i] = module_id;					
		        }
				trunk_data.tp[i] = 0;
				trunk_data.tm[i] = 0;
				trunk_data.num_ports--;
				
	            trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
	            trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
	           	trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;			

				rv = bcm_port_enable_set(unit, port, 0);				
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);
                if (rv < 0)
                {
                    npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
                                       unit, tid, bcm_errmsg(rv));
                }				
			}
		}		

		/* PHASE 4 - End */	
		
		/* PHASE 5 - Set Egress Table of connected port */
		/* do nothing */
		/* PHASE5 - END */

		/* PHASE 6 - Set Egress Table of connected port */

		
		/* 6.1 cancel trunk override */
		e_i = 0;
		conn_i = 0;		
		conn_count = 0;
		BCM_PBMP_COUNT(conn_pbmp, conn_count);
		
		PBMP_E_ITER(unit, e_port)
		{
			panel_port = PHY_2_PANEL(unit,e_port);
			if (panel_port == -1)
				continue;
			e_i++;
			
			conn_i = 0;	
			BCM_PBMP_ITER(conn_pbmp, port)
			{
				conn_i++;
				int local_plane = 0;
				int peer_plane = 0;
				int peer_unit = 0;
				int peer_module = 0;

				/* because these port have inspect, so just trans it */
				local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
				peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
				peer_unit = PPAL_PLANE_2_UNIT(delete_board_type, peer_plane);
				
				peer_module = UNIT_2_MODULE(delete_board_type, delete_slotid, peer_unit, 0);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{
					rv = bcm_trunk_override_ucast_set(unit, e_port, tid_base, peer_module, 0);				
				}
		
				peer_module = UNIT_2_MODULE(delete_board_type, delete_slotid, 
											peer_unit, TSERIES_PORT_PER_ASICMODULE);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{					
					rv = bcm_trunk_override_ucast_set(unit, e_port, tid_base, peer_module, 0);					
				}					
			}					
		}					
	

		/* 6.2-  set the peer module of connected port  */
		e_i = 0;
		conn_i = 0;		
		conn_count = 0;
		BCM_PBMP_COUNT(conn_pbmp, conn_count);
		
		PBMP_ALL_ITER(unit, e_port)
		{
			panel_port = PHY_2_PANEL(unit,e_port);
			if (panel_port == -1)
				continue;
			e_i++;
			
			conn_i = 0;
			BCM_PBMP_ITER(conn_pbmp, port)
			{
				conn_i++;
				int local_plane = 0;
				int peer_plane = 0;
				int peer_unit = 0;
				int peer_module = 0;

				/* because these port have inspect, so just trans it */
				local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
				peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
				peer_unit = PPAL_PLANE_2_UNIT(delete_board_type, peer_plane);

				peer_module = UNIT_2_MODULE(delete_board_type, delete_slotid, peer_unit, 0);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{
		            /*hg ports between fabric and line use bcm_stk_modport_add*/
		            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
						__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
		            rv = bcm_stk_port_modport_delete(unit, e_port, peer_module, port);
		            if(BCM_E_NONE != rv)
		            {
		                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
		                    peer_module, unit, port, bcm_errmsg(rv));
		            }	
				}
		
				peer_module = UNIT_2_MODULE(delete_board_type, delete_slotid, 
											peer_unit, TSERIES_PORT_PER_ASICMODULE);
				if (-1 != peer_module &&
					(e_i%conn_count == conn_i%conn_count))
				{
		            /*hg ports between fabric and line use bcm_stk_modport_add*/
		            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
						__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
		            rv = bcm_stk_port_modport_delete(unit, e_port, peer_module, port);
		            if(BCM_E_NONE != rv)
		            {
		                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
		                    peer_module, unit, port, bcm_errmsg(rv));
		            }	
				}					
			}			
		}		
		
		/* 6.3 -  set non't fabric slot modules path  */
		/* get the panel pbmp, because the mod can only through */

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			for(stk_modid = 0; stk_modid < 64; stk_modid++)
			{
				peer_slot = MODULE_2_SLOT_INDEX(stk_modid);
				if(peer_slot > CHASSIS_SLOT_COUNT)
					continue;
				if(SYS_CHASSIS_ISMASTERSLOT(peer_slot))
					continue;
				rv = bcm_stk_modport_delete(unit, stk_modid, port);				
			}
		}	
		/* PHASE 6 - End */

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 0);
		}
		
    }
	

	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		
		for(plane_port = 0; plane_port < PPAL_PLANE_PORT_COUNT(delete_board_type); plane_port++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(delete_board_type, plane_port))
				continue;
			
			peer_slot = SLOT_PORT_PEER_SLOT(delete_slotid, plane_port);
			if(-1 == peer_slot)
				continue;

			peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(0 == peer_moduletype)
				continue;

			if (!SYS_MODULE_EXTERNAL_SERVICE(peer_moduletype)
				&& !SYS_MODULE_INDEPENDENT_SERVICE(peer_moduletype))
				continue;
			
			if(SYS_MODULE_EXTERNAL_SERVICE(peer_moduletype)
				&& MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(peer_slot))
				continue;
			
			
			fabric_unit = PPAL_PLANE_2_UNIT(delete_board_type, plane_port);	
			fabric_port = PPAL_PLANE_2_PORT(delete_board_type, plane_port);
			fabric_module = UNIT_2_MODULE(delete_board_type, delete_slotid, fabric_unit, fabric_port);
			
			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(peer_slot), unit, &tid_base);				
            if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
            {
                continue;
            }
			
            for (i = 0,j = 0 ; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == fabric_port && 
					trunk_data.tm[j] == fabric_module )
                {
                    continue;
                }
                
                trunk_data.tp[i] = trunk_data.tp[j];
				trunk_data.tm[i] = trunk_data.tm[j];
				i++;
            }
			trunk_data.num_ports = i;
			for(;i<BCM_TRUNK_MAX_PORTCNT;i++)
			{
				trunk_data.tp[i] = 0;
				trunk_data.tm[i] = 0;
			}

            npd_syslog_dbg("%s %d trunk set: trunk id: %d, total num: %d, modid %d, port: %d\r\n", 
		        		__func__, __LINE__, tid_base, trunk_data.num_ports, fabric_module, fabric_port);
			rv = bcm_trunk_destroy(unit, tid_base);
			rv = bcm_trunk_create_id(unit, tid_base);
            rv = bcm_trunk_set(unit, tid_base, &trunk_data);

            if (rv < 0)
            {
                npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s",
                                   unit, tid_base, bcm_errmsg(rv));
            }
            npd_syslog_dbg("Crossbar init: add port modid %d port  %d to trunk %d\n",
                fabric_module, fabric_port, tid_base);			
		}
	}


	tseries_linecard_central_swap_extserv_trunk(-1);
	
	return 0;
}

long tseries_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
	{
    	if( SYS_MODULE_SLOT_INDEPENDENT_SERVICE(insert_slotid) )
    	{
			return tseries_linecard_central_system_indpnt_conn_init(
				product_type, insert_board_type, insert_slotid);
		}
    	if( SYS_MODULE_SLOT_EXTERNAL_SERVICE(insert_slotid) )
    	{
			return tseries_linecard_central_system_extserv_conn_init(
				product_type, insert_board_type, insert_slotid);
		}		
    	if( SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid) )
    	{
			return tseries_linecard_central_system_fabric_conn_init(
				product_type, insert_board_type, insert_slotid);
		}

	}
    else if(FULL_MESH == SYS_PRODUCT_TOPO)
    {
        tseries_linecard_fullmesh_system_conn_init(product_type, 
			insert_board_type, insert_slotid);    
		
    }
	return 0 ;	
}

long tseries_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
    if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
    {
    	if( SYS_MODULE_SLOT_INDEPENDENT_SERVICE(delete_slotid) )
    	{
			return tseries_linecard_central_system_indpnt_conn_deinit(
				product_type, delete_board_type, delete_slotid);
		}

    	if( SYS_MODULE_SLOT_EXTERNAL_SERVICE(delete_slotid) )
    	{
			return tseries_linecard_central_system_extserv_conn_deinit(
				product_type, delete_board_type, delete_slotid);
		}
		
		if (SYS_CHASSIS_SLOTNO_ISFABRIC(delete_slotid))
		{
			return tseries_linecard_central_system_fabric_conn_deinit(
				product_type, delete_board_type, delete_slotid);			
		}
    }
    else
    {
		int i = 0;
		int unit = 0;
		int port = 0;
		int rv = 0;

		bcm_pbmp_t bmp, ubmp;
		bcm_vlan_t vid;

		bcm_trunk_add_info_t  trunkEntry;
		int tid_cur;
		
		
		if(SYS_MODULE_EXT_PORT(delete_board_type))
		{
			for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
			{
				int peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, i);
				if(peer_slot != delete_slotid)
					continue;
				unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, i);
				if(-1 == unit)
					continue;
				port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, i);

				BCM_PBMP_PORT_SET(bmp, port);
				BCM_PBMP_CLEAR(ubmp);
				for(vid = 1; vid < 4094; vid++)
				{
					bcm_vlan_port_add(unit, vid, bmp, ubmp); 
				}
			}
		}
        /*todo rcj*/
    	if( SYS_MODULE_EXTERNAL_SERVICE(delete_board_type) )		
    	{		
            for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
            {    				
    			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(delete_slotid), unit, &tid_cur);
    			
    			rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
    			if(BCM_E_NONE == rv )
    			{
    				rv = bcm_trunk_destroy(unit, tid_cur);
    			}
    		}
    	}
		tseries_linecard_fullmesh_system_conn_init(product_type, delete_board_type, 
			delete_slotid);
    }
	return 0;
}


#if 0
long tseries_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    int unit;
    int port;
    int rv;
    int find_port;
    int tid = -1;
    int modid;
    int module_id, modnum, stk_modid;
    int i;

    if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
    {
    	if( SYS_MODULE_SLOT_INDEPENDENT_SERVICE(insert_slotid) )
    	{
			return tseries_linecard_central_system_indpnt_conn_init(
				product_type, insert_board_type, insert_slotid);
		}
    	if( SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid) )
    	{
			return tseries_linecard_central_system_indpnt_conn_init(
				product_type, insert_board_type, insert_slotid);
		}

		
		if(SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid))
		{
			bcm_pbmp_t panel_pbmp;
			bcm_port_t panel_port;
			
	        for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	        {
				int peer_unit, peer_port, peer_module;
				int conn_slot, conn_port;
				int peer_moduletype;
				int peer_plane;
				
				int ftid_num = {0};
				int ftid[16] = {0};
				int fslot[16] = {0};
				int tid_offset;
				int master_slot;

				
	            module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
	                 SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

	            /*set modid*/
	            rv = bcm_stk_modid_count(unit, &modnum);
				
	            /*delete exist trunk*/
	            PBMP_HG_ITER(unit, port)
	            {
	                int modid, modport;
	                bcm_trunk_t tid;
					int plane_port;
					int peer_slot;
					int peer_plane;
					
	                plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
	                       unit, port);
	                if(-1 == plane_port)
	                    continue;
	                peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
	                       plane_port);
					if(-1 == peer_slot)
						continue;
	                modid = module_id;
					modport = port;
	                if(port >= TSERIES_PORT_PER_ASICMODULE)
	                {
	                    modid = module_id + 1;
	                    modport -= TSERIES_PORT_PER_ASICMODULE;
	                }
	                rv = bcm_trunk_find(unit, modid, modport, &tid);         
	                if(BCM_E_NONE == rv)
	                {
						int temp_tid;
						/*the following code for get all trunks do not dst to insert slot*/
						for(temp_tid = 0; temp_tid < ftid_num; temp_tid++)
						{
							if(ftid[temp_tid] == tid)
								break;
						}
						if(peer_slot != insert_slotid)
						{
	    					if(temp_tid == ftid_num)
	    					{
	    						ftid[ftid_num] = tid;
								fslot[ftid_num] = peer_slot;
	    						ftid_num++;
	    					}
	                        continue;
						}
	                    rv = bcm_trunk_destroy(unit, tid); //
	                }
	            }  
	            /*Firstly set the trunk for HG, here all HG should be stack port,
	              for the panel 10G port, should be set before this funciton*/
	            PBMP_HG_ITER(unit, port)
	            {
	                bcm_trunk_chip_info_t trunk_info;
	                bcm_trunk_add_info_t trunk_data;
	                int peer_slot;
					int peer_plane;
	                int local_plane;
	                int modport;
					int other_hg_port;
	                int trunk_slot_base;
	                int find_port = FALSE;
	                int modid;
	    			int trunk_port;
	                int max_fabrictrunk_num;
					int tid_base;
					int master_slot;
					
	                find_port = FALSE;

					bcm_port_ifg_set(unit, port, 12000, BCM_PORT_DUPLEX_FULL, 8*8);
					
	                rv = bcm_trunk_chip_info_get(unit, &trunk_info);
	                if (rv < 0)
	                {
	                    npd_syslog_err("Crossbar init error: fail to get trunk chip info unit %d, error %s",
	                                       unit, bcm_errmsg(rv));
	                    return rv;
	                }

					
	                max_fabrictrunk_num 
						= trunk_info.trunk_fabric_id_max - trunk_info.trunk_fabric_id_min + 1;

	                local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
	                           unit, port);
	                peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
	                if(-1 == peer_slot)
	                    continue;

	                if(insert_slotid != peer_slot)
	                    continue;
					peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
					if(0 == peer_moduletype)
						continue;	

	                peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
					if(-1 == peer_plane)
						continue;

					peer_unit = PPAL_PLANE_2_UNIT(peer_moduletype, peer_plane);						
					peer_port = PPAL_PLANE_2_PORT(peer_moduletype, peer_plane);
					peer_module = UNIT_2_MODULE(peer_moduletype, peer_slot, peer_unit, peer_port);
	                    
	                tid_base = max_fabrictrunk_num/SYS_CHASSIS_MASTER_SLOTNUM;

	                for(master_slot = 0; master_slot < SYS_CHASSIS_MASTER_SLOTNUM;
					              master_slot++)
					{
						if(insert_slotid == SYS_CHASSIS_MASTER_SLOT_INDEX(master_slot))
							trunk_slot_base = master_slot*tid_base;
					}			  	
	                tid = trunk_slot_base + peer_unit;

	                /*port belongs a trunk*/
	                if (-1 != tid)
	                {
	                    tid = tid + trunk_info.trunk_fabric_id_min ;

	                    if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid, &trunk_data))
	                    {
							int temp_tid;
	                        rv = bcm_trunk_create_id(unit, tid);

	                        if (rv < 0)
	                        {
	                            npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
	                                               unit, tid, bcm_errmsg(rv));
	                        }

	                        bcm_trunk_add_info_t_init(&trunk_data);
	                        trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
	                        trunk_data.flags = BCM_TRUNK_FLAG_FAILOVER_ALL_LOCAL;
	                        trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
	                        trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
	                        /*trunk_data.psc = BCM_TRUNK_PSC_PORTFLOW;*/
	                        trunk_data.psc = BCM_TRUNK_PSC_SRCDSTMAC;

							/*the following code add new tid to fid array*/
	    					for(temp_tid = 0; temp_tid < ftid_num; temp_tid++)
	    					{
	    						if(ftid[temp_tid] == tid)
	    							break;
	    					}
	    					if(temp_tid == ftid_num)
	    					{
	    						ftid[ftid_num] = tid;
								fslot[ftid_num] = insert_slotid;
	    						ftid_num++;
	    					}
							
	                    }

	                    bcm_stk_modid_get(unit, &modid);
	                    modport = port;
	                    if (port >= TSERIES_PORT_PER_ASICMODULE)
	                    {
	                        modid += 1;
	                        modport -= TSERIES_PORT_PER_ASICMODULE;
	                    }
	                    for (i = 0; i < trunk_data.num_ports; i++)
	                    {
	                        if (trunk_data.tp[i] == modport)
	                        {
	                            find_port = TRUE;
	                            break;
	                        }
	                        trunk_data.tm[i] = modid;
	                    }
	                    if(find_port)
	                    {
							continue;
	                    }
	                    if ((!find_port) &&
	                         SYS_MODULE_RUNNINGSTATE(peer_slot) != RMT_BOARD_NOEXIST)
	                        
	                    {
	                        trunk_data.tp[trunk_data.num_ports] = port;
	                        trunk_data.tm[trunk_data.num_ports] = modid;
	                        trunk_data.num_ports = trunk_data.num_ports + 1;
	                    }

						
						/* 这里对其他HG口做 flood block */
						PBMP_HG_ITER(unit, other_hg_port)
						{
							//if (other_hg_port == port)
							//	continue;
							/* FLOOD_BLOCK_ALL */
							rv = bcm_port_flood_block_set(unit, port, other_hg_port, 0x8);
		                    if (rv < 0)
		                    {
		                        npd_syslog_err("Crossbar init: fail to set block trunk port unit %d tid %d port %d \n err %s\r\n",
		                                           unit, tid, port, bcm_errmsg(rv));
		                    }
							
						}
						
						
					    rv = bcm_port_enable_set(unit, port, 0);
	                    npd_syslog_dbg("%s %d trunk set: trunk id: %d, total num: %d, modid %d, port: %d(%s)\r\n", 
					        __func__, __LINE__, tid, trunk_data.num_ports, modid, port, SOC_PORT_NAME(unit, port));
	                    rv = bcm_trunk_set(unit, tid, &trunk_data);

	                    if (rv < 0)
	                    {
	                        npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
	                                           unit, tid, bcm_errmsg(rv));
	                    }
						rv = bcm_port_enable_set(unit, port, 1);
	                    npd_syslog_dbg("Crossbar init: add port modid %d port  %d to trunk %d\n",
	                        modid, port, tid);

	                }
	            }
				if (ftid_num <= 0)
				{
					continue;
				}
				
				/*set mod destination information*/
				PBMP_E_ITER(unit, panel_port)
				{
	                bcm_trunk_add_info_t trunk_data;
					int p_port = PHY_2_PANEL(unit,panel_port);
					int array;
					int tid_select;
					int rv;
					
					if(p_port == -1)
			    		continue;
					
					array = (p_port-ETH_LOCAL_PORT_START_NO(SYS_LOCAL_MODULE_SLOT_INDEX))%ftid_num;
					tid_select = ftid[array];
	                rv = bcm_trunk_get(unit, tid_select, &trunk_data);
	                if(BCM_E_NONE != rv)
						npd_syslog_err("system connect can not find setted trunk data, tid %d\n", tid_select);
					
					for(stk_modid = 0; stk_modid < 64; stk_modid++)
					{
						int peer_slot = MODULE_2_SLOT_INDEX(stk_modid);
						int d_port;
						int d_num;
						int has_set = 0;
						
						if(peer_slot > CHASSIS_SLOT_COUNT)
							continue;
						for(d_num = 0; d_num < trunk_data.num_ports; d_num++)
						{
							if(SYS_CHASSIS_ISMASTERSLOT(peer_slot))
							{
								int peer_master_slot = 0;
								int need_add = 0;
								int local_plane = 0;
								int hg_port = 0;
															
								local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, trunk_data.tp[d_num]);
								peer_master_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
								if (peer_master_slot != peer_slot)
								{
									int iter_local_plane = 0;
									int iter_peer_slot;
									int iter_peer_moduletype;
									int iter_peer_unit;
									int iter_peer_port;
									int iter_peer_module;
									int iter_peer_plane;
									int iter_has_set;
									
									/* need add the other port to this module */
									PBMP_HG_ITER(unit, hg_port)
									{									
										if (hg_port == trunk_data.tp[d_num])
											continue;

										iter_local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, hg_port);
										if (-1 == iter_local_plane)	
											continue;
										
						                iter_peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, iter_local_plane);
										if (peer_slot != iter_peer_slot)
											continue;
										
										iter_peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(iter_peer_slot);
										if (0 == iter_peer_moduletype)
											continue;

						                iter_peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, iter_local_plane);
						                iter_peer_unit = PPAL_PLANE_2_UNIT(iter_peer_moduletype, iter_peer_plane);
						                iter_peer_port = PPAL_PLANE_2_PORT(iter_peer_moduletype, iter_peer_plane);
						                iter_peer_module = UNIT_2_MODULE(
											iter_peer_moduletype, iter_peer_slot, iter_peer_unit, iter_peer_port);
										if (stk_modid != iter_peer_module)
											continue;

										if(iter_has_set == 0)
										{
										    rv = bcm_stk_port_modport_set(unit, panel_port, 
										            stk_modid, hg_port);
											iter_has_set = 1;		
										}
										else
										{
										    rv = bcm_stk_port_modport_add(unit, panel_port, 
										            stk_modid, hg_port);
										}
									}
									continue;								
								}
							}
							/* for first set module, set the port */
							if(has_set == 0)
							{
							    rv = bcm_stk_port_modport_set(unit, panel_port, 
							            stk_modid, trunk_data.tp[d_num]);
								has_set = 1;
							}
							else
							    rv = bcm_stk_port_modport_add(unit, panel_port, 
							            stk_modid, trunk_data.tp[d_num]);					
						}					
					}
				}		
	        }
		}

       /*for service board, add the trunk info to the board*/
    	if( SYS_MODULE_EXTERNAL_SERVICE(insert_board_type) )		
    	{
    		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    		{			
    			int i = 0,j = 0;
    			bcm_trunk_add_info_t  trunkEntry;
    			int tid_cur;
    				
    			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_cur);
    
    			rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
    			if(BCM_E_NOT_FOUND == rv )
    			{
    				rv = bcm_trunk_create_id(unit, tid_cur);
    
    				bcm_trunk_add_info_t_init(&trunkEntry);
    				trunkEntry.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
                    trunkEntry.mc_index = BCM_TRUNK_UNSPEC_INDEX;
                    trunkEntry.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
                    trunkEntry.psc = BCM_TRUNK_PSC_SRCMAC;
    			}
    			
    			for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
    			{
    				int peer_unit, peer_port, peer_module;
    				int conn_slot, conn_port;
    				int peer_moduletype, find_port = 0;
    
    				if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
    					continue;
    
    				conn_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
    				conn_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
    				if(-1 == conn_slot || -1 == conn_port )
    					continue;
    				
    				peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(conn_slot);
    				if(0 == peer_moduletype)
    					continue;
    
    
    				if(MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(conn_slot))
    					continue;
    
    
    				peer_unit = PPAL_PLANE_2_UNIT(peer_moduletype, conn_port);						
    				peer_port = PPAL_PLANE_2_PORT(peer_moduletype, conn_port);
    				peer_module = UNIT_2_MODULE(peer_moduletype, conn_slot, peer_unit, peer_port);
    
    				for(j = 0; j < trunkEntry.num_ports; j++)
    				{
    					if((trunkEntry.tm[j] == peer_module) 
    						&& (trunkEntry.tp[j] == peer_port))
    						find_port = 1;
    				}
    
    				if(!find_port)
    				{
    					trunkEntry.tm[trunkEntry.num_ports] = peer_module;
    					trunkEntry.tp[trunkEntry.num_ports] = peer_port;
    					trunkEntry.num_ports += 1;
    				}
    			}
    			rv = bcm_trunk_set(unit,tid_cur,&trunkEntry);	
    			if (0 != rv)
    			{ 
    				npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
    			}
    		}
    	}
    
    	/*when the fabric board inserted and there are service board, add trunk info */
    	if(SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid))
    	{
    		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    		{			
    			int i = 0,j = 0;
    			bcm_trunk_add_info_t  trunkEntry;
    			int tid_cur;
    			
    			for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
    			{
    				int conn_slot, conn_unit, conn_port, conn_module;
    				int peer_slot, peer_port;
    				int peer_moduletype;
    
    				if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
    					continue;
    
    				peer_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);	
    				peer_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
    				if(-1 == peer_slot || -1 == peer_port)
    					continue;
    
    
    				peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
    				if(0 == peer_moduletype)
    					continue;
    
    				if(!SYS_MODULE_EXTERNAL_SERVICE(peer_moduletype) 
						&& !SYS_MODULE_SLOT_INDEPENDENT_SERVICE(peer_moduletype))
    				{
    					continue;
    				}
    
    				if(-1 == PPAL_PLANE_2_PORT(peer_moduletype, peer_port))
    					continue;
    
    				nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(peer_slot), unit, &tid_cur);
    				rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);							
    				if(BCM_E_NOT_FOUND == rv )
    				{
    					rv = bcm_trunk_create_id(unit, tid_cur);
    
    					bcm_trunk_add_info_t_init(&trunkEntry);
    					trunkEntry.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
    	                trunkEntry.mc_index = BCM_TRUNK_UNSPEC_INDEX;
    	                trunkEntry.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
    	                trunkEntry.psc = BCM_TRUNK_PSC_SRCMAC;
    				}					
    				
    				conn_unit = PPAL_PLANE_2_UNIT(insert_board_type, i);						
    				conn_port = PPAL_PLANE_2_PORT(insert_board_type, i);
    				conn_module = UNIT_2_MODULE(insert_board_type, insert_slotid, conn_unit, conn_port);
    
    				for(j = 0; j < trunkEntry.num_ports; j++)
    				{
    					if((trunkEntry.tm[j] == conn_module) 
    						&& (trunkEntry.tp[j] == conn_port))
    						break;
    				}
    				if( j < trunkEntry.num_ports )
    					continue;
    				
    				trunkEntry.tm[trunkEntry.num_ports] = conn_module;
    				trunkEntry.tp[trunkEntry.num_ports] = conn_port;
    				trunkEntry.num_ports += 1;
    				
    				rv = bcm_trunk_set(unit,tid_cur,&trunkEntry);	
    				if (0 != rv)
    				{ 
    					npd_syslog_err("trunk %d set port error %d\n",tid_cur, rv);
    				}						
    			}			
    		}
    	}		
    }
    else if(FULL_MESH == SYS_PRODUCT_TOPO)
    {
        tseries_linecard_fullmesh_system_conn_init(product_type, 
			insert_board_type, insert_slotid);    
		
    }
 
    return BCM_E_NONE;      
   
}


long tseries_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
    int unit;
    int port;
    int rv;
    int find_port;
    int modid;
    int tid = -1;
    int i,j;


    if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO)
    {
    	if( SYS_MODULE_SLOT_INDEPENDENT_SERVICE(delete_slotid) )
    	{
			return tseries_linecard_central_system_indpnt_conn_init(
				product_type, delete_board_type, delete_slotid);
		}
    	
		if (SYS_CHASSIS_SLOTNO_ISFABRIC(delete_slotid))
		{

			int tid_offset;
			int master_slot;
			int module_id;
			int panel_port;
			
	        for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	        {
				int ftid_num = {0};
				int ftid[16] = {0};
				int fslot[16] = {0};
				int tid_offset;
				int master_slot;

	            module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
	                 SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

	            /* get all has created fabric trunk except the del slot*/
	            PBMP_HG_ITER(unit, port)
	            {
	                int modid, modport;
	                bcm_trunk_t tid;
					int plane_port;
					int peer_slot;
					
	                plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
	                       unit, port);
	                if(-1 == plane_port)
	                    continue;
	                peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
	                       plane_port);
					if(-1 == peer_slot)
					    continue;
					
					if (delete_slotid == peer_slot)
						continue;
			    
	                modid = module_id;
					modport = port;

	                if(port >= TSERIES_PORT_PER_ASICMODULE)
	                {
	                    modid = module_id + 1;
	                    modport -= TSERIES_PORT_PER_ASICMODULE;
	                }
	                rv = bcm_trunk_find(unit, modid, modport, &tid);         
	                if(BCM_E_NONE == rv)
	                {
						int temp_tid;
						/*the following code for get all trunks do not dst to insert slot*/
						for(temp_tid = 0; temp_tid < ftid_num; temp_tid++)
						{
							if(ftid[temp_tid] == tid)
								break;
						}
						if(temp_tid == ftid_num)
						{
							ftid[ftid_num] = tid;
							fslot[ftid_num] = peer_slot;
							ftid_num++;
						}
	                }
	            }        			
	            /*Firstly set the trunk for HG, here all HG should be stack port,
	              for the panel 10G port, should be set before this funciton*/
	            PBMP_HG_ITER(unit, port)
	            {
	                bcm_trunk_chip_info_t trunk_info;
	                bcm_trunk_add_info_t trunk_data;
	                int local_plane;
	                int peer_slot;
					int peer_plane;
	                int modport;
					int other_hg_port;
	                int trunk_slot_base;
	                int find_port = FALSE;
	                int modid;
	    			int trunk_port;
	                int max_fabrictrunk_num;
					int tid_base;
					int master_slot;
					int peer_moduletype;
					int peer_unit;
					int peer_port;
					int peer_module;



	                rv = bcm_trunk_chip_info_get(unit, &trunk_info);
	                if (rv < 0)
	                {
	                    npd_syslog_err("Crossbar init error: fail to get trunk chip info unit %d, error %s",
	                                       unit, bcm_errmsg(rv));
	                    return rv;
	                }

	                local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
	                           unit, port);
	                peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
	                if(-1 == peer_slot)
	                    continue;

	                if(delete_slotid != peer_slot)
	                    continue;

					peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
					if(0 == peer_moduletype)
						continue;	

	                peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
					if(-1 == peer_plane)
						continue;

					peer_unit = PPAL_PLANE_2_UNIT(peer_moduletype, peer_plane);						
					peer_port = PPAL_PLANE_2_PORT(peer_moduletype, peer_plane);
					peer_module = UNIT_2_MODULE(peer_moduletype, peer_slot, peer_unit, peer_port);


	                max_fabrictrunk_num 
						= trunk_info.trunk_fabric_id_max - trunk_info.trunk_fabric_id_min + 1;
						
	                tid_base = max_fabrictrunk_num/SYS_CHASSIS_MASTER_SLOTNUM;

	                for(master_slot = 0; master_slot < SYS_CHASSIS_MASTER_SLOTNUM;
					              master_slot++)
					{
						if(delete_slotid == SYS_CHASSIS_MASTER_SLOT_INDEX(master_slot))
							trunk_slot_base = master_slot*tid_base;
					}			  	
	                tid = trunk_slot_base + peer_unit;
					
					
	                /*port belongs a trunk*/
	                if (-1 == tid)
	                    continue;

	                tid = tid + trunk_info.trunk_fabric_id_min;

	                if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid, &trunk_data))
	                    continue;

	                bcm_stk_modid_get(unit, &modid);
	                modport = port;
	                if (port >= TSERIES_PORT_PER_ASICMODULE)
	                {
	                    modid += 1;
	                    modport -= TSERIES_PORT_PER_ASICMODULE;
	                }
	                for (i = 0; i < trunk_data.num_ports; i++)
	                {
						trunk_data.tm[i] = modid;
	                    if (trunk_data.tp[i] == modport)
	                    {
	                        find_port = TRUE;
	                        break;
	                    }
	                }
	                /*delete port from trunk*/
	                if ((find_port) && 
	                     (SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST
	                      || SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_HWINSERTED))
	                {
	                    int j;
	                    for(j = i; j < trunk_data.num_ports; j++)
	                    {
	                        trunk_data.tp[j] = trunk_data.tp[j+1];
	                        trunk_data.tm[j] = modid;
	                    }
	                    trunk_data.num_ports -= 1;
	                }

					rv = bcm_port_enable_set(unit, port, 0);

	                npd_syslog_dbg("%s %d trunk set: trunk id: %d, total num: %d, modid %d, port: %d(%s)\r\n", 
				        __func__, __LINE__, tid, trunk_data.num_ports, modid, port, SOC_PORT_NAME(unit, port));
					if(trunk_data.num_ports == 0)
					{
						rv = bcm_trunk_destroy(unit, tid_base);
						
					}
					else
				    {
	                    rv = bcm_trunk_set(unit, tid, &trunk_data);
					}
					

	                if (rv < 0)
	                {
	                    npd_syslog_err("Linecard switch deinit: fail to set trunk of unit %d tid %d\n err %s",
	                                       unit, tid, bcm_errmsg(rv));
	                }
					rv = bcm_port_enable_set(unit, port, 1);
	                npd_syslog_dbg("Linecard switch deinit: delete port modid %d port  %d to trunk %d\n",
	                    modid, port, tid);
	            }
				
				/*set mod destination information*/
				PBMP_E_ITER(unit, panel_port)
				{
	                bcm_trunk_add_info_t trunk_data;
					int p_port = PHY_2_PANEL(unit,panel_port);
					int array;
					int tid_select;
					int rv;
					int stk_modid = 0;
					if (ftid_num <= 0)
						break;
					
					if(p_port == -1)
			    		continue;
					array = (p_port-ETH_LOCAL_PORT_START_NO(SYS_LOCAL_MODULE_SLOT_INDEX))%ftid_num;
					tid_select = ftid[array];
	                rv = bcm_trunk_get(unit, tid_select, &trunk_data);
	                if(BCM_E_NONE != rv)
						npd_syslog_err("system connect can not find setted trunk data, tid %d\n", tid_select);
					
					for(stk_modid = 0; stk_modid < 64; stk_modid++)
					{
						int peer_slot = MODULE_2_SLOT_INDEX(stk_modid);
						int d_port;
						int d_num;
						int has_set = 0;
						
						if(peer_slot > CHASSIS_SLOT_COUNT)
							continue;
						for(d_num = 0; d_num < trunk_data.num_ports; d_num++)
						{
							if(SYS_CHASSIS_ISMASTERSLOT(peer_slot))
							{
								int peer_master_slot = 0;
								int need_add = 0;
								int local_plane = 0;
								int hg_port = 0;
															
								local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, trunk_data.tp[d_num]);
								peer_master_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
								if (peer_master_slot != peer_slot)
								{
									int iter_local_plane = 0;
									int iter_peer_slot;
									int iter_peer_moduletype;
									int iter_peer_unit;
									int iter_peer_port;
									int iter_peer_module;
									int iter_peer_plane;
									int iter_has_set;
									
									/* need add the other port to this module */
									PBMP_HG_ITER(unit, hg_port)
									{									
										if (hg_port == trunk_data.tp[d_num])
											continue;

										iter_local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, hg_port);
										if (-1 == iter_local_plane)	
											continue;
										
						                iter_peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, iter_local_plane);
										if (peer_slot != iter_peer_slot)
											continue;
										
										iter_peer_moduletype = MODULE_TYPE_ON_SLOT_INDEX(iter_peer_slot);
										if (0 == iter_peer_moduletype )
											continue;

						                iter_peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, iter_local_plane);
						                iter_peer_unit = PPAL_PLANE_2_UNIT(iter_peer_moduletype, iter_peer_plane);
						                iter_peer_port = PPAL_PLANE_2_PORT(iter_peer_moduletype, iter_peer_plane);
						                iter_peer_module = UNIT_2_MODULE(
											iter_peer_moduletype, iter_peer_slot, iter_peer_unit, iter_peer_port);
										if (stk_modid != iter_peer_module)
											continue;

										if(iter_has_set == 0)
										{
										    rv = bcm_stk_port_modport_set(unit, panel_port, 
										            stk_modid, hg_port);
											iter_has_set = 1;		
										}
										else
										{
										    rv = bcm_stk_port_modport_add(unit, panel_port, 
										            stk_modid, hg_port);
										}
									}
									continue;
								}
							}
							/* for first set module, set the port */
							if(has_set == 0)
							{
							    rv = bcm_stk_port_modport_set(unit, panel_port, 
							            stk_modid, trunk_data.tp[d_num]);
								has_set = 1;
							}
							else
							    rv = bcm_stk_port_modport_add(unit, panel_port, 
							            stk_modid, trunk_data.tp[d_num]);					
						}					
					}
				}
	        }
		}


        /*todo rcj*/
    	if( SYS_MODULE_EXTERNAL_SERVICE(delete_board_type) )		
    	{		
            for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
            {
            	int i = 0,j = 0;
    			bcm_trunk_add_info_t  trunkEntry;
    			int tid_cur;
    				
    			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(delete_slotid), unit, &tid_cur);
    			
    			rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
    			if(BCM_E_NONE == rv )
    			{
    				rv = bcm_trunk_destroy(unit, tid_cur);
    			}
    		}
    	}
    
    	if(SYS_CHASSIS_SLOTNO_ISFABRIC(delete_slotid))
    	{
    		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    		{
    			int plane_port = 0, peer_slot = 0;
    			int fabric_unit, fabric_port = 0, fabric_module;
                bcm_trunk_add_info_t trunk_data;
                int tid_base;
    			
    			for(plane_port = 0; plane_port < PPAL_PLANE_PORT_COUNT(delete_board_type); plane_port++)
    			{
    				if( -1 == PPAL_PLANE_2_UNIT(delete_board_type, plane_port))
    					continue;
    				
    				peer_slot = SLOT_PORT_PEER_SLOT(delete_slotid, plane_port);
    				if(-1 == peer_slot)
    					continue;
    
    				if(0 == MODULE_TYPE_ON_SLOT_INDEX(peer_slot) ||
    					!SYS_MODULE_EXTERNAL_SERVICE(MODULE_TYPE_ON_SLOT_INDEX(peer_slot)))
    				{
    					continue;
    				}
    				
    				fabric_unit = PPAL_PLANE_2_UNIT(delete_board_type, plane_port);	
    				fabric_port = PPAL_PLANE_2_PORT(delete_board_type, plane_port);
    				fabric_module = UNIT_2_MODULE(delete_board_type, delete_slotid, fabric_unit, fabric_port);
    				
    				nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(peer_slot), unit, &tid_base);				
                    if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
                    {
                        continue;
                    }
    				
                    for (i = 0,j = 0 ; j < trunk_data.num_ports; j++)
                    {
                        if (trunk_data.tp[j] == fabric_port && 
    						trunk_data.tm[j] == fabric_module )
                        {
                            continue;
                        }
                        
                        trunk_data.tp[i] = trunk_data.tp[j];
    					trunk_data.tm[i] = trunk_data.tm[j];
    					i++;
                    }
    				trunk_data.num_ports = i;
    				for(;i<BCM_TRUNK_MAX_PORTCNT;i++)
    				{
    					trunk_data.tp[i] = 0;
    					trunk_data.tm[i] = 0;
    				}
    
                    npd_syslog_dbg("%s %d trunk set: trunk id: %d, total num: %d, modid %d, port: %d\r\n", 
    			        		__func__, __LINE__, tid_base, trunk_data.num_ports, fabric_module, fabric_port);
    				rv = bcm_trunk_destroy(unit, tid_base);
    				rv = bcm_trunk_create_id(unit, tid_base);
                    rv = bcm_trunk_set(unit, tid_base, &trunk_data);
    
                    if (rv < 0)
                    {
                        npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s",
                                           unit, tid_base, bcm_errmsg(rv));
                    }
                    npd_syslog_dbg("Crossbar init: add port modid %d port  %d to trunk %d\n",
                        fabric_module, fabric_port, tid_base);			
    			}
    		}		
    	}        
    }
    else
    {
		if(SYS_MODULE_EXT_PORT(delete_board_type))
		{
			for(i = 0; i < PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE); i++)
			{
				bcm_pbmp_t bmp, ubmp;
				bcm_vlan_t vid;
				int peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, i);
				if(peer_slot != delete_slotid)
					continue;
				unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, i);
				if(-1 == unit)
					continue;
				port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, i);

				BCM_PBMP_PORT_SET(bmp, port);
				BCM_PBMP_CLEAR(ubmp);
				for(vid = 1; vid < 4094; vid++)
				{
					bcm_vlan_port_add(unit, vid, bmp, ubmp); 
				}
				
			}
		}
        /*todo rcj*/
    	if( SYS_MODULE_EXTERNAL_SERVICE(delete_board_type) )		
    	{		
            for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
            {
            	int i = 0,j = 0;
    			bcm_trunk_add_info_t  trunkEntry;
    			int tid_cur;
    				
    			nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(delete_slotid), unit, &tid_cur);
    			
    			rv = bcm_trunk_get(unit, tid_cur, &trunkEntry);			
    			if(BCM_E_NONE == rv )
    			{
    				rv = bcm_trunk_destroy(unit, tid_cur);
    			}
    		}
    	}
		tseries_linecard_fullmesh_system_conn_init(product_type, delete_board_type, 
			delete_slotid);
    }
    return BCM_E_NONE;      
   
}

#endif

#ifdef __cplusplus
}
#endif

