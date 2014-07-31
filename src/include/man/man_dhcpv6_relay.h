#ifndef __MAN_DHCPV_RELAY_H__
#define __MAN_DHCPV_RELAY_H__

#define MAN_INET6_ADDRSTRLEN 46

#define DHCP6R_NAME_LEN (64)

struct man_dhcpv6_relay_server_s
{
    unsigned int is_fwd_interface;
    unsigned int fwd_netif_index;
    struct in6_addr server_address;
};

/*
 * the copy from struct npd_dhcpv6_relay_s
 * dhcpv6-relay information on interface
 */
struct man_dhcpv6_relay_s
{
    unsigned int is_enable;
    unsigned int netif_index;
    struct man_dhcpv6_relay_server_s server_interface;
#define md6_server_fwd  server_interface.is_fwd_interface
#define md6_server_fwd_ifidx  server_interface.fwd_netif_index
#define md6_server_in  server_interface.server_address
#define md6_server_in8  server_interface.server_address.s6_addr
#define md6_server_in32  server_interface.server_address.s6_addr32
};

#endif

