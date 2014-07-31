/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_netif_event.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD by ETHERNET PORT module.
*
* DATE:
*		07/13/2010	
*UPDATE:
*08/13/2010              zhengzw@autelan.com          Remote event notify.
*  FILE REVISION NUMBER:
*  		$Revision: 1.256 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

extern int npd_eth_port_notifier
(
    unsigned int	eth_g_index,
    enum ETH_PORT_NOTIFIER_ENT	event
);

extern int npd_fdb_learning(struct fdb_entry_item_s * entry, int isAdd);

pthread_mutex_t port_notify_mutex;
enum
{
    NETIF_EVENT_TYPE,
    NETIF_RELATE_EVENT_TYPE
};

typedef struct netif_event_s
{
    struct list_head list;
    int event_type;
    enum PORT_NOTIFIER_ENT event;
    unsigned int netif_index;
    unsigned int son_index;
}netif_event_t;
void (*netif_notify_remote_event_callback)(void *private_data, int private_data_len) = NULL;
#ifdef HAVE_CHASSIS_SUPPORT
int netif_remote_notify_relate_event(int dest_slot, unsigned int netif_index, unsigned int son_netif, enum PORT_RELATE_ENT event, void *private_data, int len)
{
	char *tmp_buf = NULL;
	netif_remote_event_hdr *app_event = NULL;
	int msg_len = 0;
	if(private_data == NULL || len <= 0)
	{
		msg_len = sizeof(netif_remote_event_hdr);
	}
	else
	{
		msg_len = sizeof(netif_remote_event_hdr) + len;
	}
	app_event = malloc(msg_len);
	if(app_event == NULL)
	{
		return -1;
	}
	app_event->nlh.nlmsg_len = msg_len;
	app_event->nlh.nlmsg_type = TIPC_APP_EVENT_SERVICE;
	app_event->event_code = event;
	app_event->netif_index = netif_index;
	app_event->relate_netif_index = son_netif;
	tmp_buf = (char *)(app_event + 1);
	netif_app_event_hton(app_event);
	
	if(private_data != NULL && len > 0)
	{
		memcpy(tmp_buf, private_data, len);
	}
	tipc_client_sync_send(PORT_EVENT_NOTIFIER, dest_slot + 1, (char *)app_event, msg_len);
	free(app_event);
	return 0;
}

int netif_remote_notify_event(int dest_slot, unsigned int netif_index, enum PORT_NOTIFIER_ENT event, void *private_data, int len)
{
	return netif_remote_notify_relate_event(dest_slot, netif_index, 0, (enum PORT_RELATE_ENT)event, private_data, len);
}

extern unsigned int npd_mroute_hit(void* param);

int netif_remote_notify_event_handler(int sock, char *buff, int len, void *private_data)
{
	netif_remote_event_hdr *app_event = NULL;
	char *temp_buff = NULL;
	int extra_data_len = 0;
	int temp_len = len;
	static int first_run = 1;
	int ret = 0;
    int nRecvBuf;
	if(len <= 0)
	{
		return -1;
	}
	app_event = (netif_remote_event_hdr *)buff;
	netif_app_event_ntoh(app_event);
	
	while(temp_len >= app_event->nlh.nlmsg_len)
	{
		npd_syslog_dbg("Receive an event message: netif index = 0x%x, relate netif index = 0x%x, event = %d.\r\n", app_event->netif_index, app_event->relate_netif_index, app_event->event_code);
		temp_buff = NULL;
		extra_data_len = 0;
		if(app_event->nlh.nlmsg_len > sizeof(netif_remote_event_hdr))
		{
			temp_buff = (char *)(app_event + 1);
			extra_data_len = app_event->nlh.nlmsg_len - sizeof(netif_remote_event_hdr);
			/*如果是ethernet port link up事件,需要将带来的参数存入主控数据库*/
			if(extra_data_len)
			{
                struct fdb_entry_item_s entry = {0};
                nam_l2_addr_t *l2addr = (nam_l2_addr_t*)temp_buff;

				memset(&entry, 0, sizeof(struct fdb_entry_item_s));
				
				npd_syslog_dbg("Event with ext info. code %d\r\n", app_event->event_code);
                switch(app_event->event_code)
                {
#ifdef HAVE_FDB_SW_SYNC					
                    case PORT_NOTIFIER_L2ADDR_ADD:
                        entry.vlanid = l2addr->vid;
                        memcpy(entry.mac, l2addr->mac, 6);
                        entry.ifIndex = l2addr->if_index;
                        npd_fdb_learning(&entry, TRUE);
                        break;
                    case PORT_NOTIFIER_L2ADDR_DEL:
                        entry.vlanid = l2addr->vid;
                        memcpy(entry.mac, l2addr->mac, 6);
                        entry.ifIndex = l2addr->if_index;
                        npd_fdb_learning(&entry, FALSE);
                        break;
#endif
#ifdef HAVE_PIM
                    case PORT_NOTIFIER_IPMCHIT:
                        npd_mroute_hit(temp_buff);
                        break;
#endif						
                    default:
        				if(netif_notify_remote_event_callback)
        				{
        				    (*netif_notify_remote_event_callback)(temp_buff, extra_data_len);
        				}
                        break;
                }
			}
		}
		else
		{
			break;
		}
		if(app_event->event_code == PORT_NOTIFIER_L2ADDR_ADD || app_event->event_code == PORT_NOTIFIER_L2ADDR_DEL 
		          || app_event->event_code == PORT_NOTIFIER_IPMCHIT || app_event->event_code == PORT_NOTIFIER_ETHPORT_STAT)
		{
			
		}
		else
		{
    		if(app_event->relate_netif_index == 0)
    		{
    			npd_eth_port_notifier(app_event->netif_index, app_event->event_code);
    		}
    		else
    		{
				/*此处在非主用主控不会被调用到,所以可以不加锁*/
    			netif_notify_relate_event(app_event->netif_index, app_event->relate_netif_index, app_event->event_code);
    		}
		}
		/*to add app event handler here*/
		temp_len -= app_event->nlh.nlmsg_len;
		app_event = (netif_remote_event_hdr *)(buff + (len - temp_len));
		if(temp_len < sizeof(netif_remote_event_hdr))
		{
			break;
		}
		netif_app_event_ntoh(app_event);
	}
	if(first_run)
	{
		first_run = 0;
        nRecvBuf = 256 * NPD_FDB_TABLE_SIZE;
        ret = setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
        if (0 != ret)
            npd_syslog_dbg("in func recv_msgbuf_to_drv setsockopt failed, retval:%d\n", ret);

	}

	return 0;
}

