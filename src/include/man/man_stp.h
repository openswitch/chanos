#ifndef __MAN_STP_H__
#define __MAN_STP_H__


#include "netif_index.h"

extern char* stp_port_role[7];
extern char* stp_port_role_full[7];

extern char* stp_port_state[6];
extern char* stp_port_state_full[6];


#define STP_DISABLE 0xff
#define STP_HAVE_ENABLED 0xfe
#define STP_PORT_NOT_ENABLED 0xfd
#define STP_PORT_HAVE_ENABLED 0xfc
#define STP_PORT_NOT_LINK 0xfb

#define STP_DBUS_DEBUG(x) printf x
#define STP_DBUS_ERR(x) printf x

#define STP_DEBUG_FLAG_ALL       0xFF
#define STP_DEBUG_FLAG_DBG       0x1
#define STP_DEBUG_FLAG_WAR       0x2
#define STP_DEBUG_FLAG_ERR       0x4
#define STP_DEBUG_FLAG_EVT       0x8
#define STP_DEBUG_FLAG_PKT_REV   0x10
#define STP_DEBUG_FLAG_PKT_SED   0x20
#define STP_DEBUG_FLAG_PKT_ALL   0x30
#define STP_DEBUG_FLAG_PROTOCOL  0x40


extern int dcli_debug_out;
#define DCLI_DEBUG(x) if(dcli_debug_out){printf x ;}

#define DCLI_STP_OK 0
#define DCLI_STP_INVALID_PARAM (DCLI_STP_OK+1)
#define DCLI_STP_NO_SUCH_MSTID (DCLI_STP_OK+2)

#define DCLI_STP_OK 0
#define DCLI_STP_INVALID_PARAM (DCLI_STP_OK+1)
#define DCLI_STP_NO_SUCH_MSTID (DCLI_STP_OK+2)
#define DCLI_STP_HAVE_ENABLE    (DCLI_STP_OK+3)
#define DCLI_STP_NOT_ENABLE     (DCLI_STP_OK+4)
#define DCLI_STP_DBUS_ERR       (DCLI_STP_OK+5)
#define DCLI_STP_FATAL          (DCLI_STP_OK+0xff)

enum { 
  STP_OK = 0,                                     
  STP_ERROR,                                      
  STP_Cannot_Find_Vlan,      
  STP_Imlicite_Instance_Create_Failed,          
  STP_Small_Bridge_Priority,                    
  STP_Large_Bridge_Priority,                  
  STP_Small_Hello_Time,                      
  STP_Large_Hello_Time,                      
  STP_Small_Max_Age,                     
  STP_Large_Max_Age,                         
  STP_Small_Forward_Delay,                
  STP_Large_Forward_Delay,               
  STP_Small_Max_Hops,                   
  STP_Large_Max_Hops,                  
  STP_Forward_Delay_And_Max_Age_Are_Inconsistent,
  STP_Hello_Time_And_Max_Age_Are_Inconsistent, 
  STP_Hello_Time_And_Forward_Delay_Are_Inconsistent,
  STP_Vlan_Had_Not_Yet_Been_Created,           
  STP_Port_Is_Absent_In_The_Vlan,             
  STP_Big_len8023_Format,                     
  STP_Small_len8023_Format,               
  STP_len8023_Format_Gt_Len,            
  STP_Not_Proper_802_3_Packet,              
  STP_Invalid_Protocol,                    
  STP_Invalid_Version,                      
  STP_Had_Not_Yet_Been_Enabled_On_The_Vlan,   
  STP_Cannot_Create_Instance_For_Vlan,       
  STP_Cannot_Create_Instance_For_Port,      
  STP_Invalid_Bridge_Priority,           
  STP_There_Are_No_Ports,               
  STP_Cannot_Compute_Bridge_Prio,          
  STP_Another_Error,                    
  STP_Nothing_To_Do,                     
  STP_BRIDGE_NOTFOUND,                
  STP_CREATE_PORT_FAIL,                  
  STP_PORT_NOTFOUND,              
  STP_LAST_DUMMY                      
};

typedef enum {
  DisabledPort = 0,
  AlternatePort,
  BackupPort,
  RootPort,
  DesignatedPort,
  NonStpPort
} PORT_ROLE_T;

typedef enum{
	DISCARDING,
	LEARNING,
	FORWARDING
} PORT_STATE;

typedef struct{
		unsigned char local_port_no;
		unsigned int port_index;
}PORT_INFO;

