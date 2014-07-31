#ifndef _MAN_IPV6_H
#define _MAN_IPV6_H

typedef union{
    unsigned char	u6_addr8[16];
    unsigned short	u6_addr16[8];
    unsigned int 	u6_addr32[4];
}man_ip6_addr;


struct man_prefix_ip6_addr
{
  unsigned char family;
  unsigned char prefixlen;
  man_ip6_addr prefix;
};


#endif //_MAN_IPV6_H


