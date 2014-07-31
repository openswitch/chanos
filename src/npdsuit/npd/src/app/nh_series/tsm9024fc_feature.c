#ifdef __cplusplus
extern "C"
{
#endif

#include "tsm9024fc_led_code.c"

long tsm9024fc_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long tsm9024fc_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long tsm9024fc_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long tsm9024fc_slotno_get()
{
	return nbm_slotno_get();
}

long tsm9024fc_local_reset()
{
    nbm_local_reset();
	system("reboot");
	return NPD_SUCCESS;
}

long tsm9024fc_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long tsm9024fc_led_lighting(unsigned long status)
{
    npd_syslog_err("NH TGM9024FC led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long tsm9024fc_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH TGM9024FC led code loading.\n");
	return soc_ledproc_config(unit, ledproc_tsm9024fc, sizeof(ledproc_tsm9024fc));
}

long tsm9024fc_pne_mon_start()
{
    npd_syslog_err("NH TGM9024FC p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long tsm9024fc_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_TSM9024FC;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

#include <sal/appl/config.h>
long tsm9024fc_sal_config_init_defaults(void)
{
	(void)sal_config_set("os", "unix");
    sal_config_set("port_phy_addr_ge0.0","0");
    sal_config_set("port_phy_addr_ge1.0","1");
    sal_config_set("port_phy_addr_ge2.0","2");	
    sal_config_set("port_phy_addr_ge3.0","3");	
    sal_config_set("port_phy_addr_ge4.0","4");
    sal_config_set("port_phy_addr_ge5.0","5");
    sal_config_set("port_phy_addr_ge6.0","6");	
    sal_config_set("port_phy_addr_ge7.0","7");
    sal_config_set("port_phy_addr_ge8.0","8");
    sal_config_set("port_phy_addr_ge9.0","9");
    sal_config_set("port_phy_addr_ge10.0","10");	
    sal_config_set("port_phy_addr_ge11.0","11");	
    sal_config_set("port_phy_addr_ge12.0","12");	
    sal_config_set("port_phy_addr_ge13.0","13");	
    sal_config_set("port_phy_addr_ge14.0","14");	
    sal_config_set("port_phy_addr_ge15.0","15");	
    sal_config_set("port_phy_addr_ge16.0","16");	
    sal_config_set("port_phy_addr_ge17.0","17");	
    sal_config_set("port_phy_addr_ge18.0","18");	
    sal_config_set("port_phy_addr_ge19.0","19");	
    sal_config_set("port_phy_addr_ge20.0","20");	
    sal_config_set("port_phy_addr_ge21.0","21");	
    sal_config_set("port_phy_addr_ge22.0","22");	
    sal_config_set("port_phy_addr_ge23.0","23");	
	sal_config_set("phy_automedium_ge12.0","1");	
    sal_config_set("phy_automedium_ge13.0","1");	
    sal_config_set("phy_automedium_ge14.0","1");	
    sal_config_set("phy_automedium_ge15.0","1");	
    sal_config_set("phy_automedium_ge16.0","1");	
    sal_config_set("phy_automedium_ge17.0","1");	
    sal_config_set("phy_automedium_ge18.0","1");	
    sal_config_set("phy_automedium_ge19.0","1");	
    sal_config_set("phy_automedium_ge20.0","1");	
    sal_config_set("phy_automedium_ge21.0","1");	
    sal_config_set("phy_automedium_ge22.0","1");	
    sal_config_set("phy_automedium_ge23.0","1");
	sal_config_set("trunk_extend.0","1");
    return 0;
}



long tsm9024fc_local_conn_init(int product_type)
{
    return tseries_linecard_local_conn_init(product_type);
}

long tsm9024fc_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return tseries_linecard_system_conn_init(product_type, 
        insert_board_type, insert_slotid);
}

long tsm9024fc_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    return tseries_linecard_system_conn_deinit(product_type, 
        del_board_type, del_slotid);
}

fiber_module_fix_param_t tsm9024fc_sfp_param =
{
    .fiber_module_inserted = &tsm9024fc_sfp_online,
    .fiber_module_insert_detect = &tsm9024fc_sfp_detect_start,
    .fiber_module_info_get = &tsm9024fc_sfp_info_get,
};

ams_fix_param_t tsm9024fc_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &tsm9024fc_asic_led_proc,
};
ams_fix_param_t tsm9024fc_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

board_spec_fix_param_t tsm9024fc_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_TSM9024FC,
    .fiber_module_fix_param = &tsm9024fc_sfp_param,
    .ams_param = {
                    [0] = &tsm9024fc_asic_switch,
                 },
    .slotno_get = &tsm9024fc_slotno_get,
    .reset = &tsm9024fc_local_reset,
    .get_reset_type = &tsm9024fc_reset_type,
    .sys_led_lighting = &tsm9024fc_led_lighting,
    .pne_monitor_start = &tsm9024fc_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &tsm9024fc_local_conn_init,
    .system_conn_init = &tsm9024fc_system_conn_init,
    .system_conn_deinit = &tsm9024fc_system_conn_deinit,
    .asic_config_init = &tsm9024fc_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

