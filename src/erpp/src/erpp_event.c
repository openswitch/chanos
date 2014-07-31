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
#include <erpp/erpp_timer.h>
#include <erpp/erpp_log.h>


extern int ERPP_LOCAL_MODULE_ISMASTERACTIVE;
extern int ERPP_LOCAL_MODULE_ISMASTERSTANDBY;
extern array_table_index_t *erpp_global_array;
extern array_table_index_t *erpp_array_index;
extern pthread_mutex_t semErppMutex;

int erpp_dbtable_sync(char *buffer, int len, unsigned int sync_flag,int slot_index)
{
	npd_sync_msg_header_t *header = NULL;
    int op_ret = 0;
    if (ERPP_LOCAL_MODULE_ISMASTERACTIVE)
    {
        if (sync_flag & DB_SYNC_ALL)
        {
	        header = (npd_sync_msg_header_t*)buffer;
			
	        npd_dbtable_header_hton(header);	
	
            op_ret = tipc_client_sync_send(ERPP_SERVICE_MAGIC, -1, buffer, len);
        }
    }
    return 0;
}

int erpp_dbtable_init()
{
    return app_db_init(erpp_dbtable_sync, NULL, ERPP_LOCAL_MODULE_ISMASTERACTIVE);
}

int erpp_ring_id_get_from_netif(netif_index)
{
    int i = 0;
    struct erpp_domain_s entry;
    memset(&entry, 0, sizeof(struct erpp_domain_s));

	while(0 == erpp_domain_get_next(&entry))
	{
	    for(i = 0; i < ERPP_RING_SIZE; i++)
	    {
	        if ((netif_index == entry.ring[i].node.port[0].netif_index) || \ 
				(netif_index == entry.ring[i].node.port[1].netif_index))
	        {
				return i+1;
			} 
			else
			{
	            i++;
			}
	    }
	}
	return 0;
}

