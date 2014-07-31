
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
#include "os.h"
#include "npd_log.h"
#include "npd/nbm/npd_cplddef.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "nh_product_common_info.h"
#include "as_product_info.h"

#include "as_series_conn.c"
#include "ax63ge24_feature.c"
#include "ax63ge48_feature.c"
#include "ax63ge24i_feature.c"


board_spec_fix_param_t   *as_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_spec_param,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_spec_param,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_spec_param,
    /*[PPAL_BOARD_TYPE_NH_3028] = &as_3028_param,*/
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

/* family feature */
family_spec_fix_param_t as6600_spec_family_param =
{
	.family_type = FAMILY_TYPE_AS6600,
	.board_spec_param_arr = as_series_board_spec_param_arr,
};
/*
board_fix_param_t   *as3000_series_board_param_arr[] = 
{
    [PPAL_BOARD_TYPE_NH_3028] = &as_3028_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};
family_spec_fix_param_t as3000_spec_family_param =
{
	.family_type = FAMILY_TYPE_AS3000,
	.board_spec_param_arr = as3000_series_board_param_arr,
};

*/

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_AS6600] = &as6600_spec_family_param,
	/*[FAMILY_TYPE_AS3000] = &as3000_spec_family_param,*/
};


#ifdef __cplusplus

}
#endif

