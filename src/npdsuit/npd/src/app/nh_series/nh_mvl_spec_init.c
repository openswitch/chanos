
#ifdef __cplusplus
extern "C"
{
#endif

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "nbm/nbm_api.h"

#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd/nbm/npd_cplddef.h"

#include "nh_product_feature.h"

#include "nh_product_init.h"

#include "nh_product_info.h"

#include "nh_lion_conn.c"
#include "as9612x_feature.c"

board_spec_fix_param_t   *nh_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_AS9612X] = &as9612x_spec_param,
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

