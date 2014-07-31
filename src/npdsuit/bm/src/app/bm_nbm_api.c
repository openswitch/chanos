
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "npd/nbm/npd_cplddef.h"
#include "board/ts_product_feature.h"
#include "bm_cpld_util.h"
#include "bm_nbm_api.h"

//#include "npd/npd_log.h"

#define LENGTH(x)	(sizeof(x)/sizeof(x[0]))

#define DEFAULT_VALUE_NA	-2
#define NPD_BM_FILE_PATH "/dev/bm0"



#define NBM_TRUE   1
#define NBM_FALSE   0
#define NBM_OK              0
#define NBM_ERR             (NBM_OK + 1)

#define NPD_SUCCESS 	0
#define NPD_FAIL 		-1

#define NPD_TRUE 		1
#define NPD_FALSE		0

#define MAC_ADDRESS_LEN	6

#define SYSINFO_SN_LENGTH	 20
#define SYSINFO_PRODUCT_NAME  24
#define SYSINFO_SOFTWARE_NAME 24
#define SYSINFO_ENTERPRISE_NAME  64
#define SYSINFO_ENTERPRISE_SNMP_OID  128
#define SYSINFO_BUILT_IN_ADMIN_USERNAME  32
#define SYSINFO_BUILT_IN_ADMIN_PASSWORD  32

#define SYSINFO_MODULE_SERIAL_NO  20
#define SYSINFO_MODULE_NAME       32





int g_bm_fd;
int nbm_open_dev_file(void);

int nbm_open_bmfile()
{
	int fd;

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            return -1;
        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	return 0;
	
}


int npd_product_hardware_watchdog_function_check
(
	void
)
{
	return NBM_TRUE;
}

	
int nbm_cpld_operate(unsigned long cmd, cpld_op_args* ptr_cpld_args)
{
	int fd;
    int op_ret = 0;
	
	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	op_ret = ioctl (g_bm_fd, cmd, ptr_cpld_args);
    if(op_ret == -1)
    {
       DBG("nbm read cpld register  error!\n");
       return -1;
    }

	return op_ret;
}

int nbm_cpld_read_func_code(unsigned long cmd, unsigned char* value)
{
	cpld_op_args cpld_args ;
	memset(&cpld_args, 0, sizeof(cpld_args));

	nbm_cpld_operate(cmd, &cpld_args);

	*value = cpld_args.value;

	return 0;

}

int nbm_cpld_write_func_code(unsigned long cmd, unsigned char value)
{
	cpld_op_args cpld_args ;

	memset(&cpld_args, 0, sizeof(cpld_args));

	cpld_args.write_flag = 1;
	cpld_args.value = value;
	nbm_cpld_operate(cmd, &cpld_args);

	return 0;
}



