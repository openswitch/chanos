
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 * DHCPv6 Relay Agent.
 * Copy form "ISC" DHCP/BOOTP Relay Agent
 */
 
#if defined(HAVE_NPD_IPV6) && defined(HAVE_DHCPV6_RELAY)

#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_dhcpv6_relay.h"

int max_hop_count = 32;		/* Maximum hop count */

int npd_dhcp6r_socket = -1;

/* Generic interface registration routine... */
void npd_dhcp6r_socket_init()
{
	int flag;
    int on = 1;
    int hop_limit = 32;
    struct in6_addr local_address6;
    struct sockaddr_storage name;
    struct sockaddr_in6* bind_addr = (struct sockaddr_in *)&name; 

	/* Make a socket... */
	npd_dhcp6r_socket = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (npd_dhcp6r_socket < 0)
	{
		npd_syslog_dbg("Can't create dhcp socket: %m");
        return ;
	}

	/* Set the REUSEADDR option so that we don't fail to start if
	   we're being restarted. */
	flag = 1;
	if (setsockopt(npd_dhcp6r_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) < 0)
	{
		npd_syslog_dbg("Can't set SO_REUSEADDR option on dhcp socket: %m");
        return ;
	}

	/* Set the BROADCAST option so that we can broadcast DHCP responses. */
	if (setsockopt(npd_dhcp6r_socket, SOL_SOCKET, SO_BROADCAST, (char *)&flag, sizeof(flag)) < 0)
    {
		npd_syslog_dbg("Can't set SO_BROADCAST option on dhcp socket: %m");
        return ;
	}

#if defined(SO_REUSEPORT)
	/*
	 * We only set SO_REUSEPORT on AF_INET6 sockets, so that multiple
	 * daemons can bind to their own sockets and get data for their
	 * respective interfaces.  This does not (and should not) affect
	 * DHCPv4 sockets; we can't yet support BSD sockets well, much
	 * less multiple sockets.
	 */

	flag = 1;
	if (setsockopt(npd_dhcp6r_socket, SOL_SOCKET, SO_REUSEPORT, (char *)&flag, sizeof(flag)) < 0)
	{
		npd_syslog_dbg("Can't set SO_REUSEPORT option on dhcp socket: %m");
        return ;
	}
#endif

	/*
	 * If we turn on IPV6_PKTINFO, we will be able to receive 
	 * additional information, such as the destination IP address.
	 * We need this to spot unicast packets.
	 */
		
#ifdef IPV6_RECVPKTINFO
	/* RFC3542 */
	if (setsockopt(npd_dhcp6r_socket, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on)) != 0)
	{
		npd_syslog_dbg("setsockopt: IPV6_RECVPKTINFO: %m");
        return ;
	}
#else
	/* RFC2292 */
	if (setsockopt(npd_dhcp6r_socket, IPPROTO_IPV6, IPV6_PKTINFO, &on, sizeof(on)) != 0)
	{
		npd_syslog_dbg("setsockopt: IPV6_PKTINFO: %m");
        return ;
	}
#endif

	if (setsockopt(npd_dhcp6r_socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hop_limit, sizeof(int)) < 0)
	{
		npd_syslog_dbg("setsockopt: IPV6_MULTICAST_HOPS: %m");
        return ;
	}

    memset(&name, 0, sizeof(name));
    bind_addr->sin6_family = AF_INET6;
    bind_addr->sin6_port = htons(547);
    /* XXX: What will happen to multicasts if this is nonzero? */
    inet_pton(AF_INET6, "::", local_address6.s6_addr);
    memcpy(&bind_addr->sin6_addr, &local_address6, sizeof(bind_addr->sin6_addr));
#ifdef HAVE_SA_LEN
    bind_addr->sin6_len = sizeof(struct sockaddr_in6);
#endif
    
    if (bind(npd_dhcp6r_socket, (struct sockaddr *)&name, sizeof(struct sockaddr_in6)) < 0)
    {
        npd_syslog_dbg("Bind an socket error.\n");
        return ;
    }

	return ;
}


int npd_dhcp6r_send_packet6(int len, const char* raw, char* ifname, struct sockaddr_in6* to)
{
	struct msghdr m;
	struct iovec v;
	int result;
	struct in6_pktinfo *pktinfo;
	struct cmsghdr *cmsg;
	union
	{
        /* total size (sizeof(struct cmsghdr)) + sizeof(struct in6_pktinfo) */
		struct cmsghdr cmsg_sizer;
		u_int8_t pktinfo_sizer[CMSG_SPACE(sizeof(struct in6_pktinfo))];
	} control_buf;

    npd_syslog_dbg("Dhcp6r: send packet length = %d, ifname = %s.\n", len, ifname);
	/*
	 * Initialize our message header structure.
	 */
	memset(&m, 0, sizeof(m));

	/*
	 * Set the target address we're sending to.
	 */
	m.msg_name = to;
	m.msg_namelen = sizeof(struct sockaddr_in6);

	/*
	 * Set the data buffer we're sending. (Using this wacky 
	 * "scatter-gather" stuff... we only have a single chunk 
	 * of data to send, so we declare a single vector entry.)
	 */
	v.iov_base = (char *)raw;
	v.iov_len = len;
	m.msg_iov = &v;
	m.msg_iovlen = 1;

	/*
	 * Setting the interface is a bit more involved.
	 * 
	 * We have to create a "control message", and set that to 
	 * define the IPv6 packet information. We could set the
	 * source address if we wanted, but we can safely let the
	 * kernel decide what that should be. 
	 */
	m.msg_control = &control_buf;
	m.msg_controllen = sizeof(control_buf);
	cmsg = CMSG_FIRSTHDR(&m);
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_PKTINFO;
	cmsg->cmsg_len = CMSG_LEN(sizeof(*pktinfo));
    
	pktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsg);
	memset(pktinfo, 0, sizeof(*pktinfo));
	pktinfo->ipi6_ifindex = if_nametoindex(ifname);
	m.msg_controllen = cmsg->cmsg_len;

	result = sendmsg(npd_dhcp6r_socket, &m, 0);
	if (result < 0)
	{
		npd_syslog_dbg("Send dhcpv6 relay packet failed.\n");
	}
	return result;
}

