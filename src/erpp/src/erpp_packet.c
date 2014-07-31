#ifdef HAVE_ERPP

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "npd/npd_list.h"
#include "lib/netif_index.h"
#include "lib/npd_database.h"
#include "lib/db_usr_api.h"
#include "nam/nam_rx.h"
#include "lib/chassis_man_app.h"
#include "npd/npd_netif_event.h"
#include "tipc_api/tipc_api.h"

#include <erpp/erpp_main.h>
#include <erpp/erpp_dbus.h>
#include <erpp/erpp_event.h>
#include <erpp/erpp_packet.h>
#include <erpp/erpp_timer.h>
#include <erpp/erpp_log.h>

extern char SysMac[6];
extern pthread_mutex_t semErppMutex;

int erpp_msg_sock = -1;       /* XXX: send msg to npd */
int erpp_tx_packet_sock = -1;          /* XXX: send packet for tx */
struct sockaddr_un erpp_tx_packet_client;
struct sockaddr_un erpp_tx_packet_server;
struct sockaddr_un erpp_msg_client;
struct sockaddr_un erpp_msg_server;
void erpp_send_packet(char* data, unsigned int length);
void erpp_tx_type_packet(struct erpp_domain_s* entry, unsigned int packet_type, unsigned int netif_index);
int erpp_packet_hello_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{
    unsigned short temp_ring_id = ntohs(erpp_buff->ring_id);
    union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)buff;
	if(0 != memcmp(erpp_buff->sys_mac, SysMac, sizeof(SysMac)))
	{		
	    if(entry->ring[temp_ring_id-1].node.port[0].netif_index == rxPacket->sync_ctrl.netif_index)
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[1].netif_index; 
		}
		else
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[0].netif_index; 
		}		
	    erpp_send_packet(buff, ERPP_PACKET_LENGTH);
	}
	else if(entry->ring[temp_ring_id-1].node.erpp_node_status == NODE_FAIL)
	{
		entry->ring[temp_ring_id-1].node.erpp_node_status = NODE_COMPLETE;
		entry->flush_flag = 1;
		if(entry->ring[temp_ring_id-1].node.port[0].erpp_port_status == FORWARDING)
		{
			entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = BLOCK;
		}
		entry->packet_ring = temp_ring_id;		

		if (0 != erpp_domain_update(entry))
			return 1;
		
		erpp_tx_type_packet(entry, COMPLETE_FLUSH_PACKET, entry->ring[temp_ring_id-1].node.port[0].netif_index);
		erpp_tx_type_packet(entry, COMPLETE_FLUSH_PACKET, entry->ring[temp_ring_id-1].node.port[1].netif_index);
	}
	else
	{
		entry->timer_count[1] = entry->fail_timer;
		if(entry->ring[temp_ring_id-1].node.port[0].erpp_port_status == FORWARDING)
		{
			entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = BLOCK;
		}
		
		if (0 != erpp_domain_update(entry))
			return 1;
	}

	return 0;
}

int erpp_packet_complete_flush_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{
    unsigned short temp_ring_id = ntohs(erpp_buff->ring_id);
	union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)buff;

	if(0 != memcmp(erpp_buff->sys_mac, SysMac, sizeof(SysMac)))
	{	
	    if(entry->ring[temp_ring_id-1].node.erpp_node_status == PRE_FORWARDING)
	    {
			entry->ring[temp_ring_id-1].node.erpp_node_status = LINK_UP;
			if(entry->ring[temp_ring_id-1].node.port[0].erpp_port_status == BLOCK \
				|| entry->ring[temp_ring_id-1].node.port[1].erpp_port_status == BLOCK) 
		    {
		        entry->ring[temp_ring_id-1].node.port[0].erpp_port_status = FORWARDING;
			    entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = FORWARDING;
		    }
		}
		if(entry->ring[temp_ring_id-1].node.port[0].netif_index == rxPacket->sync_ctrl.netif_index)
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[1].netif_index; 
		}
		else
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[0].netif_index; 
		}	
		
	    erpp_send_packet(buff, ERPP_PACKET_LENGTH);
	}

	entry->flush_flag = 1;
	if (0 != erpp_domain_update(entry))
		return 1;
	return 0;
}

int erpp_packet_common_flush_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{
	entry->flush_flag = 1;
	if (0 != erpp_domain_update(entry))
		return 1;
	return 0;
}

