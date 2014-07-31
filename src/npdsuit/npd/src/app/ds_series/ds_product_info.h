#ifndef __US_PRODUCT_INFO_H__
#define __US_PRODUCT_INFO_H__

extern unsigned long ds_family_type_get();

extern unsigned long ds_product_hw_code_get();

extern unsigned long ds_local_module_hw_code_get();

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

extern long ds_slotno_get();

extern long ds_pne_monitor_start();

extern long ds_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long ds5600_power_man_param_init(power_param_t * param);
extern long ds5600_fan_param_init(fan_param_t * param);

extern long ds5600_pne_monitor_start(void);
extern int ds5600_pne_monitor(void);

extern long ds5600_board_mnparam_get(board_man_param_t *info);


extern void ds5600_npd_master_set(unsigned long value);

extern void ds5600_int_power_handler(unsigned char int_value, unsigned char add_value);
#endif
