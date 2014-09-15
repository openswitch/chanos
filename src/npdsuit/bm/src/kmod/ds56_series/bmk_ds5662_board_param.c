
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"
#include "bmk_operation_boot_env.h"
#include "bmk_read_eeprom.h"


int ds5662_board_ioctl_led_light(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	

	return 0;
}

int ds5662_board_ioctl_i2c_write(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	return 0;
}


ioctl_proc ds5662_board_ioctl_proc_arr[] = 
{
	{BM_IOC_WIRELESS_LED_LIGTH,			ds5662_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 		       ds5662_board_ioctl_i2c_write},
};


int ds5662_board_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds5662_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds5600_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	return result;
}

void ds5662_board_cleanup(void)
{
	if (NULL != ko_board->cpld_int_data)
	{
		kfree(ko_board->cpld_int_data);
	}
		
	ko_board->cpld_int_data = NULL;
}

kboard_fix_param ds5662_board  =
{	
	.board_code = PPAL_BOARD_HWCODE_DS5662,
	.board_type = PPAL_BOARD_TYPE_DS5662,
	.board_name = "DS5660",
		
	.board_init = ds5662_board_init,
	.board_cleanup = ds5662_board_cleanup,
	.ioctl		= bm_board_ioctl,
	.ioctl_proc_arr = ds5662_board_ioctl_proc_arr,
	
	.interrupt_handler = bm_board_interrupt_handler,

	.cpld_reg_base_addr = CPLD_REG_BASE_ADDR,
};


