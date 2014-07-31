#ifndef __NPD_IGMP_API_H__
#define __NPD_IGMP_API_H__

void igmp_snp_init
(
	void
);


int npd_igmp_snp_msg_init(void);

void l2_mc_index_alloc(unsigned int *index);

void l2_mc_index_free(unsigned int index);
void l3_mc_index_alloc(unsigned int *index);

void l3_mc_index_free(unsigned int index);

void l2_mc_index_get(unsigned int index);

void l3_mc_index_get(unsigned int index);

int npd_check_igmp_snp_vlan_status
(
	unsigned short vlanId,
	unsigned char *status
);
#endif

