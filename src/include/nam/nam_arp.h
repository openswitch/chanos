#ifndef __NAM_ARP_H__
#define __NAM_ARP_H__

#define _1K 	1024

#define NEXTHOP_TABLE_SIZE	NPD_ARPSNP_TABLE_SIZE
#define ARP_MAC_TABLE_SIZE	NPD_ARPSNP_TABLE_SIZE

#define NAM_ARP_FLAG_NONE    0
#define NAM_ARP_FLAG_HIT     0x1
#define NAM_ARP_FLAG_DROP    0x2

typedef struct {
	unsigned char mac[MAC_ADDR_LEN];
}ethernet_mac;

enum nam_arp_snoop_op_ent {
	NAM_ARP_SNOOP_ADD_ITEM = 0,
	NAM_ARP_SNOOP_DEL_ITEM,
	NAM_ARP_SNOOP_UPDATE_ITEM,
	NAM_ARP_SNOOP_ACTION_MAX
};




/**********************************************************************************
 * nam_arp_table_index_init
 *
 * Initialize ARP Next-Hop table index and ARP Mac table index
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_table_index_init
(
	void
);

/**********************************************************************************
 * nam_arp_get_mactbl_index
 *
 * Get arp mac table index
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		NAM_ARP_SNOOPING_ERR_NONE - if no error occurred
 *		NAM_ARP_SNOOPING_ERR_GENERAL - if fail to get index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_get_mactbl_index
(
	index_elem	 *val
);

/**********************************************************************************
 * nam_arp_free_mactbl_index
 *
 * Turn back arp mac table index
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		0 - if no error occurred
 *		-1  - if fail to free index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_free_mactbl_index
(
	index_elem	 val
);

/**********************************************************************************
 * nam_arp_get_nexthop_tbl_index
 *
 * Get route next-hop table index
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		NAM_ARP_SNOOPING_ERR_NONE - if no error occurred
 *		NAM_ARP_SNOOPING_ERR_GENERAL - if fail to get index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_get_nexthop_tbl_index
(
	index_elem	 *val
);

/**********************************************************************************
 * nam_arp_free_nexthop_tbl_index
 *
 * Turn back next-hop table index
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		0 - if no error occurred
 *		-1  - if fail to free index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_free_nexthop_tbl_index
(
	index_elem	 val
);

/**********************************************************************************
 * nam_arp_snooping_op_item
 *
 * ARP snooping database add to/delete from ASIC or other operations.
 *
 *	INPUT:
 *		item - ARP snooping DB items
 *		action - add or delete operation
 *	
 *	OUTPUT:
 *		tblIndex - next-hop table index which used to hold next-hop item
 *
 * 	RETURN:
 *		NAM_ARP_SNOOPING_ERR_NONE - if no error occurred
 *		NAM_ARP_SNOOPING_ERR_NULL_PTR - if input parameters have null pointer
 *		NAM_ARP_SNOOPING_ERR_BADPARAM - if parameters wrong
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_snooping_op_item
(
	struct arp_snooping_item_s *dbItem,
	unsigned int action,
	unsigned int *tblIndex
);

/**********************************************************************************
 * nam_arp_snooping_get_item
 *
 * Get route next-hop table item by index
 *
 *	INPUT:
 *		devNum - device number
 *		tblIndex - table index to Next-Hop table
 *
 *	OUTPUT:
 *		val - arp snooping item info
 *
 * 	RETURN:
 *		NAM_ARP_SNOOPING_ERR_NONE - if no error occurred
 *		NAM_ARP_SNOOPING_ERR_GENERAL - if fail to get index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
int nam_arp_snooping_get_item
(
	unsigned char devNum,
	unsigned int  tblIndex,
	struct arp_snooping_item_s *val
);

/**********************************************************************************
 * nam_set_system_arpsmaccheck_enable
 *
 *	set if enable to excute smac check or not for arp
 *
 *	INPUT:
 *		isenable - binary value to indicate enable or disable
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 - if no error occurred
 *
 **********************************************************************************/

unsigned int nam_arp_smac_check_enable
(
	unsigned int isenable
);


/*****************************************************************************************************
 *									external variable or function declearation											  *
 *																												  *
 *****************************************************************************************************/

/**********************************************************************************
 * nam_arp_solicit_send
 *	Send arp solicit packet via cpss driver
 * 
 *
 *	INPUT:
 *		item - arp snooping item info
 *		sysMac - system mac address
 *		gateway - L3 interface ip address (as source ip)
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NAM_ARP_SNOOPING_ERR_NULL_PTR - if null pointer found
 *		NAM_ARP_SNOOPING_ERR_NONE - if no error occurred
 *		NAM_ARP_SNOOPING_ERR_GENERAL - if fail to get index number
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int nam_arp_solicit_send
(
	struct arp_snooping_item_s *item,
	unsigned char *sysMac,
	unsigned int gateway
);



/**********************************************************************************
 * nam_arp_aging_dest_mac_broadcast
 *
 *	set if enable to excute smac check or not for arp
 *
 *	INPUT:
 *		isBroadCast - value to indicate broadcast or unicast
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 - if no error occurred
 *
 **********************************************************************************/

unsigned int nam_arp_aging_dest_mac_broadcast
(
	unsigned int isBroadCast
);
int nam_arp_nexthop_tbl_index_get
(
    index_elem val
);
int nam_arp_mactbl_index_get
(
	index_elem val
);
unsigned long nam_arp_gratuitous_send
(
	unsigned int netif_index,
	unsigned short vid,
	unsigned char *smac,
	unsigned char *dmac,
	unsigned int ipAddress
);
unsigned int nam_vlan_arp_trap_en(unsigned short vid,unsigned int enable);

#endif
