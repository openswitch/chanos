
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_c_slot.c
*
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for chassis slot related routine.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.40 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "chasm_manage_proto.h"
#include "nbm/npd_cplddef.h"

product_fix_param_t *snros_system_param = NULL;
board_fix_param_t *snros_local_board = NULL;
board_spec_fix_param_t *snros_local_board_spec = NULL;
board_fix_param_t **snros_board_param = NULL;
board_param_t *localmoduleinfo = NULL;
board_param_t **chassis_slots = NULL;

product_param_t *chassis[MAX_CHASSIS_NUM] = 
{
    &productinfo,
};

int sys_local_chassis_index = 0;

int device_board_type2hwcode( unsigned long board_type )
{

  int i = 0;

  if((board_type < 1) || (board_type > PPAL_BOARD_TYPE_MAX))
  {
    return PPAL_BOARD_TYPE_NONE;
  }
  for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
  {
        if(NULL == module_basic_info[i])
            continue;
  	
        if(board_type == (*module_basic_info[i]).board_type)
            return (*module_basic_info[i]).board_code;
  }

  npd_syslog_cslot_err("Can not get board type based on module type code %d\r\n", board_type);
  return PPAL_BOARD_TYPE_NONE;
}


int device_board_hwcode2type(unsigned long module_type)
{
  int i = 0;
  unsigned long board_type;

  if(module_type > PPAL_BOARD_TYPE_MAX)
  {
    return PPAL_BOARD_TYPE_NONE;
  }
  for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
  {

        if(NULL == module_basic_info[i])
          continue;
        if(board_type == (*module_basic_info[i]).board_code)
            return (*module_basic_info[i]).board_type;
  }

  npd_syslog_cslot_err("Can not get board type based on module type code %d\r\n", board_type);
  return PPAL_BOARD_TYPE_NONE;

}

char * device_board_type2name( unsigned long mtype )
{
    int i;

    if(mtype == PPAL_BOARD_TYPE_NONE)
    {
        return 0;
    }
    for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
    {
        if(NULL == module_basic_info[i])
            continue;
        if(mtype == (*module_basic_info[i]).board_type)
            return (*module_basic_info[i]).short_name;
    }
    return 0;
}


unsigned long device_board_name2type( unsigned char * namebuf, unsigned long mtype )
{
    int i;

    for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
    {
        if(NULL == module_basic_info[i])
            continue;
        if(!(strcmp(namebuf, (*module_basic_info[i]).short_name)))
            return (*module_basic_info[i]).board_type;
    }
    return PPAL_BOARD_TYPE_NONE;

}

int device_board_portnum( unsigned long mtype )
{
    int i;

    if(mtype == PPAL_BOARD_TYPE_NONE)
    {
        return 0;
    }
    for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
    {
        if(NULL == module_basic_info[i])
          continue;
        if(mtype == (*module_basic_info[i]).board_type)
            return (*module_basic_info[i]).panel_portnum;
    }
    return 0;
}


long device_board_have_pp( unsigned long mtype )
{
    int i;

    for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
    {
        if(NULL == module_basic_info[i])
            continue;
        if(mtype == (*module_basic_info[i]).board_type)
            return (*module_basic_info[i]).have_pp;
    }
    return FALSE;
}

long deviceslot_is_master_slot(int slot_index)
{
    int i;

    for(i = 0; i < snros_system_param->board_manage->master_slotnum; i++)
    {
        if(slot_index == snros_system_param->board_manage->master_slot_id[i])
            return TRUE;
    }
    return FALSE;
}


/**********************************************************************************
 * npd_init_one_chassis_slot
 *
 * init chassis slot global info structure,dedicated slot given by chssis_slot_index
 *
 *	INPUT:
 *		subslot_index - chassis slot index to initialize
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/

void npd_init_one_chassis_subslot
(
	board_param_t * board,
	int subslot_index
) 
{
	unsigned char card_type = 0;
	unsigned int i = 0;
    unsigned long subboard_hwcode;
    unsigned int subboard_state;
    sub_board_fix_param_t *subboard_fix_param = board->fix_param->subboard_fix_param;
    board_param_t *sub_board_param = board->sub_board[subslot_index];

    {
        if(sub_board_param == NULL)
        {
            sub_board_param = malloc(sizeof(board_param_t));
            if(NULL == sub_board_param)
            {
                npd_syslog_cslot_err("Can not alloc memeory for sub board %d\n");
            }
            memset(sub_board_param, 0, sizeof(board_param_t));
			sub_board_param->slot_index = subslot_index;
            sub_board_param->runstate = RMT_BOARD_NOEXIST;
            sub_board_param->state_function = rmt_board_state_desc;
			sub_board_param->man_param.modname = malloc(32);
			memset(sub_board_param->man_param.modname, 0,32);
			sub_board_param->man_param.sn = malloc(32);
			memset(sub_board_param->man_param.sn, 0,32);
			
            board->sub_board[subslot_index] = sub_board_param;
            sub_board_param->mother_board = board;
        }
		subboard_state = (*subboard_fix_param->sub_slot_inserted)(subslot_index);
        if(FALSE != subboard_state)
        {
            (*subboard_fix_param->sub_slot_board_init)(subslot_index);
            (*subboard_fix_param->sub_slot_board_type_get)(subslot_index, &subboard_hwcode);
            card_type = device_submoduletype_get(subboard_hwcode);
            sub_board_param->runstate = RMT_BOARD_HWINSERTED;
            sub_board_param->fix_spec_param = module_spec_info[card_type];
            sub_board_param->fix_param = module_basic_info[card_type];
	        sub_board_param->inserted = 1;
            for(i = 0; i < ASIC_TYPE_MAX; i++)
            {
                int j;
				if (NULL == sub_board_param->fix_spec_param->ams_param[i])
						continue;

                for(j = 0; j < sub_board_param->fix_spec_param->ams_param[i]->num; j++)
                {
                    if(sub_board_param->fix_spec_param->ams_param[i]->ams_pre_init)
                        (*sub_board_param->fix_spec_param->ams_param[i]->ams_pre_init)(j);
                    if(sub_board_param->fix_spec_param->ams_param[i]->ams_info_get)
                        (*sub_board_param->fix_spec_param->ams_param[i]->ams_info_get)(j, 
                            &(sub_board_param->ams_info[i][j]));
                }
            }
    		/* only read slot serial no and module name */
			if(sub_board_param->fix_spec_param->board_man_param_get)
    		    (*sub_board_param->fix_spec_param->board_man_param_get)(&(sub_board_param->man_param));

        }
	}

	return;
}


