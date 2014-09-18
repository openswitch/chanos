#ifdef HAVE_SMART_LINK

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
#include "lib/npd_bitop.h"
#include "quagga/thread.h"

#include "smart_link_log.h"
#include "smart_link.h"
#include "smart_link_dbus.h"
#include "smart_link_packet.h"

int sl_tx_packet_sock = -1;          /* XXX: send packet for tx */
struct sockaddr_un sl_tx_packet_addr;
int sl_app_msg_sock = -1;       /* XXX: send msg to npd */
struct sockaddr_un sl_app_msg_addr;

unsigned char sl_sys_mac[6];

int smart_link_packet_socket_init()
{
    int sock = -1;
    int fl_flags = 0;

    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        smart_link_log_error("Creat socket failed.\n");
        return -1;
    }

    fl_flags = fcntl(sock, F_GETFL, 0);
	(void)fcntl(sock, F_SETFL, fl_flags | O_NONBLOCK);

    memset(&sl_tx_packet_addr, 0, sizeof(sl_tx_packet_addr));
    sl_tx_packet_addr.sun_family = AF_LOCAL;
    strcpy(sl_tx_packet_addr.sun_path, "/tmp/packet.tx.STLK");

    sl_tx_packet_sock = sock;

    return 0;
}

int smart_link_app_msg_sock_init()
{
    int fl_flags = 0;
    int sock = -1;
    struct sockaddr_un sl_app_msg_recv_addr;
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		smart_link_log_error("Creat msg socket failed.\n");
		return -1;
	}
    fl_flags = fcntl(sock, F_GETFL, 0);
	(void)fcntl(sock, F_SETFL, fl_flags | O_NONBLOCK);

    memset(&sl_app_msg_addr, 0, sizeof(sl_app_msg_addr));
	sl_app_msg_addr.sun_family = AF_LOCAL;
	strcpy(sl_app_msg_addr.sun_path, "/tmp/smartlink_npd_common");

    memset(&sl_app_msg_recv_addr, 0, sizeof(sl_app_msg_recv_addr));
	sl_app_msg_recv_addr.sun_family = AF_LOCAL;
	strcpy(sl_app_msg_recv_addr.sun_path, "/tmp/npd_to_smartlink_common");
    
	unlink(sl_app_msg_recv_addr.sun_path);
    (void)chmod(sl_app_msg_recv_addr.sun_path, 0777);

 	if (bind(sock, (struct sockaddr*)&sl_app_msg_recv_addr, sizeof(sl_app_msg_recv_addr)) < 0)
 	{
        close(sock);
        smart_link_log_error("Bind msg recv socket failed.\n");
		return -1;
 	}

    sl_app_msg_sock = sock;

    smart_link_event(SL_MSG, sock);
    
	return 0;
}

int smart_link_msg_processor(unsigned int id)
{
    struct smart_link_s entry;

    entry.id = id;
    if (0 != smart_link_group_search(&entry))
    {
        smart_link_log_error("Get entry(%d) failed.\n", id);
        return -1;
    }

    if (0 != entry.advertise_vlan)
    {
        smart_link_send_advertise(&entry);
    }
    else
    {
        smart_link_log_packet("Entry(%d) not configure advertise vlan.\n", id);
    }
    
    return 0;
}