int npd_dhcp6r_get_ipv6_address(unsigned int netif_index, struct in6_addr* out6_address)
{
    int ni = 0;
    unsigned int ifindex = 0;
    struct in6_addr link_gateway[MAX_IP_COUNT];
    struct in6_addr link_mask[MAX_IP_COUNT];
    
    memset(&link_gateway, 0, sizeof(struct in6_addr) * MAX_IP_COUNT);
    memset(&link_mask, 0, sizeof(struct in6_addr) * MAX_IP_COUNT);

    if (NPD_FALSE == npd_intf_gindex_exist_check(netif_index, &ifindex))
    {
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

    if (NPD_FALSE == npd_ipv6_intf_addr_ip_get(ifindex, link_gateway, link_mask))
    {
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

    for (ni = 0; ni < MAX_IP_COUNT; ni++)
    {
        if ((0xfe80 != link_gateway[ni].s6_addr16[0]) && (0 != link_gateway[ni].s6_addr16[0]))
        {
            break;
        }
    }

    if (MAX_IP_COUNT == ni)
    {
        memcpy(out6_address, &link_gateway[0], sizeof(struct in6_addr));
    }
    else
    {
        memcpy(out6_address, &link_gateway[ni], sizeof(struct in6_addr));
    }

    return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}

int npd_dhcp6r_process_get_options_value(int options_type, int offset, union npd_dhcp6r_packet_u* packet, unsigned char* value)
{
    int cursor = 0;
    struct npd_dhcpv6_packet_options_s* options = NULL;

    if (NPD_DHCPV6_RELAY_REPL == packet->msg_type)
    {
        options = (struct npd_dhcpv6_packet_options_s* )packet->relay.options;
        cursor = sizeof(struct npd_dhcpv6_relay_packet_s);
    }
    else
    {
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

    do 
    {
        if (options_type == ntohs(options->option_code))
        {
             memcpy(value, &(options->option_value), ntohs(options->option_len));
             break;
        }
        else if (ntohs(options->option_code) == NPD_DHO_PAD)
        {
            cursor++;
        }
        else
        {
            cursor += sizeof(struct npd_dhcpv6_packet_options_s);
            cursor += ntohs(options->option_len);
        }

        options = (struct npd_dhcpv6_packet_options_s* )((char* )packet + cursor);
    } while (cursor < offset);

    if (cursor >= offset)
    {
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

    return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}

int npd_dhcp6r_process_down6(union npd_dhcp6r_packet_u* packet, struct npd_dhcpv6r_information_s* entry)
{
    int cursor = 0;
    unsigned int netif_index = 0;
    unsigned char relay_msg[2048];
    char relay_or_client_ifname[32];
	const struct npd_dhcpv6_packet_s *msg;
	struct sockaddr_in6 down_send_to;

    memset(&down_send_to, 0, sizeof(down_send_to));
    down_send_to.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
    down_send_to.sin6_len = sizeof(down_send_to);
#endif
    down_send_to.sin6_port = htons(546);

	/* Get the relay-msg option (carrying the message to relay). */
    memset(relay_msg, 0, sizeof(relay_msg));
    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS
        != npd_dhcp6r_process_get_options_value(NPD_D6O_RELAY_MSG, entry->dhcpv6_packet_len, packet, relay_msg))
    {
        npd_syslog_dbg("Receive dhcpv6 packet whithout relay-msg.");
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

	/* Check if we should relay the carried message. */    
	msg = (const struct npd_dhcpv6_packet_s *) relay_msg;
	switch (msg->msg_type)
	{
		/* Relay-Reply of for another relay, not a client. */
		case NPD_DHCPV6_RELAY_REPL:
		{
			down_send_to.sin6_port = htons(547);
		}
		/* Fall into: */
		case NPD_DHCPV6_ADVERTISE:
		case NPD_DHCPV6_REPLY:
		case NPD_DHCPV6_RECONFIGURE:
		case NPD_DHCPV6_RELAY_FORW:
        {
			break;
		}
		case NPD_DHCPV6_SOLICIT:
		case NPD_DHCPV6_REQUEST:
		case NPD_DHCPV6_CONFIRM:
		case NPD_DHCPV6_RENEW:
		case NPD_DHCPV6_REBIND:
		case NPD_DHCPV6_RELEASE:
		case NPD_DHCPV6_DECLINE:
		case NPD_DHCPV6_INFORMATION_REQUEST:
        {
			npd_syslog_dbg("Discarding sub packet as the packet type is invaild.");
			return DHCPV6_RELAY_RETURN_CODE_DROP;
		}
		default:
        {
			npd_syslog_dbg("Discarding sub packet as the packet type is unknown.");
			return DHCPV6_RELAY_RETURN_CODE_DROP;
		}
	}

	/* Get the interface-id (if exists) and the downstream. */
    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS
        != npd_dhcp6r_process_get_options_value(NPD_D6O_INTERFACE_ID, entry->dhcpv6_packet_len, packet, &netif_index))
    {
        npd_syslog_dbg("Receive dhcpv6 packet whithout interface-id.");
        return DHCPV6_RELAY_RETURN_CODE_DROP;
    }

    memcpy(down_send_to.sin6_addr.s6_addr, packet->relay.peer_address, sizeof(packet->relay.peer_address));

	/* Send the message to the downstream. */
    cursor = entry->dhcpv6_packet_len;
    cursor -= sizeof(struct npd_dhcpv6_relay_packet_s);

    memset(relay_or_client_ifname, 0, sizeof(relay_or_client_ifname));
    if (0 != ntohl(netif_index))
    {
        if (0 != npd_netif_index_to_name(ntohl(netif_index), relay_or_client_ifname))
        {
            npd_syslog_dbg("Down6: get client name failed.\n");
            return DHCPV6_RELAY_RETURN_CODE_DROP;
        }
    }
    
    npd_dhcp6r_send_packet6(cursor, (unsigned char *) relay_msg, relay_or_client_ifname, &down_send_to);

    return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}


int npd_dhcp6r_process_up6(union npd_dhcp6r_packet_u* packet, struct npd_dhcpv6r_information_s* entry)
{
	char forw_data[2048];
	int cursor = 0;
    unsigned int network_byte_order_idx = 0;
    char server_ifname[32];
	struct npd_dhcpv6_relay_packet_s* relay = NULL;
    struct npd_dhcpv6_packet_options_s* option_interface_id = NULL;
    struct npd_dhcpv6_packet_options_s* option_relay_msg = NULL;
    struct sockaddr_in6 up_send_to;
    struct in6_addr for_link_address;

    memset(forw_data, 0, sizeof(forw_data));
	/* XXX: Build the relay-forward header. */
	relay = (struct npd_dhcpv6_relay_packet_s *) forw_data;
	cursor = sizeof(struct npd_dhcpv6_relay_packet_s);
	relay->msg_type = NPD_DHCPV6_RELAY_FORW;

    memcpy(&relay->peer_address, entry->ipv6_hdr.ip6_src.s6_addr, sizeof(struct in6_addr));
    
	if (NPD_DHCPV6_RELAY_FORW == packet->msg_type)
	{
		if (packet->dhcpv6_hop_count >= max_hop_count)
		{
			npd_syslog_dbg("Up6: hop count exceeded\n");
			return DHCPV6_RELAY_RETURN_CODE_DROP;
		}

        relay->hop_count = packet->dhcpv6_hop_count + 1;

		if (((htons(0x2001) != entry->ipv6_hdr.ip6_src.s6_addr16[0]))
            || ((htons(0x2002) != entry->ipv6_hdr.ip6_src.s6_addr16[0]))
            || ((htons(0xfe80) != entry->ipv6_hdr.ip6_src.s6_addr16[0]))) /* global unicast address or site-address */
		{
            memset(&relay->link_address, 0, 16);
		}
        else
        {
            memset(&for_link_address, 0, sizeof(struct in6_addr));
            if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != npd_dhcp6r_get_ipv6_address(entry->interface_relay.netif_index, &for_link_address))
            {
                npd_syslog_dbg("Up6: not found interface(fowr).\n");
                return DHCPV6_RELAY_RETURN_CODE_DROP;
            }

            memcpy(&relay->link_address, &for_link_address, 16);
		}
	}
    else
	{
		relay->hop_count = 0;

        memset(&for_link_address, 0, sizeof(struct in6_addr));
        if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != npd_dhcp6r_get_ipv6_address(entry->interface_relay.netif_index, &for_link_address))
        {
            npd_syslog_dbg("Up6: not found interface(client).\n");
            return DHCPV6_RELAY_RETURN_CODE_DROP;
        }

        memcpy(&relay->link_address, &for_link_address, 16);
	}

    /* force:Add an interface-id */
	option_interface_id = (struct npd_dhcpv6_packet_options_s* )(forw_data + cursor);
    option_interface_id->option_code = htons(NPD_D6O_INTERFACE_ID);
    option_interface_id->option_len = htons(sizeof(unsigned int)); /* XXX: netif-index */
    network_byte_order_idx = htonl(entry->interface_relay.netif_index);
    memcpy(option_interface_id->option_value, &network_byte_order_idx, ntohs(option_interface_id->option_len));

    /* XXX: Calculate the offset for next options */
    cursor += sizeof(struct npd_dhcpv6_packet_options_s);   /* XXX: option header */
    cursor += ntohs(option_interface_id->option_len); 

    /* XXX: Add the relay-msg carrying the packet. */
    option_relay_msg = (struct npd_dhcpv6_packet_options_s* )(forw_data + cursor);
    option_relay_msg->option_code = htons(NPD_D6O_RELAY_MSG);
    option_relay_msg->option_len = htons((unsigned short)entry->dhcpv6_packet_len);    /* XXX: dhcpv6 packet length */
    memcpy(option_relay_msg->option_value, packet, ntohs(option_relay_msg->option_len));

    /* XXX: Calculate the offset for next options */
    cursor += sizeof(struct npd_dhcpv6_packet_options_s);   /* XXX: option header */
    cursor += ntohs(option_relay_msg->option_len); /* XXX: dhcpv6-packet length */

    /* XXX: Initialization send to structure*/
    memset(&up_send_to, 0, sizeof(struct sockaddr_in6));
    memcpy(&up_send_to.sin6_addr, &(entry->interface_relay.nd6_server_in), sizeof(struct in6_addr));
    up_send_to.sin6_family = AF_INET6;
    up_send_to.sin6_port = htons(547);
#ifdef HAVE_SA_LEN
	up_send_to.sin6_len = sizeof(struct sockaddr_in6);
#endif

    memset(server_ifname, 0, sizeof(server_ifname));
    if (0 != entry->interface_relay.nd6_server_fwd_ifidx)
    {
        if (0 != npd_netif_index_to_name(entry->interface_relay.nd6_server_fwd_ifidx, server_ifname))
        {
            npd_syslog_dbg("Up6: get server name failed.\n");
            return DHCPV6_RELAY_RETURN_CODE_DROP;
        }
    }

    npd_dhcp6r_send_packet6(cursor, forw_data, server_ifname, &up_send_to);
    
    return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}

int npd_dhcp6r(union npd_dhcp6r_packet_u* packet, struct npd_dhcpv6r_information_s* entry)
{
    int ret = 0;

    npd_syslog_dbg("Process packet, packet type is %d.\n", packet->msg_type);

    switch (packet->msg_type)
    {
        case NPD_DHCPV6_RELAY_REPL:
        {
            ret = npd_dhcp6r_process_down6(packet, entry);
            break;
        }
		case NPD_DHCPV6_SOLICIT:
		case NPD_DHCPV6_REQUEST:
		case NPD_DHCPV6_CONFIRM:
		case NPD_DHCPV6_RENEW:
		case NPD_DHCPV6_REBIND:
		case NPD_DHCPV6_RELEASE:
		case NPD_DHCPV6_DECLINE:
		case NPD_DHCPV6_INFORMATION_REQUEST:
		case NPD_DHCPV6_RELAY_FORW:
        {
            ret = npd_dhcp6r_process_up6(packet, entry);
            break;
        }
        default :
        {
            ret = DHCPV6_RELAY_RETURN_CODE_DROP;
            break;
        }
    }
    
    return ret;
}

int npd_dhcp6r_packet_len_okay(union npd_dhcp6r_packet_u* packet, int len) 
{
	if ((NPD_DHCPV6_RELAY_FORW == packet->msg_type)
        || (NPD_DHCPV6_RELAY_REPL == packet->msg_type))
	{
		if (len >= sizeof(struct npd_dhcpv6_relay_packet_s))
		{
			return 0;
		}
        else
		{
			return -1;
		}
	}
    else
    {
		if (len >= sizeof(struct npd_dhcpv6_packet_s))
		{
			return 0;
		}
        else
        {
			return -1;
		}
	}

    return 0;
}

unsigned int npd_dhcpv6_relay_packet_rx_process
(
    unsigned char isTagged,
    unsigned short vid,
    int packetType,
    unsigned int netif_index,
    unsigned long buffLen,
    unsigned char *packetBuffs
)
{
    unsigned int is_service_enable = 0;
    unsigned int lo_netif_index = 0;
    union npd_dhcp6r_packet_u* dhcpv6_packet = NULL;
    struct npd_dhcpv6r_information_s entry;

    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != npd_dhcpv6_relay_service_check(&is_service_enable))
    {
        return DHCPV6_RELAY_RETURN_CODE_DROP; 
    }

    if (!is_service_enable)
    {
        /* Dhcpv6 relay service is not enable! */
        return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    }

    memset(&entry, 0, sizeof(struct npd_dhcpv6r_information_s));

    memcpy(&(entry.ipv6_hdr), packetBuffs + sizeof(ether_header_t), sizeof(struct ip6_hdr));
    entry.dhcpv6_packet_len = entry.ipv6_hdr.ip6_plen - sizeof(udp_header_t);
    
    dhcpv6_packet = (union npd_dhcp6r_packet_u*)(packetBuffs + sizeof(ether_header_t) + sizeof(struct ip6_hdr) + sizeof(udp_header_t));

    if (0 != npd_dhcp6r_packet_len_okay(dhcpv6_packet, entry.dhcpv6_packet_len))
	{
		npd_syslog_dbg("dhcpv6r_rx_process: short packet from  netif-index %x, vid %d, dropped", netif_index, vid);
		return DHCPV6_RELAY_RETURN_CODE_DROP;
	}

    if (NPD_DHCPV6_RELAY_REPL != dhcpv6_packet->msg_type)
    {
        if (vid != NPD_PORT_L3INTF_VLAN_ID)
    	{
    		lo_netif_index = npd_netif_vlan_index(vid);
    	}
    	else
    	{
    		lo_netif_index = netif_index;
    	}

        if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != npd_dhcpv6_relay_server_get_byindex(lo_netif_index, &(entry.interface_relay)))
        {
            return DHCPV6_RELAY_RETURN_CODE_DROP;
        }
        else if (0 == entry.interface_relay.is_enable)
        {
            /* Dhcpv6 relay interface service is not enable! */
            return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
        }
    }

    return npd_dhcp6r(dhcpv6_packet, &entry);
}


