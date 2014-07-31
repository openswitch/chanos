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
#include "cpss/dxCh/dxChxGen/cpssDxChTypes.h"
#include "cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h"
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include "cpss/generic/cscd/cpssGenCscd.h"
#include "cpss/generic/bridge/cpssGenBrgFdb.h"
#include "cpss/generic/trunk/cpssGenTrunkTypes.h"
#include "cpss/generic/nst/cpssNstTypes.h"
#include "cpss/generic/pcl/cpssPcl.h"
#include "gtOs/gtGenTypes.h"
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
#include "board/ts_product_feature.h"
extern int nam_fdb_table_delete_dynamic_all(int unit);
#define TRUNK_PORT_PCL_PORTA_2_PORTB 1
#define TRUNK_PORT_PCL_PORTB_2_PORTA 2
#define TRUNK_PORT_PCL_BIDIRECTION   3
#define START_MOD_ID 0
#define CSCD_PORT_PCL_ID 128
#define AX24_RIGHT_SLOT_TRUNK 127
#define AX24_LEFT_SLOT_TRUNK 126
int ax63ge48_port_pcl_init(GT_U8 devNum, GT_U8 portNum)
{
	CPSS_INTERFACE_INFO_STC 		interfaceInfoPtr;
	CPSS_PCL_DIRECTION_ENT			direction = CPSS_PCL_DIRECTION_INGRESS_E;
	CPSS_PCL_LOOKUP_NUMBER_ENT		lookupNum0 = CPSS_PCL_LOOKUP_0_E;
	CPSS_DXCH_PCL_LOOKUP_CFG_STC	lookupCfgPtr0 = {0};
	CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT	  mode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;
	GT_STATUS					ret = 0;
	PRV_CPSS_DXCH_TABLE_ENT 		tableId = PRV_CPSS_DXCH_TABLE_PCL_CONFIG_E;
	CPSS_DXCH_PCL_RULE_FORMAT_UNT mask_format;
	CPSS_DXCH_PCL_RULE_FORMAT_UNT pattern_format;
	CPSS_DXCH_PCL_ACTION_STC pcl_action;
    memset(&interfaceInfoPtr, 0, sizeof(CPSS_INTERFACE_INFO_STC));
	interfaceInfoPtr.type = CPSS_INTERFACE_PORT_E; 
	interfaceInfoPtr.devPort.portNum = portNum;
	interfaceInfoPtr.devPort.devNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	
	mode = CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;
	
	lookupCfgPtr0.enableLookup = GT_TRUE;
	lookupCfgPtr0.pclId = CSCD_PORT_PCL_ID;
	
    lookupCfgPtr0.groupKeyTypes.nonIpKey =
									CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
	lookupCfgPtr0.groupKeyTypes.ipv4Key =
									CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;	
	lookupCfgPtr0.groupKeyTypes.ipv6Key =
						       		CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;


	ret = cpssDxChPclPortLookupCfgTabAccessModeSet(devNum, portNum, direction,lookupNum0, 0, mode, 0);
	
	ret = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_TRUE);
	
	ret = cpssDxChPclCfgTblSet(devNum, &interfaceInfoPtr, direction,
									  lookupNum0, &lookupCfgPtr0);
    if(ret != 0)
    {
		npd_syslog_dbg("%s %d: Set PCL cft table failed.(%d)\r\n", __func__, __LINE__, ret);
    }
#if 0
	memset(&mask_format, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
	memset(&pattern_format, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
	memset(&pcl_action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
	ret = cpssDxChPclRuleSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
			10, CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E, &mask_format, &pattern_format, &pcl_action);
    if(ret != 0)
    {
		npd_syslog_dbg("%s %d: Set PCL default rule (%d) failed.(%d)\r\n", __func__, __LINE__, 10, ret);
    }
	ret = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E, 10, GT_TRUE);
    if(ret != 0)
    {
		npd_syslog_dbg("%s %d: Valid PCL default rule (%d) failed.(%d)\r\n", __func__, __LINE__, 10, ret);
    }
#endif
	return ret;	
}

