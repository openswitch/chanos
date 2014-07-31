
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* common_api.c
*
* CREATOR:
*       lizheng@autelan.com
*
* DESCRIPTION:
*       APIs for common . 
*
* DATE:
*       25/10/2010
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "lib/osinc.h"
#include "lib/common_api.h"

#include "npd/ipv6.h"

static struct 
{ 
	int len; 
	char *ipstring; 
} convert[] = {
        { 0,    "0.0.0.0"   },
        { 1,    "128.0.0.0" },
        { 2,    "192.0.0.0" },
        { 3,    "224.0.0.0"   },
        { 4,    "240.0.0.0"   },
        { 5,    "248.0.0.0"   },
        { 6,    "252.0.0.0"   },
        { 7,    "254.0.0.0"   },
        { 8,    "255.0.0.0"   },
        { 9,    "255.128.0.0"   },
        { 10,   "255.192.0.0"   },
        { 11,   "255.224.0.0"   },
        { 12,   "255.240.0.0"   },
        { 13,   "255.248.0.0"   },
        { 14,   "255.252.0.0"   },
        { 15,   "255.254.0.0"   },
        { 16,   "255.255.0.0"   },
        { 17,   "255.255.128.0"   },
        { 18,   "255.255.192.0"   },
        { 19,   "255.255.224.0"   },
        { 20,   "255.255.240.0"   },
        { 21,   "255.255.248.0"   },
        { 22,   "255.255.252.0"   },
        { 23,   "255.255.254.0"   },
        { 24,   "255.255.255.0"   },
        { 25,   "255.255.255.128"   },
        { 26,   "255.255.255.192"   },
        { 27,   "255.255.255.224"   },
        { 28,   "255.255.255.240"   },
        { 29,   "255.255.255.248"   },
        { 30,   "255.255.255.252"   },
        { 31,   "255.255.255.254"   },
        { 32,   "255.255.255.255"   }
};


/******************************************************/
/*************COMMON API for ip mask***********************/
/******************************************************/

int lib_get_mask_from_masklen(int masklen, int *mask)
{
    unsigned int ipv4_mask = 0;

	  if( masklen > 32 || masklen < 0 )
	  {
	 	   return FALSE;
	  }
	  
	  if( masklen == 0 )
	  {
		    *mask = 0;
	  }
	  else if( masklen == 32 )
	  {
		    *mask = 0xFFFFFFFF;
	  }
	  else {
		    ipv4_mask = (unsigned int )(-(1<<(32-(masklen)))); 
		    *mask = htonl(ipv4_mask);
    }

    return TRUE;
}

int lib_get_masklen_from_mask(int mask, int *masklen)
{
	unsigned short ipv4_masklen = 0;
	unsigned int j = 0;
	if( masklen == NULL) 
		return FALSE;
    mask = ntohl(mask);
	for (j = 0; j < 32; j++)
		{
		if(1 & (mask>>(31-j)))
			ipv4_masklen++;
		else
			break;
	}
	*masklen = ipv4_masklen;

	return TRUE;
}

int lib_get_masklen_from_string(char *netmask)
{
	int masklen = 0;

	while( masklen < (sizeof(convert)/sizeof(convert[0])) )
	{
		if( 0 == strncmp(netmask, convert[masklen].ipstring, strlen(netmask)) )
		{
			return masklen;
		}
		masklen++;
	}

	return -1;
}

int lib_get_string_from_masklen(char *mask,short masklen)
{    

    if (mask == NULL)
	{
	  return -1;
	}

    if( masklen < 0 || masklen > sizeof(convert)/sizeof(convert[0]) )
    {
        return -1;
    }
    else if( masklen==convert[masklen].len )
    {
        sprintf(mask, convert[masklen].ipstring );
    }

    return -1;
}

int lib_get_ip_from_string(char* ipaddr)
{
	char bak[17];
	char *cp,*cp1,*cptail;
	char ip[4];
	int i,j;
	int little;
	j = strlen(ipaddr);
	if((j<7) ||(j>15))
		return -1;
	
	for(i=0;i<j;i++)
		if(!((*(ipaddr+i) == '.')||((*(ipaddr+i)>='0') &&(*(ipaddr+i)<='9'))))
			return -1;
			
	
	strncpy(bak,ipaddr,17);
	i = 0;
	cptail = bak;
	while(i<4)
	{
		cp = cptail;
	
		if(i!=3)
		{
			cp1 = (char *)strchr(cp,'.');
			if(!cp1)
				return -1;
			cptail = cp1 +1;
			*cp1 = 0;
		}
		else
		{
			cp1 = (char *)strchr(cp, '.');
			if(cp1)
				return -1;
		}

		if(strlen(cp)<1||strlen(cp)>3)
			return -1;
	
		little = atoi(cp);
		if(little > 255)
			return -1;
		
		ip[i] = (char)little;
		i++;
	}
	
	i = 0;	
	bcopy(((char *)(&ip[0])),(char *)&i,1);
	bcopy(((char *)(&ip[1])),(char *)&i+1,1);
	bcopy(((char *)(&ip[2])),(char *)&i+2,1);
	bcopy(((char *)(&ip[3])),(char *)&i+3,1);

	return i;
}

int lib_get_string_from_ip( char *ipaddr, int ip )
{
	unsigned char i[4];
	
	i[0] = i[1] = i[2] = i[3] = 0;
	bcopy(( char *)&ip,(( char *)(&i[0])),1);
	bcopy(( char *)&ip+1,(( char *)(&i[1])),1);
	bcopy(( char *)&ip+2,(( char *)(&i[2])),1);
	bcopy(( char *)&ip+3,(( char *)(&i[3])),1);
	
	sprintf( ipaddr, "%d.%d.%d.%d", i[0], i[1], i[2], i[3] );

	return TRUE;
}


int lib_get_maskv6_from_masklen(int masklen, void *maskv6)
{
	char i = 0;
	ip6_addr *mask = (ip6_addr *)maskv6;

	if( masklen > 128 || masklen < 0 )
	{
		return FALSE;
	}

	if( masklen == 0 )
	{
		memset(mask,0x0, IPV6_MAX_BYTELEN);
	}
	else if( masklen == 128 )
	{
		memset(mask,0xFF, IPV6_MAX_BYTELEN);
	}
	else {
		for(i=0;i<masklen;i++)
		{
			mask->u6_addr32[i/32] |= htonl(0x1<<(31-i%32));
		}	
	}

	return TRUE;
}

int lib_get_maskv6len_from_mask(void *maskv6, int *masklen)
{
	unsigned int i;
    unsigned int ipv6_masklen = 0;
	ip6_addr *mask = (ip6_addr *)maskv6;
    
	if( masklen == NULL) 
		return FALSE;

    for(i = 0; i < 16; i++)
    {
        while(mask->u6_addr8[i])
        {
            ipv6_masklen++;
            mask->u6_addr8[i] = mask->u6_addr8[i] & (mask->u6_addr8[i] - 1);
        }
    }
	*masklen = ipv6_masklen;

	return TRUE;
}

char* lib_get_string_from_ipv6( char *string, void *ipv6 )
{
	int i = 0;
	char *curr = string;
	int len = 0; 
	ip6_addr *ip = (ip6_addr *)ipv6;

	if( curr == NULL || ipv6 == NULL)
		return NULL;

	for(i=0;i<8;)
	{
		len = sprintf(curr,"%04x",ntohs(ip->u6_addr16[i++]));
		curr += len;
		if(i>=8) break;
		len = sprintf(curr,":");
		curr += len;		
	}
	*curr = '\0';
	
	return string;
}

char* lib_get_abbr_string_from_ipv6( char *string, void *ipv6 )
{
	if(string == NULL || ipv6 == NULL )
		return NULL;

	inet_ntop(AF_INET6, ipv6, string, INET6_ADDRSTRLEN);
	
	return string;
}

int lib_get_ipv6_from_string(const char *str,  void *ipv6_addr )
{	
	if(1 == inet_pton (AF_INET6, str, ipv6_addr))
		return 0;

	return -1;
}

int lib_get_prefix_ipv6_from_string(const char *str,  void *ipv6_addr )
{
	char *pnt;
	char *cp;
	int ret;  
	struct prefix_ip6_addr *p = (struct prefix_ip6_addr *)ipv6_addr;

	pnt = strchr (str, '/');

	/* If string doesn't contain `/' treat it as host route. */
	if (pnt == NULL) 
	{
		ret = inet_pton (AF_INET6, str, &p->prefix);
		if (ret == 0)
			return 0;
		p->prefixlen = IPV6_MAX_BITLEN;
	}
	else 
	{
		int plen;

		cp = malloc((pnt - str) + 1);
		strncpy (cp, str, pnt - str);
		*(cp + (pnt - str)) = '\0';
		ret = inet_pton (AF_INET6, cp, &p->prefix);
		free (cp);
		if (ret == 0)
			return 0;
		plen = (unsigned char) atoi (++pnt);
		if (plen > 128)
			return 0;
		p->prefixlen = plen;
	}
	p->family = AF_INET6;

	return ret;
}

int ipv6_addr_is_valid(const ip6_addr *addr) 
{
	return (addr->u6_addr32[0] & addr->u6_addr32[1] &addr->u6_addr32[2] &addr->u6_addr32[3])\
			!= htonl(0xFFFFFFFF);
}


int ipv6_addr_any(const ip6_addr *a)
{
	return ((a->u6_addr32[0] | a->u6_addr32[1] | 
		 a->u6_addr32[2] | a->u6_addr32[3] ) == 0); 
}

int ipv6_addr_is_multicast(const ip6_addr *addr)
{
	return (addr->u6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000);
}

unsigned ipv6_addr_scope2type(unsigned scope)
{
	switch(scope) {
	case IPV6_ADDR_SCOPE_NODELOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_NODELOCAL) |
			IPV6_ADDR_LOOPBACK);
	case IPV6_ADDR_SCOPE_LINKLOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL) |
			IPV6_ADDR_LINKLOCAL);
	case IPV6_ADDR_SCOPE_SITELOCAL:
		return (IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_SITELOCAL) |
			IPV6_ADDR_SITELOCAL);
	}
	return IPV6_ADDR_SCOPE_TYPE(scope);
}




