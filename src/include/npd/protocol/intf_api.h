#ifndef __PROTOCOL_INTF_API_H__
#define __PROTOCOL_INTF_API_H__


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

typedef struct if_cfg_struct
{
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

} VIRT_DEVICE_PARAM;

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


int npd_intf_set_mac_address
(
  	unsigned int ifIndex,
 	unsigned char * macAddr
);


#endif