int ax63ge48_trunk_pcl_config(GT_U8 devNum, GT_U8 portNumA, GT_U8 portNumB, GT_U8 neiborModNum, int rule_level, int slot_or_unit)
{
	GT_STATUS ret = 0;
	int i = 0;
	int modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	CPSS_DXCH_PCL_RULE_FORMAT_UNT mask_format;
	CPSS_DXCH_PCL_RULE_FORMAT_UNT pattern_format;
	CPSS_DXCH_PCL_ACTION_STC pcl_action;
	int rule_base = rule_level*3;
	memset(&mask_format, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
	memset(&pattern_format, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
	memset(&pcl_action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    ax63ge48_port_pcl_init(devNum, portNumA);
	
	mask_format.ruleIngrStdUdb.commonIngrUdb.pclId = 0xFFFF;
	mask_format.ruleIngrStdUdb.commonIngrUdb.isL2Valid = 0;
	mask_format.ruleIngrStdUdb.commonIngrUdb.isUdbValid = 0;
	mask_format.ruleIngrStdUdb.commonIngrUdb.sourcePort = 0xFF;
	mask_format.ruleIngrStdUdb.commonIngrUdb.portListBmp = 0;
	
	pattern_format.ruleIngrStdUdb.commonIngrUdb.pclId = CSCD_PORT_PCL_ID;
	pattern_format.ruleIngrStdUdb.commonIngrUdb.isL2Valid = 0;
	pattern_format.ruleIngrStdUdb.commonIngrUdb.isUdbValid = 0;
	pattern_format.ruleIngrStdUdb.commonIngrUdb.sourcePort = portNumA;
	pattern_format.ruleIngrStdUdb.commonIngrUdb.portListBmp = 0;
	
	/*if the packet a multi-destination packet(use_vidx == 1)? forward it*/
	for(i = 8; (i > 0); i--)
	{
	    ret = cpssDxChPclUserDefinedByteSet(devNum, 
		    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E, 
		    (CPSS_DXCH_PCL_PACKET_TYPE_ENT)(i - 1),
		    0,/*CPSS_PCL_DIRECTION_INGRESS_E*/
		    3,
		    CPSS_DXCH_PCL_OFFSET_L2_E,18
		    );
	}
	
	mask_format.ruleIngrStdUdb.udb[3] = 0x10;
	
	pattern_format.ruleIngrStdUdb.udb[3] = 0x10;
	
	pcl_action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
	pcl_action.actionStop = GT_TRUE;
	ret = cpssDxChPclRuleSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
			rule_base + 0, CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E, &mask_format, &pattern_format, &pcl_action);
	ret = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E, rule_base + 0, GT_TRUE);

	/*SRC DEV*/
	for(i = 8; (i > 0); i--)
	{
	    ret = cpssDxChPclUserDefinedByteSet(devNum, 
		    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E, 
		    (CPSS_DXCH_PCL_PACKET_TYPE_ENT)(i - 1),
		    0,/*CPSS_PCL_DIRECTION_INGRESS_E*/
		    4,
		    CPSS_DXCH_PCL_OFFSET_L2_E,12
		    );
	}
	/*if SRC DEV == MY HW DEVNUM, drop it*/
	/*
	mask_format.ruleIngrStdUdb.udb[3] = 0;
	mask_format.ruleIngrStdUdb.udb[4] = 0x1F;
	
	pattern_format.ruleIngrStdUdb.udb[3] = 0;
	pattern_format.ruleIngrStdUdb.udb[4] = modNum;
	
	pcl_action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
	pcl_action.actionStop = GT_FALSE;
	ret = cpssDxChPclRuleSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
			rule_base + 1, CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E, &mask_format, &pattern_format, &pcl_action);
	ret = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E, rule_base + 1, GT_TRUE);
       */
	/*TARGET DEV*/
	/*if TARGET DEV == my hw devnum, forward it*/
	for(i = 8; (i > 0); i--)
	{
	    ret = cpssDxChPclUserDefinedByteSet(devNum, 
		    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E, 
		    (CPSS_DXCH_PCL_PACKET_TYPE_ENT)(i - 1),
		    0,/*CPSS_PCL_DIRECTION_INGRESS_E*/
		    5,
		    CPSS_DXCH_PCL_OFFSET_L2_E,19
		    );
	}
	mask_format.ruleIngrStdUdb.udb[3] = 0;
	mask_format.ruleIngrStdUdb.udb[4] = 0;
	mask_format.ruleIngrStdUdb.udb[5] = 0x1F;
	
	pattern_format.ruleIngrStdUdb.udb[3] = 0;
	pattern_format.ruleIngrStdUdb.udb[4] = 0;
	pattern_format.ruleIngrStdUdb.udb[5] = modNum;
	
	pcl_action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
	pcl_action.bypassBridge = GT_TRUE;
	pcl_action.bypassIngressPipe = GT_TRUE;
	pcl_action.actionStop = GT_TRUE;
	ret = cpssDxChPclRuleSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
			rule_base + 1, CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E, &mask_format, &pattern_format, &pcl_action);
	ret = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E, rule_base + 1, GT_TRUE);
	
    /*if TARGET DEVNUM == neibor hw devnum, redirect it to portB*/
	mask_format.ruleIngrStdUdb.udb[3] = 0;
	mask_format.ruleIngrStdUdb.udb[4] = 0;
	if(slot_or_unit == 1)
	{
	    mask_format.ruleIngrStdUdb.udb[5] = 0x1E;
	}
	else
	{
	    mask_format.ruleIngrStdUdb.udb[5] = 0x1F;
	}

	pattern_format.ruleIngrStdUdb.udb[3] = 0;
	pattern_format.ruleIngrStdUdb.udb[4] = 0;
	if(slot_or_unit == 1)
	{
	    pattern_format.ruleIngrStdUdb.udb[5] = (neiborModNum & 0x1E);
	}
	else
	{
	    pattern_format.ruleIngrStdUdb.udb[5] = (neiborModNum & 0x1F);
	}
	
	memset(&pcl_action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
	pcl_action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
	pcl_action.bypassBridge = GT_TRUE;
	pcl_action.actionStop = GT_TRUE;
	pcl_action.bypassIngressPipe = GT_TRUE;
	pcl_action.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    pcl_action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
	pcl_action.redirect.data.outIf.outInterface.devPort.devNum = modNum;
	pcl_action.redirect.data.outIf.outInterface.devPort.portNum = portNumB;
	ret = cpssDxChPclRuleSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
			rule_base + 2, CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E, &mask_format, &pattern_format, &pcl_action);
	ret = cpssDxChPclRuleValidStatusSet(devNum, CPSS_PCL_RULE_SIZE_STD_E, rule_base + 2, GT_TRUE);

}

void ax63ge48_trunk_pcl_deinit(GT_U8 devNum, GT_U8 portNum)
{
	cpssDxChPclPortIngressPolicyEnable(devNum, portNum, 0);
}

