
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "nbm/nbm_api.h"

#include "ds_product_common_info.h"
#include "ds_product_info.h"

#include "ds_series_conn.c"


#include "ds5600_conn.c"
#include "ds5600_feature.c"

#include "ds5652_board_conn.c"
#include "ds5652_board_param.c"


unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;



/* us3k series   */

product_fix_param_t *ds5600_series_product_param_arr[] =
{
    [PRODUCT_DS5600] = &ds5600_fix_param,
    [PRODUCT_NH_MAX_NUM] = NULL,    		
};


product_conn_type_t *ds5600_series_product_conn_arr[] =
{
    [PRODUCT_DS5600] = &ds5600_product_conn,
    [PRODUCT_NH_MAX_NUM] = NULL,    				
};

struct board_conn_type_s *ds5600_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_DS5652] = &ds5652_board_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    				
};

struct board_fix_param_t *ds5600_series_board_param_arr[] =
{
    [PPAL_BOARD_TYPE_DS5652] = &ds5652_board_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};


/* us3k family feature */
family_common_fix_param_t ds5600_family_param =
{
	.family_type = FAMILY_TYPE_DS5600,
	.product_param_arr =  ds5600_series_product_param_arr,
	.product_conn_arr = ds5600_series_product_conn_arr,
	.board_conn_arr = ds5600_series_board_conn_arr,
	.board_conn_fullmesh_arr = ds5600_series_board_conn_arr,
	.board_param_arr = ds5600_series_board_param_arr,
};

struct family_common_fix_param_s * family_type_arr[] = 
{
	[FAMILY_TYPE_DS5600] = &ds5600_family_param,	
};

extern struct family_spec_fix_param_s *family_spec_type_arr[];
    

long ds_product_info_init(void)
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

long ds_product_param_read()
{
    /*open file "snros_product_param.cfg"*/

    /*read snros_product_param and init ppal_product_info, n_param and g_sysinfo_backboard and g_sysinfo_module*/
    return NPD_SUCCESS;
}

void product_init(void)
{
	productinfo.family_type = ds_family_type_get();
    productinfo.product_id = ds_product_hw_code_get();
    productinfo.local_module_id = ds_local_module_hw_code_get();
    productinfo.local_module_hw_version = ds_board_hw_version_get();
    productinfo.local_chassis_slot_no = ds_board_hw_slotno_get();
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;

    ds_product_info_init();

}

#ifdef __cplusplus

}
#endif

