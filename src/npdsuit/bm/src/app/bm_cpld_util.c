#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <util/npd_list.h>

#include "npd/nbm/npd_cplddef.h"
#include "bm_cpld_util.h"
#include "bm_nbm_api.h"


//#include "bmk_main.h"

int debug_flag=0;

u8		data_dev_addr = 0xff;
u8		data_dev_off = 0xff;
u8		data_dev_data0 = 0x00;
u8		data_dev_data1 = 0x00;
u16		data_bit_wide = 0x00;
u16		data_reg_addr = 0xffff;
unsigned long 	register_data = 0xffff;
unsigned int reg_addr = 0xffffffff;
unsigned int reg_val  = 0xffffffff;

int poe_port_data = 0;
int poe_port_speed = 0;
int poe_port_state = 0;
unsigned char event_type = 0x00;
unsigned char wid     = 0x00;

char	data_buf[200] = {0};
int 	data_len = 1;

char 	env_value[128];
char 	env_name[128];


//u16		data_reg16_addr = 0xffff;


typedef struct _cmd_operate_object_
{
	int 	cmd_code;
	char* 	cmd_name;
	int 	(*cmd_func)(long);
	int 	cmd_data; 
	char* 	cmd_desc;


}cmd_operate_object;


#define LENGTH(x)	(sizeof(x)/sizeof(x[0]))

#define DESC_GET_ALL "Get all function info;Not need value to set"
#define DESC_SET_WDT_ENABLE	"Set watchdog enable state; 1-enable, 0-disable."
#define DESC_SET_WDT_TIMEOUT "Set watchdog timeout."
#define DESC_CLEAR_WDT 		"Clear watchdog;Write any value"
#define DESC_RESET_BOARD	"Reset board;Write board slot."
#define DESC_READ_EEPROM	"Read eeprom content."
#define DESC_WRITE_EEPROM	"Write eeprom content."
#define DESC_READ_I2C_ADDR8 "Read byte from I2C device which reg offset is 8-bit"


#define DESC_WRITE_DEVINO	"Write devinfo to eeprom. write'devinfo.TXT'"

#define DESC_SFP_LIGHT		"set sfp port light state. \
\n\t\t\t\t\t\t-a set the port index \
\n\t\t\t\t\t\t-v 1-light on, 0-light off."

#define DESC_SFP_WRITE		"write data to sfp rom.\
\n\t\t\t\t\t\t-p set the port index \
\n\t\t\t\t\t\t-a address, the dev address. \
\n\t\t\t\t\t\t-c command, the reg offset \
\n\t\t\t\t\t\t-v value to write \
\n\t\t\t\t\t\t-l buf length."
			 
#define DESC_SFP_READ		"read data from sfp rom. \
\n\t\t\t\t\t\t-p set the port index. \
\n\t\t\t\t\t\t-a address, the dev address. \
\n\t\t\t\t\t\t-c command, the reg offset. \
\n\t\t\t\t\t\t-l read buf length."

#define DESC_TEMP_THRESHOLD	"set temp threshold. \
\n\t\t\t\t\t\t-c \[op_type\]  \
\n\t\t\t\t\t\t    0-MOD_HILIM  \
\n\t\t\t\t\t\t    1-MOD_LOLIM  \
\n\t\t\t\t\t\t    2-CORE_HILIM \
\n\t\t\t\t\t\t    3-CORE_LOLIM \
\n\t\t\t\t\t\t-v \[op_value]."


cmd_operate_object cmd_op ;


char * product_type_str[] = 
{
	"PRODUCT_DUMMY",
    "T9010",
    "T9006",
    "T9003",
    "3052",
    "3028",
    "AS6603",
    "AS3200",
    "AX8603",
    "AU3200",
    "T9014",    
	"MAX_NUM"
};

char * board_type_str[] = 
{
	"NH_NONE",
    "TSM9002",
    "TSM9024FC",
    "TGM9048",
    "TGM9024",
    "TXM9004",
    "NH_3052",
    "NH_3028",
    "ASG6648",
    "ASG6624C",
    "ASG6624CE",
	"CGM9048",
	"FW9001",    
    "AS3200-24GC",
    "AS3200-9GC-PWR",
    "AS3200-28GC-PWR",
    "AS3200-24GC-PWR",   
    "TYPE_MAX"
};

char * led_type_str[] =
{
	"LED_FAN",
	"LED_POWER",
	"LED_TEMP",
	"LED_LINECARD",
	"LED_SFP",	
	"LED_MASTER",
	"LED_TYPE_MAX",
};

char * led_insert_str[] =
{
	"ON",
	"OFF"
};

char * led_state_str[] =
{
	"GREEN",
	"YELLOW"
};

char *counter_type[32] = {"InGoodOctetsLo", "InGoodOctetsHi", "InBadOctets",
						  "OutFCSErr", "InUnicast", "Deferred",
						  "InBoardcasts", "InMulticasts", "64Octets",
						  "65 to 127Octets", "128 to 255Octets", "256 to 511Octets",
						  "512 to 1023 Octets", "1024 to MaxOctets", "OutOctetsLo",
						  "OutOctetsHi", "OutUnicast", "Excessive",
						  "OutMulticasts", "OutBoardcasts", "Single",
						  "OutPause", "InPause", "Multiple",
						  "InUndersize", "InFragments", "InOversize",
						  "InJabber", "In RxErr", "InFCSErr", 
						  "Collisions", "Late"};


/* ****************************************
  *   cmd function
  *****************************************/

int show_sfp_info(int sfp_num)
{
	int index = 0;
	int sfp_param_state = 0;
	for(index = 0; index < sfp_num; index++)
	{
		//check the light state
		if (0 == nbm_sfp_light_get(index, &sfp_param_state))
		{
			printf("\tsfp port %d light state is %s.\n", index,
				(sfp_param_state == SFP_LIGHT_ON)? "ON" : "OFF" );
		}
		else
		{
			printf("\tsfp port %d light state error.\n", index);
		}
		
		//check the presense state
		if (0 == nbm_sfp_presence_get(index, &sfp_param_state))
		{
			printf("\tsfp port %d is %s.\n", index,
				(sfp_param_state == SFP_ONLINE)? "PRESENSE" : "ABSENSE" );
		}
		else
		{
			printf("\tsfp port %d presense state error.\n", index);
		}			

		//check the cdr state
		if (0 == nbm_xfp_cdr_state_get(index, &sfp_param_state))
		{
			printf("\tsfp port %d is %s.\n", index,
				(sfp_param_state == XFP_CDR_LOCKED)? "LOCKED" : "LOCKED" );
		}
		else
		{
			printf("\tsfp port %d cdr state error.\n", index);
		}			
		
		//check the los state
		if (0 == nbm_sfp_dmi_LOS_get(index, &sfp_param_state))
		{
			printf("\tsfp port %d LOS state is %s.\n", index,
				(sfp_param_state == SFP_DMI_NORMAL)? "NORMAL" : "ALARM" );
		}
		else
		{
			printf("\tsfp port %d LOS state error.\n", index);
		}		
		
		//check the Tx_fault state
		if (0 == nbm_sfp_dmi_Tx_fault_get(index, &sfp_param_state))
		{
			printf("\tsfp port %d Tx_fault state is %s.\n", index,
				(sfp_param_state == SFP_DMI_NORMAL)? "NORMAL" : "ALARM" );
		}
		else
		{
			printf("\tsfp port %d Tx_fault state error.\n", index);
		}								
	}

	return 0;
}

