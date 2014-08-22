/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* bmk_ds6224_board_param.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		bm -- BOARD TXM9004 management (i2c, gpio, bootbus, cpld, ....) setting.
*
* UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_ds5600_series_info.h"
#include "ts_product_feature.h"
#include "bmk_operation_boot_env.h"
#include "bmk_read_eeprom.h"

int ds6224_board_ioctl_led_light(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	

	return 0;
}

int ds6224_board_ioctl_i2c_write(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	return 0;
}


ioctl_proc ds6224_board_ioctl_proc_arr[] = 
{
	{BM_IOC_WIRELESS_LED_LIGTH,			ds6224_board_ioctl_led_light},
	{BM_IOC_MUSIC_I2C_WRITE,	 		ds6224_board_ioctl_i2c_write},
};


int ds6224_board_init(void)
{
	int result = 0;
	
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":Enter ds6224_board_init\n");

	ko_board->ioctl_proc_count = LENGTH(ds6224_board_ioctl_proc_arr);
	DBG(debug_ioctl, KERN_INFO DRIVER_NAME ":ioctl count is %d\n", ko_board->ioctl_proc_count);

	return result;
}

void ds6224_board_cleanup(void)
{
	if (NULL != ko_board->cpld_int_data)
	{
		kfree(ko_board->cpld_int_data);
	}
		
	ko_board->cpld_int_data = NULL;
}

kboard_fix_param ds6224_board  =
{	
	.board_code = PPAL_BOARD_HWCODE_DS6224,
	.board_type = PPAL_BOARD_TYPE_DS6224,
	.board_name = "DS6224",
		
	.board_init = ds6224_board_init,
	.board_cleanup = ds6224_board_cleanup,
	.ioctl		= bm_board_ioctl,
	.ioctl_proc_arr = ds5600_board_ioctl_proc_arr,
	
	.interrupt_handler = bm_board_interrupt_handler,

	.cpld_reg_base_addr = CPLD_REG_BASE_ADDR,
};


