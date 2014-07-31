#ifndef __NPD_LACP_H__
#define __NPD_LACP_H__

#ifndef __constant_htons
#define __constant_htons(x)  htons(x)
#endif
/* General definitions */
#define NPD_LACP_CFGTBL_NAME    "npdLacpCfgTbl"


#define BOND_ETH_P_LACPDU       0x8809
#define ALIAS_NAME_SIZE 0x15

#define PKT_TYPE_LACPDU         __constant_htons(BOND_ETH_P_LACPDU)
#define AD_TIMER_INTERVAL       100 /*msec*/

#define MULTICAST_LACPDU_ADDR    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02}
#define AD_MULTICAST_LACPDU_ADDR    {MULTICAST_LACPDU_ADDR}

#define AD_LACP_SLOW 0
#define AD_LACP_FAST 1
#define AD_LACP_DISABLE 0
#define AD_LACP_ENABLE 1
#define AGGREGATOR_ID 4096

#define ETH_ALEN 		6

extern sequence_table_index_t * lacpport_actor_index;
struct npd_lacp_cfg_s
{
    unsigned int   lacp_enable;
};

typedef struct _mac_addr_
{
    unsigned char mac_addr_value[MAC_ADDR_LEN];
} mac_addr_t;

typedef enum
{
    AD_BANDWIDTH = 0,
    AD_COUNT
} agg_selection_t;

/* rx machine states(43.4.11 in the 802.3ad standard) */
typedef enum
{
    AD_RX_DUMMY,
    AD_RX_INITIALIZE,     /* rx Machine */
    AD_RX_PORT_DISABLED,  /* rx Machine */
    AD_RX_LACP_DISABLED,  /* rx Machine */
    AD_RX_EXPIRED,	      /* rx Machine */
    AD_RX_DEFAULTED,      /* rx Machine */
    AD_RX_CURRENT	      /* rx Machine */
} rx_states_t;

/* periodic machine states(43.4.12 in the 802.3ad standard) */
typedef enum
{
    AD_PERIODIC_DUMMY,
    AD_NO_PERIODIC,	       /* periodic machine */
    AD_FAST_PERIODIC,      /* periodic machine */
    AD_SLOW_PERIODIC,      /* periodic machine */
    AD_PERIODIC_TX	   /* periodic machine */
} periodic_states_t;

/* mux machine states(43.4.13 in the 802.3ad standard) */
typedef enum
{
    AD_MUX_DUMMY,
    AD_MUX_DETACHED,       /* mux machine */
    AD_MUX_WAITING,	       /* mux machine */
    AD_MUX_ATTACHED,       /* mux machine */
    AD_MUX_COLLECTING_DISTRIBUTING /* mux machine */
} mux_states_t;
/* timers types(43.4.9 in the 802.3ad standard) */
typedef enum
{
    MANUAL_MODE,
    STATIC_MODE,
    DYNAMIC_MODE
} aggregator_mode_t;
/* tx machine states(43.4.15 in the 802.3ad standard) */
typedef enum
{
    AD_TX_DUMMY,
    AD_TRANSMIT	   /* tx Machine */
} tx_states_t;

/* rx indication types */
typedef enum
{
    AD_TYPE_LACPDU = 1,    /* type lacpdu */
    AD_TYPE_MARKER	   /* type marker */
} pdu_type_t;

/* rx marker indication types*/
typedef enum
{
    AD_MARKER_INFORMATION_SUBTYPE = 1, /* marker imformation subtype */
    AD_MARKER_RESPONSE_SUBTYPE,     /* marker response subtype */
    AD_ETH_OAM_SUBTYPE     /* ethernet OAM subtype */
} marker_subtype_t;

/* timers types(43.4.9 in the 802.3ad standard) */
typedef enum
{
    AD_CURRENT_WHILE_TIMER,
    AD_ACTOR_CHURN_TIMER,
    AD_PERIODIC_TIMER,
    AD_PARTNER_CHURN_TIMER,
    AD_WAIT_WHILE_TIMER
} dot3ad_timers_t;

/* timers types(43.4.9 in the 802.3ad standard) */
typedef enum
{
    AD_PASSIVE,
    AD_ACTIVE
} lacp_mode_t;

#pragma pack(1)

typedef struct dot3ad_header
{
    mac_addr_t destination_address;
    mac_addr_t source_address;
    //unsigned long dot1q_tag;
    unsigned short length_type;
} dot3ad_header_t;

