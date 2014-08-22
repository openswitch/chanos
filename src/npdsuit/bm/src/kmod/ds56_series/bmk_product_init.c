
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* bmk_product_init.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		bm -- product driver init.
*
* UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/


#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>

#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "ts_product_feature.h"
#include "bmk_product_init.h"

extern kboard_fix_param * ds5600_series_board_param_arr[] ;
extern kproduct_fix_param * ds5600_series_product_param_arr[];

#include "bmk_ds5652_board_param.c"
#include "bmk_ds5652_product_param.c"

#include "bmk_ds6224_board_param.c"
#include "bmk_ds6224_product_param.c"
/* ********** variables ***************** */
kfamily_fix_param ** kfamily_type_arr = NULL;
kproduct_fix_param ** kproduct_type_arr = NULL;
kboard_fix_param ** kboard_type_arr = NULL;

spinlock_t int_read_lock = __SPIN_LOCK_UNLOCKED(int_read_lock);

unsigned int PPAL_BOARD_TYPE_NONE = PPAL_BOARD_TYPE_NH_NONE;
unsigned int PPAL_BOARD_TYPE_MAX = PPAL_BOARD_TYPE_NH_MAX;
unsigned int PRODUCT_MAX_NUM = PRODUCT_NH_MAX_NUM;

extern int bmk_ds5652_twsi_eeprom_read_byte(uint8_t addr, unsigned short data_offset, char *buf_ptr);

kboard_fix_param * ds5600_series_board_param_arr[] = 
{	

	[PPAL_BOARD_TYPE_DS5652]	= &ds5600_board,
	[PPAL_BOARD_TYPE_DS6224]	= &ds6224_board,
	[PPAL_BOARD_TYPE_NH_MAX]	= NULL,	
};


kproduct_fix_param * ds5600_series_product_param_arr[] = 
{
	[PRODUCT_DS5600] = &ds5600_product_fix_param,
	[PRODUCT_DS6224] = &ds6224_product_fix_param,
	[PRODUCT_NH_MAX_NUM]	= NULL,
};



kfamily_fix_param  ds5600_fix_param = 
{
	.family_type = FAMILY_TYPE_DS5600,
	.family_code = PPAL_HWCODE_FAMILY_DS5600,
	.kproduct_arr = ds5600_series_product_param_arr,
	.kboard_arr = ds5600_series_board_param_arr,
};

kfamily_fix_param * family_param_arr[] =
{
	[FAMILY_TYPE_DS5600] = &ds5600_fix_param,
	[FAMILY_TYPE_MAX] = NULL,
};


void register_family_array(kfamily_fix_param** family_array)
{
	kfamily_type_arr = family_array;
}

void register_product_array(kproduct_fix_param** product_array)
{
	kproduct_type_arr = product_array;
}

void register_board_array(kboard_fix_param** board_array)
{
	kboard_type_arr = board_array;
}


long bm_get_family_code(void)
{
	return PPAL_HWCODE_FAMILY_DS5600; 
}


long bm_get_product_code(void)
{
    int ret = 0;
	char buf[2];
    ret = bmk_ds5652_twsi_eeprom_read_byte(0x50, 0x10, buf);
	if(ret == 0)
	{
        ret = bmk_ds5652_twsi_eeprom_read_byte(0x50, 0x11, buf+1);
		if(ret == 0)
		{
		    switch(buf[1])
		    {
		        case 1:
					return PPAL_PRODUCT_HWCODE_DS5652;
		        case 2:
					return PPAL_PRODUCT_HWCODE_DS6224;
		        default:
					break;
		    }
		}
	}
	return 	PPAL_PRODUCT_HWCODE_DS5662;
}


long bm_get_board_code(void)
{
    int ret = 0;
	char buf[2];
    ret = bmk_ds5652_twsi_eeprom_read_byte(0x50, 0x10, buf);
	if(ret == 0)
	{
        ret = bmk_ds5652_twsi_eeprom_read_byte(0x50, 0x11, buf+1);
		if(ret == 0)
		{
		    switch(buf[1])
		    {
		        case 1:
					return PPAL_BOARD_HWCODE_DS5652;
		        case 2:
					return PPAL_BOARD_HWCODE_DS6224;
		        default:
					break;
		    }
		}
	}
	return 	PPAL_BOARD_HWCODE_DS5662;
}

int bm_get_family_type(void)
{
	unsigned long code = bm_get_family_code();
    int i;	

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":family code is %lx\n", code);

    for(i = 0; i < FAMILY_TYPE_MAX; i ++)
    {
        if(NULL == kfamily_type_arr[i])
            continue;
        if(code == (*kfamily_type_arr[i]).family_code)
        {			            
            DBG(debug_ioctl, "The family hw code is %lx\n", code);
            DBG(debug_ioctl, "The family type is %lx\n", (*kfamily_type_arr[i]).family_type);
            DBG(debug_ioctl, "The family in %d pos of array\n", i);
                
            return (*kfamily_type_arr[i]).family_type;
        }
    }

    return FAMILY_UNKNOWN;
}

