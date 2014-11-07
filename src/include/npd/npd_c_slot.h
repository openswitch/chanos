#ifndef __NPD_C_SLOT_H__
#define __NPD_C_SLOT_H__

extern char *product_id_str(unsigned int product_id);
extern char *module_id_str(unsigned int module_id);

unsigned int npd_get_global_index_by_devport
(
	unsigned char devNum,
	unsigned char portNum,
	unsigned int* eth_g_index
);

unsigned int npd_get_devport_by_global_index
(
	unsigned int eth_g_index,
	unsigned char *devNum,
	unsigned char *portNum
);

unsigned int npd_get_global_index_by_modport
(
	unsigned int modNum,
	unsigned int portNum,
	unsigned int *eth_g_index
);

unsigned int npd_get_modport_by_global_index
(
	unsigned int eth_g_index,
	unsigned char *modNum,
	unsigned char *portNum
);

void npd_init_board_check_thread
(
	void
);

extern void npd_init_chassis_slots(void);
/**********************************************************************************
 * npd_reset_one_chassis_slot
 *
 * reset chassis slot global info structure,dedicated slot given by chssis_slot_index
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index to initialize
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/

void npd_reset_one_chassis_slot
(
	int chassis_slot_index
);

int npd_pne_info_init
(
	void
);

#include "lib/netif_index.h"

extern void npd_chassis_manage_initialization_check(void);
extern long deviceslot_is_master_slot(int slot_index);
extern int chassis_info_reinit();
extern void chasm_init_done(void);

/*now only support 2 chassis cascade*/
#define MAX_CHASSIS_NUM 2

extern int sys_local_chassis_index;
/*the following is module information*/
#define SYS_LOCAL_CHASSIS_INDEX sys_local_chassis_index 
#define SYS_LOCAL_CHASSIS_NO (SYS_LOCAL_CHASSIS_INDEX + 1)

extern product_param_t *chassis[MAX_CHASSIS_NUM];
/*
#define chassis_slots chassis[SYS_LOCAL_CHASSIS_INDEX]->chassis_slots
*/
extern board_param_t **chassis_slots;

/*
	NOTE: slot_no is the lable printed in the front panel (might start with 1),
			slot_index is the index of the array chassis_slots, always start with 0
*/
extern product_param_t productinfo;

#define MAX_CHASSIS_PER_SYSTEM 2
#define SYS_CHASSIS_NUM 1

#define CHASSIS_SLOT_START_NO 1
#define CHASSIS_SUBSLOT_START_NO 0

#define SYS_MODULE_SUBSLOT_NUM(slot_id)\
    (chassis_slots[slot_id]->fix_param->subboard_fix_param->sub_slotnum)
    

#define CHASSIS_SLOT_INDEX2NO(slot_index)\
	((PRODUCT_IS_BOX)? slot_index:(CHASSIS_SLOT_START_NO + slot_index))
#define CHASSIS_SLOT_NO2INDEX(slot_no) \
	((PRODUCT_IS_BOX)? slot_no:(slot_no - CHASSIS_SLOT_START_NO))

#define CHASSIS_SLOTNO_ISLEGAL(slot_no) \
	((PRODUCT_IS_BOX)? ((slot_no < ((SYS_MODULE_SUBSLOT_NUM(0) > 0)? SYS_MODULE_SUBSLOT_NUM(0):1))):\
            ((slot_no >= CHASSIS_SLOT_START_NO) && (slot_no <= (CHASSIS_SLOT_START_NO + SYS_CHASSIS_SLOTNUM - 1))))

#define SYS_CHASSIS_SLOT_INDEX2NO(slot_index) \
    ((PRODUCT_IS_BOX)? slot_index:(CHASSIS_SLOT_START_NO + slot_index))   
#define SYS_CHASSIS_SLOT_NO2INDEX(slot_no) \
    ((PRODUCT_IS_BOX)? slot_no:(slot_no - CHASSIS_SLOT_START_NO))

#define CHASSIS_VLANID_ISLEGAL(vlan_id) (((vlan_id) >= (CHASSIS_VLAN_RANGE_MIN)) && ((vlan_id) <= (CHASSIS_VLAN_RANGE_MAX)))
#define CHASSIS_AGINGTIME_ISLEGAL(time) ((((time) >= (FDB_AGING_MIN)) && ((time) <= (FDB_AGING_MAX)))|| ((time) == (0)))

