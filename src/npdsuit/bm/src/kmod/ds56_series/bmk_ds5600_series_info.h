#ifndef BMK_PRODUCT_INFO_H
#define BMK_PRODUCT_INFO_H

#include <linux/version.h>


void ds5600_master_get_mngt_eth_mac(int port, char *mac);
void ds5600_linecard_get_mngt_eth_mac(int port, char *mac);

int bm_proc_init(void);
int bm_proc_clearup(void);

irqreturn_t bm_board_interrupt_handler(int irq, void *dev_id);
extern irqreturn_t bm_cpld_interrupt_handler(int irq, void *dev_id);

void cpld_product_family_handler(void * data);
void cpld_product_type_handler(void * data);
void cpld_module_type_handler(void * data);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int bm_common_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int bm_product_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int bm_board_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


int ioctl_proc_self_reset(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_reset(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_online(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_cpld_board_num(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_cpld_product_hwcode(struct file *filp, unsigned int cmd, unsigned long arg);

int ioctl_proc_read_product_sysinfo(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_read_module_sysinfo(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_read_sysinfo_read_16bit(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_env_exch(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_get_mac(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_bootrom_exch(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_get_producet_type(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_kernel_debug( struct file *filp, unsigned int cmd, unsigned long arg);

int ioctl_proc_product_family(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_product_type(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_product_hwcode(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_module_hwcode(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_module_type(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_backboard_version(struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_num(struct file *filp, unsigned int cmd, unsigned long arg);
#else
int bm_common_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int bm_product_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int bm_board_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);


int ioctl_proc_self_reset(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_reset(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_online(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_cpld_board_num(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_cpld_product_hwcode(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

int ioctl_proc_read_product_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_read_module_sysinfo(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_read_sysinfo_read_16bit(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_env_exch(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_get_mac(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_bootrom_exch(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_get_producet_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_kernel_debug(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

int ioctl_proc_product_family(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_product_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_product_hwcode(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_module_hwcode(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_module_type(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_backboard_version(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int ioctl_proc_board_num(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#endif
cpld_reg_ctl * util_get_cmd_reg_ctrl(unsigned int cmd);
long util_slot_is_master_slot(int slot_index);
long util_kernel_read_file(char *filename, char *buf, int *len);


#define BOARD_ISMASTERSLOT(slot_index) \
	(1 == util_slot_is_master_slot(slot_index))


extern struct file_operations bm_proc_module_sn;
extern struct file_operations bm_proc_module_name;
extern struct file_operations bm_proc_product_sn;
extern struct file_operations bm_proc_product_base_mac_addr;
extern struct file_operations bm_proc_product_name;
extern struct file_operations bm_proc_software_name;
extern struct file_operations bm_proc_enterprise_name;
extern struct file_operations bm_proc_enterprise_snmp_oid;
extern struct file_operations bm_proc_snmp_sys_oid;
extern struct file_operations bm_proc_admin_username;
extern struct file_operations bm_proc_admin_passwd;
extern struct file_operations bm_proc_slot_id;
extern struct file_operations bm_proc_board_name;
extern struct file_operations bm_proc_product_type;
extern struct file_operations bm_proc_board_type;
extern struct file_operations bm_proc_master_slot_id;
extern struct file_operations bm_proc_slot_num;

#endif
