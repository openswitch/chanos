
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"


/*
 * bmk_dummy_product.c
 */

/* *************************************
*
*
***************************************/

/*  */
int dummy_product_reset_board(int slot_index)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_product_slot_index_get(void)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 1;	
}


/* 暂时隔开。 这个和产品相关性挺大 */
int dummy_product_get_board_online_state(int slot_index)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return BOARD_REMOVE;

}





/**************************************************************************
 *
 *	proc file
 *	
 **************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_module_sn_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "1000");
	return 0;
}

static int bm_proc_dummy_module_sn_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_module_sn_show, NULL);
}

struct file_operations bm_proc_dummy_module_sn = {
    .open    = bm_proc_dummy_module_sn_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_module_name_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy_module");
	return 0;
}

static int bm_proc_dummy_module_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_module_name_show, NULL);
}

struct file_operations bm_proc_dummy_module_name = {
    .open    = bm_proc_dummy_module_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////

static int bm_proc_dummy_product_sn_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "1000");
	return 0;
}

static int bm_proc_dummy_product_sn_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_product_sn_show, NULL);
}

struct file_operations bm_proc_dummy_product_sn = {
    .open    = bm_proc_dummy_product_sn_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_product_base_mac_addr_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "000A7AFE0106");
	return 0;
}

static int bm_proc_dummy_product_base_mac_addr_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_product_base_mac_addr_show, NULL);
}

struct file_operations bm_proc_dummy_product_base_mac_addr = {
    .open    = bm_proc_dummy_product_base_mac_addr_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_product_name_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy_product");
	return 0;
}

static int bm_proc_dummy_product_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_product_name_show, NULL);
}

struct file_operations bm_proc_dummy_product_name = {
    .open    = bm_proc_dummy_product_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_software_name_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "Auteware");
	return 0;
}

static int bm_proc_dummy_software_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_software_name_show, NULL);
}

struct file_operations bm_proc_dummy_software_name = {
    .open    = bm_proc_dummy_software_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_enterprise_name_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "AUTELAN");

	return 0;
}

static int bm_proc_dummy_enterprise_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_enterprise_name_show, NULL);
}

struct file_operations bm_proc_dummy_enterprise_name = {
    .open    = bm_proc_dummy_enterprise_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_enterprise_snmp_oid_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy");

	return 0;
}

static int bm_proc_dummy_enterprise_snmp_oid_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_enterprise_snmp_oid_show, NULL);
}

struct file_operations bm_proc_dummy_enterprise_snmp_oid = {
    .open    = bm_proc_dummy_enterprise_snmp_oid_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_snmp_sys_oid_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy");
	return 0;
}

static int bm_proc_dummy_snmp_sys_oid_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_snmp_sys_oid_show, NULL);
}

struct file_operations bm_proc_dummy_snmp_sys_oid = {
    .open    = bm_proc_dummy_snmp_sys_oid_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_admin_username_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy_user");
	return 0;
}

static int bm_proc_dummy_admin_username_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_admin_username_show, NULL);
}

struct file_operations bm_proc_dummy_admin_username = {
    .open    = bm_proc_dummy_admin_username_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_admin_passwd_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy_user");
	return 0;
}

static int bm_proc_dummy_admin_passwd_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_admin_passwd_show, NULL);
}

struct file_operations bm_proc_dummy_admin_passwd = {
    .open    = bm_proc_dummy_admin_passwd_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////

static int bm_proc_dummy_slot_id_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%d\n", 1);
	return 0;
}

static int bm_proc_dummy_slot_id_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_slot_id_show, NULL);
}

struct file_operations bm_proc_dummy_slot_id = {
    .open    = bm_proc_dummy_slot_id_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};




//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_board_name_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%s\n", "dummy_board");
	return 0;
}

static int bm_proc_dummy_board_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_board_name_show, NULL);
}

struct file_operations bm_proc_dummy_board_name = {
    .open    = bm_proc_dummy_board_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_product_type_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%d\n", PRODUCT_AS2K_3200);
	return 0;
}

static int bm_proc_dummy_product_type_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_product_type_show, NULL);
}

struct file_operations bm_proc_dummy_product_type = {
    .open    = bm_proc_dummy_product_type_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_board_type_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%d\n", PPAL_BOARD_TYPE_DUMMY);
	return 0;
}

static int bm_proc_dummy_board_type_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_board_type_show, NULL);
}

struct file_operations bm_proc_dummy_board_type = {
    .open    = bm_proc_dummy_board_type_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};


//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_dummy_master_slot_id_show(struct seq_file *file, void *ptr)
{
	seq_printf(file, "%d\n", 1);
	return 0;
}

static int bm_proc_dummy_master_slot_id_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_dummy_master_slot_id_show, NULL);
}

struct file_operations bm_proc_dummy_master_slot_id = {
    .open    = bm_proc_dummy_master_slot_id_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};


proc_file dummy_product_common_files[] = 
{
	{"module_sn",				&bm_proc_dummy_module_sn},
	{"module_name", 			&bm_proc_dummy_module_name},
	{"product_sn",	 			&bm_proc_dummy_product_sn},
	{"bm_proc_dummy_product_sn", 		&bm_proc_dummy_product_sn},
	{"product_base_mac_addr", 	&bm_proc_dummy_product_base_mac_addr},
	{"product_name", 			&bm_proc_dummy_product_name},
	{"software_name", 			&bm_proc_dummy_software_name},
	{"enterprise_name", 		&bm_proc_dummy_enterprise_name},
	{"enterprise_snmp_oid", 	&bm_proc_dummy_enterprise_snmp_oid},
	{"snmp_sys_oid", 			&bm_proc_dummy_snmp_sys_oid},
	{"admin_username", 			&bm_proc_dummy_admin_username},
	{"admin_passwd", 			&bm_proc_dummy_admin_passwd},
	{"slot_id", 				&bm_proc_dummy_slot_id}, /* self board slot id */		
	{"board_name", 				&bm_proc_dummy_board_name}, /* self board name */
	{"product_type",			&bm_proc_dummy_product_type},
	{"board_type",				&bm_proc_dummy_board_type},

};



proc_file_struct dummy_product_board_spec_files_arr[] = 
{
};

void dummy_replace_iotclfunc(unsigned int cmd, IOCTL_FUC  replace_func)
{
	int index = 0;
	
	if (NULL == replace_func)
		return;
		
	for (index = 0; index < ko_board->ioctl_proc_count; index++)
	{
		ioctl_proc* ptr_proc_element =  &ko_board->ioctl_proc_arr[index];

		//DBG("current index %d cmd is %x \n",index,  proc_element.cmd);

		if ((cmd == ptr_proc_element->cmd) && (NULL != ptr_proc_element->func))
		{
			ptr_proc_element->func = replace_func;
			DBG(debug_ioctl, "replace func success.\n");
			return ;
		}
	}
}

int dummy_ioctl_proc_cpld_slot_id(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = 1;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return op_ret;
	
}

int dummy_ioctl_proc_product_hwcode(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = PPAL_PRODUCT_HWCODE_AS2K_3200;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return op_ret;
	
}

int dummy_product_init(void)
{
	ko_product->proc_common_count = LENGTH(dummy_product_common_files);
	DBG(debug_ioctl,  "proc_common_count is %d.\n", ko_product->proc_common_count);
	
	ko_product->proc_board_spec_files = NULL;
	ko_product->proc_board_spec_count = 0;
	DBG(debug_ioctl,  "proc_board_spec_count is %d.\n", ko_product->proc_board_spec_count);

	return 0;
}

void dummy_product_cleanup(void)
{
	/* do nothing */
}	

int dummy_product_proc_init(void)
{
	struct proc_dir_entry * bm_proc_dummy_entry = NULL;
	int i ;

	ko_product->proc_dir_entry = proc_mkdir(ko_product->proc_dir_name, NULL);
	if (!ko_product->proc_dir_entry) {
		DBG(debug_ioctl, "bm: proc_mkdir failed.\n");
		return -1;
	}

	for (i = 0; i < ko_product->proc_common_count; i++)
	{
		proc_file proc_file_element = ko_product->proc_common_files[i];

		DBG(debug_ioctl, "bm: proc entry index %d name is %s.\n", i, proc_file_element.name);
		bm_proc_dummy_entry = create_proc_entry(proc_file_element.name, 
			0, ko_product->proc_dir_entry);
		if (bm_proc_dummy_entry) {
			bm_proc_dummy_entry->proc_fops = proc_file_element.fops;
		} else {
			DBG(debug_ioctl, "bm: proc entry %s initialize failed.\n", proc_file_element.name);
			return -1;
		}
	}
	
	for (i = 0; i < ko_product->proc_board_spec_count; i++)
	{
		proc_file proc_file_element = ko_product->proc_board_spec_files[i];

		DBG(debug_ioctl, "bm: proc entry index %d name is %s.\n", i, proc_file_element.name);
		bm_proc_dummy_entry = create_proc_entry(proc_file_element.name, 
			0, ko_product->proc_dir_entry);
		if (bm_proc_dummy_entry) {
			bm_proc_dummy_entry->proc_fops = proc_file_element.fops;
		} else {
			DBG(debug_ioctl, "bm: proc entry %s initialize failed.\n", proc_file_element.name);
			return -1;
		}
	}	

	DBG(debug_ioctl, "bm: proc all files success.\n");
	return 0;
	
}

int dummy_product_proc_cleanup(void)
{
	int i;
	
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

	return 0;
}	


kproduct_fix_param dummy_product_fix_param = 
{
	.product_type = PRODUCT_DUMMY,
	.product_code = PPAL_PRODUCT_HWCODE_DUMMY,
	
	.product_short_name = "PRODUCT_DUMMY",
	.product_name = "",
	
	.slotnum = 1,
	.master_slotnum = 1,
	.master_slot_id = {0},
	
	.proc_dir_name = "sysinfo",
	.proc_common_files = dummy_product_common_files,

	.product_param_init		= dummy_product_init,
	.product_param_cleanup	= dummy_product_cleanup,
	.proc_files_init		= dummy_product_proc_init,
	.proc_files_cleanup		= dummy_product_proc_cleanup,
	.get_board_online_state = dummy_product_get_board_online_state,
	.reset_board	= dummy_product_reset_board,
	.slot_index_get = dummy_product_slot_index_get,
};