int get_cpld_info(long data)
{
	//get product type
	{
		int product_type;
		int product_code;
		
		product_type = nbm_get_product_type();
		product_code = nbm_get_product_hw_code();
		printf("Product Type : %s - code is %0x.\n", product_type_str[product_type],product_code);
	}

	//get board type
	{
		int module_type;
		int module_code;
		
		module_type = nbm_get_module_type();
		module_code = nbm_get_module_hw_code();
		printf("Board Type : %s - code is %0x.\n", board_type_str[module_type], module_code);		
	}

	//get board version
	{
		int board_hw_version;
		board_hw_version = nbm_get_board_hw_version();
		printf("Board Version : %d .\n", board_hw_version);

	}

	//get backboard_version
	{
		int backboard_version;
		backboard_version = nbm_get_backboard_version();
		printf("Backboard Version : %d .\n", backboard_version);
	}

	//watchdog info
	{
		unsigned int watchdog_state;

		nbm_hardware_watchdog_control_get(&watchdog_state);
		if (watchdog_state == SYSTEM_HARDWARE_WATCHDOG_ENABLE)
		{
			unsigned int wtd_timeout;
			nbm_hardware_watchdog_timeout_get(&wtd_timeout);

			printf("Watch dog is enable and time is %d.\n", wtd_timeout);
		}
		else
		{
			printf("Watch dog is disable.\n");
		}
	}

	//current slot no
	{
		int slot_index = 0;
		slot_index = nbm_slotno_get();
		printf("current slotno is %d.\n", slot_index + 1);
	}

	//get master slot
	{
		int master_slot_index = 0;
		master_slot_index = nbm_master_slotno_get();
		printf("master slot number is %d.\n" , master_slot_index+1);
	}

	//board online info
	{
		int slot_num = nbm_board_num_get();
		printf("board number is %d.\n", slot_num);
		if (slot_num > 0)
		{
			int slot_index;
			int online_state;

			printf("Board Online info :\n");
			for (slot_index = 0; slot_index < slot_num; slot_index++)
			{
				online_state = nbm_board_online(slot_index);
				
				printf("\tslot %d is %s.\n", slot_index + 1, 
					(online_state ? "online" : "removed"));
			}	

		}
	}

	//sfp_test
	{
		int sfp_num = nbm_sfp_num_get();
		printf("sfp number is %d.\n", sfp_num);		
		if (sfp_num > 0)
		{
			show_sfp_info(sfp_num);
		}	
	}	

	return 0;

}

int set_watchdog_enable(long enable_value)
{
	int enable_state = enable_value;
	if (enable_state == 1)
	{
		nbm_hardware_watchdog_control_set(SYSTEM_HARDWARE_WATCHDOG_ENABLE);
		printf("set watchdog enable.\n");
	}
	else
	{
		nbm_hardware_watchdog_control_set(SYSTEM_HARDWARE_WATCHDOG_DISABLE);
		printf("set watchdog disable.\n");
		
	}
	
	return 0;
}

int set_watchdog_timeout(long timeout_value)
{
	int timeout = (int)timeout_value;
	if (timeout > 0)
	{
		nbm_hardware_watchdog_timeout_set(timeout);
		printf("set watchdog timeout is %d.\n", timeout);
	}
	else
	{
		printf("value is out of range.\n");
	}
	
	return 0;
}

int reset_board(long slot_no)
{
	int ret;
	int slot_index = slot_no - 1;
	ret = nbm_board_reset(slot_index);
	if (ret)
	{
		printf("reset board %ld is fail.\n", slot_no);
	}
	else
	{
		printf("reset board %ld is success.\n", slot_no);
	}

	return ret;
}


int clear_watchdog(long clear_value)
{
	int ret;
	
	ret = nbm_hardware_watchdog_fillup();
	if (!ret)
	{
		printf("clear watchdog success.\n");
	}
	else
	{
		printf("clear watchdog fail.\n");
	}
	return ret;
}

int eeprom_write(long value)
{
	int ret;
	char buf[200];
	memset(buf, 0, sizeof(buf));
	buf[0] = (char)value;

	if (data_len > 1)
	{
		int index = 1;
		srand((unsigned)time(NULL));
		for (index = 1; index < data_len; index ++)
		{
			buf[index] = rand() % 255;
		}
	}
	
	ret = nbm_eeprom_write(data_dev_addr, data_reg_addr, buf, &data_len);
	if (!ret)
	{
		printf("write eeprom success .\n" );
	}
	else
	{
		printf("write eeprom fail.\n");
	}
	return ret;
}


int eeprom_read(long value)
{
	int ret;
	char buf[200];
	memset(buf, 0, sizeof(buf));

	ret = nbm_eeprom_read(data_dev_addr, data_reg_addr, buf, &data_len);
	if (!ret)
	{
		printf("read eeprom success .\n" );
	}
	else
	{
		printf("read eeprom fail.\n");
	}
	return ret;
	
}

int tempmon_read(long value)
{
	int ret;

	//ret = nbm_eeprom_read(&value);
	if (!ret)
	{
		printf("read eeprom value %ld.\n", value);
	}
	else
	{
		printf("read eeprom fail.\n");
	}
	return ret;
	
}

int i2c_byte_read_addr8(long value)
{
	int ret;

	data_reg_addr &= 0xff;		// get the 8-bit reg offset addr
	if (data_dev_addr == 0xff)
	{
		printf("specific dev_addr wrong.\n");
		return -1;
	}

	if (data_reg_addr == 0xff)
	{
		printf("specific reg_addr wrong.\n");
		return -1;
	}

	

	ret = nbm_i2c_read_byte(&value, data_dev_addr, data_reg_addr);
	if (!ret)
	{
		printf("read i2c device value %ld.\n", value);
	}
	else
	{
		printf("read i2c device fail.\n");
	}
	return ret;
	

}

int i2c_byte_read_addr16(long value)
{
	int ret;

	if (data_dev_addr == 0xff)
	{
		printf("specific dev_addr wrong.\n");
		return -1;
	}

	if (data_reg_addr == 0xff)
	{
		printf("specific reg_addr wrong.\n");
		return -1;
	}
	

	ret = nbm_i2c_read_byte(&value, data_dev_addr, data_reg_addr);
	if (!ret)
	{
		printf("read i2c device value %ld.\n", value);
	}
	else
	{
		printf("read i2c device fail.\n");
	}
	return ret;
	

}

