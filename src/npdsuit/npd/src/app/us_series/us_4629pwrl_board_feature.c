#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "osal_types.h"
#include "share/shr_error.h"
#include "dal/phy/phy_drv.h"
#include "fal/misc/fal_led.h"
#include "board_cfg.h"

extern board_port_cfg_t board_port_cfg_table[DAL_DEV_NR_ALL_MAX][DAL_DEV_NR_PORT_MAX];
extern unit_ifm_entry_t unit_port_mapping_table[FAL_UNIT_NR_PORT_MAX];

static int aus4629_pwr_console_fd = -1;

long us4629pwrl_board_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long us4629pwrl_board_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long us4629pwrl_board_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long us4629pwrl_board_slotno_get()
{
    int tipc_node_temp = tipc_get_own_node();
	return (tipc_node_temp&0x0F) - 1;
}

long us4629pwrl_board_local_reset()
{
    npd_syslog_err("NH MCU local reset not be supported.\n");
    return NPD_SUCCESS;
}

long us4629pwrl_board_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long us4629pwrl_board_led_lighting(unsigned long status)
{
    npd_syslog_err("NH MCU led lighting not be supported.\n");	
    return NPD_SUCCESS;
}

long us4629pwrl_board_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

/*long us4629pwrl_board_mnparam_get(board_man_param_t *info)
{
    info->id = PPAL_HWCODE_US_4629GX_PWRL;
    info->sn = "1001";
    return NPD_SUCCESS;
}*/

