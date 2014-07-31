#ifndef __DCLI_FDB_H__
#define __DCLI_FDB_H__

#define MAX_VLANID 4094
#define MIN_VLANID 1
#define MIN_SLOT   1
#define MAX_SLOT   4
#define MIN_PORT   1
#define MAX_PORT   24
#define ALIAS_NAME_SIZE 0x15

#define CPU_PORT_VIRTUAL_SLOT	 0x1F
#define CPU_PORT_VIRTUAL_PORT	 0x3F /* for CPU port (*,63)*/
#define SPI_PORT_VIRTUAL_SLOT	 CPU_PORT_VIRTUAL_SLOT
#define SPI_PORT_VIRTUAL_PORT	 0x1A /* for SPI port (*,26)*/


enum{
	port_type =0,
	trunk_type,
	vidx_type,
	vid_type
};

int dcli_fdb_set_agingtime(unsigned int agingtime);
int dcli_fdb_get_agingtime(unsigned int *agingtime);
int dcli_fdb_set_default_agingtime(unsigned int *agingtime);
int dcli_fdb_set_static(unsigned short vid, ETHERADDR * pMacAddr, unsigned int in_eth_index);
int dcli_fdb_set_trunk_static(unsigned short vid, unsigned short tid, ETHERADDR * pMacAddr);
int dcli_fdb_set_blacklist_delete(unsigned int set_flag, unsigned short vid, ETHERADDR * pMacAddr);
int dcli_fdb_set_blacklist_add(unsigned int set_flag, unsigned short vid, ETHERADDR * pMacAddr);
int dcli_fdb_set_nostatic(unsigned short vid, ETHERADDR * pMacAddr);
int dcli_fdb_set_delete_vlan(unsigned short vid);
int dcli_fdb_set_delete_port(unsigned int in_eth_g_index);
int dcli_fdb_set_static_delete_vlan(unsigned short vid);
int dcli_fdb_set_static_delete_port(unsigned int in_eth_g_index);
int dcli_fdb_set_delete_trunk(unsigned short tid);
int dcli_fdb_get_one(unsigned short vid, ETHERADDR * pMacAddr, unsigned int *num, NPD_FDB* item);
int dcli_fdb_get_count(unsigned int * num);
int dcli_fdb_get_all(unsigned int startIndex, unsigned int getNum, unsigned int *count, NPD_FDB **item_arr);
int dcli_fdb_get_all_dynamic(unsigned int startIndex, unsigned int getNum, unsigned int *count, NPD_FDB **item_arr);
int dcli_fdb_get_all_static(unsigned int *count, NPD_FDB **item_arr);
int dcli_fdb_get_all_blacklist(unsigned int *num, unsigned char **dmac_arr, unsigned char **smac_arr, NPD_FDB ** item_arr);
int dcli_fdb_config_system_fdb(unsigned int vid);
int dcli_fdb_set_number(unsigned int netif_index, int number);		 
int dcli_fdb_set_vlan_limit(unsigned short vlanid, int number);
int dcli_fdb_get_single_unit(unsigned int unit, unsigned int get_num,unsigned int *dnumber, NPD_FDB **item_arr);

     
#endif