int write_devinfo(long value)
{
	char * name_buf[50]; 
	char * val_buf[50];
	char  file_buf[1000];
	char * sys_buf;
	int sys_buf_len;
	int ret;
	int elem_count;
	int rlen = 1000;

	rlen = read_file_buf("devinfo.TXT", file_buf, &rlen);
	if (rlen < 0)
	{
		printf("read file count %d error.", rlen);
		return -1;
	}
	
	file_buf[rlen] = '\0';
	
	content_token(file_buf, name_buf, val_buf, &elem_count);
	
	sys_buf = gen_sysinfo_buf(name_buf, val_buf, elem_count, &sys_buf_len);
	data_len = sys_buf_len;
	if (0xff == data_dev_addr)
	{
		printf("not specify eeprom to write devinfo.\n");
		return 0;
	}

	if (0xffff == data_reg_addr)
	{
		data_reg_addr = 0;
	}

	if (sys_buf != NULL)
	{
		//write_file_buf("sysinfo", sys_buf, sys_buf_len);
		ret = nbm_eeprom_write(data_dev_addr, data_reg_addr, sys_buf, &data_len);
		if (!ret)
		{
			printf("write eeprom success .\n" );
		}
		else
		{
			printf("write eeprom fail.\n");
		}
		
		free(sys_buf);			
	}

	

	return 0;
	
}

int sfp_light_set(long value)
{
	int light_state  = 0;
	int sfp_index = (int)data_dev_addr;
	int ret;
	
	light_state = (value ? SFP_LIGHT_ON : SFP_LIGHT_OFF);
	
	printf("before sfp light set.\n");
	ret = nbm_sfp_light_set(sfp_index, light_state);
	if (ret)
	{	
		printf("set sfp index %d fail.\n", sfp_index);
	}
	
	return 0;
}

int sfp_write(long value)
{
	int sfp_param_state;
	int sfp_index = poe_port_data;
	int sfp_dev_addr = data_dev_addr;
	int sfp_reg_offset = data_reg_addr;
	int len = data_len;
	char *buf;
	char *temp;
	int ret = 0;

	if (0 == nbm_sfp_presence_get(sfp_index, &sfp_param_state))
	{
		printf("\tsfp port %d is %s.\n", sfp_index,
			(sfp_param_state == SFP_ONLINE)? "PRESENSE" : "ABSENSE" );

		if (sfp_param_state != SFP_ONLINE)
		{
			return -1;
		}
	}
	else
	{
		printf("\tsfp port %d presense state error.\n", sfp_index);
		return -1;
	}			
	
	
	buf = (char *)malloc(len);
	if (buf == NULL)
	{
		printf("there is no memory at all.\n");
		return -1;
	}
	memset(buf, 0, len);
	
	temp = buf;
	while(len--)
	{
		*temp++ = value;
	}
	
	ret = nbm_sfp_write(sfp_index, sfp_dev_addr, sfp_reg_offset, buf, len);
	if(ret)
	{
		printf("write sfp index %d error.\n", sfp_index);
		return -1;
	}
	
	if (buf)
	{
		free(buf);
		buf = NULL;
	}
	
	return 0;
}

int sfp_read(long value)
{
	int sfp_param_state;
	int sfp_index = poe_port_data;
	int sfp_dev_addr = data_dev_addr;
	int sfp_reg_offset = data_reg_addr;
	int len = data_len;
	char *buf;
	int index;
	int ret = 0;

	if (0 == nbm_sfp_presence_get(sfp_index, &sfp_param_state))
	{
		printf("\tsfp port %d is %s.\n", sfp_index,
			(sfp_param_state == SFP_ONLINE)? "PRESENSE" : "ABSENSE" );

		if (sfp_param_state != SFP_ONLINE)
		{
			return -1;
		}
	}
	else
	{
		printf("\tsfp port %d presense state error.\n", sfp_index);
		return -1;
	}			


	
	buf = (char *)malloc(len);
	if (buf == NULL)
	{
		printf("there is no memory at all.\n");
		return -1;
	}
	memset(buf, 0, len);
	//printf("buf value is %x. \n", (int)buf); /* */
	ret = nbm_sfp_read(sfp_index, sfp_dev_addr, sfp_reg_offset, buf, len);
	if(ret)
	{
		printf("read sfp index %d error.\n", sfp_index);
		return -1;
	}
	
	printf("read value is:\n");
	
	for (index = 0; index < len; index++)
	{
		if (index != 0 && (index % 16) == 0)
		{
			printf("\n");
		}

		printf("%02hhx ", buf[index]);
	}
	printf("\n");
	
	if (buf)
	{
		free(buf);
		buf = NULL;
	}
	
	return 0;
}

int temp_info(long value)
{
	int ret;
	temp_info_args temp_data = {0};

	ret = nbm_get_temp_info(&temp_data);
	if (ret != 0)
	{
		printf("show temp info error.");
		return -1;
	}
	printf("module temp %d C.\n", temp_data.module_temp);
	printf("module temp high limit %d C.\n", temp_data.module_high_limit);
	printf("module temp low limit %d C.\n", temp_data.module_low_limit);
	printf("core temp %d C.\n", temp_data.core_temp);
	printf("core temp high limit %d C.\n", temp_data.core_high_limit);
	printf("core temp low limit %d C.\n", temp_data.core_low_limit);
	
	return 0;
}

char * temp_state_str[] = 
{
	"TEMP_NORMAL",
	"TEMP_MODULE_BEYOND",
	"TEMP_MODULE_UNDERNEATH",
	"TEMP_CORE_BEYOND",
	"TEMP_CORE_UNDERNEATH",
	"TEMP_OPEN_CIRCUIT",	
};

int temp_state(long value)
{
	int ret;
	int temp_state = -1;

	ret = nbm_get_temp_state(&temp_state);
	if (ret != 0)
	{
		printf("show temp state error.\n");
		return -1;
	}
	printf("temp state is %s .\n", temp_state_str[temp_state]);
	return 0;

}

int temp_threshold(long value)
{
	int ret;
	temp_op_args temp_op = {0};

	temp_op.op_type = data_reg_addr;
	temp_op.value = value;
	
	DBG("set op_type is %d, value is %d.\n", temp_op.op_type, temp_op.value);
	ret = nbm_set_temp_threshold(&temp_op);
	if (ret != 0)
	{
		printf("show temp state error.\n");
		return -1;
	}
	//printf("temp state is %s .\n", temp_state_str[temp_state]);
	return 0;


}

int power_info(long value)
{
	int ret;
	power_info_args power_info;
	int ps_index = value;
	int inserted;
	int state;
	
	ret = nbm_get_power_present(ps_index, &inserted);
	if (ret != 0)
	{	
		printf("get power index %d present state error.\n", ps_index);
		return -1;
	}
	printf("power supply index %d is %s.\n", ps_index, 
			inserted == POWER_INSERT ? "INSERT" : "REMOVED");

	if (inserted != POWER_INSERT)
		return 0;

	power_info.index = ps_index;
	ret = nbm_get_power_info(ps_index, &power_info);
	if (ret != 0)
	{	
		printf("get power index %d info error.\n", ps_index);
		return -1;
	}
	printf("power supply info:\n");
	printf("name - %s.\n", power_info.name);

	ret = nbm_get_power_state(ps_index, &state);
	if (ret != 0)
	{	
		printf("get power index %d work state error.\n", ps_index);
		return -1;
	}
	printf("power supply index state %d is %s.\n", ps_index, 
			state == POWER_NORMAL ? "NORMAL" : "ALARM");
	
	return 0;
	
}

