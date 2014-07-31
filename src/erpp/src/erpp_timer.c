#ifdef HAVE_ERPP
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "util/npd_list.h"
#include "lib/db_app_sync.h"
#include "lib/netif_index.h"
#include "npd/npd_netif_event.h"
#include "lib/npd_bitop.h"
#include "quagga/thread.h"
#include "lib/npd_database.h"
#include "tipc_api/tipc_api.h"
#include "nam/nam_rx.h"

#include <erpp/erpp_main.h>
#include <erpp/erpp_dbus.h>
#include <erpp/erpp_event.h>
#include <erpp/erpp_packet.h>
#include <erpp/erpp_log.h>
#include <erpp/erpp_log.h>

extern array_table_index_t *erpp_global_array;
extern array_table_index_t *erpp_array_index;
pthread_mutex_t semErppMutex = PTHREAD_MUTEX_INITIALIZER;

int erpp_domain_timer_handler(struct erpp_domain_s* entry)
{   
    int i = 0;
    for(i = 1; i < ERPP_RING_SIZE+1; i++)
    {   
        if(entry->ring[i-1].is_enable == 0)
			continue;
		
	    if(entry->ring[i-1].node.erpp_node_role == NODE_MASTER)
	    {
	        if(entry->timer_count[0] == 0)
	        {
	            entry->timer_count[0] = entry->hello_timer;
				entry->packet_ring = i;				
	            erpp_tx_type_packet(entry, HELLO_PACKET, entry->ring[i-1].node.port[0].netif_index);
	        }
			if(entry->timer_count[1] == 0)
	        {
	            entry->timer_count[1] = entry->fail_timer;
				entry->ring[i-1].node.erpp_node_status = NODE_FAIL;			
				entry->flush_flag = 1;
				entry->ring[i-1].node.port[1].erpp_port_status = FORWARDING;				
				entry->packet_ring = i;				
				erpp_tx_type_packet(entry, COMMON_FLUSH_PACKET, entry->ring[i-1].node.port[0].netif_index);
	        }
			entry->timer_count[0]--;
	        entry->timer_count[1]--;
		}
		else if(entry->ring[i-1].node.erpp_node_role == NODE_EDGE)
		{
	        if(entry->timer_count[0] == 0)
	        {
	            entry->timer_count[0] = entry->hello_timer;				
				entry->packet_ring = i-1;				
	            erpp_tx_type_packet(entry, EDGE_HELLO_PACKET, entry->ring[0].node.port[0].netif_index);				
	            erpp_tx_type_packet(entry, EDGE_HELLO_PACKET, entry->ring[0].node.port[1].netif_index);
	        }
	        entry->timer_count[0]--;
		}
		else if(entry->ring[i-1].node.erpp_node_role == NODE_ASSISTANT_EDGE)
		{	
		    if(entry->timer_count[2] == 0)
	        {
				entry->ring[i-1].node.erpp_node_status = ERPP_LINK_FAIL;	
				entry->packet_ring = i;				
				erpp_tx_type_packet(entry, FAULT_PACKET, entry->ring[i-1].node.port[0].netif_index);	
		    }
	        entry->timer_count[2]--;
		}
		else
			continue;

    }
	if (0 != erpp_domain_update(entry))
		 return 1;
    return 0;
}

int erpp_timer_handler(void *arg)
{
	int ret = -1;
 	struct erpp_global_configure_s global_configure;
	struct erpp_domain_s    erpp_domain;
	
	memset(&global_configure, 0, sizeof(struct erpp_global_configure_s));
	memset(&erpp_domain, 0, sizeof(struct erpp_domain_s));
	
	ret = dbtable_array_get(erpp_global_array, 0, &global_configure);
	if(ret != 0)
	{
		return 1;
	}
	
	if(global_configure.is_enable == 0)
	{
		return 1;
	}
	
	pthread_mutex_lock(&semErppMutex);
	do
	{
		ret = erpp_domain_get_next(&erpp_domain);
		if(ret == 0)
		{
	        erpp_domain_timer_handler(&erpp_domain);			
		}
	}while(ret == 0);
	pthread_mutex_unlock(&semErppMutex);
	
	return 0;   
}

int erpp_timer_init()
{
	return osal_register_timer(1, ERPP_SERVICE_MAGIC, erpp_timer_handler, NULL, 1);
}


#endif

