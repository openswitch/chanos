
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_app_event.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		APIs used in npd to broadcast events to any other applications.
*
* DATE:
*		09/15/2010	
*UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "util/npd_list.h"
#include "npd_bitop.h"
#include "npd_database.h"
#include "npd/npd_netif_event.h"
#include "tipc_api/tipc_api.h"


netif_app_event_op netif_app_event_ops = NULL;
netif_app_relate_event_op netif_app_relate_event_ops = NULL;

int netif_app_event_ntoh(netif_remote_event_hdr *app_event)
{
	if(app_event == NULL)
		return 0;

	app_event->nlh.nlmsg_len = ntohl(app_event->nlh.nlmsg_len);
	app_event->nlh.nlmsg_type = ntohs(app_event->nlh.nlmsg_type);
	app_event->event_code = ntohl(app_event->event_code);
	app_event->netif_index = ntohl(app_event->netif_index);
	app_event->relate_netif_index = ntohl(app_event->relate_netif_index);
		
	return 0;
}


int netif_app_event_hton(netif_remote_event_hdr *app_event)
{
	if(app_event == NULL)
		return 0;
	
	app_event->nlh.nlmsg_len = htonl(app_event->nlh.nlmsg_len);
	app_event->nlh.nlmsg_type = htons(app_event->nlh.nlmsg_type);
	app_event->event_code = htonl(app_event->event_code);
	app_event->netif_index = htonl(app_event->netif_index);
	app_event->relate_netif_index = htonl(app_event->relate_netif_index);
		
	return 0;
}

int netif_app_event_op_register(netif_app_event_op event_ops, netif_app_relate_event_op relate_event_ops)
{
	if(event_ops)
	{
		netif_app_event_ops = event_ops;
	}
	if(relate_event_ops)
	{
		netif_app_relate_event_ops = relate_event_ops;
	}
	return 0;
}

int netif_app_event_op_unregister()
{
	netif_app_event_ops = NULL;
	netif_app_relate_event_ops = NULL;
	return 0;
}

int netif_app_event_handler(int sock, char *buff, int len, void *private_data)
{
	netif_remote_event_hdr *app_event = NULL;
	char *temp_buff = NULL;
	int extra_data_len = 0;
	int temp_len = len;
	if(len <= 0)
	{
		return -1;
	}

	app_event = (netif_remote_event_hdr *)buff;
	netif_app_event_ntoh(app_event);
	while(temp_len >= app_event->nlh.nlmsg_len)
	{
		temp_buff = NULL;
		extra_data_len = 0;
		if(app_event->nlh.nlmsg_len > sizeof(netif_remote_event_hdr))
		{
			temp_buff = (char *)(app_event + 1);
			extra_data_len = app_event->nlh.nlmsg_len - sizeof(netif_remote_event_hdr);
		}
		if(app_event->relate_netif_index == 0)
		{
			if(app_event->event_code >= PORT_NOTIFIER_TYPE_MAX)
			{
				return -1;
			}
			if(netif_app_event_ops)
			{
			    (*netif_app_event_ops)(app_event->netif_index, app_event->event_code, 
                    temp_buff, extra_data_len);
			}
		}
		else
		{
			if(app_event->event_code > PORT_NOTIFIER_RELATE_MAX)
			{
				return -1;
			}
			if(netif_app_relate_event_ops)
			{
			    (*netif_app_relate_event_ops)(app_event->netif_index, app_event->relate_netif_index, 
                      app_event->event_code, temp_buff, extra_data_len);
			}
		}
		/*to add app event handler here*/
		temp_len -= app_event->nlh.nlmsg_len;
		app_event = (netif_remote_event_hdr *)(buff + (len - temp_len));
		if(temp_len < sizeof(netif_remote_event_hdr))
		{
			break;
		}
	}
	return 0;
}