/* Link Aggregation Control Protocol(LACP) data unit structure(43.4.2.2 in the 802.3ad standard) */
typedef struct lacpdu
{
    unsigned char subtype;		     /* = LACP(= 0x01) */
    unsigned char version_number;
    unsigned char tlv_type_actor_info;	      /* = actor information(type/length/value) */
    unsigned char actor_information_length; /* = 20 */
    unsigned short actor_system_priority;
    mac_addr_t actor_system;
    unsigned short actor_key;
    unsigned short actor_port_priority;
    unsigned short actor_port;
    unsigned char actor_state;
    unsigned char reserved_3_1[3];	     /* = 0 */
    unsigned char tlv_type_partner_info;     /* = partner information */
    unsigned char partner_information_length;	 /* = 20 */
    unsigned short partner_system_priority;
    mac_addr_t partner_system;
    unsigned short partner_key;
    unsigned short partner_port_priority;
    unsigned short partner_port;
    unsigned char partner_state;
    unsigned char reserved_3_2[3];	     /* = 0 */
    unsigned char tlv_type_collector_info;	  /* = collector information */
    unsigned char collector_information_length; /* = 16 */
    unsigned short collector_max_delay;
    unsigned char reserved_12[12];
    unsigned char tlv_type_terminator;	     /* = terminator */
    unsigned char terminator_length;	     /* = 0 */
    unsigned char reserved_50[50];	     /* = 0 */
} lacpdu_t;

typedef struct lacpdu_header
{
    struct dot3ad_header dot3ad_header;
    struct lacpdu lacpdu;
} lacpdu_header_t;

/* Marker Protocol Data Unit(PDU) structure(43.5.3.2 in the 802.3ad standard) */
typedef struct marker
{
    unsigned char subtype;		 /*  = 0x02  (marker PDU) */
    unsigned char version_number;	 /*  = 0x01 */
    unsigned char tlv_type;		 /*  = 0x01  (marker information) */
    /*  = 0x02  (marker response information) */
    unsigned char marker_length;	 /*  = 0x16 */
    unsigned short requester_port;	 /*   The number assigned to the port by the requester */
    mac_addr_t requester_system;      /*   The requester's system id */
    unsigned long requester_transaction_id;	/*   The transaction id allocated by the requester, */
    unsigned short pad;		 /*  = 0 */
    unsigned char tlv_type_terminator;	     /*  = 0x00 */
    unsigned char terminator_length;	     /*  = 0x00 */
    unsigned char reserved_90[90];	     /*  = 0 */
} marker_t;

typedef struct marker_header
{
    struct dot3ad_header dot3ad_header;
    struct marker marker;
} marker_header_t;

#pragma pack()
typedef struct aggregator aggregator_t;
typedef struct lacpport_actor lacp_port_actor_t;
typedef struct lacpport_partner lacp_port_partner_t;
typedef struct lacpport_sm lacp_port_sm_t;
/* port structure(43.4.6 in the 802.3ad standard) */

struct lacpport_actor
{
	unsigned int netif_index;
    unsigned short actor_port_number;
    unsigned short actor_port_priority;
    mac_addr_t actor_system;	       /* This parameter is added here although it is not specified in the standard, just for simplification */
    unsigned short actor_system_priority;	 /* This parameter is added here although it is not specified in the standard, just for simplification */
    unsigned short actor_port_aggregator_identifier;
    unsigned short actor_admin_port_key;
    unsigned short actor_oper_port_key;
    unsigned char actor_admin_port_state;
    unsigned char actor_oper_port_state;
    unsigned short is_enabled;	      /* BOOLEAN */
    int is_lacp_enable;
	lacp_mode_t lacp_mode;         /*PASSIVE mode or ACTIVE mode. Passive mode will not start periodic timer.*/
	unsigned int trunk_netif_index;
};

struct lacpport_partner
{
	unsigned int netif_index;
    mac_addr_t partner_admin_system;
    mac_addr_t partner_oper_system;
    unsigned short partner_admin_system_priority;
    unsigned short partner_oper_system_priority;
    unsigned short partner_admin_key;
    unsigned short partner_oper_key;
    unsigned short partner_admin_port_number;
    unsigned short partner_oper_port_number;
    unsigned short partner_admin_port_priority;
    unsigned short partner_oper_port_priority;
    unsigned char partner_admin_port_state;
    unsigned char partner_oper_port_state;
};

struct lacpport_sm
{
	unsigned int netif_index;
    unsigned short ntt;			 /* BOOLEAN */
    unsigned short sm_vars;	      /* all state machines variables for this port */
    rx_states_t sm_rx_state;	/* state machine rx state */
    unsigned short sm_rx_timer_counter;    /* state machine rx timer counter */
    periodic_states_t sm_periodic_state;/* state machine periodic state */
    unsigned short sm_periodic_timer_counter;	/* state machine periodic timer counter */
    mux_states_t sm_mux_state;	/* state machine mux state */
    unsigned short sm_mux_timer_counter;   /* state machine mux timer counter */
    tx_states_t sm_tx_state;	/* state machine tx state */
    unsigned short sm_tx_timer_counter;    /* state machine tx timer counter(allways on - enter to transmit state 3 time per second) */
};
#ifdef __ia64__
#pragma pack(8)
#endif

