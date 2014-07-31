#ifndef _COMMON_API_H_
#define _COMMON_API_H_


typedef union{
    unsigned char	u6_addr8[16];
    unsigned short	u6_addr16[8];
    unsigned int 	u6_addr32[4];
}ip6_addr;

typedef struct prefix_ip6_addr{
	unsigned char family;
	unsigned char prefixlen;
	ip6_addr prefix;
} prefix_ipv6_stc;

int lib_get_mask_from_masklen
(
	int masklen, 
	int *mask
);
int lib_get_masklen_from_mask
(
	int mask, 
	int *masklen
);
int lib_get_masklen_from_string
(
	char *netmask
);
int lib_get_string_from_masklen
(
	char *mask,
	short masklen
);
int lib_get_ip_from_string
(
	char* ipaddr
);
int lib_get_string_from_ip
(
	char *ipaddr, 
	int ip 
);

int lib_get_maskv6len_from_mask
(
	void *maskv6, 
	int *masklen
);

int lib_get_maskv6_from_masklen
(
	int masklen, 
	void *maskv6
);

char* lib_get_string_from_ipv6
(
	char *string, 
	void *ipv6 
);

char* lib_get_abbr_string_from_ipv6
(
	char *string, 
	void *ipv6 
);

int lib_get_ipv6_from_string
(
	const char *str,  
	void *ipv6_addr 
);

int lib_get_prefix_ipv6_from_string
(
	const char *str, 
	void *prefix6
);

int ipv6_addr_type(const ip6_addr *addr);
int ipv6_addr_valid_check(struct in6_addr *ipAddr);
unsigned ipv6_addr_scope2type(unsigned scope);
int ipv6_addr_is_multicast(const ip6_addr *addr);
int ipv6_addr_any(const ip6_addr *a);

int ipv6_addr_is_valid(const ip6_addr *addr) ;

int parse_mac_addr(char* input,unsigned char *  macAddr);


#endif

