#ifndef __NPD_DHCPV6_RELAY_H__
#define __NPD_DHCPV6_RELAY_H__

#ifdef HAVE_NPD_IPV6
#include "npd/ipv6.h"
#endif

#define NPD_INET6_ADDRSTRLEN 46

#define NPD_DHCPV6_RELAY_SERVER_SIZE    128
#define NPD_DHCPV6_RELAY_SERVER_HASH_SIZE      128
#define NPD_DHCPV6_RELAY_SHOW_RUNNING_INTERFACE_SIZE    256
#define NPD_DHCPV6_RELAY_SHOW_RUNNING_SIZE    \
    (NPD_DHCPV6_RELAY_SHOW_RUNNING_INTERFACE_SIZE * NPD_DHCPV6_RELAY_SERVER_SIZE)


/* 
 * DHCPv6 message types, defined in section 5.3 of RFC 3315 
 */
#define NPD_DHCPV6_SOLICIT		    1
#define NPD_DHCPV6_ADVERTISE	    2
#define NPD_DHCPV6_REQUEST		    3
#define NPD_DHCPV6_CONFIRM		    4
#define NPD_DHCPV6_RENEW		    5
#define NPD_DHCPV6_REBIND		    6
#define NPD_DHCPV6_REPLY		    7
#define NPD_DHCPV6_RELEASE		    8
#define NPD_DHCPV6_DECLINE		    9
#define NPD_DHCPV6_RECONFIGURE	   10
#define NPD_DHCPV6_INFORMATION_REQUEST 11
#define NPD_DHCPV6_RELAY_FORW	   12
#define NPD_DHCPV6_RELAY_REPL	   13

struct npd_dhcpv6_relay_global_s
{
    unsigned int is_enable;
};

struct npd_dhcpv6_relay_server_s
{
    unsigned int is_fwd_interface;
    unsigned int fwd_netif_index;
    struct in6_addr server_address;
};

/*
 *dhcpv6-relay information on interface
 */
struct npd_dhcpv6_relay_s
{
    unsigned short vid;
    unsigned int is_enable;
    unsigned int netif_index;
    struct npd_dhcpv6_relay_server_s server_interface;
#define nd6_server_fwd  server_interface.is_fwd_interface
#define nd6_server_fwd_ifidx  server_interface.fwd_netif_index
#define nd6_server_in  server_interface.server_address
#define nd6_server_in8  server_interface.server_address.s6_addr
#define nd6_server_in16  server_interface.server_address.s6_addr16
#define nd6_server_in32  server_interface.server_address.s6_addr32
};

/*
 * DHCPv6 agent options type
 * Just supply two options
 */

#define NPD_DHO_PAD                 0

#define NPD_D6O_RELAY_MSG           9
#define NPD_D6O_INTERFACE_ID        18

/*
 * Add DHCPv6 agent options here.
 * Just for DHCPv6 agent options (INTERFACE_ID and RELAY_MSG)
 */
struct npd_dhcpv6r_information_s
{
    int dhcpv6_packet_len;
    struct ip6_hdr ipv6_hdr;
    struct npd_dhcpv6_relay_s interface_relay;
};

/* 
 * Normal packet format, defined in section 6 of RFC 3315 
 */
struct npd_dhcpv6_packet_s
{
	unsigned char msg_type;
	unsigned char transaction_id[3];
	unsigned char options[0];
};

struct npd_dhcpv6_packet_options_s
{
    unsigned short option_code;
    unsigned short option_len;
    unsigned char  option_value[0];
};

/* 
 * Relay packet format, defined in section 7 of RFC 3315 
 */
struct npd_dhcpv6_relay_packet_s
{
	unsigned char msg_type;
	unsigned char hop_count;
	unsigned char link_address[16];
	unsigned char peer_address[16];
	unsigned char options[0];
};

union npd_dhcp6r_packet_u
{
    unsigned char msg_type;
    struct npd_dhcpv6_packet_s raw;
    struct npd_dhcpv6_relay_packet_s relay;
#define dhcpv6_hop_count relay.hop_count
};

extern unsigned int npd_dhcpv6_relay_service_check(unsigned int* is_enable);


#endif

