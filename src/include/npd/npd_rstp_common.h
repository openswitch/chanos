#ifndef __NPD_RSTP_COMMON_H__
#define __NPD_RSTP_COMMON_H__



/*show running cfg mem*/
#define NPD_STP_RUNNING_CFG_MEM (3*1024) 

int npd_rstp_link_change
(
	unsigned int eth_g_index,
	enum PORT_NOTIFIER_ENT	event
);
int npd_read_socket();

int npd_mstp_disable_port
(
	unsigned int port_index
);

int npd_mstp_enable_port
(
	unsigned int port_index
);

int npd_mstp_add_port
(
	unsigned short vid,
	unsigned int index
);

int npd_mstp_del_port
(
	unsigned short vid,
	unsigned int index
);

int npd_mstp_add_vlan_on_mst
(
	unsigned short vid
);

int npd_mstp_del_vlan_on_mst
(
	unsigned short vid
);
int npd_stp_struct_init();


#endif /* __FILENAME_H_ */
