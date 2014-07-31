
#ifndef _WS_SYSINFO_H
#define _WS_SYSINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_nm_status.h"

#define DEVINFO_PRODUCT_NAME "/devinfo/product_name"

#define BM_IOC_MAGIC 0xEC
#define BM_IOC_ENV_EXCH		_IOWR(BM_IOC_MAGIC,10,boot_env_t_st)
#define SAVE_BOOT_ENV 	1
#define GET_BOOT_ENV	0
typedef struct boot_env
{	
	char name[64];	
	char value[128];	
	int operation;
}boot_env_t_st;


struct sys_HW_info
{
	int slot;
	char productname[20];
	char hw_status[10];
	char moduleName[20];
	char SN[25];
	char hw_ver[10];
	int ext_slotnum;
	struct sys_HW_info * next;
};
typedef struct sys_HW_info hw_info;

struct sys_ver
{
	unsigned int product_id;
	unsigned int sw_version;
	char *product_name;
	char *base_mac;
	char *serial_no;
	char *swname;
	char sw_version_str[128];// new add by shaojunwu 2009-04-27
    char sw_name[128];
	char sw_product_name[128];
	char sw_mac[128];
	char sw_serial_num[128];
};

struct sys_img
{
char * boot_img;

};
/***********************/

#ifndef MIN
#define MIN(a,b) ( (a)<(b) ? (a):(b) ) 
#endif

#define MAX_CPU 16
#define CPU_STATUS 8
#define DEFAULT_REFRESH 1
#define CPU_USAGE_IDEL 3

#define MAX_LEN 2048 
#define VALUE_LEN 32
#define M_SIZE 1024

#define SUCCESS 0
#define SYS_ERR 1
#define PARA_ERR 2

static char* cp_cpu_info = "/proc/stat";
static char* cp_mem_info = "/proc/meminfo";
//static char* LANGUAGE_PATH = "../htdocs/text/en-ch.txt";

typedef struct st_Cpus_Status
{   
    unsigned int cpu_no;
    float ar_cpu_usage[MAX_CPU][CPU_STATUS];
}CPUS_STU,*P_CPUS_STU;

typedef struct st_Cpu_Temperature
{    
    unsigned int un_tem_core;    
    unsigned int un_tem_sface;
}CPU_TEM,*P_CPU_TEM;

typedef struct st_Mem_Status
{    
    unsigned int un_mem_total;    
    unsigned int un_mem_used;    
    unsigned int un_mem_free;    
    unsigned int un_mem_buffers;
}MEM_STU,*P_MEM_STU;

typedef struct st_Sys_Info
{    
    P_CPUS_STU pst_cpus_status;    
    P_CPU_TEM pst_cpu_temperature;    
    P_MEM_STU pst_mem_status;
}SYS_INFO,*P_SYS_INFO;



//***************************************
extern int show_sys_ver(struct sys_ver *SysV);
#if 0

extern int show_hardware_configuration();

extern int get_bootimg_name(char* imgname);

extern int show_system_bootimg(struct sys_img *bootimg);

extern int  get_product_id();
extern int show_sys_hwconf(hw_info * sys_head,int *slot_number);//if return SN equals with "-",ex_slotnum print "-",return 0 means successful,-1 means fail
extern void free_hw_head(hw_info * hw_head);
extern char* get_token(char *str_tok, unsigned int *pos);
extern int get_cpu_status(unsigned int *cpu_times, unsigned int *count);
extern int get_cpu_usage(P_SYS_INFO pst_sys_info, unsigned int refresh );
extern int get_mem_usage( P_SYS_INFO pst_sys_info );
extern int get_cpu_temperature( P_SYS_INFO pst_sys_info );
extern int get_logo(char * logo);

extern int calc_cpu_rate( unsigned int time_old[MAX_CPU][CPU_STATUS], 
						unsigned int time_now[MAX_CPU][CPU_STATUS],
						int cpu_num, unsigned int *cpu_rate,
						unsigned int *cpu_rate_all );
#endif



#endif

