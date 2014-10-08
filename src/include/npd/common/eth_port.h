#ifndef __COMMON_ETH_PORT_H__
#define __COMMON_ETH_PORT_H__

struct port_tx {
	unsigned int excessiveCollision;
	unsigned int goodbytesl;
	unsigned int goodbytesh;
	unsigned int uncastframe;
	unsigned int bcastframe;
	unsigned int mcastframe;
	unsigned int fcframe;
	unsigned int sentMutiple;
	unsigned int crcerror_fifooverrun;
	unsigned int sent_deferred;	
};
struct port_rx {
	unsigned int uncastframes;
	unsigned int bcastframes;
	unsigned int mcastframes;
	unsigned int fifooverruns;
	unsigned int goodbytesl;
	unsigned int goodbytesh;
	unsigned int badbytes;
	unsigned int fcframe;
	unsigned int errorframe;
	unsigned int jabber;
	unsigned int underSizepkt;
	unsigned int overSizepkt;
	unsigned int fragments;
};



struct eth_port_counter_s {
	struct port_tx_s {
		unsigned long long excessiveCollision;
        unsigned long long goodframe;
        unsigned long long totalbyte;
		unsigned long long uncastframe;
		unsigned long long bcastframe;
		unsigned long long mcastframe;
		unsigned long long fcframe;
		unsigned long long sentMutiple;
		unsigned long long crcerror_fifooverrun;
		unsigned long long sent_deferred;
        unsigned long long errorframe;
        unsigned long long discardframe;
	} tx;
	struct port_rx_s {
		unsigned long long uncastframes;
		unsigned long long bcastframes;
		unsigned long long mcastframes;
		unsigned long long fifooverruns;
        unsigned long long goodframes;
        unsigned long long totalbytes;
		unsigned long long badbytes;
		unsigned long long fcframe;
		unsigned long long errorframes;
		unsigned long long jabber;
		unsigned long long underSizepkt;
		unsigned long long overSizepkt;
		unsigned long long fragments;
        unsigned long long discardframes;
	} rx;
	struct oct_s{
		unsigned long long late_collision; 
		unsigned long long collision;
		unsigned long long badCRC;
		unsigned long long b1024oct2max;
		unsigned long long b512oct21023;
		unsigned long long b256oct511;
		unsigned long long b128oct255;
		unsigned long long b64oct127;
		unsigned long long b64oct;
	}otc;
	unsigned int linkupcount;
	unsigned int linkdowncount;
};

typedef union eth_port_stats_s
{
	unsigned long long values[64];
	struct
	{
	unsigned long long ibytes;	   /*snmpIfHCInOctets*/
	unsigned long long iucastpkts; /*snmpIfHCInUcastPkts*/
	unsigned long long imcastpkts; /*snmpIfHCInMulticastPkts*/
	unsigned long long ibcastpkts; /*snmpIfHCInBroadcastPkts*/
	unsigned long long idiscards;  /*snmpIfInDiscards*/
    unsigned long long ierrors;	   /*snmpIfInErrors*/
	unsigned long long iunknown;  /*snmpIfInUnknownProtos*/
	unsigned long long obytes;  /*snmpIfHCOutOctets*/
	unsigned long long oucastpkts; /*snmpIfHCOutUcastPkts*/
	unsigned long long omcastpkts; /*snmpIfHCOutMulticastPkts*/
	unsigned long long obcastpkts; /*snmpIfHCOutBroadcastPckts*/
	unsigned long long odiscards;  /*snmpIfOutDiscards*/ 
    unsigned long long oerrors;		/*snmpIfOutErrors*/
	unsigned long long oqlen;	   /*snmpIfOutQLen*/
	unsigned long long stpdediscards;	   /*snmpDot1dBasePortDelayExceededDiscards*/
	unsigned long long stpmediscards;	   /*snmpDot1dBasePortMtuExceededDiscards,*/
	unsigned long long stpipkts;	   /*snmpDot1dTpPortInFrames*/
	unsigned long long stpidiscards;	   /*snmpDot1dPortInDiscards*/
	unsigned long long stpopkts;		   /*snmpDot1dTpPortOutFrames,*/
    unsigned long long etherdropevents;		   /*snmpEtherStatsDropEvents*/
	unsigned long long etherundersize;		   /*snmpEtherStatsUndersizePkts*/
	unsigned long long etherfragments;	   /*snmpEtherStatsFragments*/
	unsigned long long ether64;	   /*snmpEtherStatsPkts64Octets*/
	unsigned long long ether64to127; /*snmpEtherStatsPkts65to127Octets,*/
	unsigned long long ether127to255; /*snmpEtherStatsPkts128to255Octets, */
	unsigned long long ether256to511;   /*snmpEtherStatsPkts256to511Octets,   RFC 1757 (EtherStat) */
    unsigned long long ether512to1023;       /*snmpEtherStatsPkts512to1023Octets,  RFC 1757 (EtherStat) */
    unsigned long long ether1024to1518;       /*snmpEtherStatsPkts1024to1518Octets,  RFC 1757 (EtherStat) */
    unsigned long long etheroversize;       /*snmpEtherStatsOversizePkts,          RFC 1757 (EtherStat) */
    unsigned long long etherioversize;       /*snmpEtherRxOversizePkts,            */
    unsigned long long etherooversize;       /*snmpEtherTxOversizePkts,            */
    unsigned long long etherjabbers;       /*snmpEtherStatsJabbers,               RFC 1757 (EtherStat) */
    unsigned long long ethercollisions;       /*snmpEtherStatsCollisions,            RFC 1757 (EtherStat) */
    unsigned long long ethercrcalign;       /*snmpEtherStatsCRCAlignErrors,        RFC 1757 (EtherStat) */
    unsigned long long etheronoerrors;       /*snmpEtherStatsTXNoErrors,            RFC 1757 (EtherStat) */
    unsigned long long etherinoerrors;       /*snmpEtherStatsRXNoErrors,            RFC 1757 (EtherStat) */
    unsigned long long etheralignment;       /*snmpDot3StatsAlignmentErrors,        RFC 2665 */
    unsigned long long etherfcs;       /*snmpDot3StatsFCSErrors,              RFC 2665 */
    unsigned long long dot3singcollision;       /*snmpDot3StatsSingleCollisionFrames,  RFC 2665 */
    unsigned long long dot3multicollision;       /*snmpDot3StatsMultipleCollisionFrames,  RFC 2665 */
    unsigned long long dot3sqettest;       /*snmpDot3StatsSQETTestErrors,         RFC 2665 */
    unsigned long long dot3deferredt;       /*snmpDot3StatsDeferredTransmissions,  RFC 2665 */
    unsigned long long dot3latecollisions;       /*snmpDot3StatsLateCollisions,         RFC 2665 */
    unsigned long long dot3excesscollisions;       /*snmpDot3StatsExcessiveCollisions,    RFC 2665 */
    unsigned long long dot3intermactrans;       /*snmpDot3StatsInternalMacTransmitErrors,  RFC 2665 */
    unsigned long long dot3carrisense;       /*snmpDot3StatsCarrierSenseErrors,     RFC 2665 */
    unsigned long long dot3frametoolong;       /*snmpDot3StatsFrameTooLongs,          RFC 2665 */
    unsigned long long dot3internalmacrecerrors;       /*snmpDot3StatsInternalMacReceiveErrors,  RFC 2665 */
    unsigned long long dot3symbolerrors; /*snmpDot3StatsSymbolErrors,           RFC 2665 */
    unsigned long long dot3ctrlinunknown;       /*snmpDot3ControlInUnknownOpcodes,     RFC 2665 */
    unsigned long long dot3inpause;       /*snmpDot3InPauseFrames,               RFC 2665 */
    unsigned long long dot3outpause;       /*snmpDot3OutPauseFrames,              RFC 2665 */	
    unsigned long long ipmcbridge;      /*snmpBcmIPMCBridgedPckts,             Broadcom-specific */
    unsigned long long ipmcroute;       /*snmpBcmIPMCRoutedPckts,              Broadcom-specific */
    unsigned long long ipmcidropped;       /*snmpBcmIPMCInDroppedPckts,           Broadcom-specific */
    unsigned long long ipmcodropped;       /*snmpBcmIPMCOutDroppedPckts,          Broadcom-specific */
	unsigned long linkupcount;
	unsigned long linkdowncount;
	} snmp_stats;
}eth_port_stats_t;

