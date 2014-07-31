
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
#include "as_product_feature.h"

#include "as_product_init.h"

#include "as_product_info.h"

#include "as_series_conn.c"
#include "as6603_conn.c"
#include "as6603_feature.c"
#include "ax63ge24_conn.c"
#include "ax63ge48_conn.c"
#include "ax63ge24i_conn.c"
#include "ax63ge24_feature.c"
#include "ax63ge48_feature.c"
#include "ax63ge24i_feature.c"
#include "as_3028_feature.c"
#include "as_3028_conn.c"
#include "asx6602_conn.c"
#include "asx6602_feature.c"


unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;

/* t9k series */
product_fix_param_t *as_series_product_param_arr[] =
{
	[PRODUCT_T9003] = &t9003_fix_param,    	
    [PRODUCT_NH_3028] = &as3028_fix_param,
    [PRODUCT_AS6603] = &as6603_fix_param,
    [PRODUCT_AX8603] = &ax8603_fix_param,
    [PRODUCT_NH_MAX_NUM] = NULL,
};

board_fix_param_t   *as_series_board_param_arr[] = 
{
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_param,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_param,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_param,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_param,
    /*[PPAL_BOARD_TYPE_NH_3028] = &as_3028_param,*/
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};


product_conn_type_t *as_series_product_conn_arr[] =
{
	[PRODUCT_T9003] = &t9003_product_conn,    
    [PRODUCT_NH_3028] = &as_3028_product_conn,
    [PRODUCT_AS6603] = &as6603_product_conn,
    [PRODUCT_AX8603] = &as6603_product_conn,
    [PRODUCT_NH_MAX_NUM] = NULL,    
};


struct board_conn_type_s *as_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_board_conn,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_board_conn,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_board_conn,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};


struct board_conn_type_s as_series_board_conn_arr_fullmesh[] = 
{
};

/* family feature */
family_fix_param_t as6600_family_param =
{
	.family_type = FAMILY_TYPE_AS6600,
	.product_param_arr =  as_series_product_param_arr,
	.board_param_arr = as_series_board_param_arr,
	.product_conn_arr = as_series_product_conn_arr,
	.board_conn_arr = as_series_board_conn_arr,
	.board_conn_fullmesh_arr = as_series_board_conn_arr,
};

product_fix_param_t *as3000_series_product_param_arr[] =
{
    [PRODUCT_NH_3028] = &as3028_fix_param,
    [PRODUCT_NH_MAX_NUM] = NULL,
};

board_fix_param_t   *as3000_series_board_param_arr[] = 
{
    [PPAL_BOARD_TYPE_NH_3028] = &as_3028_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,
};


product_conn_type_t *as3000_series_product_conn_arr[] =
{
    [PRODUCT_NH_3028] = &as_3028_product_conn,
    [PRODUCT_NH_MAX_NUM] = NULL,    
};


struct board_conn_type_s *as3000_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_NH_3028] = &as_3028_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};


struct board_conn_type_s as3000_series_board_conn_arr_fullmesh[] = 
{
};

family_fix_param_t as3000_family_param =
{
	.family_type = FAMILY_TYPE_AS3000,
	.product_param_arr =  as3000_series_product_param_arr,
	.board_param_arr = as3000_series_board_param_arr,
	.product_conn_arr = as3000_series_product_conn_arr,
	.board_conn_arr = as3000_series_board_conn_arr,
	.board_conn_fullmesh_arr = as3000_series_board_conn_arr,
};

struct family_fix_param_s * family_type_arr[] = 
{
	[FAMILY_TYPE_AS6600] = &as6600_family_param,
	[FAMILY_TYPE_AS3000] = &as3000_family_param,
};


long as_product_info_init(void)
{
	struct family_fix_param_s* sys_family_param = family_type_arr[productinfo.family_type];

	npd_syslog_dbg("product is family type %d.\n", productinfo.family_type);

    register_product_param_array(sys_family_param->product_param_arr);
    register_board_param_array(sys_family_param->board_param_arr);
    register_product_conn_array(sys_family_param->product_conn_arr);
    register_board_conn_array(sys_family_param->board_conn_arr);
	register_board_conn_fullmesh_array(sys_family_param->board_conn_fullmesh_arr);
	
	/* 	
    register_product_param_array(nh_series_product_param_arr);
    register_board_param_array(nh_series_board_param_arr);
    register_product_conn_array(nh_series_product_conn_arr);
    register_board_conn_array(nh_series_board_conn_arr);
	register_board_conn_fullmesh_array(nh_series_board_conn_arr);
	*/
    //register_board_conn_fullmesh_array(nh_series_board_conn_arr_fullmesh);
    return 0;
   
}

long as_product_param_read()
{
    /*open file "snros_product_param.cfg"*/

    /*read snros_product_param and init ppal_product_info, n_param and g_sysinfo_backboard and g_sysinfo_module*/
    return NPD_SUCCESS;
}

void product_init(void)
{
    productinfo.family_type = as_family_type_get();
    productinfo.product_id = as_product_hw_code_get();
    productinfo.local_module_id = as_local_module_hw_code_get();
    productinfo.local_module_hw_version = as_board_hw_version_get();
    productinfo.local_chassis_slot_no = as_board_hw_slotno_get();
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;

    as_product_info_init();

}

#ifdef __cplusplus

}
#endif