int ax63ge48_trunk_port_config(GT_U8 devNum, GT_TRUNK_ID trunkId, GT_U8 portNumA, GT_U8 portNumB, GT_U8 designatedPort, GT_U8 pclMod, GT_U8 portAConnMod, GT_U8 portBConnMod, int rule_level, int slot_or_unit)
{
	int ret = 0;
	GT_U8 myModNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	CPSS_TRUNK_MEMBER_STC trunk_member[2];
	CPSS_PORTS_BMP_STC          trunkDesignatedBmp;
	int entry_index = 0;
	trunk_member[0].device = myModNum;
	trunk_member[0].port = portNumA;
	trunk_member[1].device = myModNum;
	trunk_member[1].port = portNumB;
	ret = cpssDxChTrunkMembersSet(devNum, trunkId, 2, trunk_member, 0, NULL);
	if(ret != 0)
	{
	    npd_syslog_dbg("%s %d: Add Cscd ports %d:%d and %d:%d to trunk %d failed.\r\n", __func__, __LINE__, 
		             trunk_member[0].device, 
		             trunk_member[0].port, 
		             trunk_member[1].device, 
		             trunk_member[1].port, trunkId);
		return ret;
	}
	cpssDxChPortEnableSet(devNum, portNumA, 1);
	cpssDxChPortEnableSet(devNum, portNumB, 1);
	npd_syslog_dbg("%s %d: Add Cscd ports %d:%d and %d:%d to trunk %d\r\n", __func__, __LINE__, 
		             trunk_member[0].device, 
		             trunk_member[0].port, 
		             trunk_member[1].device, 
		             trunk_member[1].port, trunkId);
	if(designatedPort == portNumA || designatedPort == portNumB)
	{
	    for(entry_index = 0; entry_index < 8; entry_index++)
    	{
    		cpssDxChTrunkDesignatedPortsEntryGet(devNum, entry_index, &trunkDesignatedBmp);
    		if(entry_index%2 == 0)
    		{
    			if(designatedPort == portNumA)
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumB);
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumA);
    			}
    			else
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumA);
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumB);
    			}
    		}
    		else
    		{
    			if(designatedPort != portNumA)
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumB);
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumA);
    			}
    			else
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumA);
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumB);
    			}
    		}
    		cpssDxChTrunkDesignatedPortsEntrySet(devNum, entry_index, &trunkDesignatedBmp);
    	}
	    npd_syslog_dbg("%s %d: Set port %d as designated port for multi-destination packets\r\n", __func__, __LINE__, 
		             designatedPort);
		cpssDxChBrgSrcIdGroupPortAdd(devNum, myModNum, designatedPort);
	}
	else
	{
		cpssDxChBrgSrcIdGroupPortAdd(devNum, myModNum, portNumA);
		cpssDxChBrgSrcIdGroupPortAdd(devNum, myModNum, portNumB);
	}
	
	switch(pclMod)
	{
		case TRUNK_PORT_PCL_PORTA_2_PORTB:
			ax63ge48_trunk_pcl_config(devNum, portNumA, portNumB, portBConnMod, rule_level, slot_or_unit);
			npd_syslog_dbg("%s %d: Redirect packet to destination mod %d from Cscd port %d to Cscd port %d\r\n", __func__, __LINE__, 
		             portBConnMod, portNumA, portNumB);
			break;
		case TRUNK_PORT_PCL_PORTB_2_PORTA:
			ax63ge48_trunk_pcl_config(devNum, portNumB, portNumA, portAConnMod, rule_level, slot_or_unit);
			npd_syslog_dbg("%s %d: Redirect packet to destination mod %d from Cscd port %d to Cscd port %d\r\n", __func__, __LINE__, 
		             portAConnMod, portNumB, portNumA);
			break;
		case TRUNK_PORT_PCL_BIDIRECTION:
			ax63ge48_trunk_pcl_config(devNum, portNumA, portNumB, portBConnMod, rule_level, slot_or_unit);
			npd_syslog_dbg("%s %d: Redirect packet to destination mod %d from Cscd port %d to Cscd port %d\r\n", __func__, __LINE__, 
		             portBConnMod, portNumA, portNumB);
			ax63ge48_trunk_pcl_config(devNum, portNumB, portNumA, portAConnMod, rule_level + 1, slot_or_unit);
			npd_syslog_dbg("%s %d: Redirect packet to destination mod %d from Cscd port %d to Cscd port %d\r\n", __func__, __LINE__, 
		             portAConnMod, portNumB, portNumA);
			break;
	}
}

void as_series_linecard_srcid_init(GT_U8 devNum)
{
	int i = 0;
	int portNum = 0;
	int modNum = 0;

	CPSS_PORTS_BMP_STC          cscdPortBmp;
	
	memset(&cscdPortBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	for(portNum = 0; portNum < 24; portNum++)
	{
		CPSS_PORTS_BMP_PORT_SET_MAC(&cscdPortBmp, portNum);
	}
    modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
         SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
	
	for(i = 0; i < 6; i++)
	{
		if(modNum == i)
		{
		    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_TRUE, &cscdPortBmp);
		}
		else
		{
		    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_FALSE, &cscdPortBmp);
		}		
	}
}

