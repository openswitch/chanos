#ifndef __NH_PRODUCT_INFO_H__
#define __NH_PRODUCT_INFO_H__

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
extern long t9006_board_online(unsigned long slot_index);

extern long t9000_power_man_param_init(power_param_t * param);
extern long t9000_fan_param_init(fan_param_t * param);

extern long t9000_pne_monitor_start(void);
extern int t9000_pne_monitor(void);

extern long t9000_board_mnparam_get(board_man_param_t *info);

extern void t9000_npd_master_set(unsigned long value);

extern void t9000_int_power_handler(unsigned char int_value, unsigned char add_value);
#endif
