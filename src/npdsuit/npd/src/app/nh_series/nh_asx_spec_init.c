
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
#include "nh_product_feature.h"

#include "nh_product_init.h"

#include "nh_product_info.h"

/*The phyId of port 1 2 3 4.now only 4 ports, fix me*/
unsigned short portNum2phyId[4] = {-1, -1, -1, -1};

#include "asx6602_feature.c"
#include "asx9604l_feature.c"


board_spec_fix_param_t   *as_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_spec_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};

board_spec_fix_param_t   *nh_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_ASX9604L] = &asx9604l_spec_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};


/* family feature */
struct family_spec_fix_param_s as6600_spec_family_param =
{
	.family_type = FAMILY_TYPE_AS6600,
	.board_spec_param_arr = as_series_board_spec_param_arr,
};

/* family feature */
struct family_spec_fix_param_s t9000_spec_family_param =
{
	.family_type = FAMILY_TYPE_T9000,
	.board_spec_param_arr = nh_series_board_spec_param_arr,
};

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_AS6600] = &as6600_spec_family_param,
	[FAMILY_TYPE_T9000]	= &t9000_spec_family_param,
};

#ifdef __cplusplus

}
#endif

