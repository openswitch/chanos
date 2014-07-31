#ifndef  __MAN_DVMRP_H__
#define __MAN_DVMRP_H__

typedef u_int   u_int32;
typedef u_short u_int16;
typedef u_char  u_int8;
typedef	u_int32 vifbitmap_t;

struct dvmrp_vifi_info{
    char    name[IFNAMSIZ]; 
    char    flag;
    u_int16    timer;
};

struct dvmrp_mrt_info{
    u_int32         source;
    u_int32         group;
    u_int32         mask;
    u_int32         sg_count;
    struct dvmrp_vifi_info  incoming;
    u_int16	         timer;
    u_int16            oifs_num;
    struct dvmrp_vifi_info  oif[0];
};

struct dvmrp_route_info{
    u_int32          origin;
    u_int32          originmask;
    u_char           metric;
    u_char           flags;
    u_int32          gateway;
    struct dvmrp_vifi_info  incoming;
    struct dvmrp_vifi_info  oif[0];    
};

struct dvmrp_nbr_info{
    struct dvmrp_vifi_info  incoming;
    u_int32       nbr_addr;
    u_char        pv;
    u_char        mv; 
    u_int32       nbr_count;
};

struct dvmrp_vif_info{
    char    name[IFNAMSIZ]; 
    u_int32  addr;
    u_int32  flag;
    char        netname[68];
    u_char   threshold;
    u_int32  neigh_num;
    u_int32  neigh_addr[0];
};

struct dvmrp_info{
    u_int32 probe_interval;
    u_int32 nbr_timeout;
    u_int32 report_interval;
    u_int32 route_timeout;
};

#endif

