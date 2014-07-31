/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
*CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*   	APIs used in NPD for igmp snooping process.
*
* DATE:
*  		03/01/2010	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.40 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAVE_IGMP_SNP
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/protocol/igmp_api.h"
#include "npd_igmp_snp_com.h"

/*include header files begin */

/*MACRO definition begin */
#define	IGMP_SNOOP_NPD_MSG_SOCK	"/tmp/igmp_snp_npdmng.sock"		/*command¢ssocket*/
#define	IGMP_MSG_MAX_SIZE		sizeof(struct igmp_msg_npd)		/*modify by wujh 08/09/23,original=1248*/
#define CPSS_IP_PROTOCOL_IPV4_E 0
#define CPSS_BRG_IPM_GV_E		1
/*MACRO definition end */


/*external variables reference begin */
/*external variables reference end */

/*external function declarations begin*/

/*external function declarations begin*/

/*local variables definition begin */

/*l2mc_list	*l2mc_member_list = NULL;*/
int		igmp_fd = 0;
unsigned char transMac[MAC_ADDR_LEN] = {0};

extern hash_table_index_t *npd_mroute_hwid_hash_index;

extern hash_table_index_t *npd_mroute_haship_index;

array_table_index_t *npd_igmpsnp_cfg_array_index = NULL;

db_table_t         *npd_igmpsnp_cfgtbl = NULL;

tbl_index_t *npd_l2_mc_index = NULL;
tbl_index_t *npd_l3_mc_index = NULL;
    
/*local variables definition end */
struct	sockaddr_un 	npd_igmpsnp_addr;   /*local addr*/	
struct	sockaddr_un		igmpsnp_addr;   	/*remote addr*/

#ifdef HAVE_MLD_SNP
array_table_index_t *npd_mldsnp_cfg_array_index = NULL;
db_table_t         *npd_mldsnp_cfgtbl = NULL;
struct  sockaddr_un		mldsnp_addr;   	/*remote addr*/
#endif

extern int nam_igmp_snp_init();

netif_event_notifier_t npd_igmpsnp_netif_notifier =
{
    .netif_event_handle_f = &npd_igmpsnp_notify_event,
    .netif_relate_handle_f = &npd_igmpsnp_relate_event
};

void l2_mc_index_alloc(unsigned int *index)
{
    nam_index_alloc(npd_l2_mc_index, index);
    return;
}

void l2_mc_index_free(unsigned int index)
{
    nam_index_free(npd_l2_mc_index, index);
    return;
}

void l3_mc_index_alloc(unsigned int *index)
{
    nam_index_alloc(npd_l3_mc_index, index);
    return;
}

void l3_mc_index_free(unsigned int index)
{
    nam_index_free(npd_l3_mc_index, index);
    return;
}

void l2_mc_index_get(unsigned int index)
{
	nam_index_get(npd_l2_mc_index, index);
	return;
}

void l3_mc_index_get(unsigned int index)
{
    nam_index_get(npd_l3_mc_index, index);
}
/*******************************************************************************
 * npd_igmp_snp_sock_init
 *
 * DESCRIPTION:
 *   		create socket communication with igmpsnp
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	ser_sock - socket number
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   - create successfully 
 * 		IGMPSNP_RETURN_CODE_ERROR	- create socket or bind error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int	npd_igmp_snp_sock_init(int *ser_sock)
{
	int recvBufSize = 256*1024;
	memset(&igmpsnp_addr,0,sizeof(igmpsnp_addr));
	memset(&npd_igmpsnp_addr,0,sizeof(npd_igmpsnp_addr));
#ifdef HAVE_MLD_SNP
    memset(&mldsnp_addr,0,sizeof(mldsnp_addr));
#endif
	if((*ser_sock = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		npd_syslog_err("create npd to igmp socket fail\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	if( setsockopt(*ser_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&recvBufSize,sizeof(int)) != 0)
	{
		npd_syslog_err("reset npd igmp socket recv buffer size failed, err:%d\n", errno);
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	npd_igmpsnp_addr.sun_family = AF_LOCAL;
	strcpy(npd_igmpsnp_addr.sun_path,"/tmp/npd2igmpSnp_server");
	unlink(npd_igmpsnp_addr.sun_path);

	igmpsnp_addr.sun_family = AF_LOCAL;
	strcpy(igmpsnp_addr.sun_path, "/tmp/npd2igmpSnp_client");
#ifdef HAVE_MLD_SNP
    mldsnp_addr.sun_family = AF_LOCAL;
    strcpy(mldsnp_addr.sun_path, "/tmp/npd2mldSnp_server");
#endif
    
	if(bind(*ser_sock , (struct sockaddr *)&npd_igmpsnp_addr, sizeof(npd_igmpsnp_addr)) == -1) 
	{
		npd_syslog_err("npd to igmp snooping socket created but failed when bind\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	chmod(npd_igmpsnp_addr.sun_path, 0777);
	return IGMPSNP_RETURN_CODE_OK;	
	
}

/*******************************************************************************
 * npd_cmd_sendto_igmpsnp
 *
 * DESCRIPTION:
 *   		npd commad send to igmpsnp 
 *
 * INPUTS:
 * 		mngCmd - npd commad message
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - send successfully		
 *		IGMPSNP_RETURN_CODE_ERROR - send fail when use sendto 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - malloc memory fail
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/

 int npd_cmd_sendto_igmpsnp
(
	struct npd_dev_event_cmd* mngCmd	
)
{
	syslog_ax_igmp_dbg("npd msg sendto igmp-snoop\n");
	struct npd_mng_igmp*	devMsg = NULL;
	dev_notify_msg*		notify_format = NULL;
	unsigned int		cmdLen = sizeof(struct npd_mng_igmp);
	int	rc,byteSend = 0;

    if(igmp_fd == 0)
        return -1;

	
	devMsg = (struct npd_mng_igmp* )malloc(sizeof(struct npd_mng_igmp));
	if(NULL == devMsg){
		syslog_ax_igmp_dbg("devMsg memory allocation Fail!\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	memset(devMsg,0,sizeof(struct npd_mng_igmp));
	
	notify_format = &(devMsg->npd_dev_mng);
	devMsg->nlh.nlmsg_len = sizeof(struct npd_mng_igmp);
	syslog_ax_igmp_dbg("IGMP Snooping devMsg->nlh.nlmsg_len %d\n,sizeof(struct nlmsghdr) %d\n, sizeof(struct dev_notify_mng) %d.\n",
								devMsg->nlh.nlmsg_len,sizeof(struct nlmsghdr),sizeof(dev_notify_msg));
	devMsg->nlh.nlmsg_type = IGMP_SNP_TYPE_DEVICE_EVENT;
	devMsg->nlh.nlmsg_flags = IGMP_SNP_FLAG_ADDR_MOD;/*can be ignored*/
	notify_format->event = mngCmd->event_type;/*port down/up,EVENT_DEV_UNREGISTER..*/
	notify_format->vlan_id = mngCmd->vid;
	notify_format->ifindex = mngCmd->port_index;

	if(NPD_IGMPSNP_EVENT_DEV_SYS_MAC_NOTI == notify_format->event){
		memcpy(notify_format->sys_mac,transMac,MAC_ADDR_LEN);
	}

	
	while(cmdLen != byteSend)
	{
        rc = sendto(igmp_fd,( char* )devMsg,sizeof(struct npd_mng_igmp),0,
							(struct sockaddr *)&igmpsnp_addr, sizeof(igmpsnp_addr));
		if(rc < 0)
		{
			if(errno == EINTR)/*send() be interrupted.*/
			{
				npd_syslog_dbg("sendto");
				continue;
			}
			else if(errno == EACCES)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto");
				printf("sendto() permision denied.\n");
				break;
			}
			else if(errno == EWOULDBLOCK)/*send()*/
			{
				npd_syslog_dbg("sendto1");
				break;
			}
			else if(errno == EBADF)
			{
				npd_syslog_dbg("sendto2");
				break;
			}
			else if(errno == ECONNRESET)
			{
				npd_syslog_dbg("sendto3");
				break;
			}
			else if(errno == EDESTADDRREQ)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto4");
				break;
			}
			else if(errno == EFAULT)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto5");
				break;
			}
			else if(errno == EINVAL)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto6");
				break;
			}
			else if(errno == EISCONN)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto7");
				break;
			}
			else if(errno == EMSGSIZE)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto8");
				break;
			}
			else if(errno == ENOBUFS)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto9");
				break;
			}
			else if(errno == ENOMEM)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto10");
				break;
			}
			else if(errno == ENOTCONN)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto11");
				break;
			}
			else if(errno == ENOTSOCK)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto12");
				break;
			}
			else if(errno == EOPNOTSUPP)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto13");
				break;
			}
			else if(errno == EPIPE)/*send() permission is denied on the destination socket file*/
			{
				npd_syslog_dbg("sendto14");
				break;
			}
			else {
				npd_syslog_dbg("sendto15");
				syslog_ax_igmp_dbg("IgmpSnpCmd Write To IGMP::Write Fail.\n");
				return IGMPSNP_RETURN_CODE_ERROR;
			}
		}
		byteSend += rc;
	}
	if(byteSend == cmdLen) {
		rc = IGMPSNP_RETURN_CODE_OK;	
	}
	free(devMsg);
	return rc;
}

int npd_igmp_cmd_send(unsigned long type, unsigned short vid, unsigned int ifindex)
{	
	struct npd_dev_event_cmd npdMngMsg;
	
	npdMngMsg.event_type = type;
	npdMngMsg.port_index = ifindex;
	npdMngMsg.vid = vid;

	return npd_cmd_sendto_igmpsnp(&npdMngMsg);
}

