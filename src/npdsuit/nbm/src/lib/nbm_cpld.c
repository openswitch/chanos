
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "nbm_util.h"
#include "nbm_cpld.h"
#include "nbm_log.h"



extern struct system_state_s	systemStateInfo;

int g_bm_fd = -1;


int nbm_open_bmfile()
{
	int fd;

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
        if(fd < 0)
        {
            nbm_syslog_err("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            return -1;
        }
        g_bm_fd = fd;
		nbm_syslog_dbg("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	return 0;
	
}

int nbm_cpld_operate(unsigned long cmd, cpld_op_args* ptr_cpld_args)
{
    int op_ret = 0;
	int fd;

	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            nbm_syslog_err("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            //return CPLD_RETURN_CODE_OPEN_FAIL;
            return -1;

        }
        g_bm_fd = fd;
		nbm_syslog_dbg("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	op_ret = ioctl (g_bm_fd, cmd, ptr_cpld_args);
    if(op_ret == -1)
    {
       nbm_syslog_err("nbm read cpld register  error!\n");
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
 * nbm_set_system_init_stage
 *
 *	Set system initialization stage
 *
 *	INPUT:
 *		stage - 0 for initialization is undergo, 1 for initialization is done
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *	
 *	NOTE:
 *		This API is used to light the SYS and RUN LEDs indicating system init stage
 *
 **********************************************************************************/
void nbm_set_system_init_stage
(
	unsigned char stage
)
{	
#if 0
	bm_op_cpld_args cpld_args = {0};
	int retval = 0;
	int fd = -1;

	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_dbg("open dev %s error(%d) when set system init stage!\n",NPD_BM_FILE_PATH,fd);
			return ;
		}
		g_bm_fd = fd;
	}

	cpld_args.regAddr = AX_SYSTEM_INIT_STAGE_REG;	
	cpld_args.slot    = 0;
	cpld_args.regData = stage ? 1:0;

	retval = ioctl (g_bm_fd,BM_IOC_CPLD_WRITE,&cpld_args);

	if(retval) {
		nbm_syslog_dbg("nbm write cpld error when set system init stage!\n");
	}
#endif    
	return;
}

/**********************************************************************************
 * nbm_cpld_reg_write
 *
 * General purpose API to write a CPLD register
 *
 *	INPUT:
 *		addr	- CPLD register address
 *		value 	- CPLD register value
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_cpld_reg_write(int addr,unsigned char value)
{       
	bm_op_cpld_args phy_args_cmd;
	int retval = 0;
	int fd = -1;

	nbm_syslog_dbg("nbm write register %#x value %#x\n",addr,value);

	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
	
		if(fd < 0)
		{
			nbm_syslog_err("open dev %s error(%d) when write cpld!",NPD_BM_FILE_PATH,fd);
			return CPLD_RETURN_CODE_OPEN_FAIL;
		}
		g_bm_fd = fd;
	}

	memset(&phy_args_cmd,0,sizeof(bm_op_cpld_args));

	phy_args_cmd.regAddr = addr;
	phy_args_cmd.slot = 0;
	phy_args_cmd.regData = value;

	retval = ioctl (g_bm_fd,BM_IOC_CPLD_WRITE,&phy_args_cmd);
	if(retval == -1) {
		nbm_syslog_err("write cpld register %#x value %#x ioctl error!", addr, value);
		return CPLD_RETURN_CODE_IOCTL_FAIL;
	}
	
	return retval;
}

 /**********************************************************************************
  * nbm_cpld_reg_read
  *
  * General purpose API to read a CPLD register
  *
  *  INPUT:
  * 	 addr	 - CPLD register address
  *  
  *  OUTPUT:
  * 	 value	 - CPLD register value
  *
  *  RETURN:
  * 	 0 - if no error occur.
  * 	 NPD_FAIL - if error occurred.
  *
  **********************************************************************************/
 int nbm_cpld_reg_read(int reg, unsigned char *value)
{
	bm_op_cpld_args cpld_args;
	int fd = -1,retval = 0;

	if(!value) {
		nbm_syslog_err("nbm read cpld register %#x null pointer error\n", reg);
		return -1;
	}
	
	nbm_syslog_dbg("nbm read cpld register %#x\n",reg);

	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
	
		if(fd < 0)
		{
			nbm_syslog_err("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
			return CPLD_RETURN_CODE_OPEN_FAIL;
		}
		g_bm_fd = fd;
	}

	memset(&cpld_args, 0, sizeof(bm_op_cpld_args));
	cpld_args.regAddr = reg;

	retval = ioctl (g_bm_fd,BM_IOC_CPLD_READ,&cpld_args);
	if(retval == -1) {
		nbm_syslog_err("nbm read cpld register %#x error!\n", reg);
		return CPLD_RETURN_CODE_IOCTL_FAIL;
	}

	/*get the return value*/
	nbm_syslog_dbg("nbm read cpld register %#x value %#x\n",reg,cpld_args.regData);

	*value = cpld_args.regData;
	return 0;
}
 
/**********************************************************************************
 *	nbm_eeprom_reg_read
 * 
 *  DESCRIPTION:
 *		 read eeprom register
 *
 *  INPUT:
 *		unsigned char twsi_channel,		- TWSI channel
 *		unsigned int eeprom_addr,		- eeprom address
 *		unsigned int eeprom_type,		- eeprom type
 *		unsigned int validOffset,			- whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid
 *		unsigned int moreThan256,		- whether the ofset is bigger than 256, true: valid false: in valid
 *		unsigned int regAddr				- address of eeprom's register
 *  
 *  OUTPUT:
 *		unsigned char *regValue			- value of eeprom's register
 *
 *  RETURN:
 * 		NBM_ERR		- set fail
 *	 	NBM_OK		- set ok
 *
 **********************************************************************************/
unsigned int nbm_eeprom_reg_read
(
	unsigned char twsi_channel,
	unsigned int eeprom_addr,
	unsigned int eeprom_type,
	unsigned int validOffset,
	unsigned int moreThan256,
	unsigned int regAddr,
	unsigned char *regValue
)
{
	int retval = NBM_OK;

#ifdef DRV_LIB_CPSS_XCAT
	twsi_op_t twsi_rw8_args;
    int fd = 0;
	memset(&twsi_rw8_args, 0, sizeof(twsi_op_t));

	if (!regValue) {
		nbm_syslog_err("nbm_eeprom_reg_read error, parameter is null\n");
		return NBM_ERR;
	}

	nbm_syslog_dbg("nbm_eeprom_reg_read, add=0x%x\n", regAddr);

	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_err("open dev %s error(%d) when read oct mdio!\n", NPD_BM_FILE_PATH, fd);
			return NBM_ERR;
		}
		g_bm_fd = fd;
	}

	twsi_rw8_args.chanNum = twsi_channel;
	twsi_rw8_args.twsi_slave.slaveAddr.address = eeprom_addr;
	twsi_rw8_args.twsi_slave.slaveAddr.type = eeprom_type;
	twsi_rw8_args.twsi_slave.validOffset = validOffset;
	twsi_rw8_args.twsi_slave.moreThen256 = moreThan256;
	twsi_rw8_args.twsi_slave.offset = regAddr;
	twsi_rw8_args.reg_val_size = 1;

	retval = ioctl (g_bm_fd, BM_IOC_TWSI_READ8_, &twsi_rw8_args);
	if (retval == -1) {
		nbm_syslog_err("read eeprom error!\n");
		retval = NBM_ERR;
	}

	nbm_syslog_dbg("nbm_eeprom_reg_read over,add=0x%x value=0x%x ret %d\n",
					twsi_rw8_args.twsi_slave.offset,twsi_rw8_args.reg_val[0], retval);
	/*get the return value*/
	*regValue = twsi_rw8_args.reg_val[0];
#endif

	return retval;
}

/**********************************************************************************
 *	nbm_eeprom_reg_write
 * 
 *  DESCRIPTION:
 *		write eeprom register
 *
 *  INPUT:
 *		unsigned char twsi_channel,		- TWSI channel
 *		unsigned int eeprom_addr,		- eeprom address
 *		unsigned int eeprom_type,		- eeprom type
 *		unsigned int validOffset,			- whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid
 *		unsigned int moreThan256,		- whether the ofset is bigger than 256, true: valid false: in valid
 *		unsigned int regAddr				- address of eeprom's register
 *		unsigned char *regValue			- value of eeprom's register
 *  
 *  OUTPUT:
 *		NULL
 *
 *  RETURN:
 * 		NBM_ERR		- set fail
 *	 	NBM_OK		- set ok
 *
 **********************************************************************************/
unsigned int nbm_eeprom_reg_write
(
	unsigned char twsi_channel,
	unsigned int eeprom_addr,
	unsigned int eeprom_type,
	unsigned int validOffset,
	unsigned int moreThan256,
	unsigned int regAddr,
	unsigned char regValue
)
{
	int retval = NBM_OK;

#ifdef DRV_LIB_CPSS_XCAT
	twsi_op_t twsi_rw8_args;
    int fd = 0;
	memset(&twsi_rw8_args, 0, sizeof(twsi_op_t));

	nbm_syslog_dbg("nbm_eeprom_reg_write, add=0x%x\n", regAddr);


	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_err("open dev %s error(%d) when write oct mdio!\n", NPD_BM_FILE_PATH, fd);
			return NBM_ERR;
		}
		g_bm_fd = fd;
	}

	twsi_rw8_args.chanNum = twsi_channel;
	twsi_rw8_args.twsi_slave.slaveAddr.address = eeprom_addr;
	twsi_rw8_args.twsi_slave.slaveAddr.type = eeprom_type;
	twsi_rw8_args.twsi_slave.validOffset = validOffset;
	twsi_rw8_args.twsi_slave.moreThen256 = moreThan256;
	twsi_rw8_args.twsi_slave.offset = regAddr;
	twsi_rw8_args.reg_val_size = 1;
	twsi_rw8_args.reg_val[0] = regValue;

	retval = ioctl (g_bm_fd, BM_IOC_TWSI_WRITE8_, &twsi_rw8_args);
	if (retval == -1) {
		nbm_syslog_err("read eeprom error!\n");
		retval = NBM_ERR;
	}

	nbm_syslog_dbg("nbm_eeprom_reg_write over,add=0x%x value=0x%x ret %d\n",
					twsi_rw8_args.twsi_slave.offset,twsi_rw8_args.reg_val[0], retval);
