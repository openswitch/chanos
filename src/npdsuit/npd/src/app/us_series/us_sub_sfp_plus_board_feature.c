#ifdef __cplusplus
extern "C"
{
#endif

#include "osal_types.h"
#include "share/shr_error.h"
#include "fal/misc/fal_led.h"


long us_sub_sfp_plus_board_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long us_sub_sfp_plus_board_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_slotno_get()
{
#if 0
    int tipc_node_temp = tipc_get_own_node();
	return (tipc_node_temp&0x0F) - 1;
#endif
    return 1;
}

long us_sub_sfp_plus_board_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");	
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long us_sub_sfp_plus_board_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_HWCODE_US_SUB_SFP_PLUS;
    info->sn = "1001";
	sprintf(info->modname, "AUX4601");
    return NPD_SUCCESS;
}

void us_sub_sfp_plus_board_sal_config_init_defaults(void)
{

}


long us_sub_sfp_plus_board_local_conn_init(int product_type)
{

    return 0;  
}

long us_sub_sfp_plus_board_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}

fiber_module_fix_param_t us_sub_sfp_plus_board_sfp_param =
{
    .fiber_module_inserted = &us_sub_sfp_plus_board_sfp_online,
    .fiber_module_insert_detect = &us_sub_sfp_plus_board_sfp_detect_start,
    .fiber_module_info_get = &us_sub_sfp_plus_board_sfp_info_get,
};
extern long nam_asic_info_get(int unit, struct ams_info_s *info);
extern long nam_asic_enable(int index);

extern unsigned long systemInitialized;
#if 0
long nh_null_asic_init(int unit)
{
    npd_syslog_dbg("only for debug chasm\n");
    systemInitialized = 1;
    return NPD_SUCCESS;
}
#endif
long us_sub_sfp_plus_board_asic_led_proc(int unit)
{
/*
    dal_phy_reg_write(0, 25, 0, 0x18, 0x370);
*/
    return NPD_SUCCESS;
}



ams_fix_param_t us_sub_sfp_plus_board_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
    .ams_led_proc = &us_sub_sfp_plus_board_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t us_sub_sfp_plus_board_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};
/*PPAL_BOARD_TYPE_US_3000 PPAL_BOARD_TYPE_US_4628PWR  板类型需要指定*/
board_spec_fix_param_t us_sub_sfp_plus_board_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_US_SUB_SFP_PLUS,
    .fiber_module_fix_param = &us_sub_sfp_plus_board_sfp_param,
    .ams_param = {
                    [0] = &us_sub_sfp_plus_board_asic_switch
                 },
    .slotno_get = &us_sub_sfp_plus_board_slotno_get,
    .reset = &us_sub_sfp_plus_board_local_reset,
    .get_reset_type = &us_sub_sfp_plus_board_reset_type,
    .sys_led_lighting = &us_sub_sfp_plus_board_led_lighting,
    .pne_monitor_start = &us_sub_sfp_plus_board_pne_mon_start,
    .board_man_param_get = &us_sub_sfp_plus_board_mnparam_get,
    .local_conn_init = &us_sub_sfp_plus_board_local_conn_init,
    .system_conn_init = &us_sub_sfp_plus_board_system_conn_init,
    .asic_config_init = &us_sub_sfp_plus_board_sal_config_init_defaults
};

#ifdef __cplusplus
extern "C"
}
#endif


