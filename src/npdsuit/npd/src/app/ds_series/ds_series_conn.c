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
#include "cpss/dxCh/dxChxGen/cpssDxChTypes.h"
#include "cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h"
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include "cpss/generic/cscd/cpssGenCscd.h"
#include "cpss/generic/bridge/cpssGenBrgFdb.h"
#include "cpss/generic/trunk/cpssGenTrunkTypes.h"
#include "cpss/generic/nst/cpssNstTypes.h"
#include "cpss/generic/pcl/cpssPcl.h"
#include "gtOs/gtGenTypes.h"
#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "nbm/nbm_api.h"
#include "npd_log.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "board/ts_product_feature.h"
extern int nam_fdb_table_delete_dynamic_all(int unit);
#define TRUNK_PORT_PCL_PORTA_2_PORTB 1
#define TRUNK_PORT_PCL_PORTB_2_PORTA 2
#define TRUNK_PORT_PCL_BIDIRECTION   3
#define START_MOD_ID 0
#define CSCD_PORT_PCL_ID 128
#define AX24_RIGHT_SLOT_TRUNK 127
#define AX24_LEFT_SLOT_TRUNK 126
#define SDK_DIFF_TRUNK	121

CPSS_PORTS_BMP_STC as_st_bmp[2];

long ds_series_linecard_local_conn_init(int product_type)
{
    memset(&as_st_bmp, 0, sizeof(as_st_bmp));
}

long ds_series_linecard_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	return 0;
}

long ds_series_linecard_system_conn_deinit(
    int product_type, 
    int delete_board_type, 
    int delete_slotid    
    )
{
}

int npd_get_modport_tid_by_global_index(unsigned int globle_index, int *tid, 
												unsigned char *mod, unsigned char *port)
{
	int ret = 0;
 	unsigned int eth_g_index[8];
	unsigned int eth_count = 0;
	int count;
	unsigned int peer_slot, peer_type;
	
	*tid = 0;
	if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(globle_index))
	{
	    *tid = npd_netif_trunk_get_tid(globle_index);
		*mod = 0;
        *port = 0;
		return 0;
	}
	else
	{
    	ret = npd_get_modport_by_global_index(globle_index, mod, port);
        if (0 != ret)
        {
            npd_syslog_dbg("eth_index %#x get asic port failed for ret %d\n",globle_index,ret);
            return -1;
        }
		*tid = 0;
    }
	
	return 0;
}

#ifdef __cplusplus
}
#endif

