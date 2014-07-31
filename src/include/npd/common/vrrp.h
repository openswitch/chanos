#ifndef __COMMON_NPD_VRRP_H__
#define __COMMON_NPD_VRRP_H__

/*NPD VRRP struct definition*/
typedef struct npd_vrrp_db_s
{
	unsigned int state;
	unsigned int l3index;		
	unsigned char hwaddr[6];	
}npd_vrrp_db_t;

int npd_vrrp_msg_init();


#endif

