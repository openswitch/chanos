/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* bmk_ds6224_board_param.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		bm -- BOARD TXM9004 management (i2c, gpio, bootbus, cpld, ....) setting.
*
* UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/

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

#define DS6224_PCA9548A_1_ADDR	0x77
#define DS6224_PCA9548A_2_ADDR	0x75
#define DS6224_CPLD0_ADDR		0x3C
#define DS6224_CPLD1_ADDR		0x3D

int ds6224_do_cpld_reg_read(int reg_addr, unsigned char *reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;

	twsi_index = 1;
	if(reg_offset & 0x80) /* CPLD1 */
	{
		chip = DS6224_CPLD1_ADDR;
		addr = reg_offset & (~0x80);
	}
	else /* CPLD0 */
	{
		chip = DS6224_CPLD0_ADDR;
		addr = reg_offset;
	}
	
	ret = bm_i2c_read(twsi_index, chip, addr, 1, reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "read cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		//DBG(debug_ioctl, "read %#016llx got %#0x\n", addr, *reg_data);
		DBG(debug_ioctl, "read reg=0x%x, data=0x%x\n", addr, *reg_data);
	}
	
	return ret;
}

int ds6224_do_cpld_reg_write(int reg_addr, unsigned char reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;
	
	twsi_index = 1;
	if(reg_offset & 0x80) /* CPLD1 */
	{
		chip = DS6224_CPLD1_ADDR;
		addr = reg_offset & (~0x80);
	}
	else /* CPLD0 */
	{
		chip = DS6224_CPLD0_ADDR;
		addr = reg_offset;
	}
	
	ret = bm_i2c_write(twsi_index, chip, addr, 1, &reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "write cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		//DBG(debug_ioctl, "write %#016llx got %#0x\n", addr, reg_data);
		DBG(debug_ioctl, "write reg=0x%x, data=0x%x\n", addr, reg_data);
	}
	
	return ret;
}

int ds6224_board_ioctl_cpld_read(struct file *filp, unsigned int cmd, unsigned long arg)
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

int ds6224_board_ioctl_cpld_write(struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_reg_args cpld_reg_data;
	int op_ret;
	int ret;

	memset((void *)&cpld_reg_data, 0, sizeof(cpld_reg_args));
	
	op_ret = copy_from_user(&cpld_reg_data,(cpld_reg_args *)arg,sizeof(cpld_reg_args));
	ret = ds6224_do_cpld_reg_write(cpld_reg_data.reg, cpld_reg_data.value);
	op_ret = copy_to_user((cpld_reg_args *)arg,&cpld_reg_data,sizeof(cpld_reg_args));

	return ret;
}

int ds6224_board_ioctl_sfp_num(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int sfp_num = 24;
	int ret;
	
	ret = __put_user(sfp_num, (int __user *)arg);
	
	return ret;
}

int ds6224_portindex_to_sfpindex(int port_index)
{
	return port_index + 0;
}


int ds6224_sfpindex_to_portindex(int sfp_index)
{
	return sfp_index - 0;
}

int ds6224_do_sfp_cpld_op(unsigned int cmd, sfp_op_args *ptr_sfp_data)
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

	if((ptr_sfp_data->index < 0) || (ptr_sfp_data->index > 23))
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

	ret = ds6224_do_cpld_reg_read(sfp_cpld_reg, &value);
	if (ret != 0)
		return ret;
	DBG(debug_ioctl, "sfp_cpld_reg is %x, value is %x.\n", sfp_cpld_reg, value);
	
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
		ret = ds6224_do_cpld_reg_write(sfp_cpld_reg, value);
	}
	
	return ret;

}

int ds6224_board_ioctl_cpld_sfp(struct file *filp, unsigned int cmd, unsigned long arg)
{
	sfp_op_args sfp_data;
	int op_ret;
	int ret;
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	
	op_ret = copy_from_user(&sfp_data, (sfp_op_args __user *)arg, sizeof(sfp_op_args));

	sfp_data.index = ds6224_portindex_to_sfpindex(sfp_data.index);
	ret = ds6224_do_sfp_cpld_op(cmd, &sfp_data);
	DBG(debug_ioctl, "ret is %d. index is %d, rwflag is %d, value is %d\n", 
				ret, sfp_data.index, sfp_data.rwflag, sfp_data.value);
	sfp_data.index = ds6224_sfpindex_to_portindex(sfp_data.index);
	
	op_ret = copy_to_user((sfp_op_args __user *)arg, &sfp_data,sizeof(sfp_op_args));
	
	return ret;
}

static int ds6224_select_sfp_channel(int channel)
{    
	unsigned char value;
	int ret;

	if((channel < 0) || (channel > 23))
		return -1;
	
	value = 0x01 << (channel/8);
	ret = bm_i2c_write(1, DS6224_PCA9548A_1_ADDR, value, 1, NULL, 0);

	value = 0x01 << (channel%8);
	ret = bm_i2c_write(1, DS6224_PCA9548A_2_ADDR, value, 1, NULL, 0);
	
	return ret;
}

