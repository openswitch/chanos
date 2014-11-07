
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

#define DS5652_PCA9548A_1_ADDR	0x77
#define DS5652_PCA9548A_2_ADDR	0x75
#define DS5652_CPLD0_ADDR		0x3C
#define DS5652_CPLD1_ADDR		0x3D

int ds5652_do_cpld_reg_read(int reg_addr, unsigned char *reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;

	twsi_index = 1;
	if(reg_offset & 0x80) /* CPLD1 */
	{
		chip = DS5652_CPLD1_ADDR;
		addr = reg_offset & (~0x80);
	}
	else /* CPLD0 */
	{
		chip = DS5652_CPLD0_ADDR;
		addr = reg_offset;
	}
	
	ret = bm_i2c_read(twsi_index, chip, addr, 1, reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "read cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		DBG(debug_ioctl, "read reg=0x%x, data=0x%x\n", addr, *reg_data);
	}
	
	return ret;
}

int ds5652_do_cpld_reg_write(int reg_addr, unsigned char reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;
	
	twsi_index = 1;
	if(reg_offset & 0x80) /* CPLD1 */
	{
		chip = DS5652_CPLD1_ADDR;
		addr = reg_offset & (~0x80);
	}
	else /* CPLD0 */
	{
		chip = DS5652_CPLD0_ADDR;
		addr = reg_offset;
	}
	
	ret = bm_i2c_write(twsi_index, chip, addr, 1, &reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "write cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		DBG(debug_ioctl, "write reg=0x%x, data=0x%x\n", addr, reg_data);
	}
	
	return ret;
}

int ds5652_board_ioctl_cpld_read(struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_reg_args cpld_reg_data;
	int op_ret;
	int ret;
	
	memset((void *)&cpld_reg_data, 0, sizeof(cpld_reg_args));
	
	op_ret = copy_from_user(&cpld_reg_data, (cpld_reg_args *)arg, sizeof(cpld_reg_args));
	ret = ds5652_do_cpld_reg_read(cpld_reg_data.reg, &cpld_reg_data.value);
	op_ret = copy_to_user(((cpld_reg_args *)arg), &cpld_reg_data, sizeof(cpld_reg_args));

	return ret;
}

int ds5652_board_ioctl_cpld_write(struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_reg_args cpld_reg_data;
	int op_ret;
	int ret;

	memset((void *)&cpld_reg_data, 0, sizeof(cpld_reg_args));
	
	op_ret = copy_from_user(&cpld_reg_data,(cpld_reg_args *)arg,sizeof(cpld_reg_args));
	ret = ds5652_do_cpld_reg_write(cpld_reg_data.reg, cpld_reg_data.value);
	op_ret = copy_to_user((cpld_reg_args *)arg,&cpld_reg_data,sizeof(cpld_reg_args));

	return ret;
}

int ds5652_board_ioctl_sfp_num(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int sfp_num = 52;
	int ret;
	
	ret = __put_user(sfp_num, (int __user *)arg);
	
	return ret;
}

int ds5652_portindex_to_sfpindex(int port_index)
{
	return port_index + 0;
}


int ds5652_sfpindex_to_portindex(int sfp_index)
{
	return sfp_index - 0;
}