/*
初始化产品sysinfo.
*/
int local_module_info_init()
{
    int ret = NPD_FAIL;
    int i;

    npd_syslog_cslot_dbg("Init local board data.\n");
    
    localmoduleinfo->inserted = TRUE;
    localmoduleinfo->fix_param = snros_local_board;
	localmoduleinfo->fix_spec_param = snros_local_board_spec;
    /*rtc_get_rtc_time(&localmoduleinfo->starting_time);*/
    localmoduleinfo->workmode = SYS_LOCAL_MODULE_IS_MCU? 
         MASTER_BOARD : SLAVE_BOARD;
    localmoduleinfo->redundancystate = MASTER_STANDBY;
    if(MASTER_BOARD == localmoduleinfo->workmode)
    {
        localmoduleinfo->state_function = local_master_state_desc;
    }
    else
    {
        localmoduleinfo->state_function = local_slave_state_desc;
    }
	/*设置状态为INIT，没有统一的宏，用0，丑陋*/
    localmoduleinfo->runstate = localmoduleinfo->state_function[0].state;
    {
        localmoduleinfo->man_param.hw_version = 0x00;
		//strncpy(localmoduleinfo->man_param.modname, "AX31MSB", strlen("AX31MSB"));
		strcpy(localmoduleinfo->man_param.modname, localmoduleinfo->fix_param->short_name);
        (*localmoduleinfo->fix_spec_param->board_man_param_get)(&(localmoduleinfo->man_param));
    }

    localmoduleinfo->sub_board[0] = localmoduleinfo;
    localmoduleinfo->mother_board = localmoduleinfo;
    for(i = 1; i < SLOT_SUBCARD_COUNT(SYS_LOCAL_MODULE_SLOT_INDEX); i++)
    {
        npd_init_one_chassis_subslot(localmoduleinfo, i);
    }
    
    for(i = 0; i < ASIC_TYPE_MAX; i++)
    {
        int j;
        if(localmoduleinfo->fix_spec_param->ams_param[i] == NULL)
            continue;
        for(j = 0; j < localmoduleinfo->fix_spec_param->ams_param[i]->num; j++)
        {
            if(localmoduleinfo->fix_spec_param->ams_param[i]->ams_pre_init)
                (*localmoduleinfo->fix_spec_param->ams_param[i]->ams_pre_init)(j);
            if(localmoduleinfo->fix_spec_param->ams_param[i]->ams_info_get)
                (*localmoduleinfo->fix_spec_param->ams_param[i]->ams_info_get)(j, 
                    &(localmoduleinfo->ams_info[i][j]));
        }
    }
    localmoduleinfo->led_status = 0;
	//add temp info init
	localmoduleinfo->temperature[0].status = TEMP_NORMAL;
	
    return ret;
}

int local_module_info_reinit()
{
    int i;


    npd_syslog_cslot_dbg("ReInit local board data.\n");

	/* Because the product reset , and sub_board point will be reset . */

    localmoduleinfo->sub_board[0] = localmoduleinfo;
    localmoduleinfo->mother_board = localmoduleinfo;
    for(i = 1; i < SLOT_SUBCARD_COUNT(SYS_LOCAL_MODULE_SLOT_INDEX); i++)
    {
        npd_init_one_chassis_subslot(localmoduleinfo, i);
    }
	return NPD_SUCCESS;
}

