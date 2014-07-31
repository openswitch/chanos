#ifndef __COMMON_SECURITY_H__
#define __COMMON_SECURITY_H__

/*IP SOURCE GUARD*/
#define NPD_SG_ENTRY_NUM     256        /*same as SG_MAX_DYNAMIC_INDEX_NUM*/
#define NPD_SG_DYNAMIC_ENTRY_OFFSET  256    /*same as SG_MAX_STATIC_INDEX_NUM*/

#define SOURCE_GUARD_STATIC 0x1
#define SOURCE_GUARD_DYNAMIC 0x2

struct ip_source_guard_entry_s
{
    unsigned int ifIndex;
    unsigned int ipAddr;
    unsigned char mac[6];
    unsigned short  vid;
    unsigned int sg_index;
    int is_static;
};

#define SFLOW_ENABLE  1
#define SFLOW_DISABLE 0

#define SFLOW_SAMPLE_MODE_MIN	0
#define SFLOW_SAMPLE_MODE_MAX   1

#define AGENT_NUM 		1
#define COLLECTOR_NUM 	3
#define SFLOW_DEFAULT_SAMPLING_RATE 100000


typedef struct sflow_agt_db_s {
	unsigned int agt_index;
	unsigned long agt_ip;
}sflow_agt_db_t;

typedef struct sflow_clt_db_s {
	unsigned int clt_index; 
	unsigned long clt_ip;
	unsigned int port;
}sflow_clt_db_t;

typedef struct sflow_intf_db_s {
	unsigned int netif_index; 
	unsigned int is_enable;
	unsigned int sampling_rate;

}sflow_intf_db_t;



#endif
