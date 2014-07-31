#ifndef __NPD_TRUNK_H__
#define __NPD_TRUNK_H__

#define CHECK_DEV_NO_ISLEGAL(dev_no) 			(0 == dev_no || 1 == dev_no)
#define CHECK_VIRPORT_NO_ISLEGAL(virport_no)	(virport_no <=29) 
#define TRUNK_MEMBER_NUM_MAX		0x8
#define TRUNK_CONFIG_SUCCESS	0x0
#define	TRUNK_CONFIG_FAIL		0xff

/*************/
#define TRUNK_PORT_EXISTS_GTDB	0xf		/*port already exists in trunkDB*/
#define TRUNK_PORT_MBRS_FULL	(TRUNK_PORT_EXISTS_GTDB+1)


#define NPD_MAX_TRUNK_ID		127
#define NPD_TRUNK_RUNNING_CFG_MEM	1024*1024

enum trunk_status_e {
	TRUNK_STATE_DOWN_E = 0,
	TRUNK_STATE_UP_E,
	TRUNK_STATE_MAX
};



#endif
