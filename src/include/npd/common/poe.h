#ifndef __COMMON_POE_H__
#define __COMMON_POE_H__

#ifdef HAVE_POE

#define NPD_POE_CFGTBL_NAME    "npdPoeCfgTbl"

#define CLASS0_POWER 154
#define CLASS1_POWER 40
#define CLASS2_POWER 70
#define CLASS3_POWER 154
#define CLASS4_POWER 300

#define POE_LOG_NONE	0x0
#define POE_LOG_ALL 0x7f
#define POE_LOG_STATUS_FLAG 1
#define POE_LOG_ERR_FLAG 1<<1
#define POE_LOG_TEMP_FLAG 1<<2
#define POE_LOG_CLASS_FLAG 1<<3
#define POE_LOG_CURRENT_FLAG 1<<4
#define POE_LOG_VOL_FLAG 1<<5
#define POE_LOG_POWER_FLAG 1<<6

typedef struct npd_poe_cfg_s
{
    unsigned int   poe_enable;
}npd_poe_cfg_t;


int npd_poe_port_check(int slot, int sub_slot, int port);

#define IS_POE_PORT(slot, sub_slot, port) \
	npd_poe_port_check(slot, sub_slot, port)


typedef enum _POE_TYPE_
{
	NO_PD,
	IEEE_POE,		/* Support IEEE 802.3af standard  */	
	IEEE_POEPLUS, 	/* Support IEEE 802.3at standard  */
	INVALID_PD
}POE_TYPE;

typedef enum _POE_CLASS_
{
	POE_CLASS_0,	/* Class 0, PSE ouput maxium 15.4W, Defualt Mode*/	
	POE_CLASS_1, 	/* Class 1, PSE ouput maxium 4.0W  */
	POE_CLASS_2, 	/* Class 2, PSE ouput maxium 7.0W  */
	POE_CLASS_3, 	/* Class 3, PSE ouput maxium 15.4W  */
	POE_CLASS_4 	/* Class 4, PSE ouput maxium 30.0W, IEEE 802.3af not Support  */
}POE_CLASS;

typedef enum _POE_PSE_MODE_
{
	//POE_PSE_NONE, 	/* PD's power level and maximum level is specified by user */
	POE_PSE_STATIC = 0,
	POE_PSE_DYNAMIC
}POE_PSE_MODE;
typedef enum _POE_MNG_MODE_
{
    POE_PSE_AUTO,
    POE_PSE_MANUAL
}POE_MNG_MODE;
typedef enum _POE_POWER_MANAGE_MODE_
{
    POE_PSE_AUTO_AND_STATIC,
	POE_PSE_AUTO_AND_DYNAMIC,
    POE_PSE_MANUAL_AND_STATIC,
	POE_PSE_MANUAL_AND_DYNAMIC
}_POE_POWER_MANAGE_MODE_;

typedef enum _POE_POWER_SUPPLY_PRIORITY_
{
	POE_PRIORITY_NONE,
	POE_PRIORITY_LOW,
	POE_PRIORITY_HIGH,
	POE_PRIORITY_CRITICAL
}POE_POWER_SUPPLY_PRIORITY;

typedef enum _POWER_THRESHOLD_TYPE_
{
	POE_THRESHOLD_DEFAULT,/*low power mode 16.2w, high power mode 31.6w*/
	POE_THRESHOLD_CLASS_BASE,/*class0 16.2w, class1 4.2w, class2 7.4w, class3 16.2w, class4 31.6w*/
	POE_THRESHOLD_USER_DEFINE,/*power defined by user*/
}POWER_THRESHOLD_TYPE;


typedef enum _POE_OPERATE_STATUS_
{
	POE_POWER_OFF,
	POE_POWER_ON,
	POE_MPS_ABSENT,/*PSE power not enough*/
	POE_POWER_SHORT,
	POE_POWER_OVERLOAD,
	POE_POWER_DENIED,/*PD request request more power than POEcan supply*/
	POE_TERMINAL_SHUTDOWN,
	POE_STARTUP_FAILURE
}POE_OPERATE_STATUS;

typedef enum _POE_DETECT_STATUS_
{
	POE_DETECT_DISABLE,
	POE_DETECT_SEARCHING,
	POE_DETECT_DELIVERING_POWER,
	POE_DETECT_TEST,
	POE_DETECT_FAULT,
	POE_DETECT_OTHER_FAULT,
	POE_DETECTT_REQUEST_POWER
}POE_DETECT_STATUS;
/*一些操作导致PSE功率分配出现变化*/
typedef enum _PSE_POWER_MANAGE_ACTION_
{
	PD_INSERT,
	PD_REMOVE,
	POE_PRIORITY_CHANGE,/*接入的设备优先级变化*/
	POE_POWER_CHANGE/*接入的PD设备功率发生变化*/
}PSE_POWER_MANAGE_ACTION;

