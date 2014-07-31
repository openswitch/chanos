#ifndef __NPD_SYSDEF_H__
#define __NPD_SYSDEF_H__



#define PORT_LINK_DOWN 0
#define PORT_LINK_UP    1

#define NOT_EXIST      0
#define ONLINE_INSERT  1
#define ONLINE_REMOVE  2

#define NPD_SUCCESS 0
#define NPD_FAIL -1

#define NPD_TRUE 	1
#define NPD_FALSE	0


/* BOARD means chassis board(slot) or box product */
#define NPD_ERR_SYSTEM_MAC  3

/*
  *	SW build number format: X.Y.Z.n or X.Y.Z build n
  *	individual number meas as:
  *		X - major version number
  *		Y - minor version number
  *		Z - compatible version number, which also means internal test release version
  *		n - build number, which increase linearly when X and Y not changed.
  *		     When X or Y change, n reset to 0, Z is irrelevant of n's increase.
  */
#define SW_MAJOR_VER(ver) ((ver) >> 28)
/* 4bit 31-28 ,Major version usually start from 1, */
#define SW_MINOR_VER(ver) (((ver) & 0x0FFFFFFF ) >> 21)
/* 7bit 27-21 ,Minor version,usually start from 0, */
#define SW_COMPATIBLE_VER(ver) (((ver) & 0x001FFFFF) >> 14)
/* 7bit 20-14, Compatible version, usually start from 0, */
#define SW_BUILD_VER(ver) ((ver) & 0x00003FFF )
/* 14bit 13-0  ,Build No, usually start from 0, */

#define SW_INT_VER(maj,min,comp,build) 		\
	((((maj) & 0x0F) << 28) + (((min) & 0x7F) << 21) + (((comp) & 0x7F) << 14) + ((build) & 0x3FFF))

/*
   Hardware version definition
All module will have version like below, 4 bits of PCB version and 4 bits of cpld version.
*/

#define HW_PCB_VER(ver) (((ver) & 0x0000FFFF) >> 8)
/* indicate pcb version number,start with 0,then 1, 2, 3, and so on. */
#define HW_CPLD_VER(ver) ((ver) & 0x000000FF)
/* indicate cpld version number, start withs 0, then 1,2,3, */

/*
If HW version is 0xFF, then cpld register is not avaible, 
*/
#define HW_VER_IGNORE 0xFF




#include "sysdef/macro_compile.h"
#include "sysdef/macro_syslog.h"
#include "sysdef/macro_eth.h"
#include "sysdef/macro_lag.h"
#include "sysdef/macro_sdk_driver.h"
#include "sysdef/macro_stp.h"
#include "sysdef/macro_qos.h"

#endif