/* XXX: command stuff starting*/

int npd_dhcpv6_relay_global_no = 0;
db_table_t* npd_db_table_dhcpv6_relay_global = NULL;
array_table_index_t* npd_db_table_dhcpv6_relay_global_idx = NULL;
db_table_t* npd_db_table_dhcpv6_relay = NULL;
hash_table_index_t* npd_db_table_dhcpv6_relay_hash_idx = NULL;

netif_event_notifier_t npd_dhcpv6_relay_netif_notifier;

unsigned int npd_dhcpv6_relay_server_genkey(void* temp)
{
	struct npd_dhcpv6_relay_s* data;
	unsigned int key = 0;
    
	if (NULL == temp)
	{
		return FALSE;
	}
	data = (struct npd_dhcpv6_relay_s*)temp;

	key = data->netif_index >> 14;

	key %= NPD_DHCPV6_RELAY_SERVER_SIZE;

	return key;
}

unsigned int npd_dhcpv6_relay_server_compare(void* temp1, void* temp2)
{
	struct npd_dhcpv6_relay_s* data1;
	struct npd_dhcpv6_relay_s* data2;
	unsigned int ret = FALSE;

	if (NULL == temp1 || NULL == temp2)
	{
		return FALSE;
	}
	data1 = (struct npd_dhcpv6_relay_s*)temp1;
	data2 = (struct npd_dhcpv6_relay_s*)temp2;

	if (data1->netif_index != data2->netif_index)
	{
		ret = FALSE;
	}
    else
    {
        ret = TRUE;
    }

	return ret;
}

