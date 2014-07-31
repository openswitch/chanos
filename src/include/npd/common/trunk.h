#ifndef __COMMON_TRUNK_H__
#define __COMMON_TRUNK_H__

#define TRUNK_MEMBER_NUM_MAX    0x8

#define NPD_TRUNK_IFNAME_SIZE 0x15

/*
  *
  * we can have 127 trunks at most
  * while trunk ID 0 means this is not a trunk
  * trunk ID 1~127 are legal value for a trunk
  *
  * each trunk support 8 port members at most
  *
  */
#define  NPD_TRUNKID_NOT_TRUNK	0
#define  NPD_TRUNKID_START		1

#define  NPD_TRUNK_PORT_NUMBER_PER	8
typedef enum {
	LOAD_BANLC_SRC_DEST_MAC = 0 ,
	LOAD_BANLC_SOURCE_DEV_PORT ,
	LOAD_BANLC_SRC_DEST_IP,
	LOAD_BANLC_TCP_UDP_RC_DEST_PORT ,
	LOAD_BANLC_MAC_IP,
	LOAD_BANLC_MAC_L4,
	LOAD_BANLC_IP_L4 ,
	LOAD_BANLC_MAC_IP_L4,
	LOAD_BANLC_MAX
}trkLoadBanlcMode;

/*
 *
 * Trunk Main Data Structure.
 *
 */
typedef struct trunk_s{
    unsigned int trunk_id;
    char name[31];
    unsigned int g_index;
    unsigned int mtu;
    unsigned int linkstate;
    npd_pbmp_t   ports;
    unsigned int master_port_index;
    unsigned int switch_port_index;
    int load_balance_mode;
    int forward_mode;
    unsigned short aggregator_mode;
	int vlag_cascade_state;
}trunk_t;




#endif