long bm_get_product_type(void)
{
	unsigned long code = bm_get_product_code();
    int i;	

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":product code is %lx\n", code);

    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
        if(NULL == kproduct_type_arr[i])
            continue;
        if(code == (*kproduct_type_arr[i]).product_code)
        {			            
            DBG(debug_ioctl, "The product hw code is %lx\n", code);
            DBG(debug_ioctl, "The product type is %lx\n", (*kproduct_type_arr[i]).product_type);
            DBG(debug_ioctl, "The product in %d pos of array\n", i);
                
            return (*kproduct_type_arr[i]).product_type;
        }
    }

    return PRODUCT_UNKNOWN;
}

int bm_get_board_type(void)
{
	unsigned long code = bm_get_board_code();
    int i;	

	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":board code is %lx\n", code);

	for(i = 0; i < PPAL_BOARD_TYPE_MAX; i ++)
    {
        if(NULL == kboard_type_arr[i])
            continue;
        if(code == (*kboard_type_arr[i]).board_code)
        {			            
            DBG(debug_ioctl, "The board hw code is %lx\n", code);
            DBG(debug_ioctl, "The board type is %lx\n", (*kboard_type_arr[i]).board_type);
            DBG(debug_ioctl, "The board in %d pos of array\n", i);
                
            return (*kboard_type_arr[i]).board_type;
        }
    }
	return BOARD_UNKNOWN;
}

int bm_common_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	DBG(debug_octeon, KERN_INFO DRIVER_NAME ":Enter bm_conmmon_ioctl. cmd = %x\n", cmd);
	
	return -2;
}

int bm_common_init()
{
	/* init kboard info */
	kboard_info = kmalloc(sizeof(kboard_param), GFP_KERNEL);
	if (NULL == kboard_info)
	{
		return -ENOMEM;
	}

	/* init family type info  */
	register_family_array(family_param_arr);
	kboard_info->family_type= bm_get_family_type();
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":family type is %ld\n", kboard_info->family_type);
	if (FAMILY_UNKNOWN == kboard_info->family_type)
	{
		DBG(debug_ioctl, "Error family type is unknown\n", kboard_info->family_type);	
		return FAMILY_UNKNOWN;
	}
	ko_family = kfamily_type_arr[kboard_info->family_type];
	if (NULL == ko_family)
	{
		DBG(debug_ioctl, KERN_INFO DRIVER_NAME "get ko_family error.\n");
		return -1;
	}

	register_product_array(ko_family->kproduct_arr);
	register_board_array(ko_family->kboard_arr);


	/* init product type info  */	
	kboard_info->product_type = bm_get_product_type();
	if (PRODUCT_UNKNOWN == kboard_info->product_type)
	{
		kboard_info->product_type = PRODUCT_DUMMY;
	}
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":product type is %ld\n", kboard_info->product_type);
	
    ko_product = kproduct_type_arr[kboard_info->product_type];
	if (NULL == ko_product)
	{
		DBG(debug_ioctl, KERN_INFO DRIVER_NAME "get ko_product error.\n");
		return -1;
	}
	
	/* init board type info  */	
	kboard_info->board_type = bm_get_board_type();
	if (BOARD_UNKNOWN == kboard_info->board_type)
	{
		kboard_info->board_type = PPAL_BOARD_TYPE_DUMMY;
	}
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":board type is %ld\n", kboard_info->board_type);

	ko_board = kboard_type_arr[kboard_info->board_type];
	if (NULL == ko_board)
	{
		DBG(debug_ioctl, KERN_INFO DRIVER_NAME "get ko_board error.\n");
		return -1;
		
	}
	kboard_info->fix_param = ko_board;

	return 0;
	
}

int bm_product_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ": Enter bm_product_init\n");
	
	result = bm_common_init();
	if (result != 0)
	{
		DBG(debug_ioctl,  "bm common init failure.\n");
		return result;
	}

	result = bm_product_series_init();
	if (result != 0)
	{
		DBG(debug_ioctl,  "bm product series init failure.\n");
		return result;
	}
	
	result = ko_product->product_param_init();
	if (result != 0)
	{
		DBG(debug_ioctl,  "product_init failure.\n");
		return result;
	}
	
	result = ko_board->board_init();
	if (result != 0)
	{
		DBG(debug_ioctl,  "board_init failure.\n");
		return result;
	}

	if(PRODUCT_DUMMY != kboard_info->product_type)
	{	
		if (NULL != ko_product->slot_index_get)
		{
			kboard_info->slot_index = ko_product->slot_index_get();
		}
		else
		{
			DBG(debug_ioctl,  "there no slot_index_get method.\n");
		}
	}
	else
	{
		kboard_info->slot_index = 0;
	}
	
	if (kboard_info->slot_index < 0)
	{
		DBG(debug_ioctl,  "get slot_index is not right, is %d", kboard_info->slot_index);
		return -EINVAL;
	}
	
	if (NULL != ko_product->proc_files_init)
	{
		ko_product->proc_files_init();
	}
	else
	{
		DBG(debug_ioctl, "no proc files init method.\n");		
	}
		
	return 0;
}

void bm_product_cleanup(void)
{
	//bm_proc_clearup();
	if (ko_product->proc_files_cleanup)
		ko_product->proc_files_cleanup();

	if (ko_board->board_cleanup)
		ko_board->board_cleanup();
	
	if (ko_product->product_param_cleanup)
		ko_product->product_param_cleanup();
		
	kboard_info->fix_param = NULL;
	kfree(kboard_info);
	ko_board = NULL;
}

