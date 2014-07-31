#ifndef __MAN_VRRP_H__
#define __MAN_VRRP_H__

#define PATH_LEN		(64)


struct vrrp_global_info
{
	unsigned int enable;
    unsigned int ms_d_count;
};

struct vrrp_info
{
	unsigned int vrid;
    unsigned int admin_enable;
	unsigned int state;
	unsigned int priority;
	unsigned int priority_actual;
	unsigned int advert;
	unsigned int preempt;
	char		 ifname[20];
	unsigned int naddr;
	unsigned int ipAddr[32];
	char		 track_ifname[20];
    unsigned int tracking_group;
	unsigned int reduce_priority;
	unsigned int delay_int;
    unsigned int virtual_mac_flag;
};

#endif
