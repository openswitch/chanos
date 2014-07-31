#ifndef __MAN_STR_PARSE_H__
#define __MAN_STR_PARSE_H__

extern int is_system_cascade;
extern int is_system_box;
extern int local_unitno;

#define DCLI_SYS_ISCASCADE (is_system_cascade)
#define DCLI_SYS_ISBOX (is_system_box)

#define TRUE 1
#define FALSE 0
#define ALIAS_NAME_SIZE 		0x15
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define ALIAS_NAME_RESD_ERROR	(ALIAS_NAME_LEN_ERROR+3)  /* Reserved Name  */


#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"


typedef struct vid_range_s
{
    unsigned int start;
    unsigned int end;
}vid_range_t;

/*parse string to trunk netif index*/
unsigned int parse_str_to_bond_index(
    char* str
    );

/*parse string to trunk id*/
int parse_trunk_no(
    char* str,
    unsigned short* trunkId
    );

/*parse string to vlan netif index*/
unsigned int parse_str_to_vlan_index(
    char* str
    );

/*parse string to vlan id*/
int parse_vlan_no(
    char* str,
    unsigned short* vlanId
    );


/*pare portlist string to port id array. the port list string format is
  1/1/1,1/1/2-6,1/1/10*/
#define MAX_PORT_NUMBER_IN_PORTLIST 300
unsigned long * parse_portlist( 
    char * portlist 
    );

#define MAX_VLAN_IN_LIST 200
/*pare vlanlist string to vlan id array. the vlan list string format is
  1,2-6,10*/
unsigned long * parse_vlanlist( 
    char * vlanlist 
    );

/*same as parse_vlan_no*/
int parse_subvlan_no(
    char* str,
    unsigned short* subvlanId
    );

int parse_vlan_string(
    char* input
    );

int trunk_name_legal_check(
    char* str,
    unsigned int len
    );


int vlan_name_legal_check(
    char* str,
    unsigned int len
    );

int parse_chassis_slot_module_port_subport_no(
    char *str,
    unsigned char *chassis, 
    unsigned char *slotno, 
    unsigned char *moduleno, 
    unsigned char *portno, 
    unsigned char *subportno
    );

unsigned int parse_str_to_eth_index(
    char *str
    );

unsigned int parse_intf_name_to_eth_index(
    char *str
    );

int parse_slotport_no(
    char *str,
    unsigned char *slotno,
    unsigned char *portno
    );

int parse_slotport_tag_no(
    char *str,
    unsigned char *slotno,
    unsigned char *portno,
    unsigned int * tag1,
    unsigned int *tag2);

int parse_eth_index_to_name(
    unsigned int eth_g_index, 
    char *name);



unsigned int parse_intf_name_to_netif_index(
    unsigned char *str
    );

void dcli_netif_name_convert_to_interface_name(
    char *netif_name, 
    char *ifname
    );

int parse_slotno_localport(
    char* str,
    unsigned int *slotno,
    unsigned int *portno
    );

/*
*MAC addr parse from string
*/
long GetMacAddr( 
    char * string, 
    char * pucMacAddr 
    );

/*****************************************************************
   Function Name	: GetMacStr
   Processing		: 根据Mac 得到<H.H.H>
   Input Parameters	: pucMacAddr--Mac地址
   Output Parameters : string--H.H.H的字符串
   Return Values	: SSL_E_NONE成功，SSL_ERROR错误
******************************************************************/
long GetMacStr( 
     char * pucMacAddr , 
     char * string
     );


int is_muti_brc_mac(
    ETHERADDR *mac
    );

int is_bcast_mac(
    ETHERADDR *mac
    );

int is_mcast_mac(
    ETHERADDR *mac
    );

int mac_format_check(
	char* str,
	int len
    );


/*here str space must greater than 17*/
int mac_to_str(
    ETHERADDR *mac, 
    char *str
    );

/*
* ip addr string to 
*/

int parse_ipaddr_withmask(
    char * str,
    unsigned long *ipAddress,
    unsigned int *mask
    );

int parse_ip_addr(
    char * str,
    unsigned long *ipAddress
    );

long mlib_ipv4_prefix_match ( 
    char * str 
    );

long CheckMaskAddr( 
    unsigned long ulMask 
    );

long IsSubnetBroadcast( 
    unsigned long ulIpAddr, 
    unsigned long ulMask 
    );

/*将A.B.C.D/M 解析成 ip ,mask */
/*例如:1.2.3.4/24,最后返回内存(网络字节序),0x01,0x02,0x03,0x04/0xff,0xff,0xff,0x00*/
long IpListToIp( 
    const char * IpList, 
    long * ulIpAddr, 
    long * ulMask 
    );

int parse_param_no(
    char* str,
    unsigned int* param
    );

#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))


int inet_atoi (
    const char *cp, 
    unsigned int  *inaddr
    );

unsigned long dcli_ip2ulong(
    char *str
    );

int parse_ip_check(
    char * str
    );

int	ip_address_format2ulong(
    char ** buf,
    unsigned long *ipAddress,
    unsigned int *mask
    );

int parse_char_ID(char* str,unsigned char* ID);
int parse_short_ID(char* str,unsigned short* ID);
int parse_int_ID(char* str,unsigned int* ID);

#endif
