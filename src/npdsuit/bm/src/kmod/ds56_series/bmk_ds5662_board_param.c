
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

#define DS5662_CPLD0_ADDR	0x66
#define DS5662_CPLD1_ADDR	0x67
#define DS5662_CPLD2_ADDR	0x65

int ds5662_do_cpld_reg_read(int reg_addr, unsigned char *reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;
	
	if(reg_offset & 0x80) /* DS5662_CPLD2 */
	{
		twsi_index = 1;
		chip = DS5662_CPLD2_ADDR;
		addr = reg_offset & (~0x80);
	}
	else if(reg_offset & 0x40) /* DS5662_CPLD1 */ 
	{
		twsi_index = 0;
		chip = DS5662_CPLD1_ADDR;
		addr = reg_offset & (~0x40);
	}
	else /* DS5662_CPLD0 */
	{
		twsi_index = 0;
		chip = DS5662_CPLD0_ADDR;
		addr = reg_offset & (~0x00);
	}
	
	ret = bm_i2c_read(twsi_index, chip, addr, 1, reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "read cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		DBG(debug_ioctl, "read %#016llx got %#0x\n", addr, *reg_data);
	}
	
	return ret;
}

int ds5662_do_cpld_reg_write(int reg_addr, unsigned char reg_data)
{
	unsigned char reg_offset;
	unsigned char twsi_index, chip, addr;
	int ret;

	reg_offset = (unsigned char)reg_addr;
	
	if(reg_offset & 0x80) /* DS5662_CPLD2 */
	{
		twsi_index = 1;
		chip = DS5662_CPLD2_ADDR;
		addr = reg_offset & (~0x80);
	}
	else if(reg_offset & 0x40) /* DS5662_CPLD1 */ 
	{
		twsi_index = 0;
		chip = DS5662_CPLD1_ADDR;
		addr = reg_offset & (~0x40);
	}
	else /* DS5662_CPLD0 */
	{
		twsi_index = 0;
		chip = DS5662_CPLD0_ADDR;
		addr = reg_offset & (~0x00);
	}
	
	ret = bm_i2c_write(twsi_index, chip, addr, 1, &reg_data, 1);
	if(ret)
	{
		DBG(debug_ioctl, "write cpld on slot %d got base address error!\n", 0);
	}
	else
	{
		DBG(debug_ioctl, "write %#016llx got %#0x\n", addr, reg_data);
	}
	
	return ret;
}

int ds5662_board_ioctl_cpld_read(struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_reg_args cpld_reg_data;
	int op_ret;
	int ret;
	
	memset((void *)&cpld_reg_data, 0, sizeof(cpld_reg_args));
	
	op_ret = copy_from_user(&cpld_reg_data, (cpld_reg_args *)arg, sizeof(cpld_reg_args));
	ret = ds5662_do_cpld_reg_read(cpld_reg_data.reg, &cpld_reg_data.value);
	op_ret = copy_to_user(((cpld_reg_args *)arg), &cpld_reg_data, sizeof(cpld_reg_args));

	return ret;
}

int ds5662_board_ioctl_cpld_write(struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_reg_args cpld_reg_data;
	int op_ret;
	int ret;

	memset((void *)&cpld_reg_data, 0, sizeof(cpld_reg_args));
	
	op_ret = copy_from_user(&cpld_reg_data,(cpld_reg_args *)arg,sizeof(cpld_reg_args));
	ret = ds5662_do_cpld_reg_write(cpld_reg_data.reg, cpld_reg_data.value);
	op_ret = copy_to_user((cpld_reg_args *)arg,&cpld_reg_data,sizeof(cpld_reg_args));

	return ret;
}

int ds5662_board_ioctl_sfp_num(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int sfp_num = 60;
	int ret;
	
	ret = __put_user(sfp_num, (int __user *)arg);
	
	return ret;
}

int ds5662_portindex_to_sfpindex(int port_index)
{
	return port_index + 0;
}


int ds5662_sfpindex_to_portindex(int sfp_index)
{
	return sfp_index - 0;
}

