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
* bmk_ds6224_param.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		bm -- T9006 product management (i2c, gpio, bootbus, cpld, ....) setting.
*
* UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"
#include <asm/uaccess.h>


extern int ds5600_ioctl_proc_cpld_fan(struct file *filp, unsigned int cmd, unsigned long arg);
extern int ds5600_ioctl_proc_power_present(struct file *filp, unsigned int cmd, unsigned long arg);
extern int ds5600_ioctl_proc_temp_info(struct file *filp, unsigned int cmd, unsigned long arg);
extern int ds5600_ioctl_proc_power_info(struct file *filp, unsigned int cmd, unsigned long arg);
extern int ds5600_ioctl_proc_power_state(struct file *filp, unsigned int cmd, unsigned long arg);

int ds6224_reset_board(int slot_index)
{
	int ret = 0;
	
	return ret;
}


/* 暂时隔开。 这个和产品相关性挺大 */
int ds6224_get_board_online_state(int slot_index)
{
	return BOARD_INSERT ;
}


int ds6224_ioctl_proc_master_slot_id(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_op_args cpld_op_data;
	int op_ret= 0;

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds6224_ioctl_proc_master_slot_id\n");
	memset(&cpld_op_data, 0, sizeof(cpld_op_args));

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = kboard_info->slot_index;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));
	
	return op_ret;
}


int ds6224_get_slot_index(void)
{
    int ret = 0;
	char buf[2] = {0, 0};
	struct file *fp;
    mm_segment_t fs;
	loff_t pos = 0;
	fp = filp_open("/mnt/stack_unit", O_RDONLY, 0);
	if(IS_ERR(fp))
	{
	    printk("Standlone pizza box.Slot index is 1.\r\n");
	}
	else
	{
	    fs = get_fs();
        set_fs(KERNEL_DS);
        pos = 0;
		ret = vfs_read(fp, buf, 1, &pos);
		filp_close(fp, NULL);
        set_fs(fs);
		if(ret == 0)
		{
		    return 0;
		}
		if(buf[0] == '1')
		{
		    return 	0;
		}
		else if(buf[0] == '2')
		{
		    return 	1;
		}
	}
	return 0;	
 }

int ds6224_ioctl_proc_cpld_slot_id(/*struct inode *inode,*/ struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;

	memset(&cpld_op_data, 0, sizeof(cpld_op_args));
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = ds6224_get_slot_index();
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));
	return 0;
}

proc_file ds6224_common_files[] = 
{
	{"module_sn",				&bm_proc_module_sn},
	{"module_name", 			&bm_proc_module_name},
	{"product_sn",	 			&bm_proc_product_sn},
	{"bm_proc_product_sn", 		&bm_proc_product_sn},
	{"product_base_mac_addr", 	&bm_proc_product_base_mac_addr},
	{"product_name", 			&bm_proc_product_name},
	{"software_name", 			&bm_proc_software_name},
	{"enterprise_name", 		&bm_proc_enterprise_name},
	{"enterprise_snmp_oid", 	&bm_proc_enterprise_snmp_oid},
	{"snmp_sys_oid", 			&bm_proc_snmp_sys_oid},
	{"admin_username", 			&bm_proc_admin_username},
	{"admin_passwd", 			&bm_proc_admin_passwd},
	{"slot_id", 				&bm_proc_slot_id}, /* self board slot id */		
	{"board_name", 				&bm_proc_board_name}, /* self board name */
	{"product_type",			&bm_proc_product_type},
	{"board_type",				&bm_proc_board_type},
	{"slot_num", 				&bm_proc_slot_num},
};

proc_file_struct ds6224_board_spec_files_arr[] = 
{
};

ioctl_proc ds6224_ioctl_proc_arr[] = 
{
	{BM_IOC_CPLD_MASTER_SLOT_ID, 	ds6224_ioctl_proc_master_slot_id},
 	{BM_IOC_CPLD_SLOT_ID,			ds6224_ioctl_proc_cpld_slot_id},
	{BM_IOC_CPLD_FAN_PRESENT, 		ds5600_ioctl_proc_cpld_fan},
	{BM_IOC_CPLD_FAN_ALARM, 		ds5600_ioctl_proc_cpld_fan},
	{BM_IOC_CPLD_FAN_SPEED,			ds5600_ioctl_proc_cpld_fan},
	{BM_IOC_CPLD_POWER_PRESENT, 	ds5600_ioctl_proc_power_present},
	{BM_IOC_POWER_STATE, 			ds5600_ioctl_proc_power_state},	
	{BM_IOC_POWER_INFO,				ds5600_ioctl_proc_power_info},
	{BM_IOC_TEMP_INFO, 				ds5600_ioctl_proc_temp_info},
};


