
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_cplddef.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"

#include "ds_product_common_info.h"
#include "ds_product_info.h"




void ds5600_power_monitor(void)
{

}

void ds5600_fan_monitor(void)
{

}

void ds5600_led_state_operate(void)
{
	
}

void ds5600_temp_monitor()
{

}

int ds5600_pne_monitor(void)
{
    return 0;
}

long ds5600_pne_monitor_start(void)
{
	return NPD_SUCCESS;
}

long ds5600_power_man_param_init(power_param_t * param)
{
	return 0;	
}

long ds5600_fan_param_init(fan_param_t * param)
{
	return 0;
}

int ds5600_read_sysinfo_file(char * filename, char * buf, int len)
{
	return NPD_SUCCESS;	
}



void ds5600_int_power_handler(unsigned char int_value, unsigned char add_value)
{

}

 
#ifdef __cplusplus
}
#endif


