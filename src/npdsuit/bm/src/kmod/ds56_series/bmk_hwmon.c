
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include "bmk_hwmon.h"

int bm_set_module_temp_high_limit(int * high_limit)
{
	return -1;
}

int bm_set_module_temp_low_limit(int * low_limit)
{
	return -1;
}

int bm_set_core_temp_high_limit(int * high_limit)
{
	return -1;
}

int bm_set_core_temp_low_limit(int * low_limit)
{
	return -1;	
}

int bm_set_temp_threshold(int type, int value)
{
	int ret = 0;

	return 0;	
}


int bm_get_temp_info(temp_info_args * temp_info)
{
	return 0;
}

int bm_get_temp_alarm(int * alarm_state)
{
	return 0;
}

