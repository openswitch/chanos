
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include "bcm/stat.h"
#include "bcm/stack.h"
#include "bcm/trunk.h"
#include "bcm/vlan.h"
#include "bcm/switch.h"
#include "bcm/port.h"
#include "bcm/mcast.h"
#include "bcm/ipmc.h"
#include "bcm/error.h"
#include "soc/drv.h"
#ifndef INCLUDE_I2C
#define INCLUDE_I2C
#include <soc/i2c.h>
#include <bcm/bcmi2c.h>
#include <bcm/error.h>
#endif

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
#include "as2k_product_feature.h"

#include "as2k_product_init.h"
#include "as2k_product_info.h"


static int console_fd = -1;

extern long nam_asic_info_get(int unit, struct ams_info_s *info);
extern long nam_asic_enable(int index);
extern unsigned long systemInitialized;


#include "as2k_3200_real_feature.c"
#include "as2k_3200_conn.c"
#ifdef HAVE_POE
#include "as2k_3200_9gc_pwr_conn.c"
#include "as2k_3200_28gc_pwr_conn.c"
#include "as2k_3200_24gc_pwr_conn.c"
#endif

#ifdef HAVE_POE
#include "as2k_3200_9gc_pwr_feature.c"
#include "as2k_3200_28gc_pwr_feature.c"
#include "as2k_3200_24gc_pwr_feature.c"
#endif
#include "as2k_3200_24gc_feature.c"

/* as2k product connect for single */
struct product_conn_type_s as2k_3200_product_conn = 
{
    .product_type = PRODUCT_AS2K_3200,
    .chassis_topo = CENTRAL_FABRIC,
    .sys_backboard = NULL,
    .ctrl_backboard = NULL,
    .superv_backboard = NULL,
};

unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;


/* au2k series   */

product_fix_param_t *as2k_series_product_param_arr[] =
{
    [PRODUCT_AS2K_3200] = &as2k_3200_real_fix_param,
    [PRODUCT_NH_MAX_NUM] = NULL,    		
};

board_fix_param_t   *as2k_series_board_param_arr[] = 
{
    [PPAL_BOARD_TYPE_AS2K_3200] = &as2k_3200_24gc_param ,  
#ifdef HAVE_POE
    [PPAL_BOARD_TYPE_AS2K_3209] = &as2k_3200_9gc_pwr_param ,  				
    [PPAL_BOARD_TYPE_AS2K_3228] = &as2k_3200_28gc_pwr_param ,  	
	[PPAL_BOARD_TYPE_AS2K_3224P] = &as2k_3200_24gc_pwr_param,
#endif
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    		
};

board_spec_fix_param_t   *as2k_series_board_spec_param_arr[] = 
{
    [PPAL_BOARD_TYPE_AS2K_3200] = &as2k_3200_24gc_spec_param ,  
#ifdef HAVE_POE
    [PPAL_BOARD_TYPE_AS2K_3209] = &as2k_3200_9gc_pwr_spec_param ,  				
    [PPAL_BOARD_TYPE_AS2K_3228] = &as2k_3200_28gc_pwr_spec_param ,  	
	[PPAL_BOARD_TYPE_AS2K_3224P] = &as2k_3200_24gc_pwr_spec_param,
#endif
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    		
};


product_conn_type_t *as2k_series_product_conn_arr[] =
{
    [PRODUCT_AS2K_3200] = &as2k_3200_product_conn,
    [PRODUCT_NH_MAX_NUM] = NULL,    				
};

struct board_conn_type_s *as2k_series_board_conn_arr[] =
{
    [PPAL_BOARD_TYPE_AS2K_3200] = &as2k_3200_board_conn,
#ifdef HAVE_POE
    [PPAL_BOARD_TYPE_AS2K_3209] = &as2k_3200_9gc_pwr_board_conn,				
    [PPAL_BOARD_TYPE_AS2K_3228] = &as2k_3200_28gc_pwr_board_conn,	
	[PPAL_BOARD_TYPE_AS2K_3224P] = &as2k_3200_24gc_pwr_board_conn,
#endif
    [PPAL_BOARD_TYPE_NH_MAX] = NULL,    				
};



family_common_fix_param_t as2k_family_param =
{
	.family_type = FAMILY_TYPE_AS2000,
	.product_param_arr =  as2k_series_product_param_arr,
	.product_conn_arr = as2k_series_product_conn_arr,
	.board_conn_arr = as2k_series_board_conn_arr,
	.board_conn_fullmesh_arr = as2k_series_board_conn_arr,
	.board_param_arr = as2k_series_board_param_arr
};

struct family_common_fix_param_s * family_type_arr[] = 
{
	[FAMILY_TYPE_AS2000] = &as2k_family_param, 
};


struct family_spec_fix_param_s as2k_family_spec_param =
{
	.family_type = FAMILY_TYPE_AS2000,
	.board_spec_param_arr = as2k_series_board_spec_param_arr,
};

struct family_spec_fix_param_s * family_spec_type_arr[] = 
{
	[FAMILY_TYPE_AS2000] = &as2k_family_spec_param, 
};


long as2k_product_info_init(void)
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

long as2k_product_param_read()
{
    /*open file "snros_product_param.cfg"*/

    /*read snros_product_param and init ppal_product_info, n_param and g_sysinfo_backboard and g_sysinfo_module*/
    return NPD_SUCCESS;
}

void product_init(void)
{
	productinfo.family_type = as2k_family_type_get();
    productinfo.product_id = as2k_product_hw_code_get();
    productinfo.local_module_id = as2k_local_module_hw_code_get();
    productinfo.local_module_hw_version = as2k_board_hw_version_get();
    productinfo.local_chassis_slot_no = as2k_board_hw_slotno_get();
    SYS_MASTER_ACTIVE_SLOT_INDEX = -1;

    as2k_product_info_init();

}

#ifdef __cplusplus

}
#endif

