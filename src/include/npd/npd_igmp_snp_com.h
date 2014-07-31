#ifndef __NPD_IGMP_SNP_COM_H__
#define __NPD_IGMP_SNP_COM_H__


#define NPD_IGMP_SNP_HASHTBL_NAME   "npdIgmpSnpHashTbl"
#define NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM (64*1024)



int npd_igmp_sysmac_notifer
(
	void
);

int npd_igmp_snp_recvmsg_proc
(
	struct igmp_notify_mod_npd* igmp_notify
);


int npd_vlan_igmp_snp_endis_config
(
	unsigned char	isAdd,
	unsigned short	vid
);

int npd_vlan_port_igmp_snp_endis_config
(
	unsigned char enable,
	unsigned short vid,
	unsigned int eth_g_idx,
	unsigned char tagMode
);

int npd_check_igmp_snp_status
(
	unsigned char * status
);

int npd_check_igmp_snp_vlan_status
(
	unsigned short vlanId,
	unsigned char *status
);

void npd_igmpsnp_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);

void npd_igmpsnp_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);


#endif
