#ifndef __NPD_DLDP_COMMON_H__
#define __NPD_DLDP_COMMON_H__


#define NPD_DLDP_RETURN_CODE_0			(0)						/* success					*/
#define NPD_DLDP_RETURN_CODE_1			(1)						/* error						*/
#define NPD_DLDP_RETURN_CODE_BASE		(0x140000)				/* return code base 			*/
#define NPD_DLDP_RETURN_CODE_OK			(0x140000)				/* success					*/
#define NPD_DLDP_RETURN_CODE_ERROR		(0x140001)				/* error 						*/
#define NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL (0x140004)			/* DLDP not enabled global		*/

#define NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST (0x140009)			/* L2 vlan not exist			*/
#define NPD_DLDP_RETURN_CODE_NOTENABLE_VLAN (0x14000a)			/* L2 vlan not enable DLDP		*/
#define NPD_DLDP_RETURN_CODE_HASENABLE_VLAN (0x14000b)			/* L2 vlan has enable DLDP		*/
#define NPD_DLDP_RETURN_CODE_NULL_PTR		(0x14000c)			/* parameter pointer is null		*/

#define	NPD_DLDP_INIT_0					(0)						/* init int/short/long variable	*/
#define NPD_DLDP_INIT_FD				(-1)					/* initialize fd, default is -1		*/

#define	NPD_DLDP_ENABLE					(1)						/* DLDP global status, enable	*/
#define	NPD_DLDP_DISABLE				(0)						/* DLDP global status, disenable	*/
#define	NPD_DLDP_VLAN_ENABLE			(1)						/* DLDP vlan status, enable		*/
#define	NPD_DLDP_VLAN_DISABLE			(0)						/* DLDP vlan status, disenable	*/
#define	NPD_DLDP_PORT_ENABLE			(1)						/* DLDP port status, enable		*/
#define	NPD_DLDP_PORT_DISABLE			(0)						/* DLDP port status, disenable	*/

#define NPD_DLDP_MAC_ADDR_LEN			(6)						/* length of the mac address	*/
#define NPD_MSG_FROM_DLDP_MAX_SIZE		sizeof(NPD_MSG_FROM_DLDP_T)
																/* 1248						*/ 
#define NPD_MSG_TO_DLDP_MAX_SIZE		sizeof(NPD_MSG_TO_DLDP_T)

#define NPD_NPD2DLDP_CLIENT				"/tmp/npd2dldp_client"	/* npd to DLDP client			*/
#define NPD_NPD2DLDP_SERVER				"/tmp/npd2dldp_server"	/* npd to DLDP server			*/	

#define NPD_DLDP_SYS_SET_INIT			(1)						/* type of DLDP system set, init	*/
#define NPD_DLDP_SYS_SET_STOP			(2)						/* type of DLDP system set, stop	*/
#define NPD_DLDP_SYS_RECOVER_PORT		(3)						/* revover port which support DLDP	*/
#define NPD_DLDP_SYS_SHUTDOWN_PORT		(4)						/* shutdown port which support DLDP	*/

#define NPD_DLDP_MAC_ADDR_LEN			(6)						/* length of the mac address	*/

#define NPD_DLDP_TYPE_DEVICE_EVENT		(3)						/* device message				*/
#define	NPD_DLDP_TYPE_NOTIFY_MSG		(1)						/* notify message				*/
#define	NPD_DLDP_FLAG_ADDR_MOD			(3)						/* notify infor for modify address	*/

/* device notify event 			*/
#define NPD_DLDP_EVENT_DEV_UP			(0x0001)
#define NPD_DLDP_EVENT_DEV_DOWN			(0x0002)
#define NPD_DLDP_EVENT_DEV_REGISTER		(0x0003)
#define NPD_DLDP_EVENT_DEV_UNREGISTER	(0x0004)
#define NPD_DLDP_EVENT_DEV_VLAN_ADDIF	(0x0005)
#define NPD_DLDP_EVENT_DEV_VLAN_DELIF	(0x0006)
#define NPD_DLDP_EVENT_DEV_SYS_MAC_NOTI	(0x0007)
#define NPD_DLDP_EVENT_DEV_SYS_DISABLE  (0x0008)

