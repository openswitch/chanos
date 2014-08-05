
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dldp_common.c
*
*
* CREATOR:
*		jinpc@autelan.com
*
* DESCRIPTION:
*		device link detection protocol for NPD module.
*
* DATE:
*		05/04/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.10 $	
*******************************************************************************/
#ifdef HAVE_DLDP
#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************
*	head files														*
**********************************************************/
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

/*********************************************************
*	global variable define											*
**********************************************************/
unsigned char DLDPEnable = NPD_DLDP_DISABLE;						/* DLDP global status, default is disable	*/							
int dldp_fd = NPD_DLDP_INIT_FD;										/* fd of the socket which by npd send 
																		message to DLDP					*/
unsigned char dldp_transMac[NPD_DLDP_MAC_ADDR_LEN] = {0};			/* temp use in transfer system mac address*/

/*local variables definition end */
struct sockaddr_un npd_dldp_addr;								/*local addr							*/	
struct sockaddr_un dldp_addr;									/*remote addr						*/


/*********************************************************
*	extern variable												*
**********************************************************/


/*********************************************************
*	functions define												*
**********************************************************/
/**********************************************************************************
 * npd_get_dldp_global_status
 *		get DLDP enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned char *status
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_get_dldp_global_status
(
	unsigned char *status
)
{
	if (!status) {
		syslog_ax_dldp_err("npd receive message from DLDP error, parameters is null.\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}
	
	*status = DLDPEnable;
	return NPD_DLDP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dldp_init
 *		init DLDP enable/disable global status in npd
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL
 *	 
 **********************************************************************************/
void npd_dldp_init
(
	void
)
{
	/*init Global DLDP Enable Flag*/
	DLDPEnable = NPD_DLDP_DISABLE;

	return ;
}

/**********************************************************************************
 * npd_dldp_init
 *		init npd to dldp socket 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		sock								- fd of the socket
 *	 	(-NPD_DLDP_RETURN_CODE_ERROR)	- fail
 *
 **********************************************************************************/
int	npd_dldp_sock_init
(
	void
)
{
	int sock = NPD_DLDP_INIT_FD;
	
	memset(&dldp_addr, 0, sizeof(struct sockaddr_un));
	memset(&npd_dldp_addr, 0, sizeof(struct sockaddr_un));

	if (-1 == (sock = socket(AF_LOCAL, SOCK_DGRAM, 0)))
	{
		syslog_ax_dldp_err("create npd to igmp socket fail, sock fd %d\n", sock);
		return (-NPD_DLDP_RETURN_CODE_ERROR);
	}
	
	npd_dldp_addr.sun_family = AF_LOCAL;
	strcpy(npd_dldp_addr.sun_path, NPD_NPD2DLDP_SERVER);

	dldp_addr.sun_family = AF_LOCAL;
	strcpy(dldp_addr.sun_path, NPD_NPD2DLDP_CLIENT);

    unlink(npd_dldp_addr.sun_path);

	if (-1 == bind(sock , (struct sockaddr *)&npd_dldp_addr, sizeof(struct sockaddr_un))) 
	{
		syslog_ax_dldp_err("npd to dldp socket created but failed when bind\n");
		return (-NPD_DLDP_RETURN_CODE_ERROR);
	}

	chmod(npd_dldp_addr.sun_path, 0777);

	return sock;	
}

/**********************************************************************************
 * npd_dldp_recv_info
 *		receive message from DLDP
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		len								- length of receive message
 *	 	(-NPD_DLDP_RETURN_CODE_ERROR)	- fail
 *
 **********************************************************************************/
unsigned int npd_dldp_recv_info
(
	NPD_MSG_FROM_DLDP_T *msg,
	unsigned int infoLen
)
{
	unsigned int len = 0; 
	unsigned int addrLen = sizeof(struct sockaddr_un);

	if (!msg) {
		syslog_ax_dldp_err("npd receive message from DLDP error, parameters is null.\n");
		return (-NPD_DLDP_RETURN_CODE_ERROR);
	}
		
	while (1) {
		len = recvfrom(dldp_fd, (char*)msg, infoLen, 0,(struct sockaddr *)&dldp_addr, &addrLen);
		if (len < 0 && errno == EINTR) {
			continue;
		}
		break;
	}

	return len;
}

/**********************************************************************************
 * npd_cmd_sendto_dldp
 *		send message to DLDP
 *
 *	INPUT:
 *		NPD_DEV_EVENT_CMD_DLDP_T *mngCmd
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 *
 **********************************************************************************/
 unsigned int npd_cmd_sendto_dldp