/**********************************************************************************
 * nbm_hardware_watchdog_control
 *
 * 	Set hardware watchdog enable or disable
 *
 *	INPUT:
 *		enabled	- enable/disable hardware watchdog
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_hardware_watchdog_control_set
(
	unsigned int enabled
)
{
	int ret;
	unsigned char cpld_value;

	if(NBM_FALSE == npd_product_hardware_watchdog_function_check()) {
		return NBM_OK;
	}

	if (enabled == SYSTEM_HARDWARE_WATCHDOG_ENABLE)
	{
		cpld_value = WATCHDOG_ENABLE;
		DBG("nbm hardware watchdog control set enable\n");

	}
	else
	{
		cpld_value = WATCHDOG_DISABLE;
	  	DBG("nbm hardware watchdog control set diable\n");

	}

	ret = nbm_cpld_write_func_code(BM_IOC_CPLD_WDT_ENABLE, cpld_value);
	if (ret)
	{
		return ret;
	}

	
	return ret;
	
	
}

/**********************************************************************************
 * nbm_hardware_watchdog_control_get
 *
 * 	Get hardware watchdog enable or disable
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		enabled	- enable/disable hardware watchdog
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_hardware_watchdog_control_get
(
	unsigned int *enabled
)
{
	unsigned char cpld_value = 0;
	int ret = 0;

	if(NBM_FALSE == npd_product_hardware_watchdog_function_check()) {
		return NBM_OK;
	}

	if(!enabled) {
		DBG("nbm hardware watchdog control get null pointer error!\n");
		return NPD_FAIL;
	}

	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_ENABLE, &cpld_value);
	if (ret)
	{
		return ret;
	}

	DBG("nbm hardware watchdog control get value %d\n", cpld_value);
	if(cpld_value == WATCHDOG_ENABLE) {
		*enabled = SYSTEM_HARDWARE_WATCHDOG_ENABLE;
	}
	else {
		*enabled = SYSTEM_HARDWARE_WATCHDOG_DISABLE;
	}

	return ret;
}

/**********************************************************************************
 * nbm_hardware_watchdog_timeout_set
 *
 * 	Set hardware watchdog timeout value
 *
 *	INPUT:
 *		timeout	- hardware watchdog timeout value
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_hardware_watchdog_timeout_set
(
	unsigned int timeout
)
{
	unsigned char regValue = 0;
	int ret = 0;
	unsigned char cpld_value;


	/* confirm if current product support hardware watchdog or not */
	if(NBM_FALSE == npd_product_hardware_watchdog_function_check()) {
		return NBM_OK;
	}

	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_TIMER, &cpld_value);
	if (ret)
	{
		return ret;
	}

	DBG("nbm hardware watchdog timeout change %d -> %d\n", regValue, timeout);

	ret = nbm_cpld_write_func_code(BM_IOC_CPLD_WDT_TIMER, timeout);	
	return ret;
}

/**********************************************************************************
 * nbm_hardware_watchdog_timeout_get
 *
 * 	Get hardware watchdog timeout value
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		timeout	- hardware watchdog timeout value
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_hardware_watchdog_timeout_get
(
	unsigned int *timeout
)
{
	unsigned char cpld_value = 0;
	int ret = 0;
	
	/* confirm if current product support hardware watchdog or not */
	if(NBM_FALSE == npd_product_hardware_watchdog_function_check()) {
		return NBM_OK;
	}

	if(!timeout) {
		DBG("nbm hardware watchdog timeout get null pointer error!\n");
		return NPD_FAIL;
	}

	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_TIMER, &cpld_value);
	if (ret)
	{
		*timeout = 0;		
		return ret;
	}

	DBG("nbm hardware watchdog timeout get value %#x\n", cpld_value);

	*timeout = cpld_value;
	return ret;
}

/**********************************************************************************
 * nbm_hardware_watchdog_fillup
 *
 * 	Fillup hardware watch dog, this is done via read or write fillup register once.
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_hardware_watchdog_fillup
(
	void
)
{
	unsigned char cpld_value = 0;
	int ret = 0;

	/* confirm if current product support hardware watchdog or not */
	if(NBM_FALSE == npd_product_hardware_watchdog_function_check()) {
		return NBM_OK;
	}

	ret = nbm_cpld_write_func_code(BM_IOC_CPLD_WDT_CLEAR, cpld_value);	
	
	return ret;
}





unsigned long nbm_get_product_type()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_PRODUCT_TYPE, &reg_data);
    return reg_data;	
}

unsigned long nbm_get_product_hw_code()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_PRODUCT_HWCODE, &reg_data);
    return reg_data;
}

unsigned long nbm_get_module_type()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_MODULE_TYPE, &reg_data);
    return reg_data;

}

unsigned long nbm_get_module_hw_code()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_MODULE_HWCODE, &reg_data);
    return reg_data;
}

unsigned char nbm_get_board_hw_version()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_PCB_VERSION, &reg_data);
    return reg_data;	
}

unsigned char nbm_get_backboard_version()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_BACKBOARD_VERSION, &reg_data);
    return reg_data;	
}


void nbm_sys_reset()
{
	
}

long nbm_board_num_get()
{
	int op_ret = 0;
	int board_num;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl (g_bm_fd, BM_IOC_CPLD_BOARD_NUM, &board_num);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	    return -1;
	}
	
	return board_num;
}