long as_series_linecard_local_conn_init(int product_type)
{
	GT_STATUS ret = 0;
	int i = 0;
	int devNum = 0;
	int modNum = 0;
	int portNum = 0;
	int instance_num = nam_asic_get_instance_num();

	for(devNum = 0; devNum < instance_num; devNum++)
	{
		CPSS_PORTS_BMP_STC          cscdPortBmp;
		memset(&cscdPortBmp, 0, sizeof(CPSS_PORTS_BMP_STC));
        modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, devNum, 0);
        ret = cpssDxChCfgHwDevNumSet(devNum, modNum);/*set the mod id*/
		/*dev map with devnum and portnum*/
		ret = cpssDxChCscdDevMapLookupModeSet(devNum, 0);/*0 = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E*/
	    ret = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum, 1);
		cpssDxChCscdDsaSrcDevFilterSet(devNum, 1);/*防止广播报文成环*/
		cpssDxChBrgSrcIdGlobalUcastEgressFilterSet(devNum, 0);
		cpssDxChTrunkHashDesignatedTableModeSet(devNum, 0);/*multi-destination packet use first index of designated table*/

		for(portNum = 0; portNum < 24; portNum++)
		{
			cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, modNum);
			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(devNum, portNum, 1);
			cpssDxChIpRouterSourceIdSet(devNum, 0 , modNum);
			cpssDxChIpRouterSourceIdSet(devNum, 1 , modNum);
			CPSS_PORTS_BMP_PORT_SET_MAC(&cscdPortBmp, portNum);
		}
		for(i = 0; i < 6; i++)
		{
			GT_BOOL cpuEnable;

			if(modNum == i)
			{
			    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_TRUE, &cscdPortBmp);
			}
			else
			{
			    cpssDxChBrgSrcIdGroupEntrySet(devNum, i, GT_FALSE, &cscdPortBmp);
			}
		}
		for(portNum = 24; portNum < 28; portNum++)
		{
			ret = cpssDxChPortXGmiiModeSet(devNum, portNum, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
			ret = cpssDxChCscdPortTypeSet(devNum, portNum, 2, 1);/*1 = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E*/
			ret = cpssDxChPortIpgBaseSet(devNum, portNum, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
			ret = cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 4);
			ret = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
			ret = cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, 0);
			cpssDxChPortEnableSet(devNum, portNum, 0);
			cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, 0);
    		cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
			ax63ge48_trunk_pcl_deinit(devNum, portNum);
		}
		if(instance_num == 1)
		{
			ax63ge48_trunk_port_config(devNum, AX24_RIGHT_SLOT_TRUNK, 24, 26, 0, 0, 0, 0, 0, 0);
			
			ax63ge48_trunk_port_config(devNum, AX24_LEFT_SLOT_TRUNK, 25, 27, 0, 0, 0, 0, 0, 0);
			
			for(i = 0; i < 6; i++)
			{
    			CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				int right_slot = 0;
				if(i/2 == modNum/2)
				{
					continue;
				}
				if(i/2 < modNum/2)
				{
					if(modNum/2 - i/2 == 1)
					{
						right_slot = 0;
					}
					else
					{
						right_slot = 1;
					}
				}
				else
				{
					if(i/2 - modNum/2 == 1)
					{
						right_slot = 1;
					}
					else
					{
						right_slot = 0;
					}
				}
				if(right_slot)/*右侧槽位*/
				{
					npd_syslog_dbg("%s %d: Packets send to right slot by trunk %d\r\n", __func__, __LINE__, AX24_RIGHT_SLOT_TRUNK);
    			    cascadeLinkPtr.linkNum = AX24_RIGHT_SLOT_TRUNK;
				    cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    ret = cpssDxChCscdDevMapTableSet(devNum, i, 0, &cascadeLinkPtr, 1);
				}
				else/*左侧槽位*/
				{
					npd_syslog_dbg("%s %d: Packets send to left slot by trunk %d\r\n", __func__, __LINE__, AX24_LEFT_SLOT_TRUNK);
    			    cascadeLinkPtr.linkNum = AX24_LEFT_SLOT_TRUNK;
				    cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    ret = cpssDxChCscdDevMapTableSet(devNum, i, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		else if(instance_num == 2)
		{
			int targetDevNum = 0;
			int linkStartNum = 127;
			CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			
			if(devNum == 0)
			{
			    ax63ge48_trunk_port_config(devNum, linkStartNum, 24, 25, 0, 0, 0, 0, 0, 0);
				targetDevNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                   SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);/*板上另一单元的modid*/
			}
			else
			{
			    ax63ge48_trunk_port_config(devNum, linkStartNum, 26, 27, 0, 0, 0, 0, 0, 0);
				targetDevNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
                                   SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);/*板上另一单元的modid*/
				
			}
			cascadeLinkPtr.linkNum = linkStartNum;
			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
			ret = cpssDxChCscdDevMapTableSet(devNum, targetDevNum, 0, &cascadeLinkPtr, 1);
		}
	}
	/* 清除所有的原先学习到的静态FDB表项 */
	for(devNum = 0; devNum < instance_num; devNum++)
	{
		ret = nam_fdb_table_delete_dynamic_all(devNum);
	}
    return 0;  
}

