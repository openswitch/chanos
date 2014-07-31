#ifndef __COMMON_IGMP_H__
#define __COMMON_IGMP_H__


typedef struct igmp_snp_item_s{
	unsigned int vlanId;
	unsigned int vidx;
	unsigned int groupip;
	npd_pbmp_t   vidxmbr;
} IGMP_SNP_ITEM;

struct npd_igmpsnp_cfg_s
{
	unsigned char npdIgmpSnpEnDis;
	unsigned char npdIgmpSnp_PortFilter;
    npd_vbmp_t vlan_admin_status;
    unsigned char  igmp_snp_port_admin_state[MAX_SWITCHPORT_PER_SYSTEM];
    unsigned short switch_port_control_count[MAX_SWITCHPORT_PER_SYSTEM];
};

struct npd_mldsnp_cfg_s
{
	unsigned char npdMldSnpEnDis;
	unsigned char npdMldSnp_PortFilter;
    npd_vbmp_t vlan_admin_status;
    unsigned short switch_port_control_count[MAX_SWITCHPORT_PER_SYSTEM];
};



#endif