long us4629pwrl_board_mnparam_get(board_man_param_t *info)
{
#define MODULE_NAME_FILE 	"/devinfo/module_name"

	char mod_name[32];
	char mod_sn [32];
	char * ptr;
	int ret = 0;
	int hw_code;

	npd_syslog_dbg("us4629gx_board_mnparam_get.\n");
	info->id = PPAL_HWCODE_US_4629GX_PWRL;
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


void us4629pwrl_board_sal_config_init_defaults(void)
{
	if((localmoduleinfo->sub_board[1] == NULL)
		|| (localmoduleinfo->sub_board[1]->fix_param == NULL))
	{
		/*dal port cfg table*/
        board_port_cfg_table[0][26].valid = SA_FALSE;
        board_port_cfg_table[0][27].valid = SA_FALSE;
        board_port_cfg_table[0][28].valid = SA_FALSE;
        board_port_cfg_table[0][29].valid = SA_FALSE;
		/*fal port cfg table*/
		unit_port_mapping_table[27].valid = SA_FALSE;
		unit_port_mapping_table[28].valid = SA_FALSE;
		unit_port_mapping_table[29].valid = SA_FALSE;
		unit_port_mapping_table[30].valid = SA_FALSE;
	}
	else
	{
		if(localmoduleinfo->sub_board[1]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_SFP_PLUS)
		{
		/*dal port cfg table*/
            board_port_cfg_table[0][26].valid = SA_TRUE;
            board_port_cfg_table[0][27].valid = SA_FALSE;
            board_port_cfg_table[0][28].valid = SA_FALSE;
            board_port_cfg_table[0][29].valid = SA_FALSE;
			
            board_port_cfg_table[0][26].port_interface = PORT_INTERFACE_XAUI;
            board_port_cfg_table[0][26].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][26].phy_type = PHY_TYPE_QT2225;
            board_port_cfg_table[0][26].phy_addr = 0;
		/*fal port cfg table*/
    		unit_port_mapping_table[27].valid = SA_TRUE;
    		unit_port_mapping_table[28].valid = SA_FALSE;
    		unit_port_mapping_table[29].valid = SA_FALSE;
    		unit_port_mapping_table[30].valid = SA_FALSE;

			unit_port_mapping_table[27].port_type = PORT_TYPE_NORMAL;
		}
		else if(localmoduleinfo->sub_board[1]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_4SFP)
		{
		/*dal port cfg table*/
            board_port_cfg_table[0][26].valid = SA_TRUE;
            board_port_cfg_table[0][26].port_interface = PORT_INTERFACE_SGMII;
            board_port_cfg_table[0][26].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][26].phy_type = PHY_TYPE_NULL;
            board_port_cfg_table[0][26].phy_addr = 0;
			
            board_port_cfg_table[0][27].valid = SA_TRUE;
            board_port_cfg_table[0][27].port_interface = PORT_INTERFACE_SGMII;
            board_port_cfg_table[0][27].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][27].phy_type = PHY_TYPE_NULL;
            board_port_cfg_table[0][27].phy_addr = 0;
			
            board_port_cfg_table[0][28].valid = SA_TRUE;
            board_port_cfg_table[0][28].port_interface = PORT_INTERFACE_SGMII;
            board_port_cfg_table[0][28].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][28].phy_type = PHY_TYPE_NULL;
            board_port_cfg_table[0][28].phy_addr = 0;
			
            board_port_cfg_table[0][29].valid = SA_TRUE;
            board_port_cfg_table[0][29].port_interface = PORT_INTERFACE_SGMII;
            board_port_cfg_table[0][29].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][29].phy_type = PHY_TYPE_NULL;
            board_port_cfg_table[0][29].phy_addr = 0;
		/*fal port cfg table*/
    		unit_port_mapping_table[27].valid = SA_TRUE;
			unit_port_mapping_table[27].port_type = PORT_TYPE_NORMAL;
			
    		unit_port_mapping_table[28].valid = SA_TRUE;
			unit_port_mapping_table[28].port_type = PORT_TYPE_NORMAL;
			
    		unit_port_mapping_table[29].valid = SA_TRUE;
			unit_port_mapping_table[29].port_type = PORT_TYPE_NORMAL;
			
    		unit_port_mapping_table[30].valid = SA_TRUE;
			unit_port_mapping_table[30].port_type = PORT_TYPE_NORMAL;

		}
	}
	if((localmoduleinfo->sub_board[2] == NULL)
		|| (localmoduleinfo->sub_board[2]->fix_param == NULL))
	{
		/*dal port cfg table*/
        board_port_cfg_table[0][25].valid = SA_TRUE;
        board_port_cfg_table[0][25].port_interface = PORT_INTERFACE_NULL;
        board_port_cfg_table[0][25].port_medium = PORT_MEDIUM_NULL;
        board_port_cfg_table[0][25].phy_type = PHY_TYPE_NULL;
        board_port_cfg_table[0][25].phy_addr = 0;
		/*fal port cfg table*/
		unit_port_mapping_table[26].valid = SA_TRUE;
		unit_port_mapping_table[26].port_type = PORT_TYPE_CAPWAP;
		unit_port_mapping_table[26].uport_id = 0;

	}
	else
	{
		if(localmoduleinfo->sub_board[2]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_SFP_PLUS)
		{
		/*dal port cfg table*/
            board_port_cfg_table[0][25].valid = SA_TRUE;
            board_port_cfg_table[0][25].port_interface = PORT_INTERFACE_XAUI;
            board_port_cfg_table[0][25].port_medium = PORT_MEDIUM_FIBER;
            board_port_cfg_table[0][25].phy_type = PHY_TYPE_QT2225;
            board_port_cfg_table[0][25].phy_addr = 1;
		/*fal port cfg table*/
		    unit_port_mapping_table[26].valid = SA_TRUE;
		    unit_port_mapping_table[26].port_type = PORT_TYPE_NORMAL;
		    unit_port_mapping_table[26].uport_id = 24;
			
		}
		else
		{
		/*dal port cfg table*/
            board_port_cfg_table[0][25].valid = SA_TRUE;
            board_port_cfg_table[0][25].port_interface = PORT_INTERFACE_NULL;
            board_port_cfg_table[0][25].port_medium = PORT_MEDIUM_NULL;
            board_port_cfg_table[0][25].phy_type = PHY_TYPE_NULL;
            board_port_cfg_table[0][25].phy_addr = 0;
		/*fal port cfg table*/
		    unit_port_mapping_table[26].valid = SA_TRUE;
		    unit_port_mapping_table[26].port_type = PORT_TYPE_CAPWAP;
		    unit_port_mapping_table[26].uport_id = 0;
		}
	}
}

int us4629pwrl_board_set_com_config(int fd, int baud_rate, 
		int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg, old_cfg;
	int speed;

	/* 保存并调试现有串口参数设置， 在这里如果串口号等出错，
	会有相关的出错信息 */
	if (tcgetattr(fd, &old_cfg) != 0)
	{
		npd_syslog_dbg("tcgetattr");
		return NPD_FAIL;	
	}
	
	/*  设置字符大小 */
	memcpy(&new_cfg, &old_cfg, sizeof(struct termios));
	cfmakeraw(&new_cfg); /* 配置为原始模式 */
	new_cfg.c_cflag &= ~CSIZE;
	
	/* 设置波特率 */
	switch(baud_rate)
	{
		case 2400:
			speed = B2400;
			break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;	
			break;		
		case 38400:
			speed = B38400;	
			break;
		case 115200:
		default:
			speed = B115200;
			break;				
	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);
	
	/* 设置停止位 */
	switch (data_bits)
	{
		case 7:
			new_cfg.c_cflag |= CS7;
			break;
		case 8:
		default:
			new_cfg.c_cflag |= CS8;
			break;
	}
	
	/* 设置奇偶校验位 */
	switch (parity)
	{

		case 'o':
		case 'O':
			new_cfg.c_cflag |= (PARODD | PARENB);
			new_cfg.c_iflag |= INPCK;
			break;	
			
		case 'e':
		case 'E':
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
			break;
		
		case 's': /* as no parity */
		case 'S': 
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
			break;

		case 'n':
		case 'N':
		default:			
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
			break;
					
	}
	
	/* 设置停止位 */
	switch (stop_bits)
	{
		case 2:
			new_cfg.c_cflag |= CSTOPB;
			break;
		case 1:
		default:
			new_cfg.c_cflag &= ~CSTOPB;
			break;	
	}
	
	/* 设置等待时间和最小接受字符 */
	new_cfg.c_cc[VTIME] = 10;
	new_cfg.c_cc[VMIN] = 1;
	
	/* 处理未接受字符 */
	tcflush(fd, TCIFLUSH);
	/* 激活新配置 */
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		npd_syslog_dbg("AU3200-29XP set_com_config tcsetattr failed.\n");
		return NPD_FAIL;	
	}
	return NPD_SUCCESS;
}


long us4629pwrl_board_poe_init()
{

	char* filename ;
	int ret = 0;
	
	filename = "/dev/ttyS1";/*注意检查串口终端是不是ttyS1 fixme?*/
	 
	if ((aus4629_pwr_console_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
	
		npd_syslog_dbg("AU3200-29XP init poe open console failed.\n");
		return NPD_FAIL;
	}

	if (fcntl(aus4629_pwr_console_fd, F_SETFL, 0) < 0) 
	{
	
		npd_syslog_dbg("AU3200-29XP init poe fcntrl F_SETFL failed.\n");
		return NPD_FAIL;
	}
	if ((ret = us4628pwr_board_set_com_config(aus4629_pwr_console_fd, 19200, 8, 'N', 1)) < 0)
	{

		npd_syslog_dbg("AU3200-29XP init poe set_com_config failed.\n");
		return NPD_FAIL;			
	}

	return NPD_SUCCESS;	
}



long us4629pwrl_board_poe_operate(void *in_data, void *out_data)
{

	int buf_len = 15;
	int i = 0;
	int len=0;
	int remain_length = 0;
	int read_len = 0;
	int read_counter = 0;
	int sum = 0;
	char *request_data = in_data;
	unsigned  char request_buf[15] = {0};
	unsigned  char response_buf[15] = {0};

	struct timeval time_wait;
	fd_set rfds;


	memset(&rfds, 0, sizeof(rfds));
	time_wait.tv_sec = 1;
	time_wait.tv_usec = 0;
	
	
	for (i = 0; i < 13; i++)
	{
		request_buf[i] = ((char *)request_data)[i];
		sum += request_buf[i];
	}
	request_buf[13]=((sum >>8)& 0xFF);
	request_buf[14]= (sum & 0xFF);
	len = write(aus4629_pwr_console_fd, request_buf, buf_len);
	if (len < 0 || len != buf_len)
	{
		
		npd_syslog_dbg("AU3200-29XP init poe write console error.\n");
		return -1;		
	}

	remain_length = 15;
	while (1)
	{
		FD_ZERO(&rfds);
		FD_SET(aus4629_pwr_console_fd, &rfds);
		select(aus4629_pwr_console_fd + 1, &rfds, NULL, NULL,&time_wait);
		if(FD_ISSET(aus4629_pwr_console_fd, &rfds)){
			read_len = read(aus4629_pwr_console_fd, response_buf + (15 - remain_length), remain_length);
			remain_length = remain_length - read_len;
			read_counter++;
			if (read_counter > 15)
				break;
			if(read_len > 2)
			{
			/*check echo number*/
			    if(response_buf[1] != request_buf[1])
			    {
			        remain_length = 15;
			        continue;
			    }
			}
			if (0 == remain_length)
				break;
		}
		else{
			break;
		}
		
	}
	if (0 != remain_length)
	{
		npd_syslog_dbg("AU3200-29XP init poe read console error.\n");
		return -1;	
	}
	sum = 0;
	for (i = 0; i < 13; i++)
	{
		sum += 	response_buf[i];	
	}
	sum &= 0xFFFF;
	
	if (sum != ((response_buf[13]<<8)|response_buf[14]))
	{
		npd_syslog_dbg("AU3200-29XP init poe read data checksum error.\n");
		return NPD_FAIL;
	}
	
	memcpy(out_data, response_buf, 15);

	return NPD_SUCCESS;
}


long us4629pwrl_board_local_conn_init(int product_type)
{

    return 0;  
}

long us4629pwrl_board_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}
poe_module_fix_param_t  us4629pwrl_board_poe_param = 
{
	.poe_ports = {0xff,0},
	.pse_total_power = 1900,/*125W*/
	.pse_guard_band = 200,/*20W*/
	.pse_mpss = 1,
	.pse_type = PSE_IEEE_POEPLUS,
	.poe_init = &us4629pwrl_board_poe_init,
	.poe_operate = &us4629pwrl_board_poe_operate,
};
fiber_module_fix_param_t us4629pwrl_board_sfp_param =
{
    .fiber_module_inserted = &us4629pwrl_board_sfp_online,
    .fiber_module_insert_detect = &us4629pwrl_board_sfp_detect_start,
    .fiber_module_info_get = &us4629pwrl_board_sfp_info_get,
};
extern long nam_asic_info_get(int unit, struct ams_info_s *info);
extern long nam_asic_enable(int index);


long us4629pwrl_board_asic_led_proc(int unit)
{
    npd_syslog_dbg("AUS4600 led code loading.\n");
    int i = 0;

    for(i = 0; i < 25; i++)
    {
        dal_phy_reg_write(0, i, 0, 0x18, 0x370);
    }
	if((localmoduleinfo->sub_board[1] != NULL)
		&& (localmoduleinfo->sub_board[1]->fix_param != NULL))
	{
		if(localmoduleinfo->sub_board[1]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_SFP_PLUS)
		{
            dal_phy_reg_write(0,26,1,0xd008,0x9);/*上面的LED为LED3, link*/
            dal_phy_reg_write(0,26,1,0xd006,0x1a);/*下面的LED为LED0, active*/
			/*rx adnd tx polarity inverting*/
            dal_phy_reg_write(0,26,4,0xC220,0xFF);
		}
		else if(localmoduleinfo->sub_board[1]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_4SFP)
		{
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

			for(i = 26 ; i<30; i++)
			{
			    music_led_port_speed_link_set(0,i,0,0x4);
			    music_led_port_speed_link_set(0,i,1,0x4);
			    music_led_port_active_display_mode_set(0,i,0,0);
			    music_led_port_active_display_mode_set(0,i,1,1);
			    music_led_port_active_display_en_set(0,i,0,0x0);
			    music_led_port_active_display_en_set(0,i,1,0x3);
			}
		
		}
	}
	if((localmoduleinfo->sub_board[2] != NULL)
		&& (localmoduleinfo->sub_board[2]->fix_param != NULL))
	{
		if(localmoduleinfo->sub_board[2]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_SFP_PLUS)
		{
            dal_phy_reg_write(0,25,1,0xd008,0x9);/*上面的LED为LED3, link*/
            dal_phy_reg_write(0,25,1,0xd006,0x1a);/*下面的LED为LED0, active*/
			/*rx adnd tx polarity inverting*/
            dal_phy_reg_write(0,25,4,0xC220,0xFF);
		}
		else
		{
		}
	}

    return NPD_SUCCESS;
}



ams_fix_param_t us4629pwrl_board_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &us4629pwrl_board_asic_led_proc,
    .ams_enable = NULL,
};
ams_fix_param_t us4629pwrl_board_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 0,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};
/*PPAL_BOARD_TYPE_US_3000 PPAL_BOARD_TYPE_US_4628PWR  板类型需要指定*/
board_spec_fix_param_t us4629pwrl_board_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_US_4629GX_PWRL,
    .fiber_module_fix_param = &us4629pwrl_board_sfp_param,
    .poe_module_fix_param = &us4629pwrl_board_poe_param,
    .ams_param = {
                    [0] = &us4629pwrl_board_asic_switch
                 },
    .slotno_get = &us4629pwrl_board_slotno_get,
    .reset = &us4629pwrl_board_local_reset,
    .get_reset_type = &us4629pwrl_board_reset_type,
    .sys_led_lighting = &us4629pwrl_board_led_lighting,
    .pne_monitor_start = &us4629pwrl_board_pne_mon_start,
    .board_man_param_get = &us4629pwrl_board_mnparam_get,
    .local_conn_init = &us4629pwrl_board_local_conn_init,
    .system_conn_init = &us4629pwrl_board_system_conn_init,
    .asic_config_init = &us4629pwrl_board_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif


