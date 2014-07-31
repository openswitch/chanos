#ifndef __MACRO_ETH_H__
#define __MACRO_ETH_H__

enum eth_port_type_e {
	ETH_INVALID,    /* invalid for no iocard presented, port index 0 when start with 1, and initializing usage.*/
	ETH_FE_TX,  /* 100M copper */
	ETH_FE_FIBER, /* 100M FIBER */
	ETH_GTX, /* GE copper */
	ETH_GE_FIBER, /* GE fiber */
	ETH_GE_SFP,
	ETH_GE_COMBO,
	ETH_XGE_XFP,
	ETH_XGTX, /* XGE Copper */
	ETH_XGE_FIBER,
	ETH_XGE_SFPPLUS,
	ETH_XGE_QSFP,
	ETH_40G_QSFP,
	ETH_40G_CFP,
	ETH_100G_CFP,
	ETH_100G_CXP,
	ETH_MNG,
	ETH_MAX
};

#define ETH_ATTR_ON 1
#define ETH_ATTR_OFF 0
#define ETH_ATTR_ENABLE 1
#define ETH_ATTR_DISABLE 0
#define ETH_ATTR_LINKUP 1
#define ETH_ATTR_LINKDOWN 0
#define ETH_ATTR_DUPLEX_FULL 0
#define ETH_ATTR_DUPLEX_HALF 1
#define ETH_ATTR_AUTONEG_DONE 1
#define ETH_ATTR_AUTONEG_NOT_DONE 0
#define ETH_ATTR_FC_ENABLE 1
#define ETH_ATTR_FC_DISABLE 0
#define ETH_ATTR_AUTONEG_SPEEDON 1 
#define ETH_ATTR_AUTONEG_SPEEDDOWN 0
#define ETH_ATTR_AUTONEG_FCON  1  
#define ETH_ATTR_AUTONEG_FCDOWN  0
#define ETH_ATTR_AUTONEG_DUPLEX_ENABLE 1 
#define ETH_ATTR_AUTONEG_DUPLEX_DISABLE 0 

#define ETH_ATTR_EEE_ENABLE 1
#define ETH_ATTR_EEE_DISABLE 0

#define ETH_ATTR_BP_ENABLE 1
#define ETH_ATTR_BP_DISABLE 0
#define ETH_ATTR_MEDIA_EXIST_PRIO  1
#define ETH_ATTR_MEDIA_NOT_EXIST_PRIO 0

/* Bits 0~11 to have 12 kinds of binary attributes */
#define ETH_ATTR_ADMIN_STATUS 	0x1	/* bit0 */
#define ETH_ATTR_LINK_STATUS 	0x2	/* bit1 */
#define ETH_ATTR_AUTONEG 	0x4	/* bit2: port auto-negotiation status */
#define ETH_ATTR_DUPLEX 	0x8	/* bit3 */
#define ETH_ATTR_FLOWCTRL 	0x10	/* bit4 */
#define ETH_ATTR_BACKPRESSURE 	0x20	/* bit5 */
#define ETH_ATTR_AUTONEG_SPEED 0x40	/* bit6 */
#define ETH_ATTR_AUTONEG_DUPLEX 0x80	/* bit7 */
#define ETH_ATTR_AUTONEG_FLOWCTRL 0x100	/* bit8 */
#define ETH_ATTR_AUTONEG_CTRL	0x200 	/* bit9 */

#define ETH_ADMIN_STATUS_BIT	0x0
#define ETH_LINK_STATUS_BIT	0x1
#define ETH_AUTONEG_BIT		0x2
#define ETH_DUPLEX_BIT		0x3
#define ETH_FLOWCTRL_BIT	0x4
#define ETH_BACKPRESSURE_BIT	0x5
#define ETH_AUTONEG_SPEED_BIT 	0x6
#define ETH_AUTONEG_DUPLEX_BIT 	0x7
#define ETH_AUTONEG_FLOWCTRL_BIT 0x8
#define ETH_AUTONEG_CTRL_BIT	0x9
#define ETH_PREFERRED_COPPER_MEDIA_BIT  0x1c
#define ETH_PREFERRED_FIBER_MEDIA_BIT 0x1d

