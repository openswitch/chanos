#ifndef __COMMON_PRODUCT_H__
#define __COMMON_PRODUCT_H__



extern int npd_env_monitor_flag;


#define ENVIROMENT_MONITOR_ENABLE	(npd_env_monitor_flag)

#define FDB_AGING_MIN		60
#define FDB_AGING_MAX		600

extern int npd_startup_end;

#endif

