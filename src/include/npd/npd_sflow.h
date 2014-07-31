#ifndef __NPD_SFLOW_H__
#define __NPD_SFLOW_H__


void npd_sflow_handle_port_relate_event
(
    unsigned int vlan_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
);

void npd_sflow_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
);

#endif
