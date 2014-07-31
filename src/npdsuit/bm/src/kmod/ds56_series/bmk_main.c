
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* bmk_main.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		bm -- board management (i2c, gpio, bootbus, cpld, ....) setting.
*
* DATE:
*		11/27/2010
*		11/09/2010             zhanwei@autelan.com           Add CPLD interrupt handler and bm_poll. Make 'select' working.
*  FILE REVISION NUMBER:
*  		$Revision: 1.49 $	
*******************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/jiffies.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include "bmk_main.h"
#include "bmk_product_feature.h"
#include "bmk_product_init.h"
#define THREASH_HOLD 10
#define DRIVER_NAME "bm"
#define DRIVER_VERSION "0.1"
extern int bm_product_init(void);
extern void bm_product_cleanup(void);

struct bm_dev_s {
	wait_queue_head_t wait;           // cpld interrupt wait queue
	unsigned int irq_board;
	spinlock_t lock;
	unsigned long interrupt_count;
	unsigned long last_update_time;
	struct timer_list enable_timer;
	struct cdev cdev;
};

struct bm_dev_s bm_dev;
kfamily_fix_param * ko_family;
kproduct_fix_param * ko_product;
kboard_fix_param*  ko_board;
kboard_param*  kboard_info = NULL;

void bm_get_mngt_eth_mac(int port, char *mac)
{
	if (ko_board && ko_board->board_get_mngt_eth_mac)
		ko_board->board_get_mngt_eth_mac(port, mac);
}
EXPORT_SYMBOL(bm_get_mngt_eth_mac);

void bm_interrupt_monitor(unsigned long data)
{
	static unsigned int monitor_trigged_counter = 0;
	struct bm_dev_s *dev = (struct bm_dev_s *)data;
	monitor_trigged_counter++;
	printk("Interrupt monitor timer is trigged.(%d timer(s))\n", monitor_trigged_counter);
	spin_lock(&dev->lock);
	if(ko_board)
	{
    	if(ko_board->irq_board)
    	{
			dev->irq_board = ko_board->irq_board;
    	    printk("I will enable board interrupt %d.\n", dev->irq_board);
    	    enable_irq(dev->irq_board);
    	}
	}
	spin_unlock(&dev->lock);
}

irqreturn_t
bm_cpld_interrupt_handler(int irq, void *dev_id)
{
	struct bm_dev_s *dev = (struct bm_dev_s *)dev_id;
	int op_ret;
	unsigned long current_jif = jiffies;

	DBG(debug_ioctl, "Enter bm_cpld_interrupt_handler.\r\n");

	spin_lock(&dev->lock);
    dev->interrupt_count++;
	if(dev->interrupt_count%THREASH_HOLD == 1)
	{
    	if(dev->last_update_time > current_jif)
    	{
    	}
    	else if(current_jif - dev->last_update_time <= HZ/THREASH_HOLD)/*100ms内允许10次中断*/
    	{
    	    DBG(debug_ioctl, "Too many interrupt in 1 second.\r\n");
			if(ko_board)
			{
    			if(ko_board->irq_board)
    			{
					dev->irq_board = ko_board->irq_board;
    			    disable_irq(dev->irq_board);
    		        mod_timer(&dev->enable_timer, jiffies + 1*HZ);
    				printk("Too many interrupt. I disabled irq %d. It will be enabled after 1 second.\r\n", dev->irq_board);
    			}
			}
    	}
	    dev->last_update_time = current_jif;
	}
	/* 读取中断寄存器的值 */
	if (ko_board && ko_board->interrupt_handler)
		op_ret = ko_board->interrupt_handler(irq, dev_id);
    if(waitqueue_active(&dev->wait))
    {
	    wake_up_interruptible(&dev->wait);
    }
	spin_unlock(&dev->lock);

	return op_ret;
}

static unsigned int bm_poll(struct file *filp, struct poll_table_struct *wait)
{
	struct bm_dev_s *dev;
	dev = filp->private_data;
	poll_wait(filp, &dev->wait, wait);
	if (dev->interrupt_count)
	{
		dev->interrupt_count = 0;
		return POLLIN | POLLRDNORM;
	}
	return 0;
}