#endif
	return retval;
}

#ifdef DRV_LIB_CPSS_XCAT
/**********************************************************************************
 *	nbm_open_laser
 * 
 *  DESCRIPTION:
 *		open laser of fiber ports in xcat
 *
 *  INPUT:
 *		NULL
 *  
 *  OUTPUT:
 *		NULL
 *
 *  RETURN:
 *		void
 *
 **********************************************************************************/
void nbm_open_laser
(
	void
)
{
	int fd = -1;
	int retval = 0;

	nbm_syslog_dbg("nbm_open_laser: open laser\n");

	if(g_bm_fd < 0) {
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_err("open dev %s error(%d) when write oct mdio!\n", NPD_BM_FILE_PATH, fd);
			return ;
		}
		g_bm_fd = fd;
	}

	retval = ioctl (g_bm_fd, BM_IOC_TWSI_OPEN_LASER);
	if (retval == -1) {
		nbm_syslog_err("nbm_open_laser: open laser error.\n");
	}

	nbm_syslog_dbg("nbm_open_laser: open laser success.\n");
	return ;
}
#endif
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

	if (enabled == SYSTEM_HARDWARE_WATCHDOG_ENABLE)
	{
		cpld_value = WATCHDOG_ENABLE;
		nbm_syslog_dbg("nbm hardware watchdog control set enable\n");

	}
	else
	{
		cpld_value = WATCHDOG_DISABLE;
	  	nbm_syslog_dbg("nbm hardware watchdog control set diable\n");

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


	if(!enabled) {
		nbm_syslog_err("nbm hardware watchdog control get null pointer error!\n");
		return NPD_FAIL;
	}

	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_ENABLE, &cpld_value);
	if (ret)
	{
		return ret;
	}

	nbm_syslog_dbg("nbm hardware watchdog control get value %d\n", cpld_value);
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


	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_TIMER, &cpld_value);
	if (ret)
	{
		return ret;
	}

	nbm_syslog_dbg("nbm hardware watchdog timeout change %d -> %d\n", regValue, timeout);

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
	
	if(!timeout) {
		nbm_syslog_err("nbm hardware watchdog timeout get null pointer error!\n");
		return NPD_FAIL;
	}

	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_WDT_TIMER, &cpld_value);
	if (ret)
	{
		*timeout = 0;		
		return ret;
	}

	nbm_syslog_dbg("nbm hardware watchdog timeout get value %#x\n", cpld_value);

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

	ret = nbm_cpld_write_func_code(BM_IOC_CPLD_WDT_CLEAR, cpld_value);	
	
	return ret;
}

