#ifndef __MAN_DHCP_SNP_H__
#define __MAN_DHCP_SNP_H__

#define DCLI_DHCP_SNP_REMOTEID_STR_LEN		(64)		/* length of user-defined remote-id string, 64	*/

struct DCLI_DHCP_SNP_BINDTABLE
{
	unsigned int  bind_type;
	unsigned char state;
	unsigned char haddr_len;
	unsigned char chaddr[6];
	unsigned short vlanId;
	unsigned int ip_addr;
	unsigned int lease_time;
	unsigned int sys_escape; 
	unsigned int cur_expire;	   
	unsigned int ifindex;
	unsigned int flags;
};

struct dcli_dhcp_snooping_global_status_s
{
	unsigned char dhcp_snp_enable;
	unsigned char dhcp_snp_opt82_enable;
};

struct dcli_dhcp_information_global_status_s
{
    unsigned char dhcp_snp_opt82_format_type;
	unsigned char dhcp_snp_opt82_fill_format_type;
	unsigned char dhcp_snp_opt82_remoteid_type;
	unsigned char dhcp_snp_opt82_remoteid_str[64];
};

struct dcli_dhcp_snp_port_s
{
    unsigned int    global_port_ifindex;
    unsigned int    trust_mode;
};

struct dcli_dhcp_snp_information_port_s
{
	unsigned int    global_port_ifindex;
	unsigned char   opt82_strategy;           /* DHCP-Snooping option82 strategy type of port	*/
	unsigned char   opt82_circuitid;          /* DHCP-Snooping option82 circuitid type of port	*/
	unsigned char   opt82_circuitid_str[64];  /* DHCP-Snooping option82 circuitid content of port */
};

typedef enum {
	DCLI_DHCP_SNP_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	DCLI_DHCP_SNP_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	DCLI_DHCP_SNP_BINDING_OPERATE_TYPE_INVALID
}DCLI_DHCP_SNP_BINDING_OPERATE_TYPE;

typedef enum {
	DCLI_DHCP_SNP_PORT_MODE_NOTRUST = 0, 				/* DHCP-Snooping trust mode of port: no trust	*/
	DCLI_DHCP_SNP_PORT_MODE_NOBIND,						/* DHCP-Snooping trust mode of port: trust but no bind	*/
	DCLI_DHCP_SNP_PORT_MODE_TRUST,						/* DHCP-Snooping trust mode of port: trust	*/
	DCLI_DHCP_SNP_PORT_MODE_INVALID
}DCLI_DHCP_SNP_PORT_MODE_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE = 0,		/* Config the configuration strategy for option 82: replace			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP,				/* Config the configuration strategy for option 82: drop			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP,				/* Config the configuration strategy for option 82: keep			*/
	DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE;

typedef enum {
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD = 0,	/* Add dhcp-snooping static binding item to bind table			*/
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL,		/* Delete dhcp-snooping static binding item from bind table		*/
	DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID
}DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE;

typedef enum {
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT = 0,		/* DHCP-Snooping circuit-id type of option 82: default, vlan id + port index	*/
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR,			/* DHCP-Snooping circuit-id type of option 82: user define string	*/
	DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID
}DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE;


#endif
