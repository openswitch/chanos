#ifndef __NPD_VLAN_H__
#define __NPD_VLAN_H__

#define	ALIAS_NAME_SIZE			0x15
#define NPD_VLAN_RUNNING_CFG_MEM (1024*1024*3)

/*show running cfg mem*/

enum vlan_status_e {
	VLAN_STATE_DOWN_E = 0,
	VLAN_STATE_UP_E,
	VLAN_STATE_MAX
};



#endif /* _NPD_VLAN_H*/
