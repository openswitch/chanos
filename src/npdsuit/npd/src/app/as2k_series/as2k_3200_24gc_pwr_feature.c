#ifdef __cplusplus
extern "C"
{
#endif


#include "as2k_3200_24gc_pwr_led_code.c"

long as2k_3200_24gc_pwr_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long as2k_3200_24gc_pwr_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_slotno_get()
{
    return 0;
}

long as2k_3200_24gc_pwr_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}


unsigned  as2k_3200_24gc_pwr_thread_fan_led_light(void *arg)
{
	unsigned int fan_present = NPD_FALSE;
	unsigned int fan_alarm = NPD_FALSE;
	unsigned int old_led_state=0 , new_led_state = 0;
	
	int poll_state = -1;
	int old_poll_state = 1;

	npd_init_tell_whoami("fan_led_lighting",0);
	
	/* check the two fan is all on present */
	fan_present = as2k_3200_fan_present(0x0C, I2C_LPT_SADDR3, I2C_LPT_3);
	/* default is alarm none */
	fan_alarm = as2k_3200_fan_alarm(0x00, I2C_LPT_SADDR3, I2C_LPT_3);

	
	if (fan_present && !fan_alarm)
	{
		/* light on  */
		old_led_state = 1;
		new_led_state = 1;
	}
	else
	{
		/* light off  */
		old_led_state = 0;
		new_led_state = 0;		
	}
	
	as2k_3200_fan_led_light(0x00,0x10,0x10,0x0C,
		I2C_LPT_SADDR3, I2C_LPT_3,new_led_state);
	old_poll_state = 1;
	do 
	{
		sleep(1);
		
		poll_state = state_file_read("/var/run/fan_poll_state");
		//npd_syslog_official_event("poll_state is %d.\n", poll_state);

		if (poll_state == 0)
		{
			if (old_poll_state == 1)
			{
				//npd_syslog_official_event("old poll state is 1.\n");
				old_poll_state = 0;
			}
			continue;
		}
		old_poll_state = poll_state;

		
		fan_present = as2k_3200_fan_present(0x0C, I2C_LPT_SADDR3, I2C_LPT_3);
		fan_alarm = as2k_3200_fan_alarm(0x00, I2C_LPT_SADDR3, I2C_LPT_3);
		if (fan_present && !fan_alarm)
		{
			new_led_state = 1;
		}
		else
		{
			new_led_state = 0;
		}
		
		if (new_led_state != old_led_state)
		{
			//npd_syslog_official_event("fan led state change. new %d old %d.\n",
			//	new_led_state, old_led_state);
			
			as2k_3200_fan_led_light(0x00,0x10,0x10,0x0C,
				I2C_LPT_SADDR3, I2C_LPT_3,new_led_state);
			old_led_state = new_led_state;
		}
	}while(1);
		
}

unsigned  as2k_3200_24gc_pwr_thread_power_led_light(void *arg)
{

	int led_on = NPD_TRUE;
	int led_color = LED_GREEN;
	npd_init_tell_whoami("power_led_lighting",0);

	/* bit LED, b'11 Green, b'00 Yellow, b'10 or b'01 Led OFF */
	as2k_3200_power_led_light(0x00,0x10, 0x30, 0x30, 0x0C,
		I2C_LPT_SADDR2, I2C_LPT_2, led_on, led_color);

	/* when system is initialization, keep the len green */
	while(1)
	{
		sleep(1);
		if(npd_startup_end)
		{
			break;
		}		
	}
	
	while(1)
	{
		usleep(500000); /* 0.5 second */
		if (as2k_is_cpu_busy())
		{
			led_color = LED_YELLOW;
		}
		else /* led close */
		{
			led_color = LED_GREEN;
		}
		as2k_3200_power_led_light(0x00,0x10, 0x30, 0x30,0x0C,
			I2C_LPT_SADDR2, I2C_LPT_2, led_on, led_color);		
		led_on = !led_on;
	}
}