int netif_event_service_monitor_handler(int event, int service_type, int instance)
{
	npd_syslog_dbg("Netif event service at slot %d ", INSTANCE_TO_SLOT(instance));
	switch(event)
	{
		case 1:
			npd_syslog_dbg("published.\r\n");
			break;
		case 2:
			npd_syslog_dbg("withdrawn.\r\n");
			break;
		default:
			npd_syslog_dbg("unknown.\r\n");
			break;
		
	}
	return 0;
}

int netif_remote_event_init(int self_slot)
{
	int ret = -1;
	ret = tipc_client_init(PORT_EVENT_NOTIFIER, netif_event_service_monitor_handler);
	if(ret != TIPC_SUCCESS)
	{
		return ret;
	}
	if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
	{
	    ret = tipc_server_init(PORT_EVENT_NOTIFIER, self_slot, netif_remote_notify_event_handler, NULL, NULL);
	    osal_thread_read_buffer_length_set(PORT_EVENT_NOTIFIER, 4000);
	}
	nam_thread_create("remote_event", (unsigned int (*)(void*))osal_thread_master_run, (void *)PORT_EVENT_NOTIFIER, TRUE, FALSE);
	return ret;
}
#endif

int netif_event_init()
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&port_notify_mutex, &attr);
#ifdef HAVE_CHASSIS_SUPPORT	
	netif_remote_event_init(SYS_LOCAL_MODULE_SLOT_INDEX + 1);
#endif
    return 0;
}

struct list_head event_list_head = 
{
    &event_list_head, &event_list_head
};

struct list_head netif_notifier_head =
{
    &netif_notifier_head, &netif_notifier_head
};

void register_netif_notifier(netif_event_notifier_t *notifier)
{
    list_add_tail(&notifier->list, &netif_notifier_head);
    return;
}

void unregister_netif_notifier(netif_event_notifier_t *notifier)
{
    list_del(&notifier->list);
    return;
}

