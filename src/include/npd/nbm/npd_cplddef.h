#ifndef NPD_CPLDDEF_H
#define NPD_CPLDDEF_H

#if defined(linux) && defined(__KERNEL__)
#include <linux/types.h>
#else
	#ifndef uint64_t
	#include <stdint.h>  
	#endif
	// Warning: here can't use this header, because will make some module compile failure.
#endif


#define MAX_INT_DATA_OFFSET	16


typedef struct _cpld_op_args_
{
	unsigned int param;			/* for handler */
	unsigned int write_flag;	/* write is 1, and read is 0 */
	
	unsigned char value;		/* the value for read or write */
}cpld_op_args;

typedef struct _npd_int_content_
{
	unsigned char data_arr[MAX_INT_DATA_OFFSET];
}npd_int_content;

typedef struct _npd_int_ctrl_
{
    unsigned int 	int_offset;
    unsigned int	add_offset;
    void (*interrupt_handler)(unsigned char int_value, unsigned char add_value);
}npd_int_ctrl;

typedef struct _npd_int_data_ctrl_
{
    unsigned char	int_mask;
    unsigned char	add_mask;
}npd_int_data_ctrl;



typedef struct int_op_s
{
	unsigned char type;		/* int type */
	unsigned char param;	
}int_op_t;

enum
{
	INT_OP_POWER = 0,
};

#define INT_OP_INSERT			0x01
#define INT_OP_PSINDEX_MASK		0x0F
#define INT_OP_PSINDEX_SHIFT	0x01


typedef struct _i2c_op_8_args_
{
	char 	dev_addr;
	char 	reg_offset;

	int 	buf_len;
	char 	data[1000];
}i2c_op_8_args;

typedef struct _sfp_op_args_
{
	int		index; 		// the index of sfp module
	int		reg_addr;	// operate data reg addr
	
	int		rwflag;		// read or write flag, 0-read, 1-write.
	int		value;		// set or get sfp params

	uint64_t buf;		// write/read data buffer, allocate by app which use it 	
	int		buf_len;	// buffer length
}sfp_op_args;	

typedef struct _power_op_args_
{
	int 	index;		// the index of power
	int		state;		// state ot the power
}power_op_args;

typedef struct _power_info_args_
{
	int		index;		// the index of power
	char 	name[20];	// name of the power
}power_info_args;

typedef struct _temp_info_args_
{
	int	module_temp;		// module temperature
	int	module_high_limit;	// module high temperatute limit
	int	module_low_limit;	// module low temperatute limit
	int	core_temp;			// core temperature
	int	core_high_limit;	// core high temperature limit
 	int core_low_limit;		// core low temperature limit
}temp_info_args;

typedef struct _temp_op_args_
{
	int 	op_type;	// the type of set ;
	int		value;		// value ot the power
}temp_op_args;

typedef struct _fan_op_args_
{
	int 	index;		// the index of power
	int 	rwflag;	// 0-read 1-write
	int		value;		// state ot the power
}fan_op_args;

typedef struct _led_op_args_
{
	int op_type;
	int rwflag;
	int index;
	int inserted;
	int state;
}led_op_args;

typedef struct boot_env
{
	char name[64];
	char value[128];
	int operation;
}boot_env_t;

typedef struct
{
	unsigned char port;
	unsigned char speed;
	unsigned char state;
}poe_port_t;

typedef struct
{
	unsigned int present;
	unsigned int type;
}poe_board_t;


typedef struct _sw_op_args_
{
	int operation;
	unsigned short dev_addr;
	unsigned short reg_addr;
	unsigned short data;
}sw_op_args;

typedef struct _bcm5396_sw_op_args_
{
	int operation;
	unsigned short dev_addr;
	unsigned short reg_addr;
	unsigned short data[4];
}bcm5396_sw_op_args;

typedef struct _sw_counter_args_
{
	unsigned short port;
	unsigned short stats_buf[64];
}sw_counter_args;

typedef struct _port_isolation_op_args_
{
	int op_type;
	int slot;
}port_isolation_op_args;

typedef struct _rtl8139_op_args_
{
	int op_type;
	int chip;
	int bitwide;
	unsigned int reg;
	unsigned int value;
}rtl8139_op_args;


typedef struct _wled_port_args_
{
	int event_type;
	unsigned char wid;
	int user_num;
}wled_port_args;

