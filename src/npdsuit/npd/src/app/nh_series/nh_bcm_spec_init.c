
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "bcm/stat.h"
#include "bcm/stack.h"
#include "bcm/trunk.h"
#include "bcm/vlan.h"
#include "bcm/port.h"
#include "bcm/mcast.h"
#include "bcm/ipmc.h"
#include "bcm/l2.h"
#include "bcm/error.h"
#include "soc/drv.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "nbm/nbm_api.h"

#include "dbus/npd/npd_dbus_def.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd/nbm/npd_cplddef.h"
#include "nh_product_feature.h"

#include "nh_product_init.h"

#include "nh_product_info.h"

#include "nh_tseries_conn.c"
#include "tsm9002_feature.c"
#include "tgm9048_feature.c"
#include "tsm9024fc_feature.c"
#include "tgm9024_feature.c"
#include "txm9004_feature.c"
#include "g96sup_feature.c"

board_spec_fix_param_t   *nh_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_TSM9002] = &tsm9002_spec_param,
    [PPAL_BOARD_TYPE_TGM9048] = &tgm9048_spec_param,
    [PPAL_BOARD_TYPE_TSM9024FC] = &tsm9024fc_spec_param,
    [PPAL_BOARD_TYPE_TGM9024] = &tgm9024_spec_param,
    [PPAL_BOARD_TYPE_TXM9004] = &txm9004_spec_param,   
    [PPAL_BOARD_TYPE_G96SUP] = &g96sup_spec_param,  
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

/* family feature */
struct family_spec_fix_param_s t9k_family_spec_param =
{
	.family_type = FAMILY_TYPE_T9000,
	.board_spec_param_arr = nh_series_board_spec_param_arr,
};


struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_T9000] = &t9k_family_spec_param,
};

#ifdef __cplusplus

}
#endif