int power_present(long value)
{
	int ret;
	int state;
	int ps_index = value;
	
	ret = nbm_get_power_present(ps_index, &state);
	if (ret != 0)
	{	
		printf("get power index %d present state error.\n", ps_index);
		return -1;
	}	
	return 0;
}

int fan_info(long value)
{
	int ret;
	int fan_index = value;
	int inserted;
	int state;
	int speed;

	fan_index = 0;
	
	ret = nbm_get_fan_present(fan_index, &inserted);
	if (ret != 0)
	{	
		printf("get fan index %d present state error.\n", fan_index);
		return -1;
	}
	printf("fan index %d is %s.\n", fan_index, 
			inserted == FAN_INSERT ? "INSERT" : "REMOVED");

	if (inserted != FAN_INSERT)
		return 0;

	ret = nbm_get_fan_state(fan_index, &state);
	if (ret != 0)
	{	
		printf("get fan %d work state error.\n", fan_index);
		return -1;
	}
	printf("fan index state %d is %s.\n", fan_index, 
			state == FAN_NORMAL ? "NORMAL" : "ALARM");

	ret = nbm_get_fan_speed(fan_index, &speed);
	if (ret != 0)
	{	
		printf("get power index %d work state error.\n", speed);
		return -1;
	}
	printf("fan index %d speed is %d.\n", fan_index, speed);	

	return 0;
	
}

int fan_speed(long value)
{
	int speed = (int)value;
	int ret;
	int fan_index = 0;
	

	printf("before fan set.\n");
	ret = nbm_set_fan_speed(fan_index, speed);
	if (ret)
	{	
		printf("set fan index %d fail.\n", fan_index);
	}
	printf("fan speed set %d success.\n", speed);

	ret = nbm_get_fan_speed(fan_index, &speed);
	if (ret != 0)
	{	
		printf("get fan index %d work state error.\n", fan_index);
		return -1;
	}
	printf("fan index %d speed now is %d.\n", fan_index, speed);	
	
	
	return 0;

}


int power_state(long value)
{
	int ret;
	int ps_index = value;
	int state;
	
	ret = nbm_get_power_state(ps_index, &state);
	if (ret != 0)
	{	
		printf("get power index %d work state error.\n", ps_index);
		return -1;
	}
	printf("power supply index state %d is %s.\n", ps_index, 
			state == POWER_NORMAL ? "NORMAL" : "ALARM");
	

	return 0;
	
}



int led_help(long value)
{
	int i;
	
	printf("LED CONTORL HELP.\n");
	printf("-----------------------------.\n");
	printf("-a Specify the op type. \n");
	for (i = 0; i < LED_TYPE_MAX; i++)
	{
		printf("	%s-%d.\n", led_type_str[i], i);
	}
	/*
	printf("	%s-%d, %s-%d, %s-%d, %s-%d, LINECARD-%d.\n",
					LED_FAN, LED_POWER, LED_TEMP, LED_SFP, LED_LINECARD);
	*/
	printf("                             \n");
	printf("-c Specify the index of led. used int LINECARD led control.\n");
	printf("                             \n");	
	printf("-v Specify the led status.\n");
	printf("    INSERT & NORMAL - 3 \n");
	printf("    INSERT & ALARM  - 1 \n");
	printf("    REMOVE          - 0 \n");
	
	return 0;
}

int show_led_operate(int ret, led_op_args led_op)
{
	if (ret != 0)
	{
		printf("led operate error.\n");
	}
	else
	{
		printf("Type:%15s, Index:%3d, ", led_type_str[led_op.op_type], led_op.index);
		printf("LED:%3s ", led_insert_str[led_op.inserted]);
		if (led_op.inserted == LED_STATUS_INSERT)
		{
			printf("%s", led_state_str[led_op.state]);
		}
		printf("\n");
	}
	return 0;
}

int led_info(long value)
{
	led_op_args led_op = {0};
	int ret = 0;
	int index = 0;	
	int linecard_cnt = 0;
	int sfp_cnt = 0;

	linecard_cnt = nbm_board_num_get()- 2;
	if (linecard_cnt < 0)
	{
		printf("get linecard number error.\n");
		return -1;
	}

	sfp_cnt = nbm_sfp_num_get();
	if (sfp_cnt < 0)
	{
		printf("get linecard number error.\n");
		return -1;
	}

	//get fan led state;
	led_op.op_type = LED_FAN;
	ret = nbm_get_led_info(&led_op);
	show_led_operate(ret, led_op);

	//get power led state;
	led_op.op_type = LED_POWER;
	ret = nbm_get_led_info(&led_op);
	show_led_operate(ret, led_op);
	
	//get temp state
	led_op.op_type = LED_TEMP;
	ret = nbm_get_led_info(&led_op);
	show_led_operate(ret, led_op);
	
	//get line card
	led_op.op_type = LED_LINECARD;
	for (index = 0; index < linecard_cnt; index++)
	{
		led_op.index = index;
		ret = nbm_get_led_info(&led_op);
		show_led_operate(ret, led_op);
	}

	//get sfp
	led_op.op_type = LED_SFP_SPEED;
	for (index = 0; index < sfp_cnt; index++)
	{
		led_op.index = index;
		ret = nbm_get_led_info(&led_op);
		show_led_operate(ret, led_op);	
	}

	led_op.op_type = LED_MASTER;
	ret = nbm_get_led_info(&led_op);
	show_led_operate(ret, led_op);
	
}

