#ifndef BMK_OPERATION_BOOT_ENV_H
#define BMK_OPERATION_BOOT_ENV_H


#define CFG_ENV_OFFSET 0x0
#define CFG_ENV_SIZE 0x10000


typedef struct{
	unsigned int store_flag;
	unsigned char mac[6];
}mac_addr_stored_t;


typedef struct sys_mac{
	
	unsigned char mac_add[6];
	unsigned char reserve[2];
}sys_mac_add;

/*gxd*/

typedef struct {
	ulong	size;			/* total bank size in bytes		*/
	ushort	sector_count;		/* number of erase units		*/
	ulong	flash_id;		/* combined device & manufacturer code	*/
} flash_info_t;

#define FLASH_CFI_8BIT		0x01

#define AMD_MANUFACT	0x00010001	/* AMD	   manuf. ID in D23..D16, D7..D0 */
#define SST_MANUFACT	0x00BF00BF	/* SST	   manuf. ID in D23..D16, D7..D0 */
/*--------------------------------------------------------------------------*/
#define FLASH_MAN_AMD	0x00000000	/* AMD					*/
#define FLASH_MAN_SST	0x00100000
#define FLASH_UNKNOWN	0xFFFF		/* unknown flash type			*/
/*--------------------------------------------------------------------------*/
#define AMD_ID_LV040B	0x004F004F		/* 29LV040B ID				*/
#define SST_ID_VF040	0x00D700D7		/*39VF040 ID (512KB=64KB x 8)  */ 
/*--------------------------------------------------------------------------*/
#define FLASH_AM040	0x0001		/* AMD Am29F040B, Am29LV040B		*/
#define FLASH_SST040	0x000E		/* SST 39xF040 ID (512KB = 4Mbit x 8 )	*/


typedef struct bootrom_file
{
	char path_name[4096];
}bootrom_file;

int do_get_or_save_boot_env(char *name, char *value, int operation);

#endif