unsigned long nbm_get_family_type_get()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_PRODUCT_FAMILY, &reg_data);
    return reg_data;	
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
	cpld_data.value= value;  // master bit
	cpld_data.write_flag = 1;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_MASTER_STATE, &cpld_data);
	return ret;
}

long nbm_board_online(unsigned long slot_index)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.param = slot_index;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_BOARD_ONLINE, &cpld_data);
	return (cpld_data.value == BOARD_INSERT);
}

long nbm_board_reset(unsigned long slot_index)
{
	int ret = 0;
	cpld_op_args cpld_data = {0};		
	memset(&cpld_data, 0, sizeof(cpld_op_args));
	cpld_data.param = slot_index;
	cpld_data.write_flag = 1;
	
	ret = nbm_cpld_operate(BM_IOC_CPLD_BOARD_RESET, &cpld_data);
	if(ret != 0)
	{
		return NPD_FAIL;
	}
	return NPD_SUCCESS;
}

long nbm_board_poweroff(unsigned long slot_index)
{
    nbm_syslog_err("nbm_board_poweroff not implemented\n");
	return 0;
}

long nbm_slotno_get()
{
	int ret = 0;
	unsigned char reg_data = 0;
	
	ret = nbm_cpld_read_func_code(BM_IOC_CPLD_SLOT_ID, &reg_data);
	if(ret != 0)
	{
		return 0;
	}
    return reg_data;	
}