typedef struct{
	unsigned char slot_no;
	unsigned char local_port_count;
	PORT_INFO port_no[6];
}SLOT_INFO;

typedef enum {
	DCLI_STP_M = 0,
	DCLI_MST_M, 
	DCLI_NOT_M
}DCLI_STP_RUNNING_MODE;

#define TIMERS_NUMBER   9
#define MAX_PORT_NUM    4096

typedef unsigned int    PORT_TIMER_T;
typedef unsigned short  UID_PORT_ID;
typedef struct {
  unsigned short  prio;
  unsigned char   addr[6];
} UID_BRIDGE_ID_T;
typedef enum {
  UID_PORT_DISABLED = 0,
  UID_PORT_DISCARDING,
  UID_PORT_LEARNING,
  UID_PORT_FORWARDING,
  UID_PORT_NON_STP,
  UID_PORT_ERR_DISABLE
} RSTP_PORT_STATE;
typedef struct timevalues_t {
  unsigned short MessageAge;
  unsigned short MaxAge;
  unsigned short ForwardDelay;
  unsigned short HelloTime;
  unsigned char  RemainingHops;  /*mstp*/
  unsigned char  Reserved[3]; /*mstp*/
} TIMEVALUES_T;

typedef enum {/* 17.12, 17.16.1 */
  FORCE_STP_COMPAT = 0,
  NORMAL_RSTP = 2,
  NORMAL_MSTP    /*mstp*/
} PROTOCOL_VERSION_T;

typedef struct bridge_id
{
  unsigned short    prio;
  unsigned char     addr[6];
} BRIDGE_ID;

typedef unsigned short  PORT_ID;

typedef enum {
  SuperiorDesignateMsg,
  RepeatedDesignateMsg,
  ConfirmedRootMsg,
  OtherMsg
} RCVD_MSG_T;


typedef struct state_mach_t {
  struct state_mach_t* next;

  char         name[32]; /* mstp for debugging */
  /*char*         name;  for debugging */
#ifdef STP_DBG
  char          debug; /* 0- no dbg, 1 - port, 2 - stpm */
  unsigned int  ignoreHop2State;
#endif

  int          changeState;
  unsigned int  State;

  void          (* concreteEnterState) (struct state_mach_t * );
  int          (* concreteCheckCondition) (struct state_mach_t * );
  char*         (* concreteGetStatName) (int);
  union {
    struct stpm_t* stpm;
    struct port_t* port;
    void         * owner;
  } owner;

} STATE_MACH_T;

typedef struct prio_vector_t {
  BRIDGE_ID root_bridge;
  unsigned long root_path_cost;
  BRIDGE_ID region_root_bridge;
  unsigned long region_root_path_cost;
  BRIDGE_ID design_bridge;
  PORT_ID   design_port;
  PORT_ID   bridge_port;
} PRIO_VECTOR_T;

typedef enum {
  P2P_FORCE_FALSE_E = 0,
  P2P_FORCE_TRUE_E = 1,
  P2P_AUTO_E = 2
} ADMIN_P2P_T;

typedef struct vlan_map_t{    /*mstp*/
  npd_vbmp_t bmp;
  unsigned long ulcount;
}VLAN_MAP_T;

