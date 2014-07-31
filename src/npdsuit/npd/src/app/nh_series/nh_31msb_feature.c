#ifdef __cplusplus
extern "C"
{
#endif

#include "u3052_led_code.c"
long nh_31msb_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long nh_31msb_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long nh_31msb_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long nh_31msb_slotno_get()
{
    int tipc_node_temp = tipc_get_own_node();

    if(SYS_CHASSIS_SLOTNUM == 6)
    {
    }
    else
    {
    	printf("(%s)Local board slot no.: %d\r\n", __func__, ((tipc_node_temp&0x0F) - 1));
    }
	
	return (tipc_node_temp&0x0F) - 1;
}

long nh_31msb_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long nh_31msb_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long nh_31msb_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");
	unsigned int param = status;
	unsigned int medium;
	unsigned int port_index;
	led_op_args led_op = {0};
	int ret;
	

	if (NPD_NETIF_ETH_TYPE != npd_netif_type_get(param))
		return NPD_SUCCESS;
	
	port_index = ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(param);
	if (port_index != 48 && port_index != 49)
		return NPD_SUCCESS;

	ret = npd_port_media_get(param, &medium);
	if (ret != NPD_TRUE)
		return NPD_FAIL;
	
	led_op.index = port_index;
	led_op.rwflag = 1;
	if (medium == ETH_GE_SFP)
		led_op.state = LED_MEDIUM_FIBER;
	else
		led_op.state = LED_MEDIUM_COPPER;
	ret = nbm_led_port_control(&led_op);
	if (ret != NPD_SUCCESS)
		return ret;
	
    return NPD_SUCCESS;
}

long nh_31msb_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long nh_31msb_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_HWCODE_NH_3052;
    info->sn = "1001";
    return NPD_SUCCESS;
}

#include <sal/appl/config.h>
void nh_3052_sal_config_init_defaults(void)
{
	sal_config_set("os", "unix");
    sal_config_set("pbmp_gport_stack.0","0x00000030");
    sal_config_set("pbmp_gport_stack.1","0x00000030");
}