int chassis_info_reinit()
{
    int ret = NPD_FAIL;
    int i;
    int local_slotid = 0;
    board_param_t **temp_slots = NULL;

    /*sw logic slot id*/
    
    local_slotid = (*snros_local_board_spec->slotno_get)();

    npd_syslog_cslot_dbg("The local slot id is %d\n", local_slotid);

    if(chassis_slots)
    {
        /*已经初始化过一次，但因为某些硬件限制，第一次初始化产品类型不正确，这里再次初始化,需要更改板卡数据，主要用于
		  业务板*/
        temp_slots = chassis_slots;
    }
    {
        chassis_slots = malloc(sizeof(board_param_t*)*SYS_CHASSIS_SLOTNUM);
        if(NULL == chassis_slots)
        {
            npd_syslog_cslot_err("Can not get enough memeory to initialized chassis\r\n");
            return NPD_FAIL;
        }
        
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            chassis_slots[i] = malloc(sizeof(board_param_t));
            if(NULL == chassis_slots[i])
            {
                npd_syslog_cslot_err("Can not get enough memeory to initialized chassis\r\n");
                return NPD_FAIL;
            }
            memset(chassis_slots[i], 0, sizeof(board_param_t));
            chassis_slots[i]->rmtstate = malloc(sizeof(int)*SYS_CHASSIS_SLOTNUM);
            memset(chassis_slots[i]->rmtstate, 0, sizeof(int)*SYS_CHASSIS_SLOTNUM);
            chassis_slots[i]->man_param.modname = malloc(32);
            memset(chassis_slots[i]->man_param.modname, 0, 32);
            chassis_slots[i]->man_param.sn = malloc(32);
            memset(chassis_slots[i]->man_param.sn, 0, 32);
            
        }
        if(temp_slots)
        {
            /*copy existing localmodule data to new*/
            memcpy(chassis_slots[local_slotid], localmoduleinfo, sizeof(board_param_t));

            free(localmoduleinfo);
        }
        localmoduleinfo = chassis_slots[local_slotid];
        localmoduleinfo->slot_index = local_slotid;
        if(!temp_slots)
        {
			local_module_info_init();
		}            
		else
		{
			local_module_info_reinit();
		}
/*这里有内存泄露，但此函数最多执行一次，因为逻辑上处理有点难，所以不释放了,不会对
          系统产生影响*/
#if 0		
        if(temp_slots)
            free(temp_slots);
#endif		
        init_conn_info(SYS_LOCAL_MODULE_TYPE, SYS_PRODUCT_TYPE);
    }
    for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
    {
        if(i == local_slotid)
            continue;
        if(SYS_CHASSIS_ISMASTERSLOT(i))
        {
            chassis_slots[i]->workmode = MASTER_BOARD;
            chassis_slots[i]->redundancystate = MASTER_STANDBY;
        }
        chassis_slots[i]->slot_index = i;
        chassis_slots[i]->state_function = rmt_board_state_desc;
        chassis_slots[i]->sub_board[0] = chassis_slots[i];
		chassis_slots[i]->mother_board = chassis_slots[i]; //fix bug 
    }

    return ret;
}

int npd_sysinfo_parse_mac_addr(char* input, unsigned char*  macAddr)
{
    int i = 0;
    char cur = 0,value = 0;

    if ((NULL == input)||(NULL == macAddr))
    {
        return NPD_FAIL;
    }

    for (i = 0; i <6; i++)
    {
        cur = *(input++);

        if (cur == ':')
        {
            i--;
            continue;
        }

        if ((cur >= '0') &&(cur <='9'))
        {
            value = cur - '0';
        }
        else if ((cur >= 'A') &&(cur <='F'))
        {
            value = cur - 'A';
            value += 0xa;
        }
        else if ((cur >= 'a') &&(cur <='f'))
        {
            value = cur - 'a';
            value += 0xa;
        }

        macAddr[i] = value;
        cur = *(input++);

        if ((cur >= '0') &&(cur <='9'))
        {
            value = cur - '0';
        }
        else if ((cur >= 'A') &&(cur <='F'))
        {
            value = cur - 'A';
            value += 0xa;
        }
        else if ((cur >= 'a') &&(cur <='f'))
        {
            value = cur - 'a';
            value += 0xa;
        }

        macAddr[i] = (macAddr[i]<< 4)|value;
    }

    return NPD_SUCCESS;
}

int chassis_info_init()
{
    int ret = NPD_FAIL;
    
    /*Just provides default settings.*/
    {
        productinfo.sys_info.sn = "1000";
        productinfo.sys_info.name = "AS9600";
        productinfo.sys_info.basemac = "000A7AFE0106";
        productinfo.sys_info.sw_name = "CHANOS";
        productinfo.sys_info.enterprise_name = "OPEN-SWITCH";
        productinfo.sys_info.enterprise_snmp_oid = "";
        productinfo.sys_info.snmp_sys_oid = "";
        productinfo.sys_info.built_in_admin_username = "admin";
        productinfo.sys_info.built_in_admin_passwd = "admin";

    }
    /*the above is only the default, the actually parameter come from 
      following function*/
    npd_syslog_cslot_dbg("before sysinfo get\n");
    {
    	ret = (*snros_system_param->product_man_param_get)( &productinfo.sys_info);
        if(NPD_SUCCESS != ret)
        {
            npd_syslog_cslot_err("Can not get product system info parameter, use default.\r\n");
        }
		/* for read sysinfo error. reset the username to default user */
		if (0==strlen(productinfo.sys_info.built_in_admin_username)) 
		{
	        strcpy(productinfo.sys_info.built_in_admin_username, "admin");
			strcpy(productinfo.sys_info.built_in_admin_passwd, "admin");	
		}
		
        device_product_reset_sn(productinfo.sys_info.sn);
        npd_sysinfo_parse_mac_addr(productinfo.sys_info.basemac, productinfo.base_mac);
		device_product_reset_admin();

		
    }
    chassis_info_reinit();

    return NPD_SUCCESS;
}


