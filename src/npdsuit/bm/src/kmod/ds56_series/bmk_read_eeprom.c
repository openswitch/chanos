/*
 * purpose: read sysinfo from EEPROM.
 * author: Autelan. Co. Ltd.
 * codeby: baolc
 * 2008-06-19
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/err.h>

#include "bmk_read_eeprom.h"

/////////////////////////////////////////////////////////////////////
//the following code are transplanted from u-boot
#define AX_EEPROM_SYSINFO_MAX_SIZE         (0x01ff)
#define AX_EEPROM_SYSINFO_TUPLE_MAX_LEN    (3+3+128)
#define AX_EEPROM_SYSINFO_MAC_ADDR_LEN     (12)


#define AX_SYSINFO_MALLOC(x)   kmalloc(x, GFP_ATOMIC)
#define AX_SYSINFO_FREE(x, s)  kfree(x) //return void

extern struct mtd_info *get_mtd_device_nm(const char *name);


/** Define data structures of sysinfo .
  *    Data here in struct are all binary data, but in EEPROM or sysinfo.txt they are all plain text strings.
  */
typedef enum
{
	AX_MULTIBOARD_SYSINFO = 0,
	AX_SINGLEBOARD_SYSINFO = 1,
} _ax_sysinfo_type_t;


typedef struct
{
	_ax_sysinfo_type_t   sysinfo_type; //0~9
	uint8_t   sysinfo_type_version; //0~9
	uint16_t  total_element_count; //0~999
	uint32_t  total_file_length; //0~99999, sysinfo.txt's total characters number, including '\n'
} _ax_sysinfo_header_t;


typedef enum
{
	AX_SYSINFO_MODULE_SERIAL_NO = 1, //data should be 20 bytes 
	AX_SYSINFO_MODULE_NAME,  //data max length should be 24 bytes 
	AX_SYSINFO_PRODUCT_SERIAL_NO, //data should be 20 bytes 
	AX_SYSINFO_PRODUCT_BASE_MAC_ADDRESS, //data should be 12 bytes 
	AX_SYSINFO_PRODUCT_MAC_COUNT = 5, //data should be 2 bytes, add by baolc, 2008-05-28
	AX_SYSINFO_PRODUCT_NAME, //data max length should be 24 bytes 
	AX_SYSINFO_SOFTWARE_NAME, //data max length should be 24 bytes 
	AX_SYSINFO_ENTERPRISE_NAME, //data max length should be 64 bytes 
	AX_SYSINFO_ENTERPRISE_SNMP_OID, //data max length should be 128 bytes 
	AX_SYSINFO_SNMP_SYS_OID, //data max length should be 128 bytes 
	AX_SYSINFO_BUILT_IN_ADMIN_USERNAME, //data max length should be 32 bytes 
	AX_SYSINFO_BUILT_IN_ADMIN_PASSWORD, //data max length should be 32 bytes 
} _ax_sysinfo_elem_type_t;


typedef struct
{
	_ax_sysinfo_elem_type_t  element_type; //0~999
	uint16_t  element_length; //0~999, data buffer's length
	unsigned char*     data_buf_ptr; //pointer to data buffer, NOTE: this buffer's memory will be alloc in populating function!!
} _ax_sysinfo_elem_tlv_t;


typedef struct
{
	_ax_sysinfo_header_t      sysinfo_header;
	_ax_sysinfo_elem_tlv_t*   sysinfo_elem_array; //NOTE: the array's memory will be alloc in populating function
} _ax_sysinfo_t;


/** Small tools for convert several chars to a number.
  *@param  char_addr   The address of string.
  *@param  char_num   The number of char.
  *@example   Input: "123", return 123
  * Note: Only support simple string and decimal number.
  */
static int _ax_tool_simple_nstrtoi(unsigned char* char_addr, int char_num)
{
	int num_decimal = 0;
	int i, j;
	int n;
	for (i=char_num-1, j=1; i>=0; i--, j*=10) //parse byte3~5 into a number
	{
		n = (*(char_addr+i) - '0');
		if (n > 9 || n < 0) 
		{
			//printk("ax_error: parse simple string to int! error char: %02x!\n", *(char_addr+i)); 
			return -1;
		}
		num_decimal += n * j;
	}
	return num_decimal;
}


/** Populate the sysinfo data structure using the text buffer which is read from EEPROM.
  * Note: The text buffer are plain text including '\n' or '\r\n'. They should be discarded when populating the sysinfo.
  * Note: The sysinfo structure's memory already allocated.
  		But the    "element array"    in the sysinfo structure would be alloc in this function!
  		And the     "data buf"     in element array would be alloc in this function too!
  		Attention their memory's free!! Using function ax_sysinfo_free()!
  *@return  0 for success. -1 for failure.
  */
