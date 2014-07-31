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
extern int nam_fdb_table_delete_dynamic_all(int unit);
#define TRUNK_PORT_PCL_PORTA_2_PORTB 1
#define TRUNK_PORT_PCL_PORTB_2_PORTA 2
#define TRUNK_PORT_PCL_BIDIRECTION   3
#define START_MOD_ID 0
#define CSCD_PORT_PCL_ID 128
#define AX24_RIGHT_SLOT_TRUNK 127
#define AX24_LEFT_SLOT_TRUNK 126
#define SDK_DIFF_TRUNK	121

CPSS_PORTS_BMP_STC as_st_bmp[2];

int as6603_cscd_need_pcl = 0;

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

int ax63ge48_trunk_port_sdkdiff_config(
    GT_TRUNK_ID trunkId, 
    GT_U8 devNumA, 
    GT_U8 portNumA, 
    GT_U8 devNumB, 
    GT_U8 portNumB, 
    GT_U8 designatedPort, 
    GT_U8 pclMod, 
    GT_U8 portAConnMod, 
    GT_U8 portBConnMod, 
    int rule_level, 
    int slot_or_unit
    )
{
	int ret = 0;
	GT_U8 myModNumA = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, devNumA, 0);
	GT_U8 myModNumB = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, devNumB, 0);
	CPSS_TRUNK_MEMBER_STC trunk_member[2];
	CPSS_PORTS_BMP_STC          trunkDesignatedBmp;
	int entry_index = 0;
	trunk_member[0].device = devNumA;
	trunk_member[0].port = portNumA;
	trunk_member[1].device = devNumB;
	trunk_member[1].port = portNumB;
	ret = cpssDxChTrunkMembersSet(devNumA, trunkId, 2, trunk_member, 0, NULL);
	if(ret != 0)
	{
	    npd_syslog_dbg("%s %d: Add Cscd ports %d:%d and %d:%d to trunk %d failed.\r\n", __func__, __LINE__, 
		             trunk_member[0].device, 
		             trunk_member[0].port, 
		             trunk_member[1].device, 
		             trunk_member[1].port, trunkId);
		return ret;
	}
	ret = cpssDxChTrunkMembersSet(devNumB, trunkId, 2, trunk_member, 0, NULL);
	if(ret != 0)
	{
	    npd_syslog_dbg("%s %d: Add Cscd ports %d:%d and %d:%d to trunk %d failed.\r\n", __func__, __LINE__, 
		             trunk_member[0].device, 
		             trunk_member[0].port, 
		             trunk_member[1].device, 
		             trunk_member[1].port, trunkId);
		return ret;
	}
	cpssDxChPortEnableSet(devNumA, portNumA, 1);
	cpssDxChPortEnableSet(devNumB, portNumB, 1);
	npd_syslog_dbg("%s %d: Add Cscd ports %d:%d and %d:%d to trunk %d\r\n", __func__, __LINE__, 
		             trunk_member[0].device, 
		             trunk_member[0].port, 
		             trunk_member[1].device, 
		             trunk_member[1].port, trunkId);
	if(designatedPort == portNumA || designatedPort == portNumB)
	{
	    for(entry_index = 0; entry_index < 8; entry_index++)
    	{
    		cpssDxChTrunkDesignatedPortsEntryGet(devNumA, entry_index, &trunkDesignatedBmp);
    		if(entry_index%2 == 0)
    		{
    			if(designatedPort == portNumA)
    			{
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumA);
    			}
    		}
    		else
    		{
    			if(designatedPort == portNumA)
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumA);
    			}
    		}
    		cpssDxChTrunkDesignatedPortsEntrySet(devNumA, entry_index, &trunkDesignatedBmp);
    	}
	    for(entry_index = 0; entry_index < 8; entry_index++)
    	{
    		cpssDxChTrunkDesignatedPortsEntryGet(devNumB, entry_index, &trunkDesignatedBmp);
    		if(entry_index%2 == 0)
    		{
    			if(designatedPort == portNumB)
    			{
    			    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkDesignatedBmp, portNumB);
    			}
    		}
    		else
    		{
    			if(designatedPort == portNumB)
    			{
    			    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trunkDesignatedBmp, portNumB);
    			}
    		}
    		cpssDxChTrunkDesignatedPortsEntrySet(devNumB, entry_index, &trunkDesignatedBmp);
    	}	    npd_syslog_dbg("%s %d: Set port %d as designated port for multi-destination packets\r\n", __func__, __LINE__, 
		             designatedPort);
        if(designatedPort == portNumA)
		    cpssDxChBrgSrcIdGroupPortAdd(devNumA, myModNumA, designatedPort);
        if(designatedPort == portNumB)
		    cpssDxChBrgSrcIdGroupPortAdd(devNumB, myModNumB, designatedPort);
	}
	else
	{
		cpssDxChBrgSrcIdGroupPortAdd(devNumA, myModNumA, portNumA);
		cpssDxChBrgSrcIdGroupPortAdd(devNumB, myModNumB, portNumB);
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

        devTableBmp = 0x3F;
		cpssDxChBrgFdbDeviceTableSet(devNum, devTableBmp);
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
			CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[devNum], portNum);
			ret = cpssDxChPortXGmiiModeSet(devNum, portNum, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
			ret = cpssDxChCscdPortTypeSet(devNum, portNum, 2, 1);/*1 = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E*/
			ret = cpssDxChPortIpgBaseSet(devNum, portNum, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
			ret = cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 4);
			ret = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
			ret = cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, 0);
			cpssDxChPortEnableSet(devNum, portNum, 0);
			cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, (as6603_cscd_need_pcl?0:1));
    		cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
			ax63ge48_trunk_pcl_deinit(devNum, portNum);
            cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
		}
		if(instance_num == 2)
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

	nbm_open_laser();

    return 0;  
}


long as_series_linecard_system_sdkdiff_conn_init(
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
	
	mySlot = SYS_LOCAL_MODULE_SLOT_INDEX;
	myType = SYS_LOCAL_MODULE_TYPE;
	myPortNum = SYS_MODULE_PORT_NUM(myType);

    for(i = 0; i < PPAL_PLANE_PORT_COUNT(myType); i++)
    {
        int conn_plane_slot;
        int conn_plane_port;
        int conn_unit;
        int conn_unit_port;
        
		if( -1 == PPAL_PLANE_2_UNIT(myType, i))
			continue;
		
		conn_plane_slot = SLOT_PORT_PEER_SLOT(mySlot, i);
		if(conn_plane_slot == -1)
			continue;

        conn_unit = PPAL_PLANE_2_UNIT(myType, i);
        conn_unit_port = PPAL_PLANE_2_PORT(myType, i);
        if(-1 == conn_unit_port)
            continue;
        if(SYS_MODULE_SDK_DIFFERENT(myType, MODULE_TYPE_ON_SLOT_INDEX(conn_plane_slot)))
        {
			ret = cpssDxChPortXGmiiModeSet(conn_unit, conn_unit_port, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
			ret = cpssDxChCscdPortTypeSet(conn_unit, conn_unit_port, 2, 2);/*1 = CPSS_CSCD_PORT_NETWORK_E*/
			ret = cpssDxChPortIpgBaseSet(conn_unit, conn_unit_port, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
			ret = cpssDxChPortPreambleLengthSet(conn_unit, conn_unit_port, 2, 4);
			ret = cpssDxChNstPortEgressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
			cpssDxChPortEnableSet(conn_unit, conn_unit_port, 0);
			cpssDxChCscdPortBridgeBypassEnableSet(conn_unit, conn_unit_port, 0);
    		cpssDxChNstPortIngressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
        }
    }

    
	/*建立直接相连的级联端口拓扑*/
	for(i = 0; i < 3; i++)
	{
		if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
		{
			continue;
		}
		if(1 == i - SYS_LOCAL_MODULE_SLOT_INDEX )
		{
            rightSlot = i;
            rightType = MODULE_TYPE_ON_SLOT_INDEX(rightSlot);
            if(PPAL_BOARD_TYPE_NONE == rightType)
                rightPortNum = 0;
            else if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
                rightPortNum = 0;
            else
                rightPortNum = SYS_MODULE_PORT_NUM(rightType);
                
			continue;
		}

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
	

	if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> NONE\r\n", __func__, __LINE__);
	    ax63ge48_trunk_port_sdkdiff_config(127, 0, 27, 1, 25, 0, 0, 0, 0, 0, 0);

	}
	if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
	    ax63ge48_trunk_port_sdkdiff_config(127, 0, 26, 1, 24, 0, 0, 0, 0, 0, 0);
	}

	if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 24)
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
	if(myPortNum == 24 && leftPortNum == 24 && rightPortNum == 48)
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
	
	if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
		/*single-destination packets*/
        if(leftSlot == insert_slotid)
        {
	        ax63ge48_trunk_port_sdkdiff_config(127, 0, 27, 1, 25, 0, 0, 0, 0, 0, 0);
            if(SYS_MODULE_SDK_DIFFERENT(myType, MODULE_TYPE_ON_SLOT_INDEX(rightSlot)))
	            ax63ge48_trunk_port_sdkdiff_config(127, 0, 26, 1, 24, 0, 0, 0, 0, 0, 0);
                
        }
        if(rightSlot == insert_slotid)
        {
	        ax63ge48_trunk_port_sdkdiff_config(127, 0, 26, 1, 24, 0, 0, 0, 0, 0, 0);
            if(SYS_MODULE_SDK_DIFFERENT(myType, MODULE_TYPE_ON_SLOT_INDEX(leftSlot)))
	            ax63ge48_trunk_port_sdkdiff_config(127, 0, 27, 1, 25, 0, 0, 0, 0, 0, 0);
                
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
		if(leftSlot != insert_slotid)
		{
            return 0;
		}
 	    ax63ge48_trunk_port_sdkdiff_config(127, 0, 27, 1, 25, 0, 0, 0, 0, 0, 0);
       
        for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;

			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 27, 0, 0, 0, 0, 0);
				
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 25, 27, 25, 0, 0, 0, 0, 0);
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

	}
	else if(myPortNum == 48 && leftPortNum == 48 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: 48-port board <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
	    ax63ge48_trunk_port_sdkdiff_config(127, 0, 26, 1, 24, 0, 0, 0, 0, 0, 0);
        for(myDevNum = 0; myDevNum < instance_num; myDevNum++)
		{
			unsigned char targetModnum;

			if(myDevNum == 0)
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 26, 24, 26, 0, 0, 0, 0, 0);
				
			}
			else
			{
			    ax63ge48_trunk_port_config(myDevNum, 127, 24, 26, 24, 0, 0, 0, 0, 0);
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
	}

	return 0;
}

