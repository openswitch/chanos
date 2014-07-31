#ifndef __NETIF_ARRAY_PORT_H__
#define __NETIF_ARRAY_PORT_H__


#include "lib/netif_index.h"

typedef struct array_port_entry_s
{
    unsigned int array_index;
    unsigned int netif_index;
}array_port_entry_t;

#define MAX_ARRAY_PORT MAX_SWITCHPORT_PER_SYSTEM

void init_array_ports(
    int sync_flag
    );

void create_array_port
(
	unsigned int netif_index
) ;
void delete_array_port
(
    unsigned int index
);

unsigned int array_port_array_index(
    unsigned int netif_index,
    unsigned int *array_index
    );

unsigned int array_port_netif_index(
    unsigned int array_index,
    unsigned int *netif_index
    );

#endif
