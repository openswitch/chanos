
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd/nbm/npd_cplddef.h"

#include "board/ts_product_feature.h"

#include "ds5652_board_feature.c"



board_spec_fix_param_t   *ds_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_DS5652] = &ds5652_board_spec_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

struct family_spec_fix_param_s ds5600_spec_family_param =
{
	.family_type = FAMILY_TYPE_DS5600,
	.board_spec_param_arr = ds_series_board_spec_param_arr,
};

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_DS5600] = &ds5600_spec_family_param
};


#ifdef __cplusplus

}
#endif

