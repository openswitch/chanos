#ifndef __NPD_NAMAPI_H__
#define __NPD_NAMAPI_H__

#include "lib/osinc.h"

#include "lib/netif_index.h"
#include "lib/npd_bitop.h"
#include "lib/common_api.h"
#include "npd/npd_netif_event.h"

#ifndef _BOOL_TEST
#define _BOOL_TEST 1
typedef unsigned char boolean;
#endif

#ifndef _1K
#define 	_1K		1024
#endif

/* virtual eth-port index for CPU & HSP */
#define CPU_PORT_VINDEX	(0)


#define ALIAS_NAME_SIZE 0x15

typedef enum nam_error_e {
	NAM_E_NONE = 0, 
	NAM_E_INTERNAL = -1, 
	NAM_E_MEMORY = -2,
	NAM_E_UNIT = -3, 
	NAM_E_PARAM = -4, 
	NAM_E_EMPTY = -5, 
	NAM_E_FULL = -6, 
	NAM_E_NOT_FOUND = -7, 
	NAM_E_EXISTS = -8, 
	NAM_E_TIMEOUT = -9, 
	NAM_E_BUSY = -10, 
	NAM_E_FAIL = -11, 
	NAM_E_DISABLED = -12, 
	NAM_E_BADID = -13, 
	NAM_E_RESOURCE = -14, 
	NAM_E_CONFIG = -15, 
	NAM_E_UNAVAIL = -16, 
	NAM_E_INIT = -17, 
	NAM_E_PORT = -18 
} nam_error_t;







/**********************************************************************************************/
/*																										   */
/*										FDB Entry Info     													   */
/*																										   */
/**********************************************************************************************/
#define 	MAC_ADDRESS_LEN	 6

	
/*
 *
 * VIDX :
 *		value 0~0xEFF means multicast group table index for Multicast( MAC or IPv4/v6) entry.
 *		value 0xFFF means multicast packet is forwarded according to VID group membership.
 *
 */
#include "npd/ipv6.h"





extern int npd_startup_end;
#endif