int led_control(long value)
{
	int op_type = data_dev_addr;
	int op_index = data_reg_addr;
	int op_value = value;
	led_op_args led_op = {0};
	int ret = 0;

	led_op.op_type = op_type;
	led_op.index = op_index;
	
	if (value & 0x01)
	{
		led_op.inserted = LED_STATUS_INSERT;
		if (value & 0x02)
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
	ret = nbm_led_control(&led_op);
	if (ret != 0)
	{
		printf("led operate error.\n");
		return -1;
	}
	printf("Type:%s, Index:%d, ", led_type_str[led_op.op_type], led_op.index);
	printf("LED:%s ", led_insert_str[led_op.inserted]);
	if (led_op.inserted == LED_STATUS_INSERT)
	{
		printf("%s", led_state_str[led_op.state]);
	}
	printf("\n");
		
	return 0;
}

int boot_version(long value)
{
	char name[128];
	int op_ret;

	op_ret = nbm_get_boot_version_name(name);
	if (op_ret != 0)
	{
		printf("get boot version name error.\n");
		return -1;
	}
	printf("boot version name is:\n");
	printf("%s\n",name);

	printf ("boot version name len is %d.\n", strlen(name));
	return 0;
}

int backplane_info(long value)
{
	struct product_sysinfo_s product_sysinfo;
	int op_ret;
	
	op_ret = nbm_read_backplane_sysinfo(&product_sysinfo) ;
	if (op_ret != 0)
	{
		printf("get backplance info error.\n");
		return -1;
	}
	printf ("get backplance info success.\n");
	return 0;
}

int kenel_debug(long value)
{
	char name[128];
	int op_ret;

	op_ret = nbm_kernel_debug(value);
	if (op_ret != 0)
	{
		printf("set kernel debug name error.\n");
		return -1;
	}
	printf("set kernel debug level success.\n");
	return 0;
}

int master_get(long value)
{
	int op_ret;

	op_ret = nbm_master_get(value);
	if (op_ret == -1)
	{
		printf("get master flag error.\n");
		return -1;
	}
	printf("get master flag is %s\n", op_ret ? "MASTER" : "SLAVE");
	return op_ret;
	
}

int master_set(long value)
{
	int op_ret;

	op_ret = nbm_master_set(value);
	if (op_ret == -1)
	{
		printf("get master flag error.\n");
		return -1;
	}
	printf("set master flag success\n");
	return op_ret;
	
}

int master_set_debug(long value)
{
	int op_ret;

	printf("value = %s\n", (value == 1) ? "Master" : "Slaver");
	
	op_ret = nbm_master_set_debug(value);
	if (op_ret == -1)
	{
		printf("set master [debug] flag error.\n");
		return -1;
	}
	printf("set master flag [debug] success\n");
	return op_ret;
}

int i2c_twsi1_read(long value)
{
	int ret;
	char buf[200];
	memset(buf, 0, sizeof(buf));

	ret = nbm_eeprom_read_one(data_dev_addr, data_reg_addr, buf, &data_len);
	if (!ret)
	{
		printf("read eeprom success .\n" );
	}
	else
	{
		printf("read eeprom fail.\n");
	}
	return ret;

}

int op_boot_env(long value)
{
	boot_env_t env_args;
	int op_ret;
	
	strcpy(env_args.name, env_name);
	memset(env_args.value, 0, sizeof(env_args.value));

	if (value == 1)
	{
		env_args.operation = GET_BOOT_ENV;
	}
	else if (value == 2)
	{
		strcpy(env_args.value, env_value);
		env_args.operation = SAVE_BOOT_ENV;
	}

	op_ret = nbm_op_boot_env(&env_args);
	if (op_ret != 0)
	{
		printf("operat boot_env error!.\n");
		return -1;
	}
	
	printf("%s = ", env_args.name);
	printf("%s\n", env_args.value);
	printf("env len is %d.\n", strlen(env_args.value));
	
	return 0;
}

int mv_switch_read(long value)
{
	sw_op_args sw_op_data;
	int op_ret;

	memset(&sw_op_data, 0, sizeof(sw_op_args));
	sw_op_data.operation = SW_READ_REGISTER;
	sw_op_data.dev_addr = data_dev_addr;
	sw_op_data.reg_addr = data_reg_addr;

	printf("dev = %x\n", sw_op_data.dev_addr);
	printf("reg = %x\n", sw_op_data.reg_addr);
	op_ret = nbm_mv_switch_op(&sw_op_data);
	if (op_ret != 0)
	{
		printf("read switch register error!.\n");
		return -1;
	}

	printf("data= %x\n", sw_op_data.data);
	
	return 0;
}

int mv_switch_write(long value)
{
	sw_op_args sw_op_data;
	int op_ret;

	memset(&sw_op_data, 0, sizeof(sw_op_args));
	sw_op_data.operation = SW_WRITE_REGISTER;
	sw_op_data.dev_addr = data_dev_addr;
	sw_op_data.reg_addr = data_reg_addr;
	sw_op_data.data = register_data;

	printf("dev = %x\n", sw_op_data.dev_addr);
	printf("reg = %x\n", sw_op_data.reg_addr);
	printf("data= %x\n", sw_op_data.data);
	
	op_ret = nbm_mv_switch_op(&sw_op_data);
	if (op_ret != 0)
	{
		printf("write switch register error!.\n");
		return -1;
	}

	return 0;
}

int mv_switch_counter(long value)
{
	sw_counter_args sw_counter_data;
	int op_ret, i, j;

	memset(&sw_counter_data, 0, sizeof(sw_counter_args));
	sw_counter_data.port = data_dev_addr;
		
	op_ret = nbm_mv_switch_counter(&sw_counter_data);
	if (op_ret != 0)
	{
		printf("switch stats counter error!.\n");
		return -1;
	}

	for (i = 0, j = 0; i < 32; i++, j+=2)
	{
		printf("%s %x %x\n", counter_type[i],sw_counter_data.stats_buf[j+1], sw_counter_data.stats_buf[j]);
	}
	
	return 0;
}

int bcm5396_switch_reg_read(long value)
{
	bcm5396_sw_op_args sw_op_data;
	int op_ret;

	memset(&sw_op_data, 0, sizeof(sw_op_args));
	sw_op_data.operation = SW_READ_REGISTER;
	sw_op_data.dev_addr = data_dev_addr;
	sw_op_data.reg_addr = data_reg_addr;

	printf("page = %x\n", sw_op_data.dev_addr);
	printf("reg = %x\n", sw_op_data.reg_addr);
	op_ret = nbm_bcm5396_switch_op(&sw_op_data);
	if (op_ret != 0)
	{
		printf("read switch register error!.\n");
		return -1;
	}
	printf("data[0]= %x\n", sw_op_data.data[0]);
	printf("data[1]= %x\n", sw_op_data.data[1]);
	printf("data[2]= %x\n", sw_op_data.data[2]);
	printf("data[3]= %x\n", sw_op_data.data[3]);
	
	return 0;
}

int bcm5396_switch_reg_write(long value)
{
	bcm5396_sw_op_args sw_op_data;
	int op_ret;
    unsigned short tmp[4];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, &register_data, sizeof(tmp));
	memset(&sw_op_data, 0, sizeof(sw_op_args));
	sw_op_data.operation = SW_WRITE_REGISTER;
	sw_op_data.dev_addr = data_dev_addr;
	sw_op_data.reg_addr = data_reg_addr;
	sw_op_data.data[0] = tmp[0];
	sw_op_data.data[1] = tmp[1];
	sw_op_data.data[2] = tmp[2];
	sw_op_data.data[3] = tmp[3];


	printf("page = %x\n", sw_op_data.dev_addr);
	printf("reg = %x\n", sw_op_data.reg_addr);
	printf("data[0]= %x\n", sw_op_data.data[0]);
	printf("data[1]= %x\n", sw_op_data.data[1]);
	printf("data[2]= %x\n", sw_op_data.data[2]);
	printf("data[3]= %x\n", sw_op_data.data[3]);
	
	op_ret = nbm_bcm5396_switch_op(&sw_op_data);
	if (op_ret != 0)
	{
		printf("write switch register error!.\n");
		return -1;
	}

	return 0;
}