long nbm_board_online(unsigned long slot_index)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.param = slot_index;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_BOARD_ONLINE, &cpld_data);
	if(ret != 0)
	{
		DBG("board online state error.\n");
		return NPD_FAIL;
	}

	//DBG("board slot index %d online state is %d.\n", cpld_data.value);
	
	return (cpld_data.value == BOARD_INSERT);
}

long nbm_board_reset(unsigned long slot_index)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.param = slot_index;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_BOARD_RESET, &cpld_data);
	if(ret != 0)
	{
		DBG("board reset error.\n");
		return NPD_FAIL;
	}
	return NPD_SUCCESS;
}

long nbm_board_poweroff(unsigned long slot_index)
{
	return 0;
}

long nbm_slotno_get()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_SLOT_ID, &reg_data);
	if(ret != 0)
	{
		DBG("Get slot id error.\n");
		return 0;
	}
    return reg_data;	
}

long nbm_local_reset()
{
	int ret = 0;
	cpld_op_args cpld_data = {0};
	memset(&cpld_data, 0, sizeof(cpld_op_args));

	/* */
	ret = nbm_cpld_operate(BM_IOC_CPLD_SELF_SYS_RESET, &cpld_data);
	return NPD_SUCCESS;
}

long nbm_master_slotno_get()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_MASTER_SLOT_ID, &reg_data);
	if(ret != 0)
	{
		DBG("Get master slot id error.\n");
		return -1;
	}
    return reg_data;	
}


int nbm_eeprom_write
(
	u8	 	dev_addr,
	u16		reg_addr,
	char * 	buf,
	int * 	len
)
{
    int op_ret = 0;
	int fd;
	i2c_op_8_args i2c_op8_data;
	int index;

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	memset(&i2c_op8_data, 0, sizeof(i2c_op_8_args));
	i2c_op8_data.dev_addr = dev_addr;
	i2c_op8_data.reg_offset = reg_addr;
	i2c_op8_data.buf_len = *len;

	for (index = 0; index < i2c_op8_data.buf_len; index++)
	{
		i2c_op8_data.data[index] = buf[index];
	}

	if (debug_flag)
	{
		printf("write value length is  %d.\n", *len);
		for (index = 0; index < *len; index++)
		{
			printf("%2x ", buf[index]);
			if (index != 0 && index % 16 == 0)
			{
				printf("\n");
			}
		}
		printf("\n");
	}

	
	DBG("want to read value length is  %d.\n", *len);

	op_ret = ioctl (g_bm_fd, BM_IOC_I2C_WRITE_EEPROM, &i2c_op8_data);
    if(op_ret == -1)
    {
       DBG("nbm read cpld register  error!\n");
       return -1;
    }

	*len = i2c_op8_data.buf_len;

	return op_ret;
	
}



int nbm_eeprom_read
(
	u8	 	dev_addr,
	u16		reg_addr,
	char * 	buf,
	int * 	len
)
{
    int op_ret = 0;
	int fd;
	i2c_op_8_args i2c_op8_data;
	int index;
	

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	memset(&i2c_op8_data, 0, sizeof(i2c_op_8_args));
	i2c_op8_data.dev_addr = dev_addr;
	i2c_op8_data.reg_offset = reg_addr;
	i2c_op8_data.buf_len = *len;
	
	//i2c_op8_data.data[0] = (unsigned char)value;
	//i2c_op8_data.buf_len = sizeof(i2c_op8_data.data);
	DBG("want to read value length is  %d.\n", *len);

	op_ret = ioctl (g_bm_fd, BM_IOC_I2C_READ_EEPROM, &i2c_op8_data);
    if(op_ret == -1)
    {
       DBG("nbm read cpld register  error!\n");
       return -1;
    }

	
	for (index = 0; index < i2c_op8_data.buf_len; index++)
	{
		buf[index] = i2c_op8_data.data[index];
	}
	*len = i2c_op8_data.buf_len;

	if (debug_flag)
	{
		printf("read value length is  %d.\n", *len);
		for (index = 0; index < *len; index++)
		{
			printf("%2x ", buf[index]);
			if (index != 0 && index % 16 == 0)
			{
				printf("\n");
			}
		}
		printf("\n");
	}
	

	return op_ret;

}

