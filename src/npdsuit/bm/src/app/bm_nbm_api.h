#ifndef BM_NBM_API_H
#define BM_NBM_API_H


typedef struct ax_sysinfo_product_t
{
	char  ax_sysinfo_module_serial_no[32]; //data should be 20 bytes the last byte is '\0'
	char  ax_sysinfo_module_name[25];  //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_product_serial_no[32]; //data should be 20 bytes the last byte is '\0' 
	char  ax_sysinfo_product_base_mac_address[13]; //data should be 12 bytes the last byte is '\0' 
	char  ax_sysinfo_product_name[25]; //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_software_name[25]; //data max length should be 24 bytes the last byte is '\0' 
	char  ax_sysinfo_enterprise_name[65]; //data max length should be 64 bytes the last byte is '\0' 
	char  ax_sysinfo_enterprise_snmp_oid[129]; //data max length should be 128 bytes the last byte is '\0' 
	char  ax_sysinfo_snmp_sys_oid[129]; //data max length should be 128 bytes the last byte is '\0' 
	char  ax_sysinfo_built_in_admin_username[33]; //data max length should be 32 bytes the last byte is '\0' 
	char  ax_sysinfo_built_in_admin_password[33]; //data max length should be 32 bytes the last byte is '\0'  
	char  ax_sysinfo_snmp_product_oid[129]; //data max length should be 128 bytes the last byte is '\0'  
	char  ax_sysinfo_support_url[65]; //data max length should be 65bytes the last byte is '\0'  
}ax_sysinfo_product_t;

typedef ax_sysinfo_product_t ax_product_sysinfo;


struct product_sysinfo_s {
	char *sn;
	char *name;
	char *basemac;  /* 12 char of mac address  with no : or - spliter.*/
	char *sw_name;
	char *enterprise_name;
	char *enterprise_snmp_oid;
	char *snmp_sys_oid;
	char *built_in_admin_username;
	char *built_in_admin_passwd;
};

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char	u8;


#define SYSTEM_HARDWARE_WATCHDOG_ENABLE		1
#define SYSTEM_HARDWARE_WATCHDOG_DISABLE	0

#define LOCALBOARD_SYSINFO_EEPROM_ADDR	0x57

#define TEMP_MON_ADDR					0x18


unsigned long nbm_get_product_type();
unsigned long nbm_get_product_hw_code();
unsigned long nbm_get_module_type();
unsigned long nbm_get_module_hw_code();
unsigned char nbm_get_board_hw_version();
unsigned char nbm_get_backboard_version();


void nbm_sys_reset();
long nbm_board_online(unsigned long slot_index);
long nbm_board_reset(unsigned long slot_index);
long nbm_board_poweroff(unsigned long slot_index);
long nbm_slotno_get();
long nbm_master_slotno_get();
long nbm_board_num_get();



int nbm_hardware_watchdog_control_set(unsigned int enabled);
int nbm_hardware_watchdog_control_get(unsigned int *enabled);
int nbm_hardware_watchdog_timeout_set(unsigned int timeout);
int nbm_hardware_watchdog_timeout_get(unsigned int *timeout);
int nbm_hardware_watchdog_fillup(void);

int nbm_eeprom_write(u8 dev_addr, u16 reg_addr, char * buf, int * len);
int nbm_eeprom_read(u8 dev_addr, u16	 reg_addr, char * buf, int * len);
int nbm_i2c_read_byte(long * read_value, u8 dev_addr, u8	reg_addr);
int nbm_eeprom_read_one(u8 dev_addr, u16	 reg_addr, char * buf, int * len);

//DMI, Digital Monitoring Interface
int nbm_sfp_num_get();
int nbm_sfp_light_get(int index, int * light_state);
int nbm_sfp_light_set(int index, int light_state);
int nbm_sfp_presence_get(int index, int * presence_state);
int nbm_sfp_write(int index, unsigned int reg_addr, char * buf, int len);
int nbm_sfp_read(int index, unsigned int reg_addr, char *buf,int len);
int nbm_sfp_dmi_LOS_get(int index, int * los_state);
int nbm_sfp_dmi_Tx_fault_get(int index, int * tx_fault_state);
int nbm_xfp_cdr_state_get(int index, int *cdr_state);

long nbm_get_power_info(int index, power_info_args * power_info);
long nbm_get_power_present(int index, int * online_state);
long nbm_get_power_state(int index, int * state);

int nbm_get_temp_info(temp_info_args * temp_data);
int nbm_get_temp_state(int * temp_state);
int nbm_set_temp_threshold(temp_op_args * temp_op);

long nbm_get_fan_state(int index, int * state);
long nbm_get_fan_present(int index, int * online_state);
long nbm_get_fan_speed(int index, int * speed);
long nbm_set_fan_speed(int index, int  speed);

long nbm_led_control(led_op_args * led_op);
long nbm_get_led_info(led_op_args * led_op)	;

long nbm_get_boot_version_name(char * vername);

int nbm_read_backplane_sysinfo(struct product_sysinfo_s *product_sysinfo) ;

int nbm_kernel_debug(int debug_level);

int nbm_master_get();

int nbm_master_set(unsigned long value);

int nbm_poe_led(poe_port_t *poe_port);
long nbm_op_boot_env(boot_env_t *env_args);
long nbm_port_isolation_control(port_isolation_op_args *port_isolation_args);
long nbm_subcard_info_operate(music_subcard_info_t *subcard_info);



#endif