long as_series_linecard_system_sdkdiff_conn_deinit(
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
	
	mySlot = SYS_LOCAL_MODULE_SLOT_INDEX;
	myType = SYS_LOCAL_MODULE_TYPE;
	myPortNum = SYS_MODULE_PORT_NUM(myType);

    for(i = 0; i < PPAL_PLANE_PORT_COUNT(myType); i++)
    {
        int conn_plane_slot;
        int conn_plane_port;
        int conn_unit;
        int conn_unit_port;
        
		if( -1 == PPAL_PLANE_2_UNIT(myType, i))
			continue;
		
		conn_plane_slot = SLOT_PORT_PEER_SLOT(mySlot, i);
		if(conn_plane_slot == -1)
			continue;

        conn_unit = PPAL_PLANE_2_UNIT(myType, i);
        conn_unit_port = PPAL_PLANE_2_PORT(myType, i);
        if(-1 == conn_unit_port)
            continue;
        if(SYS_MODULE_SDK_DIFFERENT(myType, MODULE_TYPE_ON_SLOT_INDEX(conn_plane_slot)))
        {
			ret = cpssDxChPortXGmiiModeSet(conn_unit, conn_unit_port, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
			ret = cpssDxChCscdPortTypeSet(conn_unit, conn_unit_port, 2, 2);/*1 = CPSS_CSCD_PORT_NETWORK_E*/
			ret = cpssDxChPortIpgBaseSet(conn_unit, conn_unit_port, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
			ret = cpssDxChPortPreambleLengthSet(conn_unit, conn_unit_port, 2, 4);
			ret = cpssDxChNstPortEgressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
			cpssDxChPortEnableSet(conn_unit, conn_unit_port, 0);
			cpssDxChCscdPortBridgeBypassEnableSet(conn_unit, conn_unit_port, 0);
    		cpssDxChNstPortIngressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
        }
    }

    
	/*建立直接相连的级联端口拓扑*/
	for(i = 0; i < 3; i++)
	{
		if(i == SYS_LOCAL_MODULE_SLOT_INDEX)
		{
			continue;
		}
		if(1 == i - SYS_LOCAL_MODULE_SLOT_INDEX )
		{
            rightSlot = i;
            rightType = MODULE_TYPE_ON_SLOT_INDEX(rightSlot);
            if(PPAL_BOARD_TYPE_NONE == rightType)
                rightPortNum = 0;
            else if(SYS_MODULE_RUNNINGSTATE(i) == RMT_BOARD_NOEXIST)
                rightPortNum = 0;
            else
                rightPortNum = SYS_MODULE_PORT_NUM(rightType);
                
			continue;
		}

		leftSlot = i;
		leftType = MODULE_TYPE_ON_SLOT_INDEX(leftSlot);
		if(leftType == PPAL_BOARD_TYPE_NONE)
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
	

	if(myPortNum == 48 && leftPortNum == 24 && rightPortNum == 0)
	{
		npd_syslog_dbg("%s %d: 24-port board <-> 48-port board <-> NONE\r\n", __func__, __LINE__);
        if(SYS_MODULE_SDK_DIFFERENT(myType, leftType))
	        ax63ge48_trunk_port_sdkdiff_config(127, 0, 27, 1, 25, 0, 0, 0, 0, 0, 0);

	}
	if(myPortNum == 48 && leftPortNum == 0 && rightPortNum == 24)
	{
		npd_syslog_dbg("%s %d: NONE <-> 48-port board <-> 24-port board\r\n", __func__, __LINE__);
        if(SYS_MODULE_SDK_DIFFERENT(myType, rightType))
	        ax63ge48_trunk_port_sdkdiff_config(127, 0, 26, 1, 24, 0, 0, 0, 0, 0, 0);
	}


	return 0;
}


unsigned int as_series_linecard_trunk_ports_del
(	
	
	unsigned short trunkId,
	unsigned int modid,
	unsigned int modport
)
{
	unsigned long ret = 0;
    unsigned char devNum;
	 
	CPSS_TRUNK_MEMBER_STC memberPtr;
	memberPtr.device = modid;
	memberPtr.port = modport;

	for(devNum = 0; devNum < nam_asic_get_instance_num(); devNum++)
	{
		ret = cpssDxChTrunkMemberRemove(devNum, trunkId, &memberPtr);
		if (0 == ret) {
			ret = TRUNK_CONFIG_SUCCESS;
		}
		else { 
			ret = TRUNK_CONFIG_FAIL;
		}
	}

	return ret;
}


int as_series_linecard_vlan_entry_del(unsigned int line_dev0, unsigned int line_slot_port0, 
												unsigned int line_dev1, unsigned int line_slot_port1)
{
	int i, ret, write_flag = 0;
	unsigned long numOfPp;
	unsigned short vlanId;
	CPSS_PORTS_BMP_STC pbmp ;
	CPSS_PORTS_BMP_STC t_pbmp;
	CPSS_DXCH_BRG_VLAN_INFO_STC vlan_info;
	GT_BOOL valid;
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC port_tag_cmd ;
	unsigned int netif_index0, netif_index1;
	memset(&pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&t_pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&vlan_info, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
	memset(&port_tag_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

	numOfPp = nam_asic_get_instance_num();

	for (vlanId = 1; vlanId < 4096; vlanId++)
	{
		write_flag = 0;

		for (i = 0; i < numOfPp; i++)
		{
			ret = cpssDxChBrgVlanEntryRead(i, vlanId, &pbmp, &t_pbmp, &vlan_info, &valid, &port_tag_cmd);
			if (ret != 0 || !valid)
			{
				npd_syslog_dbg("%s: get vlan %d info error.\r\n", __func__, vlanId);
				break;
			}

			if (i == line_dev0)
			{
			    int vlan_mode = 0;
				/* set  ports info */
            	ret = npd_get_global_index_by_devport(line_dev0, line_slot_port0, &netif_index0);
            	if(ret != 0)
            	{
            	    break;
            	}
				ret = npd_check_eth_port_status(netif_index0);
				if(ret != -1)
				{
				    break;
				}
				CPSS_PORTS_BMP_PORT_CLEAR_MAC(&pbmp, line_slot_port0);
				port_tag_cmd.portsCmd[line_slot_port0] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
				
				write_flag = 1;
			}

			if (i == line_dev1)
			{
			    int vlan_mode = 0;
				/* set  ports info */
            	ret = npd_get_global_index_by_devport(line_dev1, line_slot_port1, &netif_index1);
            	if(ret != 0)
            	{
            	    break;
            	}
				ret = npd_check_eth_port_status(netif_index1);
				if(ret != -1)
				{
				    break;
				}
				CPSS_PORTS_BMP_PORT_CLEAR_MAC(&pbmp, line_slot_port1);
				port_tag_cmd.portsCmd[line_slot_port1] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
				
				write_flag = 1;
			}

			if (1 == write_flag)
			{
				ret = cpssDxChBrgVlanEntryWrite(i, vlanId, &pbmp, &t_pbmp, &vlan_info, &port_tag_cmd);		
				if(0 == ret)
				{
					npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Successful.\r\n");
				}
				else
			    {
					npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Failed.\r\n");
				}	
			}
		}
	}

	return 0;
}

int as_serise_linecard_vlan_entry_add(unsigned int line_dev0, unsigned int line_slot_port0, 
												unsigned int line_dev1, unsigned int line_slot_port1)
{
	int i, ret, write_flag = 0;
	unsigned long numOfPp;
	unsigned short vlanId;
	CPSS_PORTS_BMP_STC pbmp ;
	CPSS_PORTS_BMP_STC t_pbmp;
	CPSS_DXCH_BRG_VLAN_INFO_STC vlan_info;
	GT_BOOL valid;
	CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC port_tag_cmd ;
	unsigned int netif_index0, netif_index1;
	memset(&pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&t_pbmp, 0, sizeof(CPSS_PORTS_BMP_STC));
	memset(&vlan_info, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
	memset(&port_tag_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

	numOfPp = nam_asic_get_instance_num();

	for (vlanId = 1; vlanId < 4096; vlanId++)
	{
		write_flag = 0;

		for (i = 0; i < numOfPp; i++)
		{
			ret = cpssDxChBrgVlanEntryRead(i, vlanId, &pbmp, &t_pbmp, &vlan_info, &valid, &port_tag_cmd);
			if (ret != 0 || !valid)
			{
				npd_syslog_dbg("%s: get vlan %d info error.\r\n", __func__, vlanId);
				break;
			}

			if (i == line_dev0)
			{
				/* set  ports info */
				int vlan_mode = 0;
				ret = npd_get_global_index_by_devport(line_dev0, line_slot_port0, &netif_index0);
            	if(ret == 0)
            	{
    				ret = npd_check_eth_port_status(netif_index0);
    				if(ret != -1)
    				{
    				    break;
    				}
            	}
				CPSS_PORTS_BMP_PORT_SET_MAC(&pbmp, line_slot_port0);
				CPSS_PORTS_BMP_PORT_SET_MAC(&t_pbmp, line_slot_port0);
				port_tag_cmd.portsCmd[line_slot_port0] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
				
				write_flag = 1;
			}

			if (i == line_dev1)
			{
				/* set  ports info */
				int vlan_mode = 0;
				ret = npd_get_global_index_by_devport(line_dev1, line_slot_port1, &netif_index1);
            	if(ret == 0)
            	{
    				ret = npd_check_eth_port_status(netif_index1);
    				if(ret != -1)
    				{
    				    break;
    				}
            	}
				CPSS_PORTS_BMP_PORT_SET_MAC(&pbmp, line_slot_port1);
				CPSS_PORTS_BMP_PORT_SET_MAC(&t_pbmp, line_slot_port1);
				port_tag_cmd.portsCmd[line_slot_port1] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
				
				write_flag = 1;
			}

			if (1 == write_flag)
			{
				ret = cpssDxChBrgVlanEntryWrite(i, vlanId, &pbmp, &t_pbmp, &vlan_info, &port_tag_cmd);		
				if(0 == ret)
				{
					npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Successful.\r\n");
				}
				else
			    {
					npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Failed.\r\n");
				}	
			}
		}
	}
}

void as_series_linecard_nopp_system_conn_init(int myType, int mySlot, int insertSlot)
{
	int i;
	
	for(i = 0; i < PPAL_PLANE_PORT_COUNT(myType); i++)
    {
        int conn_plane_slot;
        int conn_plane_port;
        int conn_unit;
        int conn_unit_port;
        
		if(-1 == PPAL_PLANE_2_UNIT(myType, i))
			continue;
		
		conn_plane_slot = SLOT_PORT_PEER_SLOT(mySlot, i);
		if(-1 == conn_plane_slot)
			continue;

		if(conn_plane_slot != insertSlot)
			continue;

        conn_unit = PPAL_PLANE_2_UNIT(myType, i);
        conn_unit_port = PPAL_PLANE_2_PORT(myType, i);
        if(-1 == conn_unit_port)
            continue;

		cpssDxChPortEnableSet(conn_unit, conn_unit_port, 0);
		cpssDxChPortInterfaceModeSet(conn_unit, conn_unit_port, CPSS_PORT_INTERFACE_MODE_XGMII_E);

		cpssDxChPortXGmiiModeSet(conn_unit, conn_unit_port, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
		cpssDxChCscdPortTypeSet(conn_unit, conn_unit_port, 2, 2);/*1 = CPSS_CSCD_PORT_NETWORK_E*/
		cpssDxChPortIpgBaseSet(conn_unit, conn_unit_port, 1);/*CPSS_PORT_XG_FIXED_IPG_12_BYTES_E*/
		cpssDxChPortPreambleLengthSet(conn_unit, conn_unit_port, 2, 8);
		cpssDxChNstPortEgressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
		cpssDxChCscdPortBridgeBypassEnableSet(conn_unit, conn_unit_port, 0);
		cpssDxChNstPortIngressFrwFilterSet(conn_unit, conn_unit_port, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);

		cpssDxChPortSerdesPowerStatusSet(conn_unit, conn_unit_port, CPSS_PORT_DIRECTION_BOTH_E, 0xf, GT_TRUE);
		//cpssDxChPortXgLanesSwapEnableSet(conn_unit, conn_unit_port, GT_TRUE);

		//cpssDxChPortEnableSet(conn_unit, conn_unit_port, 1);

    }
}

enum
{
	NOPP_PORT,
	DSA_PORT,
	SDKDIFF_PORT
};

int as_series_board_vlan_entry_del(GT_U8 devNum, GT_U8 portNum)
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

	for (vlanId = 1; vlanId < 4095; vlanId++)
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

int as_series_board_vlan_entry_add(GT_U8 devNum, GT_U8 portNum)
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

	for (vlanId = 1; vlanId < 4095; vlanId++)
	{
		ret = cpssDxChBrgVlanEntryRead(devNum, vlanId, 
                             			&pbmp, &t_pbmp, 
                             			&vlan_info, &valid, 
                             			&port_tag_cmd);
		if (ret != 0 || !valid)
		{
			continue;
		}

		ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
    	if(ret == 0)
    	{
			ret = npd_check_eth_port_status(netif_index);
			if(ret != -1)
			{
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
		}
		else
	    {
			npd_syslog_dbg("cpssDxChBrgVlanEntryWrite Failed.\r\n");
		}	
	}

	return 0;
}

int as_series_board_trunk_ports_del(GT_U8 devNum, GT_U8 portNum)
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
	
	if (trunkId != 0)
	{
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
	}

	return ret;
}

void as_series_board_port_config(GT_U8 devNum, GT_U8 portNum, int portType)
{
	if (NOPP_PORT == portType)
	{
		int ret = 0;
		unsigned int netif_index = 0;
		npd_syslog_dbg("NOPP Port Config: Port(%d, %d)\n", devNum, portNum);
		
    	ret = npd_get_global_index_by_devport(devNum, portNum, &netif_index);
    	if(ret != 0)
    	{
    	    cpssDxChPortEnableSet(devNum, portNum, 0);
    	}
		else
		{
            struct eth_port_s* portInfo = NULL;
            portInfo = npd_get_port_by_index(netif_index);
    		if(portInfo == NULL)
    		{
    		    cpssDxChPortEnableSet(devNum, portNum, 0);
    		}
			else
			{
			    if((portInfo->attr_bitmap & ETH_ATTR_ADMIN_STATUS) && (npd_startup_end))
			    {	
			        cpssDxChPortEnableSet(devNum, portNum, 1);
			    }
				else
				{
				    cpssDxChPortEnableSet(devNum, portNum, 0);
				}
				free(portInfo);
			}
		}
		
		cpssDxChPortInterfaceModeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_XGMII_E);

		cpssDxChPortXGmiiModeSet(devNum, portNum, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
		cpssDxChCscdPortTypeSet(devNum, portNum, 2, 2);/*1 = CPSS_CSCD_PORT_NETWORK_E*/
		cpssDxChPortIpgBaseSet(devNum, portNum, 1);/*CPSS_PORT_XG_FIXED_IPG_12_BYTES_E*/
		cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 8);
		cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_FALSE);
		cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, 0);
		cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_FALSE);

		cpssDxChPortSerdesPowerStatusSet(devNum, portNum, CPSS_PORT_DIRECTION_BOTH_E, 0xf, GT_TRUE);
		cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
	    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&as_st_bmp[devNum], portNum);
	}
	else if (DSA_PORT == portType)
	{
		npd_syslog_dbg("DSA Port Config: Port(%d, %d)\n", devNum, portNum);
		cpssDxChPortXGmiiModeSet(devNum, portNum, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
		cpssDxChCscdPortTypeSet(devNum, portNum, 2, 1);/*1 = CPSS_CSCD_PORT_DSA_MODE_EXTEND_E*/
		cpssDxChPortIpgBaseSet(devNum, portNum, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
		cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 4);
		cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
		cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, 0);
		cpssDxChPortEnableSet(devNum, portNum, 0);
		cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, (as6603_cscd_need_pcl?0:1));
		cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);

		cpssDxChPortSpeedSet(devNum, portNum, CPSS_PORT_SPEED_12000_E);
		cpssDxChPortSerdesPowerStatusSet(devNum, portNum, CPSS_PORT_DIRECTION_BOTH_E, 0xf, GT_TRUE);
        cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
	    CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[devNum], portNum);
	}
	else if (SDKDIFF_PORT == portType)
	{
		unsigned char mac_addr[6];
		
		npd_system_get_basemac(mac_addr, 6);
		
		npd_syslog_dbg("SDKDIFF Port Config: Port(%d, %d)\n", devNum, portNum);
		cpssDxChPortXGmiiModeSet(devNum, portNum, 2);/*2 = CPSS_PORT_XGMII_FIXED_E*/
		cpssDxChCscdPortTypeSet(devNum, portNum, 2, 2);/*1 = CPSS_CSCD_PORT_NETWORK_E*/
		cpssDxChPortIpgBaseSet(devNum, portNum, 0);/*CPSS_PORT_XG_FIXED_IPG_8_BYTES_E*/
		cpssDxChPortPreambleLengthSet(devNum, portNum, 2, 8);
		cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, 1);
		cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, 1);
		cpssDxChIpPortRouterMacSaLsbModeSet(devNum, portNum, 0);
		cpssDxChIpRouterPortMacSaLsbSet(devNum, portNum, mac_addr[5]);
		cpssDxChPortEnableSet(devNum, portNum, 0);
		cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, 0);
    	cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
		cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, 0, 1);
	    CPSS_PORTS_BMP_PORT_SET_MAC(&as_st_bmp[devNum], portNum);
	}
}