int erpp_packet_link_down_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{	
    unsigned short temp_ring_id = ntohs(erpp_buff->ring_id);
    union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)buff;
    if(entry->ring[temp_ring_id-1].node.erpp_node_role != NODE_MASTER)
	{		
	    if(entry->ring[temp_ring_id-1].node.port[0].netif_index == rxPacket->sync_ctrl.netif_index)
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[1].netif_index; 
		}
		else
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[0].netif_index; 
		}	
	    erpp_send_packet(buff, ERPP_PACKET_LENGTH);
	}
	else
	{   
	    if(entry->ring[temp_ring_id-1].level == 1)
	    {
		    entry->ring[temp_ring_id-1].node.port[0].erpp_port_status = FORWARDING;
		    entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = FORWARDING;
	    }
		else
		{
		    entry->ring[temp_ring_id-1].node.port[0].erpp_port_status = FORWARDING;
		    entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = BLOCK;
		}
		entry->ring[temp_ring_id-1].node.erpp_node_status = NODE_FAIL;
		entry->flush_flag = 1;		
		entry->packet_ring = temp_ring_id;
		erpp_tx_type_packet(entry, COMMON_FLUSH_PACKET, entry->ring[temp_ring_id-1].node.port[0].netif_index);
		erpp_tx_type_packet(entry, COMMON_FLUSH_PACKET, entry->ring[temp_ring_id-1].node.port[1].netif_index);
		if (0 != erpp_domain_update(entry))
			return 1;
	}

	return 0;
}

int erpp_packet_edge_hello_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{
    unsigned short temp_ring_id = ntohs(erpp_buff->ring_id);
    union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)buff;
	if(0 == memcmp(erpp_buff->sys_mac, SysMac, sizeof(SysMac)))
	{
        entry->timer_count[0] = entry->hello_timer;
	}
	else if(entry->ring[temp_ring_id].node.erpp_node_role == NODE_ASSISTANT_EDGE)
	{
		entry->timer_count[2] = entry->fault_timer;
        entry->ring[temp_ring_id].node.erpp_node_status = ERPP_LINK_NORMAL;
	}
	else
	{		
	    if(entry->ring[temp_ring_id-1].node.port[0].netif_index == rxPacket->sync_ctrl.netif_index)
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[1].netif_index; 
		}
		else
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[0].netif_index; 
		}	
		erpp_send_packet(buff, ERPP_PACKET_LENGTH);
		return 0;
	}
	
	if (0 != erpp_domain_update(entry))
		return 1;
	return 0;
}

int erpp_packet_fault_process(struct erpp_packet_s *erpp_buff, char *buff, struct erpp_domain_s *entry)
{
    unsigned short temp_ring_id = ntohs(erpp_buff->ring_id);
	union erpp_packet_sync_ctrl_u *rxPacket = (union erpp_packet_sync_ctrl_u *)buff;
	if(entry->ring[temp_ring_id-1].node.erpp_node_role == NODE_EDGE)
	{
	    if(entry->ring[temp_ring_id-1].node.port[1].erpp_port_status == FORWARDING)
			entry->ring[temp_ring_id-1].node.port[1].erpp_port_status = BLOCK;
		
		if (0 != erpp_domain_update(entry))
			return 1;
	}
	else if(0 == memcmp(erpp_buff->sys_mac, SysMac, sizeof(SysMac)))
	{
        return 0;
	}
	else
	{	    
	    if(entry->ring[temp_ring_id-1].node.port[0].netif_index == rxPacket->sync_ctrl.netif_index)
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[1].netif_index; 
		}
		else
		{
			rxPacket->sync_ctrl.netif_index = entry->ring[temp_ring_id-1].node.port[0].netif_index; 
		}	
		erpp_send_packet(buff, ERPP_PACKET_LENGTH);
	}

	return 0;
}


int erpp_packet_rx_hander
(
    char *buff, 
    struct erpp_packet_s *erpp_buff,
    struct erpp_domain_s *entry,
    unsigned int packet_type
)
{
    int ret = 0;
	
	erpp_syslog_dbg("XXX: %s %d packet_type %d\n", __func__, __LINE__, packet_type);
	switch(packet_type)
	{
        case HELLO_PACKET:
	    {
			ret = erpp_packet_hello_process(erpp_buff, buff, entry);
            break;
		}
		case COMPLETE_FLUSH_PACKET:
		{
			ret = erpp_packet_complete_flush_process(erpp_buff, buff, entry);
            break;
		}
		case COMMON_FLUSH_PACKET:
	    {
			ret = erpp_packet_common_flush_process(erpp_buff, buff, entry);
            break;
		}
		case LINK_DOWN_PACKET: 
	    {
			ret = erpp_packet_link_down_process(erpp_buff, buff, entry);
            break;
		}
		case EDGE_HELLO_PACKET:
		{
			ret = erpp_packet_edge_hello_process(erpp_buff, buff, entry);
            break;
		}
		case FAULT_PACKET:
		{
			ret = erpp_packet_fault_process(erpp_buff, buff, entry);
            break;
		}
		default:
		{
            break;
		}
	}
	return ret;
}