long nbm_local_reset()
{
	int ret = 0;
	cpld_op_args cpld_data = {0};
	memset(&cpld_data, 0, sizeof(cpld_op_args));

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
		nbm_syslog_err("Get master slot id error.\n");
		return -1;
	}
    return reg_data;	
	
}

long nbm_get_int_info(char * buf, int len)
{
	int fd;
    int op_ret = 0;
	npd_int_content  int_data;

	if (len != sizeof(int_data.data_arr))
	{
		nbm_syslog_err("int data length is not right.\n");
		return -1;
	}
	
	if(g_bm_fd <= 0)
    {
        fd = open(NPD_BM_FILE_PATH,0);
		
        if(fd < 0)
        {
            nbm_syslog_err("open dev %s error(%d) when read cpld\n",NPD_BM_FILE_PATH,fd);
            return -1;
        }
        g_bm_fd = fd;
		nbm_syslog_dbg("open dev %s success.\n", NPD_BM_FILE_PATH);
    }

	op_ret = ioctl (g_bm_fd, BM_IOC_GET_INT_INFO, &int_data);
    if(op_ret == -1)
    {
       return -1;
    }
	memcpy(buf, int_data.data_arr, len);

	return op_ret;
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
       nbm_syslog_err("nbm get power state error!\n");
       return -1;
    }

	*state = power_data.state;
	return op_ret;

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
       nbm_syslog_err("nbm get power info error!\n");
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
		nbm_syslog_err("nbm get power supply %d info error!\n", index+1);
		return -1;
	}

	*online_state = cpld_args.value;
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
		nbm_syslog_err("temp_data argument error!\n");
		return -1;
	}

	op_ret = ioctl (g_bm_fd, BM_IOC_TEMP_INFO, temp_data);
	if(op_ret == -1)
	{
		nbm_syslog_err("nbm read temp info error!\n");
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
		nbm_syslog_err("nbm read cpld register  error!\n");
	    return -1;
	}
	
	*temp_state = state;
	
	return 0;		
}

int nbm_set_temp_threshold(temp_op_args * temp_op)
{
	int op_ret = 0;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
		
	op_ret = ioctl (g_bm_fd, BM_IOC_TEMP_THRESHOLD, temp_op);
	if(op_ret == -1)
	{
		nbm_syslog_err("nbm read cpld register  error!\n");
	    return -1;
	}
		
	return 0;		
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
		nbm_syslog_err("nbm read cpld register  error!\n");
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

	nbm_syslog_dbg("sfp_data is index %d, rwflag is %d, value is %d.\n", 
			sfp_data.index, sfp_data.rwflag, sfp_data.value);
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_LIGHT, &sfp_data);

	nbm_syslog_dbg("ioctl op_ret  is %d.\n", op_ret);

	if(op_ret < 0)
	{
		nbm_syslog_err("nbm write cpld register  error!\n");
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
		nbm_syslog_err("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*presence_state = sfp_data.value;
	
	return 0;	
}

int nbm_sfp_write(int index, unsigned int dev_addr, unsigned int reg_addr, char * buf, int len)
{
	int op_ret = 0;
	sfp_op_args sfp_data;
		
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	sfp_data.reg_addr = dev_addr;
	sfp_data.rwflag = 1; /* write opt */
	sfp_data.value = reg_addr;
	sfp_data.buf = (uint64_t)(int)(buf); 
	sfp_data.buf_len = len;
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_OPERATE, &sfp_data);
	if(op_ret == -1)
	{
		nbm_syslog_err("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	return 0;		
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
       nbm_syslog_err("nbm ioctl poe_led error!\n");
       return -1;
    }
		
	return 0;
}
#if 0
int nbm_poe_board_info(poe_board_t *poe_board)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	op_ret = ioctl(g_bm_fd, BM_IOC_POE_BOARD_INFO, poe_board);
    if(op_ret == -1)
    {
       nbm_syslog_err("nbm ioctl poe_board error!\n");
       return -1;
    }

	printf("nbm_poe_board_info : present %d, type %d\n", poe_board->present, poe_board->type);
		
	return 0;
}
#endif