long as2k_3200_24gc_pwr_led_lighting(unsigned long status)
{
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_pne_mon_start()
{
	//as2k_3200_28gc_pwr_fan_led_light();
     nam_thread_create("fan_led_lighting",
		as2k_3200_24gc_pwr_thread_fan_led_light, NULL,NPD_TRUE, NPD_FALSE); 
	
    nam_thread_create("power_led_lighting", 
		as2k_3200_24gc_pwr_thread_power_led_light, NULL,NPD_TRUE, NPD_FALSE);
	
	as2k_start_pne_monitor();
    return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_BOARD_HWCODE_AS2K_3224GC_PWR;
    info->sn = "1001";
    return NPD_SUCCESS;
}



#include <sal/appl/config.h>

long as2k_3200_24gc_pwr_sal_config_init_defaults(void)
{
	sal_config_set("os", "unix");
	/* phy address can use broadcom default */
	sal_config_set("phy_fiber_pref_ge0.0", "0");
	sal_config_set("phy_fiber_pref_ge1.0", "0");
	sal_config_set("phy_fiber_pref_ge2.0", "0");
	sal_config_set("phy_fiber_pref_ge3.0", "0");
	sal_config_set("phy_fiber_pref_ge4.0", "0");
	sal_config_set("phy_fiber_pref_ge5.0", "0");
	sal_config_set("phy_fiber_pref_ge6.0", "0");
	sal_config_set("phy_fiber_pref_ge7.0", "0");	

	/* for combo port */
	sal_config_set("phy_fiber_pref_ge14.0", "1");	
	sal_config_set("phy_fiber_pref_ge15.0", "1");	
		
	/* Integrated phy port, automedium will unstable */	
	
	sal_config_set("phy_automedium_ge0.0","0");	
	sal_config_set("phy_automedium_ge1.0","0");	
	sal_config_set("phy_automedium_ge2.0","0");	
	sal_config_set("phy_automedium_ge3.0","0");	
	sal_config_set("phy_automedium_ge4.0","0");	
	sal_config_set("phy_automedium_ge5.0","0");	
	sal_config_set("phy_automedium_ge6.0","0");	
	sal_config_set("phy_automedium_ge7.0","0");	
	
	/* combo port need open automedium */
	sal_config_set("phy_automedium_ge14.0","1");	
	sal_config_set("phy_automedium_ge15.0","1");	
	
	sal_config_set("trunk_extend.0","1");

	return NPD_SUCCESS;	
}


long as2k_3200_24gc_pwr_poe_init()
{
	char* filename ;

	/* 
	memset(cmd, 0, 256);
	sprintf(cmd, "sudo mknod /dev/ttyS1 c 4 65");
	ret = system(cmd);
	if (ret != 0)
	{
		printf("AS3209GC_PWR create /dev/ttyS1 failed.\n");
		npd_syslog_dbg("AS3209GC_PWR create /dev/ttyS1 failed.\n");
		return NPD_FAIL;
	}
	*/
	
	filename = "/dev/ttyS1";
	 
	if ((console_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		printf("AS3224GC_PWR init poe open console failed.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe open console failed.\n");
		return NPD_FAIL;
	}

	if (fcntl(console_fd, F_SETFL, 0) < 0) 
	{
		printf("AS3224GC_PWR init poe fcntrl F_SETFL failed.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe fcntrl F_SETFL failed.\n");
		return NPD_FAIL;
	}
	int ret = 0;
	if ((ret = as2k_3200_set_com_config(console_fd, 19200, 8, 'N', 1)) < 0)
	{
		printf("AS3224GC_PWR init poe set_com_config failed.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe set_com_config failed.\n");
		return NPD_FAIL;			
	}
	
	return NPD_SUCCESS;	
}



long as2k_3200_24gc_pwr_poe_operate(void *in_data, void *out_data)
{
	int buf_len = 12;
	int i = 0;
	int len=0;
	int remain_length = 0;
	int read_len = 0;
	int read_counter = 0;
	int sum = 0;
	char *request_data = in_data;
	unsigned  char request_buf[12] = {0};
	unsigned  char response_buf[12] = {0};

	for (i = 0; i < 11; i++)
	{
		request_buf[i] = ((char *)request_data)[i];
		sum += request_buf[i];
	}
	request_buf[11]=(sum & 0xFF);
#if 0	
	printf("Resquest Command buffer:\n");
	printf("==============================");
	printf("==============================\n");
	for (i = 0; i < 12; i++)
	{
		if (i == 0)
		{
			int j = 0;
			for (j = 0; j < 12; j++)
				printf(" %-4d", j);
			printf("\n");	
			printf("------------------------------");
			printf("------------------------------\n");			
		}

		printf("0x%02X ", request_buf[i]);
	}	
	printf("\n");
	printf("------------------------------");
	printf("------------------------------\n");	
#endif

	len = write(console_fd, request_buf, buf_len);
	if (len < 0 || len != buf_len)
	{
		printf("AS3224GC_PWR init poe write console error.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe write console error.\n");
		return -1;		
	}
	
	remain_length = 12;
	while (1)
	{
		read_len = read(console_fd, response_buf + (12 - remain_length), remain_length);
		remain_length = remain_length - read_len;
/*
		if (read_counter != 0)
			printf("read_counter %d, read_len %d, remain_length %d.\n", read_counter, read_len, remain_length);
*/
		if (0 == remain_length)
			break;
		read_counter++;
		if (read_counter > 12)
			break;
	}
	if (0 != remain_length)
	{
		printf("AS3224GC_PWR init poe read console error.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe read console error.\n");
		return -1;	
	}
	
	sum = 0;
	for (i = 0; i < 11; i++)
	{
		sum += 	response_buf[i];	
	}
	sum &= 0xFF;

#if 0
	printf("0x%x and data is %s\n", 
		sum, (sum ==response_buf[11] ? "right" : "error"));
	printf("Response Command buffer:\n");
	printf("==============================");
	printf("==============================\n");
	for (i = 0; i < 12; i++)
	{
		if (i == 0)
		{
			int j = 0;
			for (j = 0; j < 12; j++)
				printf(" %-4d", j);
			printf("\n");	
			printf("------------------------------");
			printf("------------------------------\n");			
		}	
		printf("0x%02X ", response_buf[i]);
	}	
	printf("\n");
	printf("------------------------------");
	printf("------------------------------\n");
#endif

	
	if (sum != response_buf[11])
	{

		printf("Resquest Command buffer:\n");
		printf("==============================");
		printf("==============================\n");
		for (i = 0; i < 12; i++)
		{
			if (i == 0)
			{
				int j = 0;
				for (j = 0; j < 12; j++)
					printf(" %-4d", j);
				printf("\n");	
				printf("------------------------------");
				printf("------------------------------\n");			
			}

			printf("0x%02X ", request_buf[i]);
		}	
		printf("\n");
		printf("------------------------------");
		printf("------------------------------\n");			
		
		printf("buf_len is %d.\n", buf_len);
		printf("0x%x and data is %s\n", 
		sum, (sum ==response_buf[11] ? "right" : "error"));
		printf("Response Command buffer:\n");
		printf("==============================");
		printf("==============================\n");
		for (i = 0; i < 12; i++)
		{
			if (i == 0)
			{
				int j = 0;
				for (j = 0; j < 12; j++)
					printf(" %-4d", j);
				printf("\n");	
				printf("------------------------------");
				printf("------------------------------\n");			
			}	
			printf("0x%02X ", response_buf[i]);
		}	
		printf("\n");
		printf("------------------------------");
		printf("------------------------------\n");

		
		printf("AS3224GC_PWR init poe read data checksum error.\n");
		npd_syslog_dbg("AS3224GC_PWR init poe read data checksum error.\n");
		return NPD_FAIL;
	}

	memcpy(out_data, response_buf, 12);
	return NPD_SUCCESS;
}

long as2k_3200_24gc_pwr_i2c_sfp_operate(int unit, char * devName, int addr)
{
	int fd = 0;
	unsigned char sfpVal = 0; 
	unsigned char data = 0;
	unsigned int nbytes = 1;
	int rv = 0;
#if 0	
	if ((fd = bcm_i2c_open(unit, devName, 0, 0)) < 0) 
	{
		npd_syslog_err("Open %s failed fd %d, %s.\n", devName, fd, bcm_errmsg(fd));
		return NPD_FAIL;
	}
		
	if ((rv = bcm_i2c_read(unit, fd, 0, &data, &nbytes)) < 0)
	{
		npd_syslog_err("ERROR: dummy read byte failed: %s\n", bcm_errmsg(rv));
		return NPD_FAIL;
	}

	if (!(0x02&data))
	{
		npd_syslog_dbg("device sfp value has open");
		return NPD_SUCCESS;
	}

	sfpVal =  0xdd & data;
	if ((rv = bcm_i2c_write(unit, fd, 0, &sfpVal, 1)) < 0)
	{
		npd_syslog_err("ERROR: write byte failed: %s\n",bcm_errmsg(rv));
		return NPD_FAIL;
	}	
	
	if ((rv =  bcm_i2c_read(unit, fd, 0, &data, &nbytes)) < 0)
	{
		npd_syslog_err("ERROR: dummy read byte failed: %s\n",bcm_errmsg(rv));
		return NPD_FAIL;		
	}
#endif    
	return NPD_SUCCESS;
}


long as2k_3200_24gc_pwr_asic_led_proc(int unit)
{
    npd_syslog_dbg("AS3224GC_PWR led code for unit %d loading.\n", unit);
	/* set port sfp 23 and 24 */
	/* as2k_3200_24gc_pwr_i2c_sfp_operate(unit, I2C_LPT_0, I2C_LPT_SADDR0); */
	as2k_3200_i2c_sfp_operate(0x00, 0x22, 0x22, 0xDD, I2C_LPT_0, I2C_LPT_SADDR0, 1);
	as2k_3200_24gc_pwr_pne_mon_start();

	return soc_ledproc_config(unit, ledproc_as3200_24gc_pwr, sizeof(ledproc_as3200_24gc_pwr));
}

ipp_fix_param_t  as2k_3200_24gc_pwr_ipp_param =
{
    .ipp_portnum = 1,
    .ipp_phyport_map = {"eth0-1"},
    .ipp_board_map = {-2}
};

sub_board_fix_param_t as2k_3200_24gc_pwr_subboard_param = 
{
    .sub_slotnum = 1,     // more than 1
    .sub_slot_portnum = 0
};
poe_module_fix_param_t  as2k_3200_24gc_pwr_poe_param = 
{
	.poe_ports = {0xffffff,0},
	.pse_total_power = 4030,
	.pse_guard_band = 200,
	.pse_mpss = 1,
	.pse_type = PSE_IEEE_POEPLUS,
	.poe_init = &as2k_3200_24gc_pwr_poe_init,
	.poe_operate = &as2k_3200_24gc_pwr_poe_operate,
};
fiber_module_fix_param_t as2k_3200_24gc_pwr_sfp_param =
{
    .fiber_module_inserted = &as2k_3200_24gc_pwr_sfp_online,
    .fiber_module_insert_detect = &as2k_3200_24gc_pwr_sfp_detect_start,
    .fiber_module_info_get = &as2k_3200_24gc_pwr_sfp_info_get,
};

ams_fix_param_t as2k_3200_24gc_pwr_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = (long (*)(int ))&nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &as2k_3200_24gc_pwr_asic_led_proc,
    .ams_enable = NULL,
};

ams_fix_param_t as2k_3200_24gc_pwr_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

temper_fix_param_t as2k_3200_24gc_pwr_temper_param =
{
    .num = 0,
};

board_feature_t as2k_3200_24gc_pwr_feature = 
{
    .jumbo_size = 13000,
    .max_macaddr = 8192,
    .max_macapp = 512,
    .max_macmpls = 0,
    .max_ext_macaddr = 0,
    .max_normal_vlan = 4096,
    .max_macbased_vlan = 128,
    .max_protobased_vlan = 64,
    .max_ipsubnet_vlan = 64,
    .max_stpg = 64,
    .max_trunk = 8,
    .max_port_per_trunk = 8,
    .max_fabric_trunk_extra = 8,
    .max_vlanswitch_ingress = 256,
    .max_vlanswitch_egress = 256,
    .max_qinq = 256,
    .max_range_qinq = 128, /*a range of vlan map in one q tag id*/
    .max_sec_per_range_qinq = 16, /*a range can contain section number*/
    .max_mirror_des_port = 1,
    
    .max_l2mc = 256,
    .max_vr = 32,
    .max_ip_intf = 1,
    .max_ip_route = 16,
    .max_ip_host_extra = 128,
    .max_ip_ecmp = 16,
    .max_ip_route_per_ecmp = 16,
    .max_ipmc_extra = 0,
    .max_ipv6_route = 8,
    .max_ipv6_host_extra = 7,
    .max_ipv6_ecmp = 0,
    .max_ipv6_route_per_ecmp = 0,
    .max_ipv6mc_extra = 0,
    .max_tunnel = 512,
    .max_dscp_map = 32,
    .max_ele_per_dscp_map = 32,
    .max_mpls_virtual_port = 0,
    .max_mpls_vfi = 0,
    .max_mpls_lsp = 0,
    .max_mpls_pushtagnum = 0,
    .max_mpls_handle_tagnum = 0,
    .max_vlan_acl_std = 4096,
    .max_ingress_acl_std = 512,
    .max_egress_acl_std = 256,
    .vlan_acl_portlist_len = 8,
    .ing_acl_portlist_len = -1,
    .egr_acl_portlist_len = 8,    
    .max_qos_meter = 64,
    .max_qos_counter = 64,
    .bandwidth_adjust_size = 1, /*unit, kbps*/  

    .max_vr_ext = 0,
    .max_ip_intf_ext = 0,
    .max_ip_route_ext = 0,
    .max_ip_host_extra_ext = 0,
    .max_ip_ecmp_ext = 0,
    .max_ip_route_per_ecmp_ext = 0,
    .max_ipmc_extra_ext = 0,
    .max_ipv6_route_ext = 0,
    .max_ipv6_host_extra_ext = 0,
    .max_ipv6_ecmp_ext = 0,
    .max_ipv6_route_per_ecmp_ext = 0,
    .max_ipv6mc_extra_ext = 0,
    .max_tunnel_ext = 0,
    .max_mpls_vfi_ext = 0,
    .max_mpls_lsp_ext = 0,
    .max_vlan_acl_std_ext = 0,
    .max_ingress_acl_std_ext = 0,
    .max_egress_acl_std_ext = 0,
    .max_qos_meter_ext = 0,
    .max_qos_counter_ext = 0,

    .max_txqueue_system = 0,
    .max_txqueue_perport = 8,
    .max_rxqueue_system = 0,
    .max_rxqueue_perport = 1,
    .max_shaper_system = 0,
    .max_shaper_perport = 8,
        
    .ipv4_route_ele_acl_rule_size = 0,
    .ipv6_route_ele_acl_rule_size = 0,
    .mpls_lsp_ele_acl_rule_size = 0,

    .ext_tcam = FALSE,
    .mac_pending_learn = TRUE,
    .ext_mac_auto_learn = FALSE,
    .prohibit_sta_move = TRUE,
    .class_sta_move = TRUE,
    .mac_limit_system = TRUE,
    .mac_limit_per_port = TRUE,
    .mac_limit_per_lag = TRUE,
    .mac_limit_per_vlan = TRUE,
    .mac_limit_per_vpls = FALSE,
    .mac_delete_per_port = TRUE,
    .mac_delete_per_lag = TRUE,
    .mac_delete_per_vlan = TRUE,
    .mac_delete_perport_pervlan = TRUE,
    .mac_delete_perlag_pervlan = TRUE,
    .mac_learn_svl = TRUE,
    .private_vlan = TRUE,
    .flow_vlan = TRUE,
    .vlan_xlate = TRUE,
    .mac_based_app = TRUE,
    .mac_based_mpls = FALSE,
    .same_port_bridge = TRUE,
    
    .preingress_filter = FALSE,
    .ingress_filter = TRUE,
    .egress_filter = TRUE,
    
    .vrf = FALSE,
    .route_urpf = TRUE,

    .ipv4_tunnel = FALSE,
    .ipv6_tunnel = FALSE,
    .gre_tunnel = FALSE,
    .mpls_tunnel = FALSE,

    .mpls_vpls = FALSE,
    .mpls_h_vpls = FALSE,
    .mpls_vpn = FALSE,
    .mpls_te = FALSE,
    .mpls_frr = FALSE,
    .mpls_diffserv = FALSE,

    .wred = TRUE,
    .queue_wdrr = TRUE,
    .queue_wrr = TRUE,
    .queue_rr = TRUE,

    .stack = TRUE,
    .rspan = TRUE,
    .sflow = TRUE,

    .num_of_tcp_comparator = 8,
    .num_of_udp_comparator = 8,
    .support_acl_based_vlan = TRUE,
    
};

int as2k_3200_24gc_pwr_support_product[] =
{
    PRODUCT_AS2K_3200,
    0
};

board_fix_param_t as2k_3200_24gc_pwr_param =
{
    .board_code = PPAL_BOARD_HWCODE_AS2K_3224GC_PWR,
    .board_type = PPAL_BOARD_TYPE_AS2K_3224P,
    .full_name = "AS3224 Power Smart Switch",
    .short_name = "AS3200-24GC-PWR",

    .have_pp = TRUE,
    .master_flag = FALSE,
    .service_flag = TRUE,
    .panel_portnum = 24,

    .subboard_fix_param = &as2k_3200_24gc_pwr_subboard_param,
    .ipp_fix_param = &as2k_3200_24gc_pwr_ipp_param,
    .temper_fix_param = &as2k_3200_24gc_pwr_temper_param,
    .feature = &as2k_3200_24gc_pwr_feature,
    .sdk_type = SDK_BCM,
    .board_support_product = (int*)as2k_3200_24gc_pwr_support_product
};

board_spec_fix_param_t as2k_3200_24gc_pwr_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_AS2K_3224P,

    .fiber_module_fix_param = &as2k_3200_24gc_pwr_sfp_param,
	.poe_module_fix_param = &as2k_3200_24gc_pwr_poe_param,
    .ams_param = {
                    [0] = &as2k_3200_24gc_pwr_asic_switch
                 },

    .slotno_get = &as2k_3200_24gc_pwr_slotno_get,
    .reset = &as2k_3200_24gc_pwr_local_reset,
    .get_reset_type = &as2k_3200_24gc_pwr_reset_type,
    .sys_led_lighting = NULL,
    .pne_monitor_start = as2k_3200_24gc_pwr_pne_mon_start,
    .board_man_param_get = &as2k_3200_24gc_pwr_mnparam_get,
    .local_conn_init = &as2k_3200_local_conn_init,
    .system_conn_init = &as2k_3200_system_conn_init,
    .asic_config_init = &as2k_3200_24gc_pwr_sal_config_init_defaults,
};


#ifdef __cplusplus
extern "C"
}
#endif