/**********************************************************************************
 * npd_init_chassis_slots
 *
 * global entrance to initialize all chassis slots
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
 void dump_product_array()
 {
    int i;
    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
		if(product_type_array[i])
		{
            npd_syslog_cslot_dbg("Product %d pointer %x\n", i, product_type_array[i]);
            npd_syslog_cslot_dbg("Product type %d in array\n", (*product_type_array[i]).product_type);
		}
                
    }
}

void npd_init_chassis_slots
(
	void
) 
{
    dump_product_array();
    
    device_producttype_get();
    device_moduletype_get();
    chassis_info_init();

	return;
}

char *module_id_str
(
	unsigned int module_id	
)
{
    int i;

    if((module_id < 0) || (module_id > PPAL_BOARD_TYPE_MAX))
	{
		return "DUMMY BOARD";
	}
    
	for(i = 0;  i < PPAL_BOARD_TYPE_MAX; i++)
	{
        if(NULL == module_basic_info[i])
            continue;
		if(-1 == (*module_basic_info[i]).board_type)
		{
			break;
		}
		
        if(module_id == (*module_basic_info[i]).board_type)
            return (*module_basic_info[i]).short_name;
    }
    return NULL;
}


int npd_get_modid_by_dev_port( unsigned short usDev, unsigned short usPort)
{
	return UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOTNO, usDev, usPort);
}


/****************************************************************************************
 * npd_get_global_index_by_devport 
 *
 * get ethernet global index from PP device number and virtual port info
 *
 *	INPUT:
 *		devNum - PP device number
 *		portNum - PP port number (or say as virtual port number)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		global eth index
 *    
 *  	NOTE:
 *		this routine is the only interface provided to others get global eth index via device and port number by NPD
 *		because in NPD layer we use global index or slot port to stands for an ether port, no device number and 
 *		port number(view from PP side); And in NAM layer we use global index or device port number to stands for
 *		an ether port, no slot port number(view from product side) provided.
 ****************************************************************************************/
unsigned int npd_get_global_index_by_devport
(
	unsigned char devNum,
	unsigned char portNum,
	unsigned int *eth_g_index
)
{
	unsigned int chassis = 0;
	unsigned int slot_index = 0;
	unsigned int sub_slot_index = 0;
	unsigned int port_no = 0;
	unsigned int	sub_port = 0;
	unsigned int	retVal = 0;
    int module_type = 0;

	chassis = SYS_LOCAL_CHASSIS_INDEX;
	slot_index = SYS_LOCAL_MODULE_SLOT_INDEX;
	sub_slot_index = UNIT_PORT_2_SUB_SLOT_INDEX(devNum, portNum);
	
    if(sub_slot_index == 0)
    {
        module_type = MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot_index);
        if(-1 == PPAL_PHY_2_PANEL(module_type, devNum, portNum))
        {
            int plane_port;
            plane_port = PPAL_PHY_2_PLANE(MODULE_TYPE_ON_SLOT_INDEX(slot_index),
                devNum, portNum);
            if(-1 != plane_port && BOARD_INNER_CONN_PORT != plane_port)
            {
                int peer_slot;
                int peer_plane;
                peer_slot = SLOT_PORT_PEER_SLOT(slot_index, plane_port);
    			if(peer_slot == -1)
    			{
    				return -1;
    			}
                peer_plane = SLOT_PORT_PEER_PORT(slot_index, plane_port);
    			if(peer_plane == -1)
    			{
    				return -1;
    			}
                if(PPAL_BOARD_TYPE_NONE == MODULE_TYPE_ON_SLOT_INDEX(peer_slot))
                    return -1;
                port_no = PPAL_PLANE_2_PANEL(MODULE_TYPE_ON_SLOT_INDEX(peer_slot), 
                     peer_plane, ASIC_SWITCH_TYPE);
                port_no = ETH_LOCAL_NO2INDEX(peer_slot, port_no);
                
    	        *eth_g_index = ETH_GLOBAL_INDEX(chassis,peer_slot, sub_slot_index, port_no, sub_port);
    			return 0;
            }
            return -1;
        }
        port_no = ETH_LOCAL_NO2INDEX(slot_index, PPAL_PHY_2_PANEL(module_type, devNum, portNum));
		sub_port = PPAL_PHY_2_PANEL_SUBPORT(module_type, devNum, portNum);
    }
    else
    {
        port_no = ETH_LOCAL_NO2INDEX(slot_index, PHY_2_PANEL(devNum, portNum));
		sub_port = PHY_2_PANEL_SUBPORT(devNum, portNum);
    }
	if(PRODUCT_IS_BOX)
	    *eth_g_index = ETH_GLOBAL_INDEX(chassis,sub_slot_index, 0, port_no, sub_port);
	else
		*eth_g_index = ETH_GLOBAL_INDEX(chassis,slot_index, sub_slot_index, port_no, sub_port);
	return retVal;
}

