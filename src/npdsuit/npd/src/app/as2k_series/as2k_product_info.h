#ifndef __NH_PRODUCT_INFO_H__
#define __NH_PRODUCT_INFO_H__
extern unsigned long as2k_family_type_get();
extern unsigned long as2k_local_module_hw_code_get();
extern unsigned long as2k_product_hw_code_get(void);
extern unsigned char as2k_board_hw_version_get();

extern unsigned char as2k_board_hw_slotno_get();

extern long as2k_load_backinfo(product_man_param_t *param);

extern void as2k_sys_reset();

extern void as2k_chassis_show();

extern long as2k_board_online(unsigned long slot_index);

extern long as2k_board_detect_start();

extern long as2k_master_board_online(unsigned long slot_index);

extern long as2k_board_reset(unsigned long slot_index);

extern long as2k_board_poweroff(unsigned long slot_index);

extern long as2k_slotno_get();
extern long t9006_board_online(unsigned long slot_index);

extern int as2k_board_mnparam_get(board_man_param_t *info);

extern void as2k_npd_master_set(void);

extern long as2k_3200_local_conn_init(int product_type);

extern long as2k_3200_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    );

extern int as2k_3200_set_com_config(int fd, int baud_rate, 
		int data_bits, char parity, int stop_bits);


extern int as2k_is_cpu_busy();
extern void as2k_start_pne_monitor(void );

#define LED_YELLOW	0
#define LED_GREEN	1

extern void as2k_3200_fan_led_light(
	unsigned char param_ON,
	unsigned char param_OFF,
	unsigned char param_MASK,
	unsigned char param_IN_MASK,	
	int param_i2c_addr,
	char* param_i2c_dev,
	int led_state);
	
extern unsigned int  as2k_3200_fan_present(
	unsigned char param_PRESENT,
	int  param_i2c_addr,
	char *	param_i2c_dev);
	
extern unsigned int  as2k_3200_fan_alarm(
	unsigned char param_ALARM,
	int  param_i2c_addr,
	char *	param_i2c_dev);
	

extern unsigned  as2k_3200_power_led_light(
	unsigned char param_LED_YELLOW,
	unsigned char param_LED_OFF,
	unsigned char param_LED_GREEN,
	unsigned char param_LED_MASK,
	unsigned char param_IN_MASK,	
	int param_i2c_addr,
	char * param_i2c_dev,
	int led_on_state,
	int led_color);

#endif