int erpp_packet_rx_callback(int sock, char *buff , int len, void *private_data)
{
    int ret = 0;
    struct erpp_packet_s *erpp_buff = NULL;
	struct erpp_domain_s entry;
	
	if(len <= 0)
	{
		return 0;
	}
	
	erpp_buff = (struct erpp_packet_s* )(buff + sizeof(struct ethernet_header_s) + sizeof(union erpp_packet_sync_ctrl_u));

	if (ntohs(erpp_buff->ring_id) > ERPP_RING_SIZE)
    {
		erpp_syslog_dbg("This ring %d out of range .\n", ntohs(erpp_buff->ring_id));
		return -1;
	}
		
	memset(&entry, 0, sizeof(struct erpp_domain_s));
	entry.domain_id = ntohs(erpp_buff->domain_id);
	
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&entry))
	{
		erpp_syslog_dbg("Can not get entry domain_id %d.\n", ntohs(erpp_buff->domain_id));
		
		pthread_mutex_unlock(&semErppMutex);
		return -1;
	}

	if(entry.ring[ntohs(erpp_buff->ring_id)-1].is_enable == 0)
	{
		pthread_mutex_unlock(&semErppMutex);
		return -1;
	}
	
	ret = erpp_packet_rx_hander(buff, erpp_buff, &entry, erpp_buff->erpp_type);
	pthread_mutex_unlock(&semErppMutex);
	return ret;
}

int erpp_packet_socket_init()
{
    int ret = -1;
	memset(&erpp_tx_packet_client, 0, sizeof(struct sockaddr_un));	
	memset(&erpp_tx_packet_server, 0, sizeof(struct sockaddr_un));
	
    erpp_tx_packet_sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (erpp_tx_packet_sock < 0)
	{
		erpp_syslog_dbg("Creat socket failed.\n");
		return -1;
	}

	erpp_tx_packet_server.sun_family = AF_LOCAL;
	strcpy(erpp_tx_packet_server.sun_path, "/tmp/packet.rx.ERPP");

	erpp_tx_packet_client.sun_family = AF_LOCAL;
	strcpy(erpp_tx_packet_client.sun_path, "/tmp/packet.tx.ERPP");
	
	unlink(erpp_tx_packet_server.sun_path);
	
 	if (bind(erpp_tx_packet_sock, (struct sockaddr*)&erpp_tx_packet_server, sizeof(struct sockaddr_un)) < 0)
 	{
        close(erpp_tx_packet_sock);
        erpp_syslog_dbg("Bind msg recv socket failed.\n");
		return -1;
 	}
    
    (void)chmod(erpp_tx_packet_server.sun_path, 0777);
	
	ret = osal_register_read_fd(erpp_tx_packet_sock, ERPP_SERVICE_MAGIC, erpp_packet_rx_callback, NULL, 1);
	return ret;

}
int erpp_msg_hander(struct erpp_domain_s *erpp_buff)
{
    return 0;
}

int erpp_msg_callback(int sock, char *buff , int len, void *private_data)
{
	struct erpp_packet_s *erpp_buff = (struct erpp_packet_s *)buff;
	if(len <= 0)
	{
		return 0;
	}
	return erpp_msg_hander(erpp_buff);
}

int erpp_msg_sock_init()
{
    int ret = -1;
	memset(&erpp_msg_server, 0, sizeof(struct sockaddr_un));	
	memset(&erpp_msg_client, 0, sizeof(struct sockaddr_un));
	
    erpp_msg_sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (erpp_msg_sock < 0)
	{
		erpp_syslog_dbg("Creat socket failed.\n");
		return -1;
	}

	erpp_msg_server.sun_family = AF_LOCAL;
	strcpy(erpp_msg_server.sun_path, "/tmp/erpp_to_npd");

	erpp_msg_client.sun_family = AF_LOCAL;
	strcpy(erpp_msg_client.sun_path, "/tmp/npd_to_erpp");
	unlink(erpp_msg_client.sun_path);

 	if (bind(erpp_msg_sock, (struct sockaddr*)&erpp_msg_client, sizeof(struct sockaddr_un)) < 0)
 	{
        close(erpp_msg_sock);
        erpp_syslog_dbg("Bind msg recv socket failed.\n");
		return -1;
 	}
    
    (void)chmod(erpp_msg_client.sun_path, 0777);
	ret = osal_register_read_fd(erpp_msg_sock, ERPP_SERVICE_MAGIC, erpp_msg_callback, NULL, 1);
	return ret;
}

