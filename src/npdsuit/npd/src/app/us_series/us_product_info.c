
#ifdef __cplusplus
extern "C"
{
#endif

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_cplddef.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "os.h"
#include "npd_log.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"

#include "us_product_common_info.h"
#include "us_product_info.h"




void us3000_power_monitor(void)
{

}

void us3000_fan_monitor(void)
{

}

void us3000_led_state_operate(void)
{
	
}

void us3000_temp_monitor()
{

}

int us3000_pne_monitor(void)
{
    return 0;
}

long us3000_pne_monitor_start(void)
{
	return NPD_SUCCESS;
}

long us3000_power_man_param_init(power_param_t * param)
{
	return 0;	
}

long us3000_fan_param_init(fan_param_t * param)
{
	return 0;
}

int us3000_read_sysinfo_file(char * filename, char * buf, int len)
{
	return NPD_SUCCESS;	
}



void us3000_int_power_handler(unsigned char int_value, unsigned char add_value)
{

}

 
#ifdef __cplusplus
}
#endif