static int _ax_populate_sysinfo_with_textbuf(_ax_sysinfo_t* dest_sysinfo_ptr, unsigned char* text_buf, int buf_len)
{
	int rcode = 0;
	int elem_count = 0;
	int elem_type;
	int elem_buf_len;
	unsigned char*  elem_data_addr;
	int i;
	unsigned char* temp;
	
	if (dest_sysinfo_ptr == NULL || text_buf == NULL || buf_len < 10)
		return -1;


	DBG(debug_octeon, "before header.\n");

	if (debug_octeon)
	{
		int index ;
		for (index = 0; index < buf_len; index++)
		{
			printk("%d ", text_buf[index]);
			if (0 == (index%16))
			{
				printk("\n");
			}
		}
	}
	//first, parse sysinfo header, buffer's first 10 bytes
	dest_sysinfo_ptr->sysinfo_header.sysinfo_type = (_ax_sysinfo_type_t)(*text_buf++ - '0'); //byte1
	dest_sysinfo_ptr->sysinfo_header.sysinfo_type_version = (*text_buf++ - '0'); //byte2
	elem_count = _ax_tool_simple_nstrtoi(text_buf, 3);
	dest_sysinfo_ptr->sysinfo_header.total_element_count = elem_count; //byte3~5
	dest_sysinfo_ptr->sysinfo_header.total_file_length = buf_len; //byte6~10
	text_buf += 8;
	buf_len -= 10;

	while (*text_buf != '\r' && *text_buf != '\n' && buf_len > 0) //skip char
	{
		text_buf++;
		buf_len--;
	}
	while ((*text_buf == '\r' || *text_buf == '\n') && buf_len > 0) //skip char
	{
		text_buf++;
		buf_len--;
	}

	DBG(debug_octeon, "elem_count is %d.\n", elem_count);

	if (elem_count > 0 && buf_len <= 0)
		return -1;

	//second, begin to parse element in sysinfo
	dest_sysinfo_ptr->sysinfo_elem_array = (_ax_sysinfo_elem_tlv_t*)AX_SYSINFO_MALLOC(sizeof(_ax_sysinfo_elem_tlv_t) * elem_count); //malloc memory for elements
	if (dest_sysinfo_ptr->sysinfo_elem_array <= 0)
		return -1;
	memset(dest_sysinfo_ptr->sysinfo_elem_array, 0x00, sizeof(_ax_sysinfo_elem_tlv_t) * elem_count);
	
	i = 0;
	while (elem_count > 0 && buf_len > 0 && *text_buf != '\0')
	{
		DBG(debug_octeon, "Parsing element %d...\n", elem_count);
		elem_type = _ax_tool_simple_nstrtoi(text_buf, 3);
		if (elem_type < 0 || elem_type > 999)
			return -1;
		text_buf += 3;
		buf_len -= 3;
		DBG(debug_octeon, "Parsing element type %d...\n", elem_type);

		
		elem_buf_len = _ax_tool_simple_nstrtoi(text_buf, 3);
		if (elem_buf_len < 0 || elem_buf_len > 999)
			return -1;
		text_buf += 3;
		buf_len -= 3;
		DBG(debug_octeon, "Parsing element len %d...\n", elem_buf_len);
		

		elem_data_addr = (char*)AX_SYSINFO_MALLOC(elem_buf_len+1); //malloc memory for element's data
		if (elem_data_addr < 0)
			return -1;
		memcpy(elem_data_addr, text_buf, elem_buf_len);
		elem_data_addr[elem_buf_len] = '\0'; //end the data with '\0'
		text_buf += elem_buf_len;
		buf_len -= elem_buf_len;

		//verify the element data's validation
		temp = elem_data_addr;
		while (*temp != '\0' && *temp != '\n' && *temp != '\r')
		{
			temp++;
		}

		if (temp != &elem_data_addr[elem_buf_len])
			return -1; //there exists invalid characters, return error!
		DBG(debug_octeon, "elem_string is %s.\n", elem_data_addr);


		dest_sysinfo_ptr->sysinfo_elem_array[i].element_type = elem_type;
		dest_sysinfo_ptr->sysinfo_elem_array[i].element_length = elem_buf_len;
		dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr = elem_data_addr;

		while (*text_buf != '\r' && *text_buf != '\n' && buf_len > 0) //skip char
		{
			text_buf++;
			buf_len--;
		}
		while ((*text_buf == '\r' || *text_buf == '\n') && buf_len > 0) //skip char
		{
			text_buf++;
			buf_len--;
		}

		i++;
		elem_count --;
	}
	
	return rcode;
}


