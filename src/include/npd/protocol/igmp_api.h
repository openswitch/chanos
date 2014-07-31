#ifndef __PROTOCOL_IGMP_API_H__
#define __PROTOCOL_IGMP_API_H__

#define IGMP_SNP_TYPE_DEVICE_EVENT	3	/*device message*/

struct npd_dev_event_cmd{
	unsigned long event_type;
	unsigned short vid;
	unsigned int port_index;/*ifindex*/
};

/**device event ****/
#define NPD_IGMPSNP_EVENT_DEV_UP			(0x0001)
#define NPD_IGMPSNP_EVENT_DEV_DOWN			(0x0002)
#define NPD_IGMPSNP_EVENT_DEV_REGISTER		(0x0003)
#define NPD_IGMPSNP_EVENT_DEV_UNREGISTER	(0x0004)
#define NPD_IGMPSNP_EVENT_DEV_VLANADDIF		(0x0005)
#define NPD_IGMPSNP_EVENT_DEV_VLANDELIF		(0x0006)
#define NPD_IGMPSNP_EVENT_DEV_DELETE        (0x0007)
#define NPD_IGMPSNP_EVENT_DEV_MCROUTER_PORT_UP	 (0x000f)
#define NPD_IGMPSNP_EVENT_DEV_MCROUTER_PORT_DOWN (0x0010)
#define NPD_IGMPSNP_EVENT_DEV_SYS_MAC_NOTI		 (0x0011) 	
#define	IGMP_SNP_TYPE_NOTIFY_MSG	1	/*notify message*/
#define	IGMP_SNP_FLAG_ADDR_MOD		3	/*notify information for modify address*/


#define IGMP_SYS_SET_INIT	1
#define IGMP_SYS_SET_STOP	2
#define IGMP_SYS_SET_PORT_FILTER_ENABLE 3
#define IGMP_SYS_SET_PORT_FILTER_DISABLE 4

enum igmpmodaddtype
{
	IGMP_ADDR_ADD,
	IGMP_ADDR_DEL,
	IGMP_ADDR_RMV,
	PIM_ADDR_ADD,
	PIM_ADDR_DEL,
	IGMP_SYS_SET,
	MLD_ADDR_ADD,
	MLD_ADDR_DEL,
	MLD_ADDR_RMV,
	MLD_SYS_SET,
	PIM_MODE_SET
};

#define IGMP_VLAN_ADD_PORT	13
#define	IGMP_VLAN_DEL_PORT	14	/*VLANÉ¾³ý¶Ë¿Ú*/
#define	IGMP_PORT_DOWN		15	/*¶Ë¿Údown*/
#define	IGMP_VLAN_DEL_VLAN	16	/**/

typedef struct 
{
	unsigned long	event;
	long	vlan_id;
	unsigned long	ifindex;
	unsigned char 	sys_mac[ETH_ALEN];
	unsigned long	reserve;
}dev_notify_msg;

struct npd_mng_igmp{
		struct nlmsghdr nlh;
		dev_notify_msg npd_dev_mng;
};


struct igmp_notify_mod_npd{
	unsigned long	mod_type;
	long	vlan_id;
	unsigned long	ifindex;
	unsigned long	groupadd;	
    unsigned long   source;
    unsigned long	reserve;
#ifdef HAVE_NPD_IPV6
	ip6_addr        srcaddr;
	ip6_addr		grpaddr;
#endif
};

struct igmp_msg_npd{
		struct nlmsghdr nlh;
		struct igmp_notify_mod_npd igmp_noti_npd;
};



#endif