int ds5662_do_sfp_cpld_op(unsigned int cmd, sfp_op_args *ptr_sfp_data)
{
	cpld_reg_ctl* ptr_cpld_ctl;
	unsigned char cpldx_offset;
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

	if((ptr_sfp_data->index < 0) || (ptr_sfp_data->index > 59))
	{
		DBG(debug_ioctl, "sfp_op: can't find the index.\n");
		return -1;
	}

	if(ptr_sfp_data->index < 24) /* TWSI0 CPLD1 SFP+ */
	{
		cpldx_offset = ptr_cpld_ctl->offset | 0x00;
	}
	else if(ptr_sfp_data->index < 48) /* TWSI0 CPLD2 SFP+ */
	{
		cpldx_offset = ptr_cpld_ctl->offset | 0x40;
	}
	else /* TWSI1 CPLD3 QSFP */
	{
		cpldx_offset = 0x02 | 0x80;
	}
	
	// because index is from 0-4
	// calculate the reg offset
	sfp_cpld_reg = cpldx_offset + ptr_sfp_data->index/8;
	//sfp_cpld_reg = ptr_cpld_ctl->offset + ptr_sfp_data->index/8;
	DBG(debug_ioctl, "sfp_cpld_reg is %x, index is %d.\n", sfp_cpld_reg, ptr_sfp_data->index);

	//calculate the shift and mask.
	//index mod 8 is 3, the mask is (000000001)b << 3 -> (00001000), and shift is the 3
	sfp_cpld_shift = ptr_sfp_data->index%8;
	sfp_cpld_mask = 0x01 << sfp_cpld_shift;
	DBG(debug_ioctl, "sfp_cpld_shift is %x, sfp_cpld_mask is %x.\n", sfp_cpld_shift, sfp_cpld_mask);

	ret = ds5662_do_cpld_reg_read(sfp_cpld_reg, &value);
	if (ret != 0)
		return ret;
	DBG(debug_ioctl, "sfp_cpld_reg is %x, value is %x.\n", sfp_cpld_reg, value);
		
	
	if(ptr_sfp_data->index < 48) /* TWSI0 SFP+ */
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
		ret = ds5662_do_cpld_reg_write(sfp_cpld_reg, value);
	}
	
	return ret;

}

int ds5662_board_ioctl_cpld_sfp(struct file *filp, unsigned int cmd, unsigned long arg)
{
	sfp_op_args sfp_data;
	int op_ret;
	int ret;
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));
	
	op_ret = copy_from_user(&sfp_data, (sfp_op_args __user *)arg, sizeof(sfp_op_args));

	sfp_data.index = ds5662_portindex_to_sfpindex(sfp_data.index);
	ret = ds5662_do_sfp_cpld_op(cmd, &sfp_data);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ret is %d. index is %d, rwflag is %d, value is %d\n", 
				ret, sfp_data.index, sfp_data.rwflag, sfp_data.value);
	sfp_data.index = ds5662_sfpindex_to_portindex(sfp_data.index);
	
	op_ret = copy_to_user((sfp_op_args __user *)arg, &sfp_data,sizeof(sfp_op_args));
	
	return ret;
}

