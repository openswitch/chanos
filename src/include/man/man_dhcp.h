#ifndef __MAN_DHCP_H__
#define __MAN_DHCP_H__

#define DHCPR_NAME_LEN (64)

#define DHCPD_NAME_LEN (64)
#define DHCPD_DNS_NUM  (3)
#define DHCPD_WINS_NUM (1)
#define DHCPD_GATEWAY_NUM (1)


enum dhcpd_host_type {
	DHCPD_HOST_DYNAMIC,
	DHCPD_HOST_STATIC,
	DHCPD_HOST_TYPE_MAX
};

enum lease_query_mode{
	QUERY_BY_IP_ONLY,
	QUERY_BY_IP_RANGE,
	QUERY_BY_IP_MASK,
	QUERY_BY_HARDWARE,
	QUERY_BY_TYPE
};

struct lease_query_data {
	unsigned int ipmin;
	unsigned int ipmax;
	unsigned int ipmask;
	unsigned char hostMac[6];
	unsigned int type;
};

enum option_code_value_type
{
    option_code_value_type_null,
    option_code_value_type_ascii,
    option_code_value_type_hex,
    option_code_value_type_ip
};

struct dcli_dhcpd_pool
{
    #define MAN_DHCP_POOL_FORBIDDEN_IP_MAX_COUNT    8
    #define DHCP_POOL_OPTION43_MAX_COUNT    8
	unsigned char poolName[DHCPD_NAME_LEN];
	unsigned int  netip_addr;
	unsigned int  netmask_addr;
	unsigned int  iprange_min_addr;
	unsigned int  iprange_max_addr;
	unsigned int  dns_addr[DHCPD_DNS_NUM];
	unsigned int  wins_addr[DHCPD_WINS_NUM];
	unsigned int  default_lease_time;
	unsigned int  max_lease_time;
	unsigned char domain_name[DHCPD_NAME_LEN];	
	unsigned int  gateway_addr[DHCPD_GATEWAY_NUM];
	unsigned char bind_intf_name[DHCPD_NAME_LEN];
    unsigned int netbios_node_type;
    unsigned int forbidden_ip_count;
    unsigned int forbidden_ip_min[MAN_DHCP_POOL_FORBIDDEN_IP_MAX_COUNT];
    unsigned int forbidden_ip_max[MAN_DHCP_POOL_FORBIDDEN_IP_MAX_COUNT];
    unsigned int option43_count;
    unsigned int option43[DHCP_POOL_OPTION43_MAX_COUNT];
    unsigned int option_43_value_type;
    unsigned char buffer_43[128];
    unsigned int tot_lease_cnt;
    unsigned int valid_lease_cnt;
};

struct dcli_dhcpd_service_cfg
{
	unsigned char enDis;
	unsigned char domain_name[DHCPD_NAME_LEN];
	unsigned int  dns_addr[DHCPD_DNS_NUM];
	unsigned int  wins_addr[DHCPD_WINS_NUM];
	unsigned int  default_lease_time;
	unsigned int  max_lease_time;	
	unsigned int  gateway_addr[DHCPD_GATEWAY_NUM];
    unsigned int  ping_check;
};

struct dcli_dhcpd_intf
{
	unsigned char enDis;
	unsigned char state;
	unsigned char ifName[DHCPD_NAME_LEN];
	unsigned int  ifIndex;
	unsigned char bindpoolname[DHCPD_NAME_LEN];
    unsigned int have_more_server;
};

struct hardware {
	u_int8_t hlen;
	u_int8_t hbuf [17];
};

struct iaddr {
	unsigned len;
	unsigned char iabuf [16];
};

struct dcli_dhcpd_host
{
	struct hardware hardware;
	struct iaddr  hostip;
	unsigned char client_hostname[DHCPD_NAME_LEN];
	unsigned char flags;
	unsigned char uid[DHCPD_NAME_LEN];
};

struct dcli_dhcpd_lease
{
	unsigned int hostip;
	unsigned char hostMac[6];
	unsigned int start;
	unsigned int end;
	unsigned int type;
    unsigned int uid_len;
	unsigned char client_hostname[DHCPD_NAME_LEN];
    unsigned char client_id[DHCPD_NAME_LEN];
};

unsigned int dhcpd_get_curr_pool_info
(
	struct dcli_dhcpd_pool *poolshow,
	unsigned int *count
);

int dcli_dhcp_port2name(int portValue, char *portName);

#endif

