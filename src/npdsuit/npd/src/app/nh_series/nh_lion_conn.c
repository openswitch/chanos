#ifdef __cplusplus
extern "C"
{
#endif
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include "cpss/dxCh/dxChxGen/cpssDxChTypes.h"
#include "cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h"
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include "cpss/generic/cscd/cpssGenCscd.h"
#include "cpss/generic/bridge/cpssGenBrgFdb.h"
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include "cpss/generic/trunk/cpssGenTrunkTypes.h"
#include "cpss/generic/nst/cpssNstTypes.h"
#include "cpss/generic/pcl/cpssPcl.h"
#include "cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h"
#include "cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h"
#include "cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h"
#include "cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h"
#include "cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h"
#include "cpss/dxCh/dxChxGen/nst/cpssDxChNst.h"
#include "cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h"
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include "gtOs/gtGenTypes.h"
#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "nbm/nbm_api.h"
#include "npd_log.h"
#include "npd_trunk.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "board/ts_product_feature.h"
#define TRUNK_PORT_PCL_PORTA_2_PORTB 1
#define TRUNK_PORT_PCL_PORTB_2_PORTA 2
#define TRUNK_PORT_PCL_BIDIRECTION   3
#define START_MOD_ID 0
#define CSCD_PORT_PCL_ID 128
#define AX24_RIGHT_SLOT_TRUNK 127
#define AX24_LEFT_SLOT_TRUNK 126

#define SDK_DIFF_TRUNK 121

enum
{
	NOPP_PORT,
	DSA_PORT,
	SDKDIFF_PORT
};


typedef struct _nh_lion_board_conn_element_
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
}nh_lion_board_conn_element;

CPSS_PORTS_BMP_STC as_st_bmp[2];
nh_lion_board_conn_element nh_lion_board_conn[2][12];


void nh_lion_linecard_port_config(GT_U8 devNum, GT_U8 portNum, int portType)
{
	if (NOPP_PORT == portType)
	{
		npd_syslog_dbg("NOPP Port Config: Port(%d, %d)\n", devNum, portNum);
		/* Need config */
		cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
        cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, 1);
	}
	else if (DSA_PORT == portType)
	{
		npd_syslog_dbg("DSA Port Config: Port(%d, %d)\n", devNum, portNum);
		/* Need config */
        cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, 0);
		cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
	}
	else if (SDKDIFF_PORT == portType)
	{
		unsigned char mac_addr[6];
		npd_syslog_dbg("SDKDIFF Port Config: Port(%d, %d)\n", devNum, portNum);
				
		npd_system_get_basemac(mac_addr, 6);
		cpssDxChPortXGmiiModeSet(devNum, portNum, 0);/*2 = CPSS_PORT_XGMII_FIXED_E*/
		cpssDxChCscdPortTypeSet(devNum, portNum, 2, 2);/*2 = CPSS_CSCD_PORT_NETWORK_E*/
		cpssDxChPortIpgBaseSet(devNum, portNum, 1);/*CPSS_PORT_XG_FIXED_IPG_12_BYTES_E*/
		cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 8);
	
		cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
		cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, 1);
		cpssDxChIpPortRouterMacSaLsbModeSet(devNum, portNum, 0);
		cpssDxChIpRouterPortMacSaLsbSet(devNum, portNum, mac_addr[5]);
		cpssDxChPortEnableSet(devNum, portNum, 0);
		cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, 0);
    	cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);

        cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, 0);
		cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
	}
}