int nbm_eeprom_read_one
(
	u8	 	dev_addr,
	u16		reg_addr,
	char * 	buf,
	int * 	len
)
{
    int op_ret = 0;
	int fd;
	i2c_op_8_args i2c_op8_data;
	int index;
	

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	memset(&i2c_op8_data, 0, sizeof(i2c_op_8_args));
	i2c_op8_data.dev_addr = dev_addr;
	i2c_op8_data.reg_offset = reg_addr;
	i2c_op8_data.buf_len = *len;
	
	//i2c_op8_data.data[0] = (unsigned char)value;
	//i2c_op8_data.buf_len = sizeof(i2c_op8_data.data);
	DBG("want to read value length is  %d.\n", *len);

	op_ret = ioctl (g_bm_fd, BM_IOC_I2C_READ_EEPROM_ONE, &i2c_op8_data);
    if(op_ret == -1)
    {
       DBG("nbm read cpld register  error!\n");
       return -1;
    }

	
	for (index = 0; index < i2c_op8_data.buf_len; index++)
	{
		buf[index] = i2c_op8_data.data[index];
	}
	*len = i2c_op8_data.buf_len;

	if (debug_flag)
	{
		printf("read value length is  %d.\n", *len);
		for (index = 0; index < *len; index++)
		{
			printf("%2x ", buf[index]);
			if (index != 0 && index % 16 == 0)
			{
				printf("\n");
			}
		}
		printf("\n");
	}
	

	return op_ret;

}


int nbm_i2c_read_byte
(
	long * read_value,
	u8	 	dev_addr,
	u8		reg_addr
)
{
	int op_ret = 0;
	int fd;
	i2c_op_8_args i2c_op8_data;

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	memset(&i2c_op8_data, 0, sizeof(i2c_op_8_args));
	i2c_op8_data.dev_addr = dev_addr;
	i2c_op8_data.reg_offset = reg_addr;
	i2c_op8_data.buf_len = 1;

	DBG("current value is %d.\n", (int)i2c_op8_data.data[0]);
	op_ret = ioctl (g_bm_fd, BM_IOC_I2C_READ_8, &i2c_op8_data);
    if(op_ret == -1)
    {
       DBG("nbm read cpld register  error!\n");
       return -1;
    }

	DBG("read value is %d.\n", (int)i2c_op8_data.data[0]);
	*read_value = i2c_op8_data.data[0];

	return op_ret;

}

int nbm_open_dev_file(void)
{
	int fd;
	
    fd = open(NPD_BM_FILE_PATH,0);
	
    if(fd < 0)
    {
        DBG("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
        //return CPLD_RETURN_CODE_OPEN_FAIL;
        return -1;

    }
    g_bm_fd = fd;
	DBG("open dev %s success.\n", NPD_BM_FILE_PATH);
	return 0;
}

int nbm_sfp_num_get()
{
	int op_ret = 0;
	int sfp_num;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_NUM, &sfp_num);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	return sfp_num;
}

int nbm_sfp_light_get(int index, int * light_state)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_LIGHT, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*light_state = sfp_data.value;
	
	return 0;
}


int nbm_sfp_light_set(int index, int light_state)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	sfp_data.rwflag = 1;	//write op
	sfp_data.value = light_state; 

	DBG("sfp_data is index %d, rwflag is %d, value is %d.\n", 
			sfp_data.index, sfp_data.rwflag, sfp_data.value);
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_LIGHT, &sfp_data);

	DBG("ioctl op_ret  is %d.\n", op_ret);

	if(op_ret < 0)
	{
		DBG("nbm write cpld register  error!\n");
	       	return -1;
	}
	
	return 0;	
}

int nbm_sfp_presence_get(int index, int *presence_state)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_PRESENSE, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*presence_state = sfp_data.value;
	
	return 0;	
}

int nbm_sfp_write(int index, unsigned int reg_addr, char * buf, int len)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	sfp_data.reg_addr = reg_addr;
	sfp_data.rwflag = 1;	//write op
	sfp_data.buf = (uint64_t)buf; 
	sfp_data.buf_len = len;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_OPERATE, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	return 0;		
}

