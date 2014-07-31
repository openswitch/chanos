#ifndef __NPD_QOS_H__
#define __NPD_QOS_H__

#define QOS_POLICER_ENABLE	1
#define QOS_POLICER_DISABLE 0
#define QOS_SCH_GROUP_IS_SP	1024

#define NPD_QOS_SHOWRUN_CFG_SIZE 100*1024

#define QOS_CIR_MAX_RATE  	((10000000) *((1000) / (8)))
#define QOS_CBS_BURST 		((1000) * (1000))
#define QOS_PBS_BURST 		((1000) * (1000))
#define QOS_PIR_MAX_RATE  	((10000000) *((1000) / (8)))

#define NPD_QOS_POLICER_INDEX_VALID(x) ((x)<MAX_POLICER_NUM)
#define NPD_QOS_DSCP_VALID(x) ((x)<MAX_DSCP_DSCP_NUM)

void npd_qos_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
);

void npd_qos_handle_port_relate_event
(
    unsigned int vlan_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
);

unsigned int npd_netif_bind_poMapId_get
(
	unsigned int eth_g_index,
	unsigned int *ID
);
unsigned int npd_qos_port_bind_opt
(
	unsigned int eth_g_index,
	unsigned int policyMapIndex
);




#endif