static int ds6224_restore_sfp_channel(int channel)
{	
	unsigned char value;
	int ret;

	if((channel < 0) || (channel > 23))
		return -1;
	
	value = 0x00;
	
	/* TWSI1 SFP+ */
	ret = bm_i2c_write(1, DS6224_PCA9548A_2_ADDR, value, 1, NULL, 0);
	ret = bm_i2c_write(1, DS6224_PCA9548A_1_ADDR, value, 1, NULL, 0);
	
	return ret;
}

static int ds6224_i2c_read_sfp(unsigned char addr, unsigned short offset, unsigned char *buf,  int length)
{	
	return bm_i2c_read(1, addr, offset, 1, buf, length);
}

static int ds6224_i2c_write_sfp(unsigned char addr, unsigned short offset, unsigned char *buf,  int length)
{	
	return bm_i2c_write(1, addr, offset, 1, buf, length);
}

int ds6224_board_ioctl_i2c_sfp_operate(struct file *filp, unsigned int cmd, unsigned long arg)
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
	sfp_data.index = ds6224_portindex_to_sfpindex(sfp_data.index);
	ret = ds6224_select_sfp_channel(sfp_data.index);
	if (ret != 0)
	{
		DBG(debug_ioctl, "ds6224_select_sfp_channel is wrong.\n");
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
		ret = ds6224_i2c_read_sfp(sfp_data.reg_addr, sfp_data.value, ksfp_buf, sfp_data.buf_len);
		if (ret != 0)
		{
			DBG(debug_ioctl, "ds6224_i2c_read_sfp is wrong.\n");
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
		ret = ds6224_i2c_write_sfp(sfp_data.reg_addr, sfp_data.value, ksfp_buf, sfp_data.buf_len);
		if (ret != 0)
		{
			DBG(debug_ioctl, "ds6224_i2c_write_sfp is wrong.\n");
			goto err;
		}
	}
	
	//restore the sfp
	ret = ds6224_restore_sfp_channel(sfp_data.index);
	if (ret != 0)
	{
		DBG(debug_ioctl, "ds6224_restore_sfp_channel is wrong.\n");
		goto err;
	}
	sfp_data.index = ds6224_sfpindex_to_portindex(sfp_data.index);
	
	op_ret = copy_to_user((sfp_op_args __user *)arg, &sfp_data, sizeof(sfp_op_args));
	
err:
	if (ksfp_buf)
	{
		kfree(ksfp_buf);
		ksfp_buf = NULL;
	}
	
	return ret;
}

int ds6224_board_ioctl_led_light(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	

	return 0;
}

int ds6224_board_ioctl_i2c_write(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	return 0;
}

cpld_reg_ctl ds6224_cpld_ctrl[] = 
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

ioctl_proc ds6224_board_ioctl_proc_arr[] = 
{
	{BM_IOC_CPLD_REG_READ, 			ds6224_board_ioctl_cpld_read},
	{BM_IOC_CPLD_REG_WRITE, 		ds6224_board_ioctl_cpld_write},
		
	{BM_IOC_SFP_NUM,				ds6224_board_ioctl_sfp_num},	
	{BM_IOC_SFP_LIGHT,				ds6224_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_PRESENSE,			ds6224_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_LOS,			ds6224_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_TX_FAULT,		ds6224_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_OPERATE,			ds6224_board_ioctl_i2c_sfp_operate},
		
	{BM_IOC_WIRELESS_LED_LIGTH,		ds6224_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 	ds6224_board_ioctl_i2c_write},
};


int ds6224_board_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds6224_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds6224_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	ko_board->cpld_reg_ctrl_count = LENGTH(ds6224_cpld_ctrl);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":cpld reg count is %d\n", ko_board->cpld_reg_ctrl_count);
	
	return result;
}

void ds6224_board_cleanup(void)
{
	if (NULL != ko_board->cpld_int_data)
	{
		kfree(ko_board->cpld_int_data);
	}
		
	ko_board->cpld_int_data = NULL;
}

kboard_fix_param ds6224_board  =
{	
	.board_code = PPAL_BOARD_HWCODE_DS6224,
	.board_type = PPAL_BOARD_TYPE_DS6224,
	.board_name = "DS6224",
		
	.board_init = ds6224_board_init,
	.board_cleanup = ds6224_board_cleanup,
	.ioctl		= bm_board_ioctl,
	.ioctl_proc_arr = ds6224_board_ioctl_proc_arr,
	
	.interrupt_handler = bm_board_interrupt_handler,

	.cpld_reg_base_addr = CPLD_REG_BASE_ADDR,
	.cpld_reg_ctrl_arr = ds6224_cpld_ctrl,
};


