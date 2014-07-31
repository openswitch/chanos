
#ifdef HAVE_AAA
#ifndef _MAN_STA_H_
#define _MAN_STA_H_

#define OKB 1024
#define OMB (1024*1024)
#define OGB (1024*1024*1024)

#define STA_TYPE_WIRED     0x1
#define STA_TYPE_WIRELESS  0x2
#define STA_TYPE_PORTAL    0x3


#define PRINTKMG(stream,variable)\
{ \
	double flux; \
	if( stream < OKB ){ \
		vty_out(vty,"%s:	%llu(B)\n",variable,stream); \
	} else if((stream >= OKB) && (stream < OMB)){ \
		flux = (double)stream/OKB; \
		vty_out(vty,"%s:	%.1f(KB)\n",variable,flux);  \
	} else if((stream >= OMB) && (stream < OGB)){ \
		flux = (double)stream/OMB; \
		vty_out(vty,"%s:	%.1f(MB)\n",variable,flux); \
	} else{ \
		flux = (double)stream/OGB; \
		vty_out(vty,"%s:	%.1f(GB)\n",variable,flux);	\
	} \
} 

typedef struct{
	unsigned char id[NAME_LEN+1];
	unsigned int netif_index;
	unsigned short vlan_id;
	unsigned char ip_addr[IP_LENGTH];
	unsigned char mac[6];
}wired_user_info_s;

#endif
#endif
