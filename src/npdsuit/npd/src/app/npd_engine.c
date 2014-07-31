
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_engine.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for ASIC
*
* DATE:
*		02/21/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.12 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_engine.h"

int npd_fw_engines_init(void) {
    int i;
    board_param_t *param;

    for(i = 1; i < SLOT_SUBCARD_COUNT(SYS_LOCAL_MODULE_SLOT_INDEX) ; i++)
    {
        int j;

        param = localmoduleinfo->sub_board[i];
        if(NULL == param)
            continue;
		if(NULL == param->fix_param)
			continue;
		if(NULL == param->fix_spec_param)
			continue;
        for( j = 0; j < ASIC_TYPE_MAX; j++)
        {
            if(NULL != param->fix_spec_param->ams_param[j])
            {
                int n;
                for(n = 0; n < param->fix_spec_param->ams_param[j]->num; n++)
                {
					if(param->fix_spec_param->ams_param[j]->ams_driver_init)
					{
                        (*param->fix_spec_param->ams_param[j]->ams_driver_init)(n);
					}
                }
            }
        }
    }
    npd_syslog_dbg("Finish subboard ams driver init\n");

    for(i = 0; i < ASIC_TYPE_MAX; i++)
    {
        if(NULL != localmoduleinfo->fix_spec_param->ams_param[i])
        {
            int n;
            for(n = 0; n < localmoduleinfo->fix_spec_param->ams_param[i]->num; n++)
            {
                npd_syslog_dbg("Init asic type %d, chip %d\n",
                    i, n);
				if(localmoduleinfo->fix_spec_param->ams_param[i]->ams_driver_init)
				{
                    (*localmoduleinfo->fix_spec_param->ams_param[i]->ams_driver_init)(n);
				}
            }
        }
    }
    npd_syslog_dbg("Finish ams driver init\n");
    
    
	return NPD_SUCCESS;
}

int npd_fw_engines_led_init(void)
{
    int i;
    board_param_t *param;

    for(i = 1; i < SLOT_SUBCARD_COUNT(SYS_LOCAL_MODULE_SLOT_INDEX) ; i++)
    {
        int j;

        param = localmoduleinfo->sub_board[i];
        if(NULL == param)
            continue;
		if(NULL == param->fix_param)
			continue;
		if(NULL == param->fix_spec_param)
			continue;
        for( j = 0; j < ASIC_TYPE_MAX; j++)
        {
            if(NULL != param->fix_spec_param->ams_param[j])
            {
                int n;
                for(n = 0; n < param->fix_spec_param->ams_param[j]->num; n++)
                {
					if(param->fix_spec_param->ams_param[j]->ams_led_proc)
					{
						(*param->fix_spec_param->ams_param[j]->ams_led_proc)(n);
					}
                }
            }
        }
    }

    for(i = 0; i < ASIC_TYPE_MAX; i++)
    {
        if(NULL != localmoduleinfo->fix_spec_param->ams_param[i])
        {
            int n;
            for(n = 0; n < localmoduleinfo->fix_spec_param->ams_param[i]->num; n++)
            {
				if(localmoduleinfo->fix_spec_param->ams_param[i]->ams_led_proc)
				{
					(*localmoduleinfo->fix_spec_param->ams_param[i]->ams_led_proc)(n);
				}
            }
        }
    }

	return NPD_SUCCESS;
}

int npd_fw_engine_initialization_check(void) {
	unsigned long numOfAsic = 0;

	nam_asic_init_completion_check();
    npd_fw_engines_led_init();
	/* after all asic initialization done, check asic instance*/
	numOfAsic = nam_asic_get_instance_num();
    /*will set the default param for internal interconnect ports between chips or between subslots ports and chips*/
    if(localmoduleinfo->fix_spec_param->local_conn_init)
        (*localmoduleinfo->fix_spec_param->local_conn_init)(SYS_PRODUCT_TYPE);


	return NPD_SUCCESS;
}


#ifdef __cplusplus
}
#endif