/*******************************************************************************
 * npd_igmpsnp_recv_info
 *
 * DESCRIPTION:
 *   		use recvfrom to receive information from igmp 
 *
 * INPUTS:
 *		infoLen - the receive max size
 *
 * OUTPUTS:
 * 		msg - pointer to igmp message get from igmp
 *		len - actual receive the data size
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - complete the receive
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int npd_igmpsnp_recv_info
(
	struct igmp_msg_npd *msg,
	unsigned int  infoLen,
	int *len
)
{
	unsigned int addrLen = sizeof(igmpsnp_addr);
	while(1)
	{
		*len = recvfrom(igmp_fd, (char*)msg, infoLen, 0,(struct sockaddr *)&igmpsnp_addr, &addrLen);
		if(*len < 0 && errno == EINTR) 
		{
			continue;
		}
		break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * npd_igmp_snp_set_status
 *
 * DESCRIPTION:
 *   		set the global status is enable or disable
 *
 * INPUTS:
 *		null
 *
 * OUTPUTS:
 *    	status - output the status
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the status successfully
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/

int npd_igmp_snp_set_status(unsigned char status)
{
	struct npd_igmpsnp_cfg_s npdIgmpCfg;

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
	npdIgmpCfg.npdIgmpSnpEnDis = status;
	dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg, &npdIgmpCfg);

	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * npd_check_igmp_snp_status
 *
 * DESCRIPTION:
 *   		check the global status is enable or disable
 *
 * INPUTS:
 *		null
 *
 * OUTPUTS:
 *    	status - output the status
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the status successfully
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/

int npd_check_igmp_snp_status(unsigned char *status)
{
	struct npd_igmpsnp_cfg_s npdIgmpCfg;

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
	*status = npdIgmpCfg.npdIgmpSnpEnDis;

	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * npd_check_igmp_snp_vlan_status
 *
 * DESCRIPTION:
 *   		check the vlan status is enable or disable
 *
 * INPUTS:
 *		vlanId - vlan id
 *
 * OUTPUTS:
 *    	status - output the port status
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the status successfully
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - the vlan not exist
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/

int npd_check_igmp_snp_vlan_status(unsigned short vlanId,unsigned char *status)
{
	struct vlan_s *vlanNode = NULL;
	vlanNode = npd_find_vlan_by_vid(vlanId);
	if((NULL == vlanNode)||(vlanNode->isAutoCreated))
	{
		if(vlanNode)
		    free(vlanNode);
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

    struct npd_igmpsnp_cfg_s npdIgmpCfg;
	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);

	*status = NPD_VBMP_MEMBER(npdIgmpCfg.vlan_admin_status, vlanId);
	free( vlanNode );

	return IGMPSNP_RETURN_CODE_OK;
}


/*******************************************************************************
 * npd_igmp_snp_enable
 *
 * DESCRIPTION:
 *   		when set igmp enable
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - can find the vlan by vlanId
 *		IGMPSNP_RETURN_CODE_OK - delete the FDB entries success
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/
unsigned int npd_igmp_snp_enable()
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	
	npd_igmp_snp_set_status(TRUE);
	npd_igmp_sysmac_notifer();
	
	return ret;
}


/*******************************************************************************
 * npd_igmp_snp_disable
 *
 * DESCRIPTION:
 *   		when set igmp disable ,delete all FDB entries by  vlanId
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - can find the vlan by vlanId
 *		IGMPSNP_RETURN_CODE_OK - delete the FDB entries success
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/
unsigned int npd_igmp_snp_disable()
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	
	npd_igmp_snp_set_status(FALSE);
	
	return ret;
}

#ifdef HAVE_MLD_SNP
int npd_cmd_sendto_mldsnp
(
	struct npd_dev_event_cmd* mngCmd	
)
{
	syslog_ax_igmp_dbg("npd msg sendto mld-snoop\n");
	struct npd_mng_igmp*	devMsg = NULL;
	dev_notify_msg*		notify_format = NULL;
	unsigned int		cmdLen = sizeof(struct npd_mng_igmp);
	int	rc,byteSend = 0;

    if(igmp_fd == 0)
        return -1;

	
	devMsg = (struct npd_mng_igmp* )malloc(sizeof(struct npd_mng_igmp));
	if(NULL == devMsg){
		syslog_ax_igmp_dbg("devMsg memory allocation Fail!\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	memset(devMsg,0,sizeof(struct npd_mng_igmp*));
	
	notify_format = &(devMsg->npd_dev_mng);
	devMsg->nlh.nlmsg_len = sizeof(struct npd_mng_igmp);
	syslog_ax_igmp_dbg("MLD Snooping devMsg->nlh.nlmsg_len %d\n,sizeof(struct nlmsghdr) %d\n, sizeof(struct dev_notify_mng) %d.\n",
								devMsg->nlh.nlmsg_len,sizeof(struct nlmsghdr),sizeof(dev_notify_msg));
	devMsg->nlh.nlmsg_type = IGMP_SNP_TYPE_DEVICE_EVENT;
	devMsg->nlh.nlmsg_flags = IGMP_SNP_FLAG_ADDR_MOD;/*can be ignored*/
	notify_format->event = mngCmd->event_type;/*port down/up,EVENT_DEV_UNREGISTER..*/
	notify_format->vlan_id = mngCmd->vid;
	notify_format->ifindex = mngCmd->port_index;

	if(NPD_IGMPSNP_EVENT_DEV_SYS_MAC_NOTI == notify_format->event){
		memcpy(notify_format->sys_mac,transMac,MAC_ADDR_LEN);
	}

	
	while(cmdLen != byteSend)
	{
        rc = sendto(igmp_fd,( char* )devMsg,sizeof(struct npd_mng_igmp),0,
							(struct sockaddr *)&mldsnp_addr, sizeof(mldsnp_addr));
		if(rc < 0)
		{
			if(errno == EINTR)/*send() be interrupted.*/
			{
				npd_syslog_dbg("sendto");
				continue;
			}			
			else {
				npd_syslog_dbg("MldSnpCmd sendto fail: %s\n", strerror(errno));
				return IGMPSNP_RETURN_CODE_ERROR;
			}
		}
		byteSend += rc;
	}
	if(byteSend == cmdLen) {
		rc = IGMPSNP_RETURN_CODE_OK;	
	}
	free(devMsg);
	return rc;
}

int npd_mld_cmd_send(unsigned long type, unsigned short vid, unsigned int ifindex)
{	
	struct npd_dev_event_cmd npdMngMsg;
	
	npdMngMsg.event_type = type;
	npdMngMsg.port_index = ifindex;
	npdMngMsg.vid = vid;

	return npd_cmd_sendto_mldsnp(&npdMngMsg);
}

int npd_check_mld_snp_status(unsigned char *status)
{
	struct npd_mldsnp_cfg_s npdMldCfg;

	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
	*status = npdMldCfg.npdMldSnpEnDis;

	return MLDSNP_RETURN_CODE_SUCCESS;
}

int npd_check_mld_snp_vlan_status(unsigned short vlanId,unsigned char *status)
{
    struct npd_mldsnp_cfg_s npdMldCfg;

	if(TRUE != npd_check_vlan_exist(vlanId))
	{
		return MLDSNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);

	*status = NPD_VBMP_MEMBER(npdMldCfg.vlan_admin_status, vlanId);

	return MLDSNP_RETURN_CODE_SUCCESS;
}

int npd_mld_snp_set_status(unsigned char status)
{
	struct npd_mldsnp_cfg_s npdMldCfg;

	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
	npdMldCfg.npdMldSnpEnDis = status;
	dbtable_array_update(npd_mldsnp_cfg_array_index, 0, &npdMldCfg, &npdMldCfg);

	return IGMPSNP_RETURN_CODE_OK;
}

int npd_mld_sysmac_notifer(void)
{
    memcpy(transMac, PRODUCT_MAC_ADDRESS,6);
    syslog_ax_igmp_dbg("Packet etherHeader sMac =%02x:%02x:%02x:%02x:%02x:%02x.\n",\
    	transMac[0],transMac[1],transMac[2],transMac[3],transMac[4],transMac[5]);

    return npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_SYS_MAC_NOTI, 0, 0);
}

unsigned int npd_mld_snp_enable()
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	
	npd_mld_snp_set_status(TRUE);
	npd_mld_sysmac_notifer();
	
	return ret;
}

unsigned int npd_mld_snp_disable()
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	
	npd_mld_snp_set_status(FALSE);
		
	return ret;
}

int npd_mld_port_vlan_relate_change
(
	unsigned short vlan_id,	 
	unsigned int netif_index,
	enum PORT_RELATE_ENT event
)
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
    unsigned char vlanstatus = 0;
    int status = 0;
    int type = npd_netif_type_get(netif_index);

	ret = npd_check_mld_snp_vlan_status(vlan_id,&vlanstatus);
    if(IGMPSNP_RETURN_CODE_OK != ret)
        return ret;
    if(0 == vlanstatus)
        return ret;

    struct npd_mldsnp_cfg_s npdMldCfg;
	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
    int switch_index = netif_array_index_from_ifindex(netif_index);

    if( PORT_NOTIFIER_LEAVE == event){
		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vlan_id, netif_index);
        npdMldCfg.switch_port_control_count[switch_index]--;
    }
    if(PORT_NOTIFIER_JOIN == event){
        ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vlan_id, netif_index);
        npdMldCfg.switch_port_control_count[switch_index]++;
    }

    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        unsigned int trunk_id = npd_netif_trunk_get_tid(netif_index);
        int all_port;
        struct trunk_s node = {0};
        npd_find_trunk(trunk_id, &node);
 		NPD_PBMP_ITER(node.ports, all_port)
 		{
            npdMldCfg.switch_port_control_count[all_port] = npdMldCfg.switch_port_control_count[switch_index];
        }
    }

    dbtable_array_update(npd_mldsnp_cfg_array_index, 0, NULL, &npdMldCfg);    
    return ret;
}

long npd_mld_snp_cfg_handle_insert(void * newdata)
{
	struct npd_mldsnp_cfg_s *newtb = newdata;
    int i = 0;
    unsigned int vlanId, port_ifindex;
    port_driver_t *driver = NULL;

    if(!newtb->npdMldSnpEnDis)
        return 0;

    NPD_VBMP_ITER(newtb->vlan_admin_status, vlanId)
    {
		nam_set_mld_enable(vlanId, TRUE);
        //nam_set_igmp_enable(vlanId, TRUE);
    }    

    for(i=0; i<MAX_SWITCHPORT_PER_SYSTEM; i++)
    {
        if(newtb->switch_port_control_count[i] > 0)
        {
            port_ifindex = netif_array_index_to_ifindex(i);
            driver = port_driver_get(port_ifindex);
//          if(NULL == driver || NULL == driver->mld_trap_set)
            if(NULL == driver || NULL == driver->igmp_trap_set)
                continue;
            
//            (*driver->mld_trap_set)(0, port_ifindex, TRUE);
            (*driver->igmp_trap_set)(0, port_ifindex, TRUE);
        }
    }

    return 0;
}

