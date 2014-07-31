#ifndef __NPD_DHCP_SNP_OPTIONS_H__
#define __NPD_DHCP_SNP_OPTIONS_H__

/*********************************************************
*	macro define													*
**********************************************************/


/* 
*************************************************************
* struct defined use in option 82 fill
*************************************************************
*/
typedef struct NPD_DHCP_OPT82_SUBOPT_S{
	unsigned char subopt_type;
	unsigned char subopt_len;
}NPD_DHCP_OPT82_SUBOPT_T;

/* Circuit-ID */
/* use in extended format */
typedef struct NPD_DHCP_OPT82_CIRCUITID_S{
	unsigned char circuit_type;
	unsigned char circuit_len;
}NPD_DHCP_OPT82_CIRCUITID_T;

typedef struct NPD_DHCP_OPT82_CIRCUITID_DATA_S{
	unsigned short vlanid;
	unsigned int portindex;
}NPD_DHCP_OPT82_CIRCUITID_DATA_T;


/* Remote-ID */
/* use in extended format */
typedef struct NPD_DHCP_OPT82_REMOTEID_S{
	unsigned char remote_type;
	unsigned char remote_len;
}NPD_DHCP_OPT82_REMOTEID_T;

typedef struct NPD_DHCP_OPT82_REMOTEID_DATA_S{
	unsigned char mac[6];
}NPD_DHCP_OPT82_REMOTEID_DATA_T;


typedef struct NPD_DHCP_OPT82_COM_DATA_S{
	unsigned char data[NPD_DHCP_SNP_CIRCUITID_STR_LEN];
}NPD_DHCP_OPT82_COM_DATA_T;



#endif




