#ifndef _NPD_IPV6_H
#define _NPD_IPV6_H

#define IPV6_MAX_BYTELEN    16
#define IPV6_MAX_BITLEN    128
#define IPV6_MAX_PREFIXLEN 128

#define IPV6STR "%04x.%04x.%04x.%04x.%04x.%04x.%04x.%04x"
#define IPV6_2_STR(a) (a).u6_addr16[0],(a).u6_addr16[1],(a).u6_addr16[2],(a).u6_addr16[3],(a).u6_addr16[4],(a).u6_addr16[5],(a).u6_addr16[6],(a).u6_addr16[7] 


#ifndef __KERNEL__

#define NEXTHDR_HOP		0	/* Hop-by-hop option header. */
#define NEXTHDR_TCP		6	/* TCP segment. */
#define NEXTHDR_UDP		17	/* UDP message. */
#define NEXTHDR_IPV6		41	/* IPv6 in IPv6 */
#define NEXTHDR_ROUTING		43	/* Routing header. */
#define NEXTHDR_FRAGMENT	44	/* Fragmentation/reassembly header. */
#define NEXTHDR_ESP		50	/* Encapsulating security payload. */
#define NEXTHDR_AUTH		51	/* Authentication header. */
#define NEXTHDR_ICMP		58	/* ICMP for IPv6. */
#define NEXTHDR_NONE		59	/* No next header */
#define NEXTHDR_DEST		60	/* Destination options header. */
#define NEXTHDR_MOBILITY	135	/* Mobility header. */

#define NEXTHDR_MAX		255


/*
 *	Addr scopes
 */
#define IPV6_ADDR_MC_SCOPE(a)	\
	((a)->u6_addr8[1] & 0x0f)	/* nonstandard */
#define __IPV6_ADDR_SCOPE_INVALID	-1
#define IPV6_ADDR_SCOPE_NODELOCAL	0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL	0x02
#define IPV6_ADDR_SCOPE_SITELOCAL	0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL	0x08
#define IPV6_ADDR_SCOPE_GLOBAL		0x0e


/*
 *	Addr type
 *	
 *	type	-	unicast | multicast
 *	scope	-	local	| site	    | global
 *	v4	-	compat
 *	v4mapped
 *	any
 *	loopback
 */

#define IPV6_ADDR_ANY		  0x0000U
#define IPV6_ADDR_UNICAST     0x0001U	
#define IPV6_ADDR_MULTICAST   0x0002U	
#define IPV6_ADDR_LOOPBACK	  0x0010U
#define IPV6_ADDR_LINKLOCAL	  0x0020U
#define IPV6_ADDR_SITELOCAL   0x0040U
#define IPV6_ADDR_COMPATv4	  0x0080U
#define IPV6_ADDR_SCOPE_MASK  0x00f0U
#define IPV6_ADDR_MAPPED	  0x1000U
#define IPV6_ADDR_RESERVED	  0x2000U	/* reserved address space */

#define IPV6_ADDR_SCOPE_TYPE(scope)	((scope)<<16)

#endif

/*
*   ADDR operation
*/
#define IPV6_NET_EQUAL(ADDR1,ADDR2,MASK1,MASK2)\
			((((ADDR1).u6_addr32[0]&(MASK1).u6_addr32[0])==((ADDR2).u6_addr32[0]&(MASK2).u6_addr32[0]))\
			&&(((ADDR1).u6_addr32[1]&(MASK1).u6_addr32[1])==((ADDR2).u6_addr32[1]&(MASK2).u6_addr32[1]))\
			&&(((ADDR1).u6_addr32[2]&(MASK1).u6_addr32[2])==((ADDR2).u6_addr32[2]&(MASK2).u6_addr32[2]))\
			&&(((ADDR1).u6_addr32[3]&(MASK1).u6_addr32[3])==((ADDR2).u6_addr32[3]&(MASK2).u6_addr32[3])))	    

#define IPV6_ADDR_ZERO(ADDR) \
			( ((ADDR).u6_addr32[0]==0) && ((ADDR).u6_addr32[1]==0) && \
			  ((ADDR).u6_addr32[2]==0) && ((ADDR).u6_addr32[3]==0))

#define IPV6_ADDR_CMP(D,S)   memcmp ((D), (S), IPV6_MAX_BYTELEN)
#define IPV6_ADDR_SAME(D,S)  (memcmp ((D), (S), IPV6_MAX_BYTELEN) == 0)
#define IPV6_ADDR_COPY(D,S)  memcpy ((D), (S), IPV6_MAX_BYTELEN)


#endif //_NPD_IPV6_H


