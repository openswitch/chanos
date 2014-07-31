
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include "bmk_read_eeprom.h"
#include "bmk_write_eeprom.h"

#include "bmk_hwmon.h"

#define MAX1617_DEV_ADDR	0x18

// command register addr
#define REG_RLTS	0x00	//Read local temperature: returns latest temperature
#define REG_RRTE	0x01	//Read remote temperature: returns latest temperature
#define REG_RSL		0x02	//Read status byte (flags, busy signal)
#define REG_RCL		0x03	//Read configuration byte
#define REG_RCRA	0x04	//Read conversion rate byte
#define REG_RLHN	0x05	//Read local THIGH limit
#define REG_RLLI	0x06	//Read local TLOW limit
#define REG_RRHI	0x07	//Read remote THIGH limit
#define REG_RRLS	0x08	//Read remote TLOW limit
#define REG_WCA		0x09	//Write configuration byte
#define REG_WCRW	0x0A	//Write conversion rate byte
#define REG_WLHO	0x0B	//Write local THIGH limit
#define REG_WLLM	0x0C	//Write local TLOW limit
#define REG_WRHA	0x0D	//Write remote THIGH limit
#define REG_WRLN	0x0E	//Write remote TLOW limit
#define REG_OSHT	0x0F	//One-shot command (use send-byte format)

// alarm type mask
#define MASK_LOCAL_HILIM	0x40			
#define MASK_LOCAL_LOLIM	0x20
#define MASK_REMOTE_HILIM	0x10
#define MASK_REMOTE_LOLIM	0x08	
#define MASK_OPEN_CIRCUIT	0x04




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