#ifndef CHASSIS_SLOT_COUNT
#define CHASSIS_SLOT_COUNT (SYS_CHASSIS_SLOTNUM)
#endif
#define SLOT_SUBCARD_COUNT(slot) \
    (chassis_slots[slot]->fix_param->subboard_fix_param->sub_slotnum)

#define MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]?\
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?\
    chassis_slots[slot_index]->sub_board[subslot_index]->fix_param->board_type:0):0)
#define MODULE_TYPE_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_TYPE_ON_SUBSLOT_INDEX(0, slot_index):\
                MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, 0))


#define MODULE_POEPORTS_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param? \
     chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param : NULL)
#define MODULE_POEPORTS_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_POEPORTS_ON_SUBSLOT_INDEX(0, slot_index):\
         MODULE_POEPORTS_ON_SUBSLOT_INDEX(slot_index, 0))

#define MODULE_PSE_TOTAL_POWER_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param? \
     chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param->pse_total_power : 0)
#define MODULE_PSE_TOTAL_POWER_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))? MODULE_PSE_TOTAL_POWER_ON_SUBSLOT_INDEX(0, slot_index):\
                 MODULE_PSE_TOTAL_POWER_ON_SUBSLOT_INDEX(slot_index, 0))

#define MODULE_PSE_GUARD_BAND_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param? \
     chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param->pse_guard_band : 0)
#define MODULE_PSE_GUARD_BAND_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))? MODULE_PSE_GUARD_BAND_ON_SUBSLOT_INDEX(0, slot_index):\
                   MODULE_PSE_GUARD_BAND_ON_SUBSLOT_INDEX(slot_index, 0))

#define MODULE_PSE_TYPE_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param? \
     chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param->pse_type : 0)
#define MODULE_PSE_TYPE_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_PSE_TYPE_ON_SUBSLOT_INDEX(0, slot_index):\
                 MODULE_PSE_TYPE_ON_SUBSLOT_INDEX(slot_index, 0))

#define MODULE_PSE_MPSS_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param? \
     chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->poe_module_fix_param->pse_mpss : 0)
#define MODULE_PSE_MPSS_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_PSE_MPSS_SUBSLOT_INDEX(0,slot_index):\
                      MODULE_PSE_MPSS_SUBSLOT_INDEX(slot_index, 0))



#define MODULE_STATUS_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->runstate)
#define MODULE_STATUS_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_STATUS_ON_SUBSLOT_INDEX(0,slot_index):\
              MODULE_STATUS_ON_SUBSLOT_INDEX(slot_index, 0))

#define ASM_TYPE_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param?\
    	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->ams_param?\
    		chassis_slots[slot_index]->sub_board[subslot_index]->fix_spec_param->ams_param[0]->type:0):0)
#define ASM_TYPE_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?ASM_TYPE_ON_SUBSLOT_INDEX(0,slot_index):\
                 ASM_TYPE_ON_SUBSLOT_INDEX(slot_index, 0))
    
#define MODULE_HW_VER_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->man_param.hw_version)
#define MODULE_HW_VER_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_HW_VER_ON_SUBSLOT_INDEX(0, slot_index):\
                   MODULE_HW_VER_ON_SUBSLOT_INDEX(slot_index, 0))
    
#define MODULE_SN_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->man_param.sn)
#define MODULE_SN_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_SN_ON_SUBSLOT_INDEX(0,slot_index):\
                     MODULE_SN_ON_SUBSLOT_INDEX(slot_index, 0))
    
#define MODULE_NAME_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?\
    chassis_slots[slot_index]->sub_board[subslot_index]->man_param.modname:"-")
#define MODULE_NAME_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_NAME_ON_SUBSLOT_INDEX(0, slot_index):\
                 MODULE_NAME_ON_SUBSLOT_INDEX(slot_index,0))

#define MODULE_ONLINE_REMOVED_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->online_removed)
#define MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?MODULE_ONLINE_REMOVED_ON_SUBSLOT_INDEX(0, slot_index):\
                MODULE_ONLINE_REMOVED_ON_SUBSLOT_INDEX(slot_index,0))

#define CONFIG_TYPE_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->configure_type)
#define CONFIG_TYPE_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?CONFIG_TYPE_ON_SUBSLOT_INDEX(0, slot_index):\
                  CONFIG_TYPE_ON_SUBSLOT_INDEX(slot_index,0))

#define LED_STATUS_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->led_status)
#define LED_STATUS_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?LED_STATUS_ON_SUBSLOT_INDEX(0, slot_index):\
              LED_STATUS_ON_SUBSLOT_INDEX(slot_index,0))