long nh_3052_local_conn_init(int product_type)
{
    int unit;
    int module_id;
    int rv;
    int modid;
    int port;
   
    int modnum;
    int max_fabrictrunk_num;


    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

        
        rv = bcm_stk_my_modid_set(unit, module_id);
        if(BCM_E_NONE != rv)
        {
            npd_syslog_dbg("MODID set error, msg %s\n",
                bcm_errmsg(rv));
        }
        {
            bcm_pbmp_t allow_ports;
            BCM_PBMP_ASSIGN(allow_ports, PBMP_ALL(unit));
            BCM_PBMP_PORT_REMOVE(allow_ports, CMIC_PORT(unit));
			/*
            npd_syslog_dbg("%s %d port modid egress set: ports 0x%x, modid: %d, egress ports 0x%x \r\n", 
			    __func__, __LINE__, -1, -1, allow_ports.pbits[0]);
			    */

            rv = bcm_port_egress_set(unit, -1, -1, allow_ports);
            if(BCM_E_NONE != rv)
            {
                npd_syslog_err("Crossbar init: cpu port egress set error for all module, err %s\n",
                    bcm_errmsg(rv));
            }
			/*
            npd_syslog_dbg("%s %d port modid egress set: ports 0x%x, modid: %d, egress ports 0x%x \r\n", 
			    __func__, __LINE__, -1, module_id, PBMP_ALL(unit));
			    */
            rv = bcm_port_egress_set(unit, -1, module_id, PBMP_ALL(unit));
            if(BCM_E_NONE != rv)
            {
                npd_syslog_err("Crossbar init: cpu port egress set error for module %d err %s\n",
                    module_id,bcm_errmsg(rv));
            }
           
        }

        /*rtag7 port trunking load banlance id set*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field0, 0x7ff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field1, 0x3c7);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field0, 0x7ff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field1, 0x3c7);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field0, 0x7ff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field1, 0xf);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashHG2UnknownField0, 0xf);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashHG2UnknownField1, 0x1f);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed0, sal_srand(sal_time_usecs()));
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed1, (unsigned int)-1 - sal_srand(sal_time_usecs()));
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField0Config, BCM_HASH_FIELD_CONFIG_CRC16XOR8);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField1Config, BCM_HASH_FIELD_CONFIG_CRC16);
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet0UnicastOffset, 38);/*hash A, 5bit*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet1UnicastOffset, 13); /*souce port lbn*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0UnicastOffset, 38);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1UnicastOffset, 22);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0NonUnicastOffset, 38);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1NonUnicastOffset, 13);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet0Offset, 38);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet1Offset, 22);
        bcm_switch_control_port_set(unit, 0, bcmSwitchL2PortBlocking, 1);
        PBMP_E_ITER(unit, port)
        {
            bcm_switch_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
        }
        PBMP_ST_ITER(unit, port)
        {
            bcm_switch_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
        }

        /*port 4,5 is cascade port between two 56024*/
        for(port = 4; port <= 5; port++)
        {
            bcm_trunk_chip_info_t trunk_info;
            bcm_trunk_add_info_t trunk_data;
            int modport;
            int find_port = FALSE;
            int tid;

			rv = bcm_port_ifg_set(unit, port, 2500, BCM_PORT_DUPLEX_FULL, 8*8);
            if (rv < 0)
            {
                npd_syslog_err("Crossbar init error: fail to set ifg unit %d port %d, error %s",
                                   unit, port, bcm_errmsg(rv));
                return rv;
            }
			
            rv = bcm_trunk_chip_info_get(unit, &trunk_info);
            if (rv < 0)
            {
                npd_syslog_err("Crossbar init error: fail to get trunk chip info unit %d, error %s",
                                   unit, bcm_errmsg(rv));
                return rv;
            }

                
            tid = PPAL_PHY_2_TRUNK(SYS_LOCAL_MODULE_TYPE, unit, port);

            /*port belongs a trunk*/
            if (-1 != tid)
            {
                int i;
                int stk_modid;
                tid = tid + trunk_info.trunk_fabric_id_min;

                if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid, &trunk_data))
                {
                    rv = bcm_trunk_create_id(unit, tid);

                    if (rv < 0)
                    {
                        npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
                                           unit, tid, bcm_errmsg(rv));
                    }

                    bcm_trunk_add_info_t_init(&trunk_data);
                    trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
                    trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
                    trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
                    /*
                    trunk_data.psc = BCM_TRUNK_PSC_SRCDSTMAC;
                    */
                }

                bcm_stk_modid_get(unit, &modid);
                modport = port;
                for (i = 0; i < trunk_data.num_ports; i++)
                {
                    if (trunk_data.tp[i] == modport)
                    {
                        find_port = TRUE;
                        break;
                    }
                    trunk_data.tm[i] = modid;
                }
                if(find_port)
                {
					continue;
                }
                if (!find_port)
                {
                    trunk_data.tp[trunk_data.num_ports] = port;
                    trunk_data.tm[trunk_data.num_ports] = modid;
                    trunk_data.num_ports = trunk_data.num_ports + 1;
                }
			    rv = bcm_port_enable_set(unit, port, 0);
                npd_syslog_dbg("%s %d trunk set: trunk id: %d, total num: %d, modid %d, port: %d(%s)\r\n", 
			        __func__, __LINE__, tid, trunk_data.num_ports, modid, port, SOC_PORT_NAME(unit, port));
                rv = bcm_trunk_set(unit, tid, &trunk_data);

                if (rv < 0)
                {
                    npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s\r\n",
                                       unit, tid, bcm_errmsg(rv));
                }
				rv = bcm_port_enable_set(unit, port, 1);
                npd_syslog_dbg("Crossbar init: add port modid %d port  %d to trunk %d\n",
                    modid, port, tid);
                for(stk_modid = 0; stk_modid < 64; stk_modid++)
                {
                    if(stk_modid == module_id)
                        continue;
                    bcm_stk_modport_add(unit, stk_modid, port);
                }
            }   
        }
    }
    return 0;  
}

long nh_3052_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}

fiber_module_fix_param_t nh_31msb_sfp_param =
{
    .fiber_module_inserted = &nh_31msb_sfp_online,
    .fiber_module_insert_detect = &nh_31msb_sfp_detect_start,
    .fiber_module_info_get = &nh_31msb_sfp_info_get,
};

extern unsigned long systemInitialized;
long nh_null_asic_init(int unit)
{
    npd_syslog_dbg("only for debug chasm\n");
    systemInitialized = 1;
    return NPD_SUCCESS;
}

long nh3052_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH 3052 led code loading.\n");
	return soc_ledproc_config(unit, ledproc_u3052, sizeof(ledproc_u3052));
}



ams_fix_param_t nh_31msb_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 2,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &nh3052_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t nh_31msb_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

board_spec_fix_param_t nh_31msb_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_NH_3052,
    .fiber_module_fix_param = &nh_31msb_sfp_param,
    .ams_param = {
                    [0] = &nh_31msb_asic_switch
                 },
    .slotno_get = &nh_31msb_slotno_get,
    .reset = &nh_31msb_local_reset,
    .get_reset_type = &nh_31msb_reset_type,
    .sys_led_lighting = &nh_31msb_led_lighting,
    .pne_monitor_start = &nh_31msb_pne_mon_start,
    .board_man_param_get = &nh_31msb_mnparam_get,
    .local_conn_init = &nh_3052_local_conn_init,
    .system_conn_init = &nh_3052_system_conn_init,
    .asic_config_init = &nh_3052_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