/** 
  * copy from _ax_sysinfo_t to ax_sysinfo_product_t
  */
static void _ax_populate_global_bootinfo(_ax_sysinfo_t* dest_sysinfo_ptr, ax_sysinfo_product_t* sysinfo)
{
	int elem_count;
	int i = 0;
	int real_size = 0;

	elem_count = dest_sysinfo_ptr->sysinfo_header.total_element_count; 
	
	if (dest_sysinfo_ptr->sysinfo_elem_array == NULL)
		return;

	//copy to  sysinfo
	for (i=0; i<elem_count; i++)
	{
		if (dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr != NULL && dest_sysinfo_ptr->sysinfo_elem_array[i].element_length != 0)
		{
			switch (dest_sysinfo_ptr->sysinfo_elem_array[i].element_type)
			{
			case AX_SYSINFO_MODULE_SERIAL_NO:
				real_size = sizeof(sysinfo->ax_sysinfo_module_serial_no);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_module_serial_no, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_module_serial_no[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_module_serial_no, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);
				}
				break;
			case AX_SYSINFO_MODULE_NAME:
				real_size = sizeof(sysinfo->ax_sysinfo_module_name);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_module_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_module_name[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_module_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_PRODUCT_SERIAL_NO:
				real_size = sizeof(sysinfo->ax_sysinfo_product_serial_no);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_product_serial_no, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_product_serial_no[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_product_serial_no, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_PRODUCT_BASE_MAC_ADDRESS:
				real_size = sizeof(sysinfo->ax_sysinfo_product_base_mac_address);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_product_base_mac_address, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_product_base_mac_address[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_product_base_mac_address, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);
				}
				break;
			case AX_SYSINFO_PRODUCT_NAME:
				real_size = sizeof(sysinfo->ax_sysinfo_product_name);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_product_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_product_name[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_product_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_SOFTWARE_NAME:
				real_size = sizeof(sysinfo->ax_sysinfo_software_name);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_software_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_software_name[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_software_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_ENTERPRISE_NAME:
				real_size = sizeof(sysinfo->ax_sysinfo_enterprise_name);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_enterprise_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_enterprise_name[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_enterprise_name, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_ENTERPRISE_SNMP_OID:
				real_size = sizeof(sysinfo->ax_sysinfo_enterprise_snmp_oid);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_enterprise_snmp_oid, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_enterprise_snmp_oid[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_enterprise_snmp_oid, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_SNMP_SYS_OID:
				real_size = sizeof(sysinfo->ax_sysinfo_snmp_sys_oid);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_snmp_sys_oid, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_snmp_sys_oid[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_snmp_sys_oid, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_BUILT_IN_ADMIN_USERNAME:
				real_size = sizeof(sysinfo->ax_sysinfo_built_in_admin_username);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_built_in_admin_username, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_built_in_admin_username[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_built_in_admin_username, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			case AX_SYSINFO_BUILT_IN_ADMIN_PASSWORD:
				real_size = sizeof(sysinfo->ax_sysinfo_built_in_admin_password);
				if (dest_sysinfo_ptr->sysinfo_elem_array[i].element_length >= real_size)
				{
				    memcpy(sysinfo->ax_sysinfo_built_in_admin_password, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, real_size);
					sysinfo->ax_sysinfo_built_in_admin_password[real_size-1] = '\0';
				}
				else
				{
				    memcpy(sysinfo->ax_sysinfo_built_in_admin_password, dest_sysinfo_ptr->sysinfo_elem_array[i].data_buf_ptr, dest_sysinfo_ptr->sysinfo_elem_array[i].element_length);;
				}
				break;
			default:
				break;
			}
		}
	}
	
}



/** Read sysinfo buf from eeprom.
  * NOTE: Allocate memory inside this function! The memory should be released when not using!!
  *@param  eeprom_dev_addr   The eeprom's address in TWSI bus, 8 bit
  *@param  data_offset  The sysinfo data in eeprom's offset. 32bit addr 0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM. Here only use the lower 16bit.
  *@param  buf_len_ptr  A pointer to an integer, which will used to store the length of buffer allocated. NOT include the appended '\0' char!
  *@return   buffer's address, should be released using ax_sysinfo_buf_free() when not using!
  */
static unsigned char* _ax_twsi_read_sysinfo_buf_from_eeprom(uint8_t eeprom_dev_addr, uint32_t data_offset, uint32_t* buf_len_ptr)
{
	#define AX_SYSINFO_HEADER_BUF_LEN  11
	unsigned char header_buf[AX_SYSINFO_HEADER_BUF_LEN];
	char* buf_addr = NULL;
	unsigned int debug_ioctl = 0;
	struct mtd_info *mtd;
	char *mtd_name = "sys_info";
	int ret;
	unsigned long total;

	mtd = get_mtd_device_nm(mtd_name);
	if(IS_ERR(mtd))
	{
		DBG(debug_ioctl, "Get mtd %s error!\n", mtd_name);
		return -1;
	}
	
	data_offset &= 0xFFFF; //here only use the lower 16bit offset addr
	
	//first read the sysinfo header from eeprom, they are predefined 10 bytes
	total = AX_SYSINFO_HEADER_BUF_LEN-1;	
	ret = (*mtd->read)(mtd, (loff_t)data_offset, (size_t)total, (size_t*)&total, header_buf); 	
	if (ret || total != (AX_SYSINFO_HEADER_BUF_LEN-1))
	{
		*buf_len_ptr = -1; //read error
		return 0;
	}
	header_buf[AX_SYSINFO_HEADER_BUF_LEN-1] = 0; //end str with '\0'

	*buf_len_ptr = _ax_tool_simple_nstrtoi(&header_buf[AX_SYSINFO_HEADER_BUF_LEN-6], 5); //total buffer length
	if (*buf_len_ptr > 0xff)
	{
		DBG(debug_ioctl, "sysinfo total length 0x%x is bigger than the eeprom's total size\n", *buf_len_ptr);
		return 0; // bad head :the sysinfo total length is bigger than the eeprom's total size 	
	}
	
	//allocate memory for buffer to be written
	buf_addr = (char*)AX_SYSINFO_MALLOC(*buf_len_ptr); //malloc memory for buffer
	
	if (buf_addr <= 0)
	{
		*buf_len_ptr = -1; //malloc error
		return 0;
	}

	//read all sysinfo data from eeprom, write them to buffer
	total = *buf_len_ptr;	
	ret = (*mtd->read)(mtd, (loff_t)data_offset, (size_t)total, (size_t*)&total, buf_addr); 	
	if (ret || total != *buf_len_ptr)
	{
		*buf_len_ptr = -1; //read error
		return 0;
	}

	return buf_addr;
}


/** Free memory allocated using ax_read_sysinfo_buf_from_eeprom().
  *@return  0 for success
  */
static int _ax_sysinfo_buf_free(unsigned char* buf_addr, uint32_t buf_len)
{
	int rcode = 0;
	AX_SYSINFO_FREE(buf_addr, buf_len); //free memory 
	return rcode;
}


/**
  * Read sysinfo from eeprom.
  *@return: -1 for failure. 0 for success.
  */
static int ax_read_sysinfo_from_eeprom(uint8_t  eeprom_addr, ax_sysinfo_product_t* sysinfo)
{
	int rcode = 0;
	int buf_len = 0;
	unsigned char* buf_addr;
	_ax_sysinfo_t  tmp_ax_sysinfo;
	
	if (eeprom_addr <= 0 || sysinfo == NULL)
	{
		printk("sysinfo is NULL\n");
		return -1;
	}
	memset(sysinfo, 0x00, sizeof(ax_sysinfo_product_t));	
	buf_addr = _ax_twsi_read_sysinfo_buf_from_eeprom(eeprom_addr, 0x0000, &buf_len); //malloc buffer
	if (buf_addr <= 0x0000 || buf_len <= 0)
	{
		DBG(debug_ioctl, "malloc for buf_addr failed\n");
		return -1;
	}

	memset(&tmp_ax_sysinfo, 0x00, sizeof(_ax_sysinfo_t));
	rcode = _ax_populate_sysinfo_with_textbuf(&tmp_ax_sysinfo, buf_addr, buf_len);
	if (rcode < 0)
	{
		printk("_ax_populate_sysinfo_with_textbuf failed\n");
		return rcode;
	}

	_ax_populate_global_bootinfo(&tmp_ax_sysinfo, sysinfo);

	DBG(debug_ioctl, "sysinfo detail:\n");
	DBG(debug_ioctl, "module sn:%s\n",sysinfo->ax_sysinfo_module_serial_no);
	DBG(debug_ioctl, "module name:%s\n",sysinfo->ax_sysinfo_module_name);
	DBG(debug_ioctl, "product sn:%s\n",sysinfo->ax_sysinfo_product_serial_no);
	DBG(debug_ioctl, "product mac:%s\n",sysinfo->ax_sysinfo_product_base_mac_address);
	DBG(debug_ioctl, "product name:%s\n",sysinfo->ax_sysinfo_product_name);
	DBG(debug_ioctl, "sw name:%s\n",sysinfo->ax_sysinfo_software_name);
	DBG(debug_ioctl, "vendor name:%s\n", sysinfo->ax_sysinfo_enterprise_name);
	DBG(debug_ioctl, "snmp oid:%s\n",sysinfo->ax_sysinfo_enterprise_snmp_oid);
	DBG(debug_ioctl, "system oid:%s\n",sysinfo->ax_sysinfo_snmp_sys_oid);
	DBG(debug_ioctl, "admin username:%s\n",sysinfo->ax_sysinfo_built_in_admin_username);
	DBG(debug_ioctl, "admin password:%s\n",sysinfo->ax_sysinfo_built_in_admin_password);
	_ax_sysinfo_buf_free(buf_addr, buf_len); //release buffer
	
	return rcode;
}

int ax_read_sysinfo_from_eeprom_proc(uint8_t  eeprom_addr, ax_sysinfo_product_t* sysinfo)
{
	return ax_read_sysinfo_from_eeprom(eeprom_addr, sysinfo);
}

/////////////////////////////////////////////////////////////////////


/**
  * Read product sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int  bm_ax_read_sysinfo_product(uint8_t  eeprom_addr, ax_sysinfo_product_t* sysinfo)
{
	int rcode = 0;
	if (sysinfo == NULL || eeprom_addr <= 0)
		return -1;

	memset(sysinfo, 0x00, sizeof(ax_sysinfo_product_t));
	rcode = ax_read_sysinfo_from_eeprom(eeprom_addr, sysinfo);
	return rcode;
}


/**
  * Read single board sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int bm_ax_read_sysinfo_single_board(uint8_t  eeprom_addr, ax_sysinfo_single_board_t* sysinfo)
{
	int rcode = 0;
	ax_sysinfo_product_t  tmp_sysinfo_product;
	
	if (sysinfo == NULL || eeprom_addr <= 0)
		return -1;

	memset(sysinfo, 0x00, sizeof(ax_sysinfo_single_board_t));
	
	rcode = ax_read_sysinfo_from_eeprom(eeprom_addr, &tmp_sysinfo_product);
	if (rcode < 0)
		return rcode;

	memcpy(sysinfo->ax_sysinfo_module_name, tmp_sysinfo_product.ax_sysinfo_module_name, 24);
	memcpy(sysinfo->ax_sysinfo_module_serial_no, tmp_sysinfo_product.ax_sysinfo_product_serial_no, 20);
	return rcode;
}
/**
  * Read ¿Û°å sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int bm_ax_read_module0_sysinfo(uint8_t  eeprom_addr, ax_module_sysinfo* sysinfo)
{
	int rcode = 0;
	int i;
	ax_sysinfo_product_t  tmp_sysinfo_product;
	
	if (sysinfo == NULL || eeprom_addr <= 0)
		return -1;

	//memset(sysinfo, 0x00, sizeof(ax_sysinfo_single_board_t));
	memset(&tmp_sysinfo_product, 0x00, sizeof(ax_sysinfo_product_t));
	printk("read sysinfo\n");
	rcode = ax_read_sysinfo_from_eeprom(eeprom_addr, &tmp_sysinfo_product);
	if (rcode < 0)
	{
		printk("ax_read_sysinfo_from_eeprom failed\n");
		return rcode;
	}

	printk("module name is\n");
	printk("%s\n", tmp_sysinfo_product.ax_sysinfo_module_name);
	memcpy(sysinfo->ax_sysinfo_module_name, tmp_sysinfo_product.ax_sysinfo_module_name, 24);
	printk("\n module_serial_no is\n");

	printk("%s\n", tmp_sysinfo_product.ax_sysinfo_product_serial_no);
	memcpy(sysinfo->ax_sysinfo_module_serial_no, tmp_sysinfo_product.ax_sysinfo_module_serial_no, 20);
	return rcode;
}
/**
  * Read module sysinfo from eeprom.
  * @return: -1 for failure. 0 for sucess.
  */
int bm_ax_read_module_sysinfo(ax_module_sysinfo* sysinfo)  
{
	if (sysinfo == NULL)
	{
		printk("sysinfo is null\n");
		return -1;		
	}
	printk("Needed to be implemented.\n");
	return 0;
}