/* Bits 12~15 to represent 4bits 16 kinds of speed */
#define ETH_ATTR_SPEED_MASK 		0xF000
#define ETH_ATTR_SPEED_10M 		0x0
#define ETH_ATTR_SPEED_100M 		0x1
#define ETH_ATTR_SPEED_1000M 		0x2
#define ETH_ATTR_SPEED_10G 		0x3
#define ETH_ATTR_SPEED_12G 		0x4
#define ETH_ATTR_SPEED_2500M 		0x5
#define ETH_ATTR_SPEED_5G			0x6
#define ETH_ATTR_SPEED_40G			0x7
#define ETH_ATTR_SPEED_100G			0x8
#define ETH_ATTR_SPEED_MAX 		0xF

#define ETH_SPEED_BIT				0xC

/* preferred media 28~29 */
#define ETH_ATTR_PREFERRED_COPPER_MEDIA		0x10000000
#define ETH_ATTR_PREFERRED_FIBER_MEDIA 0x20000000
#define ETH_ATTR_REAL_COPPER_MEDIA		0x40000000
#define ETH_ATTR_REAL_FIBER_MEDIA 0x80000000

#define ETH_ATTR_DEFAULT_MINIMUM_IPG 12

/* Bits 30~31 reserved */

typedef enum {
	COMBO_PHY_MEDIA_PREFER_NONE = 0,
	COMBO_PHY_MEDIA_PREFER_FIBER,
	COMBO_PHY_MEDIA_PREFER_COPPER
}COMBO_PHY_MEDIA;

typedef struct {
	unsigned char arEther[6];
}ETHERADDR;

typedef enum {
	PORT_FULL_DUPLEX_E,
	PORT_HALF_DUPLEX_E
} PORT_DUPLEX_ENT;

typedef enum {
	PORT_SPEED_10_E,
	PORT_SPEED_100_E,
	PORT_SPEED_1000_E,
	PORT_SPEED_10000_E,
	PORT_SPEED_12000_E,
	PORT_SPEED_2500_E,
	PORT_SPEED_5000_E,
	PORT_SPEED_40G_E,
	PORT_SPEED_100G_E
}PORT_SPEED_ENT;


typedef enum{
	NPD_PORT_TYPE_NONE=0,
	NPD_PORT_TYPE_SWITCH,
	NPD_PORT_TYPE_ROUTE,
	NPD_PORT_TYPE_PROMISCUOUS
}NPD_PORT_MODE;

typedef enum{
   ETH_PORT_STREAM_PPS_E,
   ETH_PORT_STREAM_BPS_E,
   ETH_PORT_STREAM_INVALID_E
}ETH_PORT_STREAM_MODE_E;

#define	PORT_STORM_CONTROL_STREAM_DLF            0x01
#define	PORT_STORM_CONTROL_STREAM_MCAST          0x02
#define	PORT_STORM_CONTROL_STREAM_BCAST          0x04
#define	PORT_STORM_CONTROL_STREAM_UCAST          0x08  
#define	PORT_STORM_CONTROL_STREAM_ALL          (PORT_STORM_CONTROL_STREAM_DLF | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST | \
	                                            PORT_STORM_CONTROL_STREAM_MCAST| \
	                                            PORT_STORM_CONTROL_STREAM_UCAST) 

