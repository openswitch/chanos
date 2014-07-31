#ifndef __US_PRODUCT_INFO_H__
#define __US_PRODUCT_INFO_H__

extern unsigned long us_family_type_get();

extern unsigned long us_product_hw_code_get();

extern unsigned long us_local_module_hw_code_get();

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

extern long us_slotno_get();

extern long us_pne_monitor_start();

extern long us_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long us3000_power_man_param_init(power_param_t * param);
extern long us3000_fan_param_init(fan_param_t * param);

extern long us3000_pne_monitor_start(void);
extern int us3000_pne_monitor(void);

extern long us3000_board_mnparam_get(board_man_param_t *info);


extern void us3000_npd_master_set(unsigned long value);

extern void us3000_int_power_handler(unsigned char int_value, unsigned char add_value);
#endif
