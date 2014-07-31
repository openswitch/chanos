
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"


#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd_log.h"
#include "npd/nbm/npd_cplddef.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "nh_product_feature.h"

#include "nh_product_init.h"

#include "nh_product_info.h"
#include "board/ts_product_feature.h"

#include "cgm9048_feature.c"
#include "g9604x_feature.c"
#include "cgm9048s_feature.c"

board_spec_fix_param_t   *nh_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_CGM9048] = &cgm9048_spec_param,
    [PPAL_BOARD_TYPE_G9604X] = &g9604x_spec_param,
    [PPAL_BOARD_TYPE_CGM9048S] = &cgm9048s_spec_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

struct family_spec_fix_param_s t9k_spec_family_param =
{
	.family_type = FAMILY_TYPE_T9000,
	.board_spec_param_arr = nh_series_board_spec_param_arr,
};

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_T9000] = &t9k_spec_family_param
};


#ifdef __cplusplus

}
#endif

