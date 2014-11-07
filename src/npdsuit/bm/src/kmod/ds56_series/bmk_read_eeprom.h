/*
 * purpose: read sysinfo from EEPROM.
 * author: Autelan. Co. Ltd.
 * codeby: baolc
 * 2008-06-19
 */

#ifndef _BMK_READ_EEPROM_
#define _BMK_READ_EEPROM_

#include "bmk_main.h"

#define DRIVER_NAME "bm"

 #define DBG(o,f, x...) \
	if (o) { printk(KERN_DEBUG DRIVER_NAME ":" f, ## x); } 

extern unsigned int debug_ioctl;

/** 
  * Product sysinfo, the data comes from backplane-board.
  * codeby: baolc
  */
typedef struct ax_sysinfo_product_t
{
	char  ax_sysinfo_module_serial_no[32]; //data should be 20 bytes the last byte is '\0'
	char  ax_sysinfo_module_name[25];  //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_product_serial_no[32]; //data should be 20 bytes the last byte is '\0' 
	char  ax_sysinfo_product_base_mac_address[13]; //data should be 12 bytes the last byte is '\0' 
	char  ax_sysinfo_product_name[25]; //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_software_name[25]; //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_enterprise_name[65]; //data max length should be 64 bytes the last byte is '\0' 
	char  ax_sysinfo_enterprise_snmp_oid[129]; //data max length should be 128 bytes the last byte is '\0' 
	char  ax_sysinfo_snmp_sys_oid[129]; //data max length should be 128 bytes the last byte is '\0' 
	char  ax_sysinfo_built_in_admin_username[33]; //data max length should be 32 bytes the last byte is '\0' 
	char  ax_sysinfo_built_in_admin_password[33]; //data max length should be 32 bytes the last byte is '\0'  
	char  ax_sysinfo_snmp_product_oid[129]; //data max length should be 128 bytes the last byte is '\0'  
	char  ax_sysinfo_support_url[65]; //data max length should be 65bytes the last byte is '\0'   
}ax_sysinfo_product_t;

typedef ax_sysinfo_product_t ax_product_sysinfo;

typedef struct ax_read_module_sysinfo
{
	int product_id;					//the product's id must be 3~7 for 3000~7000
	int slot_num;							//0~4 
	char  ax_sysinfo_module_serial_no[21]; //data should be 21 bytes the last byte is '\0'
	char  ax_sysinfo_module_name[25];  //data max length should be 24 bytes the last byte is '\0'
}ax_module_sysinfo;

/**
  * Board info, the data comes from mainboard or child-board.
  * code by baolc
  */
typedef struct ax_sysinfo_single_board_t
{
	char  ax_sysinfo_module_serial_no[21]; //data should be 20 bytes the last byte is '\0'
	char  ax_sysinfo_module_name[25];  //data max length should be 24 bytes the last byte is '\0'
}ax_sysinfo_single_board_t;

int bm_ax_read_sysinfo_product(unsigned char eeprom_addr, ax_sysinfo_product_t* sysinfo);
/**
  * Read single board sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int bm_ax_read_sysinfo_single_board(unsigned char eeprom_addr, ax_sysinfo_single_board_t* sysinfo);

/**
  * Read module sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int bm_ax_read_module_sysinfo(ax_module_sysinfo* sysinfo);  

/**
* Read sysinfo for proc entry
* @return : -1 is failure, 0 is success.
*/
int ax_read_sysinfo_from_eeprom_proc(unsigned char  eeprom_addr, ax_sysinfo_product_t* sysinfo);

int  _ax_i2c_read8(unsigned char chip, uint addr, int alen, unsigned char *buffer, int len);
int  _ax_i2c_read8_port(unsigned char port_idx, unsigned char chip, uint addr, int alen, unsigned char *buffer, int len);

int bm_i2c_read(int twsi_index, unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);
int bm_i2c_write(int twsi_index, unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);

#endif 