/*For 8713
* led_index : 0 -- 23 for 1--24 leds
* led_mode :
* 1 -- off
* 2 -- light
* 3 -- 1HZ
* 4 -- 2HZ
* 5 -- 5HZ
* 6 -- 10HZ
* 7 -- 0.5HZ
*/
typedef struct _wled8713_port_args_
{
	int led_index;
	unsigned char led_mode;
}wled8713_port_args;

typedef struct music_subcard_info_s
{
	unsigned char index;
	unsigned char isset;
	unsigned char type;
	unsigned char version;
}music_subcard_info_t;


typedef struct _music_i2c_dev_args_
{
	unsigned char addr;
	unsigned char off;
	unsigned char data0;
	unsigned char data1;
	unsigned char count;
}music_i2c_dev_args;

typedef struct _cpld_mux_args_
{
	int master_slot;
	int loopback;
	int chanel;
}cpld_mux_args;

typedef struct _cpld_reg_args_
{
	unsigned char reg;
	unsigned char value;
}cpld_reg_args;


typedef struct _smi_op_args_
{
	unsigned int op;
	unsigned short phy_addr;
	unsigned short dev_addr;
	unsigned short reg_addr;
	unsigned short data;
}smi_op_args;


#define GET_BOOT_ENV	0
#define SAVE_BOOT_ENV 	1

#define SW_READ_REGISTER 0
#define SW_WRITE_REGISTER 1


enum
{
	KERNEL_DEBUG_IO_OFF,		
	KERNEL_DEBUG_IO_ON,
	KERNEL_DEBUG_OCTEON_OFF,
	KERNEL_DEBUG_OCTEON_ON,
};

enum
{
	WATCHDOG_DISABLE,
	WATCHDOG_ENABLE
};

enum
{
	BOARD_INSERT,
	BOARD_REMOVE
};

enum
{
	SFP_LIGHT_OFF,	
	SFP_LIGHT_ON
};

enum
{
	SFP_REMOVE,	
	SFP_ONLINE
};

enum
{
	SFP_DMI_NORMAL,
	SFP_DMI_ALARM_LOS,
	SFP_DMI_ALARM_TX_FAULT,	
} ;

enum
{
	POWER_REMOVE,	
	POWER_INSERT,
};

enum
{
	POWER_NORMAL,
	POWER_ALARM,
};

enum
{
	POWER_OVER_CURRENT_PROTECT,
	POWER_UNDER_VOLTAGE_PROTECT,
	POWER_OVER_VOLTAGE_PROTECT,
	POWER_FAN_FAULT,
	POWER_AC_FAULT,
	POWER_TEMP_HIGHT,
	POWER_3V3STANDBY_FAULT,
	POWER_12VMAINPWR_FAULT,
};

enum
{
	FAN_REMOVE,	
	FAN_INSERT,
};

enum
{
	FAN_NORMAL,
	FAN_ALARM,
};


enum
{
	TEMP_NORMAL,
	TEMP_MODULE_BEYOND,
	TEMP_MODULE_UNDERNEATH,
	TEMP_CORE_BEYOND,
	TEMP_CORE_UNDERNEATH,
	TEMP_OPEN_CIRCUIT,
	TEMP_NON_EXIST
};

// temper set type
enum
{
	TEMP_MODULE_HILIM,
	TEMP_MODULE_LOLIM,
	TEMP_CORE_HILIM,
	TEMP_CORE_LOLIM,
};

/*  */
enum
{
	LED_STATUS_INSERT,
	LED_STATUS_REMOVE
};

enum
{
	LED_STATUS_NORMAL,
	LED_STATUS_ALARM
};

enum
{
	LED_MEDIUM_COPPER,
	LED_MEDIUM_FIBER
};

enum
{
	SFP_SPD_GE,
	SFP_SPD_XGE
};


enum LED_OP_TYPE
{
	LED_FAN,
	LED_POWER,
	LED_TEMP,
	LED_LINECARD,
	LED_SFP_SPEED,
	LED_MASTER,
	LED_TYPE_MAX,
};

//CDR(Clock Data Recover)
enum 
{
	XFP_CDR_UNLOCKED,
	XFP_CDR_LOCKED,
};

enum 
{
	POE_OFF,
	POE_NORMAL,
	POE_ALARM,
	POE_LINKDOWN
};

enum
{
	POE_LINK_DOWN,
	POE_100M,
	POE_GE,
	POE_LINK_DOWN1,
};

