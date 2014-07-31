#ifndef __MAN_PIM_H__
#define __MAN_PIM_H__

typedef u_int   u_int32;
typedef u_short u_int16;
typedef u_char  u_int8;
typedef	u_int32 vifbitmap_t;

struct pim_vifi_info{
    char    name[IFNAMSIZ]; 
    char    flag;
    u_int16    timer;
};

struct pim_mrt_info{
    u_int32         source;
    u_int32         group;
    u_int32         mask;
    u_int32         rp;
    u_int32         upstream;
    u_int32         entry_flag;
    u_int32         wc_count;
    u_int32         sg_count;
    u_int32         pmbr_count;
    struct pim_vifi_info  incoming;
    u_int16	         timer;
    u_int16	         jp_timer;
    u_int16         rs_timer;
    u_int	         assert_timer;
    u_int16            oifs_num;
    struct pim_vifi_info  oif[0];
};

struct pim_rp_candidate{
    u_int32    addr;
    u_int8      priority;
    u_int16    time;
};

struct pim_bsr_router{
    u_int32  my_addr;
    u_int32  my_masklen;
    u_int8    my_priority;
    u_int32  curr_addr;
    u_int32  curr_masklen;
    u_int8    curr_priority;
    u_int16  curr_fragment_tag;
    u_int16  curr_timer;
};

struct pim_rp_set{
    u_int32   rp_addr;
    char         iif_name[IFNAMSIZ]; 
    u_int32   grp_addr;
    u_int32   masklen;
    u_int8     priority;
    u_int16   holdtime;
};

struct pim_vif_info{
    char    name[IFNAMSIZ]; 
    u_int32  addr;
    u_int32  flag;
	u_int32 holdtime;
	u_int32 priority;
    char        netname[68];
    u_char   threshold;
    u_int32  neigh_num;
    u_int32  neigh_addr[0];
};  

struct pim_ssm_range {
	u_int32 group;
	u_int32 masklen;
};

struct pim_info {
	u_int32 state;
	u_int32 mode;
	struct pim_ssm_range ssm_range[8];
};

#define PIM_M_DM              0x0
#define PIM_M_SM              0x1

#if 0
/* Debug values definition */
/* DVMRP reserved for future use */
#define DEBUG_DVMRP_PRUNE     0x00000001
#define DEBUG_DVMRP_ROUTE     0x00000002
#define DEBUG_DVMRP_PEER      0x00000004
#define DEBUG_DVMRP_TIMER     0x00000008
#define DEBUG_DVMRP_DETAIL    0x01000000
#define DEBUG_DVMRP           ( DEBUG_DVMRP_PRUNE | DEBUG_DVMRP_ROUTE | \
				DEBUG_DVMRP_PEER )

/* IGMP related */
#define DEBUG_IGMP_PROTO      0x00000010
#define DEBUG_IGMP_TIMER      0x00000020
#define DEBUG_IGMP_MEMBER     0x00000040
#define DEBUG_MEMBER          DEBUG_IGMP_MEMBER
#define DEBUG_IGMP            ( DEBUG_IGMP_PROTO | DEBUG_IGMP_TIMER | \
				DEBUG_IGMP_MEMBER )

/* Misc */
#define DEBUG_TRACE           0x00000080
#define DEBUG_TIMEOUT         0x00000100
#define DEBUG_PKT             0x00000200


/* Kernel related */
#define DEBUG_IF              0x00000400
#define DEBUG_KERN            0x00000800
#define DEBUG_MFC             0x00001000
#define DEBUG_RSRR            0x00002000

/* PIM related */
#define DEBUG_PIM_HELLO       0x00004000
#define DEBUG_PIM_REGISTER    0x00008000
#define DEBUG_PIM_JOIN_PRUNE  0x00010000
#define DEBUG_PIM_BOOTSTRAP   0x00020000
#define DEBUG_PIM_ASSERT      0x00040000
#define DEBUG_PIM_CAND_RP     0x00080000
#define DEBUG_PIM_MRT         0x00100000
#define DEBUG_PIM_TIMER       0x00200000
#define DEBUG_PIM_RPF         0x00400000
#define DEBUG_RPF             DEBUG_PIM_RPF
#define DEBUG_PIM_DETAIL      0x00800000
#define DEBUG_PIM             ( DEBUG_PIM_HELLO | DEBUG_PIM_REGISTER | \
				DEBUG_PIM_JOIN_PRUNE | DEBUG_PIM_BOOTSTRAP | \
				DEBUG_PIM_ASSERT | DEBUG_PIM_CAND_RP | \
				DEBUG_PIM_MRT | DEBUG_PIM_TIMER | \
				DEBUG_PIM_RPF ) 

#define DEBUG_MRT             ( DEBUG_DVMRP_ROUTE | DEBUG_PIM_MRT )
#define DEBUG_NEIGHBORS       ( DEBUG_DVMRP_PEER | DEBUG_PIM_HELLO )
#define DEBUG_TIMER           ( DEBUG_IGMP_TIMER | DEBUG_DVMRP_TIMER | \
				DEBUG_PIM_TIMER )
#define DEBUG_ASSERT          ( DEBUG_PIM_ASSERT )
#define DEBUG_ALL             0xffffffff


#define DEBUG_DEFAULT   0xffffffff/*  default if "-d" given without value */
#endif

#define DEBUG_PACKET   0x00000001
#define DEBUG_EVENT    0x00000002
#define DEBUG_TIMER    0x00000004
#define DEBUG_KERNEL   0x00000008
#define DEBUG_ALL      0xffffffff

#endif

