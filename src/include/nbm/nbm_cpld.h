#ifndef __NBM_CPLD_H__
#define __NBM_CPLD_H__

#include "npd/nbm/npd_cplddef.h"

#define BM_BIG_ENDIAN 0
#define BM_LITTLE_ENDIAN 1

#define SYSINFO_SN_LENGTH	 20
#define SYSINFO_PRODUCT_NAME  64
#define SYSINFO_SOFTWARE_NAME 24
#define SYSINFO_ENTERPRISE_NAME  64
#define SYSINFO_ENTERPRISE_SNMP_OID  128
#define SYSINFO_BUILT_IN_ADMIN_USERNAME  32
#define SYSINFO_BUILT_IN_ADMIN_PASSWORD  32

#define SYSINFO_MODULE_SERIAL_NO  20
#define SYSINFO_MODULE_NAME       32

#ifndef MAC_ADDRESS_LEN
#define MAC_ADDRESS_LEN 	6
#endif

#define NPD_BM_FILE_PATH "/dev/bm0"


/************* data types for ioctl **************/
typedef struct bm_op_args_s {
	unsigned long long op_addr;
	unsigned long long op_value;  /* ignore on read in arg, fill read value on write return value*/
	unsigned short op_byteorder; /* destination byte order. default is bigendiana.*/
	unsigned short op_len;
	unsigned short op_ret; /* 0 for success, other value indicate different failure.*/
} bm_op_args;

typedef struct bm_op_cpld_read {
	unsigned char 	slot;		/*which slot's CPLD to read from*/
	unsigned int	regAddr;	/*CPLD register address*/
	unsigned char	regData;	/*CPLD register value*/
}bm_op_cpld_args;

typedef struct bm_op_bit64_read {
	unsigned long long 	regAddr;	/*64-bit register address*/
	unsigned long long	regData;	/*64-bit register data*/
}bm_op_rbit64_args;

typedef struct bm_op_sysinfo_common {
	unsigned char mac_addr[MAC_ADDRESS_LEN]; /* system mac address*/
	unsigned char sn[SYSINFO_SN_LENGTH];/* module or backplane or mainboard serial number*/
}bm_op_sysinfo_args;

typedef struct bm_op_module_sysinfo {
	unsigned char slotno;
	bm_op_sysinfo_args common;
}bm_op_module_sysinfo;

typedef bm_op_sysinfo_args bm_op_backplane_sysinfo;
typedef bm_op_sysinfo_args bm_op_mainboard_sysinfo;

/** 
  * Product sysinfo, the data comes from backplane-board.
  * codeby: baolc
  */
typedef struct ax_sysinfo_product_t
{
	char  ax_sysinfo_module_serial_no[21]; /*data should be 20 bytes */
	char  ax_sysinfo_module_name[25];  /*data max length should be 24 bytes */
	char  ax_sysinfo_product_serial_no[21]; /*data should be 20 bytes */
	char  ax_sysinfo_product_base_mac_address[13]; /*data should be 12 bytes */
	char  ax_sysinfo_product_name[25]; /*data max length should be 24 bytes */
	char  ax_sysinfo_software_name[25]; /*data max length should be 24 bytes */
	char  ax_sysinfo_enterprise_name[65]; /*data max length should be 64 bytes */
	char  ax_sysinfo_enterprise_snmp_oid[129]; /*data max length should be 128 bytes */
	char  ax_sysinfo_snmp_sys_oid[129]; /*data max length should be 128 bytes */
	char  ax_sysinfo_built_in_admin_username[33]; /*data max length should be 32 bytes */
	char  ax_sysinfo_built_in_admin_password[33]; /*data max length should be 32 bytes */ 
	char  ax_sysinfo_snmp_product_oid[129]; //data max length should be 128 bytes the last byte is '\0'  
	char  ax_sysinfo_support_url[65]; //data max length should be 65bytes the last byte is '\0'  
}ax_sysinfo_product_t;

/*typedef ax_sysinfo_product_t ax_backplane_sysinfo;*/
typedef ax_sysinfo_product_t ax_product_sysinfo;

