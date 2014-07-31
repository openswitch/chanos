#ifndef __NPD_TUNNEL_H__
#define __NPD_TUNNEL_H__

#ifdef HAVE_IP_TUNNEL

#define TS_MAX_TABLE_NUM		1024
#define TT_TABLE_LEN 			4	/* for int32 len*/

/*
 * typedef: enum TUNNEL_TYPE_ENT
 *
 * Description: Enumeration of tunnel types.
 *
 * Enumerations:
 *
 *  TUNNEL_IPV4_OVER_IPV4_E      - ipv4 tunnelled over ipv4
 *                                      passenger protocol is ipv4
 *                                      tunnel protocol is ipv4
 *
 *  TUNNEL_IPV4_OVER_GRE_IPV4_E  - ipv4 tunnelled over GRE ipv4
 *                                      passenger protocol is ipv4
 *                                      tunnel protocol is GRE ipv4
 *
 *  TUNNEL_IPV6_OVER_IPV4_E      - ipv6 tunnelled over ipv4
 *                                      passenger protocol is ipv6
 *                                      tunnel protocol is ipv4
 *
 *  TUNNEL_IPV6_OVER_GRE_IPV4_E  - ipv6 tunnelled over GRE ipv4
 *                                      passenger protocol is ipv6
 *                                      tunnel protocol is GRE ipv4
 *
 *  TUNNEL_IP_OVER_MPLS_E        - ipv4/6 tunnelled over MPLS
 *                                      passenger protocol is ipv4 or ipv6
 *                                      tunnel protocol is MPLS
 *
 *  TUNNEL_ETHERNET_OVER_MPLS_E  - ethernet tunnelled over MPLS
 *                                      passenger protocol is ethernet
 *                                      tunnel protocol is MPLS
 *
 *  TUNNEL_X_OVER_IPV4_E         - X tunnelled over ipv4
 *                                      passenger protocol is not relevant
 *                                      tunnel protocol is ipv4
 *
 *  TUNNEL_X_OVER_GRE_IPV4_E     - X tunnelled over GRE ipv4
 *                                      passenger protocol is not relevant
 *                                      tunnel protocol is GRE ipv4
 *
 *  TUNNEL_X_OVER_MPLS_E         - X tunnelled over MPLS
 *                                      passenger protocol is not relevant
 *                                      tunnel protocol is MPLS
 *
 *  TUNNEL_IP_OVER_X_E           - IP tunnelled over X
 *                                      passenger protocol is ipv4/6
 *                                      tunnel protocol is not relevant
 *
 *  TUNNEL_MAC_IN_MAC_E          - MAC in MAC
 *                                      based on IEEE 802.1ah
 *                                      tunnel protocol is not relevant
 *
 */
typedef enum {

    TUNNEL_IPV4_OVER_IPV4_E        = 0,
    TUNNEL_IPV4_OVER_GRE_IPV4_E    = 1,
    TUNNEL_IPV6_OVER_IPV4_E        = 2,
    TUNNEL_IPV6_OVER_GRE_IPV4_E    = 3,
    TUNNEL_IP_OVER_MPLS_E          = 4,
    TUNNEL_ETHERNET_OVER_MPLS_E    = 5,
    TUNNEL_X_OVER_IPV4_E           = 6,
    TUNNEL_X_OVER_GRE_IPV4_E       = 7,
    TUNNEL_X_OVER_MPLS_E           = 8,
    TUNNEL_IP_OVER_X_E             = 9,
    TUNNEL_MAC_IN_MAC_E            = 10

} TUNNEL_TYPE_ENT;

int npd_tunnel_compare_byip
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
);

int npd_tunnel_compare_kmsg
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
);

int npd_tunnel_compare_byhostip
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
);

unsigned int npd_tunnel_key_generate
(
	struct tunnel_item_s *item
);

void npd_tunnel_init
(
	void
);

void npd_tunnel_release
(
	struct tunnel_item_s *item,
	unsigned int tstten,
	unsigned int rten
);