unsigned int npd_dhcpv6_relay_table_init()
{
    unsigned int ret = 0;
    struct npd_dhcpv6_relay_global_s entry;

    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_global_s));
    
	ret = create_dbtable("npd_dhcpv6_relay_global", 
							1, 
							sizeof(struct npd_dhcpv6_relay_global_s),
							NULL, 
							NULL,
							NULL, 
							NULL, 
							NULL,
							NULL,
							NULL, 
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&(npd_db_table_dhcpv6_relay_global));
	if (0 != ret)
	{
		syslog_ax_dhcp_snp_err("Create npd dhcpv6 relay global db-table fail\n");
		return NPD_FAIL;
	}


    ret = dbtable_create_array_index("npd_dhcpv6_relay_global_idx", 
									npd_db_table_dhcpv6_relay_global,  
									&npd_db_table_dhcpv6_relay_global_idx);
	if (0 != ret)
	{
		syslog_ax_dhcp_snp_err("Create npd dhcpv6 relay global array index fail\n");
		return NPD_FAIL;
	}	

    ret = dbtable_array_insert(npd_db_table_dhcpv6_relay_global_idx, &npd_dhcpv6_relay_global_no, &entry); 
    if (0 != ret)
	{
		syslog_ax_dhcp_snp_err("Insert npd dhcpv6 relay global entry fail\n");
		return NPD_FAIL;
	}

	ret = create_dbtable("npd_dhcpv6_relay_l3",
                    NPD_DHCPV6_RELAY_SERVER_SIZE,
                    sizeof(struct npd_dhcpv6_relay_s),
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL,
					NULL, 
					NULL, 
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_db_table_dhcpv6_relay));
	if (0 != ret)
	{
		syslog_ax_dhcp_snp_err("Create npd dhcpv6 relay db-table fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("npd_dhcpv6_relay_l3_hash",
                    npd_db_table_dhcpv6_relay,
                    NPD_DHCPV6_RELAY_SERVER_HASH_SIZE,
            		npd_dhcpv6_relay_server_genkey,
            		npd_dhcpv6_relay_server_compare,
            		&npd_db_table_dhcpv6_relay_hash_idx);
	if (0 != ret)
	{
		syslog_ax_dhcp_snp_err("Create npd dhcpv6 relay hash index fail\n");
		return NPD_FAIL;
	}

	npd_syslog_dbg("Npd dhcpv6 relay init table successfully.\n");
	npd_dhcp6r_socket_init();
    register_netif_notifier(&npd_dhcpv6_relay_netif_notifier);
	return NPD_OK;
}