int nbm_sfp_read(int index, unsigned int dev_addr, unsigned int reg_addr, char *buf, int len)
{
	int op_ret = 0;
	sfp_op_args sfp_data;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	sfp_data.index = index;
	sfp_data.reg_addr = dev_addr;
	sfp_data.rwflag = 0; /* read opt */
	sfp_data.value = reg_addr;
	sfp_data.buf = (uint64_t)(int)(buf); 
	sfp_data.buf_len = len;

	printf("buf value is %x. \n", (int)sfp_data.buf); /* */
	
	op_ret = ioctl (g_bm_fd, BM_IOC_SFP_OPERATE, &sfp_data);
	if(op_ret == -1)
	{
		nbm_syslog_err("nbm read cpld register  error!\n");
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
		nbm_syslog_err("nbm read cpld register  error!\n");
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
		nbm_syslog_err("nbm read cpld register  error!\n");
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
		nbm_syslog_err("nbm read cpld register  error!\n");
	       	return -1;
	}
	
	*cdr_state = sfp_data.value;
	
	return 0;	
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
       nbm_syslog_err("nbm get power state error!\n");
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
       nbm_syslog_err("nbm get power state error!\n");
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
       nbm_syslog_err("nbm get power state error!\n");
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
       nbm_syslog_err("nbm get power state error!\n");
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
       nbm_syslog_err("nbm led control error!\n");
       return -1;
    }

	return op_ret;
}

long nbm_led_port_control(led_op_args * led_op)	
{
	int op_ret;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}
	led_op->rwflag = 1; 
	
	op_ret = ioctl (g_bm_fd, BM_IOC_LED_PORT_CONTROL, led_op);
    if(op_ret == -1)
    {
       nbm_syslog_err("nbm led control error!\n");
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
       nbm_syslog_err("nbm led control error!\n");
       return -1;
    }

	return op_ret;
	
}

int nbm_led_sfp_speed(int sfp_index, int sfp_speed)
{
	int op_ret;
	led_op_args  led_op_data;
	
	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	led_op_data.rwflag = 1; 
	led_op_data.op_type = LED_SFP_SPEED;
	led_op_data.index = sfp_index;
	if (sfp_speed == SFP_SPD_XGE)
	{
		led_op_data.inserted = LED_STATUS_INSERT;
		led_op_data.state = LED_STATUS_NORMAL;
	}
	else
	{
		led_op_data.inserted = LED_STATUS_REMOVE;
	}
	
	op_ret = ioctl (g_bm_fd, BM_IOC_LED_CONTROL, &led_op_data);
    if(op_ret == -1)
    {
       nbm_syslog_err("nbm led control error!\n");
       return -1;
    }

	return op_ret;
	
}


long nbm_get_boot_version_name(char * vername)
{
	int op_ret;
	boot_env_t	env_args ;
	char *name = "version";	

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
       nbm_syslog_err("nbm led control error!\n");
       return -1;
    }

	sprintf(vername,env_args.value);
		
	return 0;
}

long nbm_interrupt_operate(int_op_t * int_op)
{
	int op_ret;

	if ((g_bm_fd <= 0) && (-1 == nbm_open_bmfile()))
	{
		return -1;
	}

	op_ret = ioctl(g_bm_fd,BM_IOC_INT_OPERATE,int_op);
    if(op_ret == -1)
    {
       nbm_syslog_err("nbm led control error!\n");
       return -1;
    }
	
	return 0;
}

/* Added by wangquan for topsec 20120423 */
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
       nbm_syslog_err("nbm ioctl port isolation!\n");
       return -1;
    }
		
	return 0;
}

/* Added by wangquan for cgm9648 20120426 */
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
       nbm_syslog_err("nbm ioctl xaui switch error!\n");
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
       nbm_syslog_err("nbm ioctl wireless led operate error!\n");
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
       nbm_syslog_err("nbm ioctl qt2225 operate error!\n");
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
       nbm_syslog_err("nbm ioctl subcard_info error!\n");
       return -1;
    }
	
	return 0;
}	


#ifdef __cplusplus
}
#endif