long npd_mld_snp_cfg_handle_update(void * newdata, void *olddata)
{
	struct npd_mldsnp_cfg_s *newtb = newdata;
    struct npd_mldsnp_cfg_s *oldtb = olddata;
    int i = 0;
    unsigned int vlanId, port_ifindex;
    npd_vbmp_t tmpVlan;
    port_driver_t *driver = NULL;

    if(newtb->npdMldSnpEnDis != oldtb->npdMldSnpEnDis)
    {
        NPD_VBMP_ITER(newtb->vlan_admin_status, vlanId)
        {
	   		nam_set_mld_enable(vlanId, newtb->npdMldSnpEnDis);
            //nam_set_igmp_enable(vlanId, newtb->npdMldSnpEnDis);
        }

        for(i=0; i<MAX_SWITCHPORT_PER_SYSTEM; i++)
        {
            if(newtb->switch_port_control_count[i] > 0)
            {
                port_ifindex = netif_array_index_to_ifindex(i);
                driver = port_driver_get(port_ifindex);
                if(NULL == driver || NULL == driver->mld_trap_set)
                //if(NULL == driver || NULL == driver->igmp_trap_set)
                    continue;
                
                (*driver->mld_trap_set)(0, port_ifindex, newtb->npdMldSnpEnDis);
                //(*driver->igmp_trap_set)(0, port_ifindex, newtb->npdMldSnpEnDis);
            }
        }
        return 0;
    }

    if(!newtb->npdMldSnpEnDis)
        return 0;

    if(NPD_VBMP_NEQ(newtb->vlan_admin_status, oldtb->vlan_admin_status))
    {
        NPD_VBMP_ASSIGN(tmpVlan, newtb->vlan_admin_status);
        NPD_VBMP_XOR(tmpVlan, oldtb->vlan_admin_status);
        NPD_VBMP_ITER(tmpVlan, vlanId)
        {
            if(NPD_VBMP_MEMBER(newtb->vlan_admin_status, vlanId))
            {
				nam_set_mld_enable(vlanId, TRUE);
        		//nam_set_igmp_enable(vlanId, TRUE);
            }
            else
            {
				nam_set_mld_enable(vlanId, FALSE);
                //nam_set_igmp_enable(vlanId, FALSE);
            }
        }
    }

    for(i=0; i < MAX_SWITCHPORT_PER_SYSTEM; i++)
    {
        if((newtb->switch_port_control_count[i] > 0 && oldtb->switch_port_control_count[i] == 0)
            || (newtb->switch_port_control_count[i] == 0 && oldtb->switch_port_control_count[i] > 0))
        {
            port_ifindex = netif_array_index_to_ifindex(i);
            driver = port_driver_get(port_ifindex);
            if(NULL == driver || NULL == driver->mld_trap_set)
            //if(NULL == driver || NULL == driver->igmp_trap_set)
                continue;
            
            if(newtb->switch_port_control_count[i] > 0)
            {
                (*driver->mld_trap_set)(0, port_ifindex, TRUE);
                //(*driver->igmp_trap_set)(0, port_ifindex, TRUE);
            }
            else
            {
                (*driver->mld_trap_set)(0, port_ifindex, FALSE);
                //(*driver->igmp_trap_set)(0, port_ifindex, FALSE);
            }
        }
    }

    return 0;
}

int npd_vlan_mld_snp_endis_config
(
	unsigned char	isAdd,
	unsigned short	vid
)
{
	syslog_ax_igmp_dbg("Enter npd_vlan_mld_snp_endis_config: vid %d endis %d\n", vid, isAdd);
	unsigned char enDis = isAdd, status = 0;
	unsigned int ret = MLDSNP_RETURN_CODE_SUCCESS;
	struct vlan_s *vlanNode = NULL;
    struct npd_mldsnp_cfg_s npdMldCfg;
    int port_ifindex;
    int all_port;
    struct trunk_s node = {0};
    unsigned int trunk_id;
    int type;
    
	/*check vlan exist.*/
	vlanNode = npd_find_vlan_by_vid(vid);
	if (NULL == vlanNode) {
		return MLDSNP_RETURN_CODE_VLAN_NOTEXIST;/*can NOT directed Return NPD_VLAN_BADPARAM.*/
	}

	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);

	if(enDis){
		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_REGISTER, vid, 0);
        NPD_VBMP_VLAN_ADD(npdMldCfg.vlan_admin_status, vid);
	}
	else{
		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_UNREGISTER, vid, 0);
        NPD_VBMP_VLAN_REMOVE(npdMldCfg.vlan_admin_status, vid);
	}

	/*clear igmp on all-ports member of vlan*/
    int switch_port;
    NPD_PBMP_ITER(vlanNode->untag_ports, switch_port)
    {            	
        port_ifindex = netif_array_index_to_ifindex(switch_port);
        if(enDis){
    		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vid, 
                port_ifindex);					
            npdMldCfg.switch_port_control_count[switch_port]++;
    	}
    	else{
    		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vid, 
                port_ifindex);					
            npdMldCfg.switch_port_control_count[switch_port]--;
    	}	

        type = npd_netif_type_get(port_ifindex);
        if(NPD_NETIF_TRUNK_TYPE == type)
        {
            trunk_id = npd_netif_trunk_get_tid(port_ifindex);
            npd_find_trunk(trunk_id, &node);
        	NPD_PBMP_ITER(node.ports, all_port)
        	{
                npdMldCfg.switch_port_control_count[all_port] = npdMldCfg.switch_port_control_count[switch_port];
            }
        }
    }

    NPD_PBMP_ITER(vlanNode->tag_ports, switch_port)
    {            	
        port_ifindex = netif_array_index_to_ifindex(switch_port);
        if(enDis){
    		ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vid, 
                port_ifindex);
            npdMldCfg.switch_port_control_count[switch_port]++;
    	}
    	else{
            ret = npd_mld_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vid, 
                port_ifindex);
            npdMldCfg.switch_port_control_count[switch_port]--;
    	}
        
        type = npd_netif_type_get(port_ifindex);
        if(NPD_NETIF_TRUNK_TYPE == type)
        {
            trunk_id = npd_netif_trunk_get_tid(port_ifindex);
            npd_find_trunk(trunk_id, &node);
        	NPD_PBMP_ITER(node.ports, all_port)
        	{
                npdMldCfg.switch_port_control_count[all_port] = npdMldCfg.switch_port_control_count[switch_port];
            }
        }
    }
    
    dbtable_array_update(npd_mldsnp_cfg_array_index, 0, NULL, &npdMldCfg);
    
    sleep(1);
	
	free(vlanNode);
	return ret;
}

