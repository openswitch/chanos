#ifndef __NPD_AMAPI_H__
#define __NPD_AMAPI_H__

#include "lib/netif_index.h"
#include "lib/npd_bitop.h"
#include "lib/common_api.h"
#include "lib/npd_database.h"
#include "lib/name_hash_index.h"
#include "lib/chassis_man_app.h"
#include "quagga/thread.h"
#include "quagga/log.h"
#include "tipc_api/tipc_api.h"

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include "npd/product_conn.h"
#include "npd/product_feature.h"
#include "npd/npd_c_slot.h"
#include "npd/os.h"
#include "npd/npd_netif_event.h"
#include "npd/npd_pkt_list.h"
#include "npd/npd_log.h"
#include "npd/jhash.h"
#include "npd/ipv6.h"

#include "npd/common/product.h"
#include "npd/common/board.h"
#include "npd/common/eth_port.h"
#include "npd/common/trunk.h"
#include "npd/common/switch_port.h"
#include "npd/common/vlan.h"
#include "npd/common/fdb.h"
#include "npd/common/intf.h"
#include "npd/common/arpsnoop.h"
#include "npd/common/route.h"
#include "npd/common/packet.h"
#include "npd/common/ndiscsnoop.h"
#include "npd/common/mirror.h"
#include "npd/common/qosacl.h"
#include "npd/common/poe.h"
#include "npd/common/stp.h"
#include "npd/common/dhcpsnoop.h"
#include "npd/common/igmp.h"
#include "npd/common/capwap.h"
#include "npd/common/tunnel.h"
#include "npd/common/vrrp.h"
#include "npd/common/security.h"

#ifndef _BOOL_TEST
#define _BOOL_TEST 1
typedef unsigned char boolean;
#endif

#ifndef _1K
#define 	_1K		1024
#endif

/* virtual eth-port index for CPU & HSP */
#define CPU_PORT_VINDEX	(0)

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

#define INDEX_ALLOCATED	0x7FFE
#define INDEX_END_OF_FREE	0x7FFF

typedef unsigned int index_elem;

typedef struct _tbl_index_s_ {
	unsigned int	max;
	unsigned int	available;
	index_elem		free;
	index_elem		(*table)[];	
}tbl_index_t;

tbl_index_t *nam_index_create
(
	unsigned int max
);

void nam_index_destroy
(
	tbl_index_t *number
);

unsigned int nam_index_get
(
	tbl_index_t   *number,
	index_elem	  val	
);

unsigned int nam_index_alloc
(
	tbl_index_t *number, 
	index_elem *val
);

unsigned int nam_index_free
(
	tbl_index_t *number, 
	index_elem 	val
);

/*******************************************************************************
* nam_thread_create
*
* DESCRIPTION:
*       Create Linux thread and start it.
*
* INPUTS:
*       name    - task name, valid when accessChip is TRUE
*       start_addr - task/thread Function to execute
*       arglist    - pointer to list of parameters for task/thread function
*	  accessChip - this thread/task need access chip register or not
*	  needJoin - this thread need pthread_join operation
*
* OUTPUTS:
*
* RETURNS:
*       0   - on success
*       1 - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
void nam_thread_create
(
	char	*name,
	unsigned int (*entry_point)(void*),
	void	*arglist,
	unsigned char accessChip,
	unsigned char needJoin
);

#endif

