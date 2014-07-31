
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dhcp_snp_options.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		dhcp snooping options for NPD module.
*
* DATE:
*		06/04/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef HAVE_DHCP_SNP
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_dhcp_snp_options.h"


/**********************************************************************************
 * npd_dhcp_snp_opt82_enable
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
unsigned int npd_dhcp_snp_opt82_enable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = npd_dhcp_snp_set_opt82_status(NPD_DHCP_SNP_OPT82_ENABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping option82 status enable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_opt82_disable
 *		set DHCP_Snooping enable option82 status, and init 
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
unsigned int npd_dhcp_snp_opt82_disable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = npd_dhcp_snp_set_opt82_status(NPD_DHCP_SNP_OPT82_DISABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping option82 status disable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_get_opt82_status
 *		get DHCP_Snooping enable/disable option82 status
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
unsigned int npd_dhcp_snp_get_opt82_status
(
	unsigned char *status
)
{
	if (!status)
	{
		syslog_ax_dhcp_snp_err("get DHCP-Snooping option82 status error, parameters is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
	{
		*status = user.dhcp_snp_opt82_enable;
	}
    else
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }
    
    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_set_opt82_status
 *		set DHCP_Snooping enable/disable option82 status
 *
 *	INPUT:
 *		unsigned char isEnable
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_snp_set_opt82_status
(
	unsigned char isEnable
)
{
	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user, 0, sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

    if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_global_cfg_get(&user))
	{
		user.dhcp_snp_opt82_enable = isEnable ? NPD_DHCP_SNP_OPT82_ENABLE : NPD_DHCP_SNP_OPT82_DISABLE;
        return npd_dhcp_snp_global_cfg_set(&user);
	}
    else
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }
	
	return DHCP_SNP_RETURN_CODE_OK;
}


void npd_dhcp_snp_print_port_func_data
(
	unsigned int g_eth_index,
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *dhcp_snp_info
)
{
	syslog_ax_dhcp_snp_dbg("==============dhcp snp func data===============\n");
	syslog_ax_dhcp_snp_dbg("eth-port %d\n", g_eth_index);
	syslog_ax_dhcp_snp_dbg("trust mode %s\n",
							(dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
							((dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
							((dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" :
							"unknow")));

	syslog_ax_dhcp_snp_dbg("strategy mode %s\n",
							(dhcp_snp_info->opt82_strategy == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
							((dhcp_snp_info->opt82_strategy  == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
							((dhcp_snp_info->opt82_strategy  == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
							"unknow")));

	syslog_ax_dhcp_snp_dbg("circuit-id mode %s\n",
							(dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
							((dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
							"unknow"));
	syslog_ax_dhcp_snp_dbg("           content [%s]\n",
							dhcp_snp_info->opt82_circuitid_str);

	syslog_ax_dhcp_snp_dbg("===============================================\n");

	return ;
}

/**********************************************************************************
 * npd_dhcp_snp_set_opt82_port_strategy
 *		set strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char strategy_mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set strategy mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_set_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char strategy_mode
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT itemA;
	memset(&itemA,0,sizeof(itemA));

	itemA.global_port_ifindex = g_eth_index;

    syslog_ax_dhcp_snp_dbg("set circuitid: port 0x%x, circuitid type %d.\n", g_eth_index, strategy_mode);
        
	if(npd_dhcp_snp_status_item_find(&itemA))
	{
		syslog_ax_dhcp_snp_dbg("Can't find the item! 0x%2x!", itemA.global_port_ifindex);
        itemA.opt82_strategy = strategy_mode;
		npd_dhcp_snp_status_item_insert(&itemA);
        
		return DHCP_SNP_RETURN_CODE_OK;
	}
	else if(itemA.opt82_strategy == strategy_mode)
	{
	    return DHCP_SNP_RETURN_CODE_OPTION82_ATTRIBUTE_ALREADY_SET;
	}
	else
	{
		itemA.opt82_strategy = strategy_mode;

        return npd_dhcp_snp_status_item_refresh(&itemA, &itemA);
	}
}
/**********************************************************************************
 * npd_dhcp_snp_get_opt82_port_strategy
 *		get strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char isTagged,
 *
 *	OUTPUT:
 *		unsigned int *strategy_mode
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_get_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *strategy_mode
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT item;

    memset(&item, 0, sizeof(item));

    item.global_port_ifindex = g_eth_index;
    if (0 == npd_dhcp_snp_status_item_find(&item))
    {
        *strategy_mode = item.opt82_strategy;
    }
    else
    {
        *strategy_mode = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE;
    }

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_set_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82  on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char circuitid_mode,
 *		char *circuitid_content
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set circuit-id mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_set_opt82_port_circuitid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char circuitid_mode,
	char *circuitid_content
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT itemA;
	
	memset(&itemA,0,sizeof(itemA));
	itemA.global_port_ifindex = g_eth_index;

	syslog_ax_dhcp_snp_dbg("set circuitid: port 0x%x, tag %d, circuitid type %d, circuitid str %s.\n",
														g_eth_index, tagMode, circuitid_mode, circuitid_content);
	if(npd_dhcp_snp_status_item_find(&itemA))
	{
		syslog_ax_dhcp_snp_dbg("Can't find the item! 0x%2x!", itemA.global_port_ifindex);
		
		itemA.opt82_circuitid = circuitid_mode;
		strncpy(itemA.opt82_circuitid_str,circuitid_content,NPD_DHCP_SNP_CIRCUITID_STR_LEN);
        /*itemA.tagMode = tagMode;*/
		npd_dhcp_snp_status_item_insert(&itemA);		
		return DHCP_SNP_RETURN_CODE_OK;
	}
	else
	{
		if((!strncmp(itemA.opt82_circuitid_str,circuitid_content,NPD_DHCP_SNP_CIRCUITID_STR_LEN))&&
			(strlen(itemA.opt82_circuitid_str) == strlen(circuitid_content)) &&
			(itemA.opt82_circuitid == circuitid_mode)
		   )
		{
			syslog_ax_dhcp_snp_err("set circuitid: CircuitId is same on port 0x%x.\n", g_eth_index);
			return DHCP_SNP_RETURN_CODE_OPTION82_ATTRIBUTE_ALREADY_SET;
		}	
		else
		{
			memset(itemA.opt82_circuitid_str, 0, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
			strncpy(itemA.opt82_circuitid_str,circuitid_content,NPD_DHCP_SNP_CIRCUITID_STR_LEN);
			itemA.opt82_circuitid = circuitid_mode;
			npd_dhcp_snp_status_item_refresh(&itemA, &itemA);

			return DHCP_SNP_RETURN_CODE_OK;
		}
	}		

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_get_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *circuitid_mode,
 *		char *circuitid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_get_opt82_port_circuitid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *circuitid_mode,
	unsigned char *circuitid_content
)
{
	unsigned int get_flag = NPD_DHCP_SNP_INIT_0;
	unsigned int tmp_port_index = g_eth_index;
	unsigned int tmp_circuitid_mode = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	unsigned int content_len = NPD_DHCP_SNP_INIT_0;
	
	NPD_DHCP_OPT82_SUBOPT_T subopt;
	NPD_DHCP_OPT82_CIRCUITID_T circuitid;
	NPD_DHCP_OPT82_CIRCUITID_DATA_T circuitid_data;

	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT item;
	memset(&item,0,sizeof(item));
	
	item.global_port_ifindex = g_eth_index;

    if (DHCP_SNP_RETURN_CODE_OK != npd_dhcp_snp_global_cfg_get(&user))
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }

	if (!circuitid_content || !circuitid_mode) {
		syslog_ax_dhcp_snp_err("get option 82 circuit-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Circuit-ID mode\n",
							vlanid, g_eth_index,
							tagMode ? "tagged" : "untagged");

	memset(&subopt, 0, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	memset(&circuitid, 0, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
	memset(&circuitid_data, 0, sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T));

	if(!npd_dhcp_snp_status_item_find(&item)){
		syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Circuit-ID mode %s content [%s] success.\n",
					vlanid, g_eth_index,
					tagMode ? "tagged" : "untagged",  
					(item.opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
					((item.opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
					"unknow"),item.opt82_circuitid_str);
		get_flag = 1;/* found */
	}


	if (1 != get_flag) {
		syslog_ax_dhcp_snp_dbg("Can't find the interface table item user default!");
		tmp_circuitid_mode = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT;
	}
	else
	{
		tmp_circuitid_mode = item.opt82_circuitid;

		syslog_ax_dhcp_snp_dbg("get the interface table item !");
		npd_dhcp_snp_print_port_func_data(g_eth_index, &item);
	}
	/*
	           0                      7                     15                   23                  31
	            --------------------------------------------------------
	           | suboption_type | suboption_len  | circuit-id_type| circuit-id_len |
	            --------------------------------------------------------
	    data |                 vlanid                     |           port-index                |
	            --------------------------------------------------------
	    note:
	        1. if fill format is standard, field of  circuit-id_type and circuit-id_len are miss
	        2. if data is default, length is 2 + 2
	            if data is user-defined, length is 64
	*/
	/* get port Circuit-ID mode and Circuit-ID content */
	if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR == tmp_circuitid_mode)
	{

		syslog_ax_dhcp_snp_dbg("NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR == tmp_circuitid_mode");
		/* use-defined use ascii format */
		circuitid.circuit_type = 0x1;
		circuitid.circuit_len = NPD_DHCP_SNP_CIRCUITID_STR_LEN;

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
		{	/* add field of circuit-id_type and circuit-id_len */
			memcpy(circuitid_content + 2, &circuitid, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_T);

		}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == user.dhcp_snp_opt82_fill_format_type)
		{	/* not need add field of circuit-id_type and circuit-id_len */
			syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");		
		}
		
		/* add user-defined string */
		memcpy(circuitid_content + 2 + content_len, item.opt82_circuitid_str, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
		content_len += sizeof(NPD_DHCP_OPT82_COM_DATA_T);

		subopt.subopt_type = DHCP_CIRCUIT_ID;
		subopt.subopt_len = content_len;
		memcpy(circuitid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	}
	else if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT == tmp_circuitid_mode)
	{
		syslog_ax_dhcp_snp_dbg("NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT == tmp_circuitid_mode");
		/* default use hex format */
		circuitid.circuit_type = 0x0;
//		circuitid.circuit_len = 0x4;
        circuitid.circuit_len = sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T);

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
		{	/* add field of circuit-id_type and circuit-id_len */
			memcpy(circuitid_content + 2, &circuitid, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_T);

		}
		else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == user.dhcp_snp_opt82_fill_format_type)
		{	/* not need add field of circuit-id_type and circuit-id_len */
			syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
		}

		if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX == user.dhcp_snp_opt82_format_type)
		{
			syslog_ax_dhcp_snp_dbg("NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX == dhcp_snp_opt82_format_type");
		
			/* add default data use in hex */
			circuitid_data.vlanid = htons(vlanid);
			circuitid_data.portindex = htonl(tmp_port_index);
			memcpy(circuitid_content + 2 + content_len, &circuitid_data, sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T);
		}
		else if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == user.dhcp_snp_opt82_format_type)
		{
			syslog_ax_dhcp_snp_dbg("NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == dhcp_snp_opt82_format_type");
		    int tmp_len = 0;

            tmp_len = content_len;
            content_len += sprintf(circuitid_content + 2 + content_len, "%02d", vlanid);
            content_len += sprintf(circuitid_content + 2 + content_len, "%02d", tmp_port_index);

			/* modify circuit-id len, becase the len is not 4 byte at this time */
			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
			{
				NPD_DHCP_OPT82_CIRCUITID_T *tmp_circuitid = NULL;
				tmp_circuitid = (NPD_DHCP_OPT82_CIRCUITID_T *)(circuitid_content + 2);
				tmp_circuitid->circuit_type = 0x1; 
				tmp_circuitid->circuit_len = content_len - tmp_len;
			}
		}

		subopt.subopt_type = DHCP_CIRCUIT_ID;
		subopt.subopt_len = content_len;
		memcpy(circuitid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	}

	*circuitid_mode = tmp_circuitid_mode;

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_get_opt82_port_remoteid
 *		get remoteid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *remoteid_mode,
 *		char *remoteid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- error
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int npd_dhcp_snp_get_opt82_port_remoteid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *remoteid_mode,
	unsigned char *remoteid_content
)
{
	unsigned int get_flag = NPD_DHCP_SNP_INIT_0;
	unsigned int tmp_remoteid_mode = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned int content_len = NPD_DHCP_SNP_INIT_0;


	NPD_DHCP_OPT82_SUBOPT_T subopt;
	NPD_DHCP_OPT82_REMOTEID_T remoteid;

	struct NPD_DHCP_SNP_GLOBAL_STATUS user;
	memset(&user,0,sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS));

	if (!remoteid_content || !remoteid_mode) {
		syslog_ax_dhcp_snp_err("get option 82 remote-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Remote-ID mode\n",
							vlanid, g_eth_index,
							tagMode ? "tagged" : "untagged");

	memset(&subopt, 0, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	memset(&remoteid, 0, sizeof(NPD_DHCP_OPT82_REMOTEID_T));

    get_flag = npd_dhcp_snp_global_cfg_get(&user);

	if(get_flag == DHCP_SNP_RETURN_CODE_OK)
	{
		tmp_remoteid_mode = user.dhcp_snp_opt82_remoteid_type;
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanid);
		
		if (tmp_remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC)
		{	/* default use hex format */
			remoteid.remote_type = 0x0;
			remoteid.remote_len = 0x6;
			syslog_ax_dhcp_snp_dbg("A tmp_remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC");

			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			
			}
            else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == user.dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}

			if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX == user.dhcp_snp_opt82_format_type)
			{
				memcpy(remoteid_content + 2 + content_len, PRODUCT_MAC_ADDRESS, 6);
				content_len += 6;

			}
			else if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == user.dhcp_snp_opt82_format_type)
			{
				content_len += sprintf(remoteid_content + 2 + content_len, "%.2x%.2x%.2x%.2x%.2x%.2x",
	        	   PRODUCT_MAC_ADDRESS[0], PRODUCT_MAC_ADDRESS[1], PRODUCT_MAC_ADDRESS[2], PRODUCT_MAC_ADDRESS[3],
	        	   PRODUCT_MAC_ADDRESS[4], PRODUCT_MAC_ADDRESS[5]);;

				/* modify remote-id len, becase the len is not 6 byte at this time */
				if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
				{
					NPD_DHCP_OPT82_REMOTEID_T *tmp_remoteid = NULL;
					tmp_remoteid = (NPD_DHCP_OPT82_REMOTEID_T *)(remoteid_content + 2);
					tmp_remoteid->remote_type = 0x1; /* ascii */
					tmp_remoteid->remote_len = 12;
				}
			}
		}
		else
		{
			syslog_ax_dhcp_snp_dbg("A tmp_remoteid_mode != NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC");

			/* user-defined on port value, ascii */
			remoteid.remote_type = 0x1;
			remoteid.remote_len = 0x40; /* 64 byte */

			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == user.dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			}
            else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == user.dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}
			
			memcpy(remoteid_content + 2 + content_len, user.dhcp_snp_opt82_remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			content_len += NPD_DHCP_SNP_REMOTEID_STR_LEN;
		}
    	subopt.subopt_type = DHCP_REMOTE_ID;
		subopt.subopt_len = content_len;
		memcpy(remoteid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	}
	else
	{
		syslog_ax_dhcp_snp_dbg("get remote information error!");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	*remoteid_mode = tmp_remoteid_mode;

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_check_opt82
 *		check the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_FOUND		- have the option
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- not have the option
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int npd_dhcp_snp_check_opt82
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

	if (!packet) {
		syslog_ax_dhcp_snp_err("remove option82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
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
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
	
        if ((optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
            && (DHCP_PADDING != optionptr[i + NPD_DHCP_SNP_OPT_CODE])
                    && (DHCP_END != optionptr[i + NPD_DHCP_SNP_OPT_CODE]))
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			
			syslog_ax_dhcp_snp_dbg("found option code %x.", code);
			return DHCP_SNP_RETURN_CODE_FOUND;
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
					return DHCP_SNP_RETURN_CODE_NOT_FOUND;
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

	syslog_ax_dhcp_snp_dbg("not found option code %x.", code);
	return DHCP_SNP_RETURN_CODE_NOT_FOUND;
}

/**********************************************************************************
 * npd_dhcp_snp_remove_opt82
 *		remove option 82 the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned int opt_len
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_remove_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int opt_len,
	unsigned int *del_len
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned char *optionptr = NULL;
	syslog_ax_dhcp_snp_dbg("remove the option 82\n");

	if (!packet) {
		syslog_ax_dhcp_snp_err("remove option82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;
	syslog_ax_dhcp_snp_dbg("1 optionptr->len = %d,\n",optionptr[NPD_DHCP_SNP_OPT_LEN]);

	while (!done)
	{
		if ((i >= length)
            || (((DHCP_PADDING != optionptr[i + NPD_DHCP_SNP_OPT_CODE])
                    && (DHCP_END != optionptr[i + NPD_DHCP_SNP_OPT_CODE]))
                    && (i + 2 >= length)))
		{
			syslog_ax_dhcp_snp_dbg("bogus packet, option fields too long.");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
        
        if ((optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
            && (DHCP_PADDING != optionptr[i + NPD_DHCP_SNP_OPT_CODE])
                && (DHCP_END != optionptr[i + NPD_DHCP_SNP_OPT_CODE]))
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_dbg("bogus packet, option fields too long.");
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			*del_len = optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2;
            memmove(optionptr + i, optionptr + (i + optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2), (opt_len - i - *del_len + 1));
					
			syslog_ax_dhcp_snp_dbg("del op 82 suc.");

			return DHCP_SNP_RETURN_CODE_OK;
		}

		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			case DHCP_PADDING:
				i++;
				syslog_ax_dhcp_snp_dbg("DHCP_PADDING.");
				break;
		/*
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				over = optionptr[i + NPD_DHCP_SNP_OPT_DATA];
				i += optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2;
				syslog_ax_dhcp_snp_err("DHCP_OPTION_OVER.");
				break;
			*/
			case DHCP_END:
				/*if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD)
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
				syslog_ax_dhcp_snp_dbg("3 optionptr->len = %d,\n",optionptr[NPD_DHCP_SNP_OPT_LEN]);
				*/
				syslog_ax_dhcp_snp_dbg("DHCP_END.\n");
					done = 1;
					break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
				break;
		}
	}

	//*del_len = NPD_DHCP_SNP_INIT_0;
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_cat_opt82_sting
 *		cat option 82 string with Circuit-ID sting and Remote-ID string.
 *
 *	INPUT:
 *		char *circuitid_str,
 *		char *remoteid_str,
 *
 *	OUTPUT:
 *		char *opt82_str,
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_cat_opt82_sting
(
	unsigned char *circuitid_str,
	unsigned char *remoteid_str,
	unsigned char *opt82_str
)
{
	unsigned int opt82_len = NPD_DHCP_SNP_INIT_0;	
	
	if (!opt82_str || !circuitid_str || !remoteid_str) {
		syslog_ax_dhcp_snp_err("cat option82 string error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	opt82_str[NPD_DHCP_SNP_OPT_CODE] = DHCP_OPTION_82;
	
	syslog_ax_dhcp_snp_dbg("1 opt82_len %d\n",opt82_len);
	
	memcpy(opt82_str + 2, circuitid_str, circuitid_str[NPD_DHCP_SNP_OPT_LEN] + 2);
	opt82_len += circuitid_str[NPD_DHCP_SNP_OPT_LEN] + 2;
	
	syslog_ax_dhcp_snp_dbg("2 opt82_len %d circuitid_str[NPD_DHCP_SNP_OPT_LEN]%02X\n",opt82_len,circuitid_str[NPD_DHCP_SNP_OPT_LEN]);
	memcpy(opt82_str + 2 + opt82_len, remoteid_str, remoteid_str[NPD_DHCP_SNP_OPT_LEN] + 2);
	opt82_len += remoteid_str[NPD_DHCP_SNP_OPT_LEN] + 2;
	syslog_ax_dhcp_snp_dbg("3 opt82_len %d remoteid_str[NPD_DHCP_SNP_OPT_LEN] %02X\n",opt82_len,remoteid_str[NPD_DHCP_SNP_OPT_LEN]);
	
	syslog_ax_dhcp_snp_err("cat opt82 str %d,~~~~%02X",opt82_len,opt82_len);
	opt82_str[NPD_DHCP_SNP_OPT_LEN] = opt82_len;
	syslog_ax_dhcp_snp_dbg("4 opt82_len %d\n",opt82_len);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_attach_opt82_string
 *		attach option 82 string to the packet with bounds checking (warning, not aligned).
 *		add an option string to the options (an option string contains an option code, * length, then data) 
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned char *string
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/

unsigned int npd_dhcp_snp_attach_opt82_string
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned char *string
)
{
	unsigned int end = NPD_DHCP_SNP_INIT_0;
	unsigned char *optionptr = NULL;

	if (!packet || !string) {
		syslog_ax_dhcp_snp_err("attach option82 string error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	end = npd_dhcp_snp_end_option(optionptr);

	syslog_ax_dhcp_snp_dbg("optionptr end %d  %02x-%02x-%02x-%02x-%02x",end ,optionptr[end-2],optionptr[end-1]
							,optionptr[end],optionptr[end+1],optionptr[end+2]);

	syslog_ax_dhcp_snp_dbg("string %02x-%02x-%02x-%02x-%02x",string[end],string[end+1]
						,string[end+2],string[end+3],string[end+4]);
	syslog_ax_dhcp_snp_dbg("NPD_DHCP_SNP_OPT_LEN  %d",end + 2 + 1);
	syslog_ax_dhcp_snp_dbg("string %s!\n",string);
	syslog_ax_dhcp_snp_dbg("string %02x!\n",string[NPD_DHCP_SNP_OPT_LEN]);

	/* end position + string length + option code/length + end option */
	if (end + string[NPD_DHCP_SNP_OPT_LEN] + 2 + 1 >= NPD_DHCP_SNP_OPTION_LEN) {
		syslog_ax_dhcp_snp_err("Option 0x%02x did not fit into the packet!\n", string[NPD_DHCP_SNP_OPT_CODE]);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("adding option 0x%02x\n", string[NPD_DHCP_SNP_OPT_CODE]);

	/* copy */
	memcpy(optionptr + end, string, string[NPD_DHCP_SNP_OPT_LEN] + 2);

	optionptr[end + string[NPD_DHCP_SNP_OPT_LEN] + 2] = DHCP_END;
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_snp_end_option
 *		return the position of the 'end' option (no bounds checking) 
 *
 *	INPUT:
 *		unsigned char *optionptr
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		unsigned int		- end position of the option
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_end_option
(
	unsigned char *optionptr
) 
{
	int i = NPD_DHCP_SNP_INIT_0;
	
	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) {
			i++;
		}else {
			i += optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2;
		}
	}

	return i;
}

/**********************************************************************************
 * npd_dhcp_snp_attach_opt82
 *		attach DHCP_Snooping option82 to dhcp packet
 *
 *	INPUT:
 *		unsigned char *packet
 *		unsigned int ifindex,
 *		unsigned char isTagged,
 *		unsigned short vlanid
 *
 *	OUTPUT:
 *		unsigned long *bufflen		- packet len
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dhcp_snp_attach_opt82
(
	unsigned char *packet,
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vlanid,
	unsigned long *bufflen
)
{

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char strategy_mode = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID;
	unsigned char circuitid_type = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	unsigned char circuitid_str[NPD_DHCP_SNP_CIRCUITID_STR_LEN + 4] = {0};
	unsigned char remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned char remoteid_str[NPD_DHCP_SNP_REMOTEID_STR_LEN + 4] = {0};
	unsigned char opt82_str[140] = {0};
	
	NPD_DHCP_MESSAGE_T *dhcp = NULL;
	struct iphdr* ip = NULL;
	struct udphdr* udp = NULL;
	struct iphdr tempip;
	
	unsigned int endposition = NPD_DHCP_SNP_INIT_0;
	unsigned int del_len = NPD_DHCP_SNP_INIT_0;
	syslog_ax_dhcp_snp_dbg("attach opt82!\n");
	if (!packet || !bufflen) {
		syslog_ax_dhcp_snp_err("attach option 82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

    memset(&circuitid_str, 0, sizeof(circuitid_str));
    memset(&remoteid_str, 0, sizeof(remoteid_str));
	memset(&tempip, 0, sizeof(struct iphdr));
	ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
	udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr)); 
	dhcp = (NPD_DHCP_MESSAGE_T *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));

	ret = npd_dhcp_snp_get_opt82_port_strategy(vlanid, ifindex, isTagged, &strategy_mode);
	/* get success */
	syslog_ax_dhcp_snp_dbg("attach opt82 step 1  !\n");
	
	
    switch (strategy_mode)
	{
        case NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE:
        {
    		syslog_ax_dhcp_snp_dbg("get option 82 port strategy is replace.\n");

    		/*
    		**********************
    		*  get Circuit-ID string
    		**********************
    		*/
    		ret = npd_dhcp_snp_get_opt82_port_circuitid(ifindex, isTagged, vlanid, &circuitid_type, circuitid_str);
    		if (DHCP_SNP_RETURN_CODE_OK != ret) {
    			syslog_ax_dhcp_snp_dbg("get option 82 port Circuit-ID error, ret %x\n", ret);
    			return DHCP_SNP_RETURN_CODE_ERROR;
    		}
    		syslog_ax_dhcp_snp_dbg("circuitid_str len %02X,%02X,%02X,%02X~~~,circuitid_str %s !\n",circuitid_str[0],circuitid_str[1],circuitid_str[2],circuitid_str[3],circuitid_str+4);
    		/*
    		**********************
    		* get Remote-ID string 
    		**********************
    		*/
    		ret = npd_dhcp_snp_get_opt82_port_remoteid(ifindex, isTagged, vlanid, &remoteid_type, remoteid_str);
    		if (DHCP_SNP_RETURN_CODE_OK != ret) {
    			syslog_ax_dhcp_snp_dbg("get option 82 port Remote-ID error, ret %x\n", ret);
    			return DHCP_SNP_RETURN_CODE_ERROR;
    		}
    		syslog_ax_dhcp_snp_dbg("remoteid_str len %02X,%02X,%02X,%02X~~~, remoteid_str %s !\n",remoteid_str[0],remoteid_str[1],remoteid_str[2],remoteid_str[3],remoteid_str+4);
    		/*
    		*******************************
    		* cat string of Circuit-ID and Remote-ID
    		*******************************
    		*/
    		ret = npd_dhcp_snp_cat_opt82_sting(circuitid_str, remoteid_str, opt82_str);
    		syslog_ax_dhcp_snp_dbg("opt82_str len %02X, %02X  opt82_str %s !\n",opt82_str[0],opt82_str[1],opt82_str+2);
    		if (DHCP_SNP_RETURN_CODE_OK != ret) {
    			syslog_ax_dhcp_snp_dbg("cat option 82 string of Circuit-ID and Remote-ID error, ret %x\n", ret);
    			return DHCP_SNP_RETURN_CODE_ERROR;
    		}

            switch (npd_dhcp_snp_check_opt82(dhcp, DHCP_OPTION_82))
            {
        		case DHCP_SNP_RETURN_CODE_FOUND:
        		{
        			/* 1: remove origin option 82*/
        			endposition = npd_dhcp_snp_end_option(dhcp->options);
        			ret = npd_dhcp_snp_remove_opt82(dhcp, DHCP_OPTION_82, endposition, &del_len);
        			if (DHCP_SNP_RETURN_CODE_OK != ret) {
        				syslog_ax_dhcp_snp_dbg("remove option 82 error, ret %x\n", ret);
        				return DHCP_SNP_RETURN_CODE_ERROR;
        			}
                    /* XXX: do not_found step right now! So not have break. */
        		}
        		case DHCP_SNP_RETURN_CODE_NOT_FOUND:
                {
        			/* 2: attach local option 82*/
        			ret = npd_dhcp_snp_attach_opt82_string(dhcp, DHCP_OPTION_82, opt82_str);
        			if (DHCP_SNP_RETURN_CODE_OK != ret)
        			{
        				syslog_ax_dhcp_snp_dbg("attach new option 82 string error, ret %x\n", ret);
        				return DHCP_SNP_RETURN_CODE_ERROR;
        			}
                    break;
        		}
        		default :
        		{
        			syslog_ax_dhcp_snp_dbg("check option 82 error, ret %x\n", ret);
        			return DHCP_SNP_RETURN_CODE_ERROR;
        		}
            }

    		/*
    		***************************
    		*modify packet ip upd head
    		***************************
    		*/
    		udp->len = htons(ntohs(udp->len) - del_len + opt82_str[NPD_DHCP_SNP_OPT_LEN] + 2);
    		ip->tot_len = htons(ntohs(udp->len) + sizeof(struct iphdr));
    		*bufflen = *bufflen - del_len + opt82_str[NPD_DHCP_SNP_OPT_LEN] + 2;
    		
    		npd_dhcp_information_checksum(ip,udp);
    		syslog_ax_dhcp_snp_dbg("ipcheck =  %d ,udpcheck= %d !\n",ip->check,udp->check);
            break;
    	}
        case NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP:
        {
            syslog_ax_dhcp_snp_dbg("get option 82 port strategy is keep\n");
            break;
        }
        case NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP:
	    {
    		ret = npd_dhcp_snp_check_opt82(dhcp, DHCP_OPTION_82);
    		if (DHCP_SNP_RETURN_CODE_FOUND == ret)
    		{
    			syslog_ax_dhcp_snp_dbg("get option 82 port strategy is drop\n");
    		//	return DHCP_SNP_RETURN_CODE_PKT_DROP;
        		endposition = npd_dhcp_snp_end_option(dhcp->options);
    			ret = npd_dhcp_snp_remove_opt82(dhcp, DHCP_OPTION_82, endposition, &del_len);
                if (DHCP_SNP_RETURN_CODE_OK != ret)
                {
    				syslog_ax_dhcp_snp_dbg("remove option 82 error, ret %x\n", ret);
    				return DHCP_SNP_RETURN_CODE_ERROR;
    			}

                udp->len = htons(ntohs(udp->len) - del_len);
                ip->tot_len = htons(ntohs(udp->len) + sizeof(struct iphdr));
                *bufflen = *bufflen - del_len;
    			npd_dhcp_information_checksum(ip,udp);
    		}
            break;
        }
        default :
        {
            syslog_ax_dhcp_snp_dbg("get option 82 port strategy is default\n");
            break;
        }
	}
	
	return DHCP_SNP_RETURN_CODE_OK;
}


unsigned int npd_dhcp_packet_information_process
(
	unsigned short vid, 
	unsigned int ifindex, 
	unsigned char isTagged, 
	unsigned char *packetBuffs, 
	unsigned long  *buffLen,
	int option_enable_flag
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_MESSAGE_T *data = NULL;
	unsigned int endposition = NPD_DHCP_SNP_INIT_0;
	unsigned int del_len = NPD_DHCP_SNP_INIT_0;
	struct udphdr *udpHdr = NULL;
	struct iphdr  *ipHdr = NULL;
	
	ipHdr  = (struct iphdr *)(packetBuffs + sizeof(struct ethhdr));
	udpHdr = (struct udphdr *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr));	
	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

	if (0 != option_enable_flag) 
	{
		syslog_ax_dhcp_snp_dbg("start option 82 process.\n");
		/* attach option 82 */
		if (NPD_DHCP_BOOTREQUEST == data->op)
		{
			ret = npd_dhcp_snp_attach_opt82(packetBuffs, ifindex, isTagged, vid, buffLen);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("attach option 82 error, ret %x\n", ret);
                
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
		}

		/* remove option 82 */
		if (NPD_DHCP_BOOTREPLY == data->op)
		{	
			syslog_ax_dhcp_snp_dbg("start remove the option 82\n");
			endposition = npd_dhcp_snp_end_option(data->options);
			ret = npd_dhcp_snp_remove_opt82(data, DHCP_OPTION_82, endposition, &del_len);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_dbg("remove option 82 error, ret %x\n", ret);
                
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}

			*buffLen = *buffLen - del_len;
			
			udpHdr->len = htons(ntohs(udpHdr->len) - del_len);

			ipHdr->tot_len = htons(ntohs(ipHdr->tot_len) - del_len); /* cheate on the psuedo-header */

			npd_dhcp_information_checksum(ipHdr,udpHdr);
		}
		syslog_ax_dhcp_snp_dbg("end option 82 process.\n");
	}
    
 	return DHCP_SNP_RETURN_CODE_OK;
}

#ifdef __cplusplus
}
#endif
#endif