/****************************************************************************************
 * npd_get_global_index_by_modport 
 *
 * get ethernet global index from PP global module number and virtual port info
 *
 *	INPUT:
 *		devNum - PP device number
 *		portNum - PP port number (or say as virtual port number)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		global eth index
 *    
 *  	NOTE:
 *		this routine is the only interface provided to others get global eth index via device and port number by NPD
 *		because in NPD layer we use global index or slot port to stands for an ether port, no device number and 
 *		port number(view from PP side); And in NAM layer we use global index or device port number to stands for
 *		an ether port, no slot port number(view from product side) provided.
 ****************************************************************************************/
unsigned int npd_get_global_index_by_modport
(
	unsigned int modNum,
	unsigned int portNum,
	unsigned int *eth_g_index
)
{
	unsigned int chassis = 0;
	unsigned int slot_index = 0;
	unsigned int sub_slot_index = 0;
	unsigned int port_no = 0;
	unsigned int	sub_port = 0;
	unsigned int	unit = 0;
	unsigned int	unit_port = 0;
	unsigned int	retVal = 0;
    int module_type;

	chassis = MOD_ID_TO_CHASSIS(modNum);
	slot_index = MOD_ID_TO_SLOT_INDEX(modNum);
	sub_slot_index = 0;
	if(slot_index < 0 || slot_index >= SYS_CHASSIS_SLOTNUM)
	{
		syslog_ax_c_slot_err("Invalid module id %d to generate netif index.\n", modNum);
		return -1;
	}
    module_type = MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot_index);

    if(module_type == PPAL_BOARD_TYPE_NONE)
        return -1;

	if(!MODULE_2_UNIT_CHECK(module_type, modNum))
		return -1;

    unit = MODULE_2_UNIT(module_type, modNum);
	
	unit_port = MODULE_PORT_2_UNIT_PORT(module_type, modNum, portNum);

    if(-1 == PPAL_PHY_2_PANEL(module_type, unit, unit_port))
    {
        int plane_port;
        plane_port = PPAL_PHY_2_PLANE(MODULE_TYPE_ON_SLOT_INDEX(slot_index),
            unit, unit_port);
        if(-1 != plane_port && BOARD_INNER_CONN_PORT != plane_port)
        {
            int peer_slot;
            int peer_plane;
            peer_slot = SLOT_PORT_PEER_SLOT(slot_index, plane_port);
			if(peer_slot == -1)
			{
				return -1;
			}
            peer_plane = SLOT_PORT_PEER_PORT(slot_index, plane_port);
			if(peer_plane == -1)
			{
				return -1;
			}
            if(PPAL_BOARD_TYPE_NONE == MODULE_TYPE_ON_SLOT_INDEX(peer_slot))
                return -1;
            port_no = PPAL_PLANE_2_PANEL(MODULE_TYPE_ON_SLOT_INDEX(peer_slot), 
                 peer_plane, ASIC_SWITCH_TYPE);
            port_no = ETH_LOCAL_NO2INDEX(peer_slot, port_no);
            
	        *eth_g_index = ETH_GLOBAL_INDEX(chassis,peer_slot, sub_slot_index, port_no, sub_port);
			return 0;

        }
        return -1;        
    }
    if(PRODUCT_IS_BOX)
    {
    	sub_slot_index = UNIT_PORT_2_SUB_SLOT_INDEX(unit, unit_port);
    	if(sub_slot_index == 0)
    	{
            port_no = ETH_LOCAL_NO2INDEX(slot_index, PPAL_PHY_2_PANEL(module_type, unit, unit_port));
			sub_port = PPAL_PHY_2_PANEL_SUBPORT(module_type, unit, unit_port);
    	}
    	else
    	{
            port_no = ETH_LOCAL_NO2INDEX(slot_index, PHY_2_PANEL(unit, unit_port));
			sub_port = PHY_2_PANEL_SUBPORT(unit, unit_port);
    	}
    }
	else
	{
	    port_no = ETH_LOCAL_NO2INDEX(slot_index, PPAL_PHY_2_PANEL(module_type, unit, unit_port));
		sub_port = PPAL_PHY_2_PANEL_SUBPORT(module_type, unit, unit_port);
	}
	*eth_g_index = ETH_GLOBAL_INDEX(chassis,slot_index, sub_slot_index, port_no, sub_port);
	if(PRODUCT_IS_BOX)
	    *eth_g_index = ETH_GLOBAL_INDEX(chassis,sub_slot_index, 0, port_no, sub_port);
	else
		*eth_g_index = ETH_GLOBAL_INDEX(chassis,slot_index, sub_slot_index, port_no, sub_port);
    

	return retVal;
}