int poe_led(long value)
{
	int ret;
	poe_port_t poe_port;

	poe_port.port = poe_port_data;
	poe_port.speed = poe_port_speed;
	poe_port.state = poe_port_state;

	ret = nbm_poe_led(&poe_port);
	if (ret != 0)
	{
		printf("write register error!.\n");
		return -1;
	}
	
	return 0;

}

#ifdef HAVE_POE
int poe_board_info(long value)
{
	int ret;
	poe_board_t poe_board= {0, 0};

	ret = nbm_poe_board_info(&poe_board);
	if (ret != 0)
	{
		printf("write register error!.\n");
		return -1;
	}

	printf("Poe Board : %s\n", poe_board.present ? "Present" : "Not Present");
	if(poe_board.present) {
		printf("Poe Board Type : %s\n", (poe_board.type == 8) ? "8 Port POE" : 
			(poe_board.type == 24) ? "24 Port POE" : "Unknown");
	}
	
	return 0;

}

#endif
#if 0
int mv_reg_read(long value)
{
	reg_op_args reg_op_data;
	int op_ret;

	memset(&reg_op_data, 0, sizeof(reg_op_args));
	reg_op_data.operation = READ_REGISTER;
	reg_op_data.reg = reg_addr;

	op_ret = nbm_mv_reg_op(&reg_op_data);
	if (op_ret != 0)
	{
		printf("read register error!.\n");
		return -1;
	}

	printf("reg = %x\n", reg_op_data.reg);
	printf("data= %x\n", reg_op_data.data);
	
	return 0;
}

int mv_reg_write(long value)
{
	reg_op_args reg_op_data;
	int op_ret;

	memset(&reg_op_data, 0, sizeof(reg_op_args));
	reg_op_data.operation = WRITE_REGISTER;
	reg_op_data.reg = reg_addr;
	reg_op_data.data = reg_val;

	op_ret = nbm_mv_reg_op(&reg_op_data);
	if (op_ret != 0)
	{
		printf("write register error!.\n");
		return -1;
	}

	printf("reg = %x\n", reg_op_data.reg);
	printf("data= %x\n", reg_op_data.data);
	
	return 0;
}
#endif

int port_isolation_control(long value)
{
	port_isolation_op_args port_isolation_args;
	int op_ret;

	memset(&port_isolation_args, 0, sizeof(port_isolation_args));

	if (data_dev_addr == 1)
	{
		port_isolation_args.op_type= PORT_ISOLATION_ADD;
	}
	else if (data_dev_addr == 2)
	{
		port_isolation_args.op_type= PORT_ISOLATION_DEL;
	}
	port_isolation_args.slot = value;

	op_ret = nbm_port_isolation_control(&port_isolation_args);
	if (op_ret != 0)
	{
		printf("port isolation %s error!.\n", data_dev_addr == 1 ? "Add" : "Delete");
		return -1;
	}
	
	return 0;
}

int xaui_switch(long value)
{
	cpld_mux_args cpld_mux_param;
	int op_ret;

	memset(&cpld_mux_param, 0, sizeof(cpld_mux_args));

	cpld_mux_param.master_slot = value;

	op_ret = nbm_xaui_switch(&cpld_mux_param);
	if (op_ret != 0)
	{
		printf("XAUI Switch error!.\n");
		return -1;
	}
	
	return 0;
}

int rtl8139_read(long value)
{
	rtl8139_op_args rtl8139_op_data;
	int op_ret;

	memset(&rtl8139_op_data, 0, sizeof(rtl8139_op_args));

	rtl8139_op_data.op_type = RTL_READ;
	rtl8139_op_data.chip = data_reg_addr;
	rtl8139_op_data.bitwide = data_bit_wide;
	rtl8139_op_data.reg = register_data;

	op_ret = nbm_rtl8139_operate(&rtl8139_op_data);
	if (op_ret != 0)
	{
		printf("RTL8139 Read error!.\n");
		return -1;
	}

	printf("RTL8139 READ: ");
	printf("reg = %x\tvalue = %x\n", rtl8139_op_data.reg, rtl8139_op_data.value);
	
	return 0;
}

int rtl8139_write(long value)
{
	rtl8139_op_args rtl8139_op_data;
	int op_ret;

	memset(&rtl8139_op_data, 0, sizeof(rtl8139_op_args));

	rtl8139_op_data.op_type = RTL_WRITE;
	rtl8139_op_data.chip = data_reg_addr;
	rtl8139_op_data.bitwide = data_bit_wide;
	rtl8139_op_data.reg = register_data;
	rtl8139_op_data.value = reg_val;

	printf("RTL8139 WRITE: ");
	printf("reg = %x\tvalue = %x\n", rtl8139_op_data.reg, rtl8139_op_data.value);

	op_ret = nbm_rtl8139_operate(&rtl8139_op_data);
	if (op_ret != 0)
	{
		printf("RTL8139 Write error!.\n");
		return -1;
	}
	
	return 0;
}

int wireless_led_op(long value)
{
	wled8713_port_args wled_port_data;
	int op_ret;
	int i;

	memset(&wled_port_data, 0, sizeof(wled_port_args));
/*
	wled_port_data.event_type = event_type;
	wled_port_data.wid = wid;
	wled_port_data.user_num = value;

	printf("event_type = %d\twid = %x\tvalue = %x\n", wled_port_data.event_type, wled_port_data.wid,
			wled_port_data.user_num);
*/

	for(i = 0; i < 24; i++){
		wled_port_data.led_index = i;
		wled_port_data.led_mode = 4;

		op_ret = nbm_wireless_led_operate(&wled_port_data);
		if (op_ret != 0)
		{
			printf("wireless_led_op error!.\n");
			return -1;
		}
		usleep(900000);
	}
	
	return 0;
}

int au4600_subcard_info(void)
{
	music_subcard_info_t subcard_info;
	int op_ret;
	int i;

	for(i = 0; i < 2; i++){
		memset(&subcard_info, 0, sizeof(music_subcard_info_t));
		subcard_info.index = i+1;
		op_ret = nbm_subcard_info_operate(&subcard_info);
		if (op_ret != 0)
		{
			printf("subcard_info error!.\n");
			return -1;
		}

		printf("Subcard info : %d\n", i+1);
		printf("subcard isset : %02x\n", subcard_info.isset);
		printf("Subcard type : %02x\n", subcard_info.type);
		printf("Subcard version : %02x\n", subcard_info.version);
	}
	return 0;
}


int music_i2c_write_op(long value)
{
	music_i2c_dev_args music_i2c_dev_data;
	int op_ret;

	memset(&music_i2c_dev_data, 0, sizeof(music_i2c_dev_args));

	music_i2c_dev_data.addr = data_dev_addr;
	music_i2c_dev_data.off = data_dev_off;
	music_i2c_dev_data.data0 = data_dev_data0;
	music_i2c_dev_data.data1 = data_dev_data1;
	music_i2c_dev_data.count = value;

	printf("addr = %x\toff = %x\tdata0 = %x\tdata1 = %x\tcount = %d\n", music_i2c_dev_data.addr,
																		music_i2c_dev_data.off,
																		music_i2c_dev_data.data0,
																		music_i2c_dev_data.data1,
																		music_i2c_dev_data.count);

	op_ret = nbm_music_i2c_write(&music_i2c_dev_data);
	if (op_ret != 0)
	{
		printf("music_i2c_write error!.\n");
		return -1;
	}
	
	return 0;
}


