#ifndef __MAN_PORTAL_H__
#define __MAN_PORTAL_H__

#define MIN_VLANID 1
#define MAX_VLANID 4094
#define MAX_L3INTF_VLANID 4095
#define MIN_BONDID       0
#define MAX_BONDID       7
#define MAXLEN_BOND_CMD  128
#define MAXLEN_BOND_NAME 5   /*bond0~bond7*/
#define ARPSNP_FLAG_DROP 0x2
#define MAC_ADDRESS_LEN 6

#define IF_NAMESIZE 20
#define DCLI_SET_FDB_ERR 0xff

#define INTERFACE_NAMSIZ      20
char * dcli_error_info_intf(int errorCode);


#define NAME_LEN	 32
typedef struct {
	char authtype;
	char state;
	unsigned char id[NAME_LEN+1];
	unsigned char passwd[NAME_LEN+1];
	unsigned int ipaddr;
	unsigned int netif_index;
}portal_user_info_s;

typedef struct
{
	unsigned int ipaddr;
	unsigned int port;
	unsigned int enable;
	unsigned char URL[256];
}dcli_asd_portal_srv_info;

typedef struct{
	int enable;
	dcli_asd_portal_srv_info srv[4];
}dcli_asd_portal_info;

#endif /*__MAN_PORTAL_H__*/