int npd_tunnel_update
(
	struct tunnel_item_s *olditem,
	struct tunnel_item_s *item,
	unsigned int tstten,  /* is update tstt info*/
	unsigned int rten  /* is update rt info*/
);

int npd_tunnel_op_item
(
	struct tunnel_item_s *item,
	enum TUNNEL_DB_ACTION action
);

int npd_tunnel_handle_tstt_msg
(
	struct tunnel_kernel_msg_s *item
);

int npd_tunnel_handle_rt_msg
(
	struct tunnel_kernel_msg_s *item,
	unsigned int dip
);

int npd_tunnel_del_rt_host
(
	struct tunnel_kernel_msg_s *item,
	unsigned int dip
);

int npd_tunnel_del_tsttnp
(
	struct tunnel_kernel_msg_s *item
);

int npd_tunnel_handle_tstt_msg_redirect
(
	struct tunnel_item_s *item
);


int npd_tunnel_update_hashtable
(
	struct tunnel_item_s *oldItem,
	struct tunnel_item_s *newItem
);

int npd_tunnel_get_sys_mac
(
	ETHERADDRS *sysMac
);

int npd_tunnel_get_ts_tab
(
	unsigned int *index
);

int npd_tunnel_del_ts_tab
(
	unsigned int index
);

int npd_tunnel_get_tt_tab
(
	void *table,
	unsigned int len,
	unsigned int *index
);

int npd_tunnel_del_tt_tab
(
	void *table,
	unsigned int len,
	unsigned int index
);

int npd_tunnel_get_nexthop_tab
(
	unsigned int *index
);

int npd_tunnel_del_nexthop_tab
(
	unsigned int index
);

int npd_tunnel_eth_get_vid
(
	unsigned int eth_g_index,
	unsigned short *vlanid
);
void *npd_tunnel_recv_netlink_msg
(
	void
);

int npd_tunnel_handle_netlink_msg
(
	struct wifi_nl_msg * nl_msg
);


#endif
#ifdef HAVE_M4_TUNNEL

#define NPD_IP_TUNNEL_SIZE  32
#define NPD_INET6_ADDRSTRLEN 46
#define NPD_INET_ADDRSTRLEN 16

#define NPD_TNL_IF_INVALID (~0UL)

#define TUNNEL_HOST_INVALID    0x00000001

enum tnl_mode
{
    ip_tunnel_none = 0,
    ip_6in4_tunnel,
    ip_tunnel_max
};

enum tnl_nhdir
{
   nh_invalid = 0,
   nh_direct,
   nh_network
};

struct npd_ip_tunnel_m_6in4_s
{
    unsigned short s_vlan;
    unsigned int sip;
    unsigned int dip;
/*
    struct in6_addr in6;
*/
};

union npd_ip_tunnel_u
{
    struct npd_ip_tunnel_m_6in4_s m_6_in_4;
};

struct npd_ip_tunnel_nh_s
{
    unsigned int nh;
    unsigned int flag;
};

struct npd_tunnel_s
{
    int id;
    unsigned int netif_index;
    unsigned int g_ifindex;
	enum tnl_mode mode;
    union npd_ip_tunnel_u tunnel;
    int tnl_sip_valid;
    int nh_count;
    enum tnl_nhdir nh_is_direct;
    int lpm;
    struct npd_ip_tunnel_nh_s nh[NPD_ROUTE_MAX_NH_NUM];
	
#define tnl_m_6in4_vlan tunnel.m_6_in_4.s_vlan
#define tnl_m_6in4_sip tunnel.m_6_in_4.sip
#define tnl_m_6in4_dip tunnel.m_6_in_4.dip
};

#define NPD_TUNNEL_VALID(X)   \
    ((ip_tunnel_none != (X)->mode)\
        && (0 != (X)->tnl_m_6in4_sip)\
        && (0 != (X)->tnl_sip_valid)\
        && (0 != (X)->tnl_m_6in4_dip))

#define NPD_TUNNEL_IF_VALID(X)   (NPD_TNL_IF_INVALID != (X)->g_ifindex)


#endif
#endif