int nh_lion_linecard_vlan_entry_del(GT_U8 devNum, GT_U8 portNum)
{
	int ret;
	unsigned short vlanId;
	CPSS_PORTS_BMP_STC pbmp ;
	CPSS_PORTS_BMP_STC t_pbmp;
	CPSS_DXCH_BRG_VLAN_INFO_STC vlan_info;
	GT_BOOL valid;
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC port_tag_cmd ;
	unsigned int netif_index;
	int vlan_mode = 0;
	
	memset(&pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&t_pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&vlan_info, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
	memset(&port_tag_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

	for (vlanId = 1; vlanId < 4096; vlanId++)
	{
		ret = cpssDxChBrgVlanEntryRead(devNum, vlanId, 
 			                            &pbmp, &t_pbmp, 
 			                            &vlan_info, &valid, 
                                        &port_tag_cmd);
		if (ret != 0 || !valid)
		{
			//npd_syslog_dbg("%s: get vlan %d info error.\r\n", __func__, vlanId);
			continue;
		}

    	ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
    	if(ret != 0)
    	{
    	    continue;
    	}
		ret = npd_check_eth_port_status(netif_index);
		if(ret != -1)
		{
		    continue;
		}
		/* set  ports info */
		CPSS_PORTS_BMP_PORT_CLEAR_MAC(&pbmp, portNum);
		port_tag_cmd.portsCmd[portNum] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
		
		ret = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &pbmp, &t_pbmp, &vlan_info, &port_tag_cmd);		
		if(0 == ret)
		{
			npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Successful.\r\n");
		}
		else
	    {
			npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Failed.\r\n");
		}	
	}

	return 0;
}


int nh_lion_linecard_vlan_entry_add(GT_U8 devNum, GT_U8 portNum)
{
	int ret;
	unsigned short vlanId;
	CPSS_PORTS_BMP_STC pbmp ;
	CPSS_PORTS_BMP_STC t_pbmp;
	CPSS_DXCH_BRG_VLAN_INFO_STC vlan_info;
	GT_BOOL valid;
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC port_tag_cmd ;
	unsigned int netif_index;
	int vlan_mode = 0;
	
	memset(&pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&t_pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&vlan_info, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
	memset(&port_tag_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

	for (vlanId = 1; vlanId < 4096; vlanId++)
	{
		ret = cpssDxChBrgVlanEntryRead(devNum, vlanId, 
                             			&pbmp, &t_pbmp, 
                             			&vlan_info, &valid, 
                             			&port_tag_cmd);
		if (ret != 0 || !valid)
		{
			//npd_syslog_dbg("%s: get vlan %d info error.\r\n", __func__, vlanId);
			continue;
		}

		ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
    	if(ret == 0)
    	{
			ret = npd_switch_port_get_vlan_mode(netif_index, &vlan_mode);
			if(ret == 0)
			{
				npd_syslog_dbg("npd_switch_port_get_vlan_mode failed %x\r\n", netif_index);
			    continue;
			}
    	}
		/* set  ports info */
		CPSS_PORTS_BMP_PORT_SET_MAC(&pbmp, portNum);
		CPSS_PORTS_BMP_PORT_SET_MAC(&t_pbmp, portNum);
		port_tag_cmd.portsCmd[portNum] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
		
		ret = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &pbmp, &t_pbmp, &vlan_info, &port_tag_cmd);		
		if(0 == ret)
		{
			npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Successful.\r\n");
		}
		else
	    {
			npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Failed.\r\n");
		}	
	}

	return 0;
}


int nh_lion_linecard_trunk_ports_del(GT_U8 devNum, GT_U8 portNum)
{
	unsigned long ret = 0;
	GT_U8 myModNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	CPSS_TRUNK_MEMBER_STC memberPtr;
	GT_TRUNK_ID trunkId;
	memberPtr.device = myModNum;
	memberPtr.port = portNum;

	ret = cpssDxChTrunkDbIsMemberOfTrunk(devNum, &memberPtr, &trunkId);
	if (GT_OK != ret)
	{
		npd_syslog_dbg("cpssDxChTrunkDbIsMemberOfTrunk(%d, %d) Failed.\r\n", devNum, portNum);
		return TRUNK_CONFIG_FAIL;
	}

	ret = cpssDxChTrunkMemberRemove(devNum, trunkId, &memberPtr);
	if (0 == ret) 
	{
		ret = TRUNK_CONFIG_SUCCESS;
	}
	else 
	{ 
		npd_syslog_dbg("Trunk port(%d, %d) delete Failed.\r\n", devNum, portNum);
		ret = TRUNK_CONFIG_FAIL;
	}

	return ret;
}


int nh_lion_linecard_trunk_ports_add(GT_U8 devNum, GT_U8 portNum, GT_U16 trunkId)
{
	int i, j, ret = 0;
	static GT_U16 init_trunkId[2] = {0, 0};
	int trunk_is_configed = 0;
	GT_U8 myModNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	CPSS_TRUNK_MEMBER_STC memberPtr;
	int entry_index = 0;

	for(i = 0; i < 2; i++)
	{
		if(init_trunkId[i] == trunkId)
		{
			trunk_is_configed = 1;
			break;
		}
	}

	if(trunk_is_configed == 0)
	{
		ret = cpssDxChTrunkMembersSet(devNum, trunkId, 0, NULL, 0, NULL);
		if(ret != 0)
		{
			npd_syslog_dbg("%s %d: Trunk %d init failed.\r\n", __func__, __LINE__, trunkId);
			return ret;
		}
		for(i = 0; i < 2; i++)
		{
			if(init_trunkId[i] == 0)
			{
				init_trunkId[i] = trunkId;
				break;
			}
		}
	}
	
	memberPtr.device = myModNum;
	memberPtr.port = portNum;
	ret = cpssDxChTrunkMemberAdd(devNum, trunkId, &memberPtr);
	if(ret != 0)
	{
	    npd_syslog_dbg("%s %d: Add Cscd ports %d:%d to trunk %d failed.\r\n", __func__, __LINE__, 
		             memberPtr.device, 
		             memberPtr.port,  trunkId);
		return ret;
	}
	
	return 0;
}


long nh_lion_linecard_local_conn_init(int product_type)
{
	GT_STATUS ret = 0;
	int i = 0;
	int devNum = 0;
	int modNum = 0;
	int portNum = 0, maxMod = 0;
	int plane_port;
	int instance_num = nam_asic_get_instance_num();
	int plane_port_num = PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE);
	maxMod = SYS_CHASSIS_SLOTNUM * CONN_MAX_GMODULE_PERSLOT;

	for(devNum = 0; devNum < instance_num; devNum++)
	{
		CPSS_PORTS_BMP_STC          cscdPortBmp;
		GT_U32 devTableBmp = 0;
		memset(&cscdPortBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
		memset(&as_st_bmp[devNum], 0, sizeof(CPSS_PORTS_BMP_STC));
        modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
        ret = cpssDxChCfgHwDevNumSet(devNum, modNum);/*set the mod id*/
		/*dev map with devnum and portnum*/
		ret = cpssDxChCscdDevMapLookupModeSet(devNum, 0);/*0 = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E*/
	    ret = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum, 1);
		cpssDxChCscdDsaSrcDevFilterSet(devNum, 1);/*防止广播报文成环*/
		cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(devNum, 0);
		cpssDxChTrunkHashDesignatedTableModeSet(devNum, 0);/*multi-destination packet use first index of designated table*/

        devTableBmp = 0xFFFFF;
		cpssDxChBrgFdbDeviceTableSet(devNum, devTableBmp);
		cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(devNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);
		for(portNum = 0; portNum < 12; portNum++)
		{
			cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, modNum);
			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(devNum, portNum, 1);
			cpssDxChIpRouterSourceIdSet(devNum, 0 , modNum);
			cpssDxChIpRouterSourceIdSet(devNum, 1 , modNum);
			CPSS_PORTS_BMP_PORT_SET_MAC(&cscdPortBmp, portNum);
		}
		for(i = 0; i < maxMod; i++)
		{
			if(modNum == i)
			{
			    CPSS_PORTS_BMP_PORT_SET_MAC(&cscdPortBmp, 15);/*cpu port for group 0*/
			    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_TRUE, &cscdPortBmp);
			}
			else
			{
				/*for 9610/9606, always add cpu to src-id entry */
			    CPSS_PORTS_BMP_PORT_SET_MAC(&cscdPortBmp, 15);/*cpu port for group 0*/
			    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_TRUE, &cscdPortBmp);
			}
		}
		for(i = 0; i < 8; i++)
		{
			cpssDxChNetIfSdmaRxResourceErrorModeSet(devNum, i, CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E);
		}
	}
	/* 清除所有的原先学习到的静态FDB表项 */
	for(devNum = 0; devNum < instance_num; devNum++)
	{
		ret = nam_fdb_table_delete_dynamic_all(devNum);
	}

    return 0;  
}

