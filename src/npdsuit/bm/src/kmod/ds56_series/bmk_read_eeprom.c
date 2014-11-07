
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/err.h>
#include <linux/file.h>
#include <linux/vfs.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/syscalls.h>
#include "bmk_read_eeprom.h"

/////////////////////////////////////////////////////////////////////
//the following code are transplanted from u-boot
#define AX_EEPROM_SYSINFO_MAX_SIZE         (0x01ff)
#define AX_EEPROM_SYSINFO_TUPLE_MAX_LEN    (3+3+128)
#define AX_EEPROM_SYSINFO_MAC_ADDR_LEN     (12)


#define AX_SYSINFO_MALLOC(x)   kmalloc(x, GFP_KERNEL)
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
			printk("%02x ", text_buf[index]);
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
	{
		return -1;
	}

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

int bm_i2c_read(int twsi_index, unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
#if 1
{	
	struct file *filp;
	struct i2c_client *client = NULL;
	struct i2c_msg i2c_msgs[2];
	char i2c_path[32];
	unsigned long timeout, read_time;
	int status;
	int i;

	memset((void *)i2c_path, 0, sizeof(i2c_path));
	memset((void *)i2c_msgs, 0, sizeof(i2c_msgs));
	
	sprintf(i2c_path, "/dev/i2c-%d", twsi_index);
	filp = filp_open(i2c_path, O_RDWR, 0);
	if(IS_ERR(filp))
	{
		DBG(debug_ioctl, "Get device file for %s failed!\n", i2c_path);
		return -1;
	}
	client = filp->private_data;
    if(client == NULL)
	{
		DBG(debug_ioctl, "Get i2c client of %s failed!\n", i2c_path);
		filp_close(filp, NULL);
		return -1;
	}

	i2c_msgs[0].addr = chip;
	i2c_msgs[0].flags = 0; /* write opt */
	i2c_msgs[0].len = alen;
	i2c_msgs[0].buf = kmalloc(alen * sizeof(unsigned char), GFP_KERNEL);
	if(!i2c_msgs[0].buf)
	{
		DBG(debug_ioctl, "%s kmalloc failed!\n", i2c_path);
		filp_close(filp, NULL);
		return -1;
    }
	memset((void *)i2c_msgs[0].buf, 0, alen);
	for(i = alen-1; i >= 0; i--)
		i2c_msgs[0].buf[i] = (unsigned char)(addr>>(i*8));

	i2c_msgs[1].addr = chip;
	i2c_msgs[1].flags = 1; /* read opt */
	i2c_msgs[1].len = len;
	i2c_msgs[1].buf = buffer;
	if(i2c_msgs[1].buf)
		memset((void *)i2c_msgs[1].buf, 0, len);
	
	/*
	 * Reads fail if the previous write didn't complete yet. We may
	 * loop a few times until this one succeeds, waiting at least
	 * long enough for one entire page write to work.
	 */
	timeout = jiffies + msecs_to_jiffies(25);
	do
	{
		read_time = jiffies;
		
		status = i2c_transfer(client->adapter, i2c_msgs, 2);

		//DBG(debug_ioctl, "read %zu@%d --> %d (%ld)\n", len, addr, status, jiffies);

		if (status == 2)
		{
		   	//DBG(debug_ioctl, "Read value from %x at %x successed.\n", chip, addr);
			kfree(i2c_msgs[0].buf);
			filp_close(filp, NULL);
			return 0;
		}

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(read_time, timeout));

	kfree(i2c_msgs[0].buf);
	filp_close(filp, NULL);
    return -1;
}
#else
{
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg i2c_msgs[2];
	struct file *filp;
	char i2c_path[32];
	int i;

	memset((void *)i2c_path, 0, sizeof(i2c_path));
	memset((void *)i2c_msgs, 0, sizeof(i2c_msgs));

	sprintf(i2c_path, "/dev/i2c-%d", twsi_index);
	filp = filp_open(i2c_path, O_RDWR, 0);
	if(IS_ERR(filp))
	{
		DBG(debug_ioctl, "%s open failed!\n", i2c_path);
		return -1;
	}
	
	i2c_data.nmsgs = 2;
	i2c_data.msgs = i2c_msgs;
	i2c_data.msgs[0].addr = chip;
	i2c_data.msgs[0].flags = 0; /* write opt */
	i2c_data.msgs[0].len = alen;
	i2c_data.msgs[0].buf = kmalloc(alen * sizeof(unsigned char), GFP_KERNEL);
	if(!i2c_data.msgs[0].buf)
	{
		DBG(debug_ioctl, "%s kmalloc failed!\n", i2c_path);  
		return -1;
    }
	memset((void *)i2c_data.msgs[0].buf, 0, alen);
	for(i = alen-1; i >= 0; i--)
		i2c_data.msgs[0].buf[i] = (unsigned char)(addr>>(i*8));
	
	i2c_data.msgs[1].addr = chip;
	i2c_data.msgs[1].flags = 1; /* read opt */
	i2c_data.msgs[1].len = len;
	i2c_data.msgs[1].buf = buffer;
	if(i2c_data.msgs[1].buf)
		memset((void *)i2c_data.msgs[1].buf, 0, len);
	
	//filp->f_op->unlocked_ioctl(filp, I2C_TIMEOUT, 0x500); /* timeout  */  
	//filp->f_op->unlocked_ioctl(filp, I2C_RETRIES, 1000);  /* retry times */
	
	if((filp->f_op->unlocked_ioctl(filp, I2C_RDWR, (unsigned long)&i2c_data)) < 0)
	{
		DBG(debug_ioctl, "%s read failed!\n", i2c_path);
		kfree(i2c_data.msgs[0].buf);
		filp_close(filp, NULL);
		return -1;
	}

	kfree(i2c_data.msgs[0].buf);
	filp_close(filp, NULL);
	
	return 0;
}
#endif

int bm_i2c_write(int twsi_index, unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len)
#if 1
{
	struct file *filp;
	struct i2c_client *client = NULL;
	struct i2c_msg i2c_msgs[2];
	char i2c_path[32];
	unsigned long timeout, write_time;
	int status;
	int	i;
	
	memset((void *)i2c_path, 0, sizeof(i2c_path));
	memset((void *)i2c_msgs, 0, sizeof(i2c_msgs));

	sprintf(i2c_path, "/dev/i2c-%d", twsi_index);
	filp = filp_open(i2c_path, O_RDWR, 0);
	if(IS_ERR(filp))
	{
		DBG(debug_ioctl, "Get device file for %s failed!\n", i2c_path);
		return -1;
	}
	client = filp->private_data;
    if(client == NULL)
	{
		DBG(debug_ioctl, "Get i2c client of %s failed!\n", i2c_path);
	    filp_close(filp, NULL);
		return -1;
	}

	i2c_msgs[0].addr = chip;
	i2c_msgs[0].flags = 0; /* write opt */
	i2c_msgs[0].len = (alen + len);
	i2c_msgs[0].buf = kmalloc((alen + len) * sizeof(unsigned char), GFP_KERNEL);
	if(!i2c_msgs[0].buf)
	{
		DBG(debug_ioctl, "%s kmalloc failed!\n", i2c_path);
	    filp_close(filp, NULL);
		return -1;
    }
	memset((void *)i2c_msgs[0].buf, 0, (alen + len));
	for(i = alen-1; i >= 0; i--)
		i2c_msgs[0].buf[i] = (unsigned char)(addr>>(i*8));
	memcpy((void *)&i2c_msgs[0].buf[alen], (const void *)buffer, len);

	/*
	 * Reads fail if the previous write didn't complete yet. We may
	 * loop a few times until this one succeeds, waiting at least
	 * long enough for one entire page write to work.
	 */
	timeout = jiffies + msecs_to_jiffies(25);
	do
	{
		write_time = jiffies;
		
		status = i2c_transfer(client->adapter, i2c_msgs, 1);

		//DBG(debug_ioctl, "write %zu@%d --> %d (%ld)\n", len, addr, status, jiffies);

		if (status == 1)
		{
		    //DBG(debug_ioctl, "Write value to %x at %x successed.\r\n", chip, addr);
			kfree(i2c_msgs[0].buf);
			filp_close(filp, NULL);
			return 0;
		}

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(write_time, timeout));
	
	kfree(i2c_msgs[0].buf);
	filp_close(filp, NULL);
    return -1;
}
#else
{
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg i2c_msgs[2];
	struct file *filp;
	char i2c_path[32];
	int	i;

	memset((void *)i2c_path, 0, sizeof(i2c_path));
	memset((void *)i2c_msgs, 0, sizeof(i2c_msgs));

	sprintf(i2c_path, "/dev/i2c-%d", twsi_index);
	filp = filp_open(i2c_path, O_RDWR, 0);
	if(IS_ERR(filp))
	{
		DBG(debug_ioctl, "%s open failed!\n", i2c_path);
		return -1;
	}

	i2c_data.nmsgs = 1;
	i2c_data.msgs = i2c_msgs;
	i2c_data.msgs[0].addr = chip;
	i2c_data.msgs[0].flags = 0; /* write opt */
	i2c_data.msgs[0].len = (alen + len);
	i2c_data.msgs[0].buf = kmalloc((alen + len) * sizeof(unsigned char), GFP_KERNEL);
	if(!i2c_data.msgs[0].buf)
	{
	    filp_close(file, NULL);
		DBG(debug_ioctl, "%s kmalloc failed!\n", i2c_path);  
		return -1;
    }
	memset((void *)i2c_data.msgs[0].buf, 0, (alen + len));
	for(i = alen-1; i >= 0; i--)
		i2c_data.msgs[0].buf[i] = (unsigned char)(addr>>(i*8));
	memcpy((void *)&i2c_data.msgs[0].buf[alen], (const void *)buffer, len);

	//filp->f_op->unlocked_ioctl(filp, I2C_TIMEOUT, 0x500); /* timeout  */  
	//filp->f_op->unlocked_ioctl(filp, I2C_RETRIES, 1000);  /* retry times */
	
	if((filp->f_op->unlocked_ioctl(filp, I2C_RDWR, (unsigned long)&i2c_data)) < 0)
	{
		DBG(debug_ioctl, "%s write failed!\n", i2c_path);
		kfree(i2c_data.msgs[0].buf);
		filp_close(filp, NULL);
		return -1;
	}
	
	kfree(i2c_data.msgs[0].buf);
	filp_close(filp, NULL);
	
	return 0;
}
#endif
/** 
  * copy from _ax_sysinfo_t to ax_sysinfo_product_t
  */
static void bmk_ds5652_populate_global_bootinfo(char* date_sysinfo_ptr, ax_sysinfo_product_t* sysinfo)
{
	int i = 0;
	char item_name[64];
	char item[256];
	char *item_name_start = NULL;
	char *item_name_end = NULL;
	int item_name_complete = 0;
	char *item_start = NULL;
	char *item_end = NULL;
	int item_complete = 0;
	
	for(i = 0; i < AX_EEPROM_SYSINFO_MAX_SIZE + 1; i++)
	{
	    //DBG(debug_ioctl, "%c ", date_sysinfo_ptr[i]);
	    if(i%16 == 15)
	    {
	        //DBG(debug_ioctl, "\r\n");
	    }
	    switch(date_sysinfo_ptr[i])
	    {
	        case '[':
				memset(item_name, 0, 64);
				item_start = NULL;
				item_end = NULL;
				item_name_complete = 0;
				item_name_start = date_sysinfo_ptr + i + 1;
				break;
	        case ']':
				item_name_end = date_sysinfo_ptr + i;
				if(item_name_start)
				{
				    memset(item, 0, 256);
				    item_name_complete = 1;
					memcpy(item_name, item_name_start, (item_name_end - item_name_start));
				}
				else
				{
				    item_name_start = NULL;
					item_name_end = NULL;
					item_name_complete = 0;
				}
				break;
			case '=':
				break;
	        case '<':
				if(item_name_complete)
				{
				    item_start = date_sysinfo_ptr + i + 1;
				}
				break;
	        case '>':
				if(item_name_complete)
				{
				    if(item_start)
				    {
    				    item_end = date_sysinfo_ptr + i;
    					if(strcmp(item_name, "ALIAS_NAME") == 0)
    					{
    					    memcpy(sysinfo->ax_sysinfo_module_name, item_start, (item_end - item_start) > 24 ? 24:(item_end - item_start));
    					    memcpy(sysinfo->ax_sysinfo_product_name, item_start, (item_end - item_start) > 24 ? 24:(item_end - item_start));
    					}
						else if(strcmp(item_name, "PRODUCT_NAME") == 0)
    					{
    					    memcpy(sysinfo->ax_sysinfo_module_name, item_start, (item_end - item_start) > 24 ? 24:(item_end - item_start));
    					    memcpy(sysinfo->ax_sysinfo_product_name, item_start, (item_end - item_start) > 24 ? 24:(item_end - item_start));
    					}
						else if(strcmp(item_name, "SERIAL_NO") == 0)
    					{
    					    memcpy(sysinfo->ax_sysinfo_module_serial_no, item_start, (item_end - item_start) > 31 ? 31:(item_end - item_start));
    					    memcpy(sysinfo->ax_sysinfo_product_serial_no, item_start, (item_end - item_start) > 31 ? 31:(item_end - item_start));
    					}
						else if(strcmp(item_name, "MAC") == 0)
    					{
    					    memcpy(sysinfo->ax_sysinfo_product_base_mac_address, item_start, (item_end - item_start) > 12 ? 12:(item_end - item_start));
    					}
						else if(strcmp(item_name, "OID") == 0)
    					{
    					    memcpy(sysinfo->ax_sysinfo_enterprise_snmp_oid, item_start, (item_end - item_start) > 128 ? 128:(item_end - item_start));
    					}
				    }
				}
		        item_name_complete = 0;
		        item_start = NULL;
				item_end = NULL;
				item_name_start = NULL;
			    item_name_end = NULL;
				memset(item_name, 0, 64);
				memset(item, 0, 256);
				break;
			default:
				break;
	    }
	}
}

int bmk_ds5652_twsi_eeprom_read_byte(uint8_t addr, unsigned short data_offset, char *buf_ptr)
{
	struct file *file;
	struct i2c_client *client = NULL;
	struct i2c_msg msg[2];
	unsigned long timeout, read_time;
	int status;
	char eeprom_offset[2];

	file = filp_open("/dev/i2c-1", O_RDWR, 0);
	if(IS_ERR(file))
	{
		DBG(debug_ioctl, "Get device file for /dev/i2c-1 failed!\n");
		return -1;
	}
	client = file->private_data;
    if(client == NULL)
	{
	    filp_close(file, NULL);
		DBG(debug_ioctl, "Get i2c client of /dev/i2c-1 failed!\n");
		return -1;
	}
	memset(msg, 0, sizeof(msg));
	
	eeprom_offset[0] = data_offset >> 8;
	eeprom_offset[1] = data_offset;
	
	msg[0].addr = addr;
	msg[0].len = 2;
	msg[0].buf = eeprom_offset;

	msg[1].addr = addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf_ptr;
	msg[1].len = 1;
	
	/*
	 * Reads fail if the previous write didn't complete yet. We may
	 * loop a few times until this one succeeds, waiting at least
	 * long enough for one entire page write to work.
	 */
	timeout = jiffies + msecs_to_jiffies(25);
	do
	{
		read_time = jiffies;
		
		status = i2c_transfer(client->adapter, msg, 2);

		//DBG(debug_ioctl, "read %zu@%d --> %d (%ld)\n",
		//		1, data_offset, status, jiffies);

		if (status == 2)
		{
		//    DBG(debug_ioctl, "Read value %x from eeprom at %x successed.\r\n", *buf_ptr, data_offset);
			filp_close(file, NULL);
			return 0;
		}

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(read_time, timeout));
	filp_close(file, NULL);
    return -1;
}

int bmk_ds5652_twsi_eeprom_write_byte(uint8_t addr, unsigned short data_offset, char data)
{
	struct file *file;
	struct i2c_client *client = NULL;
	struct i2c_msg msg[1];
	unsigned long timeout, read_time;
	int status;
	char eeprom_date[3];

	file = filp_open("/dev/i2c-1", O_RDWR, 0);
	if(IS_ERR(file))
	{
		DBG(debug_ioctl, "Get device file for /dev/i2c-1 failed!\n");
		return -1;
	}
	client = file->private_data;
    if(client == NULL)
	{
	    filp_close(file, NULL);
		DBG(debug_ioctl, "Get i2c client of /dev/i2c-1 failed!\n");
		return -1;
	}
	DBG(debug_ioctl, "Get i2c client of /dev/i2c-1.\n");
	
	memset(msg, 0, sizeof(msg));
	
	eeprom_date[0] = data_offset >> 8;
	eeprom_date[1] = data_offset;
	eeprom_date[2] = data;
	
	msg[0].addr = addr;
	msg[0].len = 3;
	msg[0].buf = eeprom_date;

	
	/*
	 * Reads fail if the previous write didn't complete yet. We may
	 * loop a few times until this one succeeds, waiting at least
	 * long enough for one entire page write to work.
	 */
	timeout = jiffies + msecs_to_jiffies(25);
	do
	{
		read_time = jiffies;
		status = i2c_transfer(client->adapter, msg, 1);

		//DBG(debug_ioctl, "write %zu@%d --> %d (%ld)\n",
		//		1, data_offset, status, jiffies);

		if (status == 1)
		{
		//    DBG(debug_ioctl, "Write %x to eeprom at %x successed.\r\n", data, data_offset);
			filp_close(file, NULL);
			return 0;
		}

		/* REVISIT: at HZ=100, this is sloooow */
		msleep(1);
	} while (time_before(read_time, timeout));
	filp_close(file, NULL);
    return -1;
}

static unsigned char* bmk_ds5652_twsi_read_sysinfo_buf_from_eeprom(uint8_t eeprom_dev_addr, uint32_t data_offset, uint32_t* buf_len_ptr)
{
    int status = 0, i = 0;
	char* buf_addr = NULL;
	//allocate memory for buffer to be written
	buf_addr = (char*)AX_SYSINFO_MALLOC(AX_EEPROM_SYSINFO_MAX_SIZE + 1); //malloc memory for buffer
	
	if (buf_addr == NULL)
	{
		*buf_len_ptr = -1; //malloc error
		return NULL;
	}
	
	for(i = 0; i < AX_EEPROM_SYSINFO_MAX_SIZE; i++)
    {
        status = bmk_ds5652_twsi_eeprom_read_byte(eeprom_dev_addr, data_offset + i, buf_addr + i);
		if(status != 0)
		{
		    break;
		}
    }
    *buf_len_ptr = i;
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
	char* buf_addr;
	if (eeprom_addr <= 0 || sysinfo == NULL)
	{
		printk("sysinfo is NULL\n");
		return -1;
	}
	memset(sysinfo, 0x00, sizeof(ax_sysinfo_product_t));	
	buf_addr = bmk_ds5652_twsi_read_sysinfo_buf_from_eeprom(eeprom_addr, 0x0000, &buf_len); //malloc buffer
	if (buf_addr <= 0x0000 || buf_len <= 0)
	{
		DBG(debug_ioctl, "malloc for buf_addr failed\n");
		return -1;
	}

	bmk_ds5652_populate_global_bootinfo(buf_addr + 0x34, sysinfo);/*0x34 is the start of sysinfo*/

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