int qt2225_read(long value)
{
	smi_op_args smi_op_param;
	int op_ret;

	memset(&smi_op_param, 0, sizeof(smi_op_args));

	smi_op_param.op = QT2225_READ;
	smi_op_param.phy_addr = data_dev_addr;
	smi_op_param.dev_addr = data_dev_off;
	smi_op_param.reg_addr = data_reg_addr;

	op_ret = nbm_qt2225_operate(&smi_op_param);
	if (op_ret != 0)
	{
		printf("qt2225_read error!.\n");
		return -1;
	}

	
	printf("QT2225 READ: ");
	printf("phy = %x\tdev = %x\treg = %x\tvalue = %x\n", smi_op_param.phy_addr,
														 smi_op_param.dev_addr,
														 smi_op_param.reg_addr,
														 smi_op_param.data);
	
	return 0;
	
}

int qt2225_write(long value)
{
	smi_op_args smi_op_param;
	int op_ret;

	memset(&smi_op_param, 0, sizeof(smi_op_args));

	smi_op_param.op = QT2225_WRITE;
	smi_op_param.phy_addr = data_dev_addr;
	smi_op_param.dev_addr = data_dev_off;
	smi_op_param.reg_addr = data_reg_addr;
	smi_op_param.data = register_data;

	printf("QT2225 WRITE: ");
	printf("phy = %x\tdev = %x\treg = %x\tvalue = %x\n", smi_op_param.phy_addr,
														 smi_op_param.dev_addr,
														 smi_op_param.reg_addr,
														 smi_op_param.data);

	op_ret = nbm_qt2225_operate(&smi_op_param);
	if (op_ret != 0)
	{
		printf("qt2225_write error!.\n");
		return -1;
	}
	
	return 0;
}

int mux_loop(long value)
{
	cpld_mux_args cpld_mux_param;
	int op_ret;

	memset(&cpld_mux_param, 0, sizeof(cpld_mux_args));

	cpld_mux_param.loopback = value;
	cpld_mux_param.chanel = register_data;

	op_ret = nbm_xaui_loop(&cpld_mux_param);
	if (op_ret != 0)
	{
		printf("XAUI channel loopback error!.\n");
		return -1;
	}
	
	return 0;
}

int cpld_reg_read(long value)
{
	cpld_reg_args cpld_reg_param;
	int op_ret;

	memset(&cpld_reg_param, 0, sizeof(cpld_reg_args));

	cpld_reg_param.reg = register_data;

	printf("Reg = %x\n", cpld_reg_param.reg);

	op_ret = nbm_cpld_register_read(&cpld_reg_param);
	if (op_ret != 0)
	{
		printf("CPLD Register Read error!.\n");
		return -1;
	}

	printf("vlaue = %x\n", cpld_reg_param.value);
	
	return 0;
}

int cpld_reg_write(long value)
{
	cpld_reg_args cpld_reg_param;
	int op_ret;

	memset(&cpld_reg_param, 0, sizeof(cpld_reg_args));

	cpld_reg_param.reg = register_data;
	cpld_reg_param.value = reg_val;

	printf("Reg = %x\n", cpld_reg_param.reg);
	printf("vlaue = %x\n", cpld_reg_param.value);

	op_ret = nbm_cpld_register_write(&cpld_reg_param);
	if (op_ret != 0)
	{
		printf("CPLD Register write error!.\n");
		return -1;
	}

	
	return 0;
}


cmd_operate_object cmd_op_arr[] =
{
	{0, "get_all", 			get_cpld_info, 			-1, DESC_GET_ALL},
	{1, "set_wdt_enable", 	set_watchdog_enable, 	-1, DESC_SET_WDT_ENABLE},
	{2, "set_wdt_timeout", 	set_watchdog_timeout, 	-1, DESC_SET_WDT_TIMEOUT},
	{3, "clear_wdt",		clear_watchdog,			-1, DESC_CLEAR_WDT},
	{4, "reset_board",		reset_board, 			-1, DESC_RESET_BOARD},
	{5, "eeprom_write", 	eeprom_write,			-1, DESC_WRITE_EEPROM},
	{6, "eeprom_read", 		eeprom_read,			-1, DESC_READ_EEPROM},
	{7, "i2c_byte_read",	i2c_byte_read_addr8,	-1, DESC_READ_I2C_ADDR8},
	{8, "write_devinfo", 	write_devinfo,			-1, DESC_WRITE_DEVINO},
	{9, "sfp_light_set", 	sfp_light_set, 			-1, DESC_SFP_LIGHT},
	{10, "sfp_write", 		sfp_write, 				-1, DESC_SFP_WRITE},
	{11, "sfp_read", 		sfp_read, 				-1, DESC_SFP_READ},
	{12, "temp_info", 		temp_info, 				-1, "show temp info"},
	{13, "temp_state", 		temp_state, 			-1, "show temp state"},
	{14, "temp_threshold", 	temp_threshold, 		-1, DESC_TEMP_THRESHOLD},
	{15, "power_info", 		power_info, 			-1, "show power info. -v ps index"},
	{16, "fan_info", 		fan_info, 				-1, "get fan info."},
	{17, "fan_speed", 		fan_speed,				-1, "set fan speed"},
	{18, "power_state", 	power_state,			-1, "set power state"},
	{19, "led_help", 		led_help,				-1, "led help"},
	{20, "led_info", 		led_info,				-1, "led_info"},
	{21, "led_control", 	led_control,			-1, "led control"}, /**/
	{22, "boot_version", 	boot_version,			-1, "boot version"}, /**/
	{23, "backplane_info", 	backplane_info,			-1, "show backplane info"}, /**/		
	{24, "kernel_debug", 	kenel_debug,			-1, "kenel debug set. 0-io_off,1-io_on,2-octeon_off,3-octeon_on"}, /**/
	{25, "master_get", 		master_get,				-1, "master flag get."}, /**/
	{26, "master_set", 		master_set,				-1, "master flag set."}, /**/
	{27, "i2c_twsi1_read", 	i2c_twsi1_read,			-1, "i2c twsi1 read."}, /**/
	{28, "boot_env", 		op_boot_env,            -1, "boot_env read/write"},
	{29, "mv_switch_counter",	mv_switch_counter,		-1, "  marvell switch stats counter"},
	{30, "mv_switch_register_read",		mv_switch_read,		-1,	"  marvell switch register read"},
	{31, "mv_switch_register_write",	mv_switch_write,	-1, "  marvell switch register write"},/*
	{32, "reg_read",		mv_reg_read,			-1,	"marvell register read"},
	{33, "reg_write",		mv_reg_write,			-1,	"marvell register write"}*/
	{34, "port_isolation_control", 	port_isolation_control, -1,	"port isolation control"},
	{35, "xaui_switch", 	xaui_switch, 			-1,	"XAUI channel switch"},
	{36, "rtl8139_read",	rtl8139_read,			-1,	"RTL8139 Read"},
	{37, "rtl8139_write",	rtl8139_write,			-1,	"RTL8139 Write"},
	{38, "wireless_led", 	wireless_led_op,		-1, "Wireless Led Light"},
	{39, "music_i2c_write", music_i2c_write_op,		-1, "MUSIC_I2C_WRITE"},
	{40, "master_set_debug",master_set_debug,		-1, "master flag set."},
	{41, "qt2225_read", 	qt2225_read,			-1, "QT2225 Read"},
	{42, "qt2225_write",	qt2225_write,			-1, "QT2225 Write"},
	{43, "mux_loopback", 	mux_loop,				-1, "XAUI channel loop"},
	{44, "cpld_reg_read", 	cpld_reg_read,			-1, "CPLD Register Read"},
	{45, "cpld_reg_write", 	cpld_reg_write,			-1, "CPLD Register Write"},
	{46, "poe_led", 		poe_led,				-1, "poe led control."}, /**/
	{47, "subcard_info", 		au4600_subcard_info,				-1, "get subcard info"}, 
	{48, "bcm5396_reg_read", 		bcm5396_switch_reg_read,				-1, "Read register value from BCM5396"}, 
	{49, "bcm5396_reg_write", 		bcm5396_switch_reg_write,				-1, "Write value to the register of BCM5396"}, 
#ifdef HAVE_POE
	{50, "poe_board_info", 		poe_board_info,				-1, "Get Poe Board Info"}, 
#endif

};