/****************************************************************************************
 * npd_get_devport_by_global_index 
 *
 * get PP device number and virtual port info from ethernet global index
 *
 *	INPUT:
 *		eth_g_index - ether global index
 *
 *	OUTPUT:
 *		devNum - PP device number
 *		portNum - PP port number (or say as virtual port number)
 *
 *	RETURN:
 *		NPD_SUCCESS - if no error occur
 *		NAM_ERR_SLOT_OUT_OF_RANGE - if slot number given is out of range
 *		NAM_ERR_PORT_OUT_OF_RANGE - if port number given is out of range
 *		NAM_ERR_MODULE_NOT_SUPPORT - if module type of specified slot is unsupported
 *    
 *  	NOTE:
 *		this routine is the only interface provided to others get device and port number via global eth index by NPD
 *		because in NPD layer we use global index or slot port to stands for an ether port, no device number and 
 *		port number(view from PP side); And in NAM layer we use global index or device port number to stands for
 *		an ether port, no slot port number(view from product side) provided.
 ****************************************************************************************/

unsigned int npd_get_devport_by_global_index
(
	unsigned int eth_g_index,
	unsigned char *devNum,
	unsigned char *portNum
)
{
	unsigned int ret = NPD_SUCCESS;
	unsigned int slot_no = 0, slot_index = 0, sub_slot_index = 0;
	unsigned int local_port_no = 0,port_index = 0, sub_port = 0;
	int module_type;
	int i = 0;

	slot_index 		= SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	slot_no			= CHASSIS_SLOT_INDEX2NO(slot_index);
	port_index 		= ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
    sub_slot_index = SUBSLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	sub_port = npd_netif_eth_get_sub_port(eth_g_index);
    ret = eth_port_legal_check(eth_g_index);
    if(0 != ret)
        return NPD_FAIL;
    ret = eth_port_local_check(eth_g_index);
    if(0 != ret)
    {
        int plane_port;
		module_type 	= MODULE_TYPE_ON_SLOT_INDEX(slot_index);
		
		if (!SYS_MODULE_ISHAVEPP(module_type))
		{
			for(i = 0; i < PPAL_PLANE_PORT_COUNT(module_type); i++)
			{
	            plane_port = PPAL_PANEL_2_PLANE(module_type,
	                ETH_LOCAL_INDEX2NO(slot_index, port_index), i);
	            if(-1 != plane_port)
	            {
	                int peer_slot;
	                peer_slot = SLOT_PORT_PEER_SLOT(slot_index, plane_port);
					if(-1 == peer_slot)
						continue;
	                if(peer_slot == SYS_LOCAL_MODULE_SLOT_INDEX)
	                {
	                    int peer_port;
	                    peer_port = SLOT_PORT_PEER_PORT(slot_index, plane_port);
	                    *devNum = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, peer_port);
	                    *portNum = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, peer_port);
	                    return NPD_SUCCESS;	                    
	                }
	            }
			}
		}
				
		*devNum = -1;
        *portNum = -1;
        return NPD_FAIL;			

    }
    if(sub_slot_index == 0)
    {
    	if (CHASSIS_SLOTNO_ISLEGAL(slot_no)) {
    		local_port_no	= ETH_LOCAL_INDEX2NO(slot_index,port_index);
    		module_type 	= MODULE_TYPE_ON_SLOT_INDEX(slot_index);
    		if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no,local_port_no)) {
    			ret = NPD_SUCCESS;
    		}
    		else {
    			syslog_ax_c_slot_err("npd_get_devport_by_global_index: illegal port %d\n",local_port_no);
    			return -NPD_FAIL;
    		}
    	}
    	else {
    		syslog_ax_c_slot_err("npd_get_devport_by_global_index: illegal slot %d, ifindex 0x%x\n",slot_no, eth_g_index);
    		return -NPD_FAIL;
    	}
	    *devNum = PANEL_SUBPORT_2_PHY_UNIT(local_port_no,sub_port);
	    *portNum = PANEL_SUBPORT_2_PHY_PORT(local_port_no, sub_port);
    }
    else
    {
		if (!local_board_conn_type->board_conn_from_sub[sub_slot_index])
			return -NPD_FAIL;

		
        *devNum = PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_UNIT(sub_slot_index, port_index, sub_port);
        *portNum = PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_PORT(sub_slot_index, port_index, sub_port);
    }
    
	return ret;	
}

