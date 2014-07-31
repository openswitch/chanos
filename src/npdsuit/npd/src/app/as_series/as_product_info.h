#ifndef __NH_PRODUCT_INFO_H__
#define __NH_PRODUCT_INFO_H__

extern unsigned long nh_local_module_hw_code_get();
extern unsigned long nh_product_hw_code_get(void);
extern unsigned char nh_board_hw_version_get();

extern unsigned char nh_board_hw_slotno_get();

extern long as_load_backinfo(product_man_param_t *param);

extern void as_sys_reset();

extern void as_chassis_show();

extern long as_board_online(unsigned long slot_index);

extern long as_board_detect_start();

extern long as_master_board_online(unsigned long slot_index);

extern long as_board_reset(unsigned long slot_index);

extern long as_board_poweroff(unsigned long slot_index);

extern long as_pne_monitor_start();
extern long as_fan_speed_adjust(unsigned long index, unsigned long speed);
extern long as_slotno_get();
extern long as6603_board_online(unsigned long slot_index);

extern long as6600_power_man_param_init(power_param_t * param);
extern long as6600_fan_param_init(fan_param_t * param);

extern long as6600_pne_monitor_start(void);
extern int as6600_pne_monitor(void);

extern int as6600_board_mnparam_get(board_man_param_t *info);

extern void as6600_npd_master_set(void);

extern void as6600_int_power_handler(unsigned char int_value, unsigned char add_value);
#endif
