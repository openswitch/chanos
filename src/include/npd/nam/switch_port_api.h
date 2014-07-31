#ifndef __NPD_SWITCH_PORT_API_H__
#define __NPD_SWITCH_PORT_API_H__





void npd_init_switch_ports(void);
void npd_delete_switch_port
(
    unsigned int index
) ;

void npd_create_switch_port
(
	unsigned int global_index,
	char *name,
	unsigned int *index,
	int link_state
	
) ;

unsigned int npd_switch_port_netif_index(
    unsigned int switch_port_index,
    unsigned int *netif_index
    );
int npd_check_netif_status(
    unsigned int netif_index,
    int *link_status
    );
int npd_netif_speed(
    unsigned int netif_index,
    int *speed);

int npd_netif_duplex_mode(
    unsigned int netif_index,
    int *duplex_mode
    );
unsigned int npd_switch_port_link_state_get(
    unsigned int switch_port_index,
    int *link_state
    );

unsigned int npd_switch_port_get_pvid
(
	unsigned int   switch_index,
	unsigned short *vlanId
);
unsigned int npd_switch_port_set_pvid
(
	unsigned int   switch_index,
	unsigned short vlanId
);

int npd_switch_port_show_running(
    unsigned int switch_port_index,
    char *showStr,
    int size
    );

int npd_save_switch_port_igmp_snp_cfg
(
	void *data, 
	char* buf,
	int *bufLen
);

port_driver_t * port_driver_get
(
	unsigned int netif_index
);

unsigned int npd_switch_port_get_vlan_mode
(
    unsigned int netif_index, int *mode
);

extern port_driver_t * port_driver_get(
	unsigned int netif_index
	);

extern int npd_check_netif_switch_mode
(
    unsigned int netif_index
);

extern unsigned int npd_fdb_learning_mode_set
(
    unsigned int eth_g_index,
    unsigned int mode
);
extern unsigned int npd_fdb_learning_mode_get
(
    unsigned int eth_g_index,
    unsigned int *mode
);
struct switch_port_db_s * npd_get_switch_port_by_index
(
	unsigned int eth_g_index
);

unsigned int npd_module_learning_mode_get
(
    unsigned int netif_index,
    unsigned int learn_module,
    unsigned int *mode
);

#endif
