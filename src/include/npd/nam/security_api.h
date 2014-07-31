#ifndef __NPD_SECURITY_API_H__
#define __NPD_SECURITY_API_H__

/*NPD LAYER API*/
int npd_source_guard_entry_del(unsigned int ipno, unsigned int eth_g_index, int sg_type);
int npd_source_guard_entry_add
(
    unsigned int ipno, 
    unsigned int eth_g_index, 
    unsigned short vid,
    char* mac,
    int sg_type
);

int npd_ip_source_guard_init();
int npd_sflow_init(void);

/*NAM LAYER API*/

int nam_source_guard_add(struct ip_source_guard_entry_s *item);

int nam_source_guard_delete(struct ip_source_guard_entry_s *item);

int nam_sflow_init(void);
int nam_sflow_port_dir_enable_set(unsigned int netif_index, int is_enable);
int nam_sflow_port_dir_ratio_set(unsigned int netif_index, int ratio);

int nam_sflow_port_dir_enable_get(unsigned int netif_index, int *is_enable);
int nam_sflow_port_dir_ratio_get(unsigned int netif_index, int *ratio);


typedef struct sflow_driver_s
{
	int (*sflow_port_dir_enable_set)(unsigned int netif_index, int is_enable);
	int (*sflow_port_dir_ratio_set)(unsigned int netif_index, int ratio);
	int (*sflow_port_dir_mode_set)(unsigned int netif_index, int mode);
	int (*sflow_port_dir_enable_get)(unsigned int netif_index, int *is_enable);
	int (*sflow_port_dir_ratio_get)(unsigned int netif_index, int *ratio);
	int (*sflow_port_dir_mode_get)(unsigned int netif_index, int *mode);
	
}sflow_driver_t;

#endif