int npd_get_modport_tid_by_global_index(unsigned int globle_index, int *tid, 
												unsigned char *mod, unsigned char *port)
{
	int ret, i;
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


int nh_lion_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int i, unit, plane_port_num, port_num;
	int plane_port, peer_slot, peer_port, peer_type;
	int modid, peer_unit, maxMod;
	int ret;
	maxMod = SYS_CHASSIS_SLOTNUM * CONN_MAX_GMODULE_PERSLOT;
	
	npd_syslog_dbg("\n******* Entering %s ********* \n\n", __func__);
	if(SYS_MODULE_RUNNINGSTATE(insert_slotid) == RMT_BOARD_NOEXIST)
	{
		npd_syslog_dbg("%s %d: Insered board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, insert_slotid);
		return 0;
	}

	plane_port_num = PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE);
	
	if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO &&
		SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid))
    {		
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(port_num = 0; port_num < plane_port_num; port_num++)
			{
				memset(&nh_lion_board_conn[unit][port_num], 0, sizeof(nh_lion_board_conn_element));

				plane_port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, port_num);
				cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, plane_port, 0);

				nh_lion_board_conn[unit][port_num].local_dev = unit;
				nh_lion_board_conn[unit][port_num].local_port = plane_port;

				nh_lion_linecard_trunk_ports_del(unit, plane_port);
				
				peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, port_num);
				if(peer_slot == -1)
				{
					npd_syslog_dbg("%s %d: Can not get peer slot.\r\n", __func__, __LINE__);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}
				if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
				{
					npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}

				peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
				if(peer_type == 0)
				{
					npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}
				peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, port_num);
				if(peer_port == -1)
				{
					nh_lion_board_conn[unit][port_num].peer_port = -1;
					continue;
				}
				peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);
				peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
				nh_lion_board_conn[unit][port_num].peer_type = peer_type;
				nh_lion_board_conn[unit][port_num].peer_slot = peer_slot;

				if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
				{
					/*Different SDK type case*/
					npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
					nh_lion_board_conn[unit][port_num].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
					nh_lion_board_conn[unit][port_num].peer_port = peer_port;
					npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
						nh_lion_board_conn[unit][port_num].peer_mod, nh_lion_board_conn[unit][port_num].peer_port);

					cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, plane_port, 
									nh_lion_board_conn[unit][port_num].peer_mod);
					cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, plane_port, 1);
					
					CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[unit], plane_port);
					nh_lion_linecard_port_config(unit, plane_port, SDKDIFF_PORT);
				    nh_lion_linecard_vlan_entry_add(unit, plane_port);
					
					nh_lion_board_conn[unit][port_num].trunk_member = 1;
					nh_lion_board_conn[unit][port_num].tid = 127;
				}
				else
				{
					nh_lion_board_conn[unit][port_num].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
					nh_lion_board_conn[unit][port_num].peer_port = peer_port;
					npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
						nh_lion_board_conn[unit][port_num].peer_mod, nh_lion_board_conn[unit][port_num].peer_port);

					CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[unit], plane_port);
					nh_lion_linecard_port_config(unit, plane_port, DSA_PORT);
				    nh_lion_linecard_vlan_entry_add(unit, plane_port);
				}
			}
		}

		/* Debug */
		npd_syslog_dbg("\nnh_lion_board_conn info \n\n");
		npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
					   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(i = 0; i < plane_port_num; i++)
			{
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].local_dev);
				npd_syslog_dbg("%-11d", nh_lion_board_conn[unit][i].local_port);
				npd_syslog_dbg("%-9d", nh_lion_board_conn[unit][i].peer_mod);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_port);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_type);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_slot);
				npd_syslog_dbg("%-13d", nh_lion_board_conn[unit][i].trunk_member);
				npd_syslog_dbg("%-5d", nh_lion_board_conn[unit][i].tid);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].is_dest_port);
				npd_syslog_dbg("%-9d", nh_lion_board_conn[unit][i].is_src_port);
				npd_syslog_dbg("%-14d", nh_lion_board_conn[unit][i].need_redirect);
				npd_syslog_dbg("%-16d\n", nh_lion_board_conn[unit][i].redirect_from_port);
			}
		}

		/* Trunk Config */
		npd_syslog_dbg("\ntrunk config\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(i = 0; i < plane_port_num; i++)
			{
				if(nh_lion_board_conn[unit][i].peer_slot == -1)
	    		{
	    			continue;
	    		}
				if(nh_lion_board_conn[unit][i].trunk_member == 1)
				{
					npd_syslog_dbg("devNum = %d	tid = %d	portNum  = %d\n", 
							unit, nh_lion_board_conn[unit][i].tid, 
							nh_lion_board_conn[unit][i].local_port);
					nh_lion_linecard_trunk_ports_add(unit,
												  nh_lion_board_conn[unit][i].local_port,
												  nh_lion_board_conn[unit][i].tid);
				}
			}
		}

		/*multi-destination flow. (src-id)*/
		npd_syslog_dbg("\nsrc-id config\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {
			int srcid = 0;
			for(i = 0; i < 12; i++)
			{
	    		if(nh_lion_board_conn[unit][i].peer_slot == -1 || 
					nh_lion_board_conn[unit][i].peer_type == 0)
	    		{
	    			continue;
	    		}
				for(srcid = 0; srcid < maxMod; srcid++)
				{
				    if(nh_lion_board_conn[unit][i].peer_mod == srcid)
				    {
				        continue;
				    }
					if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, nh_lion_board_conn[unit][i].local_port);
					    ret = cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, nh_lion_board_conn[unit][i].local_port);
					}
				}
			}
	    }

		/*dev map*/
		npd_syslog_dbg("\ndev map config\n"); 
	    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {	
	    	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
	    	int dest_mod = 0;
	        for(dest_mod = 0; dest_mod < maxMod; dest_mod++)
	        {
	        	int trunk_connect = 0;
	        	if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0)
					||(dest_mod == (UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0) +1)))
				{
					continue;
				}
				for(i = 0; i < 12; i++)
	    		{
	        		if(nh_lion_board_conn[unit][i].peer_slot == -1 || 
	    				nh_lion_board_conn[unit][i].peer_type == 0)
	        		{
	        			continue;
	        		}
					
	    			if(nh_lion_board_conn[unit][i].trunk_member)
	    			{
						trunk_connect = 1;
						break;
	    			}
	    		}
				if(trunk_connect)
				{
	    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
	    			cascadeLinkPtr.linkNum = nh_lion_board_conn[unit][i].tid;
					npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
						dest_mod, nh_lion_board_conn[unit][i].tid);
	    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
				}
		    }
	    }

		npd_syslog_dbg("\nEnable Port \n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {
			for(i = 0; i < plane_port_num; i++)
			{
	    		if(nh_lion_board_conn[unit][i].peer_slot == -1)
	    		{
	    			continue;
	    		}
				cpssDxChPortEnableSet(nh_lion_board_conn[unit][i].local_dev, nh_lion_board_conn[unit][i].local_port, 1);
			}
	    }
		
	}
	else if(FULL_MESH == SYS_PRODUCT_TOPO)
	{
	
	}

	npd_syslog_dbg("\n******* Leaving  %s ********* \n\n", __func__);

	return 0;
}

