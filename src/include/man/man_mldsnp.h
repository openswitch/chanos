#ifndef __MAN_MLDSNP_H__
#define __MAN_MLDSNP_H__
#include "lib/netif_index.h"

#define MLD_MRT_WC          0x0001
#define MLD_MRT_SG          0x0002
#define MLD_MRT_NEW         0x0010
#define MLD_MRT_STATIC      0x0020

#define MLD_VER_1           0x01
#define MLD_VER_2           0x02    /*default*/

#define MLD_SNP_PORT_STATIC     0X20

/*MLD Snoop global cfg type*/
#define MLDSNP_G_TIME_ROBUST  0x1
#define MLDSNP_G_TIME_HOST    0x2
#define MLDNSP_G_TIME_ROUTE   0x3
#define MLDSNP_G_TIME_LASTMEM 0x4
#define MLDSNP_G_TIME_LASTCNT 0x5
#define MLDSNP_G_TIME_RESP    0x6
#define MLDSNP_G_FASTLEAVE    0x10
#define MLDSNP_G_MAX_JOINGRP  0x11

struct man_mldsnp_info
{
	unsigned int mldsnp_enable;
    unsigned int mldsnp_robust_val;
	unsigned int mldsnp_resp_int;
    unsigned int mldsnp_host_aging_time;
    unsigned int mldsnp_router_aging_time;
	unsigned int mldsnp_last_member_int;
	unsigned int mldsnp_last_member_count;
	unsigned int mldsnp_fastleave_enable;
	unsigned int mldsnp_max_join_group;
};

struct man_mldsnp_vlan_info
{
	unsigned short vid;
	unsigned short mldsnp_enable;
	struct in6_addr queryAddr;
	struct in6_addr report;
	unsigned int mldCnt;
	unsigned int rtpCnt;
};

struct man_mldsnp_group_info
{
    unsigned short vlanId;
    unsigned char version;
    unsigned short flags;
    struct in6_addr srcaddr;
    struct in6_addr grpaddr;
    unsigned int portnum;
    unsigned int ifindex[MAX_SWITCHPORT_PER_SYSTEM];
	unsigned char ifmode[MAX_SWITCHPORT_PER_SYSTEM];
};

struct man_mldsnp_rtport_info
{
    unsigned short vlanId;
    unsigned char version;
	unsigned char flags;
    unsigned int ifindex;
};

#endif