/**********************************************************************************
 *
 *		set port default attribute value
 *
 **********************************************************************************/
 #define ETH_PORT_ADMIN_STATUS_DEFAULT	ETH_ATTR_ENABLE
 #define CRSMU_RGMII_PORT_MRU_DEFAULT	1518
 #define ETH_PORT_MRU_DEFAULT_CPU    1518
 #define ETH_PORT_MRU_DEFAULT	  		1522
 #define ETH_PORT_DUPLEX_DEFAULT 		PORT_FULL_DUPLEX_E
 #define ETH_PORT_SPEED_DEFAULT			PORT_SPEED_1000_E
 #define ETH_PORT_AN_DEFAULT			ETH_ATTR_ON 	/* all Auto-Nego options  */
 #define ETH_PORT_AN_DUPLEX_DEFAULT		ETH_ATTR_ON		/* duplex mode Auto-Nego*/	
 #define ETH_PORT_AN_FC_DEFAULT			ETH_ATTR_ON		/* flow control Auto-Nego*/
 #define ETH_PORT_AN_SPEED_DEFAULT		ETH_ATTR_ON		/* port speed Auto-Nego*/
 #define ETH_PORT_FC_STATE_DEFAULT		ETH_ATTR_ENABLE /* flow control state*/
 #define ETH_PORT_BP_STATE_DEFAULT		ETH_ATTR_DISABLE /* back-pressure state*/
 #define ETH_PORT_MEDIA_DEFAULT			ETH_ATTR_MEDIA_NOT_EXIST_PRIO /* media status*/
 #define ETH_PORT_LED_INTF_DEFAULT		ETH_ATTR_ENABLE
 #define ETH_PORT_IPG                   12

typedef enum {
    PORT_EEE_DISABLE,
	PORT_EEE_ENABLE
} PORT_EEE_ENT;

#define MAC_ADDR_LEN 6

enum eth_port_func_index_e {
	ETH_PORT_FUNC_PORT_BASED_VLAN,			/* untagged vlan */
	ETH_PORT_FUNC_DOT1Q_BASED_VLAN,			/* tagged vlan */
	ETH_PORT_FUNC_PROTOCOL_BASED_VLAN,		/* protocol-based vlan */
	ETH_PORT_FUNC_SUBNET_BASED_VLAN,		/* subnet-based vlan */
	ETH_PORT_FUNC_DOT1AD_EDGE, /* to be studied later on provider bridge functions. */
	ETH_PORT_FUNC_BRIDGE,	/* switch mode */
	ETH_PORT_FUNC_DOT1W,	/* spanning-tree protocol */
	ETH_PORT_FUNC_VLAN_TRANSLATION,
	ETH_PORT_FUNC_LINK_AGGREGATION,			/* trunk */
	ETH_PORT_FUNC_IPV4,				/* L3 interface route mode */
	ETH_PORT_FUNC_ACL_RULE,				/* ACL */
	ETH_PORT_FUNC_QoS_PROFILE,			/* QoS profile */
	ETH_PORT_FUNC_MODE_PROMISCUOUS,          	/* pve profile */
	ETH_PORT_FUNC_SUBIF,   				/* port have or not subif */
	ETH_PORT_FUNC_IGMP_SNP,				/* port enabled/disabled IGMP snooping Protocal */
	ETH_PORT_FUNC_PVE,   				/* private vlan info */
	ETH_PORT_FUNC_QoS_SHAPE,			/* qos shape */
	ETH_PORT_FUNC_FDB, 				/* port fdb setting */
	ETH_PORT_FUNC_STORM_CONTROL,
	ETH_PORT_FUNC_DLDP, 				/* port enabled/disabled DLDP Protocal */
	ETH_PORT_FUNC_DHCP_SNP,				/* DHCP-Snooping	*/
	ETH_PORT_FUNC_VCT_INFO,				/*port vct enable or disabe*/
	ETH_PORT_FUNC_MAX
};

#define ETH_PORT_FUNC_BITMAP_FLAG(func_index) (0x1 << (func_index))

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

typedef unsigned char eth_mac_t[6];
#define 	MAC_ADDRESS_LEN	 6

#endif
