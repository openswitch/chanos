#ifndef _BMK_HWMON_H_
#define _BMK_HWMON_H_

#include "bmk_main.h"

int bm_get_temp_info(temp_info_args * temp_info);
int bm_get_temp_alarm(int* alarm_state);
int bm_set_temp_threshold(int type, int value);

#endif//