void netif_notify_event_out(unsigned int netif_index, enum PORT_NOTIFIER_ENT event, char *private, int len)
{
	struct list_head *pos = NULL;
	struct list_head *pos_event = NULL;
	struct netif_event_s *store_event = malloc(sizeof(struct netif_event_s));

	npd_syslog_dbg("Notify NETIF event, ifindex 0x%x, event %d\n", 
		netif_index, event);

	memset(store_event, 0, sizeof(struct netif_event_s));
	store_event->event_type = NETIF_EVENT_TYPE;
	store_event->event = event;
	store_event->netif_index = netif_index;

	pthread_mutex_lock(&port_notify_mutex);

	if((event != PORT_NOTIFIER_DELETE)
		&& (event != PORT_NOTIFIER_L2DELETE)
		)
	{
		list_add_tail( &store_event->list, &event_list_head);
		if(event_list_head.next != &store_event->list)
			goto retcode;
		
		pos_event = event_list_head.next;
		while(pos_event != &event_list_head)
		{
			netif_event_t *handle_event
				 = list_entry(pos_event, netif_event_t, list);
			if(handle_event->event_type == NETIF_EVENT_TYPE)
			{
				/*here should be careful for the list sequence, should from phy layer to layer 7*/
				list_for_each(pos, &netif_notifier_head)
				{
					netif_event_notifier_t *notifier 
						 = list_entry(pos, netif_event_notifier_t, list);
					if(*notifier->netif_event_handle_f != NULL)
						(*notifier->netif_event_handle_f)(handle_event->netif_index, 
												  handle_event->event, private, len);

				}
			}
			else if(handle_event->event_type == NETIF_RELATE_EVENT_TYPE)
			{
				/*here should be careful for the list sequence, should from phy layer to layer 7*/
				list_for_each(pos, &netif_notifier_head)
				{
					netif_event_notifier_t *notifier 
						 = list_entry(pos, netif_event_notifier_t, list);
					if(*notifier->netif_relate_handle_f != NULL)
						(*notifier->netif_relate_handle_f)(handle_event->netif_index, 
											   handle_event->son_index, handle_event->event,private, len);
				}
			}
			list_del(pos_event);
			free(handle_event);
			pos_event = event_list_head.next;
		}
	}
	else
	{
		/*here should be careful for the list sequence, should from phy layer to layer 7*/
		list_for_each_prev(pos, &netif_notifier_head)
		{
			netif_event_notifier_t *notifier 
				 = list_entry(pos, netif_event_notifier_t, list);
			if(NULL != notifier->netif_event_handle_f)
				(*notifier->netif_event_handle_f)(netif_index, event,private, len);
		}
		free(store_event);
	}
retcode:	
	pthread_mutex_unlock(&port_notify_mutex);
	
}


void netif_notify_event(unsigned int netif_index, enum PORT_NOTIFIER_ENT event)
{
	netif_notify_event_out(netif_index, event, NULL, 0);    
}


void netif_notify_relate_event_out(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event, char *private, int len)
{
	struct list_head *pos;
	struct list_head *pos_event = NULL;
	struct netif_event_s *store_event = malloc(sizeof(struct netif_event_s));

	npd_syslog_dbg("Notify relate event, father ifindex 0x%x, son ifindex 0x%x, event %d\n", 
		father_ifindex, son_ifindex, event);

	memset(store_event, 0, sizeof(struct netif_event_s));
	store_event->event_type = NETIF_RELATE_EVENT_TYPE;
	store_event->event = event;
	store_event->netif_index = father_ifindex;
	store_event->son_index = son_ifindex;

	/*here should be careful for the list sequence, should from phy layer to layer 7*/
	pthread_mutex_lock(&port_notify_mutex);
	if(event != PORT_NOTIFIER_LEAVE)
	{
		list_add_tail( &store_event->list, &event_list_head);
		if(event_list_head.next != &store_event->list)
			goto retcode;
		
		pos_event = event_list_head.next;
		while(pos_event != &event_list_head)
		{
			netif_event_t *handle_event
				 = list_entry(pos_event, netif_event_t, list);
			if(handle_event->event_type == NETIF_EVENT_TYPE)
			{
				/*here should be careful for the list sequence, should from phy layer to layer 7*/
				list_for_each(pos, &netif_notifier_head)
				{
					netif_event_notifier_t *notifier 
						 = list_entry(pos, netif_event_notifier_t, list);
					if(*notifier->netif_event_handle_f != NULL)
						(*notifier->netif_event_handle_f)(handle_event->netif_index, 
												  handle_event->event,private,len);

				}
			}
			else if(handle_event->event_type == NETIF_RELATE_EVENT_TYPE)
			{
				/*here should be careful for the list sequence, should from phy layer to layer 7*/
				list_for_each(pos, &netif_notifier_head)
				{
					netif_event_notifier_t *notifier 
						 = list_entry(pos, netif_event_notifier_t, list);
					if(*notifier->netif_relate_handle_f != NULL)
						(*notifier->netif_relate_handle_f)(handle_event->netif_index, 
											   handle_event->son_index, handle_event->event,private,len);
				}
			}
			list_del(pos_event);
			free(handle_event);
			pos_event = event_list_head.next;
		}
	}
	else
	{
		list_for_each_prev(pos, &netif_notifier_head)
		{
			netif_event_notifier_t * notifier 
				 = list_entry(pos, netif_event_notifier_t, list);
			if(NULL != notifier->netif_relate_handle_f)
				(*notifier->netif_relate_handle_f)(father_ifindex, son_ifindex, event, private,len);
		}
		free(store_event);
	}
retcode:	
	pthread_mutex_unlock(&port_notify_mutex);
	
}

void netif_notify_relate_event(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event)
{
    netif_notify_relate_event_out(father_ifindex, son_ifindex, event, NULL, 0);
}

#ifdef __cplusplus
}
#endif

