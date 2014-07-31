#ifndef _MAN_PRODUCT_H
#define _MAN_PRODUCT_H



struct product_info
{
    char name[128];
    char serial_no[64];
    char sw_name[32];
    unsigned int sw_version;
    unsigned int hw_version;
    char bootrom_version[128];
    char base_mac[32];
    unsigned int product_id;
};
struct board_mng_s
{
	int run_state;
	int inserted;
	int online_removed;
	int work_mode;
	int redundancy_state;
	unsigned long configure_type;
	unsigned long led_status;
	int board_code;
	int board_type;
	int ams_type;
	unsigned char hw_ver;
	char sn[24];
	char modname[24];
	char fullname[64];
	char shortname[32];
	char sw_ver[32];
};

struct fan_info
{
    int state;
    int speed;
    unsigned int inserted;
    
};

struct temperature_info
{
    int temp_state;
	int ready;
	int core_temp;
	int core_upper_limit;
	int core_lower_limit;
	int surface_temp;
	int surface_upper_limit;
	int surface_lower_limit;
};

struct power_info
{
    int ps_state;
	char ps_type[16];
	unsigned int inserted;
};

int board_info_get_next(unsigned int *slot, struct board_mng_s* board_attr);
int temperature_get_next(int *slotno,struct temperature_info *tempre_info);
int fan_get_next(int *fanno,struct fan_info *faninfo);
int show_product_info(struct product_info *info);
int power_get_next(int *power_no, struct power_info *ps_info);
#endif
