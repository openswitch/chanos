#ifndef __NH_PRODUCT_COMMON_INFO_H__
#define __NH_PRODUCT_COMMON_INFO_H__

extern unsigned long us_family_type_get();

extern unsigned long us_local_module_hw_code_get();
extern unsigned long us_product_hw_code_get(void);
extern unsigned char us_board_hw_version_get();

extern unsigned char us_board_hw_slotno_get();

extern long us_load_backinfo(product_man_param_t *param);

extern void us_sys_reset();

extern void us_chassis_show();

extern long us_board_online(unsigned long slot_index);

extern long us_board_detect_start();

extern long us_master_board_online(unsigned long slot_index);

extern long us_board_reset(unsigned long slot_index);

extern long us_board_poweroff(unsigned long slot_index);
extern int us_main_board_slot_support(int product_type, int slot, int subslot);

extern long us_pne_monitor_start();
extern long us_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long us_slotno_get();
void us_npd_master_set(void);
#define MODULE_NAME_FILE 	"/devinfo/module_name"
#define MODULE_SN_FILE 		"/devinfo/module_sn"
#define TEMP_VARIATION	3

#endif