unsigned int npd_get_modport_by_global_index
(
	unsigned int eth_g_index,
	unsigned char *modNum,
	unsigned char *portNum
)
{
	unsigned int ret = NPD_SUCCESS;
	unsigned int slot_no = 0, slot_index = 0, sub_slot_index = 0;
	unsigned int local_port_no = 0,port_index = 0, sub_port = 0;
	int module_type;
	unsigned int unit_no = 0;
	int i = 0;
	if(npd_netif_type_get(eth_g_index) == NPD_NETIF_TRUNK_TYPE)
	{
		*portNum= npd_netif_trunk_get_tid(eth_g_index);
		*modNum = 0x1f;
		return 0;
	}
    ret = eth_port_legal_check(eth_g_index);
    if(0 != ret)
        return NPD_FAIL;
	slot_index 		= SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	slot_no			= CHASSIS_SLOT_INDEX2NO(slot_index);
	port_index 		= ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
    sub_slot_index = SUBSLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	sub_port = npd_netif_eth_get_sub_port(eth_g_index);
	if (CHASSIS_SLOTNO_ISLEGAL(slot_no)) {
		local_port_no	= ETH_LOCAL_INDEX2NO(slot_index,port_index);
		module_type 	= MODULE_TYPE_ON_SLOT_INDEX(slot_index);
		if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no,local_port_no) 
			&& SYS_MODULE_ISHAVEPP(module_type)) {
			
			ret = NPD_SUCCESS;
		}
		else 
        {
            int plane_port;
			for(i = 0; i < 2; i++)
			{
                plane_port = PPAL_PANEL_2_PLANE(MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot_index),
                    ETH_LOCAL_INDEX2NO(slot_index, port_index), i);
                if(-1 != plane_port)
                {
                    int peer_slot;
                    peer_slot = SLOT_PORT_PEER_SLOT(slot_index, plane_port);
					if(peer_slot == -1)
					{
						return -1;
					}
                    if(SYS_MODULE_ISMASTERACTIVE(peer_slot))
                    {
                        int peer_port;
                        int peer_type = 0;
						peer_type = MODULE_TYPE_ON_SLOT_INDEX(peer_slot);
                        peer_port = SLOT_PORT_PEER_PORT(slot_index, plane_port);
                        unit_no = PPAL_PLANE_2_UNIT(peer_type, peer_port);
                        *portNum = PPAL_PLANE_2_PORT(peer_type, peer_port);
                    	*modNum = UNIT_2_MODULE(peer_type, peer_slot, unit_no, *portNum);
                        *portNum = UNIT_PORT_2_MODULE_PORT(peer_type, unit_no, *portNum);
                        return NPD_SUCCESS;
                    }
                }
			}
			syslog_ax_c_slot_err("npd_get_modport_by_global_index: illegal port %d\n",local_port_no);
			return -NPD_FAIL;
		}
	}
	else
		return NPD_FAIL;
	if((PRODUCT_IS_BOX) && (slot_index > 0))
	{
		if(sub_slot_index)
		{
            unit_no = PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_UNIT(sub_slot_index, port_index, sub_port);
            *portNum = PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_PORT(sub_slot_index, port_index, sub_port);
	        *modNum = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX, unit_no, *portNum);
            *portNum = UNIT_PORT_2_MODULE_PORT(SYS_LOCAL_MODULE_TYPE, unit_no, *portNum);
			return NPD_SUCCESS;
		}
		syslog_ax_c_slot_err("npd_get_modport_by_global_index: illegal slot %d\n",slot_no);
		return -NPD_FAIL;
	}
	else
	{
        module_type =  MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot_index); 
		if(sub_port)
		{
    	    unit_no = PPAL_PANEL_SUBPORT_2_PHY_UNIT(module_type, local_port_no, sub_port);
    	    *portNum = PPAL_PANEL_SUBPORT_2_PHY_PORT(module_type, local_port_no, sub_port);
		}
		else
		{
    	    unit_no = PPAL_PANEL_2_PHY_UNIT(module_type, local_port_no);
    	    *portNum = PPAL_PANEL_2_PHY_PORT(module_type, local_port_no);
		}
    	
    	*modNum = UNIT_2_MODULE(module_type, slot_index, unit_no, *portNum);
        *portNum = UNIT_PORT_2_MODULE_PORT(module_type, unit_no, *portNum);
    	return ret;	
	}
}


/**********************************************************************************
 * npd_init_chassis_manage_thread
 *
 * initialize a thread to start chassis manage thread. Communicate with the other 
 * board for the board role deciding, configuration, and command. 
 * 
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

pthread_mutex_t chassis_init_mutex = PTHREAD_MUTEX_INITIALIZER;
void npd_init_chassis_manage_thread
(
    void
)
{
    /*here lock for avoid chassis manage thread unlock firstly*/
    pthread_mutex_lock(&chassis_init_mutex);
	nam_thread_create("ChassisManageThread",(void*)npd_chassis_manage, NULL,NPD_TRUE,NPD_FALSE);
	return;
}

void npd_chassis_manage_initialization_check(void)
{
    int ret;
    pthread_mutex_lock(&chassis_init_mutex);
    /*pne but not poe*/
	if (NULL != snros_system_param->pne_fix_param_t->pne_monitor_start)
	{
		ret = (*snros_system_param->pne_fix_param_t->pne_monitor_start)();
	    if(NPD_SUCCESS != ret)
	    {
	        npd_syslog_cslot_err("Can not start pne monitor, use default value.\r\n");
	    }
		ENVIROMENT_MONITOR_ENABLE = 1;
	}
	else
	{
		npd_syslog_cslot_err("there isn't pne monitor method. \n");
	}
}