enum
{
	PORT_ISOLATION_ADD,
	PORT_ISOLATION_DEL,
};

enum
{
	AP_JOIN_EVENT,
	AP_QUIT_EVENT,
	USER_JOIN_EVENT,
	USER_QUIT_EVENT,
};

enum
{
	RTL_READ,
	RTL_WRITE,
};

enum
{
	QT2225_READ,
	QT2225_WRITE,
};

typedef enum _SYS_CPLD_FUNC_CODE_
{
	CPLD_TEST = 19,
	CPLD_PRODUCT_FAMILY,
	CPLD_PRODUCT_TYPE,
	CPLD_PRODUCT_HWCODE,
	CPLD_MODULE_TYPE,
	CPLD_MODULE_HWCODE,
	CPLD_PCB_VERSION,
	CPLD_CPLD_VERSION,
	CPLD_BACKBOARD_VERSION,
	
	CPLD_BOARD_NUM,			
	CPLD_BOARD_RESET,		/* for app space  and kernel space*/
	CPLD_MATE_MASTER_RESET,	/* just for kernel space */
	CPLD_LINECARD_RESET,	/* just for kernel space */
	CPLD_SELF_SYS_RESET,	/* just for kernel space */
	
	CPLD_WDT_ENABLE,
	CPLD_WDT_TIMER,
	CPLD_WDT_CLEAR,
	
	CPLD_BOARD_ONLINE,			/* for app space  and kernel space*/
	CPLD_LINECARD_ONLINE,		/* just for kernel space */
	CPLD_MATE_MASTER_ONLINE,	/* just for kernel space */
	
	CPLD_MASTER_STATE,
	CPLD_MASTER_SWITCH,
	CPLD_SLOT_ID,
	CPLD_FAN_PRESENT,
	CPLD_FAN_ALARM,
	CPLD_FAN_SPEED,
	CPLD_POWER_PRESENT,
	CPLD_GET_INT_INFO,
	CPLD_MASTER_SLOT_ID,

	I2C_READ_8,
	I2C_WRITE_8,
	I2C_READ_EEPROM,
	I2C_WRITE_EEPROM,

	SFP_NUM,
	SFP_LIGHT,
	SFP_PRESENSE,
	SFP_OPERATE,
	SFP_DMI_LOS,
	SFP_DMI_TX_FAULT,
	XFP_CDR,			//CDR(Clock Data Recover)

	POWER_STATE,
	POWER_INFO,

	TEMP_STATE,
	TEMP_INFO,
	TEMP_THRESHOLD,

	LED_CONTROL,
	LED_PORT_CONTROL,

	SWITCH_OPERATION,
	SWITCH_COUNTER,

	RTL8139_OPERATE,
	WIRELESS_LED_OPERATE,
	MUSIC_I2C_WRITE_OP,

	CPLD_MUX_SWITCH,
	SMI_OPERATE,
	CPLD_MUX_LOOP,
	CPLD_REG_READ,
	
	KERNEL_DEBUG,
	READ_EEPROM_ONE,
	
	POE_LED,
	PORT_ISOLATION,
	INT_OPERATE,

	MUSIC_GET_SUBCARD_INFO,
	SWITCH_BCM5396_OPERATION,
	CPLD_REG_WRITE,
	POE_BOARD_INFO,
	CPLD_MAX_NUM
}SYS_CPLD_FUNC_CODE;

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */

#define BM_IOC_MAGIC 	0xEC 
#define BM_IOC_RESET	_IO(BM_IOC_MAGIC,0)

#define BM_IOC_G_  			_IOWR(BM_IOC_MAGIC,1,bm_op_args) // read values
#define BM_IOC_X_ 				_IOWR(BM_IOC_MAGIC,2,bm_op_args) // write and readout wrote value
#define BM_IOC_CPLD_READ		_IOWR(BM_IOC_MAGIC,3,bm_op_cpld_args) //read cpld registers
#define BM_IOC_BIT64_REG_STATE 			_IOWR(BM_IOC_MAGIC,4,bm_op_rbit64_args) //read 64-bit status register
#define BM_IOC_CPLD_WRITE   				_IOWR(BM_IOC_MAGIC,5,bm_op_cpld_args) //write cpld registers by zhang

#define BM_IOC_MODULE_SYSINFO_READ		_IOWR(BM_IOC_MAGIC, 7, bm_op_module_sysinfo) // read chassis module sysinfo
#define BM_IOC_SYSINFO_EEPROM_READ		_IOWR(BM_IOC_MAGIC, 8, bm_op_sysinfo_eeprom_args) //read sysinfo from eeprom

