#ifndef __NAM_ACL_H__
#define __NAM_ACL_H__

#define MAX_IP_STRLEN	16
#define MAX_MAC_STRLEN  18
#define MAX_PORT_NUMBER 24
#define MAX_GROUP_NUM 1024

#define MAX_ACL_RULE_NUMBER	1000
#define MAX_CFG_LEN 100
#define MAX_VID_IDNUM 4096   /*vid 0-4095*/
#define MAX_TRUNK_IDNUM 256  /*trunkid (0-255)*/
#define MAX_PORT_IDNUM 24    /*port 0-23*/
#define MAX_VLANID_IDNUM 4096 /*vlanId*/
#define ACL_SAVE_FOR_UDP_BPDU 12

#define ACCESS_PORT_TYPE	0
#define ACCESS_VID_TYPE		1
#define STANDARD_ACL_RULE	0
#define EXTENDED_ACL_RULE	1

#define	VOICE_ENTRY_ID_BASE	3000

enum {
	NAM_ACL_GRP_CLASSMAP_VFP = 2,
	NAM_ACL_GRP_CLASSMAP_IFP,
	NAM_ACL_GRP_CLASSMAP_EFP,
	NAM_ACL_GRP_VOICE_VLAN = 5,
	NAM_ACL_GRP_CLASSMAP_IP6 = 6,
	NAM_ACL_GRP_SERVICE_MAX
};

#define ACL_SWAB16(x) \
		(x = ((unsigned int)( \
		(((unsigned int)(x) & (unsigned int)0x00ffU) << 8) | \
		(((unsigned int)(x) & (unsigned int)0xff00U) >> 8) )))

#define ACL_SWAB32(x) \
		(x = ((unsigned int)( \
		(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
		(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
		(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
		(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) )))


extern unsigned long appDemoPpConfigDevAmount;


/********************************************************************************
* cpss
*
* DESCRIPTION:
*       CPSS generic PCL lib API implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 1.30 $
*******************************************************************************/
/*
 * Typedef: struct GT_BYTE_ARRY
 *
 * Description:
 *    This structure contains byte array data and length.
 *
 * Fields:
 *    data   - pointer to allocated data buffer
 *    length - buffer length
 */

 typedef enum
 {
	 ACL_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E = 0,
	 ACL_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
	 ACL_PACKET_ATTRIBUTE_MODIFY_ENABLE_E,
	 ACL_PACKET_ATTRIBUTE_MODIFY_INVALID_E
 } ACL_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT;


unsigned int nam_qos_policer_entry_set
(
	unsigned int 		policerIndex,
	QOS_POLICER_STC 	*policer
);


unsigned int nam_qos_set_counter
(
    unsigned int    policerIndex,
	unsigned int    counterSetIndex,
	int isEnable
);

unsigned int nam_qos_read_counter
(
    unsigned int policerId,
	unsigned int   counterSetIndex,
	unsigned long long *conform_bytes,
	unsigned long long *conform_pkts,
	unsigned long long *exceed_bytes,
	unsigned long long *exceed_pkts,
	unsigned long long *violate_bytes,
	unsigned long long *violate_pkts
);

unsigned int nam_qos_policer_init();

unsigned int nam_qos_policer_cir_cbs_check
(
	unsigned int  cir,
	unsigned int  cbs
);

unsigned int nam_acl_drv_mirror_action_update
(
	unsigned int ruleIndex,
	unsigned int enable,
	int netif_index
);

#endif

