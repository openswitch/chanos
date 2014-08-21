/* 
 *bm -- board management (i2c, gpio, SMI,bootbus, cpld, ....) setting 
 *
 */

#ifndef BM_MAIN_H
#define BM_MAIN_H



#include "sysdef/npd_sysdef.h"
//#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_cplddef.h"
#include "bmk_read_eeprom.h"

#define BM_MINOR_NUM 0
#define BM_MAJOR_NUM 0xEC


/************************************
*  old define
************************************/
#define BM_BIG_ENDIAN 0
#define BM_LITTLE_ENDIAN 1

#define MAC_ADDRESS_LEN	6

#define SYSINFO_SN_LENGTH	  32
#define SYSINFO_PRODUCT_NAME  24
#define SYSINFO_SOFTWARE_NAME 24
#define SYSINFO_ENTERPRISE_NAME  64
#define SYSINFO_ENTERPRISE_SNMP_OID  128
#define SYSINFO_BUILT_IN_ADMIN_USERNAME  32
#define SYSINFO_BUILT_IN_ADMIN_PASSWORD  32

/**
  * EEPROM addresses on chassis device.
  */
#define  BM_AX_BACKPLANE_EEPROM_ADDR         0x50
#define  BM_AX_MODULE0_EEPROM_ADDR           0x50

/**
  * EEPROM addresses on box device.
  */
#define  BM_AX_MAINBOARD_EEPROM_ADDR         0X50


typedef struct bm_op_args_s {
	unsigned long long op_addr;
	unsigned long long op_value;  // ignore on read in arg, fill read value on write return value
	unsigned short op_byteorder; // destination byte order. default is bigendiana.
	unsigned short op_len;
	unsigned short op_ret; // 0 for success, other value indicate different failure.
} bm_op_args;

typedef struct bm_op_cpld_read {
	unsigned char 	slot;		//which slot's CPLD to read from
	unsigned int	regAddr;	//CPLD register address
	unsigned char	regData;	//CPLD register value
}bm_op_cpld_args;

typedef struct bm_op_bit64_read {
	unsigned long long 	regAddr;	//GPIO register address
	unsigned long long	regData;	//GPIO register data
}bm_op_rbit64_args;

typedef struct bm_op_sysinfo_common {
	unsigned char mac_addr[MAC_ADDRESS_LEN]; // system mac address
	unsigned char sn[SYSINFO_SN_LENGTH]; // module or backplane or mainboard serial number
}bm_op_sysinfo_args;

typedef struct bm_op_module_sysinfo {
	unsigned char slotno;
	bm_op_sysinfo_args common;
}bm_op_module_sysinfo;

typedef bm_op_sysinfo_args bm_op_backplane_sysinfo;
typedef bm_op_sysinfo_args bm_op_mainboard_sysinfo;

/**
  * ioctl param and cmd for read sysinfo from eeprom.
  */
enum bm_eeprom_type_t
{
	BM_EEPROM_BACKPLANE, //backplane on chassis device
	BM_EEPROM_MODULE0,  //main board on chassis device
	BM_EEPROM_MAINBOARD //main board on box device
};

typedef struct bm_op_sysinfo_eeprom_args
{
	enum bm_eeprom_type_t  eeprom_type;
}bm_op_sysinfo_eeprom_args;

typedef struct bm_op_read_eth_port_stats_args
{
	int portNum;
	int clear; /* 1 for clear stats CSRs */
}bm_op_read_eth_port_stats_args;

typedef struct sys_product_type
{
	unsigned char product_num;
	unsigned char module_num;
}sys_product_type_t;


extern struct bm_dev_s bm_dev;
extern unsigned int debug_ioctl;
extern unsigned int debug_octeon ;

#endif


 