void as_series_board_srcid_init(GT_U8 devNum, GT_U8 portNum)
{
	unsigned char targetModnum = 0;
	
	for(targetModnum = 0; targetModnum < 6; targetModnum++)
	{
		cpssDxChBrgSrcIdGroupPortAdd(devNum, targetModnum, portNum);
	}
}

long as_series_board_local_conn_init(int product_type)
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
		GT_U32 devTableBmp = 0;
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
        devTableBmp = 0x3F;
		cpssDxChBrgFdbDeviceTableSet(devNum, devTableBmp);
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
	}

    return 0;  
}

typedef struct _as_board_conn_element_
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
}as_board_conn_element;

as_board_conn_element as_board_conn[2][4];

int as_series_linecard_cross_pcl_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0, j = 0;
    int plane_port = -1;
    int peer_slot = 0, peer_port = 0;
    int peer_unit = 0, peer_type = 0;
    int third_slot = 0, sec_type, need_cross_trunk = 0;
	unsigned char unit = 0, mod_id = 0, port_num = 0;
	int tid_base = 0;
	int board_with_pp_num = 0;
	int trunk_member_count = 0;
	CPSS_TRUNK_MEMBER_STC trunk_member[4];

	npd_syslog_dbg("\n******* Entering as_series_linecard_cross_pcl_system_conn_init ********* \n\n");
	if(SYS_MODULE_RUNNINGSTATE(insert_slotid) == RMT_BOARD_NOEXIST)
	{
		npd_syslog_dbg("%s %d: Insered board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, insert_slotid);
		return 0;
	}

	as_series_board_local_conn_init(product_type);
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(port_num = 24; port_num < 28; port_num++)
		{
            cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 0);
			memset(&as_board_conn[unit][(port_num - 24)], 0, sizeof(as_board_conn_element));
			as_board_conn[unit][(port_num - 24)].local_dev = unit;
			as_board_conn[unit][(port_num - 24)].local_port = port_num;
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
                   unit, port_num);
            if(-1 == plane_port)
            {
                continue;
            }

			as_series_board_trunk_ports_del(unit, port_num);
			ax63ge48_trunk_pcl_deinit(unit, port_num);
			
			if(BOARD_INNER_CONN_PORT == plane_port)
			{
				/*board inner conn case*/
				npd_syslog_dbg("%s %d: port %d:%d is board inner conn port.\r\n", __func__, __LINE__, unit, port_num);
				if(unit == 0)
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);
				}
				else
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
				}
				as_board_conn[unit][(port_num - 24)].peer_slot = SYS_LOCAL_MODULE_SLOT_INDEX;
				as_board_conn[unit][(port_num - 24)].peer_type = SYS_LOCAL_MODULE_TYPE;
				continue;
			}
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
                   plane_port);
			if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
			{
				npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			
			peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(peer_type == 0)
			{
				npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
			as_board_conn[unit][(port_num - 24)].peer_type = peer_type;
			as_board_conn[unit][(port_num - 24)].peer_slot = peer_slot;
			
			if(!SYS_MODULE_ISHAVEPP(peer_type))
			{
				/*No pp case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a Non-pp board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].is_dest_port = 1;
    			cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
    				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
    			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);
				
				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, NOPP_PORT);
			    as_series_board_vlan_entry_del(unit, port_num);
			}
			else if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
			{
				/*Different SDK type case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

                cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
                				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
                cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);

				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, SDKDIFF_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
			else
			{
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

				as_series_board_port_config(unit, port_num, DSA_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
		}
    }
	/*for 48-24*/
    for(i = 0; i < 3; i++)
    {
        if(SYS_LOCAL_MODULE_SLOT_INDEX == i)
        {
            continue;
        }
		if(SYS_MODULE_RUNNINGSTATE(i) < RMT_BOARD_REGISTERING)
		{
		    third_slot = -1;
		    continue;
		}
		sec_type = MODULE_TYPE_ON_SLOT_INDEX(i);
		if (sec_type == 0)
		{
			third_slot = -1;
			continue;
		}
    	if(!SYS_MODULE_ISHAVEPP(sec_type))
    	{
    	    third_slot = -1;
			continue;
    	}
		if(SYS_MODULE_SDK_DIFFERENT(sec_type, SYS_LOCAL_MODULE_TYPE))
		{
		    third_slot = -1;
		    continue;
		}
		if(SYS_MODULE_PORT_NUM(sec_type) == 24)
		{
		    need_cross_trunk = 1;
		}
    }
    /*处理板内级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{	
				continue;
			}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
				/*48 port board*/
				if(third_slot == -1 && need_cross_trunk == 1)
				{
        			for(j = 0; j < 4; j++)
        			{
            			if(i == j)
            			{
            				continue;
            			}
                		if(as_board_conn[unit][j].peer_slot == -1 || 
            				as_board_conn[unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
						{
							continue;
						}
                		if(as_board_conn[unit][j].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
						{
							as_board_conn[unit][j].tid = 127;
							as_board_conn[unit][j].trunk_member = 1;
							as_board_conn[unit][j].is_dest_port = 1;
							as_board_conn[unit][j].need_redirect = 1;
							if(as_board_conn[unit][i].local_dev == 0)
							{
								if(as_board_conn[unit][i].local_port == (as_board_conn[unit][j].local_port - 2))
								{
        							as_board_conn[unit][i].tid = 127;
        							as_board_conn[unit][i].trunk_member = 1;
        							as_board_conn[unit][i].is_dest_port = 0;
									as_board_conn[unit][i].need_redirect = 0;
								}
								else
								{
        							as_board_conn[unit][i].tid = 0;
        							as_board_conn[unit][i].trunk_member = 0;
        							as_board_conn[unit][i].is_dest_port = 1;
									as_board_conn[unit][i].need_redirect = 1;
									as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
									as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
								}
							}
							else
							{
								if(as_board_conn[unit][i].local_port == (as_board_conn[unit][j].local_port + 2))
								{
        							as_board_conn[unit][i].tid = 127;
        							as_board_conn[unit][i].trunk_member = 1;
        							as_board_conn[unit][i].is_dest_port = 0;
									as_board_conn[unit][i].need_redirect = 0;
								}
								else
								{
        							as_board_conn[unit][i].tid = 0;
        							as_board_conn[unit][i].trunk_member = 0;
        							as_board_conn[unit][i].is_dest_port = 1;
									as_board_conn[unit][i].need_redirect = 1;
									as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
									as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
								}
							}
						}
        			}
				}
				else
				{
    				as_board_conn[unit][i].tid = 127;
    				as_board_conn[unit][i].trunk_member = 1;
    				as_board_conn[unit][i].is_dest_port = 1;
        			for(j = 0; j < 4; j++)
        			{
            			if(i == j)
            			{
            				continue;
            			}
                		if(as_board_conn[unit][j].peer_slot == -1 || 
            				as_board_conn[unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
						{
							continue;
						}
                		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
                		{
            				as_board_conn[unit][j].tid = 127;
            				as_board_conn[unit][j].trunk_member = 1;
							if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
							{
							    if(unit == 0)
							    {
            				        as_board_conn[unit][i].is_dest_port = 0;
            				        as_board_conn[unit][j].is_dest_port = 1;
							    }
								else
								{
            				        as_board_conn[unit][i].is_dest_port = 1;
            				        as_board_conn[unit][j].is_dest_port = 0;
								}
							}
                		}
						else
						{
							as_board_conn[unit][j].tid = 0;
							as_board_conn[unit][j].trunk_member = 0;
							as_board_conn[unit][j].is_dest_port = 1;
						}
        			}
				}
				continue;
    		}
		}
    }

	
	/*24 port board*/
	if(nam_asic_get_instance_num() == 1)
	{
		unit = 0;
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				continue;
			}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
				{
					continue;
				}
				if(as_board_conn[unit][i].peer_slot == as_board_conn[unit][j].peer_slot)
				{
					int last_tid_base = tid_base;
					if(as_board_conn[unit][i].tid == 0 && tid_base == 0)
					{
						tid_base = 1;
					}
					if(as_board_conn[unit][i].tid == 0)
					{
					    as_board_conn[unit][i].tid = 127 - last_tid_base;
					}
					if(as_board_conn[unit][j].tid == 0)
					{
					    as_board_conn[unit][j].tid = 127 - last_tid_base;
					}
					as_board_conn[unit][i].trunk_member = 1;
					as_board_conn[unit][j].trunk_member = 1;
					if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
					{
					    if(as_board_conn[unit][i].local_port == 24)
					    {
						    as_board_conn[unit][i].is_dest_port = 1;
						    as_board_conn[unit][j].is_dest_port = 0;
					    }
						else
						{
						    as_board_conn[unit][i].is_dest_port = 0;
						    as_board_conn[unit][j].is_dest_port = 1;
						}
					}
					if(as_board_conn[unit][i].peer_mod != as_board_conn[unit][j].peer_mod)
					{
						/*24-48, need redirect packet btween trunk members*/
						if(third_slot == -1)
						{
							as_board_conn[unit][i].need_redirect = 1;
							as_board_conn[unit][j].need_redirect = 1;
							as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
							as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
						}
					}
				}
			}
		}
	}

	
	/*处理板间级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				continue;
			}
			/*上面的环节已经处理过了*/
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			/*上面的环节已经处理过了*/
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
				{
					continue;
				}
    			/*上面的环节已经处理过了*/
        		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
        		{
        			continue;
        		}
    			/*上面的环节已经处理过了*/
    			if(as_board_conn[unit][j].tid != 0)
    			{
    				continue;
    			}
				
				as_board_conn[unit][i].tid = 126;
				as_board_conn[unit][i].trunk_member = 1;
				as_board_conn[unit][j].tid = 126;
				as_board_conn[unit][j].trunk_member = 1;
				if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
				{
				    if(unit == 0)
				    {
					    as_board_conn[unit][i].is_dest_port = 0;
					    as_board_conn[unit][j].is_dest_port = 1;
				    }
					else
					{
					    as_board_conn[unit][i].is_dest_port = 1;
					    as_board_conn[unit][j].is_dest_port = 0;
					}
				}
				as_board_conn[unit][i].need_redirect = 1;
				as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
				as_board_conn[unit][j].need_redirect = 1;
				as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
				
			}
			if(as_board_conn[unit][i].is_dest_port == 0 && as_board_conn[unit][i].trunk_member == 0)
			{
				as_board_conn[unit][i].is_dest_port = 1;
			}
		}
    }

	/* 处理SDKDIFF 情况的端口*/
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				as_board_conn[unit][i].tid = SDK_DIFF_TRUNK;
				as_board_conn[unit][i].trunk_member = 1;
			}
		}
	}

	/* Debug */
	npd_syslog_dbg("\nas_board_conn info \n\n");
	npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
				   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].local_dev);
			npd_syslog_dbg("%-11d", as_board_conn[unit][i].local_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].peer_mod);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_port);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_type);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_slot);
			npd_syslog_dbg("%-13d", as_board_conn[unit][i].trunk_member);
			npd_syslog_dbg("%-5d", as_board_conn[unit][i].tid);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].is_dest_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].is_src_port);
			npd_syslog_dbg("%-14d", as_board_conn[unit][i].need_redirect);
			npd_syslog_dbg("%-16d\n", as_board_conn[unit][i].redirect_from_port);
		}
	}
	
    /*trunk*/
	npd_syslog_dbg("\ntrunk config\n");
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				continue;
			}
			if(as_board_conn[unit][i].trunk_member == 1)
			{
				for(j = (i + 1); j < 4; j++)
				{
					if(as_board_conn[unit][i].tid == as_board_conn[unit][j].tid)
					{
						int dest_port = 0;
						if(as_board_conn[unit][i].is_dest_port == 1)
						{
							dest_port = as_board_conn[unit][i].local_port;
						}
						else
						{
							dest_port = as_board_conn[unit][j].local_port;
						}
						npd_syslog_dbg("devNum = %d	tid = %d	portNumA = %d	portNumB = %d	designatedPort = %d\n", 
							unit, as_board_conn[unit][i].tid, 
							as_board_conn[unit][i].local_port,
							as_board_conn[unit][j].local_port, 
							dest_port);
						ax63ge48_trunk_port_config(unit, as_board_conn[unit][i].tid,
							as_board_conn[unit][i].local_port, 
							as_board_conn[unit][j].local_port,
							dest_port, 0, 0, 0, 0, 0);
					}
				}
			}
		}
    }

	/* SDKDifferent trunk config */
	/* 对于sdkdiff的情况，要去轮询对端的板子，根据对端为SDKDIFF的端口
	    反向找到要加入trunk的成员*/
	npd_syslog_dbg("\nsdkDifferent trunk config\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		trunk_member_count = 0;
		memset(trunk_member, 0, sizeof(trunk_member));
		
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    for(j = 0; j < PPAL_PLANE_PORT_COUNT(as_board_conn[unit][i].peer_type); j++)
			    {
			        if(j >= PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE))
			        {
			            break;
			        }
                    peer_slot = SLOT_PORT_PEER_SLOT(as_board_conn[unit][i].peer_slot, j);
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
					
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, as_board_conn[unit][i].peer_type)
						&& !SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
					    GT_TRUNK_ID trunk_id = 0, tmp = 0;
						GT_U8 peer_mod = 0;
				        int find_port = 0;
            			peer_port = SLOT_PORT_PEER_PORT(as_board_conn[unit][i].peer_slot, j);
						if(peer_port == -1)
						{
						    continue;
						}
						
            			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
            			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
						for(tmp = 0; tmp < trunk_member_count; tmp++)
						{
							if(trunk_member[tmp].device == peer_mod &&
								trunk_member[tmp].port == peer_port)
							{
								find_port = 1;
								break;
							}
						}

						if(!find_port)
						{
							trunk_member[trunk_member_count].device = peer_mod;
							trunk_member[trunk_member_count].port = peer_port;
							ret = cpssDxChTrunkDbIsMemberOfTrunk(unit, &trunk_member[trunk_member_count], &trunk_id);
							if(ret == 0 && trunk_id == SDK_DIFF_TRUNK)
							{
								npd_syslog_dbg("Remove: unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							    cpssDxChTrunkMemberRemove(unit, SDK_DIFF_TRUNK, &trunk_member[trunk_member_count]);
							}
							npd_syslog_dbg("unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							trunk_member_count++; 
						}
					}
			    }
			}
		}
		if(trunk_member_count)
		{
		    npd_syslog_dbg("trunk_member_count = %d\r\n", trunk_member_count);
            cpssDxChTrunkMembersSet(unit, SDK_DIFF_TRUNK, trunk_member_count, trunk_member, 0, NULL);
		}
	}

	npd_syslog_dbg("\nsrc-id config\n");
	/*multi-destination flow. (src-id)*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int srcid = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				continue;
			}
		    if(as_board_conn[unit][i].is_dest_port == 0)
		    {
		        continue;
		    }
			for(srcid = 0; srcid < 6; srcid++)
			{
			    if(as_board_conn[unit][i].peer_mod == srcid)
			    {
			        continue;
			    }
				if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
				{
					npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
						srcid, as_board_conn[unit][i].local_port);
				    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);

				}
				else if(MOD_ID_TO_SLOT_INDEX(srcid) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    int another_unit = MODULE_2_UNIT(SYS_LOCAL_MODULE_TYPE, srcid);
					int need_dest = 1;
					if(another_unit == -1)
					{
					    continue;
					}
					
					for(j = 0; j < 4; j++)
					{
					    if(i == j)
                		{
                			continue;
                		}
                		if(as_board_conn[another_unit][j].peer_slot == -1 || 
            				as_board_conn[another_unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[another_unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[another_unit][j].peer_type))
						{
							continue;
						}
     				    if(as_board_conn[another_unit][j].peer_mod == as_board_conn[unit][i].peer_mod)
     				    {
     				        if(as_board_conn[another_unit][j].is_dest_port == 1)
     				        {
     				            need_dest = 0;
							    break;
     				        }
     				    }
					}
					if(need_dest == 1)
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, as_board_conn[unit][i].local_port);
					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
					}
				}
				else
				{
				    int another_unit = 0;
					int need_dest = 1;
					int src_slot = MOD_ID_TO_SLOT_INDEX(srcid);
				    if(nam_asic_get_instance_num() == 1)
				    {
				        continue;
				    }
					if(as_board_conn[unit][i].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
					{
					    continue;
					}
					if(unit == 0)
					{
					    another_unit = 1;
					}
					for(j = 0; j < 4; j++)
					{
					    if(as_board_conn[another_unit][j].peer_slot == src_slot)
					    {
					        if(as_board_conn[another_unit][j].trunk_member == 0 &&
								(!(SYS_MODULE_SDK_DIFFERENT(as_board_conn[another_unit][j].peer_type, SYS_LOCAL_MODULE_TYPE))))
					        {
					            /*48-48*/
					            need_dest = 0;
								break;
					        }
					        else if((as_board_conn[another_unit][j].peer_port == 24 || 
								as_board_conn[another_unit][j].peer_port == 27) &&
								(!(SYS_MODULE_SDK_DIFFERENT(as_board_conn[another_unit][j].peer_type, SYS_LOCAL_MODULE_TYPE))))
					        {
					            need_dest = 0;
								break;
					        }
					    }
					}
					if(need_dest == 1)
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, as_board_conn[unit][i].local_port);
					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
					}
				}
			}
		}
    }

	npd_syslog_dbg("\ndev map config\n");
	/*dev map*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
		int dest_mod = 0;
		for(dest_mod = 0; dest_mod < 6; dest_mod++)
		{
			int trunk_connect = 0;
			int port_connect = 0;
			if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
			{
				continue;
			}
    		for(i = 0; i < 4; i++)
    		{
        		if(as_board_conn[unit][i].peer_slot == -1 || 
    				as_board_conn[unit][i].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
				{
					continue;
				}
    			if(as_board_conn[unit][i].trunk_member && dest_mod== as_board_conn[unit][i].peer_mod)
    			{
					trunk_connect = 1;
					break;
    			}
    			else
    			{
    				if(dest_mod == as_board_conn[unit][i].peer_mod)
    				{
						port_connect = as_board_conn[unit][i].local_port;
    				}
    			}
    		}
			if(trunk_connect)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = as_board_conn[unit][i].tid;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, as_board_conn[unit][i].tid);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else if(port_connect != 0)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			cascadeLinkPtr.linkNum = port_connect;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPE_PORT	linkNum = %d\n", unit, 
					dest_mod, port_connect);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = 127;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, 127);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
		}
    }

	npd_syslog_dbg("\nredirect config\n");
	/*redirect*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int dest_mod = 0;
		int run_level = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				continue;
			}
			if(as_board_conn[unit][i].need_redirect == 1)
			{
				npd_syslog_dbg("devNum = %d	portNumA = %d	portNumB = %d	neiborModNum = %d\n", unit,
					as_board_conn[unit][i].redirect_from_port, 
					as_board_conn[unit][i].local_port, 
					as_board_conn[unit][i].peer_mod);
				ax63ge48_trunk_pcl_config(unit, as_board_conn[unit][i].redirect_from_port, 
					as_board_conn[unit][i].local_port, as_board_conn[unit][i].peer_mod, 
					run_level++, 0);
			}
		}
    }


	npd_syslog_dbg("\nEnable Port \n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			cpssDxChPortEnableSet(as_board_conn[unit][i].local_dev, as_board_conn[unit][i].local_port, 1);
		}
    }

	npd_syslog_dbg("\n******* Leaving  as_series_linecard_cross_pcl_system_conn_init ********* \n\n");
	
}

int as_series_linecard_cross_pcl_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0, j = 0;
    int plane_port = -1;
    int peer_slot = 0, peer_port = 0;
    int peer_unit = 0, peer_type = 0;
    int third_slot = -1, sec_type, need_cross_trunk = 0;
	unsigned char unit = 0, mod_id = 0, port_num = 0;
	int tid_base = 0;
	int board_with_pp_num = 0;
	int trunk_member_count = 0;
	CPSS_TRUNK_MEMBER_STC trunk_member[4];

	npd_syslog_dbg("\n******* Entering as_series_linecard_cross_pcl_system_conn_deinit ********* \n\n");

	as_series_board_local_conn_init(product_type);
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(port_num = 24; port_num < 28; port_num++)
		{
            cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 0);
			memset(&as_board_conn[unit][(port_num - 24)], 0, sizeof(as_board_conn_element));
			as_board_conn[unit][(port_num - 24)].local_dev = unit;
			as_board_conn[unit][(port_num - 24)].local_port = port_num;
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
                   unit, port_num);
            if(-1 == plane_port)
            {
                continue;
            }

			as_series_board_trunk_ports_del(unit, port_num);
			ax63ge48_trunk_pcl_deinit(unit, port_num);
			
			if(BOARD_INNER_CONN_PORT == plane_port)
			{
				/*board inner conn case*/
				npd_syslog_dbg("%s %d: port %d:%d is board inner conn port.\r\n", __func__, __LINE__, unit, port_num);
				if(unit == 0)
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);
				}
				else
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
				}
				as_board_conn[unit][(port_num - 24)].peer_slot = SYS_LOCAL_MODULE_SLOT_INDEX;
				as_board_conn[unit][(port_num - 24)].peer_type = SYS_LOCAL_MODULE_TYPE;
				continue;
			}
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
                   plane_port);
			if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST || (peer_slot == delete_slotid))
			{
				npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			
			peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(peer_type == 0)
			{
				npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
			as_board_conn[unit][(port_num - 24)].peer_type = peer_type;
			as_board_conn[unit][(port_num - 24)].peer_slot = peer_slot;
			
			if(!SYS_MODULE_ISHAVEPP(peer_type))
			{
				/*No pp case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a Non-pp board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].is_dest_port = 1;
    			cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
    				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
    			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);
				
				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, NOPP_PORT);
			    as_series_board_vlan_entry_del(unit, port_num);
			}
			else if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
			{
				/*Different SDK type case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

                cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
                	UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
                cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);

				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, SDKDIFF_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
			else
			{
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

				as_series_board_port_config(unit, port_num, DSA_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
		}
    }
	/*for 48-24*/
    for(i = 0; i < 3; i++)
    {
        if(SYS_LOCAL_MODULE_SLOT_INDEX == i)
        {
            continue;
        }
		if(i == delete_slotid)
		{
		    continue;
		}
		sec_type = MODULE_TYPE_ON_SLOT_INDEX(i);
		if (sec_type == 0)
		{
			third_slot = -1;
			continue;
		}
    	if(!SYS_MODULE_ISHAVEPP(sec_type))
    	{
    	    third_slot = -1;
			continue;
    	}
		if(SYS_MODULE_SDK_DIFFERENT(sec_type, SYS_LOCAL_MODULE_TYPE))
		{
		    third_slot = -1;
		    continue;
		}
		if(SYS_MODULE_PORT_NUM(sec_type) == 24)
		{
		    need_cross_trunk = 1;
		}
    }
    /*处理板内级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
				/*48 port board*/
				if(third_slot == -1 && need_cross_trunk == 1)
				{
        			for(j = 0; j < 4; j++)
        			{
            			if(i == j)
            			{
            				continue;
            			}
                		if(as_board_conn[unit][j].peer_slot == -1 || 
            				as_board_conn[unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
						{
						    continue;
						}
                		if(as_board_conn[unit][j].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
						{
							as_board_conn[unit][j].tid = 127;
							as_board_conn[unit][j].trunk_member = 1;
							as_board_conn[unit][j].is_dest_port = 1;
							as_board_conn[unit][j].need_redirect = 1;
							if(as_board_conn[unit][i].local_dev == 0)
							{
								if(as_board_conn[unit][i].local_port == (as_board_conn[unit][j].local_port - 2))
								{
        							as_board_conn[unit][i].tid = 127;
        							as_board_conn[unit][i].trunk_member = 1;
        							as_board_conn[unit][i].is_dest_port = 0;
									as_board_conn[unit][i].need_redirect = 0;
								}
								else
								{
        							as_board_conn[unit][i].tid = 0;
        							as_board_conn[unit][i].trunk_member = 0;
        							as_board_conn[unit][i].is_dest_port = 1;
									as_board_conn[unit][i].need_redirect = 1;
									as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
									as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
								}
							}
							else
							{
								if(as_board_conn[unit][i].local_port == (as_board_conn[unit][j].local_port + 2))
								{
        							as_board_conn[unit][i].tid = 127;
        							as_board_conn[unit][i].trunk_member = 1;
        							as_board_conn[unit][i].is_dest_port = 0;
									as_board_conn[unit][i].need_redirect = 0;
								}
								else
								{
        							as_board_conn[unit][i].tid = 0;
        							as_board_conn[unit][i].trunk_member = 0;
        							as_board_conn[unit][i].is_dest_port = 1;
									as_board_conn[unit][i].need_redirect = 1;
									as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
									as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
								}
							}
						}
        			}
				}
				else
				{
    				as_board_conn[unit][i].tid = 127;
    				as_board_conn[unit][i].trunk_member = 1;
    				as_board_conn[unit][i].is_dest_port = 1;
        			for(j = 0; j < 4; j++)
        			{
            			if(i == j)
            			{
            				continue;
            			}
                		if(as_board_conn[unit][j].peer_slot == -1 || 
            				as_board_conn[unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
						{
						    continue;
						}
                		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
                		{
            				as_board_conn[unit][j].tid = 127;
            				as_board_conn[unit][j].trunk_member = 1;
							if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
							{
							    if(unit == 0)
							    {
            				        as_board_conn[unit][i].is_dest_port = 0;
            				        as_board_conn[unit][j].is_dest_port = 1;
							    }
								else
								{
            				        as_board_conn[unit][i].is_dest_port = 1;
            				        as_board_conn[unit][j].is_dest_port = 0;
								}
							}
                		}
						else
						{
							as_board_conn[unit][j].tid = 0;
							as_board_conn[unit][j].trunk_member = 0;
							as_board_conn[unit][j].is_dest_port = 1;
						}
        			}
				}
				continue;
    		}
		}
    }

	
	/*24 port board*/
	if(nam_asic_get_instance_num() == 1)
	{
		unit = 0;
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
				{
				    continue;
				}
				if(as_board_conn[unit][i].peer_slot == as_board_conn[unit][j].peer_slot)
				{
					int last_tid_base = tid_base;
					if(as_board_conn[unit][i].tid == 0 && tid_base == 0)
					{
						tid_base = 1;
					}
					if(as_board_conn[unit][i].tid == 0)
					{
					    as_board_conn[unit][i].tid = 127 - last_tid_base;
					}
					if(as_board_conn[unit][j].tid == 0)
					{
					    as_board_conn[unit][j].tid = 127 - last_tid_base;
					}
					as_board_conn[unit][i].trunk_member = 1;
					as_board_conn[unit][j].trunk_member = 1;
					if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
					{
					    if(as_board_conn[unit][i].local_port == 24)
					    {
						    as_board_conn[unit][i].is_dest_port = 1;
						    as_board_conn[unit][j].is_dest_port = 0;
					    }
						else
						{
						    as_board_conn[unit][i].is_dest_port = 0;
						    as_board_conn[unit][j].is_dest_port = 1;
						}
					}
					if(as_board_conn[unit][i].peer_mod != as_board_conn[unit][j].peer_mod)
					{
						/*24-48, need redirect packet btween trunk members*/
						if(third_slot == -1)
						{
							as_board_conn[unit][i].need_redirect = 1;
							as_board_conn[unit][j].need_redirect = 1;
							as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
							as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
						}
					}
				}
			}
		}
	}

	
	/*处理板间级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			/*上面的环节已经处理过了*/
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			/*上面的环节已经处理过了*/
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
				{
				    continue;
				}
    			/*上面的环节已经处理过了*/
        		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
        		{
        			continue;
        		}
    			/*上面的环节已经处理过了*/
    			if(as_board_conn[unit][j].tid != 0)
    			{
    				continue;
    			}
				
				as_board_conn[unit][i].tid = 126;
				as_board_conn[unit][i].trunk_member = 1;
				as_board_conn[unit][j].tid = 126;
				as_board_conn[unit][j].trunk_member = 1;
				if(as_board_conn[unit][i].local_port < as_board_conn[unit][j].local_port)
				{
				    if(unit == 0)
				    {
					    as_board_conn[unit][i].is_dest_port = 0;
					    as_board_conn[unit][j].is_dest_port = 1;
				    }
					else
					{
					    as_board_conn[unit][i].is_dest_port = 1;
					    as_board_conn[unit][j].is_dest_port = 0;
					}
				}
				as_board_conn[unit][i].need_redirect = 1;
				as_board_conn[unit][i].redirect_from_port = as_board_conn[unit][j].local_port;
				as_board_conn[unit][j].need_redirect = 1;
				as_board_conn[unit][j].redirect_from_port = as_board_conn[unit][i].local_port;
				
			}
			if(as_board_conn[unit][i].is_dest_port == 0 && as_board_conn[unit][i].trunk_member == 0)
			{
				as_board_conn[unit][i].is_dest_port = 1;
			}
		}
    }

	/* 处理SDKDIFF 情况的端口*/
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				as_board_conn[unit][i].tid = SDK_DIFF_TRUNK;
				as_board_conn[unit][i].trunk_member = 1;
			}
		}
	}

	/* Debug */
	npd_syslog_dbg("\nas_board_conn info \n\n");
	npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
				   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].local_dev);
			npd_syslog_dbg("%-11d", as_board_conn[unit][i].local_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].peer_mod);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_port);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_type);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_slot);
			npd_syslog_dbg("%-13d", as_board_conn[unit][i].trunk_member);
			npd_syslog_dbg("%-5d", as_board_conn[unit][i].tid);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].is_dest_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].is_src_port);
			npd_syslog_dbg("%-14d", as_board_conn[unit][i].need_redirect);
			npd_syslog_dbg("%-16d\n", as_board_conn[unit][i].redirect_from_port);
		}
	}
	
    /*trunk*/
	npd_syslog_dbg("\ntrunk config\n");
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			if(as_board_conn[unit][i].trunk_member == 1)
			{
				for(j = (i + 1); j < 4; j++)
				{
					if(as_board_conn[unit][i].tid == as_board_conn[unit][j].tid)
					{
						int dest_port = 0;
						if(as_board_conn[unit][i].is_dest_port == 1)
						{
							dest_port = as_board_conn[unit][i].local_port;
						}
						else
						{
							dest_port = as_board_conn[unit][j].local_port;
						}
						npd_syslog_dbg("devNum = %d	tid = %d	portNumA = %d	portNumB = %d	designatedPort = %d\n", 
							unit, as_board_conn[unit][i].tid, 
							as_board_conn[unit][i].local_port,
							as_board_conn[unit][j].local_port, 
							dest_port);
						ax63ge48_trunk_port_config(unit, as_board_conn[unit][i].tid,
							as_board_conn[unit][i].local_port, 
							as_board_conn[unit][j].local_port,
							dest_port, 0, 0, 0, 0, 0);
					}
				}
			}
		}
    }

	/* SDKDifferent trunk config */
	/* 对于sdkdiff的情况，要去轮询对端的板子，根据对端为SDKDIFF的端口
	    反向找到要加入trunk的成员*/
	npd_syslog_dbg("\nsdkDifferent trunk config\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		trunk_member_count = 0;
		memset(trunk_member, 0, sizeof(trunk_member));
		
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    for(j = 0; j < PPAL_PLANE_PORT_COUNT(as_board_conn[unit][i].peer_type); j++)
			    {
			        if(j >= PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE))
			        {
			            break;
			        }
                    peer_slot = SLOT_PORT_PEER_SLOT(as_board_conn[unit][i].peer_slot, j);
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
					
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, as_board_conn[unit][i].peer_type)
						&& !SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
					    GT_TRUNK_ID trunk_id = 0, tmp = 0;
						GT_U8 peer_mod = 0;
				        int find_port = 0;
            			peer_port = SLOT_PORT_PEER_PORT(as_board_conn[unit][i].peer_slot, j);
						if(peer_port == -1)
						{
						    continue;
						}
						
            			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
            			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
						for(tmp = 0; tmp < trunk_member_count; tmp++)
						{
							if(trunk_member[tmp].device == peer_mod &&
								trunk_member[tmp].port == peer_port)
							{
								find_port = 1;
								break;
							}
						}

						if(!find_port)
						{
							trunk_member[trunk_member_count].device = peer_mod;
							trunk_member[trunk_member_count].port = peer_port;
							ret = cpssDxChTrunkDbIsMemberOfTrunk(unit, &trunk_member[trunk_member_count], &trunk_id);
							if(ret == 0 && trunk_id == SDK_DIFF_TRUNK)
							{
								npd_syslog_dbg("Remove: unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							    cpssDxChTrunkMemberRemove(unit, SDK_DIFF_TRUNK, &trunk_member[trunk_member_count]);
							}
							npd_syslog_dbg("unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							trunk_member_count++; 
						}
					}
			    }
			}
		}
		if(trunk_member_count)
		{
		    npd_syslog_dbg("trunk_member_count = %d\r\n", trunk_member_count);
            cpssDxChTrunkMembersSet(unit, SDK_DIFF_TRUNK, trunk_member_count, trunk_member, 0, NULL);
		}
	}


	npd_syslog_dbg("\nsrc-id config\n");
	/*multi-destination flow. (src-id)*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int srcid = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
		    if(as_board_conn[unit][i].is_dest_port == 0)
		    {
		        continue;
		    }
			for(srcid = 0; srcid < 6; srcid++)
			{
			    if(as_board_conn[unit][i].peer_mod == srcid)
			    {
			        continue;
			    }
				if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
				{
					npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
						srcid, as_board_conn[unit][i].local_port);
				    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);

				}
				else if(MOD_ID_TO_SLOT_INDEX(srcid) == SYS_LOCAL_MODULE_SLOT_INDEX)
				{
				    int another_unit = MODULE_2_UNIT(SYS_LOCAL_MODULE_TYPE, srcid);
					int need_dest = 1;
					if(another_unit == -1)
					{
					    continue;
					}
					
					for(j = 0; j < 4; j++)
					{
					    if(i == j)
                		{
                			continue;
                		}
                		if(as_board_conn[another_unit][j].peer_slot == -1 || 
            				as_board_conn[another_unit][j].peer_type == 0)
                		{
                			continue;
                		}
                		if(!SYS_MODULE_ISHAVEPP(as_board_conn[another_unit][j].peer_type))
                		{
                			continue;
                		}
						if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[another_unit][j].peer_type))
						{
						    continue;
						}
     				    if(as_board_conn[another_unit][j].peer_mod == as_board_conn[unit][i].peer_mod)
     				    {
     				        if(as_board_conn[another_unit][j].is_dest_port == 1)
     				        {
     				            need_dest = 0;
							    break;
     				        }
     				    }
					}
					if(need_dest == 1)
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, as_board_conn[unit][i].local_port);
					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
					}
				}
				else
				{
				    int another_unit = 0;
					int need_dest = 1;
					int src_slot = MOD_ID_TO_SLOT_INDEX(srcid);
				    if(nam_asic_get_instance_num() == 1)
				    {
				        continue;
				    }
					if(as_board_conn[unit][i].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
					{
					    continue;
					}
					if(unit == 0)
					{
					    another_unit = 1;
					}
					for(j = 0; j < 4; j++)
					{
					    if(as_board_conn[another_unit][j].peer_slot == src_slot)
					    {
					        if(as_board_conn[another_unit][j].trunk_member == 0 &&
								(!(SYS_MODULE_SDK_DIFFERENT(as_board_conn[another_unit][j].peer_type, SYS_LOCAL_MODULE_TYPE))))
					        {
					            /*48-48*/
					            need_dest = 0;
								break;
					        }
					        else if((as_board_conn[another_unit][j].peer_port == 24 || 
								as_board_conn[another_unit][j].peer_port == 27) &&
								(!(SYS_MODULE_SDK_DIFFERENT(as_board_conn[another_unit][j].peer_type, SYS_LOCAL_MODULE_TYPE))))
					        {
					            need_dest = 0;
								break;
					        }
					    }
					}
					if(need_dest == 1)
					{
						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
							srcid, as_board_conn[unit][i].local_port);
					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
					}
				}
			}
		}
    }

	npd_syslog_dbg("\ndev map config\n");
	/*dev map*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
		int dest_mod = 0;
		for(dest_mod = 0; dest_mod < 6; dest_mod++)
		{
			int trunk_connect = 0;
			int port_connect = 0;
			if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
			{
				continue;
			}
    		for(i = 0; i < 4; i++)
    		{
        		if(as_board_conn[unit][i].peer_slot == -1 || 
    				as_board_conn[unit][i].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
        		{
        			continue;
        		}
				if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
				{
				    continue;
				}
				
    			if(as_board_conn[unit][i].trunk_member && dest_mod== as_board_conn[unit][i].peer_mod)
    			{
					trunk_connect = 1;
					break;
    			}
    			else
    			{
    				if(dest_mod == as_board_conn[unit][i].peer_mod)
    				{
						port_connect = as_board_conn[unit][i].local_port;
    				}
    			}
    		}
			if(trunk_connect)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = as_board_conn[unit][i].tid;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, as_board_conn[unit][i].tid);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else if(port_connect != 0)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			cascadeLinkPtr.linkNum = port_connect;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPE_PORT	linkNum = %d\n", unit, 
					dest_mod, port_connect);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = 127;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, 127);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
		}
    }

	npd_syslog_dbg("\nredirect config\n");
	/*redirect*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int dest_mod = 0;
		int run_level = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			if(as_board_conn[unit][i].need_redirect == 1)
			{
				npd_syslog_dbg("devNum = %d	portNumA = %d	portNumB = %d	neiborModNum = %d\n", unit,
					as_board_conn[unit][i].redirect_from_port, 
					as_board_conn[unit][i].local_port, 
					as_board_conn[unit][i].peer_mod);
				ax63ge48_trunk_pcl_config(unit, as_board_conn[unit][i].redirect_from_port, 
					as_board_conn[unit][i].local_port, as_board_conn[unit][i].peer_mod, 
					run_level++, 0);
			}
		}
    }


	npd_syslog_dbg("\nEnable Port \n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			cpssDxChPortEnableSet(as_board_conn[unit][i].local_dev, as_board_conn[unit][i].local_port, 1);
		}
    }

	npd_syslog_dbg("\n******* Leaving  as_series_linecard_cross_pcl_system_conn_deinit ********* \n\n");
	
}

long as_series_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0, j = 0;
    int plane_port = -1;
    int peer_slot = 0, peer_port = 0;
    int peer_unit = 0, peer_type = 0;
    int third_slot = 0, sec_type, need_cross_trunk = 0;
	unsigned char unit = 0, mod_id = 0, port_num = 0;
	int tid_base = 0;
	int trunk_member_count = 0;
	CPSS_TRUNK_MEMBER_STC trunk_member[4];
    if(as6603_cscd_need_pcl == 1)
    {
		return as_series_linecard_cross_pcl_system_conn_init(product_type, insert_board_type, insert_slotid);
    }
	npd_syslog_dbg("\n******* Entering as_series_linecard_system_conn_init ********* \n\n");
	if(SYS_MODULE_RUNNINGSTATE(insert_slotid) == RMT_BOARD_NOEXIST)
	{
		npd_syslog_dbg("%s %d: Insered board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, insert_slotid);
		return 0;
	}

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(port_num = 24; port_num < 28; port_num++)
		{
            cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 0);
			memset(&as_board_conn[unit][(port_num - 24)], 0, sizeof(as_board_conn_element));
			as_board_conn[unit][(port_num - 24)].local_dev = unit;
			as_board_conn[unit][(port_num - 24)].local_port = port_num;
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
                   unit, port_num);
            if(-1 == plane_port)
            {
                continue;
            }

			if(BOARD_INNER_CONN_PORT == plane_port)
			{
				/*board inner conn case*/
				npd_syslog_dbg("%s %d: port %d:%d is board inner conn port.\r\n", __func__, __LINE__, unit, port_num);
				if(unit == 0)
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);
				}
				else
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
				}
				as_board_conn[unit][(port_num - 24)].peer_slot = SYS_LOCAL_MODULE_SLOT_INDEX;
				as_board_conn[unit][(port_num - 24)].peer_type = SYS_LOCAL_MODULE_TYPE;
				continue;
			}
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
                   plane_port);
			if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST)
			{
				npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			
			peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(peer_type == 0)
			{
				npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
			as_board_conn[unit][(port_num - 24)].peer_type = peer_type;
			as_board_conn[unit][(port_num - 24)].peer_slot = peer_slot;
			
			if(!SYS_MODULE_ISHAVEPP(peer_type))
			{
				/*No pp case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a Non-pp board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].is_dest_port = 1;
    			cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
    				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
    			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);
				
				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, NOPP_PORT);
			    as_series_board_vlan_entry_del(unit, port_num);
			}
			else if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
			{
				/*Different SDK type case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

                cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
                				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
                cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);

				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, SDKDIFF_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
			else
			{
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

				as_series_board_port_config(unit, port_num, DSA_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
		}
    }

	
	/*处理板间级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			/*上面的环节已经处理过了*/
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
				tid_base = 1;
    			continue;
    		}
			/*上面的环节已经处理过了*/
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
    			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
    			{
    			    continue;
    			}
    			/*上面的环节已经处理过了*/
        		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
        		{
        			continue;
        		}
    			/*上面的环节已经处理过了*/
    			if(as_board_conn[unit][j].tid != 0)
    			{
    				continue;
    			}
				if(as_board_conn[unit][i].peer_slot == as_board_conn[unit][j].peer_slot)
				{
					as_board_conn[unit][i].tid = 127 - tid_base;
					as_board_conn[unit][j].tid = 127 - tid_base;
					as_board_conn[unit][i].trunk_member = 1;
					as_board_conn[unit][j].trunk_member = 1;
					tid_base += 1;
				}
			}
		}
    }

	/* 处理SDKDIFF 情况的端口*/
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				as_board_conn[unit][i].tid = SDK_DIFF_TRUNK;
				as_board_conn[unit][i].trunk_member = 1;
			}
		}
	}

	/* Debug */
	npd_syslog_dbg("\nas_board_conn info \n\n");
	npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
				   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].local_dev);
			npd_syslog_dbg("%-11d", as_board_conn[unit][i].local_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].peer_mod);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_port);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_type);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_slot);
			npd_syslog_dbg("%-13d", as_board_conn[unit][i].trunk_member);
			npd_syslog_dbg("%-5d", as_board_conn[unit][i].tid);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].is_dest_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].is_src_port);
			npd_syslog_dbg("%-14d", as_board_conn[unit][i].need_redirect);
			npd_syslog_dbg("%-16d\n", as_board_conn[unit][i].redirect_from_port);
		}
	}
	
    /*trunk*/
	npd_syslog_dbg("\ntrunk config\n");
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			if(as_board_conn[unit][i].trunk_member == 1)
			{
				for(j = (i + 1); j < 4; j++)
				{
					if(as_board_conn[unit][i].tid == as_board_conn[unit][j].tid)
					{
						npd_syslog_dbg("devNum = %d	tid = %d	portNumA = %d	portNumB = %d\n", 
							unit, as_board_conn[unit][i].tid, 
							as_board_conn[unit][i].local_port,
							as_board_conn[unit][j].local_port);
						ax63ge48_trunk_port_config(unit, as_board_conn[unit][i].tid,
							as_board_conn[unit][i].local_port, 
							as_board_conn[unit][j].local_port,
							0, 0, 0, 0, 0, 0);
					}
				}
			}
		}
    }

	/* SDKDifferent trunk config */
	/* 对于sdkdiff的情况，要去轮询对端的板子，根据对端为SDKDIFF的端口
	    反向找到要加入trunk的成员*/
	npd_syslog_dbg("\nsdkDifferent trunk config\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		trunk_member_count = 0;
		memset(trunk_member, 0, sizeof(trunk_member));
		
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    for(j = 0; j < PPAL_PLANE_PORT_COUNT(as_board_conn[unit][i].peer_type); j++)
			    {
			        if(j >= PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE))
			        {
			            break;
			        }
                    peer_slot = SLOT_PORT_PEER_SLOT(as_board_conn[unit][i].peer_slot, j);
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
					
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, as_board_conn[unit][i].peer_type)
						&& !SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
					    GT_TRUNK_ID trunk_id = 0, tmp = 0;
						GT_U8 peer_mod = 0;
				        int find_port = 0;
            			peer_port = SLOT_PORT_PEER_PORT(as_board_conn[unit][i].peer_slot, j);
						if(peer_port == -1)
						{
						    continue;
						}
						
            			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
            			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
						for(tmp = 0; tmp < trunk_member_count; tmp++)
						{
							if(trunk_member[tmp].device == peer_mod &&
								trunk_member[tmp].port == peer_port)
							{
								find_port = 1;
								break;
							}
						}

						if(!find_port)
						{
							trunk_member[trunk_member_count].device = peer_mod;
							trunk_member[trunk_member_count].port = peer_port;
							ret = cpssDxChTrunkDbIsMemberOfTrunk(unit, &trunk_member[trunk_member_count], &trunk_id);
							if(ret == 0 && trunk_id == SDK_DIFF_TRUNK)
							{
								npd_syslog_dbg("Remove: unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							    cpssDxChTrunkMemberRemove(unit, SDK_DIFF_TRUNK, &trunk_member[trunk_member_count]);
							}
							npd_syslog_dbg("unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							trunk_member_count++; 
						}
					}
			    }
			}
		}
		if(trunk_member_count)
		{
		    npd_syslog_dbg("trunk_member_count = %d\r\n", trunk_member_count);
            cpssDxChTrunkMembersSet(unit, SDK_DIFF_TRUNK, trunk_member_count, trunk_member, 0, NULL);
		}
	}

	npd_syslog_dbg("\nsrc-id config\n");
	/*multi-destination flow. (src-id)*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int srcid = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			for(srcid = 0; srcid < 6; srcid++)
			{
			    if(as_board_conn[unit][i].peer_mod == srcid)
			    {
			        continue;
			    }
				/*来自本单元*/
				if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
				{
					npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
						srcid, as_board_conn[unit][i].local_port);
				    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
				}
				else
				{
					/*来自本板另一个单元*/
				    if(MOD_ID_TO_SLOT_INDEX(srcid) == SYS_LOCAL_MODULE_SLOT_INDEX)
    				{
    				    int another_unit = MODULE_2_UNIT(SYS_LOCAL_MODULE_TYPE, srcid);
    					int need_dest = 1;
    					if(another_unit == -1)
    					{
    					    continue;
    					}
    					
    					for(j = 0; j < 4; j++)
    					{
                    		if(as_board_conn[another_unit][j].peer_slot == -1 || 
                				as_board_conn[another_unit][j].peer_type == 0)
                    		{
                    			continue;
                    		}
                    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[another_unit][j].peer_type))
                    		{
                    			continue;
                    		}
                			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[another_unit][j].peer_type))
                			{
                			    continue;
                			}
         				    if(as_board_conn[another_unit][j].peer_mod == as_board_conn[unit][i].peer_mod)
         				    {
         				        need_dest = 0;
    							break;
         				    }
         				    if(as_board_conn[another_unit][j].peer_slot == as_board_conn[unit][i].peer_slot)
         				    {
         				        need_dest = 0;
    							break;
         				    }
    					}
    					if(need_dest == 1)
    					{
    						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
    							srcid, as_board_conn[unit][i].local_port);
    					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
    					}
						else
						{
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
						}
    				}
				    else
    				{
						/*来自其它单板*/
    					int need_dest = 1;
    				    if(nam_asic_get_instance_num() == 1)
    				    {
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
    				        continue;
    				    }
    					if(as_board_conn[unit][i].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
    					{
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
    					    continue;
    					}
    					if(need_dest == 1)
    					{
    						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
    							srcid, as_board_conn[unit][i].local_port);
    					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
    					}
    				}
				}
			}
		}
    }

	npd_syslog_dbg("\ndev map config\n");
	/*dev map*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
		int dest_mod = 0;
		for(dest_mod = 0; dest_mod < 6; dest_mod++)
		{
			int trunk_connect = 0;
			int port_connect = 0;
			if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
			{
				continue;
			}
    		for(i = 0; i < 4; i++)
    		{
        		if(as_board_conn[unit][i].peer_slot == -1 || 
    				as_board_conn[unit][i].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
        		{
        			continue;
        		}
    			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
    			{
    			    continue;
    			}
    			if(as_board_conn[unit][i].trunk_member && dest_mod == as_board_conn[unit][i].peer_mod)
    			{
					trunk_connect = 1;
					break;
    			}
    			else
    			{
    				if(dest_mod == as_board_conn[unit][i].peer_mod)
    				{
						port_connect = as_board_conn[unit][i].local_port;
    				}
    			}
    		}
			if(trunk_connect)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = as_board_conn[unit][i].tid;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, as_board_conn[unit][i].tid);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else if(port_connect != 0)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			cascadeLinkPtr.linkNum = port_connect;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPE_PORT	linkNum = %d\n", unit, 
					dest_mod, port_connect);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = 127;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, 127);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
		}
    }

	npd_syslog_dbg("\nEnable Port \n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			cpssDxChPortEnableSet(as_board_conn[unit][i].local_dev, as_board_conn[unit][i].local_port, 1);
		}
    }

	npd_syslog_dbg("\n******* Leaving  as_series_linecard_system_conn_init ********* \n\n");
	
}

long as_series_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
	GT_STATUS ret = 0;
	int i = 0, j = 0;
    int plane_port = -1;
    int peer_slot = 0, peer_port = 0;
    int peer_unit = 0, peer_type = 0;
    int third_slot = 0, sec_type, need_cross_trunk = 0;
	unsigned char unit = 0, mod_id = 0, port_num = 0;
	int tid_base = 0;
	int trunk_member_count = 0;
	CPSS_TRUNK_MEMBER_STC trunk_member[4];
    if(as6603_cscd_need_pcl == 1)
    {
		return as_series_linecard_cross_pcl_system_conn_deinit(product_type, delete_board_type, delete_slotid);
    }
	npd_syslog_dbg("\n******* Entering as_series_linecard_system_conn_deinit ********* \n\n");

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(port_num = 24; port_num < 28; port_num++)
		{
            cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 0);
			memset(&as_board_conn[unit][(port_num - 24)], 0, sizeof(as_board_conn_element));
			as_board_conn[unit][(port_num - 24)].local_dev = unit;
			as_board_conn[unit][(port_num - 24)].local_port = port_num;
            plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
                   unit, port_num);
            if(-1 == plane_port)
            {
                continue;
            }

			if(BOARD_INNER_CONN_PORT == plane_port)
			{
				/*board inner conn case*/
				npd_syslog_dbg("%s %d: port %d:%d is board inner conn port.\r\n", __func__, __LINE__, unit, port_num);
				if(unit == 0)
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 1, 0);
				}
				else
				{
                    as_board_conn[unit][(port_num - 24)].peer_mod = 
						UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0);
				}
				as_board_conn[unit][(port_num - 24)].peer_slot = SYS_LOCAL_MODULE_SLOT_INDEX;
				as_board_conn[unit][(port_num - 24)].peer_type = SYS_LOCAL_MODULE_TYPE;
				continue;
			}
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX,
                   plane_port);
			if(SYS_MODULE_RUNNINGSTATE(peer_slot) == RMT_BOARD_NOEXIST  || (peer_slot == delete_slotid))
			{
				npd_syslog_dbg("%s %d: Board %d is not exist. Maybe pre-setted.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			
			peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
			if(peer_type == 0)
			{
				npd_syslog_dbg("%s %d: Can not get peer module type at slot %d.\r\n", __func__, __LINE__, peer_slot);
				as_board_conn[unit][(port_num - 24)].peer_slot = -1;
				continue;
			}
			peer_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
			as_board_conn[unit][(port_num - 24)].peer_type = peer_type;
			as_board_conn[unit][(port_num - 24)].peer_slot = peer_slot;
			
			if(!SYS_MODULE_ISHAVEPP(peer_type))
			{
				/*No pp case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a Non-pp board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].is_dest_port = 1;
    			cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
    				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
    			cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);
				
				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, NOPP_PORT);
			    as_series_board_vlan_entry_del(unit, port_num);
			}
			else if(SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
			{
				/*Different SDK type case*/
				npd_syslog_dbg("%s %d: Peer module type %d, is a SDK-different board.\r\n", __func__, __LINE__, peer_type);
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

                cpssDxChBrgSrcIdPortDefaultSrcIdSet(unit, port_num, 
                				UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0));
                cpssDxChBrgSrcIdPortSrcIdForceEnableSet(unit, port_num, 1);

				as_series_board_srcid_init(unit, port_num);
				as_series_board_port_config(unit, port_num, SDKDIFF_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
			else
			{
				as_board_conn[unit][(port_num - 24)].peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
				as_board_conn[unit][(port_num - 24)].peer_port = peer_port;
				npd_syslog_dbg("%s %d: Peer mod:port is %d:%d.\r\n", __func__, __LINE__,
					as_board_conn[unit][(port_num - 24)].peer_mod, as_board_conn[unit][(port_num - 24)].peer_port);

				as_series_board_port_config(unit, port_num, DSA_PORT);
			    as_series_board_vlan_entry_add(unit, port_num);
			}
		}
    }

	
	/*处理板间级联端口*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		tid_base = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			/*上面的环节已经处理过了*/
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
				tid_base = 1;
    			continue;
    		}
			/*上面的环节已经处理过了*/
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			for(j = 0; j < 4; j++)
			{
				if(i == j)
				{
					continue;
				}
        		if(as_board_conn[unit][j].peer_slot == -1 || 
    				as_board_conn[unit][j].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][j].peer_type))
        		{
        			continue;
        		}
    			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][j].peer_type))
    			{
    			    continue;
    			}
    			/*上面的环节已经处理过了*/
        		if(as_board_conn[unit][j].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
        		{
        			continue;
        		}
    			/*上面的环节已经处理过了*/
    			if(as_board_conn[unit][j].tid != 0)
    			{
    				continue;
    			}
				if(as_board_conn[unit][i].peer_slot == as_board_conn[unit][j].peer_slot)
				{
					as_board_conn[unit][i].tid = 127 - tid_base;
					as_board_conn[unit][j].tid = 127 - tid_base;
					as_board_conn[unit][i].trunk_member = 1;
					as_board_conn[unit][j].trunk_member = 1;
					tid_base += 1;
				}
			}
		}
    }

	/* 处理SDKDIFF 情况的端口*/
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(as_board_conn[unit][i].peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
    		{
    			continue;
    		}
			if(as_board_conn[unit][i].tid != 0)
			{
				continue;
			}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
				as_board_conn[unit][i].tid = SDK_DIFF_TRUNK;
				as_board_conn[unit][i].trunk_member = 1;
			}
		}
	}

	/* Debug */
	npd_syslog_dbg("\nas_board_conn info \n\n");
	npd_syslog_dbg("LocalDev  LocalPort  PeerMod  PeerPort  PeerType  PeerSlot  "
				   "TrunkMember  Tid  DestPort  SrcPort  NeedRedirect  RedirectFromPort\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		for(i = 0; i < 4; i++)
		{
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].local_dev);
			npd_syslog_dbg("%-11d", as_board_conn[unit][i].local_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].peer_mod);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_port);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_type);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].peer_slot);
			npd_syslog_dbg("%-13d", as_board_conn[unit][i].trunk_member);
			npd_syslog_dbg("%-5d", as_board_conn[unit][i].tid);
			npd_syslog_dbg("%-10d", as_board_conn[unit][i].is_dest_port);
			npd_syslog_dbg("%-9d", as_board_conn[unit][i].is_src_port);
			npd_syslog_dbg("%-14d", as_board_conn[unit][i].need_redirect);
			npd_syslog_dbg("%-16d\n", as_board_conn[unit][i].redirect_from_port);
		}
	}
	
    /*trunk*/
	npd_syslog_dbg("\ntrunk config\n");
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			if(as_board_conn[unit][i].trunk_member == 1)
			{
				for(j = (i + 1); j < 4; j++)
				{
					if(as_board_conn[unit][i].tid == as_board_conn[unit][j].tid)
					{
						npd_syslog_dbg("devNum = %d	tid = %d	portNumA = %d	portNumB = %d\n", 
							unit, as_board_conn[unit][i].tid, 
							as_board_conn[unit][i].local_port,
							as_board_conn[unit][j].local_port);
						ax63ge48_trunk_port_config(unit, as_board_conn[unit][i].tid,
							as_board_conn[unit][i].local_port, 
							as_board_conn[unit][j].local_port,
							0, 0, 0, 0, 0, 0);
					}
				}
			}
		}
    }

	/* SDKDifferent trunk config */
	/* 对于sdkdiff的情况，要去轮询对端的板子，根据对端为SDKDIFF的端口
	    反向找到要加入trunk的成员*/
	npd_syslog_dbg("\nsdkDifferent trunk config\n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		trunk_member_count = 0;
		memset(trunk_member, 0, sizeof(trunk_member));
		
		for(i = 0; i < 4; i++)
		{
			if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
			/*NoPP的情况不往下走*/
			if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    for(j = 0; j < PPAL_PLANE_PORT_COUNT(as_board_conn[unit][i].peer_type); j++)
			    {
			        if(j >= PPAL_PLANE_PORT_COUNT(SYS_LOCAL_MODULE_TYPE))
			        {
			            break;
			        }
                    peer_slot = SLOT_PORT_PEER_SLOT(as_board_conn[unit][i].peer_slot, j);
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
					
					if(SYS_MODULE_SDK_DIFFERENT(peer_type, as_board_conn[unit][i].peer_type)
						&& !SYS_MODULE_SDK_DIFFERENT(peer_type, SYS_LOCAL_MODULE_TYPE))
					{
					    GT_TRUNK_ID trunk_id = 0, tmp = 0;
						GT_U8 peer_mod = 0;
				        int find_port = 0;
            			peer_port = SLOT_PORT_PEER_PORT(as_board_conn[unit][i].peer_slot, j);
						if(peer_port == -1)
						{
						    continue;
						}
						
            			peer_unit = PPAL_PLANE_2_UNIT(peer_type, peer_port);						
            			peer_port = PPAL_PLANE_2_PORT(peer_type, peer_port);
						peer_mod = UNIT_2_MODULE(peer_type, peer_slot, peer_unit, 0);
						for(tmp = 0; tmp < trunk_member_count; tmp++)
						{
							if(trunk_member[tmp].device == peer_mod &&
								trunk_member[tmp].port == peer_port)
							{
								find_port = 1;
								break;
							}
						}

						if(!find_port)
						{
							trunk_member[trunk_member_count].device = peer_mod;
							trunk_member[trunk_member_count].port = peer_port;
							ret = cpssDxChTrunkDbIsMemberOfTrunk(unit, &trunk_member[trunk_member_count], &trunk_id);
							if(ret == 0 && trunk_id == SDK_DIFF_TRUNK)
							{
								npd_syslog_dbg("Remove: unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							    cpssDxChTrunkMemberRemove(unit, SDK_DIFF_TRUNK, &trunk_member[trunk_member_count]);
							}
							npd_syslog_dbg("unit = %d	modid = %d	port = %d\r\n", unit, peer_mod, peer_port);
							trunk_member_count++; 
						}
					}
			    }
			}
		}
		if(trunk_member_count)
		{
		    npd_syslog_dbg("trunk_member_count = %d\r\n", trunk_member_count);
            cpssDxChTrunkMembersSet(unit, SDK_DIFF_TRUNK, trunk_member_count, trunk_member, 0, NULL);
		}
	}

	npd_syslog_dbg("\nsrc-id config\n");
	/*multi-destination flow. (src-id)*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		int srcid = 0;
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
			{
			    continue;
			}
			for(srcid = 0; srcid < 6; srcid++)
			{
			    if(as_board_conn[unit][i].peer_mod == srcid)
			    {
			        continue;
			    }
				/*来自本单元*/
				if(srcid == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
				{
					npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
						srcid, as_board_conn[unit][i].local_port);
				    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
				}
				else
				{
					/*来自本板另一个单元*/
				    if(MOD_ID_TO_SLOT_INDEX(srcid) == SYS_LOCAL_MODULE_SLOT_INDEX)
    				{
    				    int another_unit = MODULE_2_UNIT(SYS_LOCAL_MODULE_TYPE, srcid);
    					int need_dest = 1;
    					if(another_unit == -1)
    					{
    					    continue;
    					}
    					
    					for(j = 0; j < 4; j++)
    					{
                    		if(as_board_conn[another_unit][j].peer_slot == -1 || 
                				as_board_conn[another_unit][j].peer_type == 0)
                    		{
                    			continue;
                    		}
                    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[another_unit][j].peer_type))
                    		{
                    			continue;
                    		}
                			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[another_unit][j].peer_type))
                			{
                			    continue;
                			}
         				    if(as_board_conn[another_unit][j].peer_mod == as_board_conn[unit][i].peer_mod)
         				    {
         				        need_dest = 0;
    							break;
         				    }
         				    if(as_board_conn[another_unit][j].peer_slot == as_board_conn[unit][i].peer_slot)
         				    {
         				        need_dest = 0;
    							break;
         				    }
    					}
    					if(need_dest == 1)
    					{
    						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
    							srcid, as_board_conn[unit][i].local_port);
    					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
    					}
						else
						{
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
						}
    				}
				    else
    				{
						/*来自其它单板*/
    					int need_dest = 1;
    				    if(nam_asic_get_instance_num() == 1)
    				    {
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
    				        continue;
    				    }
    					if(as_board_conn[unit][i].peer_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
    					{
							cpssDxChBrgSrcIdGroupPortDelete(unit, srcid, as_board_conn[unit][i].local_port);
    					    continue;
    					}
    					if(need_dest == 1)
    					{
    						npd_syslog_dbg("devNum = %d	srcid = %d	portNum = %d\n", unit, 
    							srcid, as_board_conn[unit][i].local_port);
    					    cpssDxChBrgSrcIdGroupPortAdd(unit, srcid, as_board_conn[unit][i].local_port);
    					}
    				}
				}
			}
		}
    }

	npd_syslog_dbg("\ndev map config\n");
	/*dev map*/
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        CPSS_CSCD_LINK_TYPE_STC      cascadeLinkPtr;
		int dest_mod = 0;
		for(dest_mod = 0; dest_mod < 6; dest_mod++)
		{
			int trunk_connect = 0;
			int port_connect = 0;
			if(dest_mod == UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0))
			{
				continue;
			}
    		for(i = 0; i < 4; i++)
    		{
        		if(as_board_conn[unit][i].peer_slot == -1 || 
    				as_board_conn[unit][i].peer_type == 0)
        		{
        			continue;
        		}
        		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
        		{
        			continue;
        		}
    			if(SYS_MODULE_SDK_DIFFERENT(SYS_LOCAL_MODULE_TYPE, as_board_conn[unit][i].peer_type))
    			{
    			    continue;
    			}
    			if(as_board_conn[unit][i].trunk_member && dest_mod == as_board_conn[unit][i].peer_mod)
    			{
					trunk_connect = 1;
					break;
    			}
    			else
    			{
    				if(dest_mod == as_board_conn[unit][i].peer_mod)
    				{
						port_connect = as_board_conn[unit][i].local_port;
    				}
    			}
    		}
			if(trunk_connect)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = as_board_conn[unit][i].tid;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, as_board_conn[unit][i].tid);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else if(port_connect != 0)
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    			cascadeLinkPtr.linkNum = port_connect;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPE_PORT	linkNum = %d\n", unit, 
					dest_mod, port_connect);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
			else
			{
    			cascadeLinkPtr.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    			cascadeLinkPtr.linkNum = 127;
				npd_syslog_dbg("devNum = %d	targetDevNum = %d	linkType = LINK_TYPD_TRUNK	linkNum = %d\n", unit, 
					dest_mod, 127);
    		    cpssDxChCscdDevMapTableSet(unit, dest_mod, 0, &cascadeLinkPtr, 1);
			}
		}
    }

	npd_syslog_dbg("\nEnable Port \n");
	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		for(i = 0; i < 4; i++)
		{
    		if(as_board_conn[unit][i].peer_slot == -1 || 
				as_board_conn[unit][i].peer_type == 0)
    		{
    			continue;
    		}
    		if(!SYS_MODULE_ISHAVEPP(as_board_conn[unit][i].peer_type))
    		{
    			continue;
    		}
			cpssDxChPortEnableSet(as_board_conn[unit][i].local_dev, as_board_conn[unit][i].local_port, 1);
		}
    }

	npd_syslog_dbg("\n******* Leaving  as_series_linecard_system_conn_deinit ********* \n\n");
	
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
#ifdef __cplusplus
}
#endif