typedef struct ax_read_module_sysinfo
{
	int product_id;					/*the product's id must be 3~7 for 3000~7000*/
	int slot_num;							/*0~4 */
	char  ax_sysinfo_module_serial_no[21]; /*data should be 21 bytes */
	char  ax_sysinfo_module_name[25];  /*data max length should be 24 bytes */
}ax_module_sysinfo;

/**
  * Board info, the data comes from mainboard or child-board.
  * code by baolc
  */
typedef struct ax_sysinfo_single_board_t
{
	char  ax_sysinfo_module_serial_no[21]; /*data should be 21 bytes */
	char  ax_sysinfo_module_name[25];  /*data max length should be 24 bytes */
}ax_sysinfo_single_board_t;



/*************** param for read eth_port_stats *****************/

typedef struct bm_op_read_eth_port_stats_args
{
	int portNum;
	int clear; /* 1 for clear stats CSRs */
} bm_op_read_eth_port_stats_args;


#define BM_MINOR_NUM 0
#define BM_MAJOR_NUM 0xEC

#define BM_IOC_MAGIC 0xEC 
#define BM_IOC_RESET	_IO(BM_IOC_MAGIC,0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */

#define BM_IOC_G_  			_IOWR(BM_IOC_MAGIC,1,bm_op_args) /* read values*/
#define BM_IOC_X_ 				_IOWR(BM_IOC_MAGIC,2,bm_op_args) /* write and readout wrote value*/
#define BM_IOC_CPLD_READ		_IOWR(BM_IOC_MAGIC,3,bm_op_cpld_args) /*read cpld registers*/
#define BM_IOC_BIT64_REG_STATE 			_IOWR(BM_IOC_MAGIC,4,bm_op_rbit64_args) /*read 64-bit status register*/
#define BM_IOC_CPLD_WRITE   				_IOWR(BM_IOC_MAGIC,5,bm_op_cpld_args) /*write cpld registers by zhang*/
/*#define BM_IOC_MAINBOARD_SYSINFO_READ	_IOWR(BM_IOC_MAGIC, 6, bm_op_mainboard_sysinfo) read main board sysinfo*/
/*#define BM_IOC_BACKPLANE_SYSINFO_READ	BM_IOC_MAINBOARD_SYSINFO_READ  read back plane sysinfo*/
#define BM_IOC_MODULE_SYSINFO_READ		_IOWR(BM_IOC_MAGIC, 7, bm_op_module_sysinfo) /* read chassis module sysinfo*/
#define BM_IOC_SYSINFO_EEPROM_READ		_IOWR(BM_IOC_MAGIC, 8, bm_op_sysinfo_eeprom_args) /*read sysinfo from eeprom*/
#define BM_IOC_READ_PRODUCT_SYSINFO  _IOWR(BM_IOC_MAGIC, 15, ax_product_sysinfo)/*read backplane or mainboard sysinfo from eeprom*/
#define BM_IOC_BACKPLANE_SYSINFO_READ	BM_IOC_READ_PRODUCT_SYSINFO	/* read back plane sysinfo*/
#define BM_IOC_MAINBOARD_SYSINFO_READ  BM_IOC_READ_PRODUCT_SYSINFO  /*read main board sysinfo*/
#define BM_IOC_READ_MODULE_SYSINFO _IOWR(BM_IOC_MAGIC, 6, ax_module_sysinfo)/*read module sysinfo for series 7 or series 6*/
#define BM_IOC_BOARD_ETH_PORT_STATS_READ  _IOWR(BM_IOC_MAGIC, 9, bm_op_read_eth_port_stats_args) /*read board eth-port statistics*/
#define BM_IOC_ENV_EXCH  		_IOWR(BM_IOC_MAGIC, 10,boot_env_t) /*read board eth-port statistics*/
/*#define BM_IOC_SOFT_SYSINFO_READ_16BIT _IOWR(BM_IOC_MAGIC, 10, ax_module_sysinfo)*//*read sysinfo from sub-card through soft i2c(16bit)*/