long as_series_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0;
	unsigned char myDevNum = 0, myModNum = 0, myPortNum = 0;
	unsigned char leftModNum = 0, leftPortNum = 0;
	unsigned char rightModNum = 0, rightPortNum = 0;
	unsigned char targetDevNum = 0;
	int mySlot = 0, leftSlot = 0, rightSlot;
	int myType = 0, leftType = 0, rightType = 0;
	int instance_num = nam_asic_get_instance_num();
	
	as_series_linecard_local_conn_init(product_type);
	
	mySlot = SYS_LOCAL_MODULE_SLOT_INDEX;
	myType = SYS_LOCAL_MODULE_TYPE;
	myPortNum = SYS_MODULE_PORT_NUM(myType);
	if(SYS_MODULE_RUNNINGSTATE(insert_slotid) == RMT_BOARD_NOEXIST)
	{
		npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, insert_slotid);
		return 0;
	}
	
	if(insert_slotid > mySlot)
	{
		if(insert_slotid - mySlot == 1)
		{
			rightSlot = insert_slotid;
			rightType = insert_board_type;
	        rightPortNum = SYS_MODULE_PORT_NUM(rightType);
		}
		else
		{
			leftSlot = insert_slotid;
			leftType = insert_board_type;
	        leftPortNum = SYS_MODULE_PORT_NUM(leftType);
		}
	}
	else
	{
		if(mySlot - insert_slotid == 1)
		{
			leftSlot = insert_slotid;
			leftType = insert_board_type;
	        leftPortNum = SYS_MODULE_PORT_NUM(leftType);
		}
		else
		{
			rightSlot = insert_slotid;
			rightType = insert_board_type;
	        rightPortNum = SYS_MODULE_PORT_NUM(rightType);
		}
	}
	/*建立直接相连的级联端口拓扑*/
	for(i = 0; i < 3; i++)
	{
		if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
		{
			continue;
		}
		if(i == insert_slotid)
		{
			continue;
		}
		
		if(leftType != 0)
		{
			rightSlot = i;
			rightType = MODULE_TYPE_ON_SLOT_INDEX(rightSlot);
			if(rightType == 0)
			{
				rightPortNum = 0;
			}
			else
			{
		        if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		        {
					rightPortNum = 0;
		        }
		        else
		        {
	                rightPortNum = SYS_MODULE_PORT_NUM(rightType);
		        }
			}
		}
		else if(rightType != 0)
		{
			leftSlot = i;
			leftType = MODULE_TYPE_ON_SLOT_INDEX(leftSlot);
			if(leftType == 0)
			{
				leftPortNum = 0;
			}
			else
			{
		        if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		        {
					leftPortNum = 0;
		        }
		        else
		        {
	                leftPortNum = SYS_MODULE_PORT_NUM(leftType);
		        }
			}
		}
	}
	
	if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 24-port board <-> NONE\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	if(myPortNum == 24 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 24-port board <-> 24-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 48 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 24-port board <-> NONE\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by cscd ports 25 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
			targetModnum = UNIT_2_MODULE(leftType, leftSlot, 1, 0);
			ax63ge48_trunk_port_config(myDevNum, AX24_LEFT_SLOT_TRUNK, 25, 27, 25, TRUNK_PORT_PCL_BIDIRECTION, targetModnum, leftModNum, 0, 0);
		}
	}
	else if(myPortNum == 24 && leftPortNum == 0 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: NONE <-> 24-port board <-> 48-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by cscd ports 24 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
			targetModnum = UNIT_2_MODULE(rightType, rightSlot, 1, 0);
			ax63ge48_trunk_port_config(myDevNum, AX24_RIGHT_SLOT_TRUNK, 24, 26, 24, TRUNK_PORT_PCL_BIDIRECTION, targetModnum, rightModNum, 0, 0);
		}
	}
	if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> NONE\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		targetDevNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
    		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			if(myDevNum == 0)
			{
    		    cascadeLinkPtr.linkNum = 27;
	            cpssDxChPortEnableSet(myDevNum, 27, 1);
			}
			else
			{
    		    cascadeLinkPtr.linkNum = 25;
	            cpssDxChPortEnableSet(myDevNum, 25, 1);
			}
			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, 0, cascadeLinkPtr.linkNum);
			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetDevNum, 0, &cascadeLinkPtr, 1);
		}
		#else
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;

			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 27, 0, 0, 0, 0, 0);
				
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 24,27, leftModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 24 to target mod %d redirect to 27\r\n", __func__, __LINE__, leftModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 27,24, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 27 to another mod (%d) of the same slot redirect to 24\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0));
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 25, 0, 0, 0, 0, 0);
				
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 26,25, leftModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 26 to target mod %d redirect to 25\r\n", __func__, __LINE__, leftModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 25,26, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 25 to another mod (%d) of the same slot redirect to 26\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0));
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				cascadeLinkPtr.linkNum = 127;
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 24 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 25 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 0)
					{
			            cpssDxChBrgSrcIdGroupPortDelete(myDevNum, targetModnum, 24);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slots) Cscd port 24\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slots) Cscd port 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
			}
		}
		#endif
	}
	if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		targetDevNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
    		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of right slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			if(myDevNum == 0)
			{
    		    cascadeLinkPtr.linkNum = 26;
	            cpssDxChPortEnableSet(myDevNum, 26, 1);
			}
			else
			{
    		    cascadeLinkPtr.linkNum = 24;
	            cpssDxChPortEnableSet(myDevNum, 24, 1);
			}
			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, 0, cascadeLinkPtr.linkNum);
			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetDevNum, 0, &cascadeLinkPtr, 1);
		}
		#else
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			
			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 26, 0, 0, 0, 0, 0);
				
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 25,26, rightModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 25 to target mod %d redirect to 26\r\n", __func__, __LINE__, rightModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 26,25, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 26 to another mod (%d) of the same slot redirect to 26\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0));
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 24, 0, 0, 0, 0, 0);
				
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 27,24, rightModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 27 to target mod %d redirect to 24\r\n", __func__, __LINE__, rightModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 24,27, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 24 to another mod (%d) of the same slot redirect to 27\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0));
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 25 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 24 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 0)
					{
			            cpssDxChBrgSrcIdGroupPortDelete(myDevNum, targetModnum, 25);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slots) Cscd port 25\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slots) Cscd port 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
			}
		}
		#endif
	}
	else if(myPortNum == 48 && leftPortNum == 48 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 48-port board<->NONE\r\n", __func__, __LINE__);

		for(targetDevNum = 0; targetDevNum < 2; targetDevNum++)
		{
			unsigned char targetModnum = UNIT_2_MODULE(leftType, leftSlot, targetDevNum, 0);

    		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(myDevNum == targetDevNum)/*同一平面走端口*/
				{
        			if(myDevNum == 0)
        			{
            		    cascadeLinkPtr.linkNum = 27;
	                    cpssDxChPortEnableSet(myDevNum, 27, 1);
        			}
        			else
        			{
            		    cascadeLinkPtr.linkNum = 25;
	                    cpssDxChPortEnableSet(myDevNum, 25, 1);
        			}
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, targetDevNum, cascadeLinkPtr.linkNum);
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
				}
				else/*不同一平面走本板trunk*/
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, targetDevNum, cascadeLinkPtr.linkNum);
				}
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(myDevNum == MODULE_2_UNIT(leftType, targetModnum))/*同一平面的单元*/
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of left slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (lef slot) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 48-port board\r\n", __func__, __LINE__);

		for(targetDevNum = 0; targetDevNum < 2; targetDevNum++)
		{
			unsigned char targetModnum = UNIT_2_MODULE(rightType, rightSlot, targetDevNum, 0);

    		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(myDevNum == targetDevNum)/*同一平面走端口*/
				{
        			if(myDevNum == 1)
        			{
            		    cascadeLinkPtr.linkNum = 24;
	                    cpssDxChPortEnableSet(myDevNum, 24, 1);
        			}
        			else
        			{
            		    cascadeLinkPtr.linkNum = 26;
	                    cpssDxChPortEnableSet(myDevNum, 26, 1);
        			}
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, rightSlot, targetDevNum, cascadeLinkPtr.linkNum);
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
				}
				else/*不同一平面走本板trunk*/
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, rightSlot, targetDevNum, cascadeLinkPtr.linkNum);
				}
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*跟上一种情况有很大的相同*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(myDevNum == MODULE_2_UNIT(rightType, targetModnum))/*同一平面的单元*/
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of left slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (lef slot) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 24-port board <-> 24-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member 126*/
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slot) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 24-port board <-> 48-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member 126*/
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(rightSlot == MODULE_2_SLOT_INDEX(targetModnum))
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slot) sent by left trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member 126*/
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slot) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 48 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 24-port board <-> 24-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member 126*/
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(leftSlot == MODULE_2_SLOT_INDEX(targetModnum))
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slot) sent by right trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member 127*/
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slot) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 48 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 24-port board <-> 48-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member 126*/
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				#ifndef _CSCD_PORT_NO_TRUNK_
				else if(leftSlot == MODULE_2_SLOT_INDEX(targetModnum))
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slot) sent by right trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*right trunk member 127*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*right trunk member 127*/
				}
				else if(rightSlot == MODULE_2_SLOT_INDEX(targetModnum))
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slot) sent by left trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*left trunk member 126*/
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*left trunk member 126*/
				}
				#endif
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slot) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;

			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 126, 26, 27, 0, 0, 0, 0, 0, 0);
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 126, 24, 25, 0, 0, 0, 0, 0, 0);
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 126;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetDevNum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(targetModnum == UNIT_2_MODULE(leftType, leftSlot, 0, 0))
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slot) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		
	}
	else if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> 48-port board\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
        	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
				if(MODULE_2_SLOT_INDEX(targetModnum) == leftSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 27;
	                    cpssDxChPortEnableSet(myDevNum, 27, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 25;
	                    cpssDxChPortEnableSet(myDevNum, 25, 1);
					}
				}
				else if(MODULE_2_SLOT_INDEX(targetModnum) == rightSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 26;
	                    cpssDxChPortEnableSet(myDevNum, 26, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 24;
	                    cpssDxChPortEnableSet(myDevNum, 24, 1);
					}
				}
				else
				{
					continue;
				}
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by port %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by inner trunk members and one port of outer trunk\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#else
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			if(myDevNum == 0)
			{
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);/*来自右侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 26, 27, 27, TRUNK_PORT_PCL_PORTA_2_PORTB, 0, leftModNum, 0, 1);
			}
			else
			{
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);/*来自右侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 24, 25, 25, TRUNK_PORT_PCL_PORTA_2_PORTB, 0, leftModNum, 0, 1);
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 126;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by inner trunk members and one port of outer trunk\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#endif
	}
	else if(myPortNum == 48 && leftPortNum == 48 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
        	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
				if(MODULE_2_SLOT_INDEX(targetModnum) == leftSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 27;
	                    cpssDxChPortEnableSet(myDevNum, 27, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 25;
	                    cpssDxChPortEnableSet(myDevNum, 25, 1);
					}
				}
				else if(MODULE_2_SLOT_INDEX(targetModnum) == rightSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 26;
	                    cpssDxChPortEnableSet(myDevNum, 26, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 24;
	                    cpssDxChPortEnableSet(myDevNum, 24, 1);
					}
				}
				else
				{
					continue;
				}
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by port %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd ports\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#else
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
        	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;

			if(myDevNum == 0)
			{
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);/*来自左侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 27, 26, 26, TRUNK_PORT_PCL_PORTA_2_PORTB, 0, rightModNum, 0, 1);
			}
			else
			{
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);/*来自左侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 25, 24, 24, TRUNK_PORT_PCL_PORTA_2_PORTB, 0, rightModNum, 0, 1);
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 126;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd ports\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#endif
	}
	else if(myPortNum == 48 && leftPortNum == 48 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 48-port board <-> 48-port board\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
        	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
				if(MODULE_2_SLOT_INDEX(targetModnum) == leftSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 27;
	                    cpssDxChPortEnableSet(myDevNum, 27, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 25;
	                    cpssDxChPortEnableSet(myDevNum, 25, 1);
					}
				}
				else if(MODULE_2_SLOT_INDEX(targetModnum) == rightSlot)
				{
					if(myDevNum == 0)
					{
					    cascadeLinkPtr.linkNum = 26;
	                    cpssDxChPortEnableSet(myDevNum, 26, 1);
					}
					else
					{
					    cascadeLinkPtr.linkNum = 24;
	                    cpssDxChPortEnableSet(myDevNum, 24, 1);
					}
				}
				else
				{
					continue;
				}
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by port %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by all cscd ports\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#else
		/*single-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
        	CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			if(myDevNum == 0)
			{
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, myDevNum, 0);/*来自右侧48口板的报文，从trunk的另外一个端口出*/
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, myDevNum, 1);/*来自左侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 26, 27, 27, TRUNK_PORT_PCL_BIDIRECTION, rightModNum, leftModNum, 0, 1);
			}
			else
			{
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, myDevNum, 0);/*来自右侧48口板的报文，从trunk的另外一个端口出*/
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, myDevNum, 1);/*来自左侧48口板的报文，从trunk的另外一个端口出*/
			    ax63ge48_trunk_port_config(myDevNum, 126, 24, 25, 24, TRUNK_PORT_PCL_BIDIRECTION, rightModNum, leftModNum, 0, 1);
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 126;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by inner trunk and port 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by inner trunk and port 24\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					if(myDevNum == 0)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) sent by port 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) sent by port 24\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (other slots) sent by inner trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				}
			}
		}
		#endif
	}
	return 0;
}