void chasm_init_done(void)
{
    pthread_mutex_unlock(&chassis_init_mutex);
}

/**********************************************************************************
 * npd_init_board_check_thread
 *
 * initialize a thread to check board state:read board state from CPLD and check 
 * if board is running or being pulled out.
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

void npd_init_board_check_thread
(
	void
)
{
	return;
}

/**********************************************************************************
 * npd_chassis_slot_hotplugin
 *
 * npd callback functions called when chassis slot is hot plugged in,which reported by board check thread.
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index(or slot number)
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
void npd_chassis_slot_hot_plugin
(
	int chassis_slot_index
) 
{
	/* first we should re-initialize chassis slot info*/
/*	npd_init_one_chassis_slot(chassis_slot_index);*/
	
	/* TODO: some other re-initialize process we should taken here*/

	return;
}

/**********************************************************************************
 * npd_chassis_slot_hot_pullout
 *
 * npd callback functions called when chassis slot is hot pulled out,which reported by board check thread.
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index(or slot number)
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
int PPAL_LOCAL_MODULE_ID(int unit, int phyport)
{
	return UNIT_2_MODULE(MODULE_TYPE_ON_SUBSLOT_INDEX(SYS_LOCAL_MODULE_SLOT_INDEX, 0), SYS_LOCAL_MODULE_SLOT_INDEX, unit, phyport);
}

int npd_pne_info_init(void)
{
	power_supply_man_param_t * ptr_ps_man_param;
	fan_man_param_t * ptr_fan_param;
	int index;
	char ps_name[20] = "DummyPS";
	char fan_name[20] = "DummyFAN";
	int ps_man_size = sizeof(power_supply_man_param_t) *SYS_CHASSIS_POWER_NUM ;
	int fan_man_size = sizeof(fan_man_param_t) * SYS_CHASSIS_FAN_NUM;
	int ret = 0;

	productinfo.power_supply_param = 
		(power_supply_man_param_t *)malloc(ps_man_size);
	if (NULL == productinfo.power_supply_param)
	{
		npd_syslog_cslot_err("allocate memory for power_supply_param failed. \n");
		return NPD_FAIL;
	}
		
	for (index = 0; index < SYS_CHASSIS_POWER_NUM; index++)
	{
		ptr_ps_man_param = &productinfo.power_supply_param[index];
		ptr_ps_man_param->ps_index = index;
		ptr_ps_man_param->inserted = FALSE;
		memcpy(ptr_ps_man_param->name, ps_name, 20);
		npd_syslog_cslot_dbg("ps name is %s.\n", ptr_ps_man_param->name);
		ptr_ps_man_param->status = POWER_SUPPLY_NORMAL;
	}

	/* get real info from device */
	for (index = 0; index < SYS_CHASSIS_POWER_NUM; index++)
	{
		ptr_ps_man_param = &productinfo.power_supply_param[index];
		if (NULL != snros_system_param->pne_fix_param_t->power_man_param_init)
		{
			ret = (*snros_system_param->pne_fix_param_t->power_man_param_init)((power_param_t *)ptr_ps_man_param);
	        if(NPD_SUCCESS != ret)
	        {
	            npd_syslog_cslot_err("Can not init pne monitor %d, use default value.\r\n", index);
	        }
		}
		else
		{
			npd_syslog_cslot_err("there isn't power_man_param_init method. \n");
		}
	}

	// init fan param array
	productinfo.fan_param = (fan_man_param_t *)malloc(fan_man_size);
	if (NULL == productinfo.fan_param)
	{
		npd_syslog_cslot_err("allocate memory for fan param failed. \n");
		return NPD_FAIL;
	}

	
	for (index = 0; index < SYS_CHASSIS_FAN_NUM; index++)
	{
		ptr_fan_param = &productinfo.fan_param[index];
		ptr_fan_param->fan_index = index;
		ptr_fan_param->inserted = FALSE;
		memcpy(ptr_fan_param->name, fan_name, 20);
		npd_syslog_cslot_dbg("fan name is %s.\n", ptr_fan_param->name);
		ptr_fan_param->status = FAN_NORMAL;
		ptr_fan_param->speed = 100;
	}
	
	
	/* get real info from device */
	
	for (index = 0; index < SYS_CHASSIS_FAN_NUM; index++)
	{
		ptr_fan_param = &productinfo.fan_param[index];
		if (NULL != snros_system_param->pne_fix_param_t->fan_man_param_init)
		{
			ret = (*snros_system_param->pne_fix_param_t->fan_man_param_init)((fan_param_t *)ptr_fan_param);
	        if(NPD_SUCCESS != ret)
	        {
	            npd_syslog_cslot_err("Can not init pne monitor %d, use default value.\r\n", index);
	        }
		}
		else
		{
			npd_syslog_cslot_err("there isn't fan_man_param_init method. \n");
		}
	}
	return NPD_SUCCESS;
}


#ifdef __cplusplus
}
#endif
