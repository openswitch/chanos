
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
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd_log.h"
#include "npd/nbm/npd_cplddef.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"

#include "board/ts_product_feature.h"

#include "us_3000_board_feature.c"
#include "us_4628gs_board_feature.c"
#include "us_4628pwr_board_feature.c"
#include "us_4629gx_board_feature.c"
#include "us_4629pwrl_board_feature.c"
#include "us_sub_4sfp_board_feature.c"
#include "us_sub_sfp_plus_board_feature.c"


board_spec_fix_param_t   *us_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_US_3000] = &us3000_board_spec_param,
    [PPAL_BOARD_TYPE_US_4628GS] = &us4628gs_board_spec_param,    
    [PPAL_BOARD_TYPE_US_4629GX] = &us4629gx_board_spec_param,
    [PPAL_BOARD_TYPE_US_4629GX_PWR] = &us4628pwr_board_spec_param,
    [PPAL_BOARD_TYPE_US_4629GX_PWRL] = &us4629pwrl_board_spec_param,
    [PPAL_BOARD_TYPE_US_SUB_SFP_PLUS] = &us_sub_sfp_plus_board_spec_param,
    [PPAL_BOARD_TYPE_US_SUB_4SFP] = &us_sub_4sfp_board_spec_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

struct family_spec_fix_param_s us3k_spec_family_param =
{
	.family_type = FAMILY_TYPE_US3000,
	.board_spec_param_arr = us_series_board_spec_param_arr,
};

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_US3000] = &us3k_spec_family_param
};


#ifdef __cplusplus

}
#endif

