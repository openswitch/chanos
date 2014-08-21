#ifndef _BMK_PRODUCT_INIT_H_
#define _BMK_PRODUCT_INIT_H_

int bm_common_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg);
int bm_product_series_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg);

int bm_common_init(void);
int bm_product_series_init(void);



#endif //_BMK_PRODUCT_INIT_H_