unsigned int npd_dhcpv6_relay_global_get
(
	struct npd_dhcpv6_relay_global_s* entry
)
{
    return (0 == dbtable_array_get(npd_db_table_dhcpv6_relay_global_idx, npd_dhcpv6_relay_global_no, entry)) \
        ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
}

unsigned int npd_dhcpv6_relay_global_set
(
	struct npd_dhcpv6_relay_global_s* entry
)
{
    return (0 == dbtable_array_update(npd_db_table_dhcpv6_relay_global_idx, npd_dhcpv6_relay_global_no, NULL, entry)) \
        ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
}

int npd_dhcpv6_relay_server_get_byindex(unsigned int index, struct npd_dhcpv6_relay_s* entry)
{
	struct npd_dhcpv6_relay_s data;
    
	data.netif_index = index;
	
	return (0 == dbtable_hash_search(npd_db_table_dhcpv6_relay_hash_idx, &data, NULL, entry)) \
        ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
}

int npd_dhcpv6_relay_server_set(struct npd_dhcpv6_relay_s* entry)
{
	struct npd_dhcpv6_relay_s data;

	if (0 == dbtable_hash_search(npd_db_table_dhcpv6_relay_hash_idx, entry, NULL, &data))
	{
		return (0 == dbtable_hash_update(npd_db_table_dhcpv6_relay_hash_idx, NULL, entry)) \
            ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	return (0 == dbtable_hash_insert(npd_db_table_dhcpv6_relay_hash_idx, entry)) \
        ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
}

int npd_dhcpv6_relay_server_del(struct npd_dhcpv6_relay_s* entry)
{
	struct npd_dhcpv6_relay_s data;
    
	if (0 == dbtable_hash_search(npd_db_table_dhcpv6_relay_hash_idx, entry, NULL, &data))
	{
		return (0 == dbtable_hash_delete(npd_db_table_dhcpv6_relay_hash_idx, entry, entry)) \
            ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR;
	}

	return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}

void npd_dhcpv6_relay_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private,
    int len
)
{
    struct npd_dhcpv6_relay_s entry;
    
    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
    
	npd_syslog_dbg("Npd notify DHCPv6 Relay index event: index 0x%x event %d\n", netif_index, evt);

    switch(evt)
    {
	    case PORT_NOTIFIER_L3DELETE:
        {
            if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == npd_dhcpv6_relay_server_get_byindex(netif_index, &entry))
            {
                if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != npd_dhcpv6_relay_server_del(&entry))
                {
                    npd_syslog_dbg("Npd event d6r remove helper (index 0x%x) failed\n", netif_index);
                }
            }

			break;
	    }
	    default:
        {
	        break;
	    }
    }

    return;
}