void print_help()
{
	int index;
	char help_options_msg [] = 
"\t-d\tPrint the debug info.\n\
\t-h\tPrint the help content.\n\
\t-v\tValue to set for function.\n\
\t-a\tDevice addr need to set.\n\
\t-c\tCommand offset in device.\n\
\t-p\tPoe_led port.\n\
\t-s\tPoe_led speed.\n\
\t-t\tPoe_led state.\n\
\t-l\tLength of the data to read or write.\n\
\t-s\tBoot env value.\n\
\t-r\tRegister data.\n";
	
	
	printf("Usage:bm_cpld_util [-Options] -f [cmd_code] -v [value].\n");
	printf("Functions - \n");
	printf("\t%-7s\t%-20s%s\n", "Code", "Name", "Description");
	for (index = 0; index < LENGTH(cmd_op_arr); index++)
	{
		cmd_operate_object temp = cmd_op_arr[index];
		printf("\t%-7d\t%-20s%s\n", temp.cmd_code, temp.cmd_name, temp.cmd_desc);
	}
	printf("Options - \n");
	printf("%s", help_options_msg);
	
	
}

int main(int argc, char **argv) {
	int opt;
	int ret;
	
	memset(&cmd_op,0,sizeof(cmd_operate_object));
	
	
	while ((opt = getopt(argc,argv,"hdf:v:a:b:c:l:p:s:r:n:x:e:w:o:0:1:")) != -1) {
		switch (opt) {
			case 'd':
				debug_flag = 1;
				break;
			case 'f':
				{
					int cmd_code = 0;
					memset(&cmd_op, 0, sizeof(cmd_operate_object));
					
					DBG("Enter opt f.\n");
					cmd_code = strtoul(optarg,NULL,10);
					//find the match cmd
					if (cmd_code >= 0)
					{
						
						int cmd_index;
						for (cmd_index = 0; cmd_index < LENGTH(cmd_op_arr); cmd_index++)
						{
							if (cmd_code == cmd_op_arr[cmd_index].cmd_code)
							{
								cmd_op = cmd_op_arr[cmd_index];
								break;
							}
						}					
					}

					if (cmd_op.cmd_func != NULL)
					{
						DBG("cmd_func name is %s.\n", optarg);
					}
					else
					{
						DBG("opt_arg name is %s.\n", optarg);
						DBG("cmd_func is NULL.\n");
						exit(0);
					}
				}
				//DBG("Input addr %s got 0x%016llx\n",optarg,bmu_op_args.op_addr);
				break;
			case 'v':
				DBG("Enter opt v.\n");
				if (optarg != NULL)
				{
					cmd_op.cmd_data = strtoul(optarg,NULL,10);
					if (cmd_op.cmd_data < 0)
					{
						printf("Input value need greater than or equal zero.\n");
						exit(0);	
					}
					DBG("Input value is %d\n",cmd_op.cmd_data);
				}
				else
				{
					cmd_op.cmd_data = 0;
					DBG("Input value is null\n");
				}	
				break;
			case 'n':
				strcpy(env_name,optarg);
				break;
			case 's':
				strcpy(env_value,optarg);
				break;
			case 'a':
				data_dev_addr = strtoul(optarg,NULL,16);
				DBG("Opearte the dev addr %#x.\n", data_dev_addr);
				break;
			case 'o':
				data_dev_off = strtoul(optarg,NULL,16);
				DBG("Opearte the dev addr %#x.\n", data_dev_off);
				break;
			case '0':
				data_dev_data0 = strtoul(optarg,NULL,16);
				DBG("Opearte the dev addr %#x.\n", data_dev_off);
				break;
			case '1':
				data_dev_data1 = strtoul(optarg,NULL,16);
				DBG("Opearte the dev addr %#x.\n", data_dev_off);
				break;
			case 'b':
				data_bit_wide = strtoul(optarg,NULL,10);
				DBG("Opearte reg offset %#x.\n", data_bit_wide);
				break;
			case 'c':
				data_reg_addr = strtoul(optarg,NULL,16);
				DBG("Opearte reg offset %#x.\n", data_reg_addr);
				break;
			case 'r':
				register_data = strtoul(optarg,NULL,16);
				DBG("Opearte reg value %#x.\n", register_data);
				break;
			case 'l':
				data_len = strtoul(optarg,NULL,10);
				DBG("operate data len is  %d.\n", data_len);
				break;
			case 'p':
				poe_port_data = strtoul(optarg,NULL,10);
				DBG("Opearte poe_port_data %#x.\n", poe_port_data);
				break;
			case 'x':
				reg_val = strtoul(optarg,NULL,16);
				break;
			case 'e':
				event_type = strtoul(optarg,NULL,10);
				DBG("Opearte reg offset %#x.\n", event_type);
				break;
			case 'w':
				wid = strtoul(optarg,NULL,16);
				DBG("Opearte reg value %#x.\n", wid);
				break;
			case 'h':
			default: /* '?' */
				//DBG("%s",help_msg);
				print_help();
				exit(0);
		}
	};			

	if (cmd_op.cmd_func != NULL)
	{
		cmd_op.cmd_func(cmd_op.cmd_data);
	}
	else
	{
		print_help();
		return 0;
	}
       
	return ret;;
}