int nh_lion_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	int i, unit, plane_port_num, port_num;
	int plane_port, peer_slot, peer_port, peer_type;
	int modid, peer_unit, maxMod;
	int ret;
	maxMod = SYS_CHASSIS_SLOTNUM * CONN_MAX_GMODULE_PERSLOT;
	
	npd_syslog_dbg("\n******* Entering %s ********* \n\n", __func__);

	plane_port_num = PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE);
	
	if(CENTRAL_FABRIC == SYS_PRODUCT_TOPO
		&&SYS_CHASSIS_SLOTNO_ISFABRIC(delete_slotid))
    {
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(port_num = 0; port_num < plane_port_num; port_num++)
			{
				memset(&nh_lion_board_conn[unit][port_num], 0, sizeof(nh_lion_board_conn_element));

				plane_port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, port_num);
				cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, plane_port, 0);

				nh_lion_board_conn[unit][port_num].local_dev = unit;
				nh_lion_board_conn[unit][port_num].local_port = plane_port;

				nh_lion_linecard_trunk_ports_del(unit, plane_port);
				
				peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, port_num);
				if(peer_slot == -1)
				{
					npd_syslog_dbg("%s %d: Can not get peer slot.\r\n", __func__, __LINE__);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}
				if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST || (peer_slot == delete_slotid))
				{
					npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}

				peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
				if(peer_type == 0)
				{
					npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
					nh_lion_board_conn[unit][port_num].peer_slot = -1;
					continue;
				}
				peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, port_num);
				if(peer_port == -1)
				{
					nh_lion_board_conn[unit][port_num].peer_port = -1;
					continue;
				}
				peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);
				peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
				nh_lion_board_conn[unit][port_num].peer_type = peer_type;
				nh_lion_board_conn[unit][port_num].peer_slot = peer_slot;

				if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
				{
					/*Different SDK type case*/
					npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
					nh_lion_board_conn[unit][port_num].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
					nh_lion_board_conn[unit][port_num].peer_port = peer_port;
					npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
						nh_lion_board_conn[unit][port_num].peer_mod, nh_lion_board_conn[unit][port_num].peer_port);

					cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, plane_port, 
									nh_lion_board_conn[unit][port_num].peer_mod);
					cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, plane_port, 1);
					
					CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[unit], plane_port);
					nh_lion_linecard_port_config(unit, plane_port, SDKDIFF_PORT);
				    nh_lion_linecard_vlan_entry_add(unit, plane_port);
					
					nh_lion_board_conn[unit][port_num].trunk_member = 1;
					nh_lion_board_conn[unit][port_num].tid = 127;
				}
				else
				{
					nh_lion_board_conn[unit][port_num].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
					nh_lion_board_conn[unit][port_num].peer_port = peer_port;
					npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
						nh_lion_board_conn[unit][port_num].peer_mod, nh_lion_board_conn[unit][port_num].peer_port);

					CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[unit], plane_port);
					nh_lion_linecard_port_config(unit, plane_port, DSA_PORT);
				    nh_lion_linecard_vlan_entry_add(unit, plane_port);
				}
			}
		}

		/* Debug */
		npd_syslog_dbg("\nnh_lion_board_conn info \n\n");
		npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
					   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(i = 0; i < plane_port_num; i++)
			{
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].local_dev);
				npd_syslog_dbg("%-11d", nh_lion_board_conn[unit][i].local_port);
				npd_syslog_dbg("%-9d", nh_lion_board_conn[unit][i].peer_mod);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_port);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_type);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].peer_slot);
				npd_syslog_dbg("%-13d", nh_lion_board_conn[unit][i].trunk_member);
				npd_syslog_dbg("%-5d", nh_lion_board_conn[unit][i].tid);
				npd_syslog_dbg("%-10d", nh_lion_board_conn[unit][i].is_dest_port);
				npd_syslog_dbg("%-9d", nh_lion_board_conn[unit][i].is_src_port);
				npd_syslog_dbg("%-14d", nh_lion_board_conn[unit][i].need_redirect);
				npd_syslog_dbg("%-16d\n", nh_lion_board_conn[unit][i].redirect_from_port);
			}
		}

		/* Trunk Config */
		npd_syslog_dbg("\ntrunk config\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
		{
			for(i = 0; i < plane_port_num; i++)
			{
				if(nh_lion_board_conn[unit][i].peer_slot == -1)
	    		{
	    			continue;
	    		}
				if(nh_lion_board_conn[unit][i].trunk_member == 1)
				{
					npd_syslog_dbg("devNum = %d	tid = %d	portNum  = %d\n", 
							unit, nh_lion_board_conn[unit][i].tid, 
							nh_lion_board_conn[unit][i].local_port);
					nh_lion_linecard_trunk_ports_add(unit,
												  nh_lion_board_conn[unit][i].local_port,
												  nh_lion_board_conn[unit][i].tid);
				}
			}
		}

		/*multi-destination flow. (src-id)*/
		npd_syslog_dbg("\nsrc-id config\n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {
			int srcid = 0;
			for(i = 0; i < 12; i++)
			{
	    		if(nh_lion_board_conn[unit][i].peer_slot == -1 || 
					nh_lion_board_conn[unit][i].peer_type == 0)
	    		{
	    			continue;
	    		}
				for(srcid = 0; srcid < maxMod; srcid++)
				{
				    if(nh_lion_board_conn[unit][i].peer_mod == srcid)
				    {
				        continue;
				    }
					if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, nh_lion_board_conn[unit][i].local_port);
					    ret = cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, nh_lion_board_conn[unit][i].local_port);
					}
				}
			}
	    }

		/*dev map*/
		npd_syslog_dbg("\ndev map config\n"); 
	    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {	
	    	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
	    	int dest_mod = 0;
	        for(dest_mod = 0; dest_mod < maxMod; dest_mod++)
	        {
	    		int trunk_connect = 0;
	        	if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0)
					||(dest_mod == (UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0)+1)))
				{
					continue;
				}
				for(i = 0; i < 12; i++)
	    		{
	        		if(nh_lion_board_conn[unit][i].peer_slot == -1 || 
	    				nh_lion_board_conn[unit][i].peer_type == 0)
	        		{
	        			continue;
	        		}
					
	    			if(nh_lion_board_conn[unit][i].trunk_member)
	    			{
						trunk_connect = 1;
						break;
	    			}
	    		}
				if(trunk_connect)
				{
	    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
	    			cascadeLinkPtr.linkNum = nh_lion_board_conn[unit][i].tid;
					npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
						dest_mod, nh_lion_board_conn[unit][i].tid);
	    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
				}
		    }
	    }

		npd_syslog_dbg("\nEnable Port \n");
		for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	    {
			for(i = 0; i < plane_port_num; i++)
			{
	    		if(nh_lion_board_conn[unit][i].peer_slot == -1)
	    		{
	    			continue;
	    		}
				cpssDxChPortEnableSet(nh_lion_board_conn[unit][i].local_dev, nh_lion_board_conn[unit][i].local_port, 1);
			}
	    }
		
	}
	else if(FULL_MESH == SYS_PRODUCT_TOPO)
	{
	
	}

	npd_syslog_dbg("\n******* Leaving  %s ********* \n\n", __func__);

	return 0;
}

#ifdef __cplusplus
}
#endif