int erpp_app_event_linkup(netif_index)
{
    unsigned int ring_id = 0;
    struct erpp_domain_s entry;
    memset(&entry, 0, sizeof(struct erpp_domain_s));
	
	pthread_mutex_lock(&semErppMutex);
    while (0 == erpp_domain_get_next(&entry))
    {	    
        for(ring_id = 1; ring_id < ERPP_RING_SIZE; ring_id++)
	    {
	        if(entry.ring[ring_id-1].is_enable != 0)
	        {
                if ((netif_index == entry.ring[ring_id-1].node.port[0].netif_index) || (netif_index == entry.ring[ring_id-1].node.port[1].netif_index))
		        {
		            if ((BLOCK == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (BLOCK == entry.ring[ring_id-1].node.port[1].erpp_port_status))
		            {
		                if (netif_index == entry.ring[ring_id-1].node.port[0].netif_index)
		                {
							if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
		                        entry.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
							}
							else
							{
								entry.ring[ring_id-1].node.erpp_node_status = PRE_FORWARDING;
							}
		                }
		                else
		                {   
		                    if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
		                        entry.ring[ring_id-1].node.port[1].erpp_port_status = FORWARDING;
							}
		                    else
		                    {
							    entry.ring[ring_id-1].node.erpp_node_status = PRE_FORWARDING;
		                    }
		                }
		            }
		            else if ((FORWARDING == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (BLOCK == entry.ring[ring_id-1].node.port[1].erpp_port_status))
		            {					
		                if (netif_index == entry.ring[ring_id-1].node.port[1].netif_index)
		                {							
							if(entry.ring[ring_id-1].node.erpp_node_role != NODE_MASTER)
							{
		                        entry.ring[ring_id-1].node.erpp_node_status = PRE_FORWARDING;
							}
		                }
		                else
		                {
		                    erpp_syslog_dbg("Up port recevie linkup event.\n");
		                }
		            }
		            else if ((BLOCK == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (FORWARDING == entry.ring[ring_id-1].node.port[1].erpp_port_status))
		            {
		                if (netif_index == entry.ring[ring_id-1].node.port[0].netif_index)
		                {
		                    if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
		                    {
		                        entry.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
							    entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
		                    }
							else
							{
		                        entry.ring[ring_id-1].node.erpp_node_status = PRE_FORWARDING;
							}
		                }
		                else
		                {
		                    erpp_syslog_dbg("Uplink port recevie linkup event.\n");
		                }
		            }
		            else
		            {
						if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
						{
							entry.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
							entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
						}
						else
						{
							entry.ring[ring_id-1].node.erpp_node_status = PRE_FORWARDING;
							entry.ring[ring_id-1].node.port[0].erpp_port_status = BLOCK;
							entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
						}
		                erpp_syslog_dbg("all port linkup.\n");
		            }

		            if (0 != erpp_domain_update(&entry))
		            {
		                erpp_syslog_err("Execute erpp update faild in event func linkup.\n");
		            }
		        }
			}
		}
    }  
	
	pthread_mutex_unlock(&semErppMutex);
	return 0;
}

int erpp_app_event_linkdown(netif_index)
{
    unsigned int ring_id = 0;
    struct erpp_domain_s entry;
    memset(&entry, 0, sizeof(struct erpp_domain_s));
	
	pthread_mutex_lock(&semErppMutex);
	while(0 == erpp_domain_get_next(&entry))
	{   
	    for(ring_id = 1; ring_id < ERPP_RING_SIZE; ring_id++)
	    {
	        if(entry.ring[ring_id-1].is_enable != 0)
	        {
	            if ((netif_index == entry.ring[ring_id-1].node.port[0].netif_index) || (netif_index == entry.ring[ring_id-1].node.port[1].netif_index))
		        {
		            if ((FORWARDING == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (BLOCK == entry.ring[ring_id-1].node.port[1].erpp_port_status))
					{					
						if (netif_index == entry.ring[ring_id-1].node.port[0].netif_index)
						{
							entry.ring[ring_id-1].node.port[0].erpp_port_status = BLOCK;
							if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
								entry.ring[ring_id-1].node.port[1].erpp_port_status = FORWARDING;
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, COMMON_FLUSH_PACKET, entry.ring[ring_id-1].node.port[1].netif_index);
							}
							else
							{
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, LINK_DOWN, entry.ring[ring_id-1].node.port[1].netif_index);
							}
						}
						else
						{
							if(entry.ring[ring_id-1].node.erpp_node_role != NODE_MASTER)
							{
							    erpp_tx_type_packet(&entry, LINK_DOWN, entry.ring[ring_id-1].node.port[0].netif_index);
							}
						}
					}
					else if ((BLOCK == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (FORWARDING == entry.ring[ring_id-1].node.port[1].erpp_port_status))
					{
						if (netif_index == entry.ring[ring_id-1].node.port[1].netif_index)
						{
							if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
								entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, COMMON_FLUSH_PACKET, entry.ring[ring_id-1].node.port[0].netif_index);
							}
							else
							{   
								entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
								entry.ring[ring_id-1].node.erpp_node_status = LINK_DOWN;						
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, LINK_DOWN_PACKET, entry.ring[ring_id-1].node.port[0].netif_index);
							}
						}
						else
						{
							erpp_syslog_dbg("Uplink port recevie linkup event.\n");
						}
					}
					else if ((FORWARDING == entry.ring[ring_id-1].node.port[0].erpp_port_status) && (FORWARDING == entry.ring[ring_id-1].node.port[1].erpp_port_status))
					{
						if (netif_index == entry.ring[ring_id-1].node.port[0].netif_index)
						{
							if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
								entry.ring[ring_id-1].node.port[0].erpp_port_status = BLOCK;
								entry.ring[ring_id-1].node.port[1].erpp_port_status = FORWARDING;							
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, COMMON_FLUSH_PACKET, entry.ring[ring_id-1].node.port[1].netif_index);
							}
							else
							{
								entry.ring[ring_id-1].node.port[0].erpp_port_status = BLOCK;
								entry.ring[ring_id-1].node.erpp_node_status = LINK_DOWN;							
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, LINK_DOWN_PACKET, entry.ring[ring_id-1].node.port[1].netif_index);
							}
						}
						else
						{
							if(entry.ring[ring_id-1].node.erpp_node_role == NODE_MASTER)
							{
								entry.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
								entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;							
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, COMMON_FLUSH_PACKET, entry.ring[ring_id-1].node.port[0].netif_index);
							}
							else
							{
								entry.ring[ring_id-1].node.port[1].erpp_port_status = BLOCK;
								entry.ring[ring_id-1].node.erpp_node_status = LINK_DOWN;						
								entry.packet_ring = ring_id;
								erpp_tx_type_packet(&entry, LINK_DOWN_PACKET, entry.ring[ring_id-1].node.port[0].netif_index);
							}
						}
					}
					else
					{
						erpp_syslog_dbg("all port linkup.\n");
					}

					if (0 != erpp_domain_update(&entry))
					{
						erpp_syslog_err("Execute erpp update faild in event func linkup.\n");
					}
				} 
			}
		}
	}
	pthread_mutex_unlock(&semErppMutex);
	return 0;
}