#define BOARD_CODE_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?\
	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_param->board_code):0)

#define BOARD_CODE_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?BOARD_CODE_ON_SUBSLOT_INDEX(0, slot_index):\
                BOARD_CODE_ON_SUBSLOT_INDEX(slot_index,0))

#define FULL_NAME_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?\
	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_param->full_name):"-")


#define FULL_NAME_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?FULL_NAME_ON_SUBSLOT_INDEX(0, slot_index):\
                FULL_NAME_ON_SUBSLOT_INDEX(slot_index,0))

#define SHORT_NAME_ON_SUBSLOT_INDEX(slot_index, subslot_index) \
	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?\
	(chassis_slots[slot_index]->sub_board[subslot_index]->fix_param->short_name):"-")
#define SHORT_NAME_ON_SLOT_INDEX(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?SHORT_NAME_ON_SUBSLOT_INDEX(0, slot_index):\
                       SHORT_NAME_ON_SUBSLOT_INDEX(slot_index,0))


#define ETH_LOCAL_PORT_SUBSLOT_COUNT(slot_index, subslot_index)\
    (chassis_slots[slot_index]->sub_board[subslot_index]->fix_param?chassis_slots[slot_index]->sub_board[subslot_index]->fix_param->panel_portnum:0)
    
#define ETH_LOCAL_PORT_COUNT(slot_index) \
    (((PRODUCT_IS_BOX)&&(slot_index > 0))?ETH_LOCAL_PORT_SUBSLOT_COUNT(0, slot_index):\
                    ETH_LOCAL_PORT_SUBSLOT_COUNT(slot_index, 0))

    
#define ETH_LOCAL_PORT_START_NO(slot_index) 1
#define ETH_LOCAL_PORTNO_ISLEGAL(slotno,portno) \
    (\
        CHASSIS_SLOTNO_ISLEGAL(slotno) \
        &&(portno >= ETH_LOCAL_PORT_START_NO(CHASSIS_SLOT_NO2INDEX(slotno))) \
        && (portno < (ETH_LOCAL_PORT_COUNT(CHASSIS_SLOT_NO2INDEX(slotno)) \
                        + ETH_LOCAL_PORT_START_NO(CHASSIS_SLOT_NO2INDEX(slotno)))\
            )\
    )
#define PPAL_PLANE_PORT_START_NO(slot_index) 1
#define ETH_STACK_PORTNO_ISLEGAL(slotno,portno) \
		(\
			CHASSIS_SLOTNO_ISLEGAL(slotno) \
			&&(portno >= PPAL_PLANE_PORT_START_NO(CHASSIS_SLOT_NO2INDEX(slotno))) \
			&& (portno < (PPAL_PLANE_PORT_COUNT(MODULE_TYPE_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slotno))) \
							+ PPAL_PLANE_PORT_START_NO(CHASSIS_SLOT_NO2INDEX(slotno)))\
				)\
		)

#define ETH_LOCAL_INDEX2NO(slot_index, eth_index) \
    (ETH_LOCAL_PORT_START_NO(slot_index) + (eth_index))
#define ETH_LOCAL_NO2INDEX(subslot_index,eth_no) \
    ((eth_no) - (ETH_LOCAL_PORT_START_NO(subslot_index)) )

#define CHASSIS_INDEX2NO(chassis_index)\
    (chassis_index+1)

#define CHASSIS_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index)  npd_netif_eth_get_chassis(eth_g_index)

#define SUBSLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index)  npd_netif_eth_get_subslot(eth_g_index)

#define SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index)   npd_netif_eth_get_slot(eth_g_index)

#define ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index) npd_netif_eth_get_port(eth_g_index)

#define ETH_GLOBAL_INDEX(chassis, slot, sub_slot, local, sub_port) eth_port_generate_ifindex(chassis, slot, sub_slot, local, sub_port)

#define ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_INDEX(slot_index,eth_local_index, sub_port) ETH_GLOBAL_INDEX(0, slot_index, 0, eth_local_index, sub_port)

#define SYS_MASTER_ACTIVE_SLOT_INDEX (productinfo.active_master_slot)
#define SYS_SLOTNO_IS_ILLEGAL(slotno) (((slotno) > (SYS_CHASSIS_SLOTNUM))||((slotno) < 1))