#define BM_IOC_READ_MODULE_SYSINFO _IOWR(BM_IOC_MAGIC, 6, ax_module_sysinfo)//read module sysinfo for series 7 or series 6

#define BM_IOC_BOARD_ETH_PORT_STATS_READ  _IOWR(BM_IOC_MAGIC, 9, bm_op_read_eth_port_stats_args) //read board eth-port statistics
#define BM_IOC_ENV_EXCH  		_IOWR(BM_IOC_MAGIC, 10,boot_env_t) //read board eth-port statistics
/*#define BM_IOC_SOFT_SYSINFO_READ_16BIT _IOWR(BM_IOC_MAGIC, 10, ax_module_sysinfo)*///read sysinfo from sub-card through soft i2c(16bit)

#define BM_IOC_SOFT_I2C_READ8_		_IOWR(BM_IOC_MAGIC, 11, soft_i2c_read_write_8bit)  //read data from eeprom
#define BM_IOC_SOFT_I2C_WRITE8_		_IOWR(BM_IOC_MAGIC, 12, soft_i2c_read_write_8bit)  //write data to eeprom
#define BM_IOC_SOFT_I2C_READ16_		_IOWR(BM_IOC_MAGIC, 13, soft_i2c_read_write_16bit)  //read data from sub card eeprom 
#define BM_IOC_SOFT_I2C_WRITE16_	_IOWR(BM_IOC_MAGIC, 14, soft_i2c_read_write_16bit)  //write data to sub card eeprom 

#define BM_IOC_READ_PRODUCT_SYSINFO  _IOWR(BM_IOC_MAGIC, 15, ax_product_sysinfo)//read backplane or mainboard sysinfo from eeprom
#define BM_IOC_BACKPLANE_SYSINFO_READ	BM_IOC_READ_PRODUCT_SYSINFO	// read back plane sysinfo
#define BM_IOC_MAINBOARD_SYSINFO_READ  BM_IOC_READ_PRODUCT_SYSINFO  //read main board sysinfo

#define BM_IOC_GET_MAC	_IOWR(BM_IOC_MAGIC, 16, sys_mac_add) 

#define BM_IOC_BOOTROM_EXCH    _IOWR(BM_IOC_MAGIC, 17,bootrom_file)/*gxd update bootrom based on cli*/

#define BM_IOC_GET_PRODUCET_TYPE	_IOWR(BM_IOC_MAGIC, 18, sys_product_type_t)

#define BM_IOC_CPLD_TEST	_IOWR(BM_IOC_MAGIC, CPLD_TEST, cpld_op_args)

#define	BM_IOC_CPLD_PRODUCT_FAMILY	_IOWR(BM_IOC_MAGIC, CPLD_PRODUCT_FAMILY, cpld_op_args)
#define BM_IOC_CPLD_PRODUCT_TYPE	_IOWR(BM_IOC_MAGIC, CPLD_PRODUCT_TYPE, cpld_op_args)
#define BM_IOC_CPLD_PRODUCT_HWCODE 	_IOWR(BM_IOC_MAGIC, CPLD_PRODUCT_HWCODE, cpld_op_args)

#define BM_IOC_CPLD_MODULE_TYPE		_IOWR(BM_IOC_MAGIC, CPLD_MODULE_TYPE, cpld_op_args)
#define BM_IOC_CPLD_MODULE_HWCODE 	_IOWR(BM_IOC_MAGIC, CPLD_MODULE_HWCODE, cpld_op_args)

#define BM_IOC_CPLD_PCB_VERSION		_IOWR(BM_IOC_MAGIC, CPLD_PCB_VERSION, cpld_op_args)
#define BM_IOC_CPLD_CPLD_VERSION	_IOWR(BM_IOC_MAGIC, CPLD_CPLD_VERSION, cpld_op_args)
#define BM_IOC_CPLD_BACKBOARD_VERSION	_IOWR(BM_IOC_MAGIC, CPLD_BACKBOARD_VERSION, cpld_op_args)

#define BM_IOC_CPLD_BOARD_NUM			_IOWR(BM_IOC_MAGIC, CPLD_BOARD_NUM, int)

