#ifdef __cplusplus
extern "C"
{
#endif

#include "txm9004_led_code.c"
long txm9004_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long txm9004_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long txm9004_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long txm9004_slotno_get()
{
	return nbm_slotno_get();
}

long txm9004_local_reset()
{
    nbm_local_reset();
	system("reboot");
	return NPD_SUCCESS;
}

long txm9004_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long txm9004_led_lighting(unsigned long status)
{
    npd_syslog_err("NH TXM9004 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

long txm9004_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH TXM9004 led code loading.\n");
	return soc_ledproc_config(unit, ledproc_txm9004, sizeof(ledproc_txm9004));
}

long txm9004_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long txm9004_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_TXM9004;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

#include <sal/appl/config.h>
long txm9004_sal_config_init_defaults(void)
{
	(void)sal_config_set("os", "unix");
	sal_config_set("pbmp_xport_xe.0","0x30000000");
	sal_config_set("pbmp_xport_xe.1","0x30000000");
    sal_config_set("phy_ext_rom_boot_28.0","0");
    sal_config_set("phy_ext_rom_boot_29.0","0");
    sal_config_set("phy_ext_rom_boot_28.1","0");
    sal_config_set("phy_ext_rom_boot_29.1","0");
    sal_config_set("port_phy_addr_28.0","32");
    sal_config_set("port_phy_addr_29.0","33");
    sal_config_set("port_phy_addr_28.1","32");
    sal_config_set("port_phy_addr_29.1","33");
	sal_config_set("trunk_extend.0","1");
	sal_config_set("trunk_extend.1","1");
    return 0;
}


long txm9004_local_conn_init(int product_type)
{
    return tseries_linecard_local_conn_init(product_type);
}

long txm9004_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return tseries_linecard_system_conn_init(product_type, 
        insert_board_type, insert_slotid);
}

long txm9004_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    return tseries_linecard_system_conn_deinit(product_type, 
        del_board_type, del_slotid);
}

fiber_module_fix_param_t txm9004_sfp_param =
{
    .fiber_module_inserted = &txm9004_sfp_online,
    .fiber_module_insert_detect = &txm9004_sfp_detect_start,
    .fiber_module_info_get = &txm9004_sfp_info_get,
};

ams_fix_param_t txm9004_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 2,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &txm9004_asic_led_proc,
};

board_spec_fix_param_t txm9004_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_TXM9004,
    .fiber_module_fix_param = &txm9004_sfp_param,
    .ams_param = {
                    [0] = &txm9004_asic_switch,
                 },
    .slotno_get = &txm9004_slotno_get,
    .reset = &txm9004_local_reset,
    .get_reset_type = &txm9004_reset_type,
    .sys_led_lighting = &txm9004_led_lighting,
    .pne_monitor_start = &txm9004_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &txm9004_local_conn_init,
    .system_conn_init = &txm9004_system_conn_init,
    .system_conn_deinit = &txm9004_system_conn_deinit,
    .asic_config_init = &txm9004_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

