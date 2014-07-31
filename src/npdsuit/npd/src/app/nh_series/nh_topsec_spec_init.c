
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

#include "fw9001_feature.c"
#include "dummy_feature.c"

board_spec_fix_param_t   *nh_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_TSM9002] = &dummy_spec_param,
    [PPAL_BOARD_TYPE_TGM9048] = &dummy_spec_param,
    [PPAL_BOARD_TYPE_TSM9024FC] = &dummy_spec_param,
    [PPAL_BOARD_TYPE_TGM9024] = &dummy_spec_param,
    [PPAL_BOARD_TYPE_FW9001] = &fw9001_spec_param,   
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