#define BM_IOC_CPLD_BOARD_RESET			_IOWR(BM_IOC_MAGIC, CPLD_BOARD_RESET, cpld_op_args)
#define BM_IOC_CPLD_MATE_MASTER_RESET	_IOWR(BM_IOC_MAGIC, CPLD_MATE_MASTER_RESET, cpld_op_args)
#define BM_IOC_CPLD_LINECARD_RESET		_IOWR(BM_IOC_MAGIC, CPLD_LINECARD_RESET, cpld_op_args)
#define BM_IOC_CPLD_SELF_SYS_RESET		_IOWR(BM_IOC_MAGIC, CPLD_SELF_SYS_RESET, cpld_op_args)

#define BM_IOC_CPLD_WDT_ENABLE		_IOWR(BM_IOC_MAGIC, CPLD_WDT_ENABLE, cpld_op_args)
#define BM_IOC_CPLD_WDT_TIMER		_IOWR(BM_IOC_MAGIC, CPLD_WDT_TIMER, cpld_op_args)
#define BM_IOC_CPLD_WDT_CLEAR		_IOWR(BM_IOC_MAGIC, CPLD_WDT_CLEAR, cpld_op_args)

#define BM_IOC_CPLD_BOARD_ONLINE	_IOWR(BM_IOC_MAGIC, CPLD_BOARD_ONLINE, cpld_op_args)
#define BM_IOC_CPLD_LINECARD_ONLINE	_IOWR(BM_IOC_MAGIC, CPLD_LINECARD_ONLINE, cpld_op_args)
#define BM_IOC_CPLD_MATE_MASTER_ONLINE	_IOWR(BM_IOC_MAGIC, CPLD_MATE_MASTER_ONLINE, cpld_op_args)

#define BM_IOC_CPLD_MASTER_STATE	_IOWR(BM_IOC_MAGIC, CPLD_MASTER_STATE, cpld_op_args)
#define BM_IOC_CPLD_MASTER_SWITCH	_IOWR(BM_IOC_MAGIC, CPLD_MASTER_SWITCH, cpld_op_args)
#define BM_IOC_CPLD_SLOT_ID			_IOWR(BM_IOC_MAGIC, CPLD_SLOT_ID, cpld_op_args)

#define BM_IOC_CPLD_FAN_PRESENT		_IOWR(BM_IOC_MAGIC, CPLD_FAN_PRESENT, fan_op_args)
#define BM_IOC_CPLD_FAN_ALARM		_IOWR(BM_IOC_MAGIC, CPLD_FAN_ALARM, fan_op_args)
#define BM_IOC_CPLD_FAN_SPEED		_IOWR(BM_IOC_MAGIC, CPLD_FAN_SPEED, fan_op_args)

#define BM_IOC_CPLD_POWER_PRESENT	_IOWR(BM_IOC_MAGIC, CPLD_POWER_PRESENT, cpld_op_args)
#define BM_IOC_CPLD_GET_INT_INFO	_IOWR(BM_IOC_MAGIC, CPLD_GET_INT_INFO, cpld_op_args)
#define BM_IOC_CPLD_MASTER_SLOT_ID	_IOWR(BM_IOC_MAGIC,	CPLD_MASTER_SLOT_ID, cpld_op_args)

#define BM_IOC_I2C_READ_8			_IOWR(BM_IOC_MAGIC, I2C_READ_8, i2c_op_8_args)
#define BM_IOC_I2C_WRITE_8			_IOWR(BM_IOC_MAGIC, I2C_WRITE_8, i2c_op_8_args)

#define BM_IOC_I2C_READ_EEPROM		_IOWR(BM_IOC_MAGIC, I2C_READ_EEPROM, i2c_op_8_args)
#define BM_IOC_I2C_WRITE_EEPROM		_IOWR(BM_IOC_MAGIC, I2C_WRITE_EEPROM, i2c_op_8_args)

#define BM_IOC_SFP_NUM				_IOWR(BM_IOC_MAGIC, SFP_NUM, int)
#define BM_IOC_SFP_LIGHT			_IOWR(BM_IOC_MAGIC, SFP_LIGHT, sfp_op_args)
#define BM_IOC_SFP_PRESENSE			_IOWR(BM_IOC_MAGIC, SFP_PRESENSE, sfp_op_args)	
#define BM_IOC_SFP_OPERATE			_IOWR(BM_IOC_MAGIC, SFP_OPERATE, sfp_op_args)	
#define BM_IOC_SFP_DMI_LOS			_IOWR(BM_IOC_MAGIC, SFP_DMI_LOS, sfp_op_args)
#define BM_IOC_SFP_DMI_TX_FAULT		_IOWR(BM_IOC_MAGIC, SFP_DMI_TX_FAULT, sfp_op_args)	
#define BM_IOC_XFP_CDR				_IOWR(BM_IOC_MAGIC, XFP_CDR, sfp_op_args)

