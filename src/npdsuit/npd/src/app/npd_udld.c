/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_udld.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		CHANOS uni-direction link detect protocol.
*
* DATE:
*		12/10/2011
*UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 0.01 $
*******************************************************************************/
#ifdef HAVE_UDLD
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "db_usr_api.h"
#include "dbus/npd/npd_dbus_def.h"

#include "udld/udld_main.h"
#include "udld/udld_msg.h"

struct sockaddr_un udld_npd_2_udld_addr;
struct sockaddr_un udld_udld_2_npd_addr;

int udld_msg_udld_2_npd_socket = -1;
int udld_msg_npd_2_udld_socket = -1;

int npd_udld_msg_send()
{
	return 0;
}

int npd_udld_ctrl_msg_hander(udld_ctrl_msg *udld_msg_buff)
{
	udld_netif_disable_msg *netif_msg = NULL;
	npd_syslog_dbg("UDLD: Recive CTRL msg.\r\n");
	if(udld_msg_buff->nlh.nlmsg_type != MSG_UDLD_2_NPD)
	{
		npd_syslog_err("UDLD: Wrong msg type.\r\n");
		return -1;
	}
	switch(udld_msg_buff->nlh.nlmsg_flags)
	{
		case MSG_NETIF_ADMIN_OP:
			if(udld_msg_buff->nlh.nlmsg_len != (sizeof(udld_ctrl_msg) + sizeof(udld_netif_disable_msg)))
			{
				npd_syslog_err("UDLD: Wrong msg length.\r\n");
				return -1;
			}
			netif_msg = (udld_netif_disable_msg *)udld_msg_buff->buffer;
			if(netif_msg->opcode == 0)
			{
				npd_syslog_dbg("UDLD: To disable netif %x.\r\n", netif_msg->netif_index);
				return eth_port_sw_attr_update(netif_msg->netif_index, ADMIN, 0);
			}
			else if(netif_msg->opcode == 1)
			{
				npd_syslog_dbg("UDLD: To enable netif %x.\r\n", netif_msg->netif_index);
				return eth_port_sw_attr_update(netif_msg->netif_index, ADMIN, 1);
			}
			else
			{
				npd_syslog_err("UDLD: Wrong msg op code.\r\n");
				return -1;
			}
			break;
		default:
			npd_syslog_err("UDLD: Wrong msg flags.\r\n");
			return -1;
	}
	
	return -1;
}

int npd_udld_msg_handler(int sock, char *buff , int len, void *private_data)
{
	int ret = -1;
	udld_ctrl_msg *udld_msg_buff = (udld_ctrl_msg *)buff;
	if(len <= 0)
	{
		return 0;
	}
	ret = npd_udld_ctrl_msg_hander(udld_msg_buff);
	return 0;
}

int npd_udld_msg_init()
{
	int ret = -1;
	
    memset(&udld_npd_2_udld_addr, 0, sizeof(struct sockaddr_un));
    memset(&udld_udld_2_npd_addr, 0, sizeof(struct sockaddr_un));

    if ((udld_msg_udld_2_npd_socket = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        return -1;
    }

    udld_npd_2_udld_addr.sun_family = AF_LOCAL;
    strcpy(udld_npd_2_udld_addr.sun_path, UDLD_MSG_NPD_2_UDLD);
	
    udld_udld_2_npd_addr.sun_family = AF_LOCAL;
    strcpy(udld_udld_2_npd_addr.sun_path, UDLD_MSG_UDLD_2_NPD);
	
    unlink(udld_udld_2_npd_addr.sun_path);
    if (bind(udld_msg_udld_2_npd_socket, (struct sockaddr *)&udld_udld_2_npd_addr, sizeof(udld_udld_2_npd_addr)) == -1)
    {
        return -1;
    }

    chmod(udld_udld_2_npd_addr.sun_path, 0777);
	udld_msg_npd_2_udld_socket = udld_msg_udld_2_npd_socket;
	ret = osal_register_read_fd(udld_msg_udld_2_npd_socket, UDLD_NPD_MSG_MAGIC, npd_udld_msg_handler, NULL, 1);
	return ret;
}

int npd_udld_init()
{
	if(npd_udld_msg_init() == 0)
	{
		nam_thread_create("udld_msg", (unsigned int (*)(void*))osal_thread_master_run, (void *)UDLD_NPD_MSG_MAGIC, 1, 0);
		return 0;
	}
	return -1;
}

#ifdef __cplusplus
}
#endif
#endif

