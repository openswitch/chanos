
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* file_split.h
*
* MODIFY:
*		
*
* CREATOR:
*		pangxf@autelan.com
*
* DESCRIPTION:
*		Split an image file  .
*
* DATE:
*		12/15/2012
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.6 $	
*******************************************************************************/
#ifndef __FILE_SPLIT_H__
#define __FILE_SPLIT_H__

#define BLOCK_SIZE 64*1024
#define SQUASHFS_MAGIC		0x73717368
#define CHECK_IMG_SIZE 64*1024


#define BM_IOC_ENV_EXCH		_IOWR(BM_IOC_MAGIC,10,boot_env_t)
typedef struct boot_env
{	
	char name[64];	
	char value[128];	
	int operation;
}boot_env_t;
#define BM_IOC_MAGIC 0xEC







#endif 