void npd_dhcpv6_relay_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private,
    int len
)
{
	npd_syslog_dbg("Npd notify DHCPv6 Relay relate event: vlan 0x%x index 0x%x event %d\n", \
											vlan_index, netif_index, event);

	return;
}

netif_event_notifier_t npd_dhcpv6_relay_netif_notifier =
{
    .netif_event_handle_f  = &npd_dhcpv6_relay_notify_event,
    .netif_relate_handle_f = &npd_dhcpv6_relay_relate_event
};

unsigned int npd_dhcpv6_relay_global(unsigned int is_enable)
{
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    struct npd_dhcpv6_relay_global_s entry;

    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_global_s));

    ret = npd_dhcpv6_relay_global_get(&entry);

    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == ret)
    {
        if (entry.is_enable == is_enable)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ALREADY_SET;
        }
        else
        {
            entry.is_enable = is_enable;
            ret = npd_dhcpv6_relay_global_set(&entry);
        }
    }

    return ret;
}

unsigned int npd_dhcpv6_relay_service_check(unsigned int* is_enable)
{
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    struct npd_dhcpv6_relay_global_s entry;

    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_global_s));

    ret = npd_dhcpv6_relay_global_get(&entry);

    *is_enable = entry.is_enable;

    return ret;
}

unsigned int npd_dhcpv6_relay_interface(unsigned int is_enable, char* p_ifname)
{
    unsigned int is_service_enable = 0;
    unsigned int netif_index = 0;
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    struct npd_dhcpv6_relay_s entry;

    if (NULL == p_ifname)
    {
        ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
    }
    else
	{
        ret = npd_dhcpv6_relay_service_check(&is_service_enable);

        if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != ret)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else if (0 == is_service_enable)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_SERVICE_DISABLE;
        }
        else if (NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, p_ifname))
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else
        {
            memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
            
            if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == npd_dhcpv6_relay_server_get_byindex(netif_index, &entry))
        	{
        		if (is_enable == entry.is_enable)
        		{
        			ret = DHCPV6_RELAY_RETURN_CODE_ALREADY_SET; 
        		}
        		else
        		{
                    entry.is_enable = is_enable;
                    ret = npd_dhcpv6_relay_server_set(&entry);
        		}
        	}
            else
            {
                memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_global_s));
                entry.netif_index= netif_index;
                entry.is_enable = is_enable;

                if (NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
                {
                    entry.vid = npd_netif_vlan_get_vid(netif_index);
                }
                else
                {
                    entry.vid = NPD_PORT_L3INTF_VLAN_ID;
                }

                ret = npd_dhcpv6_relay_server_set(&entry);
            }
        }
	}

    return ret;
}

