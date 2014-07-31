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



long ds_series_linecard_local_conn_init(int product_type)
{

    return 0;  
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

	return 0;
}

#ifdef __cplusplus
}
#endif