typedef enum _POE_INTERFACE_MODE_
{
	POE_SIGNAL,
	POE_SPARE,
}POE_INTERFACE_MODE;

typedef enum _POE_DETECT_TYPE_
{
	NO_DETECT,
	LEGACY_CAPACITIVE_DETECT,
	FOUR_PIN_DETECT,
	FOUR_PIN_DETECT_LEGACY,
	TWO_PIN_DETECT,
	TWO_PIN_DETECT_LEGACY,
}POE_DETECT_TYPE;

typedef enum _POE_POWER_MODE_
{
	POE_POWER_MODE_NORMAL,
	POE_HIGH_POWER_MODE,
}POE_POWER_MODE;

typedef enum _POE_POWER_UP_MODE_
{
	POE_POWER_UP_NORMAL,
	POE_POWER_UP_HIGH,
	POE_POWER_UP_PRE_AT,
}POE_POWER_UP_MODE;

struct poe_time_s
{
    int year;   
    int month;  
    int day;    
    int hh;  
    int mm;  
    int what_day;
};

struct poe_time_abs_peri_s
{
    int flag;   
    struct poe_time_s start; 
    struct poe_time_s end;   
};

struct poe_time_range_info_s
{
    char name[16];  
    int index;      
    int bind_count;
    struct poe_time_abs_peri_s abs_time;  
    struct poe_time_abs_peri_s periodic_time; 
};
struct poe_time_cfg_s 
{
    int poe_based_time_valid;
	int time_range_index;
	int is_poe_deployed;
};
typedef struct poe_intf_db_s{

	unsigned int netif_index; 
	unsigned char poe_mode;//signal or spare
	unsigned int poe_id;	
	unsigned char admin; //enable disable
	unsigned char detect_status;
	unsigned char operate_status;
	unsigned char pd_type;
	unsigned char pd_class; 
	unsigned char priority;
	unsigned char legacy_dect;
	unsigned int max_power;
	
	unsigned int power_user;
	unsigned int current;
	unsigned int voltage;
	unsigned int temperature;
	unsigned char power_threshold_type;
	unsigned char max_power_admin_flag;
	unsigned char power_up_mode; /*high-power mode or normal*/
	struct poe_time_cfg_s time;
 
}poe_intf_db_t;

typedef struct poe_pse_db_s
{
	unsigned int pse_id;
//	unsigned int type;
	unsigned int slot_no;
	unsigned char admin;
	unsigned int pse_type;
//	unsigned int legacy_dect;
	unsigned int max_power;
	unsigned int available;
	unsigned int current;
	unsigned int voltage;
	unsigned int mode;	/* poe power suply management mode， auto or manual */
	unsigned int guardBand;//剩余保证电压
}poe_pse_db_t;


int npd_get_poeport_by_global_index(unsigned int netif_index, unsigned char *poePort);
void npd_create_poe_intf
(
	unsigned int netif_index
) ;
int npd_poe_init(void);

void* npd_poe_poll_thread(void);

extern int nam_poe_port_status_get(unsigned char portNum, unsigned char *portSta, unsigned char *errType, unsigned char *pdClass, unsigned char *pdType);
extern int  nam_poe_total_power_allocate_get(int *total_power_allocate, int *available_power);
extern int nam_poe_port_extended_config_get(unsigned char portNum, unsigned char *powerMode, unsigned char *violaType, int *maxPower, unsigned char *portPriority);
extern int nam_poe_port_poe_endis_set(unsigned char portNum, unsigned char isEnable);
extern int nam_poe_maxpower_threshold_set(unsigned char portNum, unsigned int maxPower);
extern int nam_poe_port_poe_endis_set(unsigned char portNum, unsigned char isEnable);
extern int nam_poe_port_pair_set(unsigned char portNum, unsigned char pairType);
extern int nam_poe_power_threshold_set(unsigned char portNum, unsigned char powerThresholdType);
extern int nam_poe_detect_type_set(unsigned char portNum, unsigned char detectType);
extern int nam_poe_port_config_get(unsigned char portNum, unsigned char *isEnable, unsigned char *autoMode, unsigned char *detectType, unsigned char *classType, unsigned char *disconnType, unsigned char *pairType);
extern int nam_poe_port_measure_get(unsigned char portNum, int *voltage, int *poeCurrent, int *temperature, int *power);
extern int nam_poe_power_manage_config_get(unsigned char mpss, unsigned char *powerManaMode, int *totalPower, int *guardBand);
extern int nam_poe_port_priority_set(unsigned char portNum, unsigned char portPriority);
extern int nam_poe_port_err_count_get(unsigned char portNum, unsigned char *absent_count, unsigned char *overload_count, unsigned char *short_count, unsigned char *denied_count, unsigned char *invalid_count);

#endif
#endif