unsigned int npd_dhcpv6_relay_server
(
    unsigned int is_fwd_interface,
    struct in6_addr ipv6_address,
    char* p_ifname,
    char* p_fwd_ifname
)
{
    unsigned int is_service_enable = 0;
    unsigned int netif_index = 0;
    unsigned int fwd_netif_index = 0;
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    struct npd_dhcpv6_relay_s entry;

    if (NULL == p_ifname)
    {
        ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
    }
    else
	{
        ret = npd_dhcpv6_relay_service_check(&is_service_enable);

        if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != ret)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else if (0 == is_service_enable)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_SERVICE_DISABLE;
        }
        else if (NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, p_ifname))
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else
        {
            memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
            
            if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == npd_dhcpv6_relay_server_get_byindex(netif_index, &entry))
        	{
        		if (0 == entry.is_enable)
        		{
        			ret = DHCPV6_RELAY_RETURN_CODE_INTFFACE_DISABLE; 
        		}
        		else
        		{
                    entry.nd6_server_fwd = is_fwd_interface;
                    entry.nd6_server_in = ipv6_address;
                    if (is_fwd_interface)
                    {
                        if (NPD_TRUE == npd_intf_netif_get_by_name(&fwd_netif_index, p_fwd_ifname))
                        {
                            entry.nd6_server_fwd_ifidx = fwd_netif_index;
                        }
                        else
                        {
                            ret = DHCPV6_RELAY_RETURN_CODE_L3_NOT_EXIST;
                        }
                    }

                    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == ret)
                    {
                        ret = npd_dhcpv6_relay_server_set(&entry);
                    }
        		}
        	}
            else
            {
                ret = DHCPV6_RELAY_RETURN_CODE_INTFFACE_DISABLE;
            }
        }
	}

    return ret;
}

unsigned int npd_dhcpv6_relay_no_server(struct in6_addr ipv6_address, char* p_ifname)
{
    unsigned int is_service_enable = 0;
    unsigned int netif_index = 0;
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    struct npd_dhcpv6_relay_s entry;

    if (NULL == p_ifname)
    {
        ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
    }
    else
	{
        ret = npd_dhcpv6_relay_service_check(&is_service_enable);

        if (DHCPV6_RELAY_RETURN_CODE_SUCCESS != ret)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else if (0 == is_service_enable)
        {
            ret = DHCPV6_RELAY_RETURN_CODE_SERVICE_DISABLE;
        }
        else if (NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, p_ifname))
        {
            ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
        }
        else
        {
            memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
            
            if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == npd_dhcpv6_relay_server_get_byindex(netif_index, &entry))
        	{
        		if (0 == entry.is_enable)
        		{
        			ret = DHCPV6_RELAY_RETURN_CODE_INTFFACE_DISABLE; 
        		}
        		else
        		{
                    if (0 == memcmp(&(entry.nd6_server_in), &ipv6_address, sizeof(struct in6_addr)))
                    {
                        memset(&(entry.nd6_server_in), 0, sizeof(struct in6_addr));
                        ret = npd_dhcpv6_relay_server_set(&entry);
                    }
                    else
                    {
                        ret = DHCPV6_RELAY_RETURN_CODE_ADDRESS_NOT_COMPARE; 
                    }
        		}
        	}
            else
            {
                ret = DHCPV6_RELAY_RETURN_CODE_ERROR;
            }
        }
	}

    return ret;
}

unsigned int npd_dbus_dhcpv6_relay_interface_next(struct npd_dhcpv6_relay_s* entry)
{
    unsigned int ret = 0;
    
    if (0 == entry->netif_index)
    {
        ret = dbtable_hash_head(npd_db_table_dhcpv6_relay_hash_idx, NULL, entry, NULL);
    }
    else
	{
        ret = dbtable_hash_next(npd_db_table_dhcpv6_relay_hash_idx, entry, entry, NULL);
	}

    while ((0 == ret)
            && (0 == entry->is_enable)
                && ((0 == entry->nd6_server_in32[0]) && (0 == entry->nd6_server_in32[1])
                    && (0 == entry->nd6_server_in32[2]) && (0 == entry->nd6_server_in32[3])))
    {
        ret = dbtable_hash_next(npd_db_table_dhcpv6_relay_hash_idx, entry, entry, NULL);
    }

    return ((ret == 0) ? DHCPV6_RELAY_RETURN_CODE_SUCCESS : DHCPV6_RELAY_RETURN_CODE_ERROR);
}

unsigned int npd_dhcpv6_relay_show_running_global_config(char* buffer, int* size)
{
    unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    unsigned int is_service_enable = 0;

    *size = 0;
    
    ret = npd_dhcpv6_relay_service_check(&is_service_enable);
    if ((DHCPV6_RELAY_RETURN_CODE_SUCCESS == ret)  
        && (0 != is_service_enable))
    {
        *size = sprintf(buffer, "ipv6 dhcp relay enable\n");
    }
    
    return ret;
}

unsigned int npd_dhcpv6_relay_show_running_interface_config(char* buffer, int* size)
{
    char in6_buf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
    char l3_intf_name[32];
    char l3_fwd_intf_name[32];
    int length = 0;
    char* current = buffer;
    struct npd_dhcpv6_relay_s entry;

    *size = 0;
    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));

    while (DHCPV6_RELAY_RETURN_CODE_SUCCESS == npd_dbus_dhcpv6_relay_interface_next(&entry))
    {
        if (0 != entry.is_enable)
        {
            memset(l3_intf_name, 0, sizeof(l3_intf_name));
            if (0 == npd_netif_index_to_l3intf_name(entry.netif_index, l3_intf_name))
            {
                length += sprintf(current, "interface %s\n", l3_intf_name);
                current = buffer + length;
                
                length += sprintf(current, " ipv6 dhcp relay enable\n");
                current = buffer + length;

                memset(in6_buf, 0, sizeof(in6_buf));
                inet_ntop(AF_INET6, entry.nd6_server_in8, in6_buf, NPD_INET6_ADDRSTRLEN);
                
                if (0 != entry.nd6_server_fwd)
                {
                    memset(l3_fwd_intf_name, 0, sizeof(l3_fwd_intf_name));
                    if (0 == npd_netif_index_to_name(entry.nd6_server_fwd_ifidx, l3_fwd_intf_name))
                    {
                        length += sprintf(current, " ipv6 helper dhcp %s fwd-interface %s\n", in6_buf, l3_fwd_intf_name);
                        current = buffer + length;
                    }
                }
                else
                {
                    length += sprintf(current, " ipv6 helper dhcp %s\n", in6_buf);
                    current = buffer + length;
                }

                length += sprintf(current, "exit\n");
                current = buffer + length;
            }
        }
    }

    *size = length;
    
    return DHCPV6_RELAY_RETURN_CODE_SUCCESS;
}

