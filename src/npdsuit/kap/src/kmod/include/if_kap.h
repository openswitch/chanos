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
*********************************************************************************/

#ifndef __IF_KAP_H
#define __IF_KAP_H

#include <linux/types.h>
#include <linux/socket.h>

/* Uncomment to enable debugging */
/* #define TUN_DEBUG 1 */

#ifdef __KERNEL__

#ifdef KAP_DEBUG
#define DBG if(kap_debug==1)printk
#else
#define DBG( a... )
#endif
#define DBG_ERR( a... ) printk

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with if_name.  The
 * remainder may be interface specific.
 */

#define IFNAMSIZ 16
#define MAX_IP_COUNT 8

typedef enum type{

    KAP_INTERFACE_PORT_E = 0,
    KAP_INTERFACE_TRUNK_E,
    KAP_INTERFACE_VIDX_E,
    KAP_INTERFACE_VID_E,
    KAP_INTERFACE_UNKNOWN_E
}KAP_DEV_TYPE;

typedef enum link_status{
	DEV_LINK_DOWN = 0,
	DEV_LINK_UP,
	DEV_LINK_MAX
}KAP_DEV_LINK_STATUS;

struct data_struct
{
		KAP_DEV_TYPE dev_type;
		unsigned int l3_index;
		unsigned int l2_index;
		unsigned int vId;

		unsigned int data_len;		
		unsigned int data_addr;	
};

struct if_cfg_struct 
{
#define IFHWADDRLEN	6
	union
	{
		char	if_name[16];		/* if name, e.g. "en0" */
	} if_ifn;
	
	union {
		struct	sockaddr ifu_addr[MAX_IP_COUNT];
		struct	sockaddr ifu_dstaddr;
		struct	sockaddr ifu_broadaddr;
		struct	sockaddr ifu_netmask;
		struct  sockaddr ifu_hwaddr;

		unsigned int	ifu_ivalue;
		unsigned int	ifu_flag;
	}if_ifu;

	KAP_DEV_TYPE dev_type;
	KAP_DEV_LINK_STATUS dev_state;
	unsigned int l3_index;
	unsigned int l2_index;
	unsigned int vId;
	unsigned int netmask[MAX_IP_COUNT];

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


struct user_netdevice{
		struct list_head  list;
        struct net_device* dev;

		KAP_DEV_TYPE dev_type;
		unsigned int l3_index;
		unsigned int l2_index;
		unsigned int vId;

};

struct kap_struct {
	struct list_head    dev_list;
	unsigned int 		flags;
	//int			attached;
	//uid_t			owner;

	
	wait_queue_head_t	read_wait;
	struct sk_buff_head	readq;

	struct user_netdevice **udev_index;
	struct net_device_stats	stats;

	struct fasync_struct    *fasync;

	unsigned int if_flags;
	unsigned char if_mac_addr[6];

#ifdef KAP_DEBUG	
	int debug;
#endif  
};


#endif /* __KERNEL__ */

/* Read queue size */
#define KAP_READQ_SIZE	500

/* KAP device flags */
#define KAP_TUN_DEV 	0x0001	
#define KAP_TAP_DEV	0x0002
#define KAP_TYPE_MASK   0x000f

#define KAP_FASYNC	0x0010
#define KAP_NOCHECKSUM	0x0020
#define KAP_NO_PI	0x0040
#define KAP_ONE_QUEUE	0x0080
#define KAP_PERSIST 	0x0100	

/* Ioctl defines */
#define KAPSETNOCSUM  _IOW('T', 210, struct if_cfg_struct) 
#define KAPSETDEBUG   _IOW('T', 211, struct if_cfg_struct) 
#define KAPSETIFF     _IOWR('T', 212, struct if_cfg_struct) 
#define KAPSETPERSIST _IOW('T', 213, struct if_cfg_struct) 
#define KAPSETOWNER   _IOW('T', 214, struct if_cfg_struct)
#define KAPSETLINK    _IOW('T', 215, struct if_cfg_struct)
#define KAPADDIFF     _IOW('T',216,struct if_cfg_struct)
#define KAPWAITING       _IOW('T',217,int)
#define KAPSETIPADDR _IOW('T',218,struct if_cfg_struct)
#define KAPGETIPADDR _IOWR('T',219,struct if_cfg_struct)
#define KAPRETURNPKGNO _IOW('T',220,int)
#define KAPSETDEVINFO  _IOWR('T',221,struct if_cfg_struct)
#define KAPGETDEVINFO _IOWR('T',225,struct if_cfg_struct)
#define KAPSETMACADDR _IOW('T',222,struct if_cfg_struct)
#define KAPGETMACADDR _IOWR('T',223,struct if_cfg_struct)
#define KAPDELIFF _IOWR('T',224,struct if_cfg_struct)
#define KAPGETIPADDRS _IOWR('T',226,struct if_cfg_struct)



#define KAP_MAXNR	255

/* KAPSETIFF if flags */
#define KAP_IFF_TUN		0x0001
#define KAP_IFF_TAP		0x0002
#define KAP_IFF_NO_PI	0x1000
#define KAP_IFF_ONE_QUEUE	0x2000

struct kap_pi {
	unsigned short flags;
	unsigned short proto;
};
#define KAP_PKT_STRIP	0x0001

#endif /* __IF_TUN_H */