int __ipv6_addr_type(const ip6_addr *addr)
{
	int st;

	st = addr->u6_addr32[0];

	/* Consider all addresses with the first three bits different of
	   000 and 111 as unicasts.
	 */
	if ((st & htonl(0xE0000000)) != htonl(0x00000000) &&
	    (st & htonl(0xE0000000)) != htonl(0xE0000000))
		return (IPV6_ADDR_UNICAST |
			IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));

	if ((st & htonl(0xFF000000)) == htonl(0xFF000000)) {
		/* multicast */
		/* addr-select 3.1 */
		return (IPV6_ADDR_MULTICAST |
			ipv6_addr_scope2type(IPV6_ADDR_MC_SCOPE(addr)));
	}

	if ((st & htonl(0xFFC00000)) == htonl(0xFE800000))
		return (IPV6_ADDR_LINKLOCAL | IPV6_ADDR_UNICAST |
			IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL));		/* addr-select 3.1 */
	if ((st & htonl(0xFFC00000)) == htonl(0xFEC00000))
		return (IPV6_ADDR_SITELOCAL | IPV6_ADDR_UNICAST |
			IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_SITELOCAL));		/* addr-select 3.1 */
	if ((st & htonl(0xFE000000)) == htonl(0xFC000000))
		return (IPV6_ADDR_UNICAST |
			IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));			/* RFC 4193 */

	if ((addr->u6_addr32[0] | addr->u6_addr32[1]) == 0) {
		if (addr->u6_addr32[2] == 0) {
			if (addr->u6_addr32[3] == 0)
				return IPV6_ADDR_ANY;

			if (addr->u6_addr32[3] == htonl(0x00000001))
				return (IPV6_ADDR_LOOPBACK | IPV6_ADDR_UNICAST |
					IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_LINKLOCAL));	/* addr-select 3.4 */

			return (IPV6_ADDR_COMPATv4 | IPV6_ADDR_UNICAST |
				IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.3 */
		}

		if (addr->u6_addr32[2] == htonl(0x0000ffff))
			return (IPV6_ADDR_MAPPED |
				IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.3 */
	}

	return (IPV6_ADDR_RESERVED |
		IPV6_ADDR_SCOPE_TYPE(IPV6_ADDR_SCOPE_GLOBAL));	/* addr-select 3.4 */
}


int ipv6_addr_type(const ip6_addr *addr)
{
	return __ipv6_addr_type(addr) & 0xffff;
}

int ipv6_addr_valid_check(struct in6_addr *ipAddr)
{
    int type = ipv6_addr_type((const ip6_addr*)ipAddr);
    
	if(!ipv6_addr_is_valid((const ip6_addr*)ipAddr))
		return 0;

	if((type != IPV6_ADDR_UNICAST) &&
 		(type != IPV6_ADDR_MAPPED)&& 
 		(type != IPV6_ADDR_COMPATv4))
 	{
 		return 0;
 	}

	return 1;
}

int mac_format_check
(
    char* str,
    int len
)
{
    int i = 0;
    unsigned int result = 0;
    char c = 0;

    if (17 != len)
    {
        return -1;
    }

    for (; i<len; i++)
    {
        c = str[i];

        if ((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i))
        {
            if ((':'!=c)&&('-'!=c))
                return -1;
        }
        else if ((c>='0'&&c<='9')||
                 (c>='A'&&c<='F')||
                 (c>='a'&&c<='f'))
            continue;
        else
        {
            result = -1;
            return result;
        }
    }

    if ((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
            (str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
            (str[8] != str[11])||(str[8] != str[14]))
    {
        result = -1;
        return result;
    }

    return result;
}

int parse_mac_addr(char* input,unsigned char *  macAddr)
{
    int i = 0;
    char cur = 0,value = 0;

    if ((NULL == input)||(NULL == macAddr))
    {
        return -1;
    }

    if (-1 == mac_format_check(input,strlen(input)))
    {
        return -1;
    }

    for (i = 0; i <6; i++)
    {
        cur = *(input++);

        if (cur == ':')
        {
            i--;
            continue;
        }

        if ((cur >= '0') &&(cur <='9'))
        {
            value = cur - '0';
        }
        else if ((cur >= 'A') &&(cur <='F'))
        {
            value = cur - 'A';
            value += 0xa;
        }
        else if ((cur >= 'a') &&(cur <='f'))
        {
            value = cur - 'a';
            value += 0xa;
        }

        macAddr[i] = value;
        cur = *(input++);

        if ((cur >= '0') &&(cur <='9'))
        {
            value = cur - '0';
        }
        else if ((cur >= 'A') &&(cur <='F'))
        {
            value = cur - 'A';
            value += 0xa;
        }
        else if ((cur >= 'a') &&(cur <='f'))
        {
            value = cur - 'a';
            value += 0xa;
        }

        macAddr[i] = (macAddr[i]<< 4)|value;
    }

    return 0;
}


#ifdef __cplusplus
}
#endif