#define BM_IOC_POWER_STATE			_IOWR(BM_IOC_MAGIC, POWER_STATE, power_op_args)	
#define BM_IOC_POWER_INFO			_IOWR(BM_IOC_MAGIC, POWER_INFO, power_info_args)	

#define BM_IOC_TEMP_STATE			_IOWR(BM_IOC_MAGIC, TEMP_STATE, int)	
#define BM_IOC_TEMP_INFO			_IOWR(BM_IOC_MAGIC, TEMP_INFO, temp_info_args)	
#define BM_IOC_TEMP_THRESHOLD		_IOWR(BM_IOC_MAGIC, TEMP_THRESHOLD, temp_op_args)	

#define BM_IOC_LED_CONTROL			_IOWR(BM_IOC_MAGIC, LED_CONTROL, led_op_args)	
#define BM_IOC_LED_PORT_CONTROL		_IOWR(BM_IOC_MAGIC, LED_PORT_CONTROL, led_op_args)	

#define BM_IOC_SWITCH_OP			_IOWR(BM_IOC_MAGIC, SWITCH_OPERATION, sw_op_args)
#define BM_IOC_SWITCH_COUNTER		_IOWR(BM_IOC_MAGIC, SWITCH_COUNTER, sw_counter_args)

#define BM_IOC_SWITCH_BCM5396_OP			_IOWR(BM_IOC_MAGIC, SWITCH_BCM5396_OPERATION, bcm5396_sw_op_args)

#define BM_IOC_RTL8139_OPERATE		_IOWR(BM_IOC_MAGIC, RTL8139_OPERATE, rtl8139_op_args)

#define BM_IOC_XAUI_SWITCH			_IOWR(BM_IOC_MAGIC, CPLD_MUX_SWITCH, cpld_mux_args)

#define BM_IOC_SMI_OPERATE			_IOWR(BM_IOC_MAGIC, SMI_OPERATE, smi_op_args)

#define BM_IOC_MUX_LOOP				_IOWR(BM_IOC_MAGIC, CPLD_MUX_LOOP, cpld_mux_args)

#define BM_IOC_CPLD_REG_READ		_IOWR(BM_IOC_MAGIC, CPLD_REG_READ, cpld_reg_args)
#define BM_IOC_CPLD_REG_WRITE		_IOWR(BM_IOC_MAGIC, CPLD_REG_WRITE, cpld_reg_args)

#define BM_IOC_KERNEL_DEBUG 		_IOWR(BM_IOC_MAGIC, KERNEL_DEBUG, int)

#define BM_IOC_I2C_READ_EEPROM_ONE	_IOWR(BM_IOC_MAGIC, READ_EEPROM_ONE, i2c_op_8_args)

#define BM_IOC_INT_OPERATE			_IOWR(BM_IOC_MAGIC, INT_OPERATE, int_op_t)

#define BM_IOC_GET_INT_INFO			_IOWR(BM_IOC_MAGIC, CPLD_GET_INT_INFO, npd_int_content)

#define BM_IOC_POE_LED_LIGTH		_IOWR(BM_IOC_MAGIC, POE_LED, poe_port_t)

#define BM_IOC_PORT_ISOLATION_CONTROL	_IOWR(BM_IOC_MAGIC, PORT_ISOLATION, port_isolation_op_args)

#define BM_IOC_WIRELESS_LED_LIGTH	_IOWR(BM_IOC_MAGIC, WIRELESS_LED_OPERATE, wled_port_args)

#define BM_IOC_MUSIC_I2C_WRITE		_IOWR(BM_IOC_MAGIC, MUSIC_I2C_WRITE_OP, music_i2c_dev_args)

#define BM_IOC_MUSIC_SUBCARD_INFO		_IOWR(BM_IOC_MAGIC, MUSIC_GET_SUBCARD_INFO, music_subcard_info_t)

#define BM_IOC_POE_BOARD_INFO		_IOWR(BM_IOC_MAGIC, POE_BOARD_INFO, poe_board_t)

#define BM_IOC_MAXNR 128

#endif 

