
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "nbm/nbm_api.h"

#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd/nbm/npd_cplddef.h"
#include "npd/chasm_manage_proto.h"

#include "nh_product_feature.h"

#include "nh_product_init.h"
#include "nh_product_common_info.h"
#include "as_product_info.h"
#include "nh_product_info.h"

#include "t9003_conn.c"
#include "t9006_conn.c"
#include "g9006_conn.c"
#include "t9010_conn.c"
#include "t9014_conn.c"

#include "t9003_feature.c"
#include "t9006_feature.c"
#include "g9006_feature.c"
#include "t9010_feature.c"
#include "t9014_feature.c"

#include "tsm9002_conn.c"
#include "tsm9002_param.c"
#include "tgm9048_conn.c"
#include "tgm9048_param.c"
#include "tsm9024fc_conn.c"
#include "tsm9024fc_param.c"
#include "tgm9024_conn.c"
#include "tgm9024_param.c"
#include "txm9004_conn.c"
#include "txm9004_param.c"
#include "as6603_conn.c"
#include "as6603_feature.c"
#include "ax63ge24_conn.c"
#include "ax63ge24_param.c"
#include "ax63ge48_conn.c"
#include "ax63ge48_param.c"
#include "ax63ge24i_conn.c"
#include "ax63ge24i_param.c"
#include "cgm9048_conn.c"
#include "cgm9048_param.c"
#include "cgm9048s_conn.c"
#include "cgm9048s_param.c"
#include "g9604x_conn.c"
#include "g9604x_param.c"
#include "fw9001_conn.c"
#include "fw9001_param.c"
#include "as6612c_conn.c"
#include "as6612c_param.c"


#include "asx6602_conn.c"
#include "asx6602_param.c"
#include "asx9604l_conn.c"
#include "asx9604l_param.c"
#include "as9612x_conn.c"
#include "as9612x_param.c"
#include "g96sup_conn.c"
#include "g96sup_param.c"


unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;

/* t9k series */
product_fix_param_t *nh_series_product_param_arr[] =
{
    [PRODUCT_T9010] = &t9010_fix_param,
    [PRODUCT_T9006] = &t9006_fix_param,
    [PRODUCT_T9003] = &t9003_fix_param,    
    [PRODUCT_AS6603] = &as6603_fix_param, 
    [PRODUCT_T9014] = &t9014_fix_param,
    [PRODUCT_G9606] = &g9006_fix_param,
    
    [PRODUCT_NH_MAX_NUM] = NULL,
};


product_conn_type_t *nh_series_product_conn_arr[] =
{
    [PRODUCT_T9010] = &t9010_product_conn,
    [PRODUCT_T9006] = &t9006_product_conn,
	[PRODUCT_T9003] = &t9003_product_conn,
	[PRODUCT_AS6603] = &as6603_product_conn,
    [PRODUCT_T9014] = &t9014_product_conn,
	[PRODUCT_G9606] = &g9006_product_conn,	
    [PRODUCT_NH_MAX_NUM] = NULL,    
};

board_conn_type_t *nh_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_NH_NONE] = &none_board_type_conn,
    [PPAL_BOARD_TYPE_TSM9002] = &tsm9002_board_conn,
    [PPAL_BOARD_TYPE_TGM9048] = &tgm9048_board_conn,
    [PPAL_BOARD_TYPE_TSM9024FC] = &tsm9024fc_board_conn,
    [PPAL_BOARD_TYPE_TGM9024] = &tgm9024_board_conn,
    [PPAL_BOARD_TYPE_TXM9004] = &txm9004_board_conn,
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_board_conn,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_board_conn,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_board_conn,
    [PPAL_BOARD_TYPE_CGM9048] = &cgm9048_board_conn,
    [PPAL_BOARD_TYPE_FW9001] = &fw9001_board_conn,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_board_conn,
    [PPAL_BOARD_TYPE_ASX9604L] = &asx9604l_board_conn,
    [PPAL_BOARD_TYPE_AS9612X] = &as9612x_board_conn,
    [PPAL_BOARD_TYPE_AS6612C] = &as6612c_board_conn,
    [PPAL_BOARD_TYPE_G96SUP] = &g96sup_board_conn,
	[PPAL_BOARD_TYPE_G9604X] = &g9604x_board_conn,
	[PPAL_BOARD_TYPE_CGM9048S] = &cgm9048s_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};

