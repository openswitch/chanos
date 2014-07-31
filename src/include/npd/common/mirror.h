#ifndef __COMMON_MIRROR_H__
#define __COMMON_MIRROR_H__

/* invalid ingress/egress mirror source port index*/
#define MIRROR_SRC_PORT_INVALID	(~0UL)

#define MAX_MIRROR_PROFILE		4
#define MIN_MIRROR_PROFILE		1
#define DEFAULT_MIRROR_PROFILE 	0

/* invalid ingress/egress mirror vlan*/
#define MIRROR_REMOTE_VLAN_DEFAULT	(0UL)
/* default ingress mirror destination port index*/
#define MIRROR_DEST_INPORT_DEFAULT	(~0UL)
/* default egress mirror destination port index*/
#define MIRROR_DEST_EGPORT_DEFAULT	MIRROR_DEST_INPORT_DEFAULT

typedef enum {
	MIRROR_INGRESS_E = 1,
	MIRROR_EGRESS_E,
	MIRROR_BIDIRECTION_E,
	MIRROR_DIRECT_MAX_E
}MIRROR_DIRECTION_TYPE;

typedef enum {
	MIRROR_LOCAL_E = 0,
	MIRROR_REMOTE_E,
	MIRROR_GROUP_MAX_E
}MIRROR_GROUP_TYPE;


struct npd_mirror_item_s{
	unsigned char profileId;
	unsigned int in_eth_index;
	unsigned int eg_eth_index;
	npd_pbmp_t in_eth_mbr;
	npd_pbmp_t eg_eth_mbr;
	npd_pbmp_t bi_eth_mbr;
	npd_vbmp_t vlan_mbr;
	npd_vbmp_t acl_mbr;
	unsigned short in_remote_vid;
	unsigned short eg_remote_vid;	
};



#endif

