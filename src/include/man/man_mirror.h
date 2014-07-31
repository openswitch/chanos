#ifndef __DCLI_MIRROR_H__
#define __DCLI_MIRROR_H__

#define MIRROR_STR "Mirror Configuration\n"

#define MIN_MIRROR_PROFILE  (1)
#define MAX_MIRROR_PROFILE  (4)
   
/* mirror error message corresponding to error code */
extern unsigned char * dcli_mirror_err_msg[];

void dcli_mirror_init(void);
unsigned int dcli_mirror_config_profile(unsigned int profile, unsigned char add);
unsigned int dcli_mirror_delete_profile(unsigned int profile, unsigned char add);
unsigned int dcli_mirror_add_destination_port(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct);
unsigned int dcli_mirror_delete_destination_port(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct);
unsigned int dcli_mirror_add_policy_source(unsigned int profile, unsigned int ruleIndex);
unsigned int dcli_mirror_delete_policy_source(unsigned int profile, unsigned int ruleIndex);
unsigned int dcli_mirror_add_port_source(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct);
unsigned int dcli_mirror_delete_port_source(unsigned int profile, unsigned int eth_g_index, MIRROR_DIRECTION_TYPE direct);
unsigned int dcli_mirror_add_vlan_source(unsigned int profile, unsigned short vid);
unsigned int dcli_mirror_delete_vlan_source(unsigned int profile, unsigned short vid);
unsigned int dcli_mirror_add_fdb_source(unsigned int profile, unsigned short vlanId, unsigned int eth_g_index, ETHERADDR macAddr);
unsigned int dcli_mirror_delete_fdb_source(unsigned int profile, unsigned short vlanId, unsigned int eth_g_index, ETHERADDR macAddr);
unsigned int dcli_mirror_get_by_profile(unsigned int profile, struct npd_mirror_item_s *dbItem);
unsigned int dcli_mirror_get_fdb_by_profile(unsigned int profile, unsigned int *fdb_count, struct fdb_entry_item_s **static_mirror_array);


/* default ingress mirror destination port index*/
#define MIRROR_DEST_INPORT_DEFAULT	(~0UL)
/* default egress mirror destination port index*/
#define MIRROR_DEST_EGPORT_DEFAULT	MIRROR_DEST_INPORT_DEFAULT
/* invalid ingress/egress mirror vlan*/
#define MIRROR_REMOTE_VLAN_DEFAULT	(0UL)

#endif