static int bm_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg)
{
	int op_ret = 0;
	//DBG(debug_ioctl,KERN_WARNING DRIVER_NAME ":bm_ioctl before check");

	if (_IOC_TYPE(cmd) != BM_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > BM_IOC_MAXNR) return -ENOTTY;

	//DBG(debug_ioctl, KERN_WARNING DRIVER_NAME ":bm_ioctl after check\n");

	op_ret = bm_common_ioctl(/*inode, */filp, cmd, arg);
	if (op_ret != -2)	
	{
		return op_ret;
	}

	op_ret = bm_product_series_ioctl(/*inode, */filp, cmd, arg);
	if (op_ret != -2)	
	{
		return op_ret;
	}

	if (ko_product && (NULL != ko_product->ioctl))
	{
		op_ret = ko_product->ioctl(/*inode,*/ filp, cmd, arg);	
	}
	if (op_ret != -2)
	{
		return op_ret;
	}

	
	if (ko_board && (NULL != ko_board->ioctl)) /* */
	{
		return ko_board->ioctl(/*inode,*/ filp, cmd, arg);
	}
	else
	{
		return -ENOTTY;
	}
}

static long bm_compat_ioctl(struct file *filp,unsigned int cmd, unsigned long arg) 
{
	int rval;

    rval = bm_ioctl(/*filp->f_dentry->d_inode, */filp, cmd, arg);

    return (rval == -EINVAL) ? -ENOIOCTLCMD : rval;
}

static int bm_open(struct inode *inode, struct file *filp)
{
	struct bm_dev_s *dev;

	dev = container_of(inode->i_cdev, struct bm_dev_s, cdev);
	filp->private_data = dev;
	//dev->size = 0;
	return 0;
}

static int bm_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations bm_fops = 
{
	.owner	= THIS_MODULE,
	//.llseek	= bm_llseek,
	//.read 	= bm_read,
	//.write	= bm_write,
	.poll	= bm_poll,
	.unlocked_ioctl	= bm_ioctl,
  	.compat_ioctl = bm_compat_ioctl,
  	.open	= bm_open,
	.release= bm_release

};

static int bm_init(void)
{
	int result;
	dev_t bm_devt;

	bm_devt = MKDEV(BM_MAJOR_NUM,BM_MINOR_NUM);
	result = register_chrdev_region(bm_devt,1,DRIVER_NAME);
	printk(KERN_INFO DRIVER_NAME ":register major dev [%d] with debug_ioctl[%d]\n",MAJOR(bm_devt),debug_ioctl);

	if (result < 0)	{
		printk(KERN_WARNING DRIVER_NAME ":register_chrdev_region err[%d]\n",result);
		return 0;
	}
	memset(&bm_dev, 0, sizeof(struct bm_dev_s));
	init_waitqueue_head(&bm_dev.wait);
	spin_lock_init(&bm_dev.lock);
	
	init_timer(&bm_dev.enable_timer);
	bm_dev.enable_timer.expires = jiffies + 1*HZ;
	bm_dev.enable_timer.data = (unsigned long)&bm_dev;
	bm_dev.enable_timer.function = bm_interrupt_monitor;		/* timer handler */
	add_timer(&bm_dev.enable_timer);
	cdev_init(&(bm_dev.cdev),&bm_fops);
	bm_dev.cdev.owner = THIS_MODULE;
    result = cdev_add(&(bm_dev.cdev),bm_devt,1);
	if (result < 0)	{
		printk(KERN_WARNING DRIVER_NAME ":cdev_add err[%d]\n",result);
	} else {
		printk(KERN_INFO DRIVER_NAME ":loaded successfully.\n");
	}
	result = bm_product_init();
	return result;

}


static void bm_cleanup(void)
{
	printk(KERN_INFO DRIVER_NAME ":Unregister MajorNum[%d]\n",MAJOR(bm_dev.cdev.dev));	
	unregister_chrdev_region(bm_dev.cdev.dev,1);	
	cdev_del(&(bm_dev.cdev));
	printk(KERN_INFO DRIVER_NAME ":unloaded\n");

	bm_product_cleanup();

}

module_init(bm_init);
module_exit(bm_cleanup);

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CHANOS Development Team");


