#ifndef __NPD_DHCP_SNP_TBL_H__
#define __NPD_DHCP_SNP_TBL_H__

#define NPD_DHCP_SNP_HASH_TABLE_SIZE    256


#define NPD_DHCP_SNP_DBTABLE_SIZE 512
#define NPD_DHCP_SNP_DBHASH_MAC_SIZE 128
#define NPD_DHCP_SNP_DBHASH_PORT_SIZE 128
#define NPD_DHCP_SNP_DBTABLE_NAME "npd_dhcp_snp_dbtable"
#define NPD_DHCP_SNP_DBHASH_INDEX_NAME_MAC "npd_dhcp_snp_dbhash_mac_index"
#define NPD_DHCP_SNP_DBHASH_INDEX_NAME_PORT "npd_dhcp_snp_dbhash_port_index"
#define NPD_DHCP_SNP_GLOBAL_STATUS_INDEX_NAME "npd_dhcp_global_status_index"
#define NPD_DHCP_SNP_GLOBAL_STATUS_NAME "dhcp_snp_global_status_table"
#define NPD_DHCP_SNP_GLOBAL_STATUS_SIZE 1
#define NPD_DHCP_SNP_INDEX_SIZE 256
//#define NPD_DHCP_SNP_PORT_SIZE 128
#define NPD_DHCP_SNP_STATUS_ITEM_INDEX_NAME "npd_dhcp_status_item_index"
//#define NPD_DHCP_SNP_STATUS_ITEM_PORT_INDEX_NAME "npd_dhcp_status_index_by_port"
#define NPD_DHCP_SNP_STATUS_ITEM_NAME "dhcp_status_item_table"
#define NPD_DHCP_SNP_STATUS_ITEM_SIZE 4096

#define NPD_DHCP_SNP_SR_BUFFER_SIZE (NPD_DHCP_SNP_DBTABLE_SIZE * sizeof(" dhcp-snooping binding 255.255.255.255 00:00:00:00:00:00 1 10/10/10 864000000 \n"))

unsigned int npd_dhcp_snp_tbl_hash
(
	unsigned char *mac
);

void npd_dhcp_snp_update_logfile
(
	void
);






unsigned int npd_dhcp_snp_status_item_key 
(
	void *data

);

unsigned int npd_dhcp_snp_item_hash_status_compare
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *itemA,
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *itemB
);

#endif

