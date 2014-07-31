#ifndef __NPD_DHCP_SNP_COM_H__
#define __NPD_DHCP_SNP_COM_H__




void npd_dhcp_snp_save_global_cfg
(
	unsigned char *buf,
	unsigned int bufLen
);


void npd_dhcp_snp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen
);



void npd_dhcp_snp_save_bind_table
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
);





#endif