int nbm_sfp_read(int index, unsigned int reg_addr, char *buf, int len)
{
	int op_ret = 0;
	sfp_op_args sfp_data;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	sfp_data.reg_addr = reg_addr;
	sfp_data.buf = (uint64_t)buf; 
	sfp_data.buf_len = len;
	sfp_data.rwflag = 0;

	printf("buf value is %x. \n", (int)sfp_data.buf); /* */
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_OPERATE, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	return 0;	
}



int nbm_sfp_dmi_LOS_get(int index, int * los_state )
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_DMI_LOS, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*los_state = sfp_data.value;
	
	return 0;	
}

int nbm_sfp_dmi_Tx_fault_get(int index, int * tx_fault_state)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_DMI_TX_FAULT, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*tx_fault_state = sfp_data.value;
	
	return 0;		
}

int nbm_xfp_cdr_state_get(int index, int *cdr_state)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_XFP_CDR, &sfp_data);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	    return -1;
	}
	
	*cdr_state = sfp_data.value;
	
	return 0;	
}

int nbm_get_temp_info(temp_info_args * temp_data)
{
	int op_ret = 0;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	if (temp_data == NULL)
	{
		DBG("temp_data argument error!\n");
		return -1;
	}

	op_ret = ioctl (g_bm_fd, BM_IOC_TEMP_INFO, temp_data);
	if(op_ret == -1)
	{
		DBG("nbm read temp info error!\n");
	    return -1;
	}

	return 0;

}

int nbm_get_temp_state(int * temp_state)
{
	int op_ret = 0;
	int state = 0;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
		
	op_ret = ioctl (g_bm_fd, BM_IOC_TEMP_STATE, &state);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	    return -1;
	}
	
	*temp_state = state;
	
	return 0;		
}

int nbm_set_temp_threshold(temp_op_args * temp_op)
{
	int op_ret = 0;
	int state = 0;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
		
	op_ret = ioctl (g_bm_fd, BM_IOC_TEMP_THRESHOLD, temp_op);
	if(op_ret == -1)
	{
		DBG("nbm read cpld register  error!\n");
	    return -1;
	}
		
	return 0;		
}


long nbm_get_power_info(int index, power_info_args * power_info)
{
	int op_ret;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	power_info->index = index;
	op_ret = ioctl (g_bm_fd, BM_IOC_POWER_INFO, power_info);
    if(op_ret == -1)
    {
       DBG("nbm get power info error!\n");
       return -1;
    }
	
	return op_ret;
}

long nbm_get_power_present(int index, int * online_state)
{
	int ret;
	cpld_op_args cpld_args ;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&cpld_args, 0, sizeof(cpld_args));
	cpld_args.param = index;

	ret = nbm_cpld_operate(BM_IOC_CPLD_POWER_PRESENT, &cpld_args);
	if (ret != 0)
	{	
		DBG("nbm get power supply %d info error!\n", index+1);
		return -1;
	}

	*online_state = cpld_args.value;
	return 0;
}

long nbm_get_power_state(int index, int * state)
{
	int op_ret;
	power_op_args power_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	power_data.index = index;
	op_ret = ioctl (g_bm_fd, BM_IOC_POWER_STATE, &power_data);
    if(op_ret == -1)
    {
       DBG("nbm get power state error!\n");
       return -1;
    }

	*state = power_data.state;
	return op_ret;

}


long nbm_get_fan_state(int index, int * state)
{
	int op_ret;
	fan_op_args fan_data = {0};
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	fan_data.index = index;
	op_ret = ioctl (g_bm_fd, BM_IOC_CPLD_FAN_ALARM, &fan_data);
    if(op_ret == -1)
    {
       DBG("nbm get power state error!\n");
       return -1;
    }

	*state = fan_data.value;
	return op_ret;

}