int netif_app_event_handle(int sock, netif_app_event_op event_ops, netif_app_relate_event_op relate_event_ops)
{
	netif_remote_event_hdr *app_event = NULL;
	char buff[2048];
    char *temp_buff = NULL;
	int extra_data_len = 0;
	int temp_len = 0;
    int len = 0;

    len = recvfrom(sock, buff, sizeof(buff), 0, NULL, 0); 
	if(len <= 0)
	{
		return -1;
	}
    temp_len = len;

	app_event = (netif_remote_event_hdr *)buff;
	netif_app_event_ntoh(app_event);
	while(temp_len >= app_event->nlh.nlmsg_len)
	{
		temp_buff = NULL;
		extra_data_len = 0;
		if(app_event->nlh.nlmsg_len > sizeof(netif_remote_event_hdr))
		{
			temp_buff = (char *)(app_event + 1);
			extra_data_len = app_event->nlh.nlmsg_len - sizeof(netif_remote_event_hdr);
		}
		if(app_event->relate_netif_index == 0)
		{
			if(app_event->event_code >= PORT_NOTIFIER_TYPE_MAX)
			{
				return -1;
			}
			if(event_ops)
			{
			    (*event_ops)(app_event->netif_index, app_event->event_code, temp_buff, extra_data_len);
			}
		}
		else
		{
			if(app_event->event_code > PORT_NOTIFIER_RELATE_MAX)
			{
				return -1;
			}
			if(relate_event_ops)
			{
			    (*relate_event_ops)(app_event->netif_index, app_event->relate_netif_index, app_event->event_code, 
                    temp_buff, extra_data_len);
			}
		}
		/*to add app event handler here*/
		temp_len -= app_event->nlh.nlmsg_len;
		app_event = (netif_remote_event_hdr *)(buff + (len - temp_len));
		if(temp_len < sizeof(netif_remote_event_hdr))
		{
			break;
		}
	}
	return 0;
}

/*USE tipc as socket for its broadcast supporting*/
int netif_app_event_init(int app_instance)
{
    int sd = socket (AF_TIPC, SOCK_RDM,0);
    struct sockaddr_tipc server_addr = {0};
    socklen_t sz = sizeof(struct sockaddr_tipc);
    int local_slot = 0;
	if(sd <= 0)
	{
		return -1;
	}
	
    if (0 > getsockname(sd,(struct sockaddr*)&server_addr,&sz))
	{
        printf("Failed to get sock address\r\n");
		close(sd);
	    return 0;
    }
    local_slot = (server_addr.addr.id.node&0xFFFFF000);
	memset(&server_addr, sz, 0);
	
    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_NAMESEQ;
    server_addr.scope = TIPC_NODE_SCOPE;/*暂时只在本板传递EVENT*/
    server_addr.addr.nameseq.type = TIPC_APP_EVENT_SERVICE;
    server_addr.addr.nameseq.lower = local_slot + app_instance;
    server_addr.addr.nameseq.upper = local_slot + app_instance;
    
    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
        printf("Port {%u,%u,%u} could not be created\n",
        server_addr.addr.nameseq.type,
        server_addr.addr.nameseq.lower,
        server_addr.addr.nameseq.upper);
    }
	osal_register_read_fd(sd, TIPC_APP_EVENT_SERVICE, netif_app_event_handler, NULL, 1);
    return sd;
    
}

int netif_app_event_send_broadcast(char *buff, int len)
{
    struct sockaddr_tipc server_addr = {0};
    socklen_t sz = sizeof(struct sockaddr_tipc);
    int sd = socket (AF_TIPC, SOCK_RDM,0);
    int local_slot = 0;
	int ret = 0;
	if(sd <= 0)
	{
		return -1;
	}
	ret = getsockname(sd,(struct sockaddr*)&server_addr,&sz);
    if (0 > ret)
	{
        printf("Netif App Event:Failed to get sock address. Return code: %d\r\n", ret);
		close(sd);
	    return -1;
    }
    local_slot = (server_addr.addr.id.node&0xFFFFF000);
	memset(&server_addr, sz, 0);
	
    server_addr.family = AF_TIPC;
    server_addr.addrtype = TIPC_ADDR_MCAST;
    server_addr.scope = TIPC_NODE_SCOPE;/*暂时只在本板传递EVENT*/
    server_addr.addr.nameseq.type = TIPC_APP_EVENT_SERVICE;
    server_addr.addr.nameseq.lower = local_slot + 1;
    server_addr.addr.nameseq.upper = local_slot + 255;/*最多支持255个应用*/
    
    if (0 > sendto(sd, buff, len, 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        printf("Client: Failed to send\r\n");
    	close(sd);
    	return -1;
    }
	close(sd);
	return 0;
}

int netif_app_notify_relate_event(unsigned int father_ifindex, unsigned int son_ifindex, enum PORT_RELATE_ENT event, void *private_data, int len)
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
	app_event->netif_index = father_ifindex;
	app_event->relate_netif_index = son_ifindex;
	tmp_buf = (char *)(app_event + 1);
	netif_app_event_hton(app_event);
		
	if(private_data != NULL && len > 0)
	{
		memcpy(tmp_buf, private_data, len);
	}
	netif_app_event_send_broadcast((char *)app_event, msg_len);
	free(app_event);
	return 0;
}
int netif_app_notify_event(unsigned int netif_index, enum PORT_NOTIFIER_ENT event, void *private_data, int len)
{
	return netif_app_notify_relate_event(netif_index, 0, (enum PORT_RELATE_ENT)event, private_data, len);
}

#ifdef __cplusplus
}
#endif
