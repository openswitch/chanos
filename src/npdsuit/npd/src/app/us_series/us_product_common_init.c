
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


#include "us_product_common_info.h"
#include "us_product_info.h"

#include "us_3000_conn.c"
#include "us_3000_feature.c"

#include "us_3000_board_conn.c"
#include "us_3000_board_param.c"

#include "us_4628gs_board_conn.c"
#include "us_4628gs_board_param.c"

#include "us_4628pwr_board_conn.c"
#include "us_4628pwr_board_param.c"


#include "us_4629gx_board_conn.c"
#include "us_4629gx_board_param.c"

#include "us_4629pwrl_board_conn.c"
#include "us_4629pwrl_board_param.c"

#include "us_sub_4sfp_board_conn.c"
#include "us_sub_4sfp_board_param.c"

#include "us_sub_sfp_plus_board_conn.c"
#include "us_sub_sfp_plus_board_param.c"


unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;



/* us3k series   */

product_fix_param_t *us3k_series_product_param_arr[] =
{
    [PRODUCT_US3000] = &us3000_fix_param,
    [PRODUCT_NH_MAX_NUM] = NULL,    		
};


product_conn_type_t *us3k_series_product_conn_arr[] =
{
    [PRODUCT_US3000] = &us3000_product_conn,
    [PRODUCT_NH_MAX_NUM] = NULL,    				
};

struct board_conn_type_s *us3k_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_US_3000] = &us3000_board_board_conn,
    [PPAL_BOARD_TYPE_US_4628GS] = &us4628gs_board_board_conn,    
    [PPAL_BOARD_TYPE_US_4629GX] = &us4629gx_board_board_conn,
    [PPAL_BOARD_TYPE_US_4629GX_PWR] = &us4628pwr_board_board_conn,    
    [PPAL_BOARD_TYPE_US_4629GX_PWRL] = &us4629pwrl_board_board_conn,    
    [PPAL_BOARD_TYPE_US_SUB_4SFP] = &us_sub_4sfp_board_board_conn,    
    [PPAL_BOARD_TYPE_US_SUB_SFP_PLUS] = &us_sub_sfp_plus_board_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    				
};

struct board_fix_param_t *us3k_series_board_param_arr[] =
{
    [PPAL_BOARD_TYPE_US_3000] = &us3000_board_param,
    [PPAL_BOARD_TYPE_US_4628GS] = &us4628gs_board_param,    
    [PPAL_BOARD_TYPE_US_4629GX] = &us4629gx_board_param,
    [PPAL_BOARD_TYPE_US_4629GX_PWR] = &us4628pwr_board_param,    
    [PPAL_BOARD_TYPE_US_4629GX_PWRL] = &us4629pwrl_board_param,    
    [PPAL_BOARD_TYPE_US_SUB_4SFP] = &us_sub_4sfp_board_param,
    [PPAL_BOARD_TYPE_US_SUB_SFP_PLUS] = &us_sub_sfp_plus_board_param,    
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};


/* us3k family feature */
family_common_fix_param_t us3k_family_param =
{
	.family_type = FAMILY_TYPE_US3000,
	.product_param_arr =  us3k_series_product_param_arr,
	.product_conn_arr = us3k_series_product_conn_arr,
	.board_conn_arr = us3k_series_board_conn_arr,
	.board_conn_fullmesh_arr = us3k_series_board_conn_arr,
	.board_param_arr = us3k_series_board_param_arr,
};

struct family_common_fix_param_s * family_type_arr[] = 
{
	[FAMILY_TYPE_US3000] = &us3k_family_param,	
};

extern struct family_spec_fix_param_s *family_spec_type_arr[];
    

long us_product_info_init(void)
{
	struct family_common_fix_param_s* sys_family_param = family_type_arr[productinfo.family_type];
	struct family_spec_fix_param_s* sys_family_spec_param = family_spec_type_arr[productinfo.family_type];

	npd_syslog_dbg("product is family type %d.\n", productinfo.family_type);

    register_product_param_array(sys_family_param->product_param_arr);
    register_product_conn_array(sys_family_param->product_conn_arr);
    register_board_conn_array(sys_family_param->board_conn_arr);
	register_board_conn_fullmesh_array(sys_family_param->board_conn_fullmesh_arr);
    register_board_param_array(sys_family_param->board_param_arr);

    register_board_spec_param_array(sys_family_spec_param->board_spec_param_arr);
	
    return 0;
   
}

long us_product_param_read()
{
    /*open file "snros_product_param.cfg"*/

    /*read snros_product_param and init ppal_product_info, n_param and g_sysinfo_backboard and g_sysinfo_module*/
    return NPD_SUCCESS;
}

void product_init(void)
{
	productinfo.family_type = us_family_type_get();
    productinfo.product_id = us_product_hw_code_get();
    productinfo.local_module_id = us_local_module_hw_code_get();
    productinfo.local_module_hw_version = us_board_hw_version_get();
    productinfo.local_chassis_slot_no = us_board_hw_slotno_get();
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;

    us_product_info_init();

}

#ifdef __cplusplus

}
#endif