DBusMessage *npd_dbus_dhcpv6_relay_global
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
	unsigned int is_enable = 0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &is_enable,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcpv6_relay_global(is_enable);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

DBusMessage *npd_dbus_dhcpv6_relay_interface
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
	unsigned int is_enable = 0;	
    char* p_ifname = NULL;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &is_enable,
								DBUS_TYPE_STRING, &p_ifname,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcpv6_relay_interface(is_enable, p_ifname);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

DBusMessage *npd_dbus_dhcpv6_relay_server
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
	unsigned int is_fwd_interface = 0;	
    char* p_ifname = NULL;
    char* p_fwd_ifname = NULL;
    struct in6_addr ipv6_address;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[0],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[1],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[2],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[3],
								DBUS_TYPE_UINT32, &is_fwd_interface,
								DBUS_TYPE_STRING, &p_ifname,
								DBUS_TYPE_STRING, &p_fwd_ifname,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcpv6_relay_server(is_fwd_interface, ipv6_address, p_ifname, p_fwd_ifname);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

DBusMessage *npd_dbus_dhcpv6_relay_no_server
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCPV6_RELAY_RETURN_CODE_SUCCESS;
    char* p_ifname = NULL;
    struct in6_addr ipv6_address;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[0],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[1],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[2],
								DBUS_TYPE_UINT32, &ipv6_address.s6_addr32[3],
								DBUS_TYPE_STRING, &p_ifname,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    ret = npd_dhcpv6_relay_no_server(ipv6_address, p_ifname);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

DBusMessage *npd_dbus_dhcpv6_relay_global_show
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
    unsigned int ret = 0;
    unsigned int is_service_enable = 0;
	DBusMessage* reply = NULL;
	DBusMessageIter iter;

    ret = npd_dhcpv6_relay_service_check(&is_service_enable);        

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &is_service_enable);
	return reply;
}

DBusMessage *npd_dbus_dhcpv6_relay_interface_show
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

    unsigned int ret = 0;
    unsigned int netif_index = 0;
    struct npd_dhcpv6_relay_s entry;

    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
    
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &netif_index,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    entry.netif_index = netif_index;

    ret = npd_dbus_dhcpv6_relay_interface_next(&entry);
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.is_enable);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.netif_index);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_fwd);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_fwd_ifidx);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[0]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[1]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[2]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[3]);

	return reply;
}

DBusMessage *npd_dbus_dhcpv6_relay_interface_show_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

    unsigned int ret = 0;
    unsigned int netif_index = 0;
    char* p_if_name = NULL;
    struct npd_dhcpv6_relay_s entry;

    memset(&entry, 0, sizeof(struct npd_dhcpv6_relay_s));
    
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &p_if_name,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

    if (NPD_TRUE == npd_intf_netif_get_by_name(&netif_index, p_if_name))
    {
        ret = npd_dhcpv6_relay_server_get_byindex(netif_index, &entry);
    }
    else
    {
        ret = DHCPV6_RELAY_RETURN_CODE_L3_NOT_EXIST;
    }

	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.is_enable);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.netif_index);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_fwd);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_fwd_ifidx);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[0]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[1]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[2]);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &entry.nd6_server_in32[3]);

	return reply;
}

DBusMessage *npd_dbus_dhcpv6_relay_show_running_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	char* buffer = NULL;
    char* current = NULL;
    int length = 0;
    unsigned int is_service_enable = 0;
    unsigned int ret = 0;

	buffer = (char*)malloc(NPD_DHCPV6_RELAY_SHOW_RUNNING_SIZE);
	if (NULL == buffer)
	{
		syslog_ax_dhcp_snp_err("DHCPv6 relay show running config, memory request faild.\n");
		return NULL;
	}
    
	memset(buffer, 0, NPD_DHCPV6_RELAY_SHOW_RUNNING_SIZE);
    current = buffer;

    ret = npd_dhcpv6_relay_show_running_global_config(current, &length);
    current = buffer + length;

    if (DHCPV6_RELAY_RETURN_CODE_SUCCESS == ret)
    {
        ret = npd_dhcpv6_relay_service_check(&is_service_enable);
        if ((DHCPV6_RELAY_RETURN_CODE_SUCCESS == ret)
            && (0 != is_service_enable))
        {
            length = 0;
            ret = npd_dhcpv6_relay_show_running_interface_config(current, &length);
            current += length;
        }
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &buffer);

	free(buffer);

	return reply;
}

#ifdef __cplusplus
}
#endif
#endif