#define BM_IOC_SOFT_I2C_READ8_		_IOWR(BM_IOC_MAGIC, 11, soft_i2c_read_write_8bit)  /*read data from eeprom*/
#define BM_IOC_SOFT_I2C_WRITE8_		_IOWR(BM_IOC_MAGIC, 12, soft_i2c_read_write_8bit)  /*write data to eeprom*/
#define BM_IOC_SOFT_I2C_READ16_		_IOWR(BM_IOC_MAGIC, 13, soft_i2c_read_write_16bit)  /*read data from sub card eeprom */
#define BM_IOC_SOFT_I2C_WRITE16_	_IOWR(BM_IOC_MAGIC, 14, soft_i2c_read_write_16bit)  /*write data to sub card eeprom */


#define  SYSTEM_CORE_HIGH_EXTREMUM                    105
#define  SYSTEM_CORE_HIGH_MAXNUM                      100
#define  SYSTEM_CORE_HIGH_CRITICAL                    95
#define  SYSTEM_CORE_LOW_CRITICAL                     90
#define  SYSTEM_SURFACE_HIGH_CRITICAL                 90
#define  SYSTEM_SURFACE_LOW_CRITICAL                  85

#define NBM_TRUE   1
#define NBM_FALSE   0
/** Logging levels for NPD daemon
 */
enum {
	NBM_LOGPRI_TRACE = (1 << 0),   /**< function call sequences */
	NBM_LOGPRI_DEBUG = (1 << 1),   /**< debug statements in code */
	NBM_LOGPRI_INFO = (1 << 2),    /**< informational level */
	NBM_LOGPRI_WARNING = (1 << 3), /**< warnings */
	NBM_LOGPRI_ERROR = (1 << 4)    /**< error */
};

extern int g_bm_fd;


/**********************************************************************************
 * nbm_init_cpld
 *
 * read CPLD register 0x00 - CPLD type and test register
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_SUCCESS - if no error occurred
 *		NPD_FAIL - if ioctl failed or result error
 *
 **********************************************************************************/
int nbm_init_cpld
(
	void
);

/**********************************************************************************
 * nbm_init_cpld
 *
 * read CPLD register 0x00 - CPLD type and test register
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_SUCCESS - if no error occurred
 *		NPD_FAIL - if ioctl failed or result error
 *
 **********************************************************************************/
int nbm_query_mainboard_id
(
	void
);

int nbm_query_backplane_id
(
	void
);

unsigned char nbm_query_mainboard_version
(
	void
);

/**********************************************************************************
 * npd_init_one_chassis_slot
 *
 * init chassis slot global info structure,dedicated slot given by chssis_slot_index
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index to initialize
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
void npd_init_one_chassis_slot
(
	int chassis_slot_index
);

/**********************************************************************************
 * npd_chassis_slot_hotplugin
 *
 * npd callback functions called when chassis slot is hot plugged in,which reported by board check thread.
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index(or slot number)
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
void npd_chassis_slot_hot_plugin
(
	int chassis_slot_index
);

/**********************************************************************************
 * npd_chassis_slot_hot_pullout
 *
 * npd callback functions called when chassis slot is hot pulled out,which reported by board check thread.
 *
 *	INPUT:
 *		chassis_slot_index - chassis slot index(or slot number)
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
void npd_chassis_slot_hot_pullout
(
	int chassis_slot_index
);

/*******************************************************************************
* npd_init_tell_whoami
*
* DESCRIPTION:
*      This function is used by each thread to tell its name and pid to NPD_ALL_THREAD_PID_PATH
*
* INPUTS:
*	  tName - thread name.
*	  lastTeller - is this the last teller or not, pid file should be closed if true.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	  None.
*
* COMMENTS:
*       
*
*******************************************************************************/
extern void npd_init_tell_whoami
(
	char *tName,
	unsigned char lastTeller
);

int nbm_probe_chassis_module_id
(
	int slotno
);

/**********************************************************************************
 * nbm_check_fan_state
 *
 * read CPLD register to get system fan working state
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		SYSTEM_FAN_STAT_NORMAL - if system fan running in normal state.
 *		SYSTEM_FAN_STAT_ALARM - if system fan running in abnormal state.
 *		SYSTEM_FAN_STAT_MAX - if error occurred.
 *
 **********************************************************************************/
enum system_fan_status_e nbm_check_fan_state
(
	void
);

/**********************************************************************************
 * nbm_check_power_state
 *
 * read CPLD register to get system power working state
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0xFF - if error occurred.
 *		state - system power state.
 *
 **********************************************************************************/
