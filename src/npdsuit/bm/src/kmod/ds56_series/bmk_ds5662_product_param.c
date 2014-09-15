
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"
#include <asm/uaccess.h>

int ds5662_reset_board(int slot_index)
{
	int ret = 0;
	
	return ret;
}


/* 暂时隔开。 这个和产品相关性挺大 */
int ds5662_get_board_online_state(int slot_index)
{
	return BOARD_INSERT ;
}


int ds5662_ioctl_proc_master_slot_id(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_op_args cpld_op_data;
	int op_ret= 0;

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds5662_ioctl_proc_master_slot_id\n");
	memset(&cpld_op_data, 0, sizeof(cpld_op_args));

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = kboard_info->slot_index;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));
	
	return op_ret;
}


int ds5662_get_slot_index(void)
{
	return 1;	
 }

int ds5662_ioctl_proc_cpld_slot_id(/*struct inode *inode,*/ struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;

	memset(&cpld_op_data, 0, sizeof(cpld_op_args));
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = ds5662_get_slot_index();
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));
	return 0;
}

proc_file ds5662_common_files[] = 
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

proc_file_struct ds5662_board_spec_files_arr[] = 
{
};

ioctl_proc ds5662_ioctltl_proc_arr[] = 
{
	{BM_IOC_CPLD_MASTER_SLOT_ID, 	ds5662_ioctl_proc_master_slot_id},
 	{BM_IOC_CPLD_SLOT_ID,			ds5662_ioctl_proc_cpld_slot_id},
};


int ds5662_init(void)
{
	
	ko_product->proc_common_count = LENGTH(ds5662_common_files);
	DBG(debug_ioctl,  "proc_common_count is %d.\n", ko_product->proc_common_count);

	ko_product->proc_board_spec_files = NULL;
	ko_product->proc_board_spec_count = 0;
	DBG(debug_ioctl,  "proc_board_spec_count is %d.\n", ko_product->proc_board_spec_count);

	ko_product->ioctl_proc_count = LENGTH(ds5662_ioctltl_proc_arr);
	DBG(debug_ioctl,  "ioctl_proc_count is %d.\n", ko_product->ioctl_proc_count);


	return 0;
}

void ds5662_cleanup(void)
{
	/* do nothing */
}	

int ds5662_proc_init(void)
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

int ds5662_proc_cleanup(void)
{
	int i;

	DBG(debug_ioctl, "bm: ds5662_proc_cleanup.\n");
	
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

ds_fix_param_t ds5662_ds_fix_param_array[] = 
{
	{.ds_index = 0,  .info_dev_addr = 0x54},
	{.ds_index = 1,  .info_dev_addr = 0x55},			
};

ds_param_t ds5662_ds_param = 
{
	.power_num = 2,
	.ds_fix_array = ds5662_ds_fix_param_array,
	
};

kfan_param_t ds5662_fan_param = 
{
	.num = 3,
};


kproduct_fix_param ds5662_product_fix_param = 
{
	.product_type = PRODUCT_DS5662,
	.product_code = PPAL_PRODUCT_HWCODE_DS5662,
	
	.product_short_name = "DS5660",
	.product_name = "",
	
	.slotnum = 1,
	.master_slotnum = 1,
	.master_slot_id = {0},

	.ioctl		= bm_product_ioctl,
	.ioctl_proc_arr = ds5662_ioctltl_proc_arr,

	
	.proc_dir_name = "sysinfo",
	.proc_common_files = ds5662_common_files,
	
	.ds_param = &ds5662_ds_param,
	.fan_param = &ds5662_fan_param,

	.product_param_init		= ds5662_init,
	.product_param_cleanup	= ds5662_cleanup,
	.proc_files_init		= ds5662_proc_init,
	.proc_files_cleanup		= ds5662_proc_cleanup,
	.get_board_online_state = ds5662_get_board_online_state,
	.reset_board	= ds5662_reset_board,
	.slot_index_get = ds5662_get_slot_index,	
};