typedef struct _eth_port_sc_cfg_s_ {
	unsigned char ppsValid:1,
				  bpsValid:1,
				  rsvd:6;
	union{
		unsigned int pps;
		unsigned int bps;
	}value;
} eth_port_sc_cfg_t;

typedef struct _eth_port_sc_ctrl_s_ {
	eth_port_sc_cfg_t dlf;
	eth_port_sc_cfg_t bcast;
	eth_port_sc_cfg_t mcast;
} eth_port_sc_ctrl_t;

/*
 *
 * Ethernet port Main Data Structure.
 *
 */

enum PORT_RUNNING_STATE_E
{
    PORT_NORMAL,
    PORT_ONLINE_REMOVED
};

struct eth_port_s {
    unsigned int eth_port_ifindex;
	enum eth_port_type_e port_type;
    char desc[64];
    int state;
	/*
	32 bits of attributes, defined in npd_sysdef.h
	Bits 0~11 to have 12 kinds of binary attributes
	Bits 12~15 to represent 4bits 16 kinds of speed,
	Bits 16~31 reserved for future more attributes
	*/
	unsigned int attr_bitmap;
	unsigned int mtu;
	unsigned int ipg;
    unsigned int bandwidth[2]; /*for rate limit, 0 is ingress, 1 is egress*/
    unsigned int burstsize[2];

	unsigned long	lastLinkChange;/* timestamps of last link change in unit of second*/
    unsigned int switch_port_index;
    int trunkid;
    eth_port_sc_ctrl_t sc;
    int forward_mode;  /*lan switch or ip switch*/
	unsigned int vct_isable;
    unsigned int vlan_ingress_filter;
    unsigned int eee;
    int fdb_learning_mode;
    int ip_sg;  /*source guard*/
	int loopback;
	int real_speed;
	int stacking;
	int remote_unit;
	int cut_through;
	int emph_en;
	int emph_level;
	int amplitude;
	int amp_adj;
	int sc_enable;
};
 
typedef struct _eth_port_sfp_
{
	unsigned int presense;		/* */	
	unsigned int laser;			/* */
}eth_port_sfp;


enum  ETH_PORT_NOTIFIER_ENT {
	ETH_PORT_NOTIFIER_LINKUP_E = 0,
	ETH_PORT_NOTIFIER_LINKDOWN_E,
	ETH_PORT_NOTIFIER_LINKPOLL_E,	/* for eth-port link status polling message*/
	ETH_PORT_NOTIFIER_TYPE_MAX	
};

typedef int (*NPD_ETH_PORT_NOTIFIER_FUNC)(unsigned int,enum ETH_PORT_NOTIFIER_ENT);


#endif

