
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_ERPP
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/protocol/stp_api.h"
#include "npd_erpp.h"


int npd_erpp_app_msg_sock = -1;
struct	sockaddr_un erpp_app_msg_addr;
struct	sockaddr_un npd_app_msg_addr;


void npd_erpp_send(int sock, struct sockaddr_un* to, char* data, unsigned int length)
{
    int	ret = 0;
    int byte_send = 0;

    if (sock < 0)
    {
        npd_syslog_err("Socket isnot init...!\n");
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
                npd_syslog_err("sendto was interrupted.\n");
                continue;
            }
            else
            {
                npd_syslog_err("send message fail, %s.\n", strerror(errno));
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

void npd_erpp_send_msg(char* data, unsigned int length)
{
    npd_erpp_send(npd_erpp_app_msg_sock, &npd_app_msg_addr, data, length);
    return ;
}
int	nam_erpp_stp_set(unsigned int netif_index, unsigned int instance, int flag)
{
    npd_erpp_stp_flag(netif_index, instance, 1, flag);
    return 0;
}

int npd_erpp_msg_handler(int sock, char *buff , int len, void *private_data)
{
    int flag = 0;
    struct erpp_msg_to_npd_s* erpp_msg = NULL;

    if (NULL == buff)
    {
        npd_syslog_err("erpp recv-msg handler message null.\n");
        return -1;
    }
    erpp_msg = (struct erpp_msg_to_npd_s*)buff;
   
    if ((VALIDATE_BYTE_BEGIN != erpp_msg->validate_byte_begin)
        || (VALIDATE_BYTE_END != erpp_msg->validate_byte_end))
    {
        npd_syslog_err("erpp recv-msg handler message not match.\n");
        return -1;
    }
    

    npd_syslog_dbg("erpp flag %x, instance %x, is_enable %x port 1 %x\t2 %x status 1 %x\t2 %x\n",\
        erpp_msg->flush_flag, erpp_msg->instance, erpp_msg->is_enable, 
        erpp_msg->port[0].netif_index, erpp_msg->port[1].netif_index,
        erpp_msg->port[0].erpp_port_status, erpp_msg->port[1].erpp_port_status);

    if(erpp_msg->is_enable == 0)
    {
		nam_erpp_stp_set(erpp_msg->port[0].netif_index, erpp_msg->instance, NAM_STP_PORT_STATE_FORWARD_E);
		nam_erpp_stp_set(erpp_msg->port[1].netif_index, erpp_msg->instance, NAM_STP_PORT_STATE_FORWARD_E);
	}
	else
	{
		if (erpp_msg->port[0].erpp_port_status == FORWARDING)
		{
		    flag = NAM_STP_PORT_STATE_FORWARD_E;
		}
		else
			flag = NAM_STP_PORT_STATE_DISCARD_E;
		
		nam_erpp_stp_set(erpp_msg->port[0].netif_index, erpp_msg->instance, flag);

		if (erpp_msg->port[1].erpp_port_status == FORWARDING)
		{
		    flag = NAM_STP_PORT_STATE_FORWARD_E;
		}
		else
			flag = NAM_STP_PORT_STATE_DISCARD_E;

	    nam_erpp_stp_set(erpp_msg->port[1].netif_index, erpp_msg->instance, flag);
	}
	if (erpp_msg->flush_flag)
	{	    
	    (void)npd_arp_snooping_del_by_ifindex(erpp_msg->port[0].netif_index);
	    (void)npd_fdb_dynamic_entry_del_by_port(erpp_msg->port[0].netif_index);
	    (void)npd_arp_snooping_del_by_ifindex(erpp_msg->port[1].netif_index);
	    (void)npd_fdb_dynamic_entry_del_by_port(erpp_msg->port[1].netif_index);
	}

    return 0;
}

int npd_erpp_msg_init()
{
	int ret = -1;
	
    memset(&npd_app_msg_addr, 0, sizeof(struct sockaddr_un));
    memset(&erpp_app_msg_addr, 0, sizeof(struct sockaddr_un));

    if ((npd_erpp_app_msg_sock = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        return -1;
    }

    npd_app_msg_addr.sun_family = AF_LOCAL;
    strcpy(npd_app_msg_addr.sun_path, "/tmp/npd_to_erpp");
	
    erpp_app_msg_addr.sun_family = AF_LOCAL;
    strcpy(erpp_app_msg_addr.sun_path, "/tmp/erpp_to_npd");
	
    unlink(erpp_app_msg_addr.sun_path);
    if (bind(npd_erpp_app_msg_sock, (struct sockaddr *)&erpp_app_msg_addr, sizeof(erpp_app_msg_addr)) == -1)
    {
        return -1;
    }

    chmod(erpp_app_msg_addr.sun_path, 0777);
	ret = osal_register_read_fd(npd_erpp_app_msg_sock, ERPP_NPD_MSG_MAGIC, npd_erpp_msg_handler, NULL, 1);
	
	return ret;
}

int npd_erpp_init()
{
	if(npd_erpp_msg_init() == 0)
	{
		nam_thread_create("erpp_msg", (unsigned int (*)(void*))osal_thread_master_run, (void *)ERPP_NPD_MSG_MAGIC, 1, 0);
		return 0;
	}
	return -1;
}

#endif