unsigned char nbm_check_power_state
(
	void
);

/**********************************************************************************
 * nbm_get_sys_temperature
 *
 * read GPIO register to get system temperature.
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		intDegree - CPU internel temperature
 *		extDegree - CPU surface temperature
 *
 * 	RETURN:
 *		0xffffffff	- if error occurred.
 *		0 - if no error occurred
 *
 **********************************************************************************/
unsigned int nbm_get_sys_temperature
(
	unsigned short *intDegree,
	unsigned short *extDegree
);

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
);

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
int nbm_cpld_reg_write
(
	int addr,
	unsigned char value
);

/**********************************************************************************
 * nbm_cpld_reg_read
 *
 * General purpose API to read a CPLD register
 *
 *	INPUT:
 *		addr	- CPLD register address
 *	
 *	OUTPUT:
 *		value	- CPLD register value
 *
 *	RETURN:
 *		0 - if no error occur.
 *		NPD_FAIL - if error occurred.
 *
 **********************************************************************************/
int nbm_cpld_reg_read
(
	int reg,
	unsigned char *value
);
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
);

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
);

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
);

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
);

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
);

#ifdef DRV_LIB_BCM
/**********************************************************************************
 * nbm_reset_led_mutex_cpld
 *		- reset CPLD register 0x06 bit[4]
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 *	 NOTE:
 *		This API is used to enable Fiber-optic ports and electric ports mutex,
 *		indicating system init stage
 *
 **********************************************************************************/
void nbm_reset_led_mutex_cpld
(
	void
);

#endif

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
);

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
);

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
);
#endif
unsigned long nbm_get_family_type_get();
unsigned long nbm_get_product_type();
unsigned long nbm_get_product_hw_code();
unsigned long nbm_get_module_type();
unsigned long nbm_get_module_hw_code();
unsigned char nbm_get_board_hw_version();
unsigned char nbm_get_backboard_version();


void nbm_sys_reset();
long nbm_local_reset();

int nbm_master_set(unsigned long value);
int nbm_master_get();

long nbm_board_online(unsigned long slot_index);
long nbm_board_reset(unsigned long slot_index);
long nbm_board_poweroff(unsigned long slot_index);

long nbm_slotno_get();
long nbm_get_int_info(char * buf, int len);
long nbm_get_power_state(int index, int * state);
long nbm_get_power_info(int index, power_info_args * power_info);
long nbm_get_power_present(int index, int * online_state);

int nbm_get_temp_info(temp_info_args * temp_data);
int nbm_get_temp_state(int * temp_state);
int nbm_set_temp_threshold(temp_op_args * temp_op);

int nbm_sfp_light_get(int index, int * light_state);
int nbm_sfp_light_set(int index, int light_state);
int nbm_sfp_presence_get(int index, int * presence_state);
int nbm_sfp_write(int index, unsigned int reg_addr, char * buf, int len);
int nbm_sfp_read(int index, unsigned int reg_addr, char *buf,int len);
int nbm_sfp_dmi_LOS_get(int index, int * los_state);
int nbm_sfp_dmi_Tx_fault_get(int index, int * tx_fault_state);
int nbm_xfp_cdr_state_get(int index, int *cdr_state);

long nbm_get_fan_state(int index, int * state);
long nbm_get_fan_present(int index, int * online_state);
long nbm_get_fan_speed(int index, int * speed);
long nbm_set_fan_speed(int index, int  speed);

long nbm_led_control(led_op_args * led_op);
long nbm_led_port_control(led_op_args * led_op);
long nbm_get_led_info(led_op_args * led_op);
int nbm_led_sfp_speed(int sfp_index, int sfp_speed);

long nbm_get_boot_version_name(char * vername);
long nbm_interrupt_operate(int_op_t * int_op);
long nbm_port_isolation_control(port_isolation_op_args *port_isolation_args);
long nbm_qt2225_operate(smi_op_args *smi_op_param);
long nbm_subcard_info_operate(music_subcard_info_t *subcard_info);
long nbm_xaui_switch(cpld_mux_args *cpld_mux_param);
int nbm_poe_led(poe_port_t *poe_port);
long nbm_wireless_led_operate(wled_port_args *wled_port_data);

#endif