#define SYS_CHASSIS_ISMASTERSLOT(slot_index) \
    (TRUE == deviceslot_is_master_slot(slot_index))
    
#define SYS_CHASSIS_ISSLAVESLOT(slot_index) \
    (FALSE == deviceslot_is_master_slot(slot_index))
    
#define SYS_CHASSIS_SLOTNO_ISFABRIC(slot_index) \
    SYS_CHASSIS_ISMASTERSLOT(slot_index)
    
#define SYS_CHASSIS_ISLINE(slot_index) \
    SYS_CHASSIS_ISSLAVESLOT(slot_index)

#define SYS_MODULE_WORKMODE_INDEX(slot_index) \
	(chassis_slots[slot_index]->workmode)

#define SYS_MODULE_WORKMODE_ISMASTER(slot_index) \
    ((MASTER_BOARD) == (chassis_slots[slot_index]->workmode))
    
#define SYS_MODULE_WORKMODE_ISSLAVE(slot_index) \
    ((SLAVE_BOARD) == (chassis_slots[slot_index]->workmode))

#define SYS_MODULE_REDUNDANCY_INDEX(slot_index) \
	(chassis_slots[slot_index]->redundancystate)

#define SYS_MODULE_ISMASTERACTIVE(slot_index) \
    (((chassis_slots[slot_index]->workmode) == MASTER_BOARD) \
      &&((chassis_slots[slot_index]->redundancystate) == MASTER_ACTIVE))
      
#define SYS_MODULE_ISMASTERSTANDBY(slot_index) \
    (((chassis_slots[slot_index]->workmode) == MASTER_BOARD) \
      &&((chassis_slots[slot_index]->redundancystate) == MASTER_STANDBY))

#define MODULE_IS_MCU(type) \
    (module_basic_info[type]->master_flag)

#define SYS_LOCAL_MODULE_SLOTNO \
    (CHASSIS_SLOT_INDEX2NO(localmoduleinfo->slot_index))
#define SYS_SLOTINDEX_IS_REMOTE(slot_index) \
    (((unsigned int)slot_index < SYS_CHASSIS_SLOTNUM) \
       && (slot_index != SYS_LOCAL_MODULE_SLOT_INDEX))

#define SYS_LOCAL_MODULE_SLOT_INDEX \
    (localmoduleinfo->slot_index)
#define SYS_LOCAL_MODULE_WORKMODE_ISMASTER \
    ((MASTER_BOARD) == (localmoduleinfo->workmode))
#define SYS_LOCAL_MODULE_WORKMODE_ISSLAVE \
    ((SLAVE_BOARD) == (localmoduleinfo->workmode))
#define SYS_LOCAL_MODULE_ISMASTERACTIVE \
    ((localmoduleinfo->workmode == MASTER_BOARD) \
      &&(localmoduleinfo->redundancystate == MASTER_ACTIVE))
#define SYS_LOCAL_MODULE_ISMASTERSTANDBY \
    ((localmoduleinfo->workmode == MASTER_BOARD)\
      &&(localmoduleinfo->redundancystate == MASTER_STANDBY))
#define SYS_LOCAL_MODULE_WORKMODE \
    (localmoduleinfo->workmode)
#define SYS_LOCAL_MODULE_REDUNDANCY_STATE \
    (localmoduleinfo->redundancystate)

#define SYS_LOCAL_MODULE_LED_LIGHT(status) \
	(localmoduleinfo->fix_spec_param->sys_led_lighting ? \
		localmoduleinfo->fix_spec_param->sys_led_lighting(status):0)

#define SYS_LOCAL_MODULE_INSERTED(panel_port) \
	(localmoduleinfo->fix_spec_param->fiber_module_fix_param->fiber_module_inserted(panel_port))
	
#define SYS_LOCAL_MODULE_TCV_INFO_GET(panel_port, ptcv_info) \
	(ptcv_info ? localmoduleinfo->fix_spec_param->fiber_module_fix_param->fiber_module_info_get(panel_port, ptcv_info):0)

#define LOCAL_MODULE_ID (productinfo.local_module_id)
#define LOCAL_MODULE_HW_VERSION	(productinfo.local_module_hw_version)

#define SYS_MODULE_PORT_NUM(type)	 \
    ((*module_basic_info[type]).panel_portnum)

#define SYS_MODULE_ISHAVEPP(type) \
    ((*module_basic_info[type]).have_pp)

#define SYS_MODULE_NONE_SERVICE(type) \
     ((*module_basic_info[type]).service_flag == 0)