typedef enum {
  Mine,
  Aged,
  Received,
  Disabled
} INFO_IS_T;
typedef struct port_t {
  struct port_t*     next;
  struct port_t*     nextMst;/*used to chain per port for cist and each mst, the header is always cist port */
  VLAN_MAP_T    vlan_map;


  /* per Port state machines */
  STATE_MACH_T*     info;      /* 17.21 */
  STATE_MACH_T*     roletrns;  /* 17.23 */
  STATE_MACH_T*     sttrans;   /* 17.24 */
  STATE_MACH_T*     topoch;    /* 17.25 */
  STATE_MACH_T*     migrate;   /* 17.26 */
  STATE_MACH_T*     transmit;  /* 17.26 */
  STATE_MACH_T*     p2p;       /* 6.4.3, 6.5.1 */
  STATE_MACH_T*     edge;      /*  */
  STATE_MACH_T*     pcost;     /*  */
  STATE_MACH_T*     receive ;  /* 13.28  mstp*/

  STATE_MACH_T*     machines; /* list of machines */

  struct stpm_t*    owner; /* Bridge, that this port belongs to */
  
  /* per port Timers  */
  PORT_TIMER_T      fdWhile;      /* 17.15.1 */
  PORT_TIMER_T      helloWhen;    /* 17.15.2 */
  PORT_TIMER_T      mdelayWhile;  /* 17.15.3 */
  PORT_TIMER_T      rbWhile;      /* 17.15.4 */
  PORT_TIMER_T      rcvdInfoWhile;/* 17.15.5 */
  PORT_TIMER_T      rrWhile;      /* 17.15.6 */
  PORT_TIMER_T      tcWhile;      /* 17.15.7 */
  PORT_TIMER_T      txCount;      /* 17.18.40 */
  PORT_TIMER_T      lnkWhile;

  PORT_TIMER_T*     timers[TIMERS_NUMBER]; /*list of timers */

  int              agreed;        /* 17.18.1 */
  PRIO_VECTOR_T     designPrio;    /* 17.18.2 */
  TIMEVALUES_T      designTimes;   /* 17.18.3 */
  int              forward;       /* 17.18.4 */
  int              forwarding;    /* 17.18.5 */
  INFO_IS_T         infoIs;        /* 17.18.6 */
  int              initPm;        /* 17.18.7  */
  int              learn;         /* 17.18.8 */
  int              learning;      /* 17.18.9 */
  int              mcheck;        /* 17.18.10 */
  PRIO_VECTOR_T     msgPrio;       /* 17.18.11 */
  TIMEVALUES_T      msgTimes;      /* 17.18.12 */
  int              newInfo;       /* 17.18.13 */
  int              operEdge;      /* 17.18.14 */
  int              adminEdge;     /* 17.18.14 */
  int              portEnabled;   /* 17.18.15 */
  PORT_ID           port_array_id;       /* 17.18.16 */
  PORT_ID           port_prio;
  PORT_ID           port_id;
  PRIO_VECTOR_T     portPrio;      /* 17.18.17 */
  TIMEVALUES_T      portTimes;     /* 17.18.18 */
  int              proposed;      /* 17.18.19 */
  int              proposing;     /* 17.18.20 */
  int              rcvdBpdu;      /* 17.18.21 */
  RCVD_MSG_T        rcvdInfo;       /* 17.18.22, 13.24.23 *//* 1...1*/
 /*  RCVD_MSG_T        rcvdMsg;       17.18.22 */
  int              rcvdRSTP;      /* 17/18.23 */
  int              rcvdSTP;       /* 17.18.24 */
  int              rcvdTc;        /* 17.18.25 */
  int              rcvdTcAck;     /* 17.18.26 */
  int              rcvdTcn;       /* 17.18.27 */
  int              reRoot;        /* 17.18.28 */
  int              reselect;      /* 17.18.29 */
  PORT_ROLE_T       role;          /* 17.18.30 */
  int              selected;      /* 17.18.31 */
  PORT_ROLE_T       selectedRole;  /* 17.18.32 */
  int              sendRSTP;      /* 17.18.33 */
  int              sync;          /* 17.18.34 */
  int              synced;        /* 17.18.35 */
  int              tc;            /* 17.18.36 */
  int              tcAck;         /* 17.18.37 */
  int              tcProp;        /* 17.18.38 */

  int              updtInfo;      /* 17.18.41 *//* 1...1*/
  
  int              agree;      /*mstp 13.24.1*//* 1...1*/
  int              changedMaster;     /*13.24.3*//* 1...1*/
  int              infoInternal;      /*13.24.10*/
  int              mstiMaster;     /*13.24.13*/
  int              mstiMastered;     /*13.24.14*/
  int              newInfoCist;       /*13.24.19*/
  int              newInfoMsti;       /*13.24.20*/
  int              rcvdInternal;        /*13.24.22*/
  int              rcvdMsg;     /* mstp 13.24.24*//* 1...1*/
  int              rootGuard;
  int              bpduGuard;

  /* message information */
  unsigned char     msgBpduVersion;
  unsigned char     msgBpduType;
  unsigned char     msgPortRole;
  unsigned char     msgFlags;

  unsigned long     adminPCost; /* may be ADMIN_PORT_PATH_COST_AUTO */
  unsigned long     operPCost;
  unsigned long     operSpeed;
  unsigned long     usedSpeed;
  int               LinkDelay;   /* TBD: LinkDelay may be managed ? */
  int              adminEnable; /* 'has LINK' ,modify link up or down*/
  int              wasInitBpdu;  
  int              admin_non_stp;

  int              p2p_recompute;
  int              operPointToPointMac;
  ADMIN_P2P_T       adminPointToPointMac;

  /* statistics */
  unsigned long     rx_cfg_bpdu_cnt;
  unsigned long     rx_rstp_bpdu_cnt;
  unsigned long     rx_tcn_bpdu_cnt;

  unsigned long     uptime;       /* 14.8.2.1.3.a */

  int               port_index;
  /*char*             port_name;*/
  char             port_name[32]; /*mstp*/
  unsigned short vlan_id; /*mstp*/
#ifdef STP_DBG
  unsigned int	    skip_rx;
  unsigned int	    skip_tx;
#endif

  /* configuration digest snooping option 
   *   When option enabled,send mstp packet with configuration digest originate
   * by the sender 
   */
  unsigned char 	configDigestSnp:1, /* 0 - disable, 1 - enable */
  					rsvd:7;
  unsigned char		digest[16];
} PORT_T;