/* aggregator structure(43.4.5 in the 802.3ad standard) */
struct aggregator 
{
    mac_addr_t aggregator_mac_address;
    unsigned short aggregator_identifier;
    unsigned short is_individual;		 /* BOOLEAN */
    unsigned short actor_admin_aggregator_key;
    unsigned short actor_oper_aggregator_key;
    mac_addr_t partner_system;
    unsigned short partner_system_priority;
    unsigned short partner_oper_aggregator_key;
    unsigned short receive_state;		/* BOOLEAN */
    unsigned short transmit_state;		/* BOOLEAN */	
    unsigned short aggregator_mode;	
    /* ****** PRIVATE PARAMETERS ****** */
    unsigned long netif_index;  /*Netif index.(use this finding the ethernet interface or trunk interface if turnked)*/
	unsigned short is_active;	    /* BOOLEAN. Indicates if this aggregator is active*/
    unsigned short num_of_ports;
};

/* system structure */
typedef struct dot3ad_system
{
    unsigned short sys_priority;
    mac_addr_t sys_mac_addr;
} dot3ad_system_t;

#ifdef __ia64__
#pragma pack()
#endif

/* ================= AD Exported structures to the main bonding code ================== */
#define BOND_AD_INFO(bond)   ((bond)->dot3ad_info)
#define SLAVE_AD_INFO(slave) ((slave)->dot3ad_info)

struct dot3ad_bond_info
{
    dot3ad_system_t system;	    /* 802.3ad system structure */
    unsigned long agg_select_timer;	    /* Timer to select aggregator after all adapter's hand shakes */
    unsigned long agg_select_mode;	    /* Mode of selection of active aggregator(bandwidth/count) */
    int lacp_fast;		/* whether fast periodic tx should be
				 * requested
				 */
	unsigned long dot3ad_timer;
};

/* General definitions */
#define AD_SHORT_TIMEOUT           1
#define AD_LONG_TIMEOUT            0
#define AD_STANDBY                 0x2
#define AD_MAX_TX_IN_SECOND        1
#define AD_COLLECTOR_MAX_DELAY     0

/* Timer definitions(43.4.4 in the 802.3ad standard) */
#define AD_FAST_PERIODIC_TIME      1
#define AD_SLOW_PERIODIC_TIME      30
#define AD_SHORT_TIMEOUT_TIME      (3*AD_FAST_PERIODIC_TIME)
#define AD_LONG_TIMEOUT_TIME       (3*AD_SLOW_PERIODIC_TIME)
#define AD_CHURN_DETECTION_TIME    60
#define AD_AGGREGATE_WAIT_TIME     2

/* Port state definitions(43.4.2.2 in the 802.3ad standard) */
#define AD_STATE_LACP_ACTIVITY   0x1
#define AD_STATE_LACP_TIMEOUT    0x2
#define AD_STATE_AGGREGATION     0x4
#define AD_STATE_SYNCHRONIZATION 0x8
#define AD_STATE_COLLECTING      0x10
#define AD_STATE_DISTRIBUTING    0x20
#define AD_STATE_DEFAULTED       0x40
#define AD_STATE_EXPIRED         0x80

/* Port Variables definitions used by the State Machines(43.4.7 in the 802.3ad standard) */
#define AD_PORT_BEGIN           0x1
#define AD_PORT_LACP_ENABLED    0x2
#define AD_PORT_ACTOR_CHURN     0x4
#define AD_PORT_PARTNER_CHURN   0x8
#define AD_PORT_READY           0x10
#define AD_PORT_READY_N         0x20
#define AD_PORT_MATCHED         0x40
#define AD_PORT_STANDBY         0x80
#define AD_PORT_SELECTED        0x100
#define AD_PORT_MOVED           0x200

/* Port Key definitions
     key is determined according to the link speed, duplex and
    user key(which is yet not supported)
                 ------------------------------------------------------------
    Port key :   | User key                       |      Speed       |Duplex|
                 ------------------------------------------------------------
              16                               6               1 0 */
#define  AD_DUPLEX_KEY_BITS    0x1
#define  AD_SPEED_KEY_BITS     0x3E
#define  AD_USER_KEY_BITS      0xFFC0

/*dalloun */
#define     AD_LINK_SPEED_BITMASK_1MBPS       0x1
#define     AD_LINK_SPEED_BITMASK_10MBPS      0x2
#define     AD_LINK_SPEED_BITMASK_100MBPS     0x4
#define     AD_LINK_SPEED_BITMASK_1000MBPS    0x8
#define     AD_LINK_SPEED_BITMASK_10000MBPS   0x10
/*endalloun */

/* compare MAC addresses */
#define MAC_ADDRESS_COMPARE(A, B) memcmp(A, B, ETH_ALEN)

void npd_lacp_init();


#endif