long as_series_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0;
	unsigned char myDevNum = 0, myModNum = 0, myPortNum = 0;
	unsigned char leftModNum = 0, leftPortNum = 0;
	unsigned char rightModNum = 0, rightPortNum = 0;
	unsigned char targetDevNum = 0;
	int mySlot = 0, leftSlot = 0, rightSlot;
	int myType = 0, leftType = 0, rightType = 0;
	int instance_num = nam_asic_get_instance_num();
	
	as_series_linecard_local_conn_init(product_type);
	
	mySlot = SYS_LOCAL_MODULE_SLOT_INDEX;
	myType = SYS_LOCAL_MODULE_TYPE;
	myPortNum = SYS_MODULE_PORT_NUM(myType);
	
	if(delete_slotid > mySlot)
	{
		if(delete_slotid - mySlot == 1)
		{
			rightSlot = delete_slotid;
			rightType = 0;
	        rightPortNum = 0;
		}
		else
		{
			leftSlot = delete_slotid;
			leftType = 0;
	        leftPortNum = 0;
		}
	}
	else
	{
		if(mySlot - delete_slotid == 1)
		{
			leftSlot = delete_slotid;
			leftType = 0;
	        leftPortNum = 0;
		}
		else
		{
			rightSlot = delete_slotid;
			rightType = 0;
	        rightPortNum = 0;
		}
	}
	/*建立直接相连的级联端口拓扑*/
	for(i = 0; i < 3; i++)
	{
		if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
		{
			continue;
		}
		if(i == delete_slotid)
		{
			continue;
		}
		if(i > mySlot)
		{
    		if(i - mySlot == 1)
    		{
    			rightSlot = i;
    			rightType = MODULE_TYPE_ON_SLOT_INDEX(rightSlot);
				if(rightType == 0)
				{
					rightPortNum = 0;
				}
				else
				{
					if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		            {
    	                rightPortNum = 0;
					}
					else
					{
    	                rightPortNum = SYS_MODULE_PORT_NUM(rightType);
					}
				}
    		}
    		else
    		{
    			leftSlot = i;
    			leftType = MODULE_TYPE_ON_SLOT_INDEX(leftSlot);
				if(leftType == 0)
				{
					leftPortNum = 0;
				}
				else
				{
					if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		            {
    	                leftPortNum = 0;
					}
					else
					{
    	                leftPortNum = SYS_MODULE_PORT_NUM(leftType);
					}
				}
    		}
		}
		else
		{
    		if(mySlot - i == 1)
    		{
    			leftSlot = i;
    			leftType = MODULE_TYPE_ON_SLOT_INDEX(leftSlot);
				if(leftType == 0)
				{
					leftPortNum = 0;
				}
				else
				{
					if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		            {
    	                leftPortNum = 0;
					}
					else
					{
    	                leftPortNum = SYS_MODULE_PORT_NUM(leftType);
					}
				}
    		}
    		else
    		{
    			rightSlot = i;
    			rightType = MODULE_TYPE_ON_SLOT_INDEX(rightSlot);
				if(rightType == 0)
				{
					rightPortNum = 0;
				}
				else
				{
					if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
		            {
    	                rightPortNum = 0;
					}
					else
					{
    	                rightPortNum = SYS_MODULE_PORT_NUM(rightType);
					}
				}
    		}
		}
	}
	npd_syslog_dbg("%s %d: board removing.\r\n", __func__, __LINE__);
	if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 24-port board <-> NONE\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	if(myPortNum == 24 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 24-port board <-> 24-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 24 && leftPortNum == 48 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 24-port board <-> NONE\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by cscd ports 25 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
			targetModnum = UNIT_2_MODULE(leftType, leftSlot, 1, 0);
			ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 25, TRUNK_PORT_PCL_BIDIRECTION, targetModnum, leftModNum, 0, 0);
		}
	}
	else if(myPortNum == 24 && leftPortNum == 0 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: NONE <-> 24-port board <-> 48-port board\r\n", __func__, __LINE__);
		myDevNum = 0;
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by cscd ports 24 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
			targetModnum = UNIT_2_MODULE(rightType, rightSlot, 1, 0);
			ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 24, TRUNK_PORT_PCL_BIDIRECTION, targetModnum, rightModNum, 0, 0);
		}
	}
	else if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> NONE\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		targetDevNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
    		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			if(myDevNum == 0)
			{
    		    cascadeLinkPtr.linkNum = 27;
	            cpssDxChPortEnableSet(myDevNum, 27, 1);
			}
			else
			{
    		    cascadeLinkPtr.linkNum = 25;
	            cpssDxChPortEnableSet(myDevNum, 25, 1);
			}
			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, 0, cascadeLinkPtr.linkNum);
			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetDevNum, 0, &cascadeLinkPtr, 1);
		}
		#else
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;

			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 27, 0, 0, 0, 0, 0);
				
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 24,27, leftModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 24 to target mod %d redirect to 27\r\n", __func__, __LINE__, leftModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 27,24, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 27 to another mod (%d) of the same slot redirect to 24\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0));
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 25, 0, 0, 0, 0, 0);
				
				leftModNum = UNIT_2_MODULE(leftType, leftSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 26,25, leftModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 26 to target mod %d redirect to 25\r\n", __func__, __LINE__, leftModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 25,26, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 25 to another mod (%d) of the same slot redirect to 26\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0));
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 24 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 25 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 0)
					{
			            cpssDxChBrgSrcIdGroupPortDelete(myDevNum, targetModnum, 24);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slots) Cscd port 24\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (left slots) Cscd port 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
			}
		}
		#endif
	}
	else if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
		#ifdef _CSCD_PORT_NO_TRUNK_
		targetDevNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
    		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of right slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
			if(myDevNum == 0)
			{
    		    cascadeLinkPtr.linkNum = 26;
	            cpssDxChPortEnableSet(myDevNum, 26, 1);
			}
			else
			{
    		    cascadeLinkPtr.linkNum = 24;
	            cpssDxChPortEnableSet(myDevNum, 24, 1);
			}
			npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, 0, cascadeLinkPtr.linkNum);
			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetDevNum, 0, &cascadeLinkPtr, 1);
		}
		#else
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			
			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 26, 0, 0, 0, 0, 0);
				
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 25,26, rightModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 25 to target mod %d redirect to 26\r\n", __func__, __LINE__, rightModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 26,25, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 26 to another mod (%d) of the same slot redirect to 26\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0));
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 24, 0, 0, 0, 0, 0);
				
				rightModNum = UNIT_2_MODULE(rightType, rightSlot, 0, 0);
				ax63ge48_trunk_pcl_config(myDevNum, 27,24, rightModNum, 0, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 27 to target mod %d redirect to 24\r\n", __func__, __LINE__, rightModNum);
				ax63ge48_trunk_pcl_config(myDevNum, 24,27, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0), 1, 0);
				npd_syslog_dbg("%s %d: PCL config: Packets from cscd port 24 to another mod (%d) of the same slot redirect to 27\r\n", __func__, __LINE__, UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0));
			}
			
    		for(targetModnum = 0; targetModnum < 6; targetModnum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(MODULE_2_SLOT_INDEX(targetModnum) != SYS_LOCAL_MODULE_SLOT_INDEX)
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to target mod %d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum, cascadeLinkPtr.linkNum);
        		    ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
				}
    		}
		}
		/*multi-destination packets*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 0)
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 25 and 26\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
					    cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
				        npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by Cscd ports 24 and 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) discard\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					if(myDevNum == 0)
					{
			            cpssDxChBrgSrcIdGroupPortDelete(myDevNum, targetModnum, 25);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slots) Cscd port 25\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (right slots) Cscd port 27\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
					}
				}
			}
		}
		#endif
	}
	else if(myPortNum == 48 && leftPortNum == 48 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 48-port board<->NONE\r\n", __func__, __LINE__);

		for(targetDevNum = 0; targetDevNum < 2; targetDevNum++)
		{
			unsigned char targetModnum = UNIT_2_MODULE(leftType, leftSlot, targetDevNum, 0);

    		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(myDevNum == targetDevNum)/*同一平面走端口*/
				{
        			if(myDevNum == 0)
        			{
            		    cascadeLinkPtr.linkNum = 27;
	                    cpssDxChPortEnableSet(myDevNum, 27, 1);
        			}
        			else
        			{
            		    cascadeLinkPtr.linkNum = 25;
	                    cpssDxChPortEnableSet(myDevNum, 25, 1);
        			}
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, targetDevNum, cascadeLinkPtr.linkNum);
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
				}
				else/*不同一平面走本板trunk*/
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, leftSlot, targetDevNum, cascadeLinkPtr.linkNum);
				}
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(myDevNum == MODULE_2_UNIT(leftType, targetModnum))/*同一平面的单元*/
				{
					if(myDevNum == 1)
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
				        cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of left slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (lef slot) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	else if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 48)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 48-port board\r\n", __func__, __LINE__);

		for(targetDevNum = 0; targetDevNum < 2; targetDevNum++)
		{
			unsigned char targetModnum = UNIT_2_MODULE(rightType, rightSlot, targetDevNum, 0);

    		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
    		{
        		CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
				if(myDevNum == targetDevNum)/*同一平面走端口*/
				{
        			if(myDevNum == 1)
        			{
            		    cascadeLinkPtr.linkNum = 24;
	                    cpssDxChPortEnableSet(myDevNum, 24, 1);
        			}
        			else
        			{
            		    cascadeLinkPtr.linkNum = 26;
	                    cpssDxChPortEnableSet(myDevNum, 26, 1);
        			}
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd port %d\r\n", __func__, __LINE__, mySlot, myDevNum, rightSlot, targetDevNum, cascadeLinkPtr.linkNum);
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
				}
				else/*不同一平面走本板trunk*/
				{
					cascadeLinkPtr.linkNum = 127;
    				cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			    npd_syslog_dbg("%s %d: Packets from slot:unit %d:%d to %d:%d by cscd trunk %d\r\n", __func__, __LINE__, mySlot, myDevNum, rightSlot, targetDevNum, cascadeLinkPtr.linkNum);
				}
        		ret = cpssDxChCscdDevMapTableSet(myDevNum, targetModnum, 0, &cascadeLinkPtr, 1);
    		}
		}
		/*跟上一种情况有很大的相同*/
		for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;
			as_series_linecard_srcid_init(myDevNum);
			for(targetModnum = 0; targetModnum < 6; targetModnum++)
			{
				if(targetModnum == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, myDevNum, 0))
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);
					}
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (my-self)sent by both cscd port and cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(MOD_ID_TO_SLOT_INDEX(targetModnum) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (same slot with me) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else if(myDevNum == MODULE_2_UNIT(rightType, targetModnum))/*同一平面的单元*/
				{
					if(myDevNum == 1)
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 26);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 27);/*inner trunk member*/
					}
					else
					{
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 24);/*inner trunk member*/
			            cpssDxChBrgSrcIdGroupPortAdd(myDevNum, targetModnum, 25);/*inner trunk member*/
					}
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (direct connected unit of left slot) sent by cscd trunk members\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
				else
				{
					npd_syslog_dbg("%s %d: My slot:unit %d:%d. From SRC_ID %d (lef slot) dropped\r\n", __func__, __LINE__, mySlot, myDevNum, targetModnum);
				}
			}
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