DBusMessage * npd_dbus_mldsnp_config_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vid;
	unsigned int isEnable = 0,ret = IGMPSNP_RETURN_CODE_OK;
	unsigned char	status = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID))) {
		 syslog_ax_igmp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_igmp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = npd_check_mld_snp_status(&status);
	if(0 == status ){
		syslog_ax_igmp_err("MLD Snooping NOT Enabled Global.");
		ret = MLDSNP_RETURN_CODE_DISABLED;
	}
	else{
		ret = npd_check_mld_snp_vlan_status(vid,&status);
		if(MLDSNP_RETURN_CODE_SUCCESS != ret){
			 syslog_ax_igmp_err("vlan %d not created.",vid);
		}
		else if(0 == isEnable &&0 == status ){
			ret = MLDSNP_RETURN_CODE_VLAN_DISABLED;
			 syslog_ax_igmp_err("vlan %d not enabled MLD Snooping.",vid);
		}
		else if(1== isEnable && 1== status){
			ret = MLDSNP_RETURN_CODE_VLAN_ENABLED;
			 syslog_ax_igmp_err("vlan %d already enabled MLD Snooping.",vid);
		}
		else{
			 syslog_ax_igmp_dbg("get vid = %d ,isEnable = %d",vid,isEnable);
			ret = npd_vlan_mld_snp_endis_config(isEnable,vid);
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

#endif

extern unsigned int npd_mroute_compar_ip_vlan
(
	void *data1,
	void *data2
);
/*******************************************************************************
 * npd_igmp_snp_recvmsg_proc
 *
 * DESCRIPTION:
 *		 config dev according to running result of igmp snoop protocol
 *
 * INPUTS:
 * 		igmp_notify - notify message
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - config success
 *
 * COMMENTS:
 *  
 **
 ********************************************************************************/
int npd_igmp_snp_recvmsg_proc(struct igmp_notify_mod_npd* igmp_notify)
{
	unsigned int ret = 0;
	unsigned int igmpSysSet =0;/*fdb_entry_exist = 0;*/
	struct npd_mroute_item_s igmpSnpItem, dupItem, nextItem;
	unsigned int swPortIndex = 0;

	memset( &igmpSnpItem, 0, sizeof(struct npd_mroute_item_s));
	memset( &dupItem, 0, sizeof(struct npd_mroute_item_s));
	memset( &nextItem, 0, sizeof(struct npd_mroute_item_s));
	
	igmpSnpItem.dst_vid = igmp_notify->vlan_id;
    igmpSnpItem.dstl3_netif_index = npd_netif_vlan_get_index(igmp_notify->vlan_id);
    igmpSnpItem.sip = igmp_notify->source;	
    igmpSnpItem.srcl2_netif_index = -1;
    igmpSnpItem.srcl3_g_index = -1;
    igmpSnpItem.srcl3_netif_index = -1;
	igmpSnpItem.dip = (unsigned int)igmp_notify->groupadd;
	igmpSnpItem.flag |= NPD_ROUTE_VALID;
	igmpSnpItem.family = AF_INET;
#ifdef HAVE_NPD_IPV6
    IPV6_ADDR_COPY(&igmpSnpItem.dipv6, &igmp_notify->grpaddr);
    IPV6_ADDR_COPY(&igmpSnpItem.sipv6, &igmp_notify->srcaddr);
#endif
	
	syslog_ax_igmp_dbg("npd receive message from igmp_snp:get igmpMsg:type %d, vlan_id %d, port 0x%x group_ip 0x%x, %s %d.\n",
						igmp_notify->mod_type, igmp_notify->vlan_id, igmp_notify->ifindex,igmp_notify->groupadd,
						(IGMP_SYS_SET ==igmp_notify->mod_type)?"reserve":"vidx",
						igmp_notify->reserve);
    npd_key_database_lock();
#ifdef HAVE_ROUTE    
    npd_mroute_lock();
    dbtable_hash_lock(npd_mroute_hwid_hash_index);
#endif
	switch(igmp_notify->mod_type)
	{
		case IGMP_ADDR_ADD:			
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);            
            ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
            if(0 != ret)
            {
                igmpSnpItem.rt_type = RTN_L2MULTICAST;
                igmpSnpItem.svid = igmp_notify->vlan_id;
                igmpSnpItem.srcl3_netif_index = npd_netif_vlan_get_index(igmp_notify->vlan_id);
				NPD_PBMP_PORT_ADD(igmpSnpItem.dst_l2_ports, swPortIndex);
				NPD_PBMP_PORT_ADD(igmpSnpItem.l2_real_ports, swPortIndex);
				if(igmpSnpItem.sip != 0)
				{
					dupItem = igmpSnpItem;
                    dupItem.sip = 0;
        			ret = dbtable_hash_search(npd_mroute_haship_index, &dupItem, NULL, &dupItem);
                    if (0 == ret) /*imports ports from (*,G) into (S,G)*/
                    {
        				NPD_PBMP_OR(igmpSnpItem.dst_l2_ports, dupItem.dst_l2_ports);
                    }
				}
				if(0 != npd_mroute_nexthop_alloc(&igmpSnpItem))
				{
					npd_syslog_route_err("Igmp alloc nexthop fail, mroute dip %x dstvid %d, mcindex %d", 
	                     igmpSnpItem.dip,
	                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);					
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
				if(0 != dbtable_hash_insert(npd_mroute_haship_index, &igmpSnpItem))
				{
					npd_syslog_route_err("Igmp fail to add port %x mroute dip %x dstvid %d, mcindex %d", 
																	igmp_notify->ifindex, igmpSnpItem.dip,
                     	 										    igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);					
					npd_mroute_nexthop_check_free(&igmpSnpItem);
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
                npd_syslog_route_dbg("Igmp add port %x mroute dip %x dstvid %d, mcindex %d", 
                     igmp_notify->ifindex, igmpSnpItem.dip,
                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
			}
			else
			{
            	/*if found, re-just entry rt_type and add ports*/
                if(dupItem.rt_type == RTN_MULTICAST)
                    dupItem.rt_type = RTN_L2L3MULTICAST;
    			NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
               	NPD_PBMP_PORT_ADD(dupItem.l2_real_ports, swPortIndex);
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem);
			}
			if(igmpSnpItem.sip == 0)
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
	            {
	                if(dupItem.sip == 0) 
						continue;
					NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
					dbtable_hash_update(npd_mroute_haship_index, NULL, &dupItem);
	                npd_syslog_route_dbg("Igmp update add port %x mroute dip %x dstvid %d", 
						                    igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
	            }
			}
			break;			
		case IGMP_ADDR_DEL:
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
			if(0 != ret)
			{
				break;	/*if entry not found, just break out*/
			}
			else if(!NPD_PBMP_MEMBER(dupItem.dst_l2_ports, swPortIndex))
			{
				ret = IGMPSNP_RETURN_CODE_OK;
				goto recv_end;
			}

			NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports,swPortIndex);
			NPD_PBMP_PORT_REMOVE(dupItem.l2_real_ports,swPortIndex);
			if(dupItem.rt_type == RTN_L2MULTICAST && NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
			{
				dbtable_hash_delete(npd_mroute_haship_index, &dupItem, NULL);
				npd_syslog_route_dbg("Igmp del port %x l2mc dip %x dstvid %d", igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				npd_mroute_nexthop_check_free(&dupItem);	
			}
			else
			{
				if(dupItem.rt_type == RTN_L2L3MULTICAST && NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
					dupItem.rt_type = RTN_MULTICAST;
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
				npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
			}				

			if (igmpSnpItem.sip == 0)
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
				{
					if(dupItem.sip == 0 || NPD_PBMP_MEMBER(dupItem.l2_real_ports, swPortIndex))
					{
						continue;
					}
					NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports,swPortIndex);
					dbtable_hash_update(npd_mroute_haship_index, NULL, &dupItem);
                    npd_syslog_route_dbg("Igmp update del port %x mroute dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				}
			}
			break;
		case IGMP_ADDR_RMV:			
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
			if(0 != ret)
			{
				break;	/*if entry not found, just break out*/
			}
			
			if(dupItem.sip == 0)
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
	 			{
					if(igmpSnpItem.sip == 0) {
						continue;
					}
					NPD_PBMP_REMOVE(igmpSnpItem.dst_l2_ports, dupItem.dst_l2_ports);
					NPD_PBMP_OR(igmpSnpItem.dst_l2_ports, dupItem.l2_real_ports);
                    dbtable_hash_update(npd_mroute_haship_index, NULL, &igmpSnpItem);                    
                    npd_syslog_route_dbg("Igmp clear port %x mroute dip %x dstvid %d", igmp_notify->ifindex, igmpSnpItem.dip,igmpSnpItem.dst_vid);
	 			}
			}
			if(dupItem.rt_type == RTN_L2MULTICAST)
			{
				dbtable_hash_delete(npd_mroute_haship_index, &dupItem, NULL);
				npd_syslog_route_dbg("Igmp del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				npd_mroute_nexthop_check_free(&dupItem);	
			}
			else
			{
				if(dupItem.rt_type == RTN_L2L3MULTICAST)
					dupItem.rt_type = RTN_MULTICAST;
				/*Not consider ports from PIM Snooping, suppose the mc entry are not created on one intf by IGMP Snooping and PIM at same time*/
				NPD_PBMP_CLEAR(dupItem.dst_l2_ports);
				NPD_PBMP_CLEAR(dupItem.l2_real_ports);
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
				npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
			}
			break;
		case IGMP_SYS_SET:
			igmpSysSet = igmp_notify->reserve;
			if( IGMP_SYS_SET_INIT == igmpSysSet)  /*set igmp snooping Enable Global Flag */
			{				
				/*transfer Product system MAC address to igmp snooping*/
				ret = npd_igmp_snp_enable();
				if(IGMPSNP_RETURN_CODE_OK != ret){
					syslog_ax_igmp_dbg("npd sysmac notifier error!\n");
				}
			}
			else if(IGMP_SYS_SET_STOP == igmpSysSet)  /*set igmp snooping Disable Global Flag*/
			{				
				ret = npd_igmp_snp_disable();
			}
			break;
#ifdef HAVE_PIM            
        case PIM_ADDR_ADD:
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);    
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
            if (0 != ret) /*not found, creaet a new L2MC entry*/
            {
                igmpSnpItem.rt_type = RTN_L2MULTICAST;
                igmpSnpItem.svid = igmp_notify->vlan_id;
				igmpSnpItem.srcl3_netif_index = npd_netif_vlan_get_index(igmp_notify->vlan_id);
    			NPD_PBMP_PORT_ADD(igmpSnpItem.dst_l2_ports, swPortIndex);
				NPD_PBMP_PORT_ADD(igmpSnpItem.l2_real_ports,swPortIndex);
                if (igmpSnpItem.sip != 0)
                {
                    dupItem = igmpSnpItem;
                    dupItem.sip = 0;
        			ret = dbtable_hash_search(npd_mroute_haship_index, &dupItem, NULL, &dupItem);
                    if (0 == ret) /*imports ports from (*,G) into (S,G)*/
                    {
        				NPD_PBMP_OR(igmpSnpItem.dst_l2_ports, dupItem.dst_l2_ports);
                    }
                }
				if(0 != npd_mroute_nexthop_alloc(&igmpSnpItem))
				{
					npd_syslog_route_err("pim snp alloc nexthop fail, mroute dip %x dstvid %d, mcindex %d", 
								                     igmpSnpItem.dip, igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
				if(0 != dbtable_hash_insert(npd_mroute_haship_index, &igmpSnpItem))
				{
					npd_syslog_route_err("pim snp fail to add port %x mroute dip %x dstvid %d, mcindex %d", 
										                     igmp_notify->ifindex, igmpSnpItem.dip,
										                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
					npd_mroute_nexthop_check_free(&igmpSnpItem);
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
                npd_syslog_route_dbg("Igmp add port %x mroute dip %x dstvid %d, mcindex %d", 
                     igmp_notify->ifindex, igmpSnpItem.dip,
                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
            }
            else
            {
            	/*if found, re-just entry rt_type and add ports*/
                if(dupItem.rt_type == RTN_MULTICAST)
                    dupItem.rt_type = RTN_L2L3MULTICAST;
    			NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
				NPD_PBMP_PORT_ADD(dupItem.l2_real_ports,swPortIndex);
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem);
            }

            if (igmpSnpItem.sip == 0) /*if it's (*,G), exports ports into all (S,G)*/
            {
                for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
                    0 == ret ;
                    (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
                {
                    if (dupItem.sip == 0)
                        continue;
    				NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
    				dbtable_hash_update(npd_mroute_haship_index, NULL, &dupItem);
                    npd_syslog_route_dbg("Igmp update add port %x into mroute dip %x dstvid %d", igmp_notify->ifindex,dupItem.dip,dupItem.dst_vid);
                }
            }
            break;
        case PIM_ADDR_DEL:
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
            if(0 != ret)
            {
                break;	/*if entry not found, just break out*/
            }
			else if(!NPD_PBMP_MEMBER(dupItem.dst_l2_ports, swPortIndex))
			{
				ret = IGMPSNP_RETURN_CODE_OK;
				goto recv_end;
			}

			NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports,swPortIndex);
			NPD_PBMP_PORT_REMOVE(dupItem.l2_real_ports,swPortIndex);
			if(dupItem.rt_type == RTN_L2MULTICAST && NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
			{
				dbtable_hash_delete(npd_mroute_haship_index, &dupItem, NULL);
				npd_syslog_route_dbg("Igmp del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				npd_mroute_nexthop_check_free(&dupItem);	
			}
			else
			{
				if(dupItem.rt_type == RTN_L2L3MULTICAST && NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
					dupItem.rt_type = RTN_MULTICAST;
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
            	npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
			}				

            if (igmpSnpItem.sip == 0)
            {
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
				{
                    if (dupItem.sip == 0 || NPD_PBMP_MEMBER(dupItem.l2_real_ports, swPortIndex)) {
                        continue;
                    }
					NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports,swPortIndex);
					dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
	            	npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
                }
            }
            break;
		case PIM_MODE_SET:
			npd_mroute_pim_mode_set(igmp_notify->reserve);
			break;			
#endif          
#ifdef HAVE_MLD_SNP
		case MLD_ADDR_ADD:
			igmpSnpItem.family = AF_INET6;
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);
            ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
            if(0 != ret)
            {
                igmpSnpItem.rt_type = RTN_L2MULTICAST;
                igmpSnpItem.svid = igmp_notify->vlan_id;
                igmpSnpItem.srcl3_netif_index = npd_netif_vlan_get_index(igmp_notify->vlan_id);				
				NPD_PBMP_PORT_ADD(igmpSnpItem.dst_l2_ports, swPortIndex);
				NPD_PBMP_PORT_ADD(igmpSnpItem.l2_real_ports, swPortIndex);
				if(!IPV6_ADDR_ZERO(igmpSnpItem.sipv6))
				{
					dupItem = igmpSnpItem;
                    memset(&(dupItem.sipv6), 0, sizeof(dupItem.sipv6));
        			ret = dbtable_hash_search(npd_mroute_haship_index, &dupItem, NULL, &dupItem);
                    if (0 == ret) /*imports ports from (*,G) into (S,G)*/
                    {
        				NPD_PBMP_OR(igmpSnpItem.dst_l2_ports, dupItem.dst_l2_ports);
                    }
				}
				if(0 != npd_mroute_nexthop_alloc(&igmpSnpItem))
				{
					npd_syslog_route_err("mld snp alloc nexthop fail, mroute dip %x:%x:%x:%x dstvid %d, mcindex %d", 
						                     igmpSnpItem.dipv6.u6_addr32[0],igmpSnpItem.dipv6.u6_addr32[1],
						                     igmpSnpItem.dipv6.u6_addr32[2],igmpSnpItem.dipv6.u6_addr32[3],
						                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
				if(0 != dbtable_hash_insert(npd_mroute_haship_index, &igmpSnpItem))
				{
					npd_syslog_route_err("mld snp fail to add port %x mroute dip %x:%x:%x:%x dstvid %d, mcindex %d", 
										                     igmp_notify->ifindex, igmpSnpItem.dipv6.u6_addr32[0],
										                     igmpSnpItem.dipv6.u6_addr32[1],igmpSnpItem.dipv6.u6_addr32[2],
										                     igmpSnpItem.dipv6.u6_addr32[3],
										                     igmpSnpItem.dst_vid, igmpSnpItem.tbl_index);
					npd_mroute_nexthop_check_free(&igmpSnpItem);
					ret = IGMPSNP_RETURN_CODE_ERROR;
					goto recv_end;
				}
			}
			else
			{	
				if(NPD_PBMP_MEMBER(dupItem.dst_l2_ports, swPortIndex))
				{
					ret = IGMPSNP_RETURN_CODE_OK;
					goto recv_end;
				}
                if(dupItem.rt_type == RTN_MULTICAST)
                    dupItem.rt_type = RTN_L2L3MULTICAST;
    			NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
				NPD_PBMP_PORT_ADD(dupItem.l2_real_ports, swPortIndex);
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem);
			}

			if(IPV6_ADDR_ZERO(igmpSnpItem.sipv6))
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
	            {
	            	if(IPV6_ADDR_ZERO(dupItem.sipv6))
						continue;
					NPD_PBMP_PORT_ADD(dupItem.dst_l2_ports, swPortIndex);
					dbtable_hash_update(npd_mroute_haship_index, NULL, &dupItem);
	            }
			}
			break;			
		case MLD_ADDR_DEL:
			igmpSnpItem.family = AF_INET6;
			swPortIndex = netif_array_index_from_ifindex(igmp_notify->ifindex);
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
			if(0 != ret)
			{
				break;	/*if entry not found, just break out*/
			}
			else if(!NPD_PBMP_MEMBER(dupItem.dst_l2_ports, swPortIndex))
			{
				ret = IGMPSNP_RETURN_CODE_OK;
				goto recv_end;
			}
			NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports,swPortIndex);
			NPD_PBMP_PORT_REMOVE(dupItem.l2_real_ports,swPortIndex);
			if(dupItem.rt_type == RTN_L2MULTICAST && NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
			{
				dbtable_hash_delete(npd_mroute_haship_index, &dupItem, NULL);
				npd_syslog_route_dbg("Igmp del port %x l2mc dip %x dstvid %d", igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				npd_mroute_nexthop_check_free(&dupItem);	
			}
			else
			{
				if(dupItem.rt_type == RTN_L2L3MULTICAST && !NPD_PBMP_IS_NULL(dupItem.dst_l2_ports))
					dupItem.rt_type = RTN_MULTICAST;
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
				npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
			}				

			if(IPV6_ADDR_ZERO(dupItem.sipv6))
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
				{
					if(IPV6_ADDR_ZERO(dupItem.sipv6) || NPD_PBMP_MEMBER(dupItem.l2_real_ports, swPortIndex))
					{
						continue;
					}
	                NPD_PBMP_PORT_REMOVE(dupItem.dst_l2_ports, swPortIndex);
					dbtable_hash_update(npd_mroute_haship_index, NULL, &dupItem);	                
				}
			}
			break;
		case MLD_ADDR_RMV:
			igmpSnpItem.family = AF_INET6;			
			ret = dbtable_hash_search(npd_mroute_haship_index, &igmpSnpItem, NULL, &dupItem);
			if(0 != ret)
			{
				break;	/*if entry not found, just break out*/
			}
			if(IPV6_ADDR_ZERO(dupItem.sipv6))
			{
	            for((ret = dbtable_hash_head_key(npd_mroute_haship_index, &igmpSnpItem, &dupItem,npd_mroute_compar_ip_vlan)); 
	                0 == ret ;
	                (ret = dbtable_hash_next_key(npd_mroute_haship_index, &dupItem, &dupItem,npd_mroute_compar_ip_vlan)))
	 			{
					if(IPV6_ADDR_ZERO(igmpSnpItem.sipv6))
					{
						continue;
					}
					NPD_PBMP_REMOVE(igmpSnpItem.dst_l2_ports, dupItem.dst_l2_ports);
					NPD_PBMP_OR(igmpSnpItem.dst_l2_ports, dupItem.l2_real_ports);
                    dbtable_hash_update(npd_mroute_haship_index, NULL, &igmpSnpItem);                    
                    npd_syslog_route_dbg("Igmp clear port %x mroute dip %x dstvid %d", igmp_notify->ifindex, igmpSnpItem.dip,igmpSnpItem.dst_vid);
	 			}
			}
			if(dupItem.rt_type == RTN_L2MULTICAST)
			{
				dbtable_hash_delete(npd_mroute_haship_index, &dupItem, NULL);
				npd_syslog_route_dbg("Igmp del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
				npd_mroute_nexthop_check_free(&dupItem);	
			}
			else
			{
				if(dupItem.rt_type == RTN_L2L3MULTICAST)
					dupItem.rt_type = RTN_MULTICAST;
				/*Not consider ports from PIM Snooping, suppose the mc entry are not created on one intf by IGMP Snooping and PIM at same time*/
				NPD_PBMP_CLEAR(dupItem.dst_l2_ports);
				NPD_PBMP_CLEAR(dupItem.l2_real_ports);
				dbtable_hash_update(npd_mroute_haship_index, &dupItem, &dupItem); 
				npd_syslog_route_dbg("Igmp update del port %x l2mc dip %x dstvid %d",igmp_notify->ifindex, dupItem.dip,dupItem.dst_vid);
			}            
			break;
		case MLD_SYS_SET:
			igmpSysSet = igmp_notify->reserve;
			if( IGMP_SYS_SET_INIT == igmpSysSet)  /*set igmp snooping Enable Global Flag */
			{				
				/*transfer Product system MAC address to igmp snooping*/
				ret = npd_mld_snp_enable();
				if(IGMPSNP_RETURN_CODE_OK != ret){
					syslog_ax_igmp_dbg("npd sysmac notifier error!\n");
				}
			}
			else if(IGMP_SYS_SET_STOP == igmpSysSet)  /*set igmp snooping Disable Global Flag*/
			{				
				ret = npd_mld_snp_disable();
			}
			break;
#endif
		default :
			syslog_ax_igmp_dbg("npd can NOT proccess the running result of Protocol");
			break;
	}
recv_end:	
#ifdef HAVE_ROUTE    
    dbtable_hash_unlock(npd_mroute_hwid_hash_index);
    npd_mroute_unlock();
#endif
    npd_key_database_unlock();
	return ret;
}
 /*******************************************************************************
  * npd_igmp_sysmac_notifer
  *
  * DESCRIPTION:
  * 	 	transfer Product system MAC address to igmp snooping	
  *
  * INPUTS:
  *  		null
  *
  * OUTPUTS:
  * 	 	null
  *
  * RETURNS:
  *		IGMPSNP_RETURN_CODE_OK - the mac address is ok and send ok
  *		IGMPSNP_RETURN_CODE_OUT_RANGE - the mac address out of range
  *		IGMPSNP_RETURN_CODE_ERROR - send commad to igmp error
  *
  * COMMENTS:
  * 	 
  **
  ********************************************************************************/
 int npd_igmp_sysmac_notifer(void)
 {
	memcpy(transMac, PRODUCT_MAC_ADDRESS,6);
	syslog_ax_igmp_dbg("Packet etherHeader sMac =%02x:%02x:%02x:%02x:%02x:%02x.\n",\
		transMac[0],transMac[1],transMac[2],transMac[3],transMac[4],transMac[5]);

	return npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_SYS_MAC_NOTI, 0, 0);
 }
 /*******************************************************************************
  * npd_igmp_port_link_change
  *
  * DESCRIPTION:
  *		when the igmp port link state change ,send the commad to igmp_snooping
  *
  * INPUTS:
  *  		eth_g_index - ether global index include device and port number
  *		event - the port notifier event 
  *
  * OUTPUTS:
  * 	 	null
  *
  * RETURNS:
  *		ret - get the returns from npd_cmd_sendto_igmpsnp.
  *
  * COMMENTS:
  * 	 
  **
  ********************************************************************************/
int npd_igmp_port_link_change
(
	unsigned int eth_g_index,	
	enum PORT_NOTIFIER_ENT event
)
{
	unsigned short vid = 0;
	unsigned int netif_index = eth_g_index;
	unsigned long event_type = 0;
	int ret = IGMPSNP_RETURN_CODE_OK;

	if( PORT_NOTIFIER_LINKUP_E == event) 
    {
		event_type = NPD_IGMPSNP_EVENT_DEV_UP;
	}
	else if( PORT_NOTIFIER_LINKDOWN_E == event)
    {
		event_type = NPD_IGMPSNP_EVENT_DEV_DOWN;
	}
	else if(PORT_NOTIFIER_L2DELETE == event ||
            PORT_NOTIFIER_DELETE == event)
	{
        struct npd_igmpsnp_cfg_s npdIgmpCfg;
        int switch_index;

        dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
#ifdef HAVE_MLD_SNP
        struct npd_mldsnp_cfg_s npdMldCfg;
        dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
#endif
        event_type = NPD_IGMPSNP_EVENT_DEV_DELETE;

        if(PORT_NOTIFIER_L2DELETE == event)
        {
            event_type = NPD_IGMPSNP_EVENT_DEV_DELETE;		
        }

        if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(eth_g_index))
		{			
			vid = npd_netif_vlan_get_vid(eth_g_index);
			netif_index = 0;
            NPD_VBMP_VLAN_REMOVE(npdIgmpCfg.vlan_admin_status, vid);
#ifdef HAVE_MLD_SNP
            NPD_VBMP_VLAN_REMOVE(npdMldCfg.vlan_admin_status, vid);
#endif
		}
        else
        {   
            switch_index = netif_array_index_from_ifindex(eth_g_index);
            npdIgmpCfg.igmp_snp_port_admin_state[switch_index] = 1;
            npdIgmpCfg.switch_port_control_count[switch_index] = 0;
#ifdef HAVE_MLD_SNP
            npdMldCfg.switch_port_control_count[switch_index] = 0;
#endif
        }

        dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, NULL, &npdIgmpCfg);    
#ifdef HAVE_MLD_SNP
        dbtable_array_update(npd_mldsnp_cfg_array_index, 0, NULL, &npdMldCfg);    
#endif
	}
    else
        goto retcode;
    
	ret = npd_igmp_cmd_send(event_type, vid, netif_index);
#ifdef HAVE_MLD_SNP
    npd_mld_cmd_send(event_type, vid, netif_index);
#endif
retcode:
	return ret;
}

 
/*******************************************************************************
* npd_igmp_port_vlan_relate_change
*
* DESCRIPTION:
*	 when the igmp port relate state change ,send the commad to igmp_snooping
*
* INPUTS:
*             vlan_id
*		 eth_g_index - ether global index include device and port number
*	 event - the port notifier event 
*
* OUTPUTS:
*		 null
*
* RETURNS:
*	 ret - get the returns from npd_cmd_sendto_igmpsnp.
*
* COMMENTS:
*	  
**
********************************************************************************/
int npd_igmp_port_vlan_relate_change
(
	unsigned short vlan_id,	 
	unsigned int netif_index,
	enum PORT_RELATE_ENT event
)
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
    unsigned char vlanstatus = 0;
    int type = npd_netif_type_get(netif_index);

	ret = npd_check_igmp_snp_vlan_status(vlan_id,&vlanstatus);
    if(IGMPSNP_RETURN_CODE_OK != ret)
        return ret;
    if(0 == vlanstatus)
        return ret;
#if 0
    npd_switch_port_igmp_status(netif_array_index_from_ifindex(netif_index),
        &status);

    if(status == 2)
        status = 0;
#endif
    struct npd_igmpsnp_cfg_s npdIgmpCfg;
	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
    int switch_index = netif_array_index_from_ifindex(netif_index);

    if( PORT_NOTIFIER_LEAVE == event){
		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vlan_id, netif_index);
        npdIgmpCfg.switch_port_control_count[switch_index]--;
    }
    if(PORT_NOTIFIER_JOIN == event){
        if(npdIgmpCfg.igmp_snp_port_admin_state[switch_index])
        {
            ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, 0, netif_index);
            ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vlan_id, netif_index);
        }
        npdIgmpCfg.switch_port_control_count[switch_index]++;
    }

    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        unsigned int trunk_id = npd_netif_trunk_get_tid(netif_index);
        int all_port;
        struct trunk_s node = {0};
        npd_find_trunk(trunk_id, &node);
 		NPD_PBMP_ITER(node.ports, all_port)
 		{
            npdIgmpCfg.switch_port_control_count[all_port] = npdIgmpCfg.switch_port_control_count[switch_index];
        }
    }

    dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, NULL, &npdIgmpCfg);    
    return ret;
}

int npd_igmp_port_trunk_relate_change
(
	unsigned int trunk_index,	 
	unsigned int netif_index,
	enum PORT_RELATE_ENT event
)
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
    struct npd_igmpsnp_cfg_s npdIgmpCfg;
    struct npd_mroute_item_s igmpSnpItem;

    int switch_index = netif_array_index_from_ifindex(trunk_index);
    int port_index = netif_array_index_from_ifindex(netif_index);
    
	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
#ifdef HAVE_MLD_SNP
    struct npd_mldsnp_cfg_s npdMldCfg;
	dbtable_array_get(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
#endif

    if( PORT_NOTIFIER_LEAVE == event)
    {
        npdIgmpCfg.igmp_snp_port_admin_state[port_index] = 1;
        npdIgmpCfg.switch_port_control_count[port_index] = 0;
#ifdef HAVE_MLD_SNP
        npdMldCfg.switch_port_control_count[port_index] = 0;
#endif
    }    
    if (PORT_NOTIFIER_JOIN == event)
    {
        npdIgmpCfg.igmp_snp_port_admin_state[port_index] = npdIgmpCfg.igmp_snp_port_admin_state[switch_index];
        npdIgmpCfg.switch_port_control_count[port_index] = npdIgmpCfg.switch_port_control_count[switch_index];            
#ifdef HAVE_MLD_SNP
        npdMldCfg.switch_port_control_count[port_index] = npdMldCfg.switch_port_control_count[switch_index];                    
#endif
    }
    dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, NULL, &npdIgmpCfg);    
#ifdef HAVE_MLD_SNP
    dbtable_array_update(npd_mldsnp_cfg_array_index, 0, NULL, &npdMldCfg);    
#endif

    for((ret = dbtable_hash_head(npd_mroute_haship_index, NULL, &igmpSnpItem, NULL)); 
        0 == ret ;
        (ret = dbtable_hash_next(npd_mroute_haship_index, &igmpSnpItem, &igmpSnpItem, NULL)))
    {
        if (!NPD_PBMP_MEMBER(igmpSnpItem.dst_l2_ports,switch_index))
            continue;
        
        if(PORT_NOTIFIER_JOIN == event)
        {
            nam_asic_l2mc_member_add(netif_index, igmpSnpItem.l2mc_index, igmpSnpItem.dip, igmpSnpItem.dst_vid);
        }
        
        if(PORT_NOTIFIER_LEAVE == event)
        {
            nam_asic_l2mc_member_del(netif_index, igmpSnpItem.l2mc_index, igmpSnpItem.dip, igmpSnpItem.dst_vid);
        }

    }
    return ret;
}

long npd_igmp_snp_cfg_handle_insert(void * newdata)
{
	struct npd_igmpsnp_cfg_s *newtb = newdata;
    int i = 0;
    unsigned int vlanId, port_ifindex;
    port_driver_t *driver = NULL;

    if(!newtb->npdIgmpSnpEnDis)
        return 0;

    NPD_VBMP_ITER(newtb->vlan_admin_status, vlanId)
    {
		nam_set_igmp_enable(vlanId, TRUE);
    }    

    for(i=0; i<MAX_SWITCHPORT_PER_SYSTEM; i++)
    {
        if(newtb->igmp_snp_port_admin_state[i] && 
            newtb->switch_port_control_count[i] > 0)
        {
            port_ifindex = netif_array_index_to_ifindex(i);
            driver = port_driver_get(port_ifindex);
            if(NULL == driver || NULL == driver->igmp_trap_set)
                continue;
            
            (*driver->igmp_trap_set)(0, port_ifindex, TRUE);
        }
    }

    return 0;
}

long npd_igmp_snp_cfg_handle_update(void * newdata, void *olddata)
{
	struct npd_igmpsnp_cfg_s *newtb = newdata;
    struct npd_igmpsnp_cfg_s *oldtb = olddata;
    int i = 0;
    unsigned int vlanId, port_ifindex;
    npd_vbmp_t tmpVlan;
    port_driver_t *driver = NULL;

    if(newtb->npdIgmpSnpEnDis != oldtb->npdIgmpSnpEnDis)
    {
        NPD_VBMP_ITER(newtb->vlan_admin_status, vlanId)
        {
    		nam_set_igmp_enable(vlanId, newtb->npdIgmpSnpEnDis);
        }

        for(i=0; i<MAX_SWITCHPORT_PER_SYSTEM; i++)
        {
            if(newtb->igmp_snp_port_admin_state[i] && 
                newtb->switch_port_control_count[i] > 0)
            {
                port_ifindex = netif_array_index_to_ifindex(i);
                driver = port_driver_get(port_ifindex);
                if(NULL == driver || NULL == driver->igmp_trap_set)
                    continue;
                
                (*driver->igmp_trap_set)(0, port_ifindex, newtb->npdIgmpSnpEnDis);
            }
        }
        return 0;
    }

    if(!newtb->npdIgmpSnpEnDis)
        return 0;

    if(NPD_VBMP_NEQ(newtb->vlan_admin_status, oldtb->vlan_admin_status))
    {
        NPD_VBMP_ASSIGN(tmpVlan, newtb->vlan_admin_status);
        NPD_VBMP_XOR(tmpVlan, oldtb->vlan_admin_status);
        NPD_VBMP_ITER(tmpVlan, vlanId)
        {
            if(NPD_VBMP_MEMBER(newtb->vlan_admin_status, vlanId))
            {
        		nam_set_igmp_enable(vlanId, TRUE);
            }
            else
            {
                nam_set_igmp_enable(vlanId, FALSE);
            }
        }
    }

    for(i=0; i < MAX_SWITCHPORT_PER_SYSTEM; i++)
    {
        if(newtb->igmp_snp_port_admin_state[i] != oldtb->igmp_snp_port_admin_state[i])
        {
            if(!newtb->switch_port_control_count[i])
                continue;

            port_ifindex = netif_array_index_to_ifindex(i);
            driver = port_driver_get(port_ifindex);
            if(NULL == driver || NULL == driver->igmp_trap_set)
                continue;
            
            (*driver->igmp_trap_set)(0, port_ifindex, newtb->igmp_snp_port_admin_state[i]);
        }
        else if((newtb->switch_port_control_count[i] > 0 && oldtb->switch_port_control_count[i] == 0)
            || (newtb->switch_port_control_count[i] == 0 && oldtb->switch_port_control_count[i] > 0))
        {
            if(!newtb->igmp_snp_port_admin_state[i])
                continue;

            port_ifindex = netif_array_index_to_ifindex(i);
            driver = port_driver_get(port_ifindex);
            if(NULL == driver || NULL == driver->igmp_trap_set)
                continue;
            
            if(newtb->switch_port_control_count[i] > 0)
            {
                (*driver->igmp_trap_set)(0, port_ifindex, TRUE);
            }
            else
            {
                (*driver->igmp_trap_set)(0, port_ifindex, FALSE);
            }
        }
    }

    return 0;
}
 
 int npd_igmp_snp_table_init()
 {
 	int ret;
	struct npd_igmpsnp_cfg_s npdIgmpCfg = {0};

	register_netif_notifier(&npd_igmpsnp_netif_notifier);

	ret = create_dbtable( "IGMP_SNP_CFG_DB", 1, sizeof(struct npd_igmpsnp_cfg_s),\
					npd_igmp_snp_cfg_handle_update, 
					NULL,
					npd_igmp_snp_cfg_handle_insert, 
					NULL,
					NULL,
					NULL, 
					NULL, 
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_igmpsnp_cfgtbl));
	if( 0  != ret )
	{
		syslog_ax_igmp_err("create npd igmp snp cfg array table fail\n");
		return NPD_FAIL;
	}	
	
	ret = dbtable_create_array_index( "IGMP_SNP_CFG_IDX", npd_igmpsnp_cfgtbl, &npd_igmpsnp_cfg_array_index);
	if( 0  != ret )
	{
		syslog_ax_igmp_err("create npd igmp snp cfg array index fail\n");
		return NPD_FAIL;
	}	

	/*init Global igmp Snooping Enable Flag*/
	npdIgmpCfg.npdIgmpSnpEnDis = FALSE;
    memset(npdIgmpCfg.igmp_snp_port_admin_state, 1, MAX_SWITCHPORT_PER_SYSTEM);
	dbtable_array_insert_byid(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);

#ifdef HAVE_MLD_SNP
    struct npd_mldsnp_cfg_s npdMldCfg = {0};
	ret = create_dbtable( "MLD_SNP_CFG_DB", 1, sizeof(struct npd_mldsnp_cfg_s),\
					npd_mld_snp_cfg_handle_update, 
					NULL,
					npd_mld_snp_cfg_handle_insert, 
					NULL,
					NULL,
					NULL, 
					NULL, 
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_mldsnp_cfgtbl));
	if( 0  != ret )
	{
		syslog_ax_igmp_err("create npd mld snp cfg array table fail\n");
		return NPD_FAIL;
	}	
	
	ret = dbtable_create_array_index( "MLD_SNP_CFG_IDX", npd_mldsnp_cfgtbl, &npd_mldsnp_cfg_array_index);
	if( 0  != ret )
	{
		syslog_ax_igmp_err("create npd mld snp cfg array index fail\n");
		return NPD_FAIL;
	}	

	/*init Global igmp Snooping Enable Flag*/
	npdMldCfg.npdMldSnpEnDis = FALSE;
	dbtable_array_insert_byid(npd_mldsnp_cfg_array_index, 0, &npdMldCfg);