int ds5652_do_sfp_cpld_op(unsigned int cmd, sfp_op_args *ptr_sfp_data)
{
	cpld_reg_ctl* ptr_cpld_ctl;
	int sfp_cpld_reg;
	int sfp_cpld_shift;
	unsigned char sfp_cpld_mask;
	unsigned char value, temp;
	int ret;
	
	ptr_cpld_ctl = util_get_cmd_reg_ctrl(cmd);
	if (ptr_cpld_ctl == NULL)
	{
		DBG(debug_ioctl, "sfp_op: can't find the cmd.\n");
		return -1;
	}

	if((ptr_sfp_data->index < 0) || (ptr_sfp_data->index > 51))
	{
		DBG(debug_ioctl, "sfp_op: can't find the index.\n");
		return -1;
	}
	
	// because index is from 0-4
	// calculate the reg offset
	sfp_cpld_reg = ptr_cpld_ctl->offset + ptr_sfp_data->index/8;
	DBG(debug_ioctl, "sfp_cpld_reg is %x, index is %d.\n", sfp_cpld_reg, ptr_sfp_data->index);

	//calculate the shift and mask.
	//index mod 8 is 3, the mask is (000000001)b << 3 -> (00001000), and shift is the 3
	sfp_cpld_shift = ptr_sfp_data->index%8;
	sfp_cpld_mask = 0x01 << sfp_cpld_shift;
	DBG(debug_ioctl, "sfp_cpld_shift is %x, sfp_cpld_mask is %x.\n", sfp_cpld_shift, sfp_cpld_mask);

	ret = ds5652_do_cpld_reg_read(sfp_cpld_reg, &value);
	if (ret != 0)
		return ret;
	DBG(debug_ioctl, "sfp_cpld_reg is %x, value is %x.\n", sfp_cpld_reg, value);
		
	
	if(ptr_sfp_data->index < 48) /* TWSI1 SFP+ */
	{
		switch(cmd)
		{
			case BM_IOC_SFP_LIGHT:
				break;
			case BM_IOC_SFP_PRESENSE:
				break;
			case BM_IOC_SFP_DMI_LOS:
				break;
			case BM_IOC_SFP_DMI_TX_FAULT:
				break;
			default:
				DBG(debug_ioctl, "sfp not support the cmd, index is %d.\n", ptr_sfp_data->index);
				break;
		}
	}
	else /* TWSI1 QSFP */
	{
		switch(cmd)
		{
			case BM_IOC_SFP_PRESENSE:
				break;
			default:
				DBG(debug_ioctl, "qsfp not support the cmd, index is %d.\n", ptr_sfp_data->index);
				break;
		}
	}
	
	if (!ptr_sfp_data->rwflag ) /* read opt */
	{
		temp = (value & sfp_cpld_mask) >> sfp_cpld_shift;
		DBG(debug_ioctl, "sfp_cpld_reg is %x, temp is %x.\n", sfp_cpld_reg, temp);
		switch(cmd)
		{
			case BM_IOC_SFP_LIGHT:
				ptr_sfp_data->value = (temp ? SFP_LIGHT_OFF : SFP_LIGHT_ON);
				break;
			case BM_IOC_SFP_PRESENSE:
				ptr_sfp_data->value = (temp ? SFP_REMOVE : SFP_ONLINE);
				break;
			case BM_IOC_SFP_DMI_LOS:
				ptr_sfp_data->value = (temp ? SFP_DMI_ALARM_LOS : SFP_DMI_NORMAL);
				break;
			case BM_IOC_SFP_DMI_TX_FAULT:
				ptr_sfp_data->value = (temp ? SFP_DMI_ALARM_TX_FAULT : SFP_DMI_NORMAL);
				break;
			case BM_IOC_XFP_CDR:
				ptr_sfp_data->value = (temp ? XFP_CDR_LOCKED : XFP_CDR_UNLOCKED);
				break;
			default:
				DBG(debug_ioctl, "sfp read not support the cmd, index is %d.\n", ptr_sfp_data->index);
				break;
		}
	}
	else /* write opt */
	{
		value &= ~(sfp_cpld_mask);
		switch(cmd)
		{
			case BM_IOC_SFP_LIGHT:
				temp = (ptr_sfp_data->value == SFP_LIGHT_OFF);
				break;
			default:
				DBG(debug_ioctl, "sfp write not support the cmd, index is %d.\n", ptr_sfp_data->index);
				break;
		}
		temp = (temp << sfp_cpld_shift) & sfp_cpld_mask;
		value |= temp;
		ret = ds5652_do_cpld_reg_write(sfp_cpld_reg, value);
	}
	
	return ret;

}

int ds5652_board_ioctl_cpld_sfp(struct file *filp, unsigned int cmd, unsigned long arg)
{
	sfp_op_args sfp_data;
	int op_ret;
	int ret;
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	
	op_ret = copy_from_user(&sfp_data, (sfp_op_args __user *)arg, sizeof(sfp_op_args));

	sfp_data.index = ds5652_portindex_to_sfpindex(sfp_data.index);
	ret = ds5652_do_sfp_cpld_op(cmd, &sfp_data);
	DBG(debug_ioctl, "ret is %d. index is %d, rwflag is %d, value is %d\n", 
				ret, sfp_data.index, sfp_data.rwflag, sfp_data.value);
	sfp_data.index = ds5652_sfpindex_to_portindex(sfp_data.index);
	
	op_ret = copy_to_user((sfp_op_args __user *)arg, &sfp_data,sizeof(sfp_op_args));
	
	return ret;
}