long nbm_get_fan_present(int index, int * online_state)
{
	int op_ret;
	fan_op_args fan_data = {0};
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	fan_data.index = index;
	op_ret = ioctl (g_bm_fd, BM_IOC_CPLD_FAN_PRESENT, &fan_data);
    if(op_ret == -1)
    {
       DBG("nbm get power state error!\n");
       return -1;
    }

	*online_state = fan_data.value;
	
	return op_ret;

}


long nbm_get_fan_speed(int index, int * speed)
{
	int op_ret;
	fan_op_args fan_data = {0};
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	fan_data.index = index;
	op_ret = ioctl (g_bm_fd, BM_IOC_CPLD_FAN_SPEED, &fan_data);
    if(op_ret == -1)
    {
       DBG("nbm get power state error!\n");
       return -1;
    }

	*speed = fan_data.value;
	return op_ret;

}


long nbm_set_fan_speed(int index, int  speed)
{
	int op_ret;
	fan_op_args fan_data = {0};
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	fan_data.index = index;
	fan_data.value = speed;
	fan_data.rwflag = 1;	//tell driver for write
	
	op_ret = ioctl (g_bm_fd, BM_IOC_CPLD_FAN_SPEED, &fan_data);
    if(op_ret == -1)
    {
       DBG("nbm get power state error!\n");
       return -1;
    }

	return op_ret;
}

long nbm_led_control(led_op_args * led_op)	
{
	int op_ret;
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	led_op->rwflag = 1; 

	op_ret = ioctl (g_bm_fd, BM_IOC_LED_CONTROL, led_op);
    if(op_ret == -1)
    {
       DBG("nbm led control error!\n");
       return -1;
    }

	return op_ret;
}

long nbm_get_led_info(led_op_args * led_op)	
{
	int op_ret;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	led_op->rwflag = 0; 
	
	op_ret = ioctl (g_bm_fd, BM_IOC_LED_CONTROL, led_op);
    if(op_ret == -1)
    {
       DBG("nbm led control error!\n");
       return -1;
    }

	return op_ret;
	
}

long nbm_get_boot_version_name(char * vername)
{
	int op_ret;
	boot_env_t	env_args ;
	char *name = "version";		
	char version[128] ;
	char *tmp = version;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	memset(&env_args, 0, sizeof(boot_env_t));

	sprintf(env_args.name,name);
	env_args.operation = GET_BOOT_ENV;
	
	op_ret = ioctl(g_bm_fd,BM_IOC_ENV_EXCH,&env_args);
    if(op_ret == -1)
    {
       DBG("nbm led control error!\n");
       return -1;
    }
	
	sprintf(version,env_args.value); 
	tmp = strstr(version, "version");
	tmp += strlen("version") + 1;
	strcpy(vername, tmp);
		
	return 0;
}


