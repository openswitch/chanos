
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*<tun_global>.h
*
*MODIFY:
*	by <zhubo@autelan.com> on 11/13/2007 revision <x.x.x..>
*
*CREATOR:
*	<zhubo@autelan.com>
*
*DESCRIPTION:
*<some description about this file>
*
*DATE:
*	11/13/2007	
*	
*******************************************************************************/
#ifndef _TUN_GLOBAL_H_ 
#define _TUN_GLOBAL_H_

/* include header file begin */
/* kernel or sys part */
/*#include <xxxx/xxxx.h> */
/* user or app part */
#include <linux/types.h>
#include <linux/socket.h>

/* include header file end */

/* MACRO definition begin */
/* notations */
/* MACRO definition end */

/* structure definition begin */
typedef enum type{

    TUN_INTERFACE_PORT_E = 0,
    TUN_INTERFACE_TRUNK_E,
    TUN_INTERFACE_VIDX_E,
    TUN_INTERFACE_VID_E
}TUN_DEV_TYPE;

typedef struct _usrPktMsg_s{

	TUN_DEV_TYPE dev_type;
	unsigned int if_index;
	unsigned int port_index;
	unsigned int vId;

	unsigned int data_len;		
	unsigned char* data_addr;	
}RxPktMSG;

struct cmd_struct 
{
#define IFHWADDRLEN	6
	union
	{
		char	if_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	} if_ifn;
	
	union {
		struct	sockaddr ifu_addr;
		struct	sockaddr ifu_dstaddr;
		struct	sockaddr ifu_broadaddr;
		struct	sockaddr ifu_netmask;
		struct  sockaddr ifu_hwaddr;
		unsigned int	ifu_ivalue;
		unsigned long	ifu_flag;
		}if_ifu;

		TUN_DEV_TYPE dev_type;
		unsigned int if_index;
		unsigned int port_index;
		unsigned int vId;

		unsigned char mac_addr[6];
};

#define	if_name	if_ifn.if_name	/* interface name 	*/
#define	if_hwaddr	if_ifu.ifu_hwaddr	/* MAC address 		*/
#define	if_addr	if_ifu.ifu_addr	/* address		*/
#define	if_dstaddr	if_ifu.ifu_dstaddr	/* other end of p-p lnk	*/
#define	if_broadaddr	if_ifu.ifu_broadaddr	/* broadcast address	*/
#define	if_netmask	if_ifu.ifu_netmask	/* interface net mask	*/
#define	if_flag	if_ifu.ifu_flag	/* flags		*/
#define	if_owner if_ifu.ifu_ivalue /*set dev owner*/
#define	if_type if_ifu.ifu_flag      /*set dev type*/
#define	if_debug if_ifu.ifu_ivalue   /*set dev debug*/
/* structure definition end */

/* local functions declearations begin */

/* local functions declearations end */

#endif /* _TUN_GLOBAL_H_ */