(
	NPD_DEV_EVENT_CMD_DLDP_T *mngCmd	
)
{
	unsigned int rc = NPD_DLDP_INIT_0;
	unsigned int cmdLen = sizeof(NPD_MSG_TO_DLDP_T);
	unsigned int byteSend = NPD_DLDP_INIT_0;

	NPD_MSG_TO_DLDP_T *devMsg = NULL;
	NPD_NOTIFY_MOD_DLDP_T *notify_format = NULL;

	if (!mngCmd) {
		syslog_ax_dldp_err("npd send message to DLDP error, parameters is null.\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	syslog_ax_dldp_dbg("npd send %s message to DLDP.\n",
						(mngCmd->event_type == NPD_DLDP_EVENT_DEV_REGISTER) ? "register vlan" :
						((mngCmd->event_type == NPD_DLDP_EVENT_DEV_UNREGISTER) ? "unregister vlan" :
						((mngCmd->event_type == NPD_DLDP_EVENT_DEV_VLAN_ADDIF) ? "add port" :
						((mngCmd->event_type == NPD_DLDP_EVENT_DEV_VLAN_DELIF) ? "del port" :
						((mngCmd->event_type == NPD_DLDP_EVENT_DEV_SYS_MAC_NOTI) ? "notify system MAC" :
						((mngCmd->event_type == NPD_DLDP_EVENT_DEV_SYS_DISABLE) ? "disable service" :
						"unknow"
						))))));

	/* malloc memory for debMsg	*/
	devMsg = (NPD_MSG_TO_DLDP_T *)malloc(sizeof(NPD_MSG_TO_DLDP_T));
	if (NULL == devMsg) {
		syslog_ax_dldp_err("devMsg memory alloc Fail!\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}
	memset(devMsg, 0, sizeof(NPD_MSG_TO_DLDP_T));
	
	notify_format = &(devMsg->npd_notify_msg);
	devMsg->nlh.nlmsg_len = sizeof(NPD_MSG_TO_DLDP_T);
	
	devMsg->nlh.nlmsg_type = NPD_DLDP_TYPE_DEVICE_EVENT;
	devMsg->nlh.nlmsg_flags = NPD_DLDP_FLAG_ADDR_MOD;
	
	notify_format->mod_type = mngCmd->event_type;
	notify_format->vlan_id = mngCmd->vlan_id;
	notify_format->ifindex = mngCmd->ifindex;
	notify_format->slot_no = mngCmd->soltno;
	notify_format->port_no = mngCmd->portno;
	notify_format->tagmode = mngCmd->tagmode;

	if (NPD_DLDP_EVENT_DEV_SYS_MAC_NOTI == notify_format->mod_type){
		memcpy(notify_format->sys_mac, dldp_transMac, NPD_DLDP_MAC_ADDR_LEN);
	}

	while (cmdLen != byteSend)
	{
		rc = sendto(dldp_fd, (char *)devMsg, sizeof(NPD_MSG_TO_DLDP_T), 0,
					(struct sockaddr *)&dldp_addr, sizeof(struct sockaddr_un));
		if (rc < 0)
		{
			/* send() be interrupted.	*/
			if (errno == EINTR) {
				npd_syslog_dbg("sendto \n");
				continue;
			}else if (errno == EACCES) {
				npd_syslog_dbg("sendto \n");
				npd_syslog_dbg("sendto() permision denied.\n");
				break;
			}else if (errno == EWOULDBLOCK) {
				npd_syslog_dbg("sendto1\n");
				break;
			}else if (errno == EBADF) {
				npd_syslog_dbg("sendto2\n");
				break;
			}else if (errno == ECONNRESET) {
				npd_syslog_dbg("sendto3\n");
				break;
			}else if (errno == EDESTADDRREQ) {
				npd_syslog_dbg("sendto4\n");
				break;
			}else if (errno == EFAULT) {
				npd_syslog_dbg("sendto5\n");
				break;
			}else if (errno == EINVAL) {
				npd_syslog_dbg("sendto6\n");
				break;
			}else if (errno == EISCONN) {
				npd_syslog_dbg("sendto7\n");
				break;
			}else if (errno == EMSGSIZE) {
				npd_syslog_dbg("sendto8\n");
				break;
			}else if (errno == ENOBUFS)	{
				npd_syslog_dbg("sendto9\n");
				break;
			}else if (errno == ENOMEM) {
				npd_syslog_dbg("sendto10\n");
				break;
			}else if (errno == ENOTCONN) {
				npd_syslog_dbg("sendto11\n");
				break;
			}else if (errno == ENOTSOCK) {
				npd_syslog_dbg("sendto12\n");
				break;
			}else if (errno == EOPNOTSUPP) {
				npd_syslog_dbg("sendto13\n");
				break;
			}else if (errno == EPIPE) {
				npd_syslog_dbg("sendto14\n");
				break;
			}else {
				npd_syslog_dbg("sendto15\n");
				syslog_ax_dldp_err("dldpCmd write to DLDP, write fail.\n");
				return NPD_DLDP_RETURN_CODE_ERROR;
			}
		}
		byteSend += rc;
	}
	
	if (byteSend == cmdLen) {
		rc = NPD_DLDP_RETURN_CODE_OK;	
	}

	free(devMsg);
	return rc;
}

/**********************************************************************************
 * npd_dldp_sysmac_notifer
 *		init npd to dldp socket 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 *
 **********************************************************************************/
unsigned int npd_dldp_sysmac_notifer
(
	void
)
{
 	unsigned int i = NPD_DLDP_INIT_0;
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char macByte = NPD_DLDP_INIT_0;
	unsigned char sysMac[13] = {0};	
	NPD_DEV_EVENT_CMD_DLDP_T npdMngMsg;

	memset(&npdMngMsg, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
	npdMngMsg.event_type = NPD_DLDP_EVENT_DEV_SYS_MAC_NOTI;
	npdMngMsg.vlan_id = 0;
	npdMngMsg.ifindex = 0;

	memcpy(dldp_transMac, PRODUCT_MAC_ADDRESS, 6);

	ret = npd_cmd_sendto_dldp(&npdMngMsg);
	if (NPD_DLDP_RETURN_CODE_OK != ret) {
		syslog_ax_dldp_err("npd mng msg write fail.\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	return ret;
}

/**********************************************************************************
 * npd_dldp_global_disable
 *		clear flag in vlan node and port node
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 *
 **********************************************************************************/
unsigned int npd_dldp_global_disable
(
	void
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned short vlanid = NPD_DLDP_INIT_0;
	struct vlan_s *vlanNode = NULL;
#ifdef CORRECT_DB

	for (vlanid = 1; vlanid <= 4095; vlanid++) {
		/* check vlan exist */
		
		vlanNode = npd_find_vlan_by_vid(vlanid);
		if (NULL == vlanNode)
		{
			/*syslog_ax_dldp_err("vlan %d not exist, continue.\n", vlanid);*/
			ret = NPD_DLDP_RETURN_CODE_OK;
			continue;
		}
		else
	    {
			/* set the DLDP enable/disable flag on vlan node */
			if (vlanNode->dldpEnDis == NPD_DLDP_ENABLE)
			{
				ret = npd_dldp_vlan_endis_config(NPD_DLDP_DISABLE, vlanid);
				if (ret != NPD_DLDP_RETURN_CODE_OK)
				{
					syslog_ax_dldp_err("disable DLDP on vlan %d error, ret %x.\n",
										vlanid, ret);
					npd_put_vlan(vlanNode);
					return NPD_DLDP_RETURN_CODE_ERROR;
				}
			}
			else
			{
				syslog_ax_dldp_warning("vlan %d not enabled DLDP.\n", vlanid);
				ret = NPD_DLDP_RETURN_CODE_OK;
			}
			npd_put_vlan(vlanNode);
		}
	}
#endif
	return ret;
}

/**********************************************************************************
 * npd_dldp_recover_port_status
 *		recover port state of DLDP
 *
 *	INPUT:
 *		dldp_notify	- message for modify hardware
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK				- success
 *		NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 	NPD_DLDP_RETURN_CODE_NULL_PTR			- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dldp_recover_port_status
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char devNum = NPD_DLDP_INIT_0;
	unsigned char portNum = NPD_DLDP_INIT_0;
	unsigned int stpId = NPD_DLDP_INIT_0;

	if (NULL == dldp_notify) {
		syslog_ax_dldp_err("recover port DLDP status error, parameter is NULL.\n");
 		return NPD_DLDP_RETURN_CODE_NULL_PTR;
	}

	syslog_ax_dldp_dbg("recover vlan %d ethport %x DLDP status to DISABLE.\n",
					dldp_notify->vlan_id, dldp_notify->ifindex);

	
	ret = nam_vlan_stpid_get(dldp_notify->vlan_id, &stpId);/*when bind,mstid == stpId*/
	if (NPD_DLDP_RETURN_CODE_0 != ret) {
		syslog_ax_dldp_err("recover port DLDP status, get stpid error, ret %x.\n", ret);
 		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	ret = nam_stp_state_set(dldp_notify->ifindex, stpId, NAM_STP_PORT_STATE_DISABLE_E);
	if (NPD_DLDP_RETURN_CODE_0 != ret) {
		syslog_ax_dldp_err("recover port DLDP status, set DISABLE stat error, ret %x.\n", ret);
 		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	return ret;
}

/**********************************************************************************
 * npd_dldp_discard_port_status
 *		set port state of DLDP is discard
 *
 *	INPUT:
 *		dldp_notify	- message for modify hardware
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK				- success
 *		NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 	NPD_DLDP_RETURN_CODE_NULL_PTR			- parameter is null
 *
 **********************************************************************************/
unsigned int npd_dldp_discard_port_status
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned int stpId = NPD_DLDP_INIT_0;

	if (NULL == dldp_notify) {
		syslog_ax_dldp_err("set port DLDP status error, parameter is NULL.\n");
 		return NPD_DLDP_RETURN_CODE_NULL_PTR;
	}

	syslog_ax_dldp_dbg("set vlan %d ethport %x DLDP status is DISCARD.\n",
					dldp_notify->vlan_id, dldp_notify->ifindex);

	ret = nam_vlan_stpid_get(dldp_notify->vlan_id, &stpId);/*when bind,mstid == stpId*/
	if (NPD_DLDP_RETURN_CODE_0 == ret) {
		/* found, update*/
		ret = nam_stp_state_set(dldp_notify->ifindex, stpId, NAM_STP_PORT_STATE_DISCARD_E);
		if (NPD_DLDP_RETURN_CODE_0 != ret) {
			syslog_ax_dldp_err("set port DLDP status, set DISCARD stat error, ret %x.\n", ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
		
		ret = NPD_DLDP_RETURN_CODE_OK;
	}else {
		/* not found, defautl bind to stpID 0, and set*/
		stpId = 0;
		ret = nam_stp_vlan_bind_stg(dldp_notify->vlan_id, stpId);
		if (NPD_DLDP_RETURN_CODE_0 != ret) {
			syslog_ax_dldp_err("set port DLDP status, bind vlan %d to stpid %d error, ret %x.\n",
							dldp_notify->vlan_id, stpId, ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}

		ret = nam_stp_state_set(dldp_notify->ifindex, stpId, NAM_STP_PORT_STATE_DISCARD_E);
		if (NPD_DLDP_RETURN_CODE_0 != ret) {
			syslog_ax_dldp_err("set port DLDP status, set DISCARD stat error, ret %x.\n", ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}

		ret = NPD_DLDP_RETURN_CODE_OK;
	}

	return ret;
}

/**********************************************************************************
 * npd_dldp_check_vlan_status
 *		check vlan exist, if exist get DLDP status.
 *
 *	INPUT:
 *		 unsigned short vlanid		- l2 vlan id
 *
 *	OUTPUT:
 *		unsigned char *status		- vlan's  DLDP status

 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK				- success
 *		NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 	NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *
 **********************************************************************************/
unsigned int npd_dldp_check_vlan_status
(
	unsigned short vlanid,
	unsigned char *status
)
{
	struct vlan_s *vlanNode = NULL;
#ifdef CORRECT_DB

	if (!status) {
		syslog_ax_dldp_err("dldp check vlan status error, parameters is null.\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}
	
	vlanNode = npd_find_vlan_by_vid(vlanid);
	if ((NULL == vlanNode)||(vlanNode->isAutoCreated)) {
		syslog_ax_dldp_err("not found vlan %d entity.\n", vlanid);
	    npd_put_vlan(vlanNode);
		return NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST;
	}
	*status =vlanNode->dldpEnDis;
	npd_put_vlan(vlanNode);
#endif
	return NPD_DLDP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dldp_vlan_port_endis_flag_set
 *		set port DLDP flag in vlan
 *
 *	INPUT:
 *		unsigned short vlanId		- vlan id
 *		unsigned int  eth_g_idx		- eth-port index
 *		unsigned char tagMode		- tag/untag
 *		unsigned char en_dis		- enable/disable
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK				- success
 *		NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 	NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *
 **********************************************************************************/
unsigned int npd_dldp_vlan_port_endis_flag_set
(
	unsigned short vlanId,
	unsigned int  eth_g_idx,
	unsigned char tagMode,
	unsigned char en_dis
)
{
	unsigned char isTagged = NPD_DLDP_INIT_0;
#ifdef CORRECT_DB
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos = NULL;
	vlan_port_list_s *portList = NULL;

	syslog_ax_dldp_dbg("DLDP set vlan %d %s eth-port %#x dldp_flag %d.\n",
						vlanId, tagMode ? "tagged" : "untagged", eth_g_idx, en_dis);
	
	vlanNode = npd_find_vlan_by_vid(vlanId);
	if (NULL == vlanNode) {
		return NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == tagMode) {
		portList = vlanNode->tagPortList;
	}else if(NPD_FALSE == tagMode) {
		portList = vlanNode->untagPortList;
	}else {
		syslog_ax_dldp_err("tag mode is error.\n");
	    npd_put_vlan(vlanNode);
		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	if (NULL == portList) {
	    npd_put_vlan(vlanNode);
		return NPD_DLDP_RETURN_CODE_ERROR;
	}else if (0 != portList->count) {
		syslog_ax_dldp_dbg("vlan %d %s port count %d\n",
							vlanId, isTagged ? "tagged" : "untagged", portList->count);
		list = &(portList->list);
		__list_for_each(pos, list) {
			node = list_entry(pos, struct vlan_port_list_node_s, list);
			if (NULL != node && eth_g_idx == node->eth_global_index) {
				node->dldpEnFlag = en_dis;
				syslog_ax_dldp_dbg("dldp_endis_flag %s ok.\n", node->dldpEnFlag ? "enabled" : "disabled");
	            npd_put_vlan(vlanNode);
				return NPD_DLDP_RETURN_CODE_OK;
			}
		}
	}
	npd_put_vlan(vlanNode);
#endif
	return NPD_DLDP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 * npd_dldp_vlan_endis_config
 *		
 *
 *    DISCRIPTION:
 *		config the dldp in vlan
 *
 *	INPUT:
 *		unsigned char isEnable		- enable/disable DLDP on vlan
 *		unsigned short vlanid		- l2 vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK				- success
 *		NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 	NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *
 **********************************************************************************/
unsigned int npd_dldp_vlan_endis_config
(
	unsigned char isEnable,
	unsigned short vlanid
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned int i = NPD_DLDP_INIT_0;
	unsigned char enDis = isEnable;
	unsigned char enDis_port = NPD_DLDP_PORT_DISABLE;

	struct vlan_s *vlanNode = NULL;
	struct eth_port_s *ethPort = NULL;
	unsigned int slot_index = NPD_DLDP_INIT_0;
	unsigned int slot_no = NPD_DLDP_INIT_0;
	unsigned int port_index = NPD_DLDP_INIT_0;
	unsigned int port_no = NPD_DLDP_INIT_0;
	unsigned char tagMode = NPD_DLDP_INIT_0;

	unsigned int mbrCnt = NPD_DLDP_INIT_0;
	unsigned int eth_g_index[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];
	
	NPD_DEV_EVENT_CMD_DLDP_T npdMsg_vlan;
	NPD_DEV_EVENT_CMD_DLDP_T npdMsg_port;
#ifdef CORRECT_DB

	memset(eth_g_index, 0, sizeof(unsigned int) * MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT * CHASSIS_SLOT_COUNT);
	memset(&npdMsg_vlan, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
	memset(&npdMsg_port, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));

	syslog_ax_dldp_dbg("start config DLDP on vlan\n");

	/* check vlan exist	*/
	ret = npd_check_vlan_exist(vlanid);
	if (NPD_TRUE != ret) {
		syslog_ax_dldp_err("vlan %d not exist.\n", vlanid);
		return NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST;
	}else {
		/* set the DLDP enable/disable flag on vlan node */
		vlanNode = npd_find_vlan_by_vid(vlanid);
		vlanNode->dldpEnDis = enDis;

		#if 0
		ret = nam_dldp_set_vlan_filter(vlanid, enDis);
		if (NPD_DLDP_RETURN_CODE_0 != ret ) {
			syslog_ax_dldp_err("set vlan %d filter fail, ret %x.", vlanid, ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
		#endif

		/* send message to DLDP, to register/unregister vlan	*/
		if (enDis) {
			npdMsg_vlan.event_type = NPD_DLDP_EVENT_DEV_REGISTER;
			ret = npd_fdb_add_dldp_vlan_system_mac(vlanid);
			if (NPD_DLDP_RETURN_CODE_0 != ret ) {
				syslog_ax_dldp_err("add vlan %d dldp fdb item fail, ret %x.\n", vlanid, ret);
	            npd_put_vlan(vlanNode);
				return NPD_DLDP_RETURN_CODE_ERROR;
			}
		}else {
			npdMsg_vlan.event_type = NPD_DLDP_EVENT_DEV_UNREGISTER;
			ret = npd_fdb_del_dldp_vlan_system_mac(vlanid);
			if (NPD_DLDP_RETURN_CODE_0 != ret ) {
				syslog_ax_dldp_err("del vlan %d dldp fdb item fail, ret %x.\n", vlanid, ret);
	            npd_put_vlan(vlanNode);
				return NPD_DLDP_RETURN_CODE_ERROR;
			}
		}
	    npd_put_vlan(vlanNode);
		npdMsg_vlan.vlan_id= vlanid;
		npdMsg_vlan.ifindex = NPD_DLDP_INIT_0;
		
		/*dgram socket*/
		ret = npd_cmd_sendto_dldp(&npdMsg_vlan);
		if (NPD_DLDP_RETURN_CODE_OK != ret ) {
			syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}

		/* enDis: enable, set DLDP on all-ports member of vlan	*/
		/* enDis: disable, clear DLDP on all-ports member of vlan	*/
		enDis_port = (enDis == NPD_DLDP_VLAN_ENABLE) ? NPD_DLDP_PORT_ENABLE : NPD_DLDP_PORT_DISABLE;

		/* config untag ports */
        
		{
            unsigned int array_port;
			NPD_PBMP_ITER(vlanNode->untag_ports, array_port)
			{
                unsigned int netif_index = netif_array_index_to_ifindex(array_port);
				ethPort = npd_get_port_by_index(netif_index);
				if(ethPort == NULL)
				{
					continue;
				}
				if (enDis) {
					syslog_ax_dldp_dbg("Before >>port %d funcs_run_bitmap 0x%x.\n",
										eth_g_index[i], ethPort->funcs.funcs_run_bitmap);
					ethPort->funcs.funcs_run_bitmap |= (1 << ETH_PORT_FUNC_DLDP);
					syslog_ax_dldp_dbg("After >>port %d funcs_run_bitmap 0x%x.\n",
										eth_g_index[i], ethPort->funcs.funcs_run_bitmap);
				}else {
					ethPort->funcs.funcs_run_bitmap ^= 1 << ETH_PORT_FUNC_DLDP;
				}
                free(ethPort);
				/* set/clear vlan port DLDP enDis flag*/
				ret = npd_dldp_vlan_port_endis_flag_set(vlanid, eth_g_index[i], 0, enDis_port);
				if (NPD_DLDP_RETURN_CODE_OK != ret) {
					syslog_ax_dldp_err("%s eth-port %#x DLDP flag error, ret %x.\n",
										enDis_port ? "set" : "clear", eth_g_index[i], ret);
					continue;
				}

				/* send message to DLDP, to register/unregister port	*/
				/* get solt_no and port_no */
				npd_eth_port_get_slotno_portno_by_eth_g_index(eth_g_index[i], &slot_no, &port_no);
				/* get port tagmode */
				ret = npd_vlan_check_contain_port(vlanid, eth_g_index[i], &tagMode);
				if (NPD_TRUE != ret) {
					syslog_ax_dldp_warning("Port %d has not be the member of vlan % d\n",
											eth_g_index[i], vlanid);
					continue;
				}

				if (enDis) {
					npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_ADDIF;
				}else {
					npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_DELIF;
				}
				npdMsg_port.vlan_id = vlanid;
				npdMsg_port.ifindex = eth_g_index[i];
				npdMsg_port.soltno = slot_no;
				npdMsg_port.portno = port_no;
				npdMsg_port.tagmode = tagMode;

				/*dgram socket*/
				ret = npd_cmd_sendto_dldp(&npdMsg_port);
				if (NPD_DLDP_RETURN_CODE_OK != ret) {
					syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
					return NPD_DLDP_RETURN_CODE_ERROR;
				}

				/* clear for next loop */
				slot_no = NPD_DLDP_INIT_0;
				port_no = NPD_DLDP_INIT_0;
				tagMode = NPD_DLDP_INIT_0;

				memset(&npdMsg_port, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
			}
			
			ret = NPD_DLDP_RETURN_CODE_OK;
		}
		else {
			syslog_ax_dldp_warning("Warning: npd DLDP get vlan's untag port member null, ret %x.\n", ret);
			ret = NPD_DLDP_RETURN_CODE_OK;
			/*return NPD_DLDP_RETURN_CODE_ERROR;*/
		}

		/* config tag ports */
		memset(eth_g_index, 0, sizeof(unsigned int) * MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT * CHASSIS_SLOT_COUNT);
		mbrCnt = NPD_DLDP_INIT_0;
		ret = npd_vlan_member_port_index_get(vlanid,	/*vlanId			*/
											 1, 		/*tagMode, tag	*/
											 eth_g_index,
											 &mbrCnt);
		if (NPD_TRUE == ret)
		{
			for(i = 0; i < mbrCnt; i++)
			{
				ethPort = npd_get_port_by_index(eth_g_index[i]);
				if (NULL == ethPort) {
					syslog_ax_dldp_err("npd DLDP get port by index error, eth-port is null.\n");
					return NPD_DLDP_RETURN_CODE_ERROR;
				}

				if (enDis) {
					syslog_ax_dldp_dbg("Before >>port %d funcs_run_bitmap 0x%x.\n",
										eth_g_index[i], ethPort->funcs.funcs_run_bitmap);
					ethPort->funcs.funcs_run_bitmap |= (1 << ETH_PORT_FUNC_DLDP);
					syslog_ax_dldp_dbg("After >>port %d funcs_run_bitmap 0x%x.\n",
										eth_g_index[i], ethPort->funcs.funcs_run_bitmap);
				}else {
					ethPort->funcs.funcs_run_bitmap ^= 1 << ETH_PORT_FUNC_DLDP;
				}
				free(ethPort);
				/* set/clear vlan port DLDP enDis flag*/
				ret = npd_dldp_vlan_port_endis_flag_set(vlanid, eth_g_index[i], 1, enDis_port);
				if (NPD_DLDP_RETURN_CODE_OK != ret) {
					syslog_ax_dldp_err("%s eth-port %#x DLDP flag error, ret %x.\n",
										enDis_port ? "set" : "clear", eth_g_index[i], ret);
					continue;
				}

				/* send message to DLDP, to register/unregister port	*/
				/* get solt_no and port_no */
				npd_eth_port_get_slotno_portno_by_eth_g_index(eth_g_index[i], &slot_no, &port_no);
				/* get port tagmode */
				ret = npd_vlan_check_contain_port(vlanid, eth_g_index[i], &tagMode);
				if (NPD_TRUE != ret) {
					syslog_ax_dldp_warning("Port %d has not be the member of vlan % d\n",
											eth_g_index[i], vlanid);
					continue;
				}

				if (enDis) {
					npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_ADDIF;
				}else {
					npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_DELIF;
				}
				npdMsg_port.vlan_id= vlanid;
				npdMsg_port.ifindex = eth_g_index[i];
				npdMsg_port.soltno = slot_no;
				npdMsg_port.portno = port_no;
				npdMsg_port.tagmode = tagMode;
				
				/*dgram socket*/
				ret = npd_cmd_sendto_dldp(&npdMsg_port);
				if (NPD_DLDP_RETURN_CODE_OK != ret) {
					syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
					return NPD_DLDP_RETURN_CODE_ERROR;
				}

				/* clear for next loop */
				slot_no = NPD_DLDP_INIT_0;
				port_no = NPD_DLDP_INIT_0;
				tagMode = NPD_DLDP_INIT_0;
				memset(&npdMsg_port, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
			}

			ret = NPD_DLDP_RETURN_CODE_OK;
		}
		else {
			syslog_ax_dldp_warning("Warning: npd DLDP get vlan's tag port member null, ret %x.\n", ret);
			ret = NPD_DLDP_RETURN_CODE_OK;
			/*return NPD_DLDP_RETURN_CODE_ERROR;*/
		}
	}
#endif
	return ret;
}

/**********************************************************************************
 * npd_dldp_msg_disable
 *		send DISABLE service message to DLDP. 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 *
 **********************************************************************************/
unsigned int npd_dldp_msg_disable
(
	void
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	NPD_DEV_EVENT_CMD_DLDP_T npdMsg_vlan;
	
	memset(&npdMsg_vlan, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));

	syslog_ax_dldp_dbg("send DISABLE service message to DLDP.\n");

	npdMsg_vlan.event_type = NPD_DLDP_EVENT_DEV_SYS_DISABLE;
	npdMsg_vlan.vlan_id= NPD_DLDP_INIT_0;
	npdMsg_vlan.ifindex = NPD_DLDP_INIT_0;
	
	/*dgram socket*/
	ret = npd_cmd_sendto_dldp(&npdMsg_vlan);
	if (NPD_DLDP_RETURN_CODE_OK != ret ) {
		syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	return ret;
}

/**********************************************************************************
 * npd_dldp_recvmsg_proc
 *		config dev according to running result of dldp protocol
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NPD_DLDP_RETURN_CODE_OK		- success
 *	 	NPD_DLDP_RETURN_CODE_ERROR		- fail
 *
 **********************************************************************************/
unsigned int npd_dldp_recvmsg_proc
(
	NPD_DLDP_NOTIFY_MOD_NPD_T *dldp_notify
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned short vlanId = NPD_DLDP_INIT_0;
	unsigned int eth_g_index = NPD_DLDP_INIT_0;

	if (!dldp_notify) {
		syslog_ax_dldp_err("npd receive message from DLDP error, patameter is null!\n");
		return NPD_DLDP_RETURN_CODE_ERROR;
	}

	syslog_ax_dldp_dbg("npd receive message from dldp: type %d vlan_id %d eth_g_index %d reserve %d.\n",
						dldp_notify->mod_type, 
						dldp_notify->vlan_id,
						dldp_notify->ifindex,
						dldp_notify->reserve);

	vlanId = dldp_notify->vlan_id;
	eth_g_index = (unsigned int)dldp_notify->ifindex;

	switch (dldp_notify->mod_type)
	{
		case NPD_DLDP_SYS_SET_INIT:
		{
			/* set DLDP Enable Global Flag */
			if (!DLDPEnable) {
				DLDPEnable = NPD_DLDP_ENABLE;
			}
			syslog_ax_dldp_dbg("Set DLDP enable global flag: %s.\n", DLDPEnable ? "enable" : "disable");

			/* transfer Product system MAC address to DLDP */
			ret = npd_dldp_sysmac_notifer();
			if (NPD_DLDP_RETURN_CODE_OK != ret){
				syslog_ax_dldp_err("npd sysmac notifier to DLDP error, ret %x!\n", ret);
			}
			break;	
		}
		case NPD_DLDP_SYS_SET_STOP:
		{
			/* clear DLDP Enable Global Flag */
			DLDPEnable = NPD_DLDP_DISABLE;
			syslog_ax_dldp_dbg("set DLDP enable global flag: %s.\n", DLDPEnable ? "enable" : "disable");

			/* need clear flag in vlan node and port node*/
			ret = npd_dldp_global_disable();
			if (NPD_DLDP_RETURN_CODE_OK != ret){
				syslog_ax_dldp_err("npd global disable DLDP error, ret %x!\n", ret);
			}

			/* send message to dldp for end service */
			ret = npd_dldp_msg_disable();
			if (NPD_DLDP_RETURN_CODE_OK != ret){
				syslog_ax_dldp_err("send global disable DLDP service error, ret %x!\n", ret);
			}

			break;	
		}
		case NPD_DLDP_SYS_RECOVER_PORT:
		{
			ret = npd_dldp_recover_port_status(dldp_notify);
			if (NPD_DLDP_RETURN_CODE_OK != ret){
				syslog_ax_dldp_err("recover port DLDP status error, ret %x!\n", ret);
			}

			break;	
		}
		case NPD_DLDP_SYS_SHUTDOWN_PORT:
		{
			ret = npd_dldp_discard_port_status(dldp_notify);
			if (NPD_DLDP_RETURN_CODE_OK != ret){
				syslog_ax_dldp_err("set port DLDP status is DISCARD error, ret %x!\n", ret);
			}

			break;	
		}
		default:
		{
			syslog_ax_dldp_dbg("npd can not proccess the message which type is %d of DLDP Protocol.\n",
								dldp_notify->mod_type);
			break;
		}
	}

	return ret;
}

/**********************************************************************************
 * npd_dldp_save_vlan_cfg
 *		get string of DLDP show running config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void npd_dldp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned int i = NPD_DLDP_INIT_0;
	unsigned int length = NPD_DLDP_INIT_0;
	unsigned char status = NPD_DLDP_INIT_0;
	unsigned char *showStr = NULL;
#ifdef CORRECT_DB

	struct vlan_s* node = NULL;

	if (NULL == buf || NULL == enDis) {
		syslog_ax_dldp_err("dldp show running config, parameter is null error\n");
		return ;
	}

	showStr = buf;

	ret = npd_get_dldp_global_status(&status);
	if (NPD_DLDP_ENABLE == status)
	{
		*enDis = NPD_DLDP_ENABLE;
		if ((length + 20) < 2048) {
			length += sprintf(showStr, "config dldp enable\n");
			syslog_ax_dldp_dbg("%s\n", showStr);
			showStr = buf + length;
		}

		for (i = 1; i < NPD_MAX_VLAN_ID; i++)
		{
			if (NPD_TRUE == npd_check_vlan_exist(i))
			{
				node = npd_find_vlan_by_vid(i);
				if (NULL != node)
				{
					if (NPD_DLDP_VLAN_ENABLE == node->dldpEnDis)
					{
						if((length + 30) < 2048) {
							length += sprintf(showStr, "config dldp vlan %d enable\n", i);
							syslog_ax_dldp_dbg("%s\n", showStr);
							showStr = buf + length;
						}
					}
	                npd_put_vlan(node);
				}
			}
		}
	}
	else {
		*enDis = NPD_DLDP_DISABLE;
	}
#endif
	return ;
} 

/**********************************************************************************
 * npd_dldp_port_add
 *		when add a port to vlan which have enabled DLDP,
 *		here need to enable on the port and send a add port message to DLDP
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan id
 *		unsigned int eth_g_index,		- port global index
 *		unsigned char isTagged			- port's tag mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
unsigned int npd_dldp_port_add
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned char isTagged
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char global_status = NPD_DLDP_INIT_0;
	unsigned char vlan_status = NPD_DLDP_INIT_0;
	struct eth_port_s *ethPort = NULL;
	unsigned int slot_index = NPD_DLDP_INIT_0;
	unsigned int slot_no = NPD_DLDP_INIT_0;
	unsigned int port_index = NPD_DLDP_INIT_0;
	unsigned int port_no = NPD_DLDP_INIT_0;
	NPD_DEV_EVENT_CMD_DLDP_T npdMsg_port;
	memset(&npdMsg_port, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
#ifdef CORRECT_DB

	syslog_ax_dldp_dbg("DLDP add port to vlan %d\n", vlanId);

	/* check if DLDP enable global*/
	ret = npd_get_dldp_global_status(&global_status);
	if (global_status != NPD_DLDP_ENABLE) {
		syslog_ax_dldp_err("DLDP not enable global.\n");
		return NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	/* check target vlan if enable DLDP*/
	ret = npd_dldp_check_vlan_status(vlanId, &vlan_status);
	if (vlan_status == NPD_DLDP_VLAN_ENABLE) {
		ethPort = npd_get_port_by_index(eth_g_index);
		if (NULL == ethPort) {
			syslog_ax_dldp_err("DLDP get port by index error, eth-port is null.\n");
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
		
		syslog_ax_dldp_dbg("Before >>port %d funcs_run_bitmap 0x%x.\n",
							eth_g_index, ethPort->funcs.funcs_run_bitmap);
		ethPort->funcs.funcs_run_bitmap |= (1 << ETH_PORT_FUNC_DLDP);
		syslog_ax_dldp_dbg("After >>port %d funcs_run_bitmap 0x%x.\n",
							eth_g_index, ethPort->funcs.funcs_run_bitmap);

		ret = npd_dldp_vlan_port_endis_flag_set(vlanId, eth_g_index, isTagged, NPD_DLDP_PORT_ENABLE);
		if (ret != NPD_DLDP_RETURN_CODE_OK) {
			syslog_ax_dldp_err("enable eth-port %#x DLDP flag error, ret %x.\n",
								eth_g_index, ret);
			return ret;
		}

		/* send message to DLDP, to unregister port */
		/* get solt_no and port_no */
		npd_eth_port_get_slotno_portno_by_eth_g_index(eth_g_index, &slot_no, &port_no);

		npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_ADDIF;
		npdMsg_port.vlan_id = vlanId;
		npdMsg_port.ifindex = eth_g_index;
		npdMsg_port.soltno = slot_no;
		npdMsg_port.portno = port_no;
		npdMsg_port.tagmode = isTagged;
		
		/*dgram socket*/
		ret = npd_cmd_sendto_dldp(&npdMsg_port);
		if (NPD_DLDP_RETURN_CODE_OK != ret) {
			syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
	}
	else {
		syslog_ax_dldp_dbg("not enable DLDP on vlan %d.\n", vlanId);
		ret = NPD_DLDP_RETURN_CODE_OK;
	}
#endif
	return ret;
}


/**********************************************************************************
 * npd_dldp_port_del
 *		when del a port from vlan which have enabled DLDP,
 *		here need to disable on the port and send a delete port message to DLDP
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan id
 *		unsigned int eth_g_index,		- port global index
 *		unsigned char isTagged			- port's tag mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
unsigned int npd_dldp_port_del
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned char isTagged
)
{
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char global_status = NPD_DLDP_INIT_0;
	unsigned char vlan_status = NPD_DLDP_INIT_0;
	struct eth_port_s *ethPort = NULL;
	unsigned int slot_index = NPD_DLDP_INIT_0;
	unsigned int slot_no = NPD_DLDP_INIT_0;
	unsigned int port_index = NPD_DLDP_INIT_0;
	unsigned int port_no = NPD_DLDP_INIT_0;
	NPD_DEV_EVENT_CMD_DLDP_T npdMsg_port;
	memset(&npdMsg_port, 0, sizeof(NPD_DEV_EVENT_CMD_DLDP_T));
#ifdef CORRECT_DB

	syslog_ax_dldp_dbg("DLDP delete port from vlan %d\n", vlanId);

	/* check if DLDP enable global*/
	ret = npd_get_dldp_global_status(&global_status);
	if (global_status != NPD_DLDP_ENABLE) {
		syslog_ax_dldp_err("DLDP not enable global.\n");
		return NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	/* check target vlan if enable DLDP*/
	ret = npd_dldp_check_vlan_status(vlanId, &vlan_status);
	if (vlan_status == NPD_DLDP_VLAN_ENABLE) {
		ethPort = npd_get_port_by_index(eth_g_index);
		if (NULL == ethPort) {
			syslog_ax_dldp_err("npd DLDP get port by index error, eth-port is null.\n");
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
		
		ethPort->funcs.funcs_run_bitmap ^= 1 << ETH_PORT_FUNC_DLDP;

		ret = npd_dldp_vlan_port_endis_flag_set(vlanId, eth_g_index, isTagged, NPD_DLDP_PORT_DISABLE);
		if (ret != NPD_DLDP_RETURN_CODE_OK) {
			syslog_ax_dldp_err("disable eth-port %#x DLDP flag error, ret %x.\n",
								eth_g_index, ret);
			return ret;
		}

		/* send message to DLDP, to unregister port	*/
		/* get solt_no and port_no */
		npd_eth_port_get_slotno_portno_by_eth_g_index(eth_g_index, &slot_no, &port_no);

		npdMsg_port.event_type = NPD_DLDP_EVENT_DEV_VLAN_DELIF;
		npdMsg_port.vlan_id = vlanId;
		npdMsg_port.ifindex = eth_g_index;
		npdMsg_port.soltno = slot_no;
		npdMsg_port.portno = port_no;
		npdMsg_port.tagmode = isTagged;
		
		/*dgram socket*/
		ret = npd_cmd_sendto_dldp(&npdMsg_port);
		if (NPD_DLDP_RETURN_CODE_OK != ret) {
			syslog_ax_dldp_err("npd DLDP mng msg write fail, ret %x.\n", ret);
			return NPD_DLDP_RETURN_CODE_ERROR;
		}
 	}
	else {
		syslog_ax_dldp_dbg("not enable DLDP on vlan %d.\n", vlanId);
		ret = NPD_DLDP_RETURN_CODE_OK;
	}
#endif
	return ret;
}

/**********************************************************************************
 * npd_dbus_dldp_check_global_status
 *		check and get DLDP enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_check_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char status = NPD_DLDP_INIT_0;	
	
	dbus_error_init(&err);
	
	ret = npd_get_dldp_global_status(&status);
	if (ret != NPD_DLDP_RETURN_CODE_OK) {
		syslog_ax_dldp_err("check dldp global status %s error, ret %x\n",
							status ? "enable" : "disable", ret);
	}
	syslog_ax_dldp_dbg("check dldp global status %s\n",
						status ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &status);
	return reply;
} 

/**********************************************************************************
 * npd_dbus_dldp_config_vlan
 *		enable/disable DLDP on vlan and its ports, base vlan 
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
  *			NPD_DLDP_RETURN_CODE_OK				- success
 *			NPD_DLDP_RETURN_CODE_ERROR				- fail
 *	 		NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *			NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL	- not enable DLDP global
 *			NPD_DLDP_RETURN_CODE_NOTENABLE_VLAN	- L2 vlan not enable DLDP
 *			NPD_DLDP_RETURN_CODE_HASENABLE_VLAN	- L2 vlan has enabled DLDP
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_config_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter  iter;
	DBusError err;

	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DLDP_VLAN_DISABLE;
	unsigned char status_global = NPD_DLDP_DISABLE;
	unsigned char status_vlan = NPD_DLDP_VLAN_DISABLE;
	unsigned short vlanid = NPD_DLDP_INIT_0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanid,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dldp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dldp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dldp_err("return error caused dbus.\n");
		return NULL;
	}

	syslog_ax_dldp_dbg("get params: vlan %d enDis %s.\n",
						vlanid, isEnable ? "enable" : "disable");

	ret = npd_get_dldp_global_status(&status_global);
	if (NPD_DLDP_DISABLE == status_global) {
		syslog_ax_dldp_err("DLDP not enabled global.\n");
		ret = NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL;
	}else {
		/* check vlan exist, if exist get DLDP status.	*/
		ret = npd_dldp_check_vlan_status(vlanid, &status_vlan);
		if (NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST == ret) {
			syslog_ax_dldp_err("vlan %d not exist.\n", vlanid);
			ret = NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST;
		}else if (NPD_DLDP_RETURN_CODE_ERROR == ret) {
			syslog_ax_dldp_err("parameter is null.\n", vlanid);
			ret = NPD_DLDP_RETURN_CODE_ERROR;
		}else if ((NPD_DLDP_VLAN_DISABLE == isEnable) &&
				  (NPD_DLDP_VLAN_DISABLE == status_vlan)) {
			syslog_ax_dldp_err("vlan %d not enabled DLDP.\n", vlanid);
			ret = NPD_DLDP_RETURN_CODE_NOTENABLE_VLAN;
		}else if ((NPD_DLDP_VLAN_ENABLE == isEnable) &&
				 (NPD_DLDP_VLAN_ENABLE == status_vlan)) {
			syslog_ax_dldp_err("vlan %d already enabled DLDP.\n", vlanid);
			ret = NPD_DLDP_RETURN_CODE_HASENABLE_VLAN;
		}else {
			ret = npd_dldp_vlan_endis_config(isEnable, vlanid);
			if (ret != NPD_DLDP_RETURN_CODE_OK) {
				syslog_ax_dldp_err("%s DLDP on vlan %d error, ret %x.\n",
									isEnable ? "enable" : "disable", vlanid, ret);
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/**********************************************************************************
 * npd_dbus_dldp_config_vlan
 *		enable/disable DLDP on vlan and its ports, base vlan 
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			NPD_DLDP_RETURN_CODE_OK				- success
 *			NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL	- not enable DLDP global
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_get_vlan_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusError err;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;

	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned int count = NPD_DLDP_INIT_0;
	unsigned short vlanid = NPD_DLDP_INIT_0;
	struct vlan_s *vlanNode = NULL;
	unsigned char status_global = NPD_DLDP_DISABLE;
#ifdef CORRECT_DB
	dbus_error_init(&err);

	ret = npd_get_dldp_global_status(&status_global);
	if (NPD_DLDP_DISABLE == status_global) {
		syslog_ax_dldp_err("DLDP not enabled global.\n");
		ret = NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL;
	}else {
		for (vlanid = 1; vlanid <= 4095; vlanid++) {
			/* check vlan exist */
			vlanNode = npd_find_vlan_by_vid(vlanid);
			if (NULL == vlanNode) {
				syslog_ax_dldp_err("vlan %d not exist, continue.\n", vlanid);
				continue;
			}else {
				if (vlanNode->dldpEnDis == NPD_DLDP_ENABLE) {
					count++;
				}
				npd_put_vlan(vlanNode);
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &count);
#endif
	return reply;
}

/**********************************************************************************
 * npd_dbus_dldp_get_product_id
 *		get product id 
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			NPD_DLDP_RETURN_CODE_OK				- success
 *			NPD_DLDP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_get_product_id
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusError err;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;

	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned int productId = 0; 

	dbus_error_init(&err);

	productId = PRODUCT_ID;

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &productId);
	return reply;
}

/**********************************************************************************
 * npd_dbus_dldp_exchange_ifindex_to_slotport
 *		get slot_no/port_no by ethportindex
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			NPD_DLDP_RETURN_CODE_OK				- success
 *			NPD_DLDP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_exchange_ifindex_to_slotport
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	DBusError err;
	
	unsigned int ret = NPD_DLDP_RETURN_CODE_OK;
	unsigned char slotNo = NPD_DLDP_INIT_0;
	unsigned char localPortNo = NPD_DLDP_INIT_0;
	unsigned int slotIndex = NPD_DLDP_INIT_0;
	unsigned int portIndex = NPD_DLDP_INIT_0;
	unsigned int eth_g_index = NPD_DLDP_INIT_0;
	
	syslog_ax_dldp_dbg("dldp convert eth_g_index to slot/port\n");

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_dldp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dldp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_dldp_dbg("get param: eth_port_index %d.\n", eth_g_index);

	slotIndex = SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	slotNo = CHASSIS_SLOT_INDEX2NO(slotIndex);
	portIndex = ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	localPortNo = ETH_LOCAL_INDEX2NO(slotIndex, portIndex);
	if (CHASSIS_SLOTNO_ISLEGAL(slotNo)) {
		if (ETH_LOCAL_PORTNO_ISLEGAL(slotNo,localPortNo)) {
			ret = NPD_DLDP_RETURN_CODE_OK;
		}
		else {
			 syslog_ax_dldp_err("illegal slot no!\n");
			ret = NPD_DLDP_RETURN_CODE_ERROR;
		}
	}
	else {
		 syslog_ax_vlan_err("illegal port no!\n");
		ret = NPD_DLDP_RETURN_CODE_ERROR;
	}

	syslog_ax_dldp_dbg("retval %x slot_no %d port_no %d.\n",
						ret, slotNo, localPortNo);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &slotNo);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &localPortNo);
	return reply;
}

/**********************************************************************************
 * npd_dbus_dldp_vlan_show_running_config
 *		DLDP show running config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			NPD_DLDP_RETURN_CODE_OK				- success
 *			NPD_DLDP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dldp_vlan_show_running_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;

	showStr = (unsigned char*)malloc(NPD_DLDP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dldp_err("DLDP show running config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DLDP_RUNNING_CFG_MEM);

	/*save DLDP cfg*/
	npd_dldp_save_vlan_cfg(showStr, NPD_DLDP_RUNNING_CFG_MEM, &en_dis);
	syslog_ax_dldp_dbg("DLDP service %s\n", en_dis ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

int npd_dldp_msg_handler(char *msg, int len)
{
	unsigned int ret = NPD_DLDP_INIT_0;
	NPD_MSG_FROM_DLDP_T *dldp_msg = (NPD_MSG_FROM_DLDP_T *)msg;
	NPD_DLDP_NOTIFY_MOD_NPD_T *notify_msg = NULL;

	if (NPD_DLDP_TYPE_NOTIFY_MSG == dldp_msg->nlh.nlmsg_type)
	{
		if (NPD_DLDP_FLAG_ADDR_MOD == dldp_msg->nlh.nlmsg_flags)
		{
			notify_msg = (NPD_DLDP_NOTIFY_MOD_NPD_T *)&(dldp_msg->dldp_notify_msg);
			ret = npd_dldp_recvmsg_proc(notify_msg);
		}
	}
	return 0;
}

/**********************************************************************************
*npd_dldp_mng_thread_dgram()
*	DESCRIPTION:
*		DLDP command message handle thread

*	INPUTS:
*		NULL
*
*	OUTPUTS:
*		NULL
*
*	RETURN VALUE:
*		NULL
*
***********************************************************************************/
int npd_dldp_msg_init
(
	void
)
{
	int sock = NPD_DLDP_INIT_FD;

	npd_dldp_init();

	/* create socket communication		*/
	if (0 > (sock = npd_dldp_sock_init()))
	{
		syslog_ax_dldp_err("Failed to create npd to dldp msg socket fd %d\n", sock);
		return -1;
	}
	
	dldp_fd = sock;
	syslog_ax_dldp_dbg("Create npd to dldp msg socket (%d) ok.\n", sock);
	
    npd_app_msg_socket_register(sock, "dldpMsg", npd_dldp_msg_handler, NPD_MSG_FROM_DLDP_MAX_SIZE);
    return 0;
}

#ifdef __cplusplus
}
#endif
#endif