#endif
	return NPD_OK;
 }
/**********************************************************************************
*igmp_snp_init()
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*	
*DESCRIPTION:
*	IGMP SNOOP init Global igmp Snooping Enable Flag 
*	and FDB entry listhead and l2mc GroupArray
*
***********************************************************************************/
void igmp_snp_init(void)
{	
	npd_igmp_snp_table_init();

    npd_l2_mc_index = nam_index_create(2048);
	npd_l3_mc_index = nam_index_create(2048);
	nam_igmp_snp_init();
}

int npd_igmp_msg_handler(char *msg, int len)
{
	int ret;
	struct igmp_msg_npd *igmp_msg = (struct igmp_msg_npd *)msg;
	struct igmp_notify_mod_npd *notify_msg = NULL;
	
	if(IGMP_SNP_TYPE_NOTIFY_MSG == igmp_msg->nlh.nlmsg_type)
	{
		if(IGMP_SNP_FLAG_ADDR_MOD == igmp_msg->nlh.nlmsg_flags)
		{
			notify_msg = (struct igmp_notify_mod_npd*) &(igmp_msg->igmp_noti_npd);
			ret = npd_igmp_snp_recvmsg_proc(notify_msg);
		}
	}
	return ret;
}

/**********************************************************************************
*npd_igmp_snp_msg_init()
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*	
*DESCRIPTION:
*	IGMP SNOOP command message handle thread
*
***********************************************************************************/
int npd_igmp_snp_msg_init(void)
{
	int sock = 0;

	/*create socket communication*/
	if(IGMPSNP_RETURN_CODE_OK != npd_igmp_snp_sock_init(&sock))
	{
		syslog_ax_igmp_err("create igmp snooping manage thread socket failed.\r\n");
		return -1;
	}
	igmp_fd = sock ;
	syslog_ax_igmp_dbg("create igmp snooping manage thread socket fd %d ok\n",sock);
    npd_app_msg_socket_register(sock, "igmpMsg", npd_igmp_msg_handler, sizeof(struct igmp_msg_npd));
    return 0;
}