typedef struct Mst_Configuration_Identifier /* 13.7 */
{
    unsigned char    FormatSelector ;
    unsigned char    ConfigurationName[32] ;
    unsigned char    RevisionLevel[2] ;
    unsigned char    ConfigurationDigest[16] ;
}MST_CFG_ID_S;

typedef enum uid_stp_mode{
  STP_STATE_DISABLED,
  STP_STATE_ENABLED
} UID_STP_MODE_T;

typedef struct stpm_t {
  struct stpm_t*        next;

  struct port_t*        ports;

  /* The only "per bridge" state machine */
  STATE_MACH_T*         rolesel;   /* the Port Role Selection State machione: 17.22 */
  STATE_MACH_T*         machines;

  /* variables */
  PROTOCOL_VERSION_T    ForceVersion;   /* 17.12, 17.16.1 */
  BRIDGE_ID             BrId;           /* 17.17.2 */
  TIMEVALUES_T          BrTimes;        /* 17.17.4 */
  PORT_ID               rootPortId;     /* 17.17.5 */
  PRIO_VECTOR_T         rootPrio;       /* 17.17.6 */
  TIMEVALUES_T          rootTimes;      /* 17.17.7 */
  MST_CFG_ID_S          MstConfigId ;       /*mstp(13.23.8)*/
  
  int                   vlan_id;        /* let's say: tag */
  char              	name[32];    /*mstp*/
  VLAN_MAP_T            vlan_map;
  /*char*                 name;            name of the VLAN, maily for debugging */
  UID_STP_MODE_T        admin_state;    /* STP_DISABLED or STP_ENABLED; type see in UiD */

  unsigned long         timeSince_Topo_Change; /* 14.8.1.1.3.b */
  unsigned long         Topo_Change_Count;     /* 14.8.1.1.3.c */
  unsigned char         Topo_Change;           /* 14.8.1.1.3.d */
  unsigned char			digest[16 + 1];		   /* User-defined string of the digest */
} STPM_T;
typedef struct {
  /* service data */
	char              vlan_name[20]; /* name of the VLAN, key of the bridge */
	unsigned int      port_no; /* key of the entry */


	/* protocol data */
	UID_PORT_ID       port_id;
    UID_PORT_ID       port_prio;
	RSTP_PORT_STATE   state;
	unsigned int     path_cost;

	UID_BRIDGE_ID_T   designated_root;
	unsigned int     designated_cost;
	UID_BRIDGE_ID_T   designated_bridge;
	UID_PORT_ID       designated_port;

#if 0
	int               infoIs;
	unsigned short    handshake_flags;

				
	unsigned long     rx_cfg_bpdu_cnt;
	unsigned long     rx_rstp_bpdu_cnt;
	unsigned long     rx_tcn_bpdu_cnt;
	int               fdWhile;      /* 17.15.1 */
	int               helloWhen;    /* 17.15.2 */
	int               mdelayWhile;  /* 17.15.3 */
	int               rbWhile;      /* 17.15.4 */
	int               rcvdInfoWhile;/* 17.15.5 */
	int               rrWhile;      /* 17.15.6 */
	int               tcWhile;      /* 17.15.7 */
	int               txCount;      /* 17.18.40 */
	int               lnkWhile;

	unsigned long     uptime;       /* 14.8.2.1.3.a */
#endif
	unsigned int 	   linkState;
	unsigned int     oper_port_path_cost;
	unsigned int		   role;	
	unsigned int    oper_point2point;
	unsigned int     oper_edge;
	unsigned int     oper_stp_neigb;
	unsigned char     top_change_ack;
	unsigned char     tc;
} UID_STP_PORT_STATE_T;

#endif