int nbm_read_backplane_sysinfo(struct product_sysinfo_s *product_sysinfo) 
{
	ax_product_sysinfo  sysinfo;
	int fd = -1;
	int result = 0;
	char *ptr = NULL;
	int len = 0;

	memset(&sysinfo, 0, sizeof(ax_product_sysinfo));

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	result = ioctl(g_bm_fd,BM_IOC_BACKPLANE_SYSINFO_READ,&sysinfo);
	if (result != 0)
	{
		DBG("read backplane sysinfo error!\n");
	    memset(&sysinfo, 0, sizeof(ax_product_sysinfo));
		return -1;
	}


	/* read module SN */
	ptr = (char *)malloc(SYSINFO_SN_LENGTH+1);
	if(NULL == ptr)
	{
		DBG("read backplane sysinfo alloc SN memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_SN_LENGTH+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_serial_no,SYSINFO_SN_LENGTH+1);
	ptr[SYSINFO_SN_LENGTH] = '\0';
	product_sysinfo->sn = ptr;
	/* read system base mac*/
	ptr = (char*)malloc(2*MAC_ADDRESS_LEN+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc MAC memory error\n");
		return -1;
	}
	memset(ptr,0,2*MAC_ADDRESS_LEN+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_base_mac_address,2*MAC_ADDRESS_LEN);
	ptr[2*MAC_ADDRESS_LEN] = '\0';
	product_sysinfo->basemac = ptr;

	/*read system name*/
	ptr = (char *)malloc(SYSINFO_PRODUCT_NAME+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc NAME memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_PRODUCT_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_name,SYSINFO_PRODUCT_NAME+1);
	ptr[SYSINFO_PRODUCT_NAME] = '\0';
	product_sysinfo->name = ptr;


	/*read system sw_name*/
	ptr = (char *)malloc(SYSINFO_SOFTWARE_NAME+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc sw_name memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_SOFTWARE_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_software_name,SYSINFO_SOFTWARE_NAME+1);
	ptr[SYSINFO_SOFTWARE_NAME] = '\0';
	product_sysinfo->sw_name = ptr;

	/*read system enterprise_name*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_NAME+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc enterprise_name memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_enterprise_name,SYSINFO_ENTERPRISE_NAME+1);
	ptr[SYSINFO_ENTERPRISE_NAME] = '\0';
	product_sysinfo->enterprise_name = ptr;

	/*read system enterprise_snmp_oid*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_SNMP_OID+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc enterprise_snmp_oid memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_SNMP_OID+1);
	memcpy(ptr,sysinfo.ax_sysinfo_enterprise_snmp_oid,SYSINFO_ENTERPRISE_SNMP_OID+1);
	ptr[SYSINFO_ENTERPRISE_SNMP_OID] = '\0';
	product_sysinfo->enterprise_snmp_oid = ptr;

	/*read system snmp_sys_oid*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_SNMP_OID+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc enterprise_snmp_oid memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_SNMP_OID+1);
	memcpy(ptr,sysinfo.ax_sysinfo_snmp_sys_oid,SYSINFO_ENTERPRISE_SNMP_OID+1);
	ptr[SYSINFO_ENTERPRISE_SNMP_OID] = '\0';
	product_sysinfo->snmp_sys_oid = ptr;

	/*read system built_in_admin_username*/
	ptr = (char *)malloc(SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc built_in_admin_username memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_built_in_admin_username,SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	ptr[SYSINFO_BUILT_IN_ADMIN_USERNAME] = '\0';
	product_sysinfo->built_in_admin_username = ptr;

	/*read system built_in_admin_passwd*/
	ptr = (char *)malloc(SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	if(NULL == ptr) {
		DBG("read backplane sysinfo alloc built_in_admin_passwd memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	memcpy(ptr,sysinfo.ax_sysinfo_built_in_admin_password,SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	ptr[SYSINFO_BUILT_IN_ADMIN_PASSWORD] = '\0';
	product_sysinfo->built_in_admin_passwd = ptr;

	ptr = NULL;

	DBG("read EEPROM get sysinfo:\n");
	DBG("%-12s:%s\n","PRODUCT NAME",product_sysinfo->name);
	DBG("%-12s:%s\n","SYSTEM MAC",product_sysinfo->basemac);
	DBG("%-12s:%s\n","SERIAL No.",product_sysinfo->sn);
	DBG("%-12s:%s\n","SOFTWARE ID",product_sysinfo->sw_name);
	DBG("%-12s:%s\n","VENDOR NAME",product_sysinfo->enterprise_name);
	DBG("%-12s:%s\n","SYSTEM OID",product_sysinfo->snmp_sys_oid);
	DBG("%-12s:%s\n","SNMP OID",product_sysinfo->enterprise_snmp_oid);
	DBG("%-12s:%s\n","USERNAME",product_sysinfo->built_in_admin_username);
	DBG("%-12s:%s\n","PASSWORD",product_sysinfo->built_in_admin_passwd);
	return 0;
}

int nbm_kernel_debug(int debug_level)
{
	int op_ret  = 0;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl (g_bm_fd, BM_IOC_KERNEL_DEBUG, &debug_level);
    if(op_ret == -1)
    {
       DBG("nbm set kernel debug success!\n");
       return -1;
    }

	return op_ret;
	
}

int nbm_master_get()
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.value= 1;  // master bit
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_MASTER_STATE, &cpld_data);
	if (ret != 0)
	{
		return ret;
	}
	return cpld_data.value;
}

int nbm_master_set(unsigned long value)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.value= 1;  // master bit
	cpld_data.write_flag = 1;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_MASTER_STATE, &cpld_data);
	
}

int nbm_master_set_debug(unsigned long value)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.value = value;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_MASTER_SWITCH, &cpld_data);
}