int erpp_app_event_l2delete(netif_index)
{

}

int erpp_app_event_l2create(netif_index)
{

}

int erpp_app_event_delete(netif_index)
{

}
int erpp_netif_event_callback(unsigned int netif_index, int event)
{
	int ret = -1;
	struct erpp_global_configure_s global_configure;
	struct erpp_domain_s domain;
	
	memset(&global_configure, 0, sizeof(struct erpp_global_configure_s));
	
	ret = dbtable_array_get(erpp_global_array, 0, &global_configure);
	if(ret != 0)
	{
		erpp_syslog_err("ERRP: Netif event, can not find global configuration.\n");
		return -1;
	}
	
	if(global_configure.is_enable == 0)
	{
		erpp_syslog_err("ERRP: Netif event, service is not enabled.\n");
		return 0;
	}
	
	erpp_syslog_dbg("XXX: %s %d event %d\n", __func__, __LINE__, event);
	switch(event)
	{
		case PORT_NOTIFIER_LINKUP_E:
			erpp_app_event_linkup(netif_index);
			break;
		case PORT_NOTIFIER_LINKDOWN_E:
			erpp_app_event_linkdown(netif_index);
			break;
		case PORT_NOTIFIER_L2DELETE:			
			erpp_app_event_l2delete(netif_index);
			break;
		case PORT_NOTIFIER_L2CREATE:
			erpp_app_event_l2create(netif_index);
			break;
		case PORT_NOTIFIER_DELETE:
			erpp_app_event_delete(netif_index);
			break;
		default:
			break;
	}
	return ret;
}

void erpp_app_event_handler
(
	unsigned int netif_index,
	int event,
	char* buff,
	int len
)
{
    if(event == NOTIFIER_SWITCHOVER)
    {
        ERPP_LOCAL_MODULE_ISMASTERACTIVE = app_act_master_running();
        if(ERPP_LOCAL_MODULE_ISMASTERACTIVE)
        {
            erpp_syslog_dbg("standby becomes active \n");
            ERPP_LOCAL_MODULE_ISMASTERACTIVE = 1;
            ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 0;
        }
        else
        {
            erpp_syslog_dbg("active becomes standby \n");
            ERPP_LOCAL_MODULE_ISMASTERACTIVE = 0;
            ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 1;
        }
        erpp_dbtable_init();
    }
	else
	{
		erpp_netif_event_callback(netif_index, event);
	}

}

void erpp_app_relate_event_handler
(
	unsigned int netif_index,
	int event,
	char* buff,
	int len
)
{
	return ;
}

int erpp_app_event_init()
{

	int ret = -1;
    ret = netif_app_event_init(ERPP_SWITCH_OVER_INSTANCE);
	if(ret <= 0)
	{
		return -1;
	}
    ret = netif_app_event_op_register(erpp_app_event_handler, erpp_app_relate_event_handler);
	if(ret != 0)
	{
		return -1;
	}
    ret = osal_thread_create(NULL, osal_thread_master_run, 80, 0x4000, (void *)TIPC_APP_EVENT_SERVICE);
   	if(ret != 0){
		erpp_syslog_err("ERPP: erpp_event_init ---- osal_thread_create error\n");
		return -1;
	}
	erpp_syslog_dbg("ERPP: Create erpp event service.\n");
	return 0;

}


#endif