int ds6224_init(void)
{
	
	ko_product->proc_common_count = LENGTH(ds6224_common_files);
	DBG(debug_ioctl,  "proc_common_count is %d.\n", ko_product->proc_common_count);

	ko_product->proc_board_spec_files = NULL;
	ko_product->proc_board_spec_count = 0;
	DBG(debug_ioctl,  "proc_board_spec_count is %d.\n", ko_product->proc_board_spec_count);

	ko_product->ioctl_proc_count = LENGTH(ds6224_ioctl_proc_arr);
	DBG(debug_ioctl,  "ioctl_proc_count is %d.\n", ko_product->ioctl_proc_count);


	return 0;
}

void ds6224_cleanup(void)
{
	/* do nothing */
}	

int ds6224_proc_init(void)
{
	struct proc_dir_entry * bm_proc_entry = NULL;
	int i ;

	ko_product->proc_dir_entry = proc_mkdir(ko_product->proc_dir_name, NULL);
	if (!ko_product->proc_dir_entry) {
		DBG(debug_ioctl, "bm: proc_mkdir failed.\n");
		return -1;
	}

	DBG(debug_ioctl, "bm: show proc common file.\n");

	for (i = 0; i < ko_product->proc_common_count; i++)
	{
		proc_file proc_file_element = ko_product->proc_common_files[i];

		DBG(debug_ioctl, "bm: proc entry index %d name is %s.\n", i, proc_file_element.name);
		bm_proc_entry = create_proc_entry(proc_file_element.name, 
			0, ko_product->proc_dir_entry);
		if (bm_proc_entry) {
			bm_proc_entry->proc_fops = proc_file_element.fops;
		} else {
			DBG(debug_ioctl, "bm: proc entry %s initialize failed.\n", proc_file_element.name);
			return -1;
		}
	}

	DBG(debug_ioctl, "bm: show proc board spec files.\n");
	/* */
	
	for (i = 0; i < ko_product->proc_board_spec_count; i++)
	{
		proc_file proc_file_element = ko_product->proc_board_spec_files[i];

		DBG(debug_ioctl, "bm: proc entry index %d name is %s.\n", i, proc_file_element.name);
		bm_proc_entry = create_proc_entry(proc_file_element.name, 
			0, ko_product->proc_dir_entry);
		if (bm_proc_entry) {
			bm_proc_entry->proc_fops = proc_file_element.fops;
		} else {
			DBG(debug_ioctl, "bm: proc entry %s initialize failed.\n", proc_file_element.name);
			return -1;
		}
	}	

	DBG(debug_ioctl, "bm: proc all files success.\n");
	return 0;
	
}

int ds6224_proc_cleanup(void)
{
	int i;

	DBG(debug_ioctl, "bm: ds6224_proc_cleanup.\n");
	
	for (i = 0; i < ko_product->proc_board_spec_count; i++)
	{
		remove_proc_entry(ko_product->proc_board_spec_files[i].name, ko_product->proc_dir_entry);
	}
	
	for (i = 0; i < ko_product->proc_common_count; i++)
	{
		remove_proc_entry(ko_product->proc_common_files[i].name, ko_product->proc_dir_entry);
	}
	
	/* Remove dir entry */
	remove_proc_entry(ko_product->proc_dir_name, NULL);
	ko_product->proc_dir_entry = NULL;

	DBG(debug_ioctl, "bm: remove proc entry success.\n");
	
	return 0;
}	


ds_fix_param_t ds6224_ds_fix_param_array[] = 
{
	{.ds_index = 0,  .info_dev_addr = 0x54},
	{.ds_index = 1,  .info_dev_addr = 0x55},			
};

ds_param_t ds6224_ds_param = 
{
	.power_num = 2,
	.ds_fix_array = ds6224_ds_fix_param_array,
	
};

kfan_param_t ds6224_fan_param = 
{
	.num = 3,
};


kproduct_fix_param ds6224_product_fix_param = 
{
	.product_type = PRODUCT_DS6224,
	.product_code = PPAL_PRODUCT_HWCODE_DS6224,
	
	.product_short_name = "DS6224",
	.product_name = "",
	
	.slotnum = 1,
	.master_slotnum = 1,
	.master_slot_id = {0},

	.ioctl		= bm_product_ioctl,
	.ioctl_proc_arr = ds6224_ioctl_proc_arr,

	
	.proc_dir_name = "sysinfo",
	.proc_common_files = ds6224_common_files,
	
	.ds_param = &ds6224_ds_param,
	.fan_param = &ds6224_fan_param,

	.product_param_init		= ds6224_init,
	.product_param_cleanup	= ds6224_cleanup,
	.proc_files_init		= ds6224_proc_init,
	.proc_files_cleanup		= ds6224_proc_cleanup,
	.get_board_online_state = ds6224_get_board_online_state,
	.reset_board	= ds6224_reset_board,
	.slot_index_get = ds6224_get_slot_index,	
};