void npd_igmpsnp_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
)
{
    int type = npd_netif_type_get(netif_index);

    if((type != NPD_NETIF_ETH_TYPE)
        &&(type != NPD_NETIF_TRUNK_TYPE)
        &&(type != NPD_NETIF_VLAN_TYPE))
        return;

	syslog_ax_igmp_err("npd notify igmp index event: index 0x%x event %d\n", netif_index, evt);

    switch(evt)
    {
	    case PORT_NOTIFIER_L2CREATE:			
		case PORT_NOTIFIER_INSERT:		
			break;
        case PORT_NOTIFIER_LINKUP_E:
	    case PORT_NOTIFIER_FORWARDING:
			npd_igmp_port_link_change(netif_index, PORT_NOTIFIER_LINKUP_E);
			break;
        case PORT_NOTIFIER_DISCARD:
	    case PORT_NOTIFIER_LINKDOWN_E:
	    case PORT_NOTIFIER_REMOVE:			
			npd_igmp_port_link_change(netif_index, PORT_NOTIFIER_LINKDOWN_E);
			break;
		case PORT_NOTIFIER_DELETE: // clear swportcnt and admin for port/trunk, clear admin for vlan
			npd_igmp_port_link_change(netif_index, PORT_NOTIFIER_DELETE);
			break;
		case PORT_NOTIFIER_L2DELETE: //clear swportcnt and admin for port			
			npd_igmp_port_link_change(netif_index, PORT_NOTIFIER_L2DELETE);
	        break;
	    default:
	        break;
    }

    return;
}