int ds5662_board_ioctl_i2c_sfp_operate(struct file *filp, unsigned int cmd, unsigned long arg)
{
	sfp_op_args sfp_data;
	char* ksfp_buf;
	int op_ret = 0;
	cpld_reg_ctl *ptr_cpld_ctl = NULL;
	int sfp_sel_reg;
	int sfp_sel_shift = 0;
	unsigned char sfp_sel_mask = 0x00; /* because we operate the 8 bit data */
	unsigned short value = 0;
	int sfp_dev_addr = 0x50;
	int ret = 0;

	int sfp_cpld_reg;
	int sfp_cpld_shift;
	unsigned char sfp_cpld_mask;
	
	memset(&sfp_data, 0, sizeof(sfp_op_args));

	op_ret = copy_from_user(&sfp_data, (sfp_op_args __user *)arg, sizeof(sfp_op_args));
	
	if (sfp_data.buf_len <= 0)
	{
		DBG(debug_ioctl, KERN_ERR DRIVER_NAME ":%s(%d):buf length must greater than zero.\n", __FILE__, __LINE__);
		return -1;		
	}
	ksfp_buf = (char *)kmalloc(sfp_data.buf_len, GFP_KERNEL);
	if (!ksfp_buf)
	{
		DBG(debug_ioctl, KERN_ERR DRIVER_NAME ":%s(%d):ksfp_data kmalloc memory fail.\n", __FILE__, __LINE__);
		return -ENOMEM;
	}
	memset(ksfp_buf, 0, sizeof(sfp_data.buf_len));
	
	op_ret = copy_from_user(ksfp_buf, (char __user *)sfp_data.buf, sfp_data.buf_len);
#if 0
	if(sfp_data.index < 48) /* TWSI0 SFP+ */
	{
		
	}
	else /* TWSI1 QSFP */
	{
		ptr_cpld_ctl = util_get_cmd_reg_ctrl(cmd);
		if (ptr_cpld_ctl == NULL)
		{
			DBG(debug_ioctl, KERN_ERR DRIVER_NAME ":can't find the cmd.\n");
			kfree(ksfp_buf);
			return -1;
		}

		sfp_cpld_reg = ptr_cpld_ctl.offset + (sfp_data.index-48) / 8;
		sfp_cpld_shift = (sfp_data->index-48) % 8;
		sfp_cpld_mask = 0x01 << sfp_cpld_shift;

		sfp_data.index = ds5662_portindex_to_sfpindex(sfp_data.index);
		ret = ds5662_do_sfp_cpld_op(cmd, &sfp_data);
		DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ret is %d. index is %d, rwflag is %d, value is %d\n", 
				ret, sfp_data.index, sfp_data.rwflag, sfp_data.value);
		sfp_data.index = ds5662_sfpindex_to_portindex(sfp_data.index);

		/* select the sfp module */
		if((sfp_data->index - 48) < 8) /* 0x00XX */
		{
			value = 0x0000 | sfp_sel_mask;
			//value = ~value;
		}
		else /* 0xXX00 */
		{
			value = 0x0000 | (sfp_sel_mask << 8);
			//value = ~value;
		}
		ret = do_cpld_reg_write(ptr_cpld_ctl->offset, value);
		ret += do_cpld_reg_write((ptr_cpld_ctl->offset+1), (value>>8));
		if (ret != 0)
		{
			ret = -1;
			goto err;
		}

		if (!sfp_data.rwflag) /* read opt */
		{

#if 0
			ret  = _ax_i2c_read8( sfp_dev_addr, sfp_data.reg_addr, 1, 
				ksfp_buf, sfp_data.buf_len);
			if (ret != 0)
			{
				DBG(debug_ioctl, "cgm9048s_i2c_sfp_read is wrong.\n");
				//return -1;
				ret = -ENOTTY;
				goto err;
			}
			op_ret = copy_to_user((char *)sfp_data.buf, ksfp_buf, sfp_data.buf_len);
#endif
		}
		else /* write opt */
		{
#if 0
			ret  = _ax_i2c_write8( sfp_dev_addr, sfp_data.reg_addr, 1, 
				ksfp_buf, sfp_data.buf_len);	
			if (ret != 0)
			{
				DBG(debug_ioctl, "cgm9048s_i2c_sfp_write is wrong.\n");
				//return -1;
				ret = -ENOTTY;
				goto err;
			}
#endif
		}
		
		/* restore the sfp */
		ret = do_cpld_reg_write(ptr_cpld_ctl->offset, 0x00);
		ret += do_cpld_reg_write((ptr_cpld_ctl->offset+1), 0x00);
		if (ret != 0)
		{
			ret = -1;
			goto err;
		}
	}
#endif	
err:
	if (ksfp_buf)
	{
		kfree(ksfp_buf);
		ksfp_buf = NULL;
	}
	
	return ret;
}

int ds5662_board_ioctl_led_light(struct file *filp, unsigned int cmd, unsigned long arg)
{
	

	return 0;
}

int ds5662_board_ioctl_i2c_write(struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	return 0;
}

cpld_reg_ctl ds5662_cpld_ctrl[] = 
{
	/* CPLD1/2*/
	{BM_IOC_SFP_PRESENSE,  		0x00, 1, 0xFF, 0, NULL},
	{BM_IOC_SFP_DMI_TX_FAULT,   0x03, 1, 0xFF, 0, NULL},
	{BM_IOC_SFP_DMI_LOS,      	0x06, 1, 0xFF, 0, NULL},
    {BM_IOC_SFP_LIGHT,			0x09, 1, 0xFF, 0, NULL},
    
	/* CPLD3 */
	{BM_IOC_SFP_OPERATE,		0x18, 1, 0x0F, 0, NULL},
};

ioctl_proc ds5662_board_ioctl_proc_arr[] = 
{
	{BM_IOC_CPLD_REG_READ, 			ds5662_board_ioctl_cpld_read},
	{BM_IOC_CPLD_REG_WRITE, 		ds5662_board_ioctl_cpld_write},
		
	{BM_IOC_SFP_NUM,				ds5662_board_ioctl_sfp_num},	
	{BM_IOC_SFP_LIGHT,				ds5662_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_PRESENSE,			ds5662_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_LOS,			ds5662_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_DMI_TX_FAULT,		ds5662_board_ioctl_cpld_sfp},
	{BM_IOC_SFP_OPERATE,			ds5662_board_ioctl_i2c_sfp_operate},

	{BM_IOC_WIRELESS_LED_LIGTH,		ds5662_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 	ds5662_board_ioctl_i2c_write},
};

int ds5662_board_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds5662_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds5600_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	ko_board->cpld_reg_ctrl_count = LENGTH(ds5652_cpld_ctrl);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":cpld reg count is %d\n", ko_board->cpld_reg_ctrl_count);
	
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
	.cpld_reg_ctrl_arr = ds5662_cpld_ctrl,
};


