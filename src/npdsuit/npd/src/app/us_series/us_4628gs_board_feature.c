#ifdef __cplusplus
extern "C"
{
#endif

#include "osal_types.h"
#include "share/shr_error.h"
#include "fal/misc/fal_led.h"


long us4628gs_board_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long us4628gs_board_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long us4628gs_board_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long us4628gs_board_slotno_get()
{
    int tipc_node_temp = tipc_get_own_node();
	return (tipc_node_temp&0x0F) - 1;
}

long us4628gs_board_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long us4628gs_board_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long us4628gs_board_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");	
    return NPD_SUCCESS;
}

long us4628gs_board_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long us4628gs_board_mnparam_get(board_man_param_t *info)
{
#define MODULE_NAME_FILE 	"/devinfo/module_name"

	char mod_name[32];
	char mod_sn [32];
	char * ptr;
	int ret = 0;
	int hw_code;

	npd_syslog_dbg("us3000_board_mnparam_get.\n");
	info->id = PPAL_HWCODE_US_4628GS;
	info->sn = "1001";
	
	ret = us_read_sysinfo_file(MODULE_NAME_FILE,mod_name, 32);
	if (ret != NPD_SUCCESS)
	{
		npd_syslog_err("get module name error.\n");
		return ret;
	}
	ptr = (char *)memchr(mod_name, '\n', 32);  
	*ptr = '\0';
	strncpy(info->modname, mod_name, strlen(mod_name));
	info->modname[strlen(mod_name)] = '\0';
	npd_syslog_dbg("modname is %s.\n",info->modname);

	
    return NPD_SUCCESS;
}

void us4628gs_board_sal_config_init_defaults(void)
{
}


long us4628gs_board_local_conn_init(int product_type)
{

    return 0;  
}

long us4628gs_board_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}

fiber_module_fix_param_t us4628gs_board_sfp_param =
{
    .fiber_module_inserted = &us4628gs_board_sfp_online,
    .fiber_module_insert_detect = &us4628gs_board_sfp_detect_start,
    .fiber_module_info_get = &us4628gs_board_sfp_info_get,
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
long us4628gs_board_asic_led_proc(int unit)
{
    npd_syslog_dbg("AUS4600 led code loading.\n");
    int i = 0;
   #if 1
    /*SFP*/
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_6, 1);
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_0, 0);
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_1, 0);
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_2, 0);
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_3, 0);
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_4, 0);    
    fal_led_group_en_set(0, FAL_LED_GROUP_INDEX_5, 0);
    fal_led_group_mode_set(0, FAL_LED_GROUP_INDEX_6, FAL_LED_GROUP_MODE_1);
    fal_led_strobe_offset_set(0, 0x6c);
    fal_led_data_signal_set(0, FAL_LED_DATA_SIGNAL_LOW);
    fal_led_interface_set(0, FAL_LED_INTERFACE_WIRE_3_MODE);
    
    #if 0
    fal_led_blink_mode_set(fal_unit_t unit_id,fal_blink_id_t blink_id,fal_blink_duty_t blink_duty,fal_blink_freq_t 
    blink_freq);
    #endif
    /*phy*/

    for(i = 0; i < 25; i++)
    {
        dal_phy_reg_write(0, i, 0, 0x18, 0x370);
    }
    #endif
    return NPD_SUCCESS;
}



ams_fix_param_t us4628gs_board_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &us4628gs_board_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t us4628gs_board_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};
/*PPAL_BOARD_TYPE_US_3000 PPAL_BOARD_TYPE_US_4628GS  ¡ã?¨¤¨¤D¨ªD¨¨¨°a???¡§*/
board_spec_fix_param_t us4628gs_board_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_US_4628GS,
    .fiber_module_fix_param = &us4628gs_board_sfp_param,
    .ams_param = {
                    [0] = &us4628gs_board_asic_switch
                 },
    .slotno_get = &us4628gs_board_slotno_get,
    .reset = &us4628gs_board_local_reset,
    .get_reset_type = &us4628gs_board_reset_type,
    .sys_led_lighting = &us4628gs_board_led_lighting,
    .pne_monitor_start = &us4628gs_board_pne_mon_start,
    .board_man_param_get = &us4628gs_board_mnparam_get,
    .local_conn_init = &us4628gs_board_local_conn_init,
    .system_conn_init = &us4628gs_board_system_conn_init,
    .asic_config_init = &us4628gs_board_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif


