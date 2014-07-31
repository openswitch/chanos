
#ifdef __cplusplus
extern "C"
{
#endif


#include "lib/osinc.h"

#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_cplddef.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"

#include "chassis_man_app.h"

#include "nh_product_common_info.h"


#define PRODUCT_TYPE_NAME	"Multi-Layer Core Intelligent Switch"

char *base_mac = "001F641201FF";
extern long  chasm_os_upgrade(unsigned int slot_index);
unsigned long nh_family_type_get()
{
	return nbm_get_family_type_get();
}

unsigned long nh_product_hw_code_get()
{
	return nbm_get_product_hw_code();
}

unsigned long nh_local_module_hw_code_get()
{
	//return nbm_get_module_hw_code();
	return nbm_get_module_type();
}

unsigned char nh_board_hw_version_get()
{
	return nbm_get_board_hw_version();
}

unsigned char nh_board_hw_slotno_get()
{
	return 0;
}

long nh_load_backinfo(product_man_param_t *param)
{
    npd_syslog_dbg("%s: Get product sys info\n", __FUNCTION__);

    param->basemac = base_mac;
    param->sn = snros_system_param->serial_no;
    param->name = snros_system_param->product_short_name;
    param->sw_name = "AuteWare OS";
    param->enterprise_name = "Autelan Co., Ltd.";
    param->enterprise_snmp_oid = "1000";
	
/*    param->built_in_admin_name = "netgear";
    param->built_in_admin_passwd = "";
    
*/
    nbm_read_backplane_sysinfo(param);

	npd_syslog_dbg("enterprise name is %s, name is %s\n --%s", 
			param->enterprise_name, param->name);
	npd_syslog_dbg("Get product name is %s\n ", param->name );
	
    return NPD_SUCCESS;
}

void nh_sys_reset()
{
    npd_syslog_err("reset now not be supported.\n");
}

void nh_chassis_show()
{
    npd_syslog_err("chassis show now not be supported.\n"); 
}

long nh_board_online(unsigned long slot_index)
{
	return nbm_board_online(slot_index);
}

long nh_board_detect_start()
{
    return NPD_SUCCESS;
}

long nh_master_board_online(unsigned long slot_index)
{
    return TRUE;
}

long nh_board_reset(unsigned long slot_index)
{
	return nbm_board_reset(slot_index);
}

long nh_board_poweroff(unsigned long slot_index)
{
    npd_syslog_err("board power off not be supported.\n");
    return NPD_SUCCESS;
}

long nh_slotno_get()
{
	return nbm_slotno_get();
}

long nh_pne_monitor_start()
{
    npd_syslog_err("pne monitor not be supported.\n");
    return NPD_SUCCESS;
}

long nh_fan_speed_adjust(unsigned long index, unsigned long speed)
{
	int ret = 0;
    //npd_syslog_err("fan speed adjust not be supported.\n");
	ret = nbm_set_fan_speed(index, speed); /* */
	if (ret != 0)
	{	
		npd_syslog_err("set fan index %d speed error.\n");
		return NPD_FAIL;
	}
	
    return NPD_SUCCESS;
}

long nh_os_upgrade(unsigned int slot_index)
{
	return chasm_os_upgrade(slot_index);
}
void nh_power_monitor(void)
{
	int inserted = 0;
	int power_state = 0;
	int ret = 0;
	int index = 0;
	power_supply_man_param_t * power_man_array = productinfo.power_supply_param;

	for (index = 0; index < SYS_CHASSIS_POWER_NUM; index++)
	{
		
		ret = nbm_get_power_present(index, &inserted);
		if (ret != 0)
		{	
			npd_syslog_err("get power index %d present state error.\n", index+1);
			continue;
		}
		if (inserted == POWER_INSERT &&
			(power_man_array[index].inserted == FALSE))
		{
			power_man_array[index].inserted = TRUE;
			npd_syslog_warning("power-supply %d inserted.\n", index+1);			
		}
		else if (inserted == POWER_REMOVE &&
			(power_man_array[index].inserted == TRUE))
		{
			power_man_array[index].inserted = FALSE;
			npd_syslog_warning("power-supply %d removed.\n", index+1);						
		}
		
		if (FALSE == power_man_array[index].inserted)
			continue;


		ret = nbm_get_power_state(index, &power_state);
		if (ret != 0)
		{
			npd_syslog_err("get power unit %d state error.\n", index+1);
			continue;
		}
		
		if (power_state == POWER_NORMAL &&
			power_man_array[index].status != POWER_NORMAL)
		{
			power_man_array[index].status = POWER_NORMAL;
			//npd_syslog_warning("power-supply %d work normal.\n", index+1);
		}
		else if (power_state == POWER_ALARM &&
			power_man_array[index].status != POWER_ALARM)
		{
			power_man_array[index].status = POWER_ALARM;
			npd_syslog_warning("power-supply %d work failed.\n", index+1);
		}
	}
}

void nh_fan_monitor(void)
{
	int fan_state = 0;
	int inserted = 0;
	int ret = 0;
	int index = 0;
	fan_man_param_t * fan_man_array = productinfo.fan_param;

	for (index = 0; index < SYS_CHASSIS_FAN_NUM; index++)
	{
		ret = nbm_get_fan_present(index, &inserted);
		if (ret != 0)
		{	
			npd_syslog_err("get fan index %d present state error.\n", index+1);
			continue;
		}
		if (inserted == FAN_INSERT &&
			(fan_man_array[index].inserted == FALSE))
		{
			fan_man_array[index].inserted = TRUE;
			npd_syslog_warning("fan %d inserted.\n", index+1);			
		}
		else if (inserted == POWER_REMOVE &&
			(fan_man_array[index].inserted == TRUE))
		{
			fan_man_array[index].inserted = FALSE;
			npd_syslog_warning("fan %d removed.\n", index+1);						
		}
		
		if (FALSE == fan_man_array[index].inserted)
			continue;

		ret = nbm_get_fan_state(index, &fan_state);
		if (ret != 0)
		{
			npd_syslog_err("get fan %d state error.\n", index+1);
			continue;
		}
		
		if (fan_state == FAN_NORMAL &&
			fan_man_array[index].status != FAN_NORMAL)
		{
			fan_man_array[index].status = FAN_NORMAL;
			//npd_syslog_warning("fan %d work normal.\n", index+1);
		}
		else if (fan_state == FAN_ALARM &&
			fan_man_array[index].status != FAN_ALARM)
		{
			fan_man_array[index].status = FAN_ALARM;
			npd_syslog_warning("fan %d work ALARM.\n", index+1);
		}
	}
	
}

void nh_led_state_operate(void)
{
	int index = 0;
	led_op_args led_op = {0};
	int op_ret = 0;
	int state = 0;
	int insert_cnt = 0;
	int normal_cnt = 0;

	fan_man_param_t * fan_man_array = productinfo.fan_param;
	power_supply_man_param_t * power_man_array = productinfo.power_supply_param;

	if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		//control the fan led status
		memset(&led_op, 0, sizeof(led_op_args));
		for (index = 0; index < SYS_CHASSIS_FAN_NUM; index++)
		{		
			if (fan_man_array[index].inserted == TRUE)
			{
				insert_cnt++;
				if (fan_man_array[index].status == FAN_NORMAL)
				{
					normal_cnt++;
				}
			}
		}
		if (insert_cnt > 0)
		{
			led_op.inserted = LED_STATUS_INSERT;
			if (normal_cnt == insert_cnt)
			{
				led_op.state = LED_STATUS_NORMAL;
			}
			else
			{
				led_op.state = LED_STATUS_ALARM;
			}
		}
		else
		{
			led_op.inserted = LED_STATUS_REMOVE;
		}
		led_op.op_type = LED_FAN;
		op_ret = nbm_led_control(&led_op);		
		if (op_ret != 0)
		{
			npd_syslog_err("fan set led error.\n", index+1);
		}

		// control the power led status	
		memset(&led_op, 0, sizeof(led_op_args));	
		insert_cnt = 0;
		normal_cnt = 0;
		for (index = 0; index < SYS_CHASSIS_POWER_NUM; index++)
		{
			if (power_man_array[index].inserted == TRUE)
			{
				insert_cnt++;
				if (power_man_array[index].status == FAN_NORMAL)
				{
					normal_cnt++;
				}
			}
		}
		led_op.inserted = LED_STATUS_INSERT;
		if (SYS_LOCAL_MODULE_RUNNINGSTATE == LOCAL_ACTMASTER_RUNNING)
		{
			led_op.state = LED_STATUS_NORMAL;
		}
		else
		{
			led_op.state = LED_STATUS_ALARM;
		}
		led_op.op_type = LED_POWER;	
		op_ret = nbm_led_control(&led_op);		
		if (op_ret != 0)
		{
			npd_syslog_err("power supply set led error.\n");
		}

		// TEMP
		memset(&led_op, 0, sizeof(led_op_args));
		state = localmoduleinfo->temperature[0].status;
		if (state == TEMP_NORMAL)
		{
			led_op.state = LED_STATUS_NORMAL;
		}
		else
		{
			led_op.state = LED_STATUS_ALARM;
		}
		led_op.op_type = LED_TEMP;
		op_ret = nbm_led_control(&led_op);		
		if (op_ret != 0)
		{
			npd_syslog_err("temp set led error.\n", index+1);
		}


		//linecard
		memset(&led_op, 0, sizeof(led_op_args));	
		for (index = 0; index < SYS_CHASSIS_SLOTNUM; index++)
		{
			if (SYS_MODULE_WORKMODE_ISSLAVE(index)) 
			{
				if( CHASSIS_SLOT_INSERTED(index))
				{	
					led_op.inserted = LED_STATUS_INSERT;
					if (SYS_MODULE_RUNNINGSTATE(index) == RMT_BOARD_RUNNING)
					{
						led_op.state = LED_STATUS_NORMAL;
					}
					else
					{
						led_op.state = LED_STATUS_ALARM;
					}
				}
				else
				{
					led_op.inserted = LED_STATUS_REMOVE;
				}
				led_op.op_type = LED_LINECARD;

				op_ret = nbm_led_control(&led_op);		
				if (op_ret != 0)
				{
					npd_syslog_err("linecard %d set led error.\n", index+1);
				}		
				led_op.index++; //should be here for over bound
			}
		}
	}

	//master and stanby mode 
	memset(&led_op, 0, sizeof(led_op_args));	
	if (SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
	{
		if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			led_op.inserted = LED_STATUS_INSERT;
			led_op.state = LED_STATUS_NORMAL;
		}
		else
		{
			led_op.inserted = LED_STATUS_REMOVE;
		}
		led_op.op_type = LED_MASTER;
		op_ret = nbm_led_control(&led_op);		
		if (op_ret != 0)
		{
			npd_syslog_err("linecard  %d set led error.\n", index+1);
		}		
	}

	if (SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
	{
		memset(&led_op, 0, sizeof(led_op_args));	
		led_op.inserted = LED_STATUS_INSERT;
		if (localmoduleinfo->rmtstate[localmoduleinfo->slot_index] == RMT_BOARD_RUNNING)
		{
			led_op.state = LED_STATUS_NORMAL;
		}
		else
		{
			led_op.state = LED_STATUS_ALARM;
		}
		led_op.op_type = LED_POWER;	
		
		op_ret = nbm_led_control(&led_op);		
		if (op_ret != 0)
		{
			npd_syslog_err("power supply set led error.\n");
		}		
	}
	
	
}

void nh_temp_monitor()
{
	int op_ret;
	int state;
	int changed = FALSE;
	int current_temp;
	int temp_limit;
	int orig_state = localmoduleinfo->temperature[0].status;
	temp_info_args  temp_data;

	if (SYS_LOCAL_MODULE_TEMPER_COUNT <= 0)
	{
		return ;
	}

	op_ret = nbm_get_temp_state(&state);
	if (op_ret != 0)
	{
		npd_syslog_err("get slot %d temp state error.\n", SYS_LOCAL_MODULE_SLOTNO);
		return ;
	}
	op_ret = nbm_get_temp_info(&temp_data);
	if (op_ret != 0)
	{
		npd_syslog_err("get slot %d temp info error.\n", SYS_LOCAL_MODULE_SLOTNO);
		return ;
	}
	
	if (state == TEMP_NORMAL && orig_state != state) //alarm -> normal
	{
		if (orig_state == TEMP_MODULE_BEYOND)
		{
			current_temp = temp_data.module_temp;
			temp_limit = temp_data.module_high_limit;
			if (current_temp < temp_limit - TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (orig_state == TEMP_MODULE_UNDERNEATH)
		{
			current_temp = temp_data.module_temp;
			temp_limit = temp_data.module_low_limit;
			if (current_temp > temp_limit + TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (orig_state == TEMP_CORE_BEYOND)
		{
			current_temp = temp_data.core_temp;
			temp_limit = temp_data.core_high_limit;
			if (current_temp < temp_limit - TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (orig_state == TEMP_CORE_UNDERNEATH)
		{
			current_temp = temp_data.core_temp;
			temp_limit = temp_data.core_low_limit;
			if (current_temp > temp_limit + TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else
		{
			changed = FALSE;
		}
		
		if (changed)
		{
			npd_syslog_warning("local slot %d temp normal.", SYS_LOCAL_MODULE_SLOTNO);
			npd_syslog_warning("current temp is %d, limit is %d.\n", current_temp, temp_limit);		
		}
		
	}
	else if (state != TEMP_NORMAL && orig_state != state) // normal -> alarm 
	{
		if (state == TEMP_MODULE_BEYOND)
		{
			current_temp = temp_data.module_temp;
			temp_limit = temp_data.module_high_limit;
			if (current_temp > temp_limit + TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (state == TEMP_MODULE_UNDERNEATH)
		{
			current_temp = temp_data.module_temp;
			temp_limit = temp_data.module_low_limit;
			if (current_temp < temp_limit - TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (state == TEMP_CORE_BEYOND)
		{
			current_temp = temp_data.core_temp;
			temp_limit = temp_data.core_high_limit;
			if (current_temp > temp_limit + TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else if (state == TEMP_CORE_UNDERNEATH)
		{
			current_temp = temp_data.core_temp;
			temp_limit = temp_data.core_low_limit;
			if (current_temp < temp_limit - TEMP_VARIATION)
			{
				changed = TRUE;
			}
		}
		else
		{
			changed = FALSE;
		}	
		
		if (changed)
		{
			npd_syslog_warning("local slot %d temp alarm.", SYS_LOCAL_MODULE_SLOTNO);
			npd_syslog_warning("current temp is %d, limit is %d.\n", current_temp, temp_limit);		
		}
	}

	if (changed)
	{
		localmoduleinfo->temperature[0].status = state;
	}
}

int nh_pne_monitor(void)
{
	static int slave_indpnt = 0;
	npd_init_tell_whoami("pneMonitor", (unsigned char)0);
	slave_indpnt = app_slave_indpnt_runget();
	do
	{
		sleep(10);
		if (!ENVIROMENT_MONITOR_ENABLE)
			continue;
		if (1 == slave_indpnt)
			continue;
		
		nh_temp_monitor();
		
		if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{			
			//check power state
			nh_power_monitor();
			//check fan state			
			nh_fan_monitor();
			// check temp state
		}
		// operate the led state
		nh_led_state_operate();
	}while(1);
}

long nh_power_man_param_init(power_param_t * param)
{
	power_info_args power_info;
	unsigned int ps_index = param->ps_index;
	int ret;
	int state;


	ret = nbm_get_power_present(ps_index, &state);
	if (ret != 0)
	{	
		npd_syslog_err("get power index %d present state error.\n", ps_index);
		return -1;
	}
	if (state == POWER_INSERT)
	{
		param->inserted = TRUE;
	}
	else
	{
		param->inserted = FALSE;
	}

	if (param->inserted != TRUE)
	{
		npd_syslog_err("power index %d removed. user default info.\n", ps_index);
		return -1;
	}

	power_info.index = ps_index;
	ret = nbm_get_power_info(ps_index, &power_info);
	if (ret != 0)
	{	
		npd_syslog_err("get power index %d info error.\n", ps_index);
		return -1;
	}
	memcpy(param->name, power_info.name, 20);
	

	ret = nbm_get_power_state(ps_index, &state);
	if (ret != 0)
	{	
		npd_syslog_err("get power index %d work state error.\n", ps_index);
		return -1;
	}
	if (state == POWER_NORMAL)
	{
		param->status = POWER_NORMAL;
	}
	else
	{
		param->status = POWER_ALARM;
	}
	
	return 0;
	
}

long nh_fan_param_init(fan_param_t * param)
{
	unsigned int fan_index = param->fan_index;
	int ret;
	int state;
	long speed = param->speed; 
	

	ret = nbm_get_fan_present(fan_index, &state);
	if (ret != 0)
	{	
		npd_syslog_err("get power index %d present state error.\n", fan_index);
		return -1;
	}
	if (state == FAN_INSERT)
	{
		param->inserted = TRUE;
	}
	else
	{
		param->inserted = FALSE;
	}

	if (param->inserted != TRUE)
	{
		npd_syslog_err("fan index %d removed. user default info.\n", fan_index);
		return -1;
	}

	
	ret = nbm_get_fan_state(fan_index, &state);
	if (ret != 0)
	{	
		npd_syslog_err("get power index %d work state error.\n");
		return -1;
	}
	if (state == FAN_NORMAL)
	{
		param->status = FAN_NORMAL;
	}
	else
	{
		param->status = FAN_ALARM;
	}

	ret = nbm_set_fan_speed(fan_index, speed); /* */
	if (ret != 0)
	{	
		npd_syslog_err("set fan index %d speed error.\n");
		return -1;
	}
	
	return 0;
}

int nh_read_sysinfo_file(char * filename, char * buf, int len)
{
	int fd;
	int size = 0;

	fd = open(filename,O_RDONLY);
	if (fd < 0)
	{
		npd_syslog_err("open file error.\n", filename);
		return NPD_FAIL;
	}
	size = read(fd,buf,len);
	if (size < 0)
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	if (NULL != strstr(buf, "error"))
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	if(strncmp(buf, "BLANKDEVINFO", strlen("BLANKDEVINFO")) == 0)
	{
	    close(fd);
		return NPD_FAIL;
	}
	
	close(fd);
	return NPD_SUCCESS;	
}

int nh_board_mnparam_get(board_man_param_t *info)
{
	char mod_name[32];
	char mod_sn [32];
	char * ptr;
	int ret = 0;
	int hw_code;
	
	hw_code = nbm_get_module_hw_code();
	if (-1 == hw_code)
	{
		npd_syslog_err("get module hwcode error.\n");
		return NPD_FAIL;
	}

	ret = nh_read_sysinfo_file(OEM_MODULE_NAME_FILE,mod_name, 32);
	if (ret != NPD_SUCCESS)
	{
		ret = nh_read_sysinfo_file(MODULE_NAME_FILE,mod_name, 32);
		if (ret != NPD_SUCCESS)
		{
			npd_syslog_err("get module name error.\n");
			//return ret;
		}
	}
	
	ptr = (char *)memchr(mod_name, '\n', 32);  
	if(ptr)
	{
	    *ptr = '\0';
	}
	strncpy(info->modname, mod_name, strlen(mod_name));
	info->modname[strlen(mod_name)] = '\0';
	npd_syslog_dbg("modname is %s.\n",info->modname);

	
	ret = nh_read_sysinfo_file(MODULE_SN_FILE, mod_sn, 32);
	if (ret != NPD_SUCCESS)
	{
		npd_syslog_err("get module sn error.\n");
		return ret;
	}
	ptr = (char *)memchr(mod_sn, '\n', 32);
	if(ptr)
	{
	    *ptr = '\0';
	}
	strncpy(info->sn, mod_sn, strlen(mod_sn));
	info->sn[strlen(mod_sn)] = '\0';
	npd_syslog_dbg("sn is %s.\n",info->sn);

	return NPD_SUCCESS;
}

void nh_npd_master_set(unsigned long value)
{
	nbm_master_set(value);
}

void nh_int_power_handler(unsigned char int_value, unsigned char add_value)
{
	npd_int_data_ctrl power_int_data_array[] = 
	{
		{0x01, 0x01},
		{0x02, 0x02},
		{0x04, 0x04},
		{0x08, 0x08},				
	};
	npd_int_data_ctrl* int_data_ctl = NULL ;
	int index = 0;
	power_supply_man_param_t * power_man_array = productinfo.power_supply_param;
	power_info_args power_info;
	int_op_t int_op;
	int op_ret;
	int mask =  INT_OP_PSINDEX_MASK;
	int shift = INT_OP_PSINDEX_SHIFT;
	memset(&int_op, 0, sizeof(int_op_t));
	
	npd_syslog_dbg("Enter nh_int_power_handler.\n");
	
	for (index = 0 ; index < LENGTH(power_int_data_array); index++)
	{
		int_data_ctl = &power_int_data_array[index];
		if (int_value & int_data_ctl->int_mask) //unit interrupt
		{	
			int_op.param |= (index & mask) << shift;  
			if (add_value & int_data_ctl->add_mask) // unit removed
			{
				//process the inserted true
				power_man_array[index].inserted = FALSE;
				npd_syslog_dbg("the power unit %d removed.\n", index+1);
				int_op.param &= ~(INT_OP_INSERT);      
				op_ret = nbm_interrupt_operate(&int_op);
				memset(power_man_array[index].name, 0, 20);
				
			}
			else		      // unit inserted
			{
				//process the inserted true
				power_man_array[index].inserted = TRUE;
				npd_syslog_warning("the power unit %d inserted.\n", index+1);
				int_op.param |= INT_OP_INSERT;
				op_ret = nbm_interrupt_operate(&int_op);
				if (op_ret != 0)
				{
					return;
				}
				power_info.index = index;
				op_ret = nbm_get_power_info(index, &power_info);
				if (op_ret != 0)
				{	
					npd_syslog_err("get power index %d info error.\n", index);
					return;
				}
				memcpy(power_man_array[index].name, power_info.name, 20);
				npd_syslog_dbg("power %d name is %s.\n", index, power_man_array[index].name);				
				
			}		
			
		}
	}
}


#ifdef __cplusplus
}
#endif

