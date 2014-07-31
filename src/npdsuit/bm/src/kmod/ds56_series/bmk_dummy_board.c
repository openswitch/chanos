
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"
#include "bmk_operation_boot_env.h"
#include "bmk_read_eeprom.h"

//spinlock_t int_read_lock = SPIN_LOCK_UNLOCKED;
int dummy_board_slot_index_get(void);

extern spinlock_t int_read_lock;

cpld_int_reg_param dummy_board_int_param_arr[] =
{
};

/**************************************************************************
 *
 *	ioctrl proc
 *	
 **************************************************************************/


int dummy_board_ioctl_proc_get_int_info(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_cpld_read(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_cpld_write(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_soft_i2c_read16(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

//write two bytes to eeprom through 16bit internal address
int dummy_board_ioctl_proc_soft_i2c_write16(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_soft_i2c_read8(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_soft_i2c_write8(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_g(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_x(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_bit64_reg_state(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_read_product_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}
int dummy_board_ioctl_proc_read_module_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}
int dummy_board_ioctl_proc_read_sysinfo_read_16bit(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}
int dummy_board_ioctl_proc_board_eth_port_stats_read(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}
int dummy_board_ioctl_proc_env_exch(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	boot_env_t bm_op_boot_env_args;
	int retval = 0;
	int op_ret = 0;

	memset(&bm_op_boot_env_args, 0, sizeof(boot_env_t));	
	
	DBG(debug_ioctl, "%s %d: need to be supported.\n", __func__, __LINE__);
	op_ret = copy_from_user(&bm_op_boot_env_args,(boot_env_t *)arg, sizeof(boot_env_t));
	retval = -1;
	op_ret = copy_to_user((boot_env_t*)arg,&bm_op_boot_env_args,sizeof(boot_env_t));	

	return retval;
}

int dummy_board_ioctl_proc_get_mac(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}
int dummy_board_ioctl_proc_bootrom_exch(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	bootrom_file bootrom;
	int retval = 0;
	int op_ret = 0;

	memset(&bootrom, 0, sizeof(bootrom_file));	
	DBG(debug_ioctl,"%s %d: need to be supported.\n", __FUNCTION__, __LINE__);
	op_ret = copy_from_user(&bootrom,(bootrom_file*)arg,sizeof(bootrom_file));
	retval = -1;	

	return retval;
}
int dummy_board_ioctl_proc_get_producet_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}


int dummy_board_ioctl_proc_board_reset(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_wdt_enable(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_board_online(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_ioctl,"Enter %s.\n", __FUNCTION__);
	return 0;
}

int dummy_board_ioctl_proc_cpld_general_handler(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = 1;
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return op_ret;
}


int dummy_board_ioctl_proc_cpld_slot_id(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret;
	cpld_op_args cpld_op_data;
	
	op_ret = copy_from_user(&cpld_op_data, (cpld_op_args *)arg, sizeof(cpld_op_args));
	cpld_op_data.value = dummy_board_slot_index_get();
	op_ret = copy_to_user((cpld_op_args *)arg, &cpld_op_data, sizeof(cpld_op_args));

	return op_ret;
}


void dummy_board_cpld_product_family_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = FAMILY_TYPE_T9000;

}

void dummy_board_cpld_product_type_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = PRODUCT_DUMMY;	
}

void dummy_board_cpld_module_type_handler(void * data)
{
	cpld_op_args * ptr_op_param = (cpld_op_args *)data;

	ptr_op_param->value = PPAL_BOARD_TYPE_DUMMY;	

}

cpld_reg_ctl dummy_board_cpld_ctrl[] = 
{
	
    {BM_IOC_CPLD_TEST,				 0x00, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_PRODUCT_FAMILY,     0x01, 1, 0xFF, 0, dummy_board_cpld_product_family_handler},
	{BM_IOC_CPLD_PRODUCT_TYPE,       0x02, 1, 0x1C, 2, dummy_board_cpld_product_type_handler},
    {BM_IOC_CPLD_MODULE_TYPE,        0x03, 1, 0xFF, 0, dummy_board_cpld_module_type_handler},
    {BM_IOC_CPLD_PCB_VERSION,        0x04, 1, 0x07, 0, NULL},
    {BM_IOC_CPLD_CPLD_VERSION,       0x05, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_BACKBOARD_VERSION,  0x02, 1, 0x03, 0, NULL},
    {BM_IOC_CPLD_MATE_MASTER_RESET,  0x12, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_LINECARD_RESET,     0x07, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_SELF_SYS_RESET,     0x08, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_WDT_ENABLE,         0x0D, 1, 0x01, 0, NULL},
    {BM_IOC_CPLD_WDT_TIMER,          0x0E, 1, 0x3F, 0, NULL},
    {BM_IOC_CPLD_WDT_CLEAR,          0x0F, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_LINECARD_ONLINE,    0x10, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_MATE_MASTER_ONLINE, 0x11, 1, 0x01, 0, NULL},
    {BM_IOC_CPLD_MASTER_STATE,       0x38, 1, 0x01, 0, NULL},
    {BM_IOC_CPLD_MASTER_SWITCH,      0x38, 1, 0x01, 0, NULL},
    {BM_IOC_CPLD_SLOT_ID,            0x2E, 1, 0x01, 0, NULL},
    {BM_IOC_CPLD_FAN_PRESENT,        0x11, 1, 0x02, 1, NULL},
    {BM_IOC_CPLD_FAN_ALARM,          0x11, 1, 0x04, 2, NULL},
    {BM_IOC_CPLD_FAN_SPEED,          0x3A, 1, 0xFF, 0, NULL},
    {BM_IOC_CPLD_POWER_PRESENT,      0x32, 1, 0x1F, 0, NULL},

	{BM_IOC_CPLD_PRODUCT_HWCODE,     0x02, 1, 0x1C, 2, NULL},
    {BM_IOC_CPLD_MODULE_HWCODE,      0x03, 1, 0xFF, 0, NULL},
};

ioctl_proc dummy_board_ioctltl_proc_arr[] = 
{
	{BM_IOC_G_, 					dummy_board_ioctl_proc_g},
	{BM_IOC_X_, 					dummy_board_ioctl_proc_x},
	{BM_IOC_CPLD_READ, 				dummy_board_ioctl_proc_cpld_read},
	{BM_IOC_CPLD_WRITE, 			dummy_board_ioctl_proc_cpld_write},
	{BM_IOC_BIT64_REG_STATE, 		dummy_board_ioctl_proc_bit64_reg_state},
	{BM_IOC_READ_PRODUCT_SYSINFO, 	dummy_board_ioctl_proc_read_product_sysinfo},
	{BM_IOC_READ_MODULE_SYSINFO, 	dummy_board_ioctl_proc_read_module_sysinfo},
	{BM_IOC_BOARD_ETH_PORT_STATS_READ, 	dummy_board_ioctl_proc_board_eth_port_stats_read},
	{BM_IOC_ENV_EXCH, 				dummy_board_ioctl_proc_env_exch},
	{BM_IOC_GET_MAC, 				dummy_board_ioctl_proc_get_mac},
	{BM_IOC_BOOTROM_EXCH, 			dummy_board_ioctl_proc_bootrom_exch},
	{BM_IOC_GET_PRODUCET_TYPE, 		dummy_board_ioctl_proc_get_producet_type},	
	{BM_IOC_GET_INT_INFO,			dummy_board_ioctl_proc_get_int_info},

	{BM_IOC_CPLD_PRODUCT_FAMILY,	dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_PRODUCT_TYPE,		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_MODULE_TYPE,		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_PCB_VERSION,		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_CPLD_VERSION,		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_BACKBOARD_VERSION,	dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_BOARD_RESET, 		dummy_board_ioctl_proc_board_reset},
	{BM_IOC_CPLD_SELF_SYS_RESET, 	ioctl_proc_self_reset},

	{BM_IOC_CPLD_WDT_ENABLE,		dummy_board_ioctl_proc_wdt_enable},
	{BM_IOC_CPLD_WDT_TIMER,			dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_WDT_CLEAR,			dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_BOARD_ONLINE, 		dummy_board_ioctl_proc_board_online},
	
	{BM_IOC_CPLD_MASTER_STATE, 		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_MASTER_SWITCH,		dummy_board_ioctl_proc_cpld_general_handler},
 	{BM_IOC_CPLD_SLOT_ID,			dummy_board_ioctl_proc_cpld_slot_id},
	{BM_IOC_CPLD_FAN_PRESENT, 		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_FAN_ALARM, 		dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_FAN_SPEED,			dummy_board_ioctl_proc_cpld_general_handler},
	{BM_IOC_CPLD_POWER_PRESENT, 	dummy_board_ioctl_proc_cpld_general_handler},
	

	{BM_IOC_CPLD_PRODUCT_HWCODE,    dummy_board_ioctl_proc_cpld_general_handler},
    {BM_IOC_CPLD_MODULE_HWCODE,     dummy_board_ioctl_proc_cpld_general_handler},

};


int dummy_board_slot_index_get()
{
	return 1;
}

int dummy_board_init(void)
{
	int result = 0;

	ko_board->ioctl_proc_count = LENGTH(dummy_board_ioctltl_proc_arr);
	DBG(debug_ioctl,  "ioctl_proc_count is %d.\n", ko_board->ioctl_proc_count);
	
	ko_board->cpld_int_reg_param_count = LENGTH(dummy_board_int_param_arr);
	DBG(debug_ioctl,  "cpld_int_reg_param_count is %d.\n", ko_board->cpld_int_reg_param_count);
	
	ko_board->cpld_reg_ctrl_count = LENGTH(dummy_board_cpld_ctrl);
	DBG(debug_ioctl,  "cpld_reg_ctrl_count is %d.\n", ko_board->cpld_reg_ctrl_count);

	return result;
	
}

void dummy_board_cleanup(void)
{

}


kboard_fix_param dummy_board_board  =
{	
	.board_code = PPAL_BOARD_HWCODE_DUMMY,
	.board_type = PPAL_BOARD_TYPE_DUMMY,
	.board_name = "DUMMY_BOARD",
		
	.board_init = dummy_board_init,
	.board_cleanup = dummy_board_cleanup,
	//.slot_index_get = dummy_board_slot_index_get,

	.ioctl		= bm_board_ioctl,
	.ioctl_proc_arr = dummy_board_ioctltl_proc_arr,
	
	.interrupt_handler = bm_board_interrupt_handler,
	.irq_board	= 0,
	.cpld_int_reg_param_arr = dummy_board_int_param_arr,		/* int reg */
	
	.cpld_reg_base_addr = CPLD_REG_BASE_ADDR,
	.cpld_reg_ctrl_arr = dummy_board_cpld_ctrl,
};