void erpp_send(int sock, struct sockaddr_un* to, char* data, unsigned int length)
{
    int	ret = 0;
    int byte_send = 0;

    if (sock < 0)
    {
        erpp_syslog_dbg("Socket isnot init...!\n");
        return;
    }

    while (length != byte_send)
    {
        ret = sendto(sock, data + byte_send, length - byte_send, 0,
                (struct sockaddr *)to, sizeof(*to));
        if (ret < 0)
        {
            if (errno == EINTR)/*send() be interrupted.*/
            {
                erpp_syslog_dbg("sendto was interrupted.\n");
                continue;
            }
            else
            {
                erpp_syslog_dbg("send message fail, %s.\n", strerror(errno));
                break;
            }
        }
        else
        {
            byte_send += ret;
        }
    }

    return ;
}

void erpp_send_packet(char* data, unsigned int length)
{
    erpp_send(erpp_tx_packet_sock, &erpp_tx_packet_client, data, length);
    return ;
}

void erpp_send_msg(char* data, unsigned int length)
{
    erpp_send(erpp_msg_sock, &erpp_msg_server, data, length);
    return ;
}

void erpp_assembly_packet(char* data, struct erpp_domain_s* entry, unsigned int erpp_type, unsigned int netif_index)
{
    union erpp_packet_sync_ctrl_u* rxPacket = NULL;
    struct ethernet_header_s* frame = NULL;	
    struct erpp_packet_s* packet = NULL;

    rxPacket = (union erpp_packet_sync_ctrl_u*)data;
    memset(rxPacket, 0, sizeof(union erpp_packet_sync_ctrl_u));
    
    rxPacket->sync_ctrl.netif_index = netif_index;
    rxPacket->sync_ctrl.vid = (unsigned short)entry->control_vlan_id[0];
    rxPacket->sync_ctrl.istagged = 1;
    rxPacket->sync_ctrl.packet_len = sizeof(struct erpp_packet_s);

    frame = (struct ethernet_header_s*)(data + sizeof(union erpp_packet_sync_ctrl_u));
    memset(frame, 0, sizeof(struct ethernet_header_s));
    memcpy(frame->dmac, ERPP_MUTICAST_MAC, sizeof(SysMac));
    memcpy(frame->smac, SysMac, sizeof(SysMac));
	
    packet = (struct erpp_packet_s* )(data + sizeof(struct ethernet_header_s) + sizeof (union erpp_packet_sync_ctrl_u));
    memset(packet, 0, sizeof(struct erpp_packet_s));
	
	packet->fix.pri = FIX_PRI;
	packet->fix.vlanid = (unsigned short)entry->control_vlan_id[0];
	packet->fix.control = FIX_CONTROL;
	packet->fix.dsap_ssap = htons(DSAP_SSAP);	
    memcpy(packet->fix.oui, FIX_OUI, sizeof(packet->fix.oui));
	packet->fix.frame_length = htons(FRAME_LENGTH);
	packet->fix.fixup_1 = htons(FIX_1);
	packet->fix.fixup_2 = FIX_2;
	packet->fix.fixup_3 = FIX_3;

	packet->erpp_length = htons(ERPP_LENGTH);
	packet->erpp_version = ERPP_VERSION;
	packet->erpp_type = erpp_type;
	packet->domain_id = htons(entry->domain_id);
	packet->ring_id = htons(entry->ring[entry->packet_ring-1].ring_id);
	packet->level = entry->ring[entry->packet_ring-1].level;
	packet->hello_timer = htons(entry->hello_timer);
	packet->fail_timer = htons(entry->fail_timer);
    memcpy(packet->sys_mac, SysMac, sizeof(SysMac));

    return ;
}

void erpp_tx_type_packet(struct erpp_domain_s* entry, unsigned int packet_type, unsigned int netif_index)
{
    char* data = NULL;

    data = (char*)malloc(ERPP_PACKET_LENGTH);
    if (NULL == data)
    {
        erpp_syslog_dbg("Memory exhaust, %s.\n", __func__);
        return ;
    }
    memset(data, 0, ERPP_PACKET_LENGTH);

    erpp_assembly_packet(data, entry, packet_type, netif_index);
    erpp_send_packet(data, ERPP_PACKET_LENGTH);

    if (NULL != data)
    {
        free(data);
    }
    else
    {
        printf("Memory leak, %s.\n", __func__);
    }

    return ;
}

#endif

