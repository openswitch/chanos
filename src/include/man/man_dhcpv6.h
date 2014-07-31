#ifndef __MAN_DHCPV6_H__
#define __MAN_DHCPV6_H__
#include <netinet/ip6.h>
#include <netinet/in.h>

#define IN6_ARE_ADDR_AND(a,b) \
    do {(((unsigned int *) (a))[0] = (((unsigned int *) (a))[0] & ((const unsigned int *) (b))[0]));   \
        (((unsigned int *) (a))[1] = (((unsigned int *) (a))[1] & ((const unsigned int *) (b))[1]));   \
        (((unsigned int *) (a))[2] = (((unsigned int *) (a))[2] & ((const unsigned int *) (b))[2]));   \
        (((unsigned int *) (a))[3] = (((unsigned int *) (a))[3] & ((const unsigned int *) (b))[3]));   \
	   } while (0)

#define D6O_IA_NA               3
#define D6O_IA_TA               4
#define D6O_IA_PD               25 /* RFC3633 */

#define DHCPV6_IF_LEN (32)
#define DHCPV6_NAME_LEN (64)
#define DHCPV6_DUID_SIZE    (64)
#define DHCPV6_IPV6_ADDRESS_SIZE    (48)
#define DHCPV6_VENDOR_SIZE    (256)
#define DHCPV6_DNS_NUM  (3)

struct man_dhcpdv6_service_s
{
	unsigned int    is_enable;
};

struct man_dhcpdv6_host_s
{
	char ipv6_address[48];
    char duid[128];
};

struct man_dhcpdv6_pool_s
{
	char pool_name[DHCPV6_NAME_LEN];
    char domain[DHCPV6_IPV6_ADDRESS_SIZE];
    char dns_server[DHCPV6_DNS_NUM][DHCPV6_IPV6_ADDRESS_SIZE];
    char prefix[DHCPV6_IPV6_ADDRESS_SIZE];
    char range_low[DHCPV6_IPV6_ADDRESS_SIZE];
    char range_high[DHCPV6_IPV6_ADDRESS_SIZE];
    unsigned int preferred_lifetime;
    unsigned int valid_lifetime;
    unsigned int preference;
    unsigned int rapid_commit;
    unsigned int enterprise_number;
    char vendor_data[DHCPV6_VENDOR_SIZE];
};

struct man_dhcpdv6_if_s
{
	unsigned int    is_enable;
	char   if_name[DHCPV6_IF_LEN];
};

#define DHCPDV6_DUID_LEN    14

struct man_dhcpdv6_ia_xx_s
{
    unsigned int iaid;
    unsigned int duid_len;
    unsigned char duid[DHCPDV6_DUID_LEN];
	unsigned short ia_type;
 	time_t cltt;
};

struct man_dhcpdv6_iasubopt_s
{
    struct man_dhcpdv6_ia_xx_s ia;
	struct in6_addr addr;			/* IPv6 address/prefix */
	unsigned char plen;				/* iaprefix prefix length */
	unsigned char state;			/* state */
	time_t hard_lifetime_end_time;		/* time address expires */
	time_t soft_lifetime_end_time;		/* time ephemeral expires */
	unsigned int prefer;			/* cached preferred lifetime */
	unsigned int valid;			/* cached valid lifetime */
    int heap_index;				/* index into heap, or -1 (internal use only) */
};

 
#endif

