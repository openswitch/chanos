#ifndef __MAN_VLAN_H__
#define __MAN_VLAN_H__


//original defined in dcli_vlan.c, and zhe func was moved here so zhe define moved here
#define DBUS_VLAN_ERROR 1
#define DCLI_GET_NEXTVLAN_END -1
#define DCLI_GET_NEXTVLAN_SUCCESS 0


int dcli_vlan_member_get(unsigned short vid, unsigned char isTagged, unsigned char type, npd_pbmp_t *mbr);

int dcli_vlan_get_vname_base_info(char *name, struct vlan_s *vlanInfo);

int dcli_vlan_get_base_info(unsigned short vid, struct vlan_s *vlanInfo);

int dcli_vlan_add_del_port(boolean addel, unsigned short vlanid, unsigned int netif_index, boolean tagged, unsigned int pvid_set);

int dcli_vlan_port_pvid(boolean addel, unsigned short vlanid, unsigned int eth_g_index, boolean tagged);

int dcli_create_vlan_byid(unsigned long vid);

int dcli_delete_vlan_byid(unsigned int vid);

int dcli_create_vlan_by_id_and_name(unsigned short vid, char *vlan_name);

int dcli_config_vlan_name(unsigned short vid, char *vlan_name);

int dcli_config_layer2_vlan_entity(char *vlan_name, unsigned short *vlan_id);

int dcli_netif_port_access_mode(unsigned int netif_index, int mode);

int dcli_vlan_private( unsigned int vid, int is_enabled);

int dcli_netif_private(unsigned int netif_index,  int mode);

int dcli_vlan_assoc_mac(unsigned int vid,  ETHERADDR *macAddr, int isAdd);

int  dcli_vlan_show_associate_mac(unsigned int vlanId,  ETHERADDR *macAddr);

int dcli_vlan_assoc_subnet(unsigned int vid, unsigned long ipno, unsigned long ipmask, int isAdd);

int dcli_switchport_cfg_subnetvlan(unsigned int netif_index, int isEnable, int preferred);

int dcli_switchport_cfg_macvlan(unsigned int netif_index, int isEnable, int preferred);

int dcli_netif_qinq_mode_set(unsigned int isEnable, unsigned int netif_index);

int dcli_netif_qinq_miss_drop(int isEnable, unsigned int netif_index);

int dcli_vlan_xlate_cfg(unsigned int netif_index, unsigned int isEgress, unsigned int isAdd, vlan_xlate_db_entry_t *entry);

int dcli_outer_tpid_add(unsigned int netif_index, int isSet, unsigned short outer_tpid);

int dcli_get_next_vlanid(unsigned short in_vid, unsigned short *out_vid);

int dcli_add_del_subvlan(unsigned char isAdd, unsigned char isTagged, unsigned short subvlanId, unsigned short vlanId);

int dcli_config_add_del_prot_ethertype(unsigned char isAdd, unsigned int frame_type, unsigned int groupid, unsigned short etherType);

int dcli_config_vlan_add_del_prot_ethertype(unsigned int isAdd, unsigned int netif_index, unsigned int groupid, unsigned short vlanId);

int dcli_config_vlan_active_protvlan(unsigned short protvlanId, unsigned short *out_protvlanId);

int dcli_config_vlan_active_supervlan(unsigned short supervid, unsigned short *out_supervid);

int dcli_show_supervlan(unsigned short vlanId, unsigned short *out_vlan_id);

int dcli_set_port_vlan_ingress_filter(unsigned int eth_g_index, unsigned char enDis);

int dcli_show_port_pvid(unsigned int eth_g_index, unsigned short *pvid);

int dcli_show_port_list_pvid(DBusMessageIter	 *iter);

int dcli_del_vlan_by_id(unsigned short vlanId);

int dcli_del_vlan_by_name(char *vlanName);

int dcli_config_vlan_filter(unsigned short vlanId, unsigned int vlanfilterType, unsigned int en_dis);

int dcli_privlan_member_get(unsigned short vid, unsigned char pri_mode, unsigned char type, npd_pbmp_t *mbr);

int wp_show_vlan_member_name
(
	unsigned char member_type, 
	unsigned char show_type,
	npd_pbmp_t member, 
	char member_name[][64],
	unsigned int *member_counter
);
int dcli_check_vlan_exist(unsigned short vlan_id);
int dcli_create_vlan_intf(unsigned short vlan_id);
int dcli_delete_vlan_intf(unsigned short vlan_id);
int dcli_get_mac_based_vlan_number(void);

#endif