#define SYS_MODULE_EXTERNAL_SERVICE(type) \
    ((*module_basic_info[type]).service_flag == \
      SERVICE_AS_EXTERNAL_SYSTEM)
#define SYS_MODULE_EXT_PORT(type) \
    ((*module_basic_info[type]).service_flag == \
      SERVICE_AS_PORT_EXT)

#define SYS_MODULE_INTERNAL_SERVICE(type) \
     ((*module_basic_info[type]).service_flag == \
           SERVICE_AS_INTERNAL_SYSTEM)


#define SYS_MODULE_INDEPENDENT_SERVICE(type) \
     ((*module_basic_info[type]).service_flag == \
           SERVICE_AS_INDEPENDENT_SYSTEM)

#define SYS_MODULE_SDK_DIFFERENT(type1, type2) \
    ((*module_basic_info[type1]).sdk_type != \
      (*module_basic_info[type2]).sdk_type)

#define SYS_MODULE_SLOT_EXTERNAL_SERVICE(slot_index) \
    (chassis_slots[slot_index]->fix_param?\
      (chassis_slots[slot_index]->fix_param->service_flag \
        == SERVICE_AS_EXTERNAL_SYSTEM):0)
		
#define SYS_MODULE_SLOT_INTERNAL_SERVICE(slot_index) \
    (chassis_slots[slot_index]->fix_param?\
      (chassis_slots[slot_index]->fix_param->service_flag \
        == SERVICE_AS_INTERNAL_SYSTEM):0)		
		
#define SYS_MODULE_SLOT_INDEPENDENT_SERVICE(slot_index) \
    (chassis_slots[slot_index]->fix_param?\
      (chassis_slots[slot_index]->fix_param->service_flag \
        == SERVICE_AS_INDEPENDENT_SYSTEM):0)

    
#define SYS_MODULE_SLOT_ISHAVEPP(slot_index, sub_slot_index) \
    (chassis_slots[slot_index]->sub_board[sub_slot_index]->fix_param?chassis_slots[slot_index]->sub_board[sub_slot_index]->fix_param->have_pp:0)


#define SYS_MODULE_RUNNINGSTATE(slot_index) \
    (chassis_slots[slot_index]->runstate)
#define SYS_LOCAL_MODULE_RUNNINGSTATE \
    (localmoduleinfo->runstate)

#define SYS_LOCAL_MODULE_SYNC_OVER_CONF \
    do { \
        if(localmoduleinfo->fix_spec_param->asic_after_data_sync_conf)\
        {\
            (*localmoduleinfo->fix_spec_param->asic_after_data_sync_conf)(); \
        }\
    } while(0); \
            

#define SYS_MODULE_REMOTE_RUNSTATE(slot_index) \
	(chassis_slots[slot_index]->rmtstate[slot_index])
	
#define SYS_LOCAL_MODULE_REMOTE_RUNSTATE \
	(localmoduleinfo->rmtstate[localmoduleinfo->slot_index])


#define CHASSIS_SHOW_FUNC \
    (snros_system_param->product_show_chassis)

#define CHASSIS_SLOT_INSERTED_FUN \
    (snros_system_param->board_manage->board_inserted)
#define CHASSIS_SLOT_INSERTED(slot_index) \
    ((*CHASSIS_SLOT_INSERTED_FUN)(slot_index))
#define CHASSIS_SLOT_INSERTED_STR(slot_index)  \
    ((TRUE == (CHASSIS_SLOT_INSERTED(slot_index))) ?"on line":"empty")

#define CHASSIS_SLOT_INSERTED_MASTER_FUN \
    (snros_system_param->board_manage->master_board_inserted)
#define CHASSIS_SLOT_INSERTED_MASTER(slot_index) \
    ((*CHASSIS_SLOT_INSERTED_MASTER_FUN)(slot_index))

#define BOARD_SUBSLOT_INSERTED_FUN \
    (snros_local_board->subboard_fix_param->sub_slot_inserted)
#define BOARD_SUBSLOT_INSERTED(subslot_index) \
    ((*BOARD_SUBSLOT_INSERTED_FUN)(subslot_index))

#define SYS_CHASSIS_SLOTNUM \
    (snros_system_param->board_manage->slotnum)
#define SYS_IS_DISTRIBUTED \
    (TRUE == snros_system_param->board_manage->can_distr)
#define SYS_IS_CENTRAL \
    (FALSE == snros_system_param->board_manage->can_distr)