static int ds5652_select_sfp_channel(int channel)
{    
	unsigned char value;
	int ret;

	if((channel < 0) || (channel > 51))
		return -1;
	
	value = 0x01 << (channel/8);
	ret = bm_i2c_write(1, DS5652_PCA9548A_1_ADDR, value, 1, NULL, 0);

	value = 0x01 << (channel%8);
	if (channel < 48) /* TWSI1 SFP+ */
	{
		ret = bm_i2c_write(1, DS5652_PCA9548A_2_ADDR, value, 1, NULL, 0);
	}
	else /* TWSI1 QSFP */
	{	
		value = (~value) & 0x0F;
		ret = ds5652_do_cpld_reg_write(0x8A, value); /* 0x80|0x0A */
	}
	
	return ret;
}

static int ds5652_restore_sfp_channel(int channel)
{	
	unsigned char value;
	int ret;

	if((channel < 0) || (channel > 51))
		return -1;
	
	value = 0x00;
	
	if (channel < 48) /* TWSI1 SFP+ */
	{
		ret = bm_i2c_write(1, DS5652_PCA9548A_2_ADDR, value, 1, NULL, 0);
	}
	else /* TWSI1 QSFP */
	{
		ret = ds5652_do_cpld_reg_write(0x8A, 0x0F); /* 0x80|0x0A */
	}
	
	ret = bm_i2c_write(1, DS5652_PCA9548A_1_ADDR, value, 1, NULL, 0);
	
	return ret;
}

static int ds5652_i2c_read_sfp(unsigned char addr, unsigned short offset, unsigned char *buf,  int length)
{	
	return bm_i2c_read(1, addr, offset, 1, buf, length);
}

static int ds5652_i2c_write_sfp(unsigned char addr, unsigned short offset, unsigned char *buf,  int length)
{	
	return bm_i2c_write(1, addr, offset, 1, buf, length);
}

int ds5652_board_ioctl_i2c_sfp_operate(struct file *filp, unsigned int cmd, unsigned long arg)
{
	sfp_op_args sfp_data;
	char* ksfp_buf;
	int op_ret;
	int ret;
	int i;
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));

	op_ret = copy_from_user(&sfp_data, (sfp_op_args __user *)arg, sizeof(sfp_op_args));

	if (sfp_data.buf_len <= 0)
	{
		DBG(debug_ioctl, "buf length must greater than zero.\n");
		return -1;
	}
	
	ksfp_buf = (char *)kmalloc(sfp_data.buf_len, GFP_KERNEL);
	if (!ksfp_buf)
	{
		DBG(debug_ioctl, "ksfp_buf kmalloc memory fail.\n");
		return -ENOMEM;
	}
	memset(ksfp_buf, 0, sizeof(sfp_data.buf_len));
	
	//select the sfp module
	sfp_data.index = ds5652_portindex_to_sfpindex(sfp_data.index);
	ret = ds5652_select_sfp_channel(sfp_data.index);
	if (ret != 0)
	{
		DBG(debug_ioctl, "ds5652_select_sfp_channel is wrong.\n");
		goto err;
	}

	DBG(debug_ioctl, "sfp_data.index = %d\n", sfp_data.index);
	DBG(debug_ioctl, "sfp_data.reg_addr = 0x%x\n", sfp_data.reg_addr);
	DBG(debug_ioctl, "sfp_data.value = 0x%x\n", sfp_data.value);
	DBG(debug_ioctl, "sfp_data.buf_len = %d\n", sfp_data.buf_len);
	
	if (!sfp_data.rwflag) /* read opt */
	{
		/* 
		 * sfp_data.reg_addr is dev_addr: 0xa0 or 0xa2
		 * sfp_data.value is reg_addr: 0x00 ~ 0xFF
		 */
		ret = ds5652_i2c_read_sfp(sfp_data.reg_addr, sfp_data.value, ksfp_buf, sfp_data.buf_len);
		if (ret != 0)
		{
			DBG(debug_ioctl, "ds5652_i2c_read_sfp is wrong.\n");
			goto err;
		}
#if 0
		for (i = 0; i < sfp_data.buf_len; i++)
		{
			if (i != 0 && (i % 16) == 0)
			{
				printk(KERN_DEBUG "\n");
			}
			printk(KERN_DEBUG "%02hhx ", ksfp_buf[i]);
		}
#endif
		op_ret = copy_to_user((char *)sfp_data.buf, ksfp_buf, sfp_data.buf_len);
	}
	else /* write opt */
	{
		op_ret = copy_from_user(ksfp_buf, (char __user *)sfp_data.buf, sfp_data.buf_len);

		/* 
		 * sfp_data.reg_addr is dev_addr: 0xa0 or 0xa2
		 * sfp_data.value is reg_addr: 0x00 ~ 0xFF
		 */
		ret = ds5652_i2c_write_sfp(sfp_data.reg_addr, sfp_data.value, ksfp_buf, sfp_data.buf_len);
		if (ret != 0)
		{
			DBG(debug_ioctl, "ds5652_i2c_write_sfp is wrong.\n");
			goto err;
		}
	}
	
	//restore the sfp
	ret = ds5652_restore_sfp_channel(sfp_data.index);
	if (ret != 0)
	{
		DBG(debug_ioctl, "ds5652_restore_sfp_channel is wrong.\n");
		goto err;
	}
	sfp_data.index = ds5652_sfpindex_to_portindex(sfp_data.index);
	
	op_ret = copy_to_user((sfp_op_args __user *)arg, &sfp_data, sizeof(sfp_op_args));
	
