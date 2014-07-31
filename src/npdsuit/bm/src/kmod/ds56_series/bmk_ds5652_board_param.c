
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


#define I2C_DEV_NUM	1

#define GP0_REG 0
#define GP1_REG 1
#define IODIR0_REG 6
#define IODIR1_REG 7

#define WIRELESS_LED_NUM 6

#define I2C_ADDR_LEN 1

typedef int sa_i32_t;
typedef unsigned int sa_u32_t;
typedef unsigned char sa_u8_t;


struct task_struct * ds5600_board_led_task = NULL;

int ds5600_board_slot_index_get(void);

void ds5600_board_led_light(int light, int cycle)
{
		
}

int ds5600_board_led_thread(void *data)
{
	
	return 0;
}

int ds5600_board_create_led_thread(void)
{
	int err;
#if 0	
    ds5600_board_led_task = kthread_create(ds5600_board_led_thread, NULL, "kwled");

	if (IS_ERR(ds5600_board_led_task))
	{
	    DBG(debug_ioctl, "Unable to start kernel thread kwled\n");
		err = PTR_ERR(ds5600_board_led_task);
		ds5600_board_led_task = NULL;
		return err;
	}

	wake_up_process(ds5600_board_led_task);
#endif
	return 0;
}

void ds5600_board_led_prepare_data(wled_port_args *wled_port)
{


}


int ds5600_board_ioctl_led_light(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	

	return 0;
}

int ds5600_board_ioctl_i2c_write(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	return 0;
}


ioctl_proc ds5600_board_ioctl_proc_arr[] = 
{
	{BM_IOC_WIRELESS_LED_LIGTH,			ds5600_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 		       ds5600_board_ioctl_i2c_write},
};


int ds5600_board_init(void)
{
	int i, result = 0;
	unsigned char gp_data[] = {0xFF, 0xFF};
	unsigned char iodir_data[] = {0x00, 0x00};
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds5600_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds5600_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	return result;
}

void ds5600_board_cleanup(void)
{
	/* just do nothing */
	//free_irq(ko_board->irq_board, &bm_dev);
	if (NULL != ko_board->cpld_int_data)
	{
		kfree(ko_board->cpld_int_data);
	}
		
	ko_board->cpld_int_data = NULL;
}

kboard_fix_param ds5600_board  =
{	
	.board_code = PPAL_BOARD_HWCODE_DS5652,
	.board_type = PPAL_BOARD_TYPE_DS5652,
	.board_name = "DS5652",
		
	.board_init = ds5600_board_init,
	.board_cleanup = ds5600_board_cleanup,
	.ioctl		= bm_board_ioctl,
	.ioctl_proc_arr = ds5600_board_ioctl_proc_arr,
	
	.interrupt_handler = bm_board_interrupt_handler,

	.cpld_reg_base_addr = CPLD_REG_BASE_ADDR,
};