#define PRODUCT_IS_BOX \
    (1 == SYS_CHASSIS_SLOTNUM)
#define SYS_CHASSIS_MASTER_SLOTNUM \
    (snros_system_param->board_manage->master_slotnum)
#define SYS_CHASSIS_MASTER_SLOT_INDEX(i) \
    (snros_system_param->board_manage->master_slot_id[i])
#define SYS_CHASSIS_SLAVE_SLOTNUM \
    (SYS_CHASSIS_SLOTNUM-SYS_CHASSIS_MASTER_SLOTNUM)
#define SYS_CHASSIS_INTERRUPT_HANDER_FUNC \
	(snros_system_param->interrupt_handler)
#define SYS_CHASSIS_MASTER_SET_FUNC \
	(snros_system_param->master_set)



#define SYS_CHASSIS_POWER_NUM \
	(snros_system_param->pne_fix_param_t->power_num)

#define SYS_CHASSIS_FAN_NUM \
	(snros_system_param->pne_fix_param_t->fan_num)


#define SYS_PRODUCT_TYPE \
    (snros_system_param->product_type)
#define SYS_PRODUCT_HWCODE \
    (snros_system_param->product_code)
#define SYS_PRODUCT_TOPO \
    (snros_system_param->board_manage->topo)


#define SYS_LOCAL_MODULE_TEMPER_COUNT \
	((NULL == snros_local_board->temper_fix_param)?0:snros_local_board->temper_fix_param->num)


#define SYS_LOCAL_MODULE_TYPE \
    (snros_local_board->board_type)
#define SYS_LOCAL_MODULE_HWCODE \
    (snros_local_board->board_code)
#define SYS_LOCAL_MODULE_SUBSLOTNUM \
    (snros_local_board->subboard_fix_param->sub_slotnum)

#define SYS_LOCAL_MODULE_IS_MCU \
    ((snros_local_board->master_flag) \
     ||(SYS_CHASSIS_ISMASTERSLOT(SYS_LOCAL_MODULE_SLOT_INDEX) \
         && (SYS_PRODUCT_TOPO == FULL_MESH)))
   
#define SYS_LOCAL_IPP_NUM \
    (snros_local_board->ipp_fix_param->ipp_portnum)
#define SYS_LOCAL_IPP_PHYPORT(i) \
    (snros_local_board->ipp_fix_param->ipp_phyport_map[i])
#define SYS_LOCAL_IPP_DSTSLOT(i) \
    (snros_local_board->ipp_fix_param->ipp_board_map[i])
#define SYS_LOCAL_MNG_ETH_NUM \
    (snros_local_board->manage_eth_fix_param->manage_eth_portnum)
#define SYS_LOCAL_MNG_ETH_PHYPORT(i) \
    (snros_local_board->manage_eth_fix_param->manage_phyport_map[i])

#define SYS_MODULE_SLOT_HWCODE(slot_index) \
    (chassis_slots[slot_index]->fix_param?chassis_slots[slot_index]->fix_param->board_code:0)

#define SYS_MODULE_SLOT_ISMCU(slot_index) \
    ((chassis_slots[slot_index]->fix_param && chassis_slots[slot_index]->fix_param->master_flag) \
      || (SYS_CHASSIS_ISMASTERSLOT(slot_index) \
            && (SYS_PRODUCT_TOPO == FULL_MESH)))

#define SYS_MODULE_VFP_PORTLIST_LEN(mtype) \
     (module_basic_info[mtype]->feature->vlan_acl_portlist_len)

#define SYS_MODULE_IFP_PORTLIST_LEN(mtype) \
     (module_basic_info[mtype]->feature->ing_acl_portlist_len)

#define SYS_MODULE_EFP_PORTLIST_LEN(mtype) \
     (module_basic_info[mtype]->feature->egr_acl_portlist_len)

#define SYS_MODULE_SUPPORT_CROSSBOARD_TRUNK(mtype) \
    (module_basic_info[mtype]->feature->trunk_notcross_board == FALSE)


#define MAX_VFP_RULE_NUM(mtype) (module_basic_info[mtype]->feature->max_vlan_acl_std)
#define MAX_IFP_RULE_NUM(mtype) (module_basic_info[mtype]->feature->max_ingress_acl_std)
#define MAX_EFP_RULE_NUM(mtype) (module_basic_info[mtype]->feature->max_egress_acl_std)
           
void npd_init_chassis_manage_thread(void);
    

#endif
