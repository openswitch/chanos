
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_product_init.h"
#include "bmk_operation_boot_env.h"
#include "bmk_read_eeprom.h"
#include "board/ts_product_feature.h"

#include "bmk_ds5600_series_info.h"


unsigned int debug_ioctl = 0;    
unsigned int debug_octeon =  0 ; 


/** 
  * data from chassis device.
  */
struct ax_sysinfo_product_t       bm_ax_sysinfo_on_backplane;
struct ax_sysinfo_single_board_t  bm_ax_sysinfo_on_module0;
extern mac_addr_stored_t stored_mac;
/**
  * data from box device.
  */
extern long bm_get_product_code(void);

ioctl_proc ds5600_ioctl_proc_arr[] = 
{
	{BM_IOC_READ_PRODUCT_SYSINFO, 	ioctl_proc_read_product_sysinfo},
	{BM_IOC_READ_MODULE_SYSINFO, 	ioctl_proc_read_module_sysinfo},
	{BM_IOC_ENV_EXCH, 				ioctl_proc_env_exch},
	{BM_IOC_GET_MAC, 				ioctl_proc_get_mac},
	{BM_IOC_BOOTROM_EXCH, 			ioctl_proc_bootrom_exch},
	{BM_IOC_GET_PRODUCET_TYPE, 		ioctl_proc_get_producet_type},	

	{BM_IOC_CPLD_PRODUCT_FAMILY,	ioctl_proc_product_family},
	{BM_IOC_CPLD_PRODUCT_TYPE,		ioctl_proc_product_type},
	{BM_IOC_CPLD_PRODUCT_HWCODE,    ioctl_proc_product_hwcode},
    {BM_IOC_CPLD_MODULE_HWCODE,     ioctl_proc_module_hwcode},

	{BM_IOC_CPLD_MODULE_TYPE,		ioctl_proc_module_type},
		
	{BM_IOC_CPLD_BOARD_NUM, 		ioctl_proc_board_num},
	{BM_IOC_CPLD_BOARD_RESET, 		ioctl_proc_board_reset},
	{BM_IOC_CPLD_SELF_SYS_RESET, 	ioctl_proc_self_reset},

	{BM_IOC_CPLD_BOARD_ONLINE, 		ioctl_proc_board_online},
	
	{BM_IOC_KERNEL_DEBUG,			ioctl_proc_kernel_debug},
};

int bm_product_series_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int index = 0;

	//DBG(debug_ioctl,"Enter board bm_common_ioctl\n");
	//DBG(debug_ioctl,"cmd is %x .\n", cmd);

	for (index = 0; index < LENGTH(ds5600_ioctl_proc_arr); index++)
	{
		ioctl_proc proc_element =  ds5600_ioctl_proc_arr[index];
		
		if ((cmd == proc_element.cmd) && (NULL != proc_element.func))
		{
			return proc_element.func(/*inode,*/ filp, cmd, arg);
		}
	}
	//DBG(debug_ioctl,"can't find the cmd\n");
	return -2; 
}

int bm_product_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int index = 0;

	//DBG(debug_ioctl,"Enter board bm_product_ioctl\n");
	//DBG(debug_ioctl,"cmd is %x .\n", cmd);

	for (index = 0; index < ko_product->ioctl_proc_count; index++)
	{
		ioctl_proc proc_element =  ko_product->ioctl_proc_arr[index];
		
		if ((cmd == proc_element.cmd) && (NULL != proc_element.func))
		{
			return proc_element.func(/*inode, */filp, cmd, arg);
		}
	}
	//DBG(debug_ioctl,"can't find the cmd\n");
	return -2;
}

int bm_board_ioctl(/*struct inode *inode,*/ struct file *filp, unsigned int cmd, unsigned long arg)
{
	int index = 0;

	//DBG(debug_ioctl,"Enter board bm_board_ioctl\n");
	//DBG(debug_ioctl,"cmd is %x .\n", cmd);
	
	for (index = 0; index < ko_board->ioctl_proc_count; index++)
	{
		ioctl_proc proc_element =  ko_board->ioctl_proc_arr[index];
		
		if ((cmd == proc_element.cmd) && (NULL != proc_element.func))
		{
			return proc_element.func(/*inode, */filp, cmd, arg);
		}
	}

	DBG(debug_ioctl,"can't find the cmd\n");
	return -2;
}

/* read int reg value and minor reg value for just */
irqreturn_t bm_board_interrupt_handler(int irq, void *dev_id)
{	
	DBG(debug_ioctl, "Enter bm_board_interrupt_handler. \n");
	return IRQ_HANDLED;
}

/**************************************************
*
*	cpld handler
*	
**************************************************/
void cpld_product_family_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = kboard_info->family_type;
}

void cpld_product_type_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = kboard_info->product_type;	
}

void cpld_module_type_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = kboard_info->board_type;	
}


/***************************************************
*
*	ioctl_proc  
*
*****************************************************/


/****************************************************************/
//int ioctl_proc_product_family(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
int ioctl_proc_product_family(struct file *filp, unsigned int cmd, unsigned long arg)

{
    cpld_op_args cpld_op_data;
	int  op_ret = 0;

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	
	cpld_op_data.value = ko_family->family_type;

	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return 0;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_product_type(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_product_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    cpld_op_args cpld_op_data;
	int  op_ret = 0;

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	
	cpld_op_data.value = ko_product->product_type;

	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_module_hwcode(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_module_hwcode(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    cpld_op_args cpld_op_data;
	int  op_ret = 0;

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	
	cpld_op_data.value = ko_board->board_code;

	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_module_type(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_module_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    cpld_op_args cpld_op_data;
	int  op_ret = 0;

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));

	cpld_op_data.value = ko_board->board_type;

	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_pcb_version(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_pcb_version(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
    cpld_op_args cpld_op_data;
	int  op_ret = 0;

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	
	cpld_op_data.value = 0;

	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_read_product_sysinfo(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_read_product_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
	ax_product_sysinfo product_sysinfo;
	int retval = 0;
	int op_ret = 0;
	
	memset(&product_sysinfo, 0, sizeof(product_sysinfo));
	
	op_ret = copy_from_user(&product_sysinfo, (ax_product_sysinfo *)arg, sizeof(ax_product_sysinfo));
	retval = bm_ax_read_sysinfo_product(BM_AX_BACKPLANE_EEPROM_ADDR, &product_sysinfo);
	op_ret = copy_to_user((ax_product_sysinfo *)arg, &product_sysinfo, sizeof(ax_product_sysinfo));	

	return retval;
	
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_read_module_sysinfo(/*struct inode *inode,*/ struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_read_module_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
	ax_module_sysinfo module_sysinfo;
	int retval = 0;
	int op_ret = 0;

	memset(&module_sysinfo, 0, sizeof(ax_module_sysinfo));
	
	op_ret = copy_from_user(&module_sysinfo, (ax_module_sysinfo *)arg, sizeof(ax_module_sysinfo));
	retval = bm_ax_read_module_sysinfo(&module_sysinfo);
	op_ret = copy_to_user((ax_module_sysinfo *)arg, &module_sysinfo, sizeof(ax_module_sysinfo));

	return retval;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_env_exch(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_env_exch(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
	boot_env_t bm_op_boot_env_args;
	int retval = 0;
	int op_ret = 0;

	memset(&bm_op_boot_env_args, 0, sizeof(boot_env_t));	
	
	op_ret = copy_from_user(&bm_op_boot_env_args,(boot_env_t *)arg, sizeof(boot_env_t));
	retval = do_get_or_save_boot_env(bm_op_boot_env_args.name, bm_op_boot_env_args.value, bm_op_boot_env_args.operation);
	op_ret = copy_to_user((boot_env_t*)arg,&bm_op_boot_env_args,sizeof(boot_env_t));	

	return retval;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int ioctl_proc_get_mac(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
#else
int ioctl_proc_get_mac(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
	sys_mac_add sys_mac;
	int retval = 0;
	int op_ret = 0;
	
	memset(&sys_mac, 0, sizeof(sys_mac_add));	

	DBG(debug_ioctl, "%s %d: need to be supported.\n", __func__, __LINE__);
	op_ret = copy_from_user(&sys_mac,(sys_mac_add *)arg,sizeof(sys_mac_add));
	retval = -1;
	op_ret = copy_to_user((sys_mac_add *)arg,&sys_mac,sizeof(sys_mac_add));

	return retval;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)

#else

#endif

int ioctl_proc_bootrom_exch(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	bootrom_file bootrom;
	int retval = 0;
	int op_ret = 0;

	memset(&bootrom, 0, sizeof(bootrom_file));	
	
	DBG(debug_ioctl, "%s %d: need to be supported.\n", __func__, __LINE__);
	op_ret = copy_from_user(&bootrom,(bootrom_file*)arg,sizeof(bootrom_file));
	retval = -1;

	return retval;
}
int ioctl_proc_get_producet_type(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	sys_product_type_t product_type;
	int retval = 0;
	int op_ret = 0;

	memset(&product_type, 0, sizeof(sys_product_type_t));	

	DBG(debug_ioctl, "%s %d: need to be supported.\n", __func__, __LINE__);
	op_ret = copy_from_user(&product_type, (sys_product_type_t *)arg, sizeof(sys_product_type_t));
	retval = -1;
	op_ret = copy_to_user((sys_product_type_t *)arg, &product_type, sizeof(sys_product_type_t));	

	return retval;
}

int ioctl_proc_kernel_debug(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	int debug_level = 0;

	retval = copy_from_user(&debug_level, (int *)arg, sizeof(int));
	printk("set debug level is %d.\n", debug_level);
	if (debug_level <= KERNEL_DEBUG_IO_ON)
	{
		debug_ioctl = debug_level - KERNEL_DEBUG_IO_OFF;
		printk("set debug ioctl level is %d.\n", debug_ioctl);
	}
	else if (debug_level <= KERNEL_DEBUG_OCTEON_ON)
	{
		debug_octeon = debug_level - KERNEL_DEBUG_OCTEON_OFF;
		printk("set debug octeon level is %d.\n", debug_octeon);		

	}

	return 0;
}

int ioctl_proc_self_reset(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;

	if (ko_product->reset_board)
	{
		retval = ko_product->reset_board(kboard_info->slot_index);
		if (retval < 0)
		{
			DBG(debug_ioctl, "reset board %d error.\n", kboard_info->slot_index);
			return -1;
		}
	}
	else
	{
		DBG(debug_ioctl, "reset board method none.\n");
		retval = -1;
	}

	return retval;

}


int ioctl_proc_board_reset(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0, op_ret= 0;
	int slot_index = 0;
	cpld_op_args cpld_op_data;

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ioctl_proc_cpld_general_handler\n");
	memset(&cpld_op_data, 0, sizeof(cpld_op_args));

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));

	slot_index = cpld_op_data.param;

	/* FIXME:Now just write hard code,  future need fix */
	if (ko_product->reset_board)
	 	ko_product->reset_board(slot_index);

	return retval;
}

int ioctl_proc_board_online(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_op_args cpld_op_data;
	int retval = 0, op_ret= 0;
	int slot_index = 0;

	DBG(debug_octeon, KERN_INFO DRIVER_NAME ":Enter ioctl_proc_board_online\n");

	memset(&cpld_op_data, 0, sizeof(cpld_op_args));

	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args*)arg, sizeof(cpld_op_args));
	//retval = do_get_product_type(&product_type);
	slot_index = cpld_op_data.param;

	if (ko_product->get_board_online_state)
		cpld_op_data.value = ko_product->get_board_online_state(slot_index);
	else
		cpld_op_data.value = -1;
	
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));	

	return retval;

}

int ioctl_proc_board_num(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int board_num = ko_product->slotnum;
	
	DBG(debug_ioctl, "board num %d .\n", board_num);
	ret = __put_user(board_num, (int __user *)arg);
	if (ret != 0)
	{
		DBG(debug_ioctl, "board num %d put user fail.\n", board_num);
		ret = -1;
	}
	
	return 0;
}

int ioctl_proc_product_hwcode(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	cpld_op_args cpld_op_data;
	int  op_ret= 0;

	DBG(debug_ioctl, "Enter product hwcode.\n");

	memset(&cpld_op_data, 0, sizeof(cpld_op_args));
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = ko_product->product_code;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));
	return op_ret;
}

/**************************************************************************
 *
 *	proc file
 *	
 **************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_module_sn_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_MODULE0_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_module_serial_no) == 0)
		seq_printf(file, "BLANKDEVINFO\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_module_serial_no);
	
	return 0;
}

static int bm_proc_module_sn_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_module_sn_show, NULL);
}

struct file_operations bm_proc_module_sn = {
    .open    = bm_proc_module_sn_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_module_name_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_MODULE0_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_module_name) == 0)
		seq_printf(file, "BLANKDEVINFO\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_module_name);

	return 0;
}

static int bm_proc_module_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_module_name_show, NULL);
}

struct file_operations bm_proc_module_name = {
    .open    = bm_proc_module_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////

static int bm_proc_product_sn_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_product_serial_no) == 0)
		seq_printf(file, "BLANKDEVINFO\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_product_serial_no);

	return 0;
}

static int bm_proc_product_sn_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_product_sn_show, NULL);
}

struct file_operations bm_proc_product_sn = {
    .open    = bm_proc_product_sn_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_product_base_mac_addr_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_product_base_mac_address) == 0)
		seq_printf(file, "000000AABBCC\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_product_base_mac_address);

	return 0;
}

static int bm_proc_product_base_mac_addr_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_product_base_mac_addr_show, NULL);
}

struct file_operations bm_proc_product_base_mac_addr = {
    .open    = bm_proc_product_base_mac_addr_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_product_name_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_product_name) == 0)
		seq_printf(file, "BLANKDEVINFO\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_product_name);

	return 0;
}

static int bm_proc_product_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_product_name_show, NULL);
}

struct file_operations bm_proc_product_name = {
    .open    = bm_proc_product_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_software_name_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_software_name) == 0)
		seq_printf(file, "CHANOS\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_software_name);

	return 0;
}

static int bm_proc_software_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_software_name_show, NULL);
}

struct file_operations bm_proc_software_name = {
    .open    = bm_proc_software_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_enterprise_name_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_enterprise_name) == 0)
		seq_printf(file, "CHANOS\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_enterprise_name);

	return 0;
}

static int bm_proc_enterprise_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_enterprise_name_show, NULL);
}

struct file_operations bm_proc_enterprise_name = {
    .open    = bm_proc_enterprise_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_enterprise_snmp_oid_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_enterprise_snmp_oid) == 0)
		seq_printf(file, "31656");
	else
		seq_printf(file, "%s", sysinfo.ax_sysinfo_enterprise_snmp_oid);

	return 0;
}

static int bm_proc_enterprise_snmp_oid_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_enterprise_snmp_oid_show, NULL);
}

struct file_operations bm_proc_enterprise_snmp_oid = {
    .open    = bm_proc_enterprise_snmp_oid_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_snmp_sys_oid_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_snmp_sys_oid) == 0)
		seq_printf(file, "BLANKDEVINFO");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_snmp_sys_oid);

	return 0;
}

static int bm_proc_snmp_sys_oid_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_snmp_sys_oid_show, NULL);
}

struct file_operations bm_proc_snmp_sys_oid = {
    .open    = bm_proc_snmp_sys_oid_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_snmp_product_oid_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_snmp_product_oid) == 0)
		seq_printf(file, "BLANKDEVINFO");
	else
		seq_printf(file, "%s", sysinfo.ax_sysinfo_snmp_product_oid);

	return 0;
}

static int bm_proc_snmp_product_oid_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_snmp_product_oid_show, NULL);
}

struct file_operations bm_proc_snmp_product_oid = {
    .open    = bm_proc_snmp_product_oid_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_support_url_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_support_url)== 0)
		seq_printf(file, "www.open-switch.org");
	else
		seq_printf(file, "%s", sysinfo.ax_sysinfo_support_url);

	return 0;
}

static int bm_proc_support_url_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_support_url_show, NULL);
}

struct file_operations bm_proc_support_url = {
    .open    = bm_proc_support_url_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_admin_username_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_built_in_admin_username)== 0)
		seq_printf(file, "admin\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_built_in_admin_username);

	return 0;
}

static int bm_proc_admin_username_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_admin_username_show, NULL);
}

struct file_operations bm_proc_admin_username = {
    .open    = bm_proc_admin_username_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_admin_passwd_show(struct seq_file *file, void *ptr)
{
	ax_sysinfo_product_t sysinfo;
	int rval = 0;
	
	memset(&sysinfo, 0, sizeof(sysinfo));
	rval = ax_read_sysinfo_from_eeprom_proc(BM_AX_BACKPLANE_EEPROM_ADDR, &sysinfo);

	if (rval < 0 || strlen(sysinfo.ax_sysinfo_built_in_admin_password)== 0)
		seq_printf(file, "admin\n");
	else
		seq_printf(file, "%s\n", sysinfo.ax_sysinfo_built_in_admin_password);

	return 0;
}

static int bm_proc_admin_passwd_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_admin_passwd_show, NULL);
}

struct file_operations bm_proc_admin_passwd = {
    .open    = bm_proc_admin_passwd_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////

static int bm_proc_slot_id_show(struct seq_file *file, void *ptr)
{
	int slot_index = 0;

	if (ko_product->slot_index_get)
	{
		slot_index = ko_product->slot_index_get();
	}
	else
	{
		slot_index = -1;
	}
		
	if (slot_index < 0)
		seq_printf(file, "Read slot no error\n");
	else
		seq_printf(file, "%d\n", slot_index+1);

	return 0;
}

static int bm_proc_slot_id_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_slot_id_show, NULL);
}

struct file_operations bm_proc_slot_id = {
    .open    = bm_proc_slot_id_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int bm_proc_board_name_show(struct seq_file *file, void *ptr)
{
	if (!ko_board->board_name)
		seq_printf(file, "Read board name error\n");
	else
		seq_printf(file, "%s\n", ko_board->board_name);

	return 0;
}

static int bm_proc_board_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_board_name_show, NULL);
}

struct file_operations bm_proc_board_name = {
    .open    = bm_proc_board_name_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int bm_proc_product_type_show(struct seq_file *file, void *ptr)
{
	if (!kboard_info)
		seq_printf(file, "Read product type error\n");
	else
		seq_printf(file, "%ld\n", kboard_info->product_type);

	return 0;
}

static int bm_proc_product_type_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_product_type_show, NULL);
}

struct file_operations bm_proc_product_type = {
    .open    = bm_proc_product_type_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_board_type_show(struct seq_file *file, void *ptr)
{


	if (!kboard_info)
		seq_printf(file, "Read board type error\n");
	else
		seq_printf(file, "%ld\n", kboard_info->board_type);

	return 0;
}

static int bm_proc_board_type_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_board_type_show, NULL);
}

struct file_operations bm_proc_board_type = {
    .open    = bm_proc_board_type_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};


//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_master_slot_id_show(struct seq_file *file, void *ptr)
{
	int board_state = 1;
	if (board_state < 0)
		seq_printf(file, "Read master slot id state error\n");
	else
		seq_printf(file, "%d\n", kboard_info->slot_index);

	return 0;
}

static int bm_proc_master_slot_id_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_master_slot_id_show, NULL);
}

struct file_operations bm_proc_master_slot_id = {
    .open    = bm_proc_master_slot_id_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

//////////////////////////////////////////////////////////////////////////////////////
static int bm_proc_slot_num_show(struct seq_file *file, void *ptr)
{
	int slot_num = 0;

	if (ko_product->slotnum)
		slot_num = ko_product->slotnum;
	else
		slot_num = 0;

	seq_printf(file, "%d\n", slot_num);

	return 0;
}

static int bm_proc_slot_num_open(struct inode *inode, struct file *file)
{
    return single_open(file, bm_proc_slot_num_show, NULL);
}

struct file_operations bm_proc_slot_num = {
    .open    = bm_proc_slot_num_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

int bm_product_series_init(void)
{
	return 0;
}