void smart_link_send(int sock, struct sockaddr_un* to, char* data, unsigned int length)
{
    int	ret = 0;
    int byte_send = 0;

    if (sock < 0)
    {
        smart_link_log_packet("Socket isnot init...!\n");
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
                smart_link_log_error("sendto was interrupted.\n");
                continue;
            }
            else
            {
                smart_link_log_error("send message fail, %s.\n", strerror(errno));
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

void smart_link_send_packet(char* data, unsigned int length)
{
    smart_link_send(sl_tx_packet_sock, &sl_tx_packet_addr, data, length);
    return ;
}

void smart_link_send_msg(char* data, unsigned int length)
{
    smart_link_send(sl_app_msg_sock, &sl_app_msg_addr, data, length);
    return ;
}

int smart_link_recv_msg(struct thread * t)
{
    int ret = 0;
    int id = 0;
    int sock = -1;
    char buf[sizeof(id)];

    sock = THREAD_FD (t);
    smart_link_event(SL_MSG, sock);
    
    ret = read(sock, buf, sizeof(id));
    if (sizeof(id) != ret)
    {
        smart_link_log_error("Read message size mismatch (size:%s).\n", ret);
        return -1;
    }

    memcpy(&id, buf, sizeof(id));

    if (id < 1 || id > SMART_LINK_GROUP_SIZE)
    {
        smart_link_log_error("Entry id out of range (id:%d).\n", id);
        return -1;
    }

    smart_link_log_packet("read message %d\n", id);

    if (0 != smart_link_msg_processor(id))
    {
        smart_link_log_error("Receive message process failed.\n");
        /* XXX:not return -1 as of receive success, but process failed */
    }

    return 0;
}

int smart_link_sysmac_init()
{
    int fd = -1;
    char buf[32];
    char tmp_buf[16];
    unsigned int tmp_sys_mac[6];
    int ret = -1;
    int ni = 0;
    int nj = 0;

    fd = open("/devinfo/mac", O_RDONLY);
    if (fd < 0) 
    {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ret = read(fd, tmp_buf, 12);
    if (ret > 0)
    {
        for (ni = 0, nj = 0; ni < 12;)
        {
            buf[nj++] = tmp_buf[ni++];
            buf[nj++] = tmp_buf[ni++];
            buf[nj++] = ' ';
        }
        sscanf(buf, "%02x %02x %02x %02x %02x %02x", \
            &tmp_sys_mac[0], &tmp_sys_mac[1], &tmp_sys_mac[2], \
            &tmp_sys_mac[3], &tmp_sys_mac[4], &tmp_sys_mac[5]);
        
        sl_sys_mac[0] = (unsigned char)(tmp_sys_mac[0] & 0xff);
        sl_sys_mac[1] = (unsigned char)(tmp_sys_mac[1] & 0xff);
        sl_sys_mac[2] = (unsigned char)(tmp_sys_mac[2] & 0xff);
        sl_sys_mac[3] = (unsigned char)(tmp_sys_mac[3] & 0xff);
        sl_sys_mac[4] = (unsigned char)(tmp_sys_mac[4] & 0xff);
        sl_sys_mac[5] = (unsigned char)(tmp_sys_mac[5] & 0xff);
        
        smart_link_log_packet("%02x-%02x-%02x-%02x-%02x-%02x\n", \
                    sl_sys_mac[0], sl_sys_mac[1], sl_sys_mac[2], \
                    sl_sys_mac[3], sl_sys_mac[4], sl_sys_mac[5]);
    }
    else
    {
        close(fd);
        return -1;
    }
    close(fd);
    
    return 0;
}

void smart_link_assembly_packet(char* data, struct smart_link_s* entry)
{
    int vlan_id = 0;
    union smart_link_packet_sync_ctrl_u* sync_ctrl = NULL;
    struct ethernet_header_s* frame = NULL;
    struct advertise_s* packet = NULL;

    sync_ctrl = (union smart_link_packet_sync_ctrl_u*)data;
    memset(sync_ctrl->reserved, 0, sizeof(union smart_link_packet_sync_ctrl_u));
    
    sync_ctrl->sync_ctrl.netif_index = entry->master_port;
    sync_ctrl->sync_ctrl.vid = (unsigned short)entry->advertise_vlan;
    sync_ctrl->sync_ctrl.istagged = 1;
    sync_ctrl->sync_ctrl.packet_len = sizeof(struct advertise_s);

    frame = (struct ethernet_header_s*)(data + sizeof(union smart_link_packet_sync_ctrl_u));
    memset(frame, 0, sizeof(struct ethernet_header_s));
    memcpy(frame->dmac, SL_MUTICAST_MAC, sizeof(sl_sys_mac));
    memcpy(frame->smac, sl_sys_mac, sizeof(sl_sys_mac));
    frame->type = htons(SL_STLK_TYPE);

    packet = (struct advertise_s*)(data + sizeof(struct ethernet_header_s) + sizeof(union smart_link_packet_sync_ctrl_u));
    memset(packet, 0, sizeof(struct advertise_s));

    packet->version = SL_VERSION;
    packet->action = CLEAR_FDB | FLUSH_ARP;
    packet->auth_mode = SL_AUTHMODE_DLT;
    memcpy(packet->smac, sl_sys_mac, sizeof(sl_sys_mac));
    vlan_id = (unsigned short)entry->advertise_vlan;
    packet->advertise_vlan = htons(vlan_id);
    memcpy(&(packet->data_vlan), &(entry->data_vlan), sizeof(packet->data_vlan));

    return ;
}

void smart_link_send_advertise(struct smart_link_s* entry)
{
    char* data = NULL;
    int length = 0;

    length += sizeof(union smart_link_packet_sync_ctrl_u);
    length += sizeof(struct ethernet_header_s);
    length += sizeof(struct advertise_s);

    data = (char*)malloc(length);
    if (NULL == data)
    {
        smart_link_log_error("Memory exhaust, %s.\n", __func__);
        return ;
    }
    memset(data, 0, sizeof(length));

    smart_link_assembly_packet(data, entry);
    smart_link_send_packet(data, length);

    if (NULL != data)
    {
        free(data);
    }
    else
    {
        smart_link_log_error("Memory leak, %s.\n", __func__);
    }

    return ;
}

#endif