void npd_igmpsnp_relate_event
(
    unsigned int father_index, //shoud be vlaninex, or trunkindex
    unsigned int son_index,  //shold be trunkindex, or portindex
    enum PORT_RELATE_ENT event,
    char *private, int len
)
{
    int type = npd_netif_type_get(father_index);
    unsigned int vid;
   
	syslog_ax_igmp_dbg("npd notify igmp relate event: father_index 0x%x son_index 0x%x event %d\n", \
											father_index, son_index, event);

    switch(type)
    {
        case NPD_NETIF_VLAN_TYPE:
            vid = npd_netif_vlan_get_vid(father_index);
        	npd_igmp_port_vlan_relate_change(vid, son_index, event);    
#ifdef HAVE_MLD_SNP
            npd_mld_port_vlan_relate_change(vid, son_index, event);    
#endif
            break;
        case NPD_NETIF_TRUNK_TYPE:
            npd_igmp_port_trunk_relate_change(father_index, son_index, event);
            break;
        default:
            break;
    }
	return;
}

/*NPD IGMPSNP DBUS Operation*/
/*npd_dbus_igmpsnp_check_vlan_igmp_snoop*/
DBusMessage * npd_dbus_igmpsnp_check_vlan_igmp_snoop
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	unsigned short vlanId = 0;
	unsigned char status = 0;	
	DBusError err;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID))) {
		 syslog_ax_igmp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_igmp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_check_igmp_snp_status(&status);
	if(0 == status){
		ret = IGMPSNP_RETURN_CODE_ENABLE_GBL;
	}
	else {
		ret = npd_check_igmp_snp_vlan_status(vlanId,&status);
		syslog_ax_igmp_dbg("vlan endis IGMP :vlan %d,status = %d",vlanId,status);
	} 

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE,
									 &status);
	return reply;
}

DBusMessage * npd_dbus_igmpsnp_vlan_count
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int i,vCount = 0,ret = IGMPSNP_RETURN_CODE_OK;
	unsigned char status = 0;
	DBusError err;
	
	dbus_error_init(&err);
	npd_check_igmp_snp_status(&status);
	if(0 == status){
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	else {
		for(i =0;i<NPD_VLAN_NUMBER_MAX;i++){
			if(TRUE == npd_check_vlan_exist(i)){
				ret = npd_check_igmp_snp_vlan_status(i, &status);
				if(IGMPSNP_RETURN_CODE_OK == ret && 1 == status) {
					vCount++;
				}
			}
		}
		syslog_ax_igmp_dbg("endis IGMP :vCount = %d, status = %d", vCount, status);
		ret = IGMPSNP_RETURN_CODE_OK;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &vCount);
	return reply;
}

DBusMessage * npd_dbus_igmpsnp_config_vlan
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	vid;
	unsigned int isEnable = 0,ret = IGMPSNP_RETURN_CODE_OK;
	unsigned char	status = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID))) {
		 syslog_ax_igmp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_igmp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	ret = npd_check_igmp_snp_status(&status);
	if(0 == status ){
		syslog_ax_igmp_err("IGMP Snooping NOT Enabled Global.");
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	else{
		ret = npd_check_igmp_snp_vlan_status(vid,&status);
		if(IGMPSNP_RETURN_CODE_OK != ret){
			 syslog_ax_igmp_err("vlan %d not created.",vid);
		}
		else if(0 == isEnable &&0 == status ){
			ret = IGMPSNP_RETURN_CODE_NOTENABLE_VLAN;
			 syslog_ax_igmp_err("vlan %d not enabled IGMP Snooping.",vid);
		}
		else if(1== isEnable && 1== status){
			ret = IGMPSNP_RETURN_CODE_HASENABLE_VLAN;
			 syslog_ax_igmp_err("vlan %d already enabled IGMP Snooping.",vid);
		}
		else{
			 syslog_ax_igmp_dbg("get vid = %d ,isEnable = %d",vid,isEnable);
			ret = npd_vlan_igmp_snp_endis_config(isEnable,vid);
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage * npd_dbus_igmpsnp_vlan_list_show
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{	
	DBusMessage* reply;
	DBusMessageIter		iter;
	DBusMessageIter		iter_array;
	DBusError err;
	npd_pbmp_t eth_bmp, ethu_bmp;
	unsigned int switch_port_index;
	unsigned int	i,ret = IGMPSNP_RETURN_CODE_OK;
	unsigned short	vlanId=0;
	struct vlan_s*	vlanNode = NULL;
    struct npd_igmpsnp_cfg_s npdIgmpCfg;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID))) {
		 syslog_ax_igmp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_igmp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_igmp_dbg("npd_dbus_igmpsnp_inf_show: begin vlan %d", vlanId);

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);

	//npd_check_igmp_snp_status(&enableFlag);
	if(!npdIgmpCfg.npdIgmpSnpEnDis){
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	else if( vlanId > CHASSIS_VLAN_RANGE_MAX){
		ret = IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	else {
		for (;vlanId<CHASSIS_VLAN_RANGE_MAX;vlanId++)
		{			
			vlanNode = npd_find_vlan_by_vid(vlanId);
//			if( NULL != vlanNode && (NPD_TRUE == vlanNode->igmpSnpEnDis) ) 
			if( NULL != vlanNode && (NPD_VBMP_MEMBER(npdIgmpCfg.vlan_admin_status, vlanId)) ) 
			{
				syslog_ax_igmp_dbg("npd_dbus_igmpsnp_inf_show: vlan %d", vlanId);
				ret = IGMPSNP_RETURN_CODE_OK;	
				break;
			}
			else{
				if( NULL != vlanNode )
					free(vlanNode);
				vlanNode = NULL;				
			}
		}
		if(vlanId >= CHASSIS_VLAN_RANGE_MAX)
			ret = IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);			
    dbus_message_iter_append_basic(&iter, 
                                     DBUS_TYPE_UINT16,
                                     &vlanId);
	if( ret == IGMPSNP_RETURN_CODE_OK ) 
	{	
		memset(&eth_bmp ,0 ,sizeof(eth_bmp));
	    memset(&ethu_bmp, 0, sizeof(ethu_bmp));

	    NPD_PBMP_ITER(vlanNode->untag_ports, switch_port_index)
	    {
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port_index])
                NPD_PBMP_PORT_ADD(ethu_bmp, switch_port_index);
                
	    }  
	    NPD_PBMP_ITER(vlanNode->tag_ports, switch_port_index)
	    {
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port_index])
                NPD_PBMP_PORT_ADD(eth_bmp, switch_port_index);
	    }  

	    
		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	 /* ip addr*/														
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for (i = 0; i < (sizeof(npd_pbmp_t)/4); i++ ) {
			
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);												
			dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  ((unsigned int *)&eth_bmp + i));				
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);

		dbus_message_iter_open_container (&iter,
										   DBUS_TYPE_ARRAY,
										   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
													DBUS_TYPE_UINT32_AS_STRING	 /* ip addr*/														
										   DBUS_STRUCT_END_CHAR_AS_STRING,
										   &iter_array);
		for (i = 0; i < (sizeof(npd_pbmp_t)/4); i++ ) {
			
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);												
			dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  ((unsigned int *)&ethu_bmp + i));				
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);

		if(vlanNode)
		    free(vlanNode);
	}
									 	
	return reply;
}