board_fix_param_t *nh_series_board_param_arr[] =
{
    [PPAL_BOARD_TYPE_TSM9002] = &tsm9002_param,
    [PPAL_BOARD_TYPE_TGM9048] = &tgm9048_param,
    [PPAL_BOARD_TYPE_TSM9024FC] = &tsm9024fc_param,
    [PPAL_BOARD_TYPE_TGM9024] = &tgm9024_param,
    [PPAL_BOARD_TYPE_TXM9004] = &txm9004_param,
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_param,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_param,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_param,
    [PPAL_BOARD_TYPE_CGM9048] = &cgm9048_param,
    [PPAL_BOARD_TYPE_FW9001] = &fw9001_param,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_param,
    [PPAL_BOARD_TYPE_ASX9604L] = &asx9604l_param,
    [PPAL_BOARD_TYPE_AS9612X] = &as9612x_param,
    [PPAL_BOARD_TYPE_AS6612C] = &as6612c_param,   
    [PPAL_BOARD_TYPE_G96SUP] = &g96sup_param,   
	[PPAL_BOARD_TYPE_G9604X] = &g9604x_param, 
    [PPAL_BOARD_TYPE_CGM9048S] = &cgm9048s_param,

    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};

struct board_conn_type_s nh_series_board_conn_arr_fullmesh[] = 
{
};


board_conn_type_t *as_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_TGM9048] = &tgm9048_board_conn,
    [PPAL_BOARD_TYPE_TSM9024FC] = &tsm9024fc_board_conn,
    [PPAL_BOARD_TYPE_TGM9024] = &tgm9024_board_conn,
    [PPAL_BOARD_TYPE_TXM9004] = &txm9004_board_conn,
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_board_conn,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_board_conn,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_board_conn,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_board_conn,
    [PPAL_BOARD_TYPE_AS9612X] = &as9612x_board_conn,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};

board_fix_param_t *as_series_board_param_arr[] =
{
    [PPAL_BOARD_TYPE_TGM9048] = &tgm9048_param,
    [PPAL_BOARD_TYPE_TSM9024FC] = &tsm9024fc_param,
    [PPAL_BOARD_TYPE_TGM9024] = &tgm9024_param,
    [PPAL_BOARD_TYPE_TXM9004] = &txm9004_param,
    [PPAL_BOARD_TYPE_AX63GE24] = &ax63ge24_param,
    [PPAL_BOARD_TYPE_AX63GE24I] = &ax63ge24i_param,
    [PPAL_BOARD_TYPE_AX63GE48] = &ax63ge48_param,
    [PPAL_BOARD_TYPE_ASX6602] = &asx6602_param,
    [PPAL_BOARD_TYPE_AS9612X] = &as9612x_param,
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    
};

struct board_conn_type_s as_series_board_conn_arr_fullmesh[] = 
{
};



/* family feature */
family_common_fix_param_t as6600_family_param =
{
	.family_type = FAMILY_TYPE_AS6600,
	.product_param_arr =  nh_series_product_param_arr,
	.product_conn_arr = nh_series_product_conn_arr,
	.board_conn_arr = as_series_board_conn_arr,
	.board_conn_fullmesh_arr = as_series_board_conn_arr,
	.board_param_arr = as_series_board_param_arr,
};


/* family feature */
family_common_fix_param_t t9k_family_param =
{
	.family_type = FAMILY_TYPE_T9000,
	.product_param_arr =  nh_series_product_param_arr,
	.product_conn_arr = nh_series_product_conn_arr,
	.board_conn_arr = nh_series_board_conn_arr,
	.board_conn_fullmesh_arr = nh_series_board_conn_arr,
	.board_param_arr = nh_series_board_param_arr,
};


struct family_common_fix_param_s * family_type_arr[] = 
{
	[FAMILY_TYPE_T9000] = &t9k_family_param,
	[FAMILY_TYPE_AS6600] = &as6600_family_param,
};

extern struct family_spec_fix_param_s *family_spec_type_arr[];
    

long nh_product_info_init(void)
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

long nh_product_param_read()
{
    /*open file "snros_product_param.cfg"*/

    /*read snros_product_param and init ppal_product_info, n_param and g_sysinfo_backboard and g_sysinfo_module*/
    return NPD_SUCCESS;
}

void product_init(void)
{
	productinfo.family_type = nh_family_type_get();
    productinfo.product_id = nh_product_hw_code_get();
    productinfo.local_module_id = nh_local_module_hw_code_get();
    productinfo.local_module_hw_version = nh_board_hw_version_get();
    productinfo.local_chassis_slot_no = nh_board_hw_slotno_get();
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;

    nh_product_info_init();

}

#ifdef __cplusplus

}
#endif

