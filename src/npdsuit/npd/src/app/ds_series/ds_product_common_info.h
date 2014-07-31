#ifndef __NH_PRODUCT_COMMON_INFO_H__
#define __NH_PRODUCT_COMMON_INFO_H__

extern unsigned long ds_family_type_get();

extern unsigned long ds_local_module_hw_code_get();
extern unsigned long ds_product_hw_code_get(void);
extern unsigned char ds_board_hw_version_get();

extern unsigned char ds_board_hw_slotno_get();

extern long ds_load_backinfo(product_man_param_t *param);

extern void ds_sys_reset();

extern void ds_chassis_show();

extern long ds_board_online(unsigned long slot_index);

extern long ds_board_detect_start();

extern long ds_master_board_online(unsigned long slot_index);

extern long ds_board_reset(unsigned long slot_index);

extern long ds_board_poweroff(unsigned long slot_index);
extern int ds_main_board_slot_support(int product_type, int slot, int subslot);

extern long ds_pne_monitor_start();
extern long ds_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long ds_slotno_get();
void ds_npd_master_set(void);
#define MODULE_NAME_FILE 	"/devinfo/module_name"
#define MODULE_SN_FILE 		"/devinfo/module_sn"
#define TEMP_VARIATION	3

#endif
