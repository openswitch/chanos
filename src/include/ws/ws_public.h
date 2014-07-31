#ifndef _WS_PUBLIC_H
#define _WS_PUBLIC_H

#define IFI_NAME 16
#define	IFI_HADDR 8

typedef struct ifi_info
{
  char ifi_name[IFI_NAME];
  u_char ifi_haddr[IFI_HADDR];
  u_short ifi_hlen;
  short ifi_flags;
  short ifi_myflags;
  struct sockaddr *ifi_addr;
  struct sockaddr *ifi_brdaddr;
  struct sockaddr *ifi_dstaddr;
  struct sockaddr *ifi_mask;
  struct ifi_info *ifi_next;
}ifi_info;

typedef struct inf
{
	char if_addr[32];
	char if_name[32];
	char if_stat[32];
	char if_mask[32];
	int  upflag;
	struct inf *next;
}infi;
#define IFI_ALIAS 1
struct ifi_info *get_ifi_info(int, int);

extern void free_ifi_info(ifi_info *ifihead);

extern ifi_info *get_ifi_info(int family, int doaliases);

extern char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

extern int interface_list_ioctl (int af,struct inf * interface);

extern void free_inf(infi * infter);

#endif 