err:
	if (ksfp_buf)
	{
		kfree(ksfp_buf);
		ksfp_buf = NULL;
	}
	
	return ret;
}

void ds5600_board_led_light(int light, int cycle)
{
		
}

int ds5600_board_led_thread(void *data)
{
	
	return 0;
}

int ds5600_board_create_led_thread(void)
{
#if 0	
	int err;
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

cpld_reg_ctl ds5652_cpld_ctrl[] = 
{
 	/* CPLD0 */
	{BM_IOC_CPLD_POWER_PRESENT, 0x00, 1, 0xC0, 0, NULL}, /* 0x00 */
	{BM_IOC_POWER_STATE, 		0x00, 1, 0x30, 0, NULL}, /* 0x00 */
	
	////{BM_IOC_CPLD_FAN_PRESENT, 	0x00, 1, 0x30, 0, NULL}, /* fan() 特殊处理 */
	{BM_IOC_CPLD_FAN_ALARM, 	0x00, 1, 0x0E, 0, NULL}, /* 0x00 */
	{BM_IOC_CPLD_FAN_SPEED, 	0x13, 1, 0x0F, 0, NULL}, /* 0x13 */
	
	{BM_IOC_SFP_DMI_LOS,      	0x01, 1, 0xFF, 0, NULL}, /* 0x01~0x06 */
	{BM_IOC_SFP_DMI_TX_FAULT,   0x07, 1, 0xFF, 0, NULL}, /* 0x07~0x0c */
	{BM_IOC_SFP_LIGHT,			0x0d, 1, 0xFF, 0, NULL}, /* 0x0d~0x12 */
	//{BM_IOC_CPLD_TEST,			0x3e, 1, 0xFF, 0, NULL},
	//{BM_IOC_CPLD_CPLD_VERSION,	0x3f, 1, 0xFF, 0, NULL},

	/* CPLD1 "reg |= 0x80" */
	//{BM_IOC_CPLD_WDT_ENABLE,  	0x88, 1, 0x0F, 0, NULL}, /* 0x08 */
	//{BM_IOC_CPLD_WDT_TIMER,  		0x88, 1, 0x0F, 0, NULL}, /* 0x08 */
	//{BM_IOC_CPLD_WDT_CLEAR,  		0x80, 1, 0xFF, 0, NULL}, /* fan() 特殊处理 */
	{BM_IOC_SFP_PRESENSE,  		0x81, 1, 0x0F, 0, NULL}, /* 0x01~0x07 */
	//{BM_IOC_CPLD_TEST,			0x9d, 1, 0xFF, 0, NULL}, /* 0x1d */
	//{BM_IOC_CPLD_CPLD_VERSION,	0x9e, 1, 0xFF, 0, NULL}, /* 0x1e */
};

ioctl_proc ds5600_board_ioctl_proc_arr[] = 
{
	{BM_IOC_CPLD_REG_READ, 			ds5652_board_ioctl_cpld_read},
	{BM_IOC_CPLD_REG_WRITE, 		ds5652_board_ioctl_cpld_write},
		
	{BM_IOC_SFP_NUM,				ds5652_board_ioctl_sfp_num},	
	{BM_IOC_SFP_LIGHT,				ds5652_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_PRESENSE,			ds5652_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_LOS,			ds5652_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_TX_FAULT,		ds5652_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_OPERATE,			ds5652_board_ioctl_i2c_sfp_operate},
		
	{BM_IOC_WIRELESS_LED_LIGTH,		ds5600_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 	ds5600_board_ioctl_i2c_write},
};


int ds5600_board_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds5600_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds5600_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	ko_board->cpld_reg_ctrl_count = LENGTH(ds5652_cpld_ctrl);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":cpld reg count is %d\n", ko_board->cpld_reg_ctrl_count);
	
	return result;
}

void ds5600_board_cleanup(void)
{
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
	.cpld_reg_ctrl_arr = ds5652_cpld_ctrl,
};


