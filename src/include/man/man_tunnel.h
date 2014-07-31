#ifndef __HAVE_IP_TUNNEL_H__
#define __HAVE_IP_TUNNEL_H__

#define MAN_INET6_ADDRSTRLEN 46
#define MAN_INET_ADDRSTRLEN 16

#define NPD_ROUTE_MAX_NH_NUM    64

#define TUNNEL_HOST_INVALID    0x00000001

enum man_tnlmode
{
    ip_tunnel_none = 0,
    ip_6in4_tunnel,
    ip_tunnel_max
};

enum man_tnldir
{
    nh_invalid = 0,
    nh_direct,
    nh_network
};

struct man_ip_tunnel_m_6in4_s
{
    unsigned short s_vlan;
    unsigned int sip;
    unsigned int dip;
/*
    struct in6_addr in6;
*/
};

union man_ip_tunnel_u
{
    struct man_ip_tunnel_m_6in4_s m_6_in_4;
};

struct man_ip_tunnel_nh_s
{
    unsigned int nh;

    unsigned int flag;
};

struct man_ip_tunnel_s
{
    int id;
    unsigned int netif_index;
    unsigned int g_ifindex;
	enum man_tnlmode mode;    
    union man_ip_tunnel_u tunnel;
    int tnl_sip_valid;
    int nh_count;
    enum man_tnldir nh_is_direct;
    int lpm;    
    struct man_ip_tunnel_nh_s nh[NPD_ROUTE_MAX_NH_NUM];

#define tnl_m_6in4_vlan tunnel.m_6_in_4.s_vlan
#define tnl_m_6in4_sip tunnel.m_6_in_4.sip
#define tnl_m_6in4_dip tunnel.m_6_in_4.dip

};

#endif
