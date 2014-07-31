#ifndef __NH_PRODUCT_COMMON_INFO_H__
#define __NH_PRODUCT_COMMON_INFO_H__

extern unsigned long nh_family_type_get();

extern unsigned long nh_local_module_hw_code_get();
extern unsigned long nh_product_hw_code_get(void);
extern unsigned char nh_board_hw_version_get();

extern unsigned char nh_board_hw_slotno_get();

extern long nh_load_backinfo(product_man_param_t *param);

extern void nh_sys_reset();

extern void nh_chassis_show();

extern long nh_board_online(unsigned long slot_index);

extern long nh_board_detect_start();

extern long nh_master_board_online(unsigned long slot_index);

extern long nh_board_reset(unsigned long slot_index);

extern long nh_board_poweroff(unsigned long slot_index);

extern long nh_pne_monitor_start();
extern long nh_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long nh_slotno_get();
void nh_npd_master_set(unsigned long value);
long nh_os_upgrade(unsigned int slot_index);

#define MODULE_NAME_FILE 	"/proc/sysinfo/module_name"
#define MODULE_SN_FILE 		"/proc/sysinfo/module_sn"
#define OEM_MODULE_NAME_FILE 	"/devinfo/module_name"

#define TEMP_VARIATION	3

#endif