int nbm_poe_led(poe_port_t *poe_port)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_POE_LED_LIGTH, poe_port);
    if(op_ret == -1)
    {
       DBG("nbm ioctl poe_led error!\n");
       return -1;
    }
		
	return 0;
}

long nbm_op_boot_env(boot_env_t *env_args)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_ENV_EXCH, env_args);
    if(op_ret == -1)
    {
       DBG("nbm ioctl env error!\n");
       return -1;
    }
		
	return 0;
}

long nbm_mv_switch_op(sw_op_args *sw_op_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_SWITCH_OP, sw_op_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl mv_switch_op error!\n");
       return -1;
    }
		
	return 0;
}

long nbm_bcm5396_switch_op(bcm5396_sw_op_args *sw_op_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_SWITCH_BCM5396_OP, sw_op_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl BCM5396_switch_op error!\n");
       return -1;
    }
		
	return 0;
}

long nbm_mv_switch_counter(sw_counter_args *sw_counter_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_SWITCH_COUNTER, sw_counter_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl mv_switch_counter error!\n");
       return -1;
    }
		
	return 0;
}



long nbm_port_isolation_control(port_isolation_op_args *port_isolation_args)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd,BM_IOC_PORT_ISOLATION_CONTROL, port_isolation_args);
    if(op_ret == -1)
    {
       DBG("nbm ioctl port isolation control error!\n");
       return -1;
    }
		
	return 0;
}

long nbm_xaui_switch(cpld_mux_args *cpld_mux_param)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_XAUI_SWITCH, cpld_mux_param);
    if(op_ret == -1)
    {
       DBG("nbm ioctl xaui switch error!\n");
       return -1;
    }
	
	return 0;
}

long nbm_rtl8139_operate(rtl8139_op_args *rtl8139_op_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_RTL8139_OPERATE, rtl8139_op_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl rtl8139 operate error!\n");
       return -1;
    }
	
	return 0;
}	

long nbm_subcard_info_operate(music_subcard_info_t *subcard_info)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_MUSIC_SUBCARD_INFO, subcard_info);
    if(op_ret == -1)
    {
       DBG("nbm ioctl subcard_info error!\n");
       return -1;
    }
	
	return 0;
}	

long nbm_wireless_led_operate(wled_port_args *wled_port_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_WIRELESS_LED_LIGTH, wled_port_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl wireless led operate error!\n");
       return -1;
    }
	
	return 0;
}	

long nbm_music_i2c_write(music_i2c_dev_args *music_i2c_dev_data)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_MUSIC_I2C_WRITE, music_i2c_dev_data);
    if(op_ret == -1)
    {
       DBG("nbm ioctl music_i2c_write error!\n");
       return -1;
    }
	
	return 0;
}	

long nbm_qt2225_operate(smi_op_args *smi_op_param)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_SMI_OPERATE, smi_op_param);
    if(op_ret == -1)
    {
       DBG("nbm ioctl qt2225 operate error!\n");
       return -1;
    }
	
	return 0;
}	


long nbm_xaui_loop(cpld_mux_args *cpld_mux_param)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_MUX_LOOP, cpld_mux_param);
    if(op_ret == -1)
    {
       DBG("nbm ioctl xaui loop error!\n");
       return -1;
    }
	
	return 0;
}

long nbm_cpld_register_read(cpld_reg_args *cpld_reg_param)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_CPLD_REG_READ, cpld_reg_param);
    if(op_ret == -1)
    {
       DBG("nbm ioctl cpld read error!\n");
       return -1;
    }
	
	return 0;
}

long nbm_cpld_register_write(cpld_reg_args *cpld_reg_param)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_CPLD_REG_WRITE, cpld_reg_param);
    if(op_ret == -1)
    {
       DBG("nbm ioctl cpld write error!\n");
       return -1;
    }
	
	return 0;
}