/*show running cfg mem*/
#define NPD_DLDP_RUNNING_CFG_MEM (1024*1024)

typedef struct NPD_DLDP_NLMSGHDR_S
{
	unsigned int	nlmsg_len;					/* Length of message including header */
	unsigned short	nlmsg_type; 				/* Message content	*/
	unsigned short	nlmsg_flags;				/* Additional flags 	*/
	unsigned int	nlmsg_seq;					/* Sequence number	*/
	unsigned int	nlmsg_pid;					/* Sending process PID	*/
}NPD_DLDP_NLMSGHDR_T;

typedef struct NPD_DEV_EVENT_CMD_DLDP_S
{
	unsigned long  event_type;					/* event type					*/
	unsigned short vlan_id;						/* vlan id					*/
	unsigned int   ifindex;						/* eth-port index				*/
	unsigned int   soltno;						/* asic-port soltno			*/
	unsigned int   portno;						/* asic-port portno			*/
	unsigned int   tagmode;						/* asic-port tag mode			*/
}NPD_DEV_EVENT_CMD_DLDP_T;


/*******************************************/
/* NPD notify message to DLDP						*/
/*******************************************/
typedef struct NPD_NOTIFY_MOD_DLDP_S
{
	unsigned long  mod_type;
	unsigned short vlan_id;
	unsigned long  ifindex;
	unsigned int   slot_no; 					/* solt no of port		*/
	unsigned int   port_no; 					/* port no of port		*/
	unsigned int   tagmode; 					/* asic-port tag mode	*/
	unsigned char  sys_mac[NPD_DLDP_MAC_ADDR_LEN];
	unsigned long  reserve;
}NPD_NOTIFY_MOD_DLDP_T;

typedef struct NPD_MSG_DLDP_S{
	NPD_DLDP_NLMSGHDR_T nlh;
	NPD_NOTIFY_MOD_DLDP_T npd_notify_msg;
}NPD_MSG_TO_DLDP_T;


/*******************************************/
/* DLDP notify message to NPD						*/
/*******************************************/
typedef struct NPD_DLDP_NOTIFY_MOD_NPD_S
{
	unsigned long mod_type;			/* modify type		*/
	unsigned short vlan_id;			/* vlan id			*/
	unsigned long ifindex;			/* port index			*/
	unsigned long reserve;			/*					*/
}NPD_DLDP_NOTIFY_MOD_NPD_T;

typedef struct NPD_MSG_FROM_DLDP_S{
	NPD_DLDP_NLMSGHDR_T nlh;
	NPD_DLDP_NOTIFY_MOD_NPD_T dldp_notify_msg;
}NPD_MSG_FROM_DLDP_T;


unsigned int npd_get_dldp_global_status
(
	unsigned char *status
);

void npd_dldp_init
(
	void
);

int	npd_dldp_sock_init
(
	void
);

unsigned int npd_dldp_recv_info
(
	NPD_MSG_FROM_DLDP_T *msg,
	unsigned int infoLen
);

 unsigned int npd_cmd_sendto_dldp
(
	NPD_DEV_EVENT_CMD_DLDP_T *mngCmd	
);

unsigned int npd_dldp_sysmac_notifer
(
	void
);


unsigned int npd_dldp_global_disable
(
	void
);

unsigned int npd_dldp_recover_port_status
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
);

unsigned int npd_dldp_discard_port_status
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
);


unsigned int npd_dldp_check_vlan_status
(
	unsigned short vlanid,
	unsigned char *status
);


unsigned int npd_dldp_vlan_port_endis_flag_set
(
	unsigned short vlanId,
	unsigned int  eth_g_idx,
	unsigned char tagMode,
	unsigned char en_dis
);


unsigned int npd_dldp_vlan_endis_config
(
	unsigned char isEnable,
	unsigned short vlanid
);

unsigned int npd_dldp_recvmsg_proc
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
);

int npd_dldp_msg_init
(
	void
);


void npd_dldp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
);


extern unsigned int npd_fdb_add_dldp_vlan_system_mac
(
	unsigned short vlanId
);


extern unsigned int npd_fdb_del_dldp_vlan_system_mac
(
	unsigned short vlanId
);


#endif