DBusMessage * npd_dbus_igmpsnp_portmbr_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned char	status = 0,isEnable =0,tagMode = 0;
	unsigned short	vid =0;	
	unsigned int	eth_g_index=0;
	unsigned int 	ret = IGMPSNP_RETURN_CODE_OK;
	DBusError err;
    struct npd_igmpsnp_cfg_s npdIgmpCfg;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_BYTE,&isEnable,
							DBUS_TYPE_INVALID))) {
		syslog_ax_igmp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_igmp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		syslog_ax_igmp_err("return error caused dbus.\r\n");
		return NULL;
	}
	
	syslog_ax_igmp_dbg("To set IGMP-SNP at vid %d, port 0x%x, value %d.\n",\
											vid,eth_g_index,isEnable);
    
	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
	//ret = npd_check_igmp_snp_status(&status);
	status = npdIgmpCfg.npdIgmpSnpEnDis;
	if(0 == status ){
		syslog_ax_igmp_err("IGMP Snooping NOT Enabled Global.\r\n");
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	else{
		{
			syslog_ax_igmp_dbg("check port %x IGMP Snp status.\r\n",eth_g_index);
			//ret = npd_vlan_port_igmp_endis_flag_check(vid, eth_g_index, tagMode, &status);
            status = npdIgmpCfg.igmp_snp_port_admin_state[netif_array_index_from_ifindex(eth_g_index)];
			//if(IGMPSNP_RETURN_CODE_OK == ret)
			{
    			if(0 == isEnable && 0 == status ){
    				ret = IGMPSNP_RETURN_CODE_NOTENABLE_PORT;
    				syslog_ax_igmp_err(" port %x not enabled IGMP Snooping.\r\n",eth_g_index);
    			}
    			else if(1 == isEnable && 1 == status){
    				ret = IGMPSNP_RETURN_CODE_HASENABLE_PORT;
    				syslog_ax_igmp_err("port %x already enabled IGMP Snooping.\r\n",eth_g_index);
    			}
    			else{
    				/* here add or delete eth port <slot/port> to(from) vlan */
    				syslog_ax_igmp_dbg("%s IGMP Snp on vlan %d, port %x.\r\n",(isEnable)?"En":"Dis",\
    										vid,eth_g_index);
    				ret = npd_vlan_port_igmp_snp_endis_config(isEnable,vid,eth_g_index,tagMode);
    			}
			}
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

void npd_save_vlan_igmp_snp_cfg(char* buf,int bufLen,unsigned char *enDis)
{
	char *showStr = buf,*cursor = NULL;
	int i = 0;
	int cfgflag = 0,length = 0, port_length = 0;

	cursor = showStr;

	struct npd_igmpsnp_cfg_s npdIgmpCfg;

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
//	rc= npd_check_igmp_snp_status(&status);
	if(NPD_TRUE == npdIgmpCfg.npdIgmpSnpEnDis){		
		for(i = 1; i < NPD_MAX_VLAN_ID; i++) {			
			{
                if(NPD_VBMP_MEMBER(npdIgmpCfg.vlan_admin_status, i)){
					cfgflag = 1;
					if((length + 18) < NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM) {
						length += sprintf(showStr,"vlan %d\n",i);
						 syslog_ax_vlan_dbg("%s\n\n",showStr);
						showStr = buf + length;
					}
							
					if((length + 27) < NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM) {
						length += sprintf(showStr," igmp-snooping enable\n");
						 syslog_ax_vlan_dbg("%s\n",showStr);
						showStr = buf+length;
					}
					
					if(cfgflag){
						length += sprintf(showStr,"exit\n");
						 syslog_ax_vlan_dbg("%s",showStr);
						showStr = buf + length;
					}
				}
				//free(node);
			}
		}

		port_length = length;
		dbtable_array_show(switch_ports, buf, &port_length, NULL,
				 NULL, &npd_save_switch_port_igmp_snp_cfg);
		
	}
	else {
		*enDis = NPD_FALSE;
	}
} 

DBusMessage * npd_dbus_igmpsnp_show_vlan_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *showStr = NULL;
	unsigned char en_dis = 0;
	showStr = (char*)malloc(NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		 syslog_ax_vlan_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM);

	/*save vlan cfg*/
	npd_save_vlan_igmp_snp_cfg(showStr,NPD_VLAN_IGMP_SNP_RUNNING_CFG_MEM,&en_dis);
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}

/*******************************************************************************
 * npd_vlan_igmp_snp_endis_config
 *
 * DESCRIPTION:
 *   		enable/disable IGMP Snooping in vlan 
 *
 * INPUTS:
 * 		isAdd - enalbe or disable flag 
 *		vid - vlan id
 *
 * OUTPUTS:
 *    	null	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - config endis success
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - can not find the vlan 
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int npd_vlan_igmp_snp_endis_config
(
	unsigned char	isAdd,
	unsigned short	vid
)
{
	syslog_ax_igmp_dbg("Enter npd_vlan_igmp_snp_endis_config: vid %d endis %d\n", vid, isAdd);
	unsigned char enDis = isAdd;
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	struct vlan_s *vlanNode = NULL;
    struct npd_igmpsnp_cfg_s npdIgmpCfg;
    int port_ifindex;
    int all_port;
    struct trunk_s node = {0};
    unsigned int trunk_id;
    int type;
    
	/*check vlan exist.*/
	vlanNode = npd_find_vlan_by_vid(vid);
	if (NULL == vlanNode) {
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;/*can NOT directed Return NPD_VLAN_BADPARAM.*/
	}

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
    //status = npdIgmpCfg.npdIgmpSnpEnDis;
	//npd_check_igmp_snp_status(&status);
	/*stamp igmp on vlan struct*/
	//vlanNode->igmpSnpEnDis = enDis;
	//vlanNode->igmpSnpState = enDis & status;


	if(enDis){
		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_REGISTER, vid, 0);
        NPD_VBMP_VLAN_ADD(npdIgmpCfg.vlan_admin_status, vid);
	}
	else{
		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_UNREGISTER, vid, 0);
        NPD_VBMP_VLAN_REMOVE(npdIgmpCfg.vlan_admin_status, vid);
	}

	/*clear igmp on all-ports member of vlan*/

    int switch_port;
    NPD_PBMP_ITER(vlanNode->untag_ports, switch_port)
    {            	
        port_ifindex = netif_array_index_to_ifindex(switch_port);
        if(enDis){
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port]){
        		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vid, 
                    port_ifindex);					
				ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, 0, 
                    port_ifindex);					
            }
            npdIgmpCfg.switch_port_control_count[switch_port]++;
    	}
    	else{
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port]){
        		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vid, 
                    port_ifindex);					
            }
            npdIgmpCfg.switch_port_control_count[switch_port]--;
    	}	

        type = npd_netif_type_get(port_ifindex);
        if(NPD_NETIF_TRUNK_TYPE == type)
        {
            trunk_id = npd_netif_trunk_get_tid(port_ifindex);
            npd_find_trunk(trunk_id, &node);
        	NPD_PBMP_ITER(node.ports, all_port)
        	{
                npdIgmpCfg.switch_port_control_count[all_port] = npdIgmpCfg.switch_port_control_count[switch_port];
            }
        }
    }

    NPD_PBMP_ITER(vlanNode->tag_ports, switch_port)
    {            	
        port_ifindex = netif_array_index_to_ifindex(switch_port);
        if(enDis){
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port]){
        		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, vid, 
                    port_ifindex);
				ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, 0, 
                    port_ifindex);					
            }
            npdIgmpCfg.switch_port_control_count[switch_port]++;
    	}
    	else{
            if(npdIgmpCfg.igmp_snp_port_admin_state[switch_port]){
                ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, vid, 
                    port_ifindex);
            }	
            npdIgmpCfg.switch_port_control_count[switch_port]--;
    	}
        
        type = npd_netif_type_get(port_ifindex);
        if(NPD_NETIF_TRUNK_TYPE == type)
        {
            trunk_id = npd_netif_trunk_get_tid(port_ifindex);
            npd_find_trunk(trunk_id, &node);
        	NPD_PBMP_ITER(node.ports, all_port)
        	{
                npdIgmpCfg.switch_port_control_count[all_port] = npdIgmpCfg.switch_port_control_count[switch_port];
            }
        }
    }
    
    dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, NULL, &npdIgmpCfg);
    
    sleep(2);
	
	free(vlanNode);

	return ret;
}
/*******************************************************************************
 * npd_vlan_port_igmp_snp_endis_config
 *
 * DESCRIPTION:
 *   		enable IGMP Snooping on port of vlan Member
 *
 * INPUTS:
 * 		enable - enable or disable flag 
 *		vid - vlan id
 *		eth_g_idx - ether global index include device and port number
 *		tagMode - port is tag or untag
 *
 * OUTPUTS:
 *    	null	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - config endis success
 *		IGMPSNP_RETURN_CODE_ERROR - get devMum and virPortNum error
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - can not find the vlan which include the port
 *		IGMPSNP_RETURN_CODE_PORT_NOT_EXIST - port not exist
 *		IGMPSNP_RETURN_CODE_ERROR_HW - config hardware error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int npd_vlan_port_igmp_snp_endis_config
(
	unsigned char enable,
	unsigned short vid,
	unsigned int eth_g_idx,
	unsigned char tagMode
)
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	struct switch_port_db_s switch_port = {0};
    struct npd_igmpsnp_cfg_s npdIgmpCfg;
    int switch_index;
    int all_port;
    struct trunk_s node = {0};
    unsigned int trunk_id;
    int type;
    
	syslog_ax_igmp_dbg("%s igmp-snooping on vlan %d port index 0x%x.\n",
						enable ? "enable" : "disable", vid, eth_g_idx);
	syslog_ax_igmp_dbg("find exsiting vlan %d support IGMP.",vid);

    switch_port.global_port_ifindex = eth_g_idx;
    ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
    if(0 != ret)
		return IGMPSNP_RETURN_CODE_PORT_NOT_EXIST;

	dbtable_array_get(npd_igmpsnp_cfg_array_index, 0, &npdIgmpCfg);
    switch_index = netif_array_index_from_ifindex(eth_g_idx);
	/*config IGMP Snooping Protocal*/
	if(enable){
		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, 0, eth_g_idx);
        npdIgmpCfg.igmp_snp_port_admin_state[switch_index] = TRUE;
    }
	else{
		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, 0, eth_g_idx);
        npdIgmpCfg.igmp_snp_port_admin_state[switch_index] = FALSE;
	}

    type = npd_netif_type_get(eth_g_idx);
    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        trunk_id = npd_netif_trunk_get_tid(eth_g_idx);
        npd_find_trunk(trunk_id, &node);
    	NPD_PBMP_ITER(node.ports, all_port)
    	{
            npdIgmpCfg.igmp_snp_port_admin_state[all_port] = npdIgmpCfg.igmp_snp_port_admin_state[switch_index];
        }
    }

    dbtable_array_update(npd_igmpsnp_cfg_array_index, 0, NULL, &npdIgmpCfg);
    
    int temp_vid;
    NPD_VBMP_ITER(switch_port.allow_tag_vlans, temp_vid)
    {
        if(!NPD_VBMP_MEMBER(npdIgmpCfg.vlan_admin_status, temp_vid))
            continue;
    	if(enable){
    		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, temp_vid, eth_g_idx);
    	}
    	else{
    		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, temp_vid, eth_g_idx);
        }
    }
    NPD_VBMP_ITER(switch_port.allow_untag_vlans, temp_vid)
    {
        if(!NPD_VBMP_MEMBER(npdIgmpCfg.vlan_admin_status, temp_vid))
            continue;
    	if(enable){
    		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANADDIF, temp_vid, eth_g_idx);
    	}
    	else{
    		ret = npd_igmp_cmd_send(NPD_IGMPSNP_EVENT_DEV_VLANDELIF, temp_vid, eth_g_idx);
        }
    }
	
	return ret;	
}
#endif

#ifdef __cplusplus
}
#endif
