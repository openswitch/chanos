
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_asd.c
*
*
*CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*   	APIs used in NPD for ASD process.
*
* DATE:
*  		03/01/2010	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.40 $
*******************************************************************************/
#ifdef HAVE_AAA
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "wcpss/waw.h"
#include "npd/protocol/security_api.h"
#include "npd_asd.h"

#define	ASD_NPD_MSG_SOCK	"/var/run/wcpss/npd_ctl"
#define NPD_ASD_MSG_SOCK    "/var/run/wcpss/asd_ctl"

#define	ASD_MNG_MAX_SIZE	sizeof(struct npd_mng_asd)
#define ASD_MNG_MAX_LEN     (1024)

#define MAX_SWITCH_PORT MAX_SWITCHPORT_PER_SYSTEM

/*local variables definition end */
struct	sockaddr_un 	npd_asd_table_addr;   /*local addr*/	
struct	sockaddr_un		asd_table_addr;   	/*remote addr*/
struct	sockaddr_un		asd_wireless_table_addr;   	/*remote addr*/
int		npd_asd_fd = 0;
static int npd_asd_pid = -1;

hash_table_index_t *npd_asd_hashifidx_index = NULL;
sequence_table_index_t *npd_asd_seqcfg_index = NULL;
hash_table_index_t *npd_asd_ufdb_hash_index = NULL;
#ifdef HAVE_PORTAL
array_table_index_t *npd_asd_portal_index = NULL;
hash_table_index_t  *npd_asd_portal_bypass_name = NULL;
unsigned int         npd_asd_portal_id = 0;
extern hash_table_index_t *l3intf_addr_if_index;

void npd_asd_inf_portal_refresh(unsigned int netif_index);
#endif

void npd_asd_init(void);

void npd_asd_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
    );
void npd_asd_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
    );
netif_event_notifier_t npd_asd_netif_notifier =
{
    .netif_event_handle_f = &npd_asd_notify_event,
    .netif_relate_handle_f = &npd_asd_relate_event
};


int npd_asd_send_port_vlan(
	unsigned long port_index, 
	unsigned short vlan_id, 
	Operate wOp,
	int result)
{
	int op_ret = ASD_RETURN_CODE_SUCCESS;
	WIRED_TableMsg wPORT;

	memset(&wPORT, 0, sizeof(wPORT));
	wPORT.wType = PORT_TYPE;
	wPORT.wOp = wOp;
	wPORT.u.wPORT.PORTID = port_index;
	wPORT.u.wPORT.VLAN_ID = vlan_id;
	
	if(sendto(npd_asd_fd, &wPORT, sizeof(wPORT), 0, (struct sockaddr *) &asd_table_addr, sizeof(asd_table_addr)) < 0){
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
		op_ret = ASD_RETURN_CODE_ERROR;
	}
	return op_ret;
}

int npd_asd_send_new_mac_auth(
	unsigned long port_index, 
	unsigned short vlan_id, 
	unsigned char *mac)
{
	int op_ret = ASD_RETURN_CODE_SUCCESS;
	WIRED_TableMsg wPORT;

	memset(&wPORT, 0, sizeof(wPORT));
	wPORT.wType = PORT_TYPE;
	wPORT.wOp = MACAUTH_STA;
	wPORT.u.wPORT.PORTID = port_index;
	wPORT.u.wPORT.VLAN_ID = vlan_id;
	memcpy(wPORT.u.wPORT.STAMAC, mac, MAC_LEN);
	if(sendto(npd_asd_fd, &wPORT, sizeof(wPORT), 0, (struct sockaddr *) &asd_table_addr, sizeof(asd_table_addr)) < 0){
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
		op_ret = ASD_RETURN_CODE_ERROR;
	}
	return op_ret;
}

int npd_asd_send_arp_info_to_wirelss(
	unsigned long   port_index, 
	unsigned short  vlan_id, 
	int     Op,
	unsigned char   STAMAC[MAC_LEN],
	unsigned int    ip_addr)
{
	int     op_ret = ASD_RETURN_CODE_SUCCESS;
	TableMsg    arp_info;
    

	memset(&arp_info, 0, sizeof(arp_info));
	arp_info.Type = STA_TYPE;
	arp_info.u.STA.vlan_id = vlan_id;
    arp_info.u.STA.BSSIndex = npd_netif_get_bssindex(port_index);
    arp_info.u.STA.ip_addr = ip_addr;
    memcpy(arp_info.u.STA.STAMAC, STAMAC, 6);

    if (1 == Op)
    {
	    arp_info.Op = STA_ARP_ADD;
    }
    else if (2 == Op)
    {
	    arp_info.Op = STA_ARP_DELETE;
    }
    else if (3 == Op)
    {
    	arp_info.Op = STA_ARP_UPDATE;
    }
    
	if (sendto(npd_asd_fd, &arp_info, sizeof(arp_info), 0, (struct sockaddr *) &asd_wireless_table_addr, sizeof(asd_wireless_table_addr)) < 0)
    {
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
		op_ret = ASD_RETURN_CODE_ERROR;
	}
	return op_ret;
}

void npd_asd_l3if_del(unsigned int netif_index)
{
#ifdef HAVE_PORTAL
    unsigned int result = 0,ret = 0;
	unsigned int portal_ctrl, portal_srvid;
    unsigned char ifname[32]={0};
    char command_str[256];
	
	result = npd_intf_get_portal_server(&netif_index, &portal_ctrl, &portal_srvid);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

    if (portal_ctrl)
    {
        if (0 == portal_srvid)
        {
             npd_asd_inf_portal_refresh(netif_index);
        }            
        npd_intf_portal_set(FALSE, netif_index);
    }
#endif

}

void npd_asd_inf_addr_event(unsigned int flag,  unsigned int netaddr, unsigned int netif_index)
{
#ifdef HAVE_PORTAL
    unsigned int result = 0,ret = 0;
	unsigned int portal_ctrl, portal_srvid;
    NPD_L3INTF_ADDR intfAddr;
    unsigned char ipaddress[16] = {0};
	unsigned char intfname[32] = {0};
    char command_str[256];
    unsigned int  ifindex;
    
	result = npd_intf_get_portal_server(&netif_index, &portal_ctrl, &portal_srvid);
	if(result != 0)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

    if (0 == portal_ctrl || 0 != portal_srvid)
        return;
 
    npd_netif_index_to_name(netif_index, intfname);  
    npd_asd_inf_portal_refresh(netif_index);
    if (TRUE == flag)
    {       
        lib_get_string_from_ip(ipaddress, netaddr);
        memset(command_str, 0, 256);
	    sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -A PREROUTING -i %s -p tcp --dport 80 -j DNAT --to-destination %s:8001",intfname,ipaddress);
	    system(command_str);
    }
    else
    {   
		ret = npd_intf_gindex_exist_check(netif_index, &ifindex);
		if(ret != NPD_TRUE)
		{
			return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
		}
        intfAddr.ifindex = ifindex;
        for((ret = dbtable_hash_head_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index)); 
        	0 == ret ;
        	(ret = dbtable_hash_next_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index)))
        {
            if (intfAddr.ipAddr == netaddr)
                continue;
            else 
                break;
        }
        if (0 == ret)
        {
            lib_get_string_from_ip(ipaddress, intfAddr.ipAddr);
            memset(command_str, 0, 256);
	        sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -A PREROUTING -i %s -p tcp --dport 80 -j DNAT --to-destination %s:8001",intfname,ipaddress);
            system(command_str);
        }
    }
#endif        
}

void npd_asd_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
	WIRED_TableMsg wPORT;
	int self = 0;
	int type;


	syslog_ax_asd_dbg("npd notify asd index event: index 0x%x event %d\n", netif_index, evt);

	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return;
	}

	self = syscall(SYS_gettid);
	if(self == npd_asd_pid)
	{
		return;
	}
	
	memset(&wPORT, 0, sizeof(wPORT));
	type = npd_netif_type_get(netif_index);

	if((NPD_NETIF_ETH_TYPE == type)||(NPD_NETIF_TRUNK_TYPE == type)){
		wPORT.wType = PORT_TYPE;
		wPORT.u.wPORT.PORTID = netif_index;
	}	
	else if(NPD_NETIF_VLAN_TYPE == type){
		wPORT.wType = VLAN_TYPE;
		wPORT.u.wVLAN.VLANID = npd_netif_vlan_get_vid(netif_index);
	}
	else{
		return;
	}

    switch(evt)
    {
	    case PORT_NOTIFIER_LINKUP_E:
			wPORT.wOp= LINK_UP;
	        break;
	    case PORT_NOTIFIER_LINKDOWN_E:
	    case PORT_NOTIFIER_REMOVE:
			wPORT.wOp= LINK_DOWN;
	        break;
	    case PORT_NOTIFIER_CREATE:
	    	if(NPD_NETIF_ETH_TYPE == type){
				return;
	    	}
	    case PORT_NOTIFIER_L2CREATE:
			wPORT.wOp= WID_ADD;
	        break;
	    case PORT_NOTIFIER_DELETE:
	    case PORT_NOTIFIER_L2DELETE:
			wPORT.wOp= WID_DEL;
	        break;  
        case PORT_NOTIFIER_L3CREATE:
            return;
        case PORT_NOTIFIER_L3DELETE:
            npd_asd_l3if_del(netif_index);
            return;
	    default:
	        return;
    }		

	if(sendto(npd_asd_fd, &wPORT, sizeof(wPORT), 0, (struct sockaddr *) &asd_table_addr, sizeof(asd_table_addr)) < 0){
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
	}
	
    return;
}


void npd_asd_relate_event(
    unsigned int father_index,
    unsigned int son_ifindex,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{

	int father_type, son_type;
	unsigned short pvid = 0, vlan_id = 0;
	unsigned int wOp;
	int self = 0;
    unsigned int *netaddr = (unsigned int*)private;
	

	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return;
	}
	
	self = syscall(SYS_gettid);
	if(self == npd_asd_pid)
	{
		return;
	}
	
	switch(event)
	{
		case PORT_NOTIFIER_LEAVE: 
		case PORT_NOTIFIER_JOIN:
        {
            father_type = npd_netif_type_get(father_index);
	        son_type = npd_netif_type_get(son_ifindex);
	        syslog_ax_asd_dbg("npd notify asd index event: son_ifindex 0x%x father_index:%d event %d\n", son_ifindex, father_index, event);

	        if((father_type != NPD_NETIF_VLAN_TYPE) || 
		    ((son_type != NPD_NETIF_ETH_TYPE) && (son_type != NPD_NETIF_TRUNK_TYPE) )){
		        return;
	        }
	        vlan_id = npd_netif_vlan_get_vid(father_index);

	        npd_vlan_port_pvid_get(son_ifindex, &pvid);
	        if(pvid != vlan_id)
		        return;
			
			wOp = (event==PORT_NOTIFIER_LEAVE)?WIRED_LEAVE:WIRED_JOIN;
			break;
	    }		
        case PORT_NOTIFIER_ADDR_ADD:
            if (netaddr)
                npd_asd_inf_addr_event(TRUE, *netaddr, father_index);
            return;
        case PORT_NOTIFIER_ADDR_DEL:
            if (netaddr)
                npd_asd_inf_addr_event(FALSE, *netaddr,father_index);
            return;
		default:
			return;
	}
	
	npd_asd_send_port_vlan(son_ifindex, vlan_id, wOp,0);	

    return;
}

int npd_asd_set_port_auth_state(
	unsigned int ifIndex, 
	unsigned int vlanId,
	char authorized
	)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	memset(&dbItem, 0, sizeof(dbItem));
	
	dbItem.ifIndex = ifIndex;
	dbItem.authorized = authorized;
	syslog_ax_asd_dbg("npd_asd_set_port_auth_state\r\n");
	syslog_ax_asd_dbg("ifIndex:%d & vlanid:%d & auto learn:%d\r\n", ifIndex, vlanId, authorized);
	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		syslog_ax_asd_dbg("dbItem exist in dbtable\r\n");
		dbItem.authMode = dupItem.authMode;

		status = dbtable_hash_update( npd_asd_hashifidx_index, &dupItem, &dbItem);
		syslog_ax_asd_dbg("npd asd dbtable operate status:%d\r\n", status);		
	}
	else{
		syslog_ax_asd_dbg("Dot1x not enabled on this port, no auth state\r\n");	
	}
	if( 0 != status )
		return ASD_RETURN_CODE_ERROR;
	return ASD_RETURN_CODE_SUCCESS;
}


int npd_asd_get_port_auth_state
( 
	unsigned int ifIndex, 
	unsigned char *state
)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	dbItem.ifIndex = ifIndex;
	*state = 0;

	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		*state = dupItem.authorized;
	}

	return ASD_RETURN_CODE_SUCCESS;
}

int npd_asd_get_convert_mng( struct npd_mng_asd *npdMsg, WIRED_TableMsg *asdMsg)
{
	int ret = ASD_RETURN_CODE_SUCCESS;
	
	switch(asdMsg->wType)
	{
		case SYSTEM_AUTH_CONTROL:
			if(asdMsg->wOp){
				npdMsg->action = NPD_MSG_ASD_ENABLE_SYSTEM_AUTH_CONTROL;
				syslog_ax_asd_dbg("action: %d\r\n",
						npdMsg->action);	
			}
			else{
				npdMsg->action = NPD_MSG_ASD_DISABLE_SYSTEM_AUTH_CONTROL;
				syslog_ax_asd_dbg("action: %d\r\n",
						npdMsg->action);	
			}
			break;
		case SWITCHOVER_TYPE:
			if(SWITCHOVER_OP == asdMsg->wOp){
				npdMsg->ifIndex = asdMsg->u.wSTA.PORTID;
				npdMsg->action = NPD_MSG_ASD_SWITCHOVER;
				syslog_ax_asd_dbg("action: %d\r\n",
						npdMsg->action);	
			}
			break;
		case STA_TYPE:
			if( asdMsg->wOp == WID_ADD ) {
				npdMsg->action = NPD_MNG_ASD_ADD_USER;
				npdMsg->type = asdMsg->u.wSTA.Sta_authtype;
				npdMsg->ifIndex = asdMsg->u.wSTA.PORTID;
				npdMsg->vlanId = asdMsg->u.wSTA.VLANID;
				npdMsg->ipaddr = asdMsg->u.wSTA.Sta_ip;
				memcpy(npdMsg->mac, asdMsg->u.wSTA.STAMAC, 6);
				syslog_ax_asd_dbg("action: %d, vlan_id %d, ifindex 0x%x, \
						mac %02x:%02x:%02x:%02x:%02x:%02x\r\n",
						npdMsg->action, npdMsg->vlanId, npdMsg->ifIndex, \
						npdMsg->mac[0], npdMsg->mac[1], npdMsg->mac[2], \
						npdMsg->mac[3], npdMsg->mac[4], npdMsg->mac[5]);				
			}
			else if( asdMsg->wOp == WID_DEL ) {
				npdMsg->action = NPD_MNG_ASD_DEL_USER;
				npdMsg->type = asdMsg->u.wSTA.Sta_authtype;
				npdMsg->ifIndex = asdMsg->u.wSTA.PORTID;
				npdMsg->vlanId = asdMsg->u.wSTA.VLANID;
				npdMsg->ipaddr = asdMsg->u.wSTA.Sta_ip;
				memcpy(npdMsg->mac, asdMsg->u.wSTA.STAMAC, 6);
				syslog_ax_asd_dbg("action: %d, vlan_id %d, ifindex 0x%x, \
						mac %02x:%02x:%02x:%02x:%02x:%02x\r\n",
						npdMsg->action, npdMsg->vlanId, npdMsg->ifIndex, \
						npdMsg->mac[0], npdMsg->mac[1], npdMsg->mac[2], \
						npdMsg->mac[3], npdMsg->mac[4], npdMsg->mac[5]);				
			}
			else if(asdMsg->wOp == STA_ARP_DELETE){
				npdMsg->action	= NPD_MNG_ASD_STA_ARP_DELETE;
				memcpy(npdMsg->mac, asdMsg->u.wSTA.STAMAC, 6);
			}
			else {
				syslog_ax_asd_err("Unknow ASD operation for STA\r\n");
			}
			break;
		case PORT_TYPE:
			syslog_ax_asd_dbg("npd receive message from asd:type:%d, op:%d, portid:%d port_ctrl:%d vlan_id:%d\r\n",
						asdMsg->wType, asdMsg->wOp, asdMsg->u.wPORT.PORTID, \
						asdMsg->u.wPORT.port_ctrl, asdMsg->u.wPORT.VLAN_ID);
			npdMsg->ifIndex = asdMsg->u.wPORT.PORTID;
			npdMsg->vlanId = asdMsg->u.wPORT.VLAN_ID;
			memcpy(npdMsg->mac, asdMsg->u.wPORT.STAMAC, MAC_LEN);
			if(asdMsg->wOp == WID_MODIFY){
				switch(asdMsg->u.wPORT.port_ctrl){
					case Init_state:
						npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_REINIT;
						syslog_ax_asd_dbg("Disable dot1x: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
					case ForceAuthorized:
						npdMsg->action = NPD_MNG_ASD_DISABLE_DOT1X;
						syslog_ax_asd_dbg("Disable dot1x: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
					case ForceUnauthorized:
						npdMsg->action = NPD_MNG_ASD_ENABLE_DOT1X;
						syslog_ax_asd_dbg("Enable dot1x: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
					case Auto:
						npdMsg->action = NPD_MNG_ASD_SET_PORT_AUTH;
						syslog_ax_asd_dbg("Enable port auth: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
					case MacBased:
						npdMsg->action = NPD_MNG_ASD_SET_MAC_AUTH;
						syslog_ax_asd_dbg("Enable mac auth: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
                    case MAB:
						npdMsg->action = NPD_MNG_ASD_SET_MAB_AUTH;
						syslog_ax_asd_dbg("Enable mac auth: action:%d, ifIndex:%d, vlanId:%d\r\n",
							npdMsg->action, npdMsg->ifIndex, npdMsg->vlanId);
						break;
					default:
						break;
				}			
			}
			else if(asdMsg->wOp == MV_PORT_TO_CONFIGED_VLAN_AND_REINIT){
				syslog_ax_asd_dbg("Move port to configed vlan and reinit!\r\n");
				npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_REINIT;
				break;
				syslog_ax_asd_dbg("Move port %d' to vlan %d action:%d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId, npdMsg->action);
			}
			else if(asdMsg->wOp == MV_PORT_TO_CONFIGED_VLAN_AND_DENY){
				syslog_ax_asd_dbg("Move port to configed vlan and deny access!\r\n");
				npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_DENY;
				break;
				syslog_ax_asd_dbg("Move port %d' to vlan %d action:%d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId, npdMsg->action);
			}
			else if(asdMsg->wOp == MV_PORT_TO_GUEST_VLAN){
				syslog_ax_asd_dbg("Move port to guest vlan!\r\n");
				npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_GUEST_VLAN;
				break;
				syslog_ax_asd_dbg("Move port %d' to vlan %d action:%d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId, npdMsg->action);
			}
			else if(asdMsg->wOp == MV_PORT_TO_AUTH_FAIL_VLAN){
				syslog_ax_asd_dbg("Move port to auth fail vlan\r\n");
				npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_AUTH_FAIL_VLAN;
				break;
				syslog_ax_asd_dbg("Move port %d' to vlan %d action:%d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId, npdMsg->action);
			}
			else if(asdMsg->wOp == MV_PORT_TO_CONFIGED_VLAN_AND_ALLOW){
				syslog_ax_asd_dbg("Move port to configed vlan and allow access!\r\n");
				npdMsg->action = NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_ALLOW;
				break;
				syslog_ax_asd_dbg("Move port %d' to vlan %d action:%d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId, npdMsg->action);
			}
			else if(asdMsg->wOp == SET_GUEST_VLAN){
				npdMsg->action = NPD_MNG_ASD_SET_PORT_GUEST_VLAN;
				syslog_ax_asd_dbg("Set port %d's guest vlan %d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId);
				
			}
			else if(asdMsg->wOp == SET_AUTH_FAIL_VLAN){
				npdMsg->action = NPD_MNG_ASD_SET_PORT_AUTH_FAIL_VLAN;
				syslog_ax_asd_dbg("Set port %d's auth fail vlan %d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId);
				
			}
			/*else if(asdMsg->wOp == AUTH_SUCCESS_VLAN){
				npdMsg->action = NPD_MNG_ASD_SET_PORT_AUTH_SUCCESS_VLAN;
				syslog_ax_asd_dbg("Set port %d's auth success vlan %d\r\n", npdMsg->ifIndex,\
					npdMsg->vlanId);
				
			}*/
			else if(asdMsg->wOp == GET_PORT_INFO){
				npdMsg->action = NPD_MNG_ASD_GET_PORT_INFO;
				syslog_ax_asd_dbg("Set port %d's auth fail vlan %d\r\n", npdMsg->ifIndex);				
			}
			else {
				syslog_ax_asd_err("Unknow ASD operation for PORT\r\n");
			}
			break;
#ifdef HAVE_CAPWAP_ENGINE			
        case UFDB_STA_TYPE:	{
			printf("func:%s, line:%d\r\n", __func__, __LINE__);
			memcpy(npdMsg->mac, asdMsg->u.ufdb_sta.STAMAC, MAC_LEN);
			npdMsg->ifIndex	= asdMsg->u.ufdb_sta.netif_index;
			npdMsg->link_state	= asdMsg->u.ufdb_sta.link_state;
			npdMsg->flags = asdMsg->u.ufdb_sta.flags;
			if(WID_ADD == asdMsg->wOp){
				 npdMsg->action = NPD_MNG_ASD_UFDB_ENTRY_ADD;
			}
			else if(WID_DEL == asdMsg->wOp){
				 npdMsg->action = NPD_MNG_ASD_UFDB_ENTRY_DEL;
			}
			else{
				syslog_ax_asd_err("Unknow ASD operation for UFDB\r\n");
			}
			break;
		}
#endif //HAVE_CAPWAP_ENGINE	
#ifdef HAVE_PORTAL
		case PORTAL_TYPE: {
			npdMsg->ifIndex = asdMsg->u.wPortal.srvID;
			npdMsg->ipaddr = asdMsg->u.wPortal.ipaddr;
			npdMsg->l4port = asdMsg->u.wPortal.port;
			npdMsg->enable = asdMsg->u.wPortal.enable;
			if(WID_ADD == asdMsg->wOp){
				 npdMsg->action = NPD_MNG_ASD_SET_PORTAL;
			}
			else if(WID_DEL == asdMsg->wOp){
				 npdMsg->action = NPD_MNG_ASD_DEL_PORTAL;
			}
			else if(WID_MODIFY == asdMsg->wOp)
			{
				npdMsg->action = NPD_MNG_ASD_SET_PORTAL;
			}
			else{
				syslog_ax_asd_err("Unknow ASD operation for PORTAL\r\n");
			}
			break;				
		}
#endif
		default:
			syslog_ax_asd_err("Unknow ASD Type\r\n");
			break;
	}

	return ret;
}

int npd_asd_set_dot1x_by_ifindex
( 
	unsigned int ifIndex, 
	unsigned int vlanId, 
	unsigned char flag
)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status ;

	syslog_ax_asd_dbg("npd_asd_set_dot1x_by_ifindex\r\n");
	syslog_ax_asd_dbg("ifIndex:%d & vlanid:%d & isEnable:%d\r\n", ifIndex, vlanId, flag);
	
	memset(&dbItem, 0, sizeof(dbItem));
	
	dbItem.ifIndex = ifIndex;
	
	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		syslog_ax_asd_dbg("dbItem exist in dbtable\r\n");
		if( flag == FALSE) {
			syslog_ax_asd_dbg("hash delete dbItem\r\n");
			status = dbtable_hash_delete( npd_asd_hashifidx_index, &dupItem, &dupItem);
		}
	}
	else {
		if( flag == TRUE ) {
			syslog_ax_asd_dbg("hash insert dbItem\r\n");
			syslog_ax_asd_dbg("dbItem authMode:%d\r\n",\
				dbItem.authMode);
			dbItem.authMode = NPD_ASD_AUTH_PORT_MODE;
			dbItem.authorized = FALSE;		
			status = dbtable_hash_insert( npd_asd_hashifidx_index, &dbItem );
		}
	}

	syslog_ax_asd_dbg("npd asd dbtable operate status:%d\r\n", status);
	if(status)
		return ASD_RETURN_CODE_ERROR;
	return ASD_RETURN_CODE_SUCCESS;
}

unsigned int npd_asd_check_vlan_valid( 
	unsigned int netif_index,
	unsigned short vlan_id,
	int state)
{
	unsigned short curr_vlan;

	
	if((auth_fail_state == state)|| (guest_vlan_state == state)){
		return ASD_RETURN_CODE_SUCCESS;
	}
	
	if(npd_vlan_port_pvid_get(netif_index, &curr_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", netif_index);
		return ASD_RETURN_CODE_GET_PVID_FAIL;
	}

	if(curr_vlan == vlan_id){
		return ASD_SECURITY_VLAN_INVALID;
	}
	
	return ASD_RETURN_CODE_SUCCESS;
}

int npd_asd_get_authMode_by_ifindex
( 
	unsigned int ifIndex, 
	unsigned char *mode
)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	dbItem.ifIndex = ifIndex;
	*mode = 0;

	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		*mode = dupItem.authMode;
	}

	return ASD_RETURN_CODE_SUCCESS;
}

int npd_asd_set_authMode_by_ifindex
( 
	unsigned int ifIndex, 
	unsigned int vlanId, 
	unsigned char mode
)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	syslog_ax_asd_dbg("npd_asd_set_authMode_by_ifindex\r\n");
	syslog_ax_asd_dbg("ifIndex:%d & mode:%d\r\n", ifIndex, mode);
	
	memset(&dbItem, 0, sizeof(dbItem));
	
	dbItem.ifIndex = ifIndex;
	dbItem.authMode = mode;
	
	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		syslog_ax_asd_dbg("npd_asd_set_authMode_by_ifindex item exist, update it\r\n");
		dbItem.authorized = FALSE;
		status = dbtable_hash_update( npd_asd_hashifidx_index, &dupItem, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}
	else {
		syslog_ax_asd_dbg("npd_asd_set_authMode_by_ifindex item not exist, insert it\r\n");
		dbItem.authorized = FALSE;
		status = dbtable_hash_insert( npd_asd_hashifidx_index, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}

	return ASD_RETURN_CODE_SUCCESS;
}
int npd_asd_set_guest_vlan
( 
	unsigned int ifIndex, 
	unsigned short guest_vlan
)
{
#if 0
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	memset(&dbItem, 0, sizeof(dbItem));
	
	dbItem.ifIndex = ifIndex;
	//dbItem.vlanId = 0;
	//dbItem.guest_vlan_id = guest_vlan;

	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		if( guest_vlan == dupItem.guest_vlan_id)
			return ASD_RETURN_CODE_SUCCESS;

		dbItem.isEnable = dupItem.isEnable;
		dbItem.authMode = dupItem.authMode;
		dbItem.auth_fail_vlan_id = dupItem.auth_fail_vlan_id;
		dbItem.auth_success_vlan_id = dupItem.auth_success_vlan_id;
		dbItem.authorized = dupItem.authorized;
		
		
		status = dbtable_hash_update( npd_asd_hashifidx_index, &dupItem, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}
	else {
		dbItem.isEnable = FALSE;
		dbItem.authorized = FALSE;
		dbItem.authMode = NPD_ASD_AUTH_PORT_MODE;
		status = dbtable_hash_insert( npd_asd_hashifidx_index, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}
#endif
	return ASD_RETURN_CODE_SUCCESS;
}

int npd_asd_set_auth_fail_vlan
( 
	unsigned int ifIndex, 
	unsigned short auth_fail_vlan
)
{
#if 0
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	memset(&dbItem, 0, sizeof(dbItem));
	
	dbItem.ifIndex = ifIndex;
	//dbItem.vlanId = 0;
	dbItem.auth_fail_vlan_id = auth_fail_vlan;

	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if( 0 == status ) {
		if( auth_fail_vlan == dupItem.auth_fail_vlan_id)
			return ASD_RETURN_CODE_SUCCESS;

		dbItem.isEnable = dupItem.isEnable;
		dbItem.authMode = dupItem.authMode;
		dbItem.auth_success_vlan_id = dupItem.auth_success_vlan_id;
		//dbItem.guest_vlan_id = dupItem.guest_vlan_id;
		dbItem.authorized = dupItem.authorized;
		
		
		status = dbtable_hash_update( npd_asd_hashifidx_index, &dupItem, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}
	else {
		dbItem.isEnable = FALSE;
		dbItem.authorized = FALSE;
		dbItem.authMode = NPD_ASD_AUTH_PORT_MODE;
		status = dbtable_hash_insert( npd_asd_hashifidx_index, &dbItem);
		if( 0 != status )
			return ASD_RETURN_CODE_ERROR;
	}
#endif
	return ASD_RETURN_CODE_SUCCESS;
}

int npd_asd_check_authMode_MAB(unsigned int netif_index)
{
	struct npd_asd_item_s dbItem, dupItem;
	int status;

	dbItem.ifIndex = netif_index;
	status = dbtable_hash_search( npd_asd_hashifidx_index, &dbItem, NULL, &dupItem);
	if(0 == status)
	{
		if(!dupItem.authorized && NPD_ASD_AUTH_MAB_MODE == dupItem.authMode)
			return NPD_TRUE;
	}
    
    return NPD_FALSE;
}

int npd_asd_calc_netif_learn_status(unsigned char mode, unsigned char authorize)
{
	int flag = 0;

	switch(mode)
	{
		case NPD_ASD_AUTH_PORT_MODE:
			flag = authorize;
			break;
		case NPD_ASD_AUTH_MAC_MODE:
			flag = 0;
			break;
		case NPD_ASD_AUTH_MAB_MODE:
			flag = 1;
			break;
		default:
			break;
	}

	return flag;
}

unsigned int npd_asd_keyifidx_generate( void* item )
{
    struct npd_asd_item_s *asdItem = (struct npd_asd_item_s *)item;
	unsigned int key = 0;//i = 0;

	if(NULL == asdItem) {
		syslog_ax_asd_err("npd asd items make key null pointers error.");
		return ~0UI;
	}

	key = asdItem->ifIndex;

	key = key & 0x07FF;

	key %= (NPD_ASD_HASH_IFIDX_SIZE);
	
	return key;	
}
#ifdef HAVE_CAPWAP_ENGINE
int npd_asd_ufdb_mac_generate( struct npd_asd_ufdb_s *ufdbItem )
{
	unsigned int key = 0;//i = 0;

	if(NULL == ufdbItem) {
		syslog_ax_asd_err("npd asd ufdb items make key null pointers error.");
		return ~0UI;
	}

	key = ufdbItem->mac[0]^ufdbItem->mac[1]^ufdbItem->mac[2]\
		 ^ufdbItem->mac[3]^ufdbItem->mac[4]^ufdbItem->mac[5];

	key %= (NPD_ASD_UFDB_HASH_SIZE);
	
	return key;	
}
#endif //HAVE_CAPWAP_ENGINE

/**********************************************************************************
 * npd_asd_compare
 *
 * compare two of asd database(Hash table) items
 *
 *	INPUT:
 *		itemA	- asd database item
 *		itemB	- asd database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_asd_compare
(
    void *item1,
    void *item2
)
{
	struct npd_asd_item_s *itemA = (struct npd_asd_item_s*) item1;
	struct npd_asd_item_s *itemB = (struct npd_asd_item_s*) item2;
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd igmp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->ifIndex != itemB->ifIndex )
	{
		equal = FALSE;
	}
	
	return equal;

} 

/**********************************************************************************
 * npd_asd_ufdb_compare
 *
 * compare two of asd ufdb database(Hash table) items
 *
 *	INPUT:
 *		itemA	- asd ufdb database item
 *		itemB	- asd ufdb database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
 #ifdef HAVE_CAPWAP_ENGINE
unsigned int npd_asd_ufdb_compare
(
	struct npd_asd_ufdb_s *itemA,
	struct npd_asd_ufdb_s *itemB
)
{
	unsigned int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd asd ufdb items compare null pointers error.");
		return FALSE;
	}

	if(memcmp(itemA->mac, itemB->mac, MAC_ADDR_LEN)!= 0)
	{
		equal = FALSE;
	}
	
	return equal;

} 
#endif //HAVE_CAPWAP_ENGINE

#ifdef HAVE_PORTAL
int npd_asd_portal_conf_get(int srvID, npd_asd_portal_srv *entry)
{
	return dbtable_array_get(npd_asd_portal_index, srvID, entry);
}

int npd_asd_portal_conf_set(int srvID, npd_asd_portal_srv *entry)
{
	npd_asd_portal_srv prentry;
	if(0 == dbtable_array_get(npd_asd_portal_index, srvID, &prentry))
		return dbtable_array_update(npd_asd_portal_index, srvID, entry, entry);

	return dbtable_array_insert_byid(npd_asd_portal_index,srvID,entry);
}

int npd_asd_portal_conf_del(int srvID)
{
	npd_asd_portal_srv delEntry;
	return dbtable_array_delete(npd_asd_portal_index, srvID, &delEntry);
}

int npd_asd_portal_bypass_ip_generate( portal_user_info_s *userinfo )
{
	unsigned int key = 0;

	if(NULL == userinfo) {
		syslog_ax_asd_err("npd asd portal client-bypass user items make key null pointers error.");
		return ~0UI;
	}

	key = userinfo->ipaddr;
	key %= (256);
	
	return key;	
}

unsigned int npd_asd_portal_bypass_ip_compare
(
	portal_user_info_s *itemA,
	portal_user_info_s *itemB
)
{
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd asd portal client-bypass user items compare null pointers error.");
		return FALSE;
	}

	if( (itemA->ipaddr) != (itemB->ipaddr) )
	{
		equal = FALSE;
	}
	
	return equal;

} 
#endif

long npd_asd_dbtbl_handle_update( void *newItem, void *oldItem)
{
	int ret = ASD_RETURN_CODE_SUCCESS;	 
	int flag = 0;
	struct npd_asd_item_s *origItem = NULL, *updateItem = NULL;


	if( (newItem == NULL ) || ( oldItem == NULL ) )
		return IGMPSNP_RETURN_CODE_ERROR;

	origItem = (struct npd_asd_item_s *)oldItem;
	updateItem = (struct npd_asd_item_s *)newItem;

	syslog_ax_asd_dbg("Here to update old ASD entry On HW, ifIndex 0x%x, authMode %d\n",
														origItem->ifIndex, origItem->authMode );
	syslog_ax_asd_dbg("Here to update new ASD entry On HW, ifIndex 0x%x, authMode %d\n",
														updateItem->ifIndex, updateItem->authMode);

	flag = npd_asd_calc_netif_learn_status(updateItem->authMode, updateItem->authorized);
	syslog_ax_asd_dbg("Set auto learn %d on port:%d\r\n", updateItem->authorized,updateItem->ifIndex);
	ret = npd_fdb_set_netif_learn_status( updateItem->ifIndex, flag);
	if(1 != flag){
		syslog_ax_asd_dbg("Delete fdb entry on port %d\r\n", updateItem->ifIndex);
		npd_fdb_entry_del_by_port(updateItem->ifIndex);
	}	 		

	return ret;
}

long npd_asd_dbtbl_handle_insert( void *newItem )
{
	int ret = ASD_RETURN_CODE_SUCCESS;
	int flag;
	struct npd_asd_item_s *opItem = NULL;

	if( newItem == NULL ){
		syslog_ax_asd_dbg("The new item to be insert is NULL\r\n");
		return ASD_RETURN_CODE_ERROR;
	}

	opItem = (struct npd_asd_item_s *)newItem;

	syslog_ax_asd_dbg("Here to insert ASD entry On HW, ifIndex 0x%x, authMode %d\n",
														opItem->ifIndex, opItem->authMode );

	flag = npd_asd_calc_netif_learn_status(opItem->authMode, opItem->authorized);
	syslog_ax_asd_dbg("Set auto learn %d on port:%d\r\n", flag, opItem->ifIndex);
	npd_fdb_set_netif_learn_status(opItem->ifIndex, flag);
	if(1 != flag)
	{
		syslog_ax_asd_dbg("Delete fdb entry on port %d\r\n", opItem->ifIndex);
		npd_fdb_entry_del_by_port(opItem->ifIndex);
	}	 
	return ret;
}

long npd_asd_dbtbl_handle_delete( void *delItem )
{
	int ret = ASD_RETURN_CODE_SUCCESS;
	struct npd_asd_item_s *opItem = NULL; 

	if( delItem == NULL ){
		syslog_ax_asd_dbg("The to be deleted item is NULL\r\n");
		return ASD_RETURN_CODE_ERROR;
	}

	opItem = (struct npd_asd_item_s *)delItem;

	syslog_ax_asd_dbg("Here to delete ASD entry On HW, ifIndex 0x%x, authMode %d\n",
																opItem->ifIndex, opItem->authMode);

	syslog_ax_asd_dbg("Set auto learn enable on port:%d\r\n", opItem->ifIndex);
	npd_fdb_set_netif_learn_status(opItem->ifIndex, TRUE);
	return ret;
}


 int npd_asd_cfgtbl_handle_update( void *msg, int startIdx, int endIdx)
 {
	 int ret = ASD_RETURN_CODE_SUCCESS;
	 	 
	 return ret;
 }


/*******************************************************************************
 * npd_asd_sock_init
 *
 * DESCRIPTION:
 *   		create socket communication with asd
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	ser_sock - socket number
 *
 * RETURNS:
 * 		ASD_RETURN_CODE_SUCCESS   - create successfully 
 * 		ASD_RETURN_CODE_ERROR	     - create socket or bind error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int	npd_asd_sock_init(int *ser_sock)
{
	memset(&asd_table_addr,0,sizeof(asd_table_addr));
	memset(&npd_asd_table_addr,0,sizeof(npd_asd_table_addr));
	memset(&asd_wireless_table_addr,0,sizeof(asd_wireless_table_addr));
    
	if((*ser_sock = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		syslog_ax_asd_err("create npd to asd socket fail\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	npd_asd_table_addr.sun_family = AF_LOCAL;
	strcpy(npd_asd_table_addr.sun_path,"/tmp/npd_ctl");

	asd_table_addr.sun_family = AF_LOCAL;
	strcpy(asd_table_addr.sun_path, "/tmp/asd_ctl");

	asd_wireless_table_addr.sun_family = AF_LOCAL;
	strcpy(asd_wireless_table_addr.sun_path, "/var/run/wcpss/asd_table");


    unlink(npd_asd_table_addr.sun_path);

	if(bind(*ser_sock , (struct sockaddr *)&npd_asd_table_addr, sizeof(npd_asd_table_addr)) == -1) 
	{
		syslog_ax_asd_err("npd to asd socket created but failed when bind\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	chmod(npd_asd_table_addr.sun_path, 0777);
	return IGMPSNP_RETURN_CODE_OK;	
	
}

/*******************************************************************************
 * npd_asd_recv_info
 *
 * DESCRIPTION:
 *   		use recvfrom to receive information from asd 
 *
 * INPUTS:
 *		infoLen - the receive max size
 *
 * OUTPUTS:
 * 		msg - pointer to asd message get from asd
 *		len - actual receive the data size
 *
 * RETURNS:
 *		ASD_RETURN_CODE_SUCCESS - complete the receive
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int npd_asd_recv_info
(
	struct npd_mng_asd *msg,
	unsigned int  infoLen,
	int *len
)
{
	unsigned int addrLen = sizeof(asd_table_addr);
	unsigned char buf[ASD_MNG_MAX_LEN];
	WIRED_TableMsg *recvMsg = NULL;
	while(1)
	{
		*len = recvfrom(npd_asd_fd, (char*)buf, ASD_MNG_MAX_LEN, 0,(struct sockaddr *)&asd_table_addr, &addrLen);
		if(*len < 0 && errno == EINTR) 
		{
			continue;
		}
		break;
	}
	
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return ASD_RETURN_CODE_SUCCESS;
	}

	recvMsg = (WIRED_TableMsg *)buf;
	syslog_ax_asd_dbg("npd receive message from asd:len:%d,type:%d, op:%d, portid:%d port_ctrl:%d\r\n",
						*len,recvMsg->wType, recvMsg->wOp, recvMsg->u.wPORT.PORTID, \
						recvMsg->u.wPORT.port_ctrl);	
	npd_asd_get_convert_mng( msg, recvMsg);	
	
	return ASD_RETURN_CODE_SUCCESS;
}



int npd_asd_system_auth_control_on(struct npd_mng_asd * asd_msg)
{
	int op_ret = ASD_RETURN_CODE_SUCCESS;
	WIRED_TableMsg wPortStatus;
	switch_port_db_t switch_port;
	int array_index;
	int netif_type;
	int i;

	syslog_ax_asd_dbg("Asd system auth control enable\r\n");
	memset(&wPortStatus, 0, sizeof(wPortStatus));
	wPortStatus.wType = SYSTEM_AUTH_CONTROL;
	wPortStatus.wOp = UPLOAD_PORT_STATUS;
	
	for(i=0; i < MAX_SWITCHPORT_PER_SYSTEM; i++){
		 op_ret = dbtable_array_get(switch_ports, i, &switch_port);
	    if(op_ret != 0)
	    {
			syslog_ax_asd_dbg("ethindex 0x%x is the last or error\r\n", i);
	        continue;
	    }

		netif_type = npd_netif_type_get(switch_port.global_port_ifindex);
	    
        if(NPD_NETIF_TRUNK_TYPE == netif_type)
        {
            array_index = npd_netif_trunk_get_tid(switch_port.global_port_ifindex);
            NPD_PBMP_PORT_ADD(wPortStatus.u.wSTATUS.trunk_map, array_index);
			
			if(PORT_LINK_UP == switch_port.link_state){
				NPD_PBMP_PORT_ADD(wPortStatus.u.wSTATUS.trunk_link_map, array_index);
			}
        }
        else if(NPD_NETIF_ETH_TYPE == netif_type){
			array_index = eth_port_array_index_from_ifindex(switch_port.global_port_ifindex);
			
			NPD_PBMP_PORT_ADD(wPortStatus.u.wSTATUS.port_map, array_index);
			
			if(PORT_LINK_UP == switch_port.link_state){
				NPD_PBMP_PORT_ADD(wPortStatus.u.wSTATUS.link_map, array_index);
			}
		}
	}
	
	if(sendto(npd_asd_fd, &wPortStatus, sizeof(wPortStatus), 0, (struct sockaddr *) &asd_table_addr, sizeof(asd_table_addr)) < 0){
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
		op_ret = ASD_RETURN_CODE_ERROR;
		goto error;
	}
	op_ret = ASD_RETURN_CODE_SUCCESS;
error:
	return op_ret;
}


int npd_asd_system_auth_control_off(struct npd_mng_asd * asd_msg)
{
	return  0;
}

int npd_asd_switchover(struct npd_mng_asd * asd_msg)
{
	return npd_fdb_static_entry_del_by_port(asd_msg->ifIndex);
}

int npd_asd_user_log_on(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId, pvid;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;
	
	syslog_ax_asd_dbg("Asd add user\r\n");
#ifdef HAVE_PORTAL
	if(asd_msg->type == PORTAL)
	{
		npd_asd_portal_sta_set(TRUE, asd_msg->ipaddr);
		return ret;
	}
#endif	
	syslog_ax_asd_dbg("Dot1x mode is Mac-based\r\n");
	if( NPD_FDB_ERR_NODE_EXIST == npd_fdb_check_static_authen_entry_exist(asd_msg->mac,\
										asd_msg->vlanId, &ifIndex)) {
		syslog_ax_asd_dbg("User exists already\r\n");
		ret = ASD_RETURN_CODE_SUCCESS;
	}

	syslog_ax_asd_dbg("Add user as static FDB entry\r\n");
	ret = npd_fdb_static_authen_entry_add(asd_msg->mac, asd_msg->vlanId, asd_msg->ifIndex);

	if(0 == npd_vlan_port_pvid_get(ifIndex, &pvid) && pvid != vlanId)
		ret = npd_vlan_assoc_mac_set(TRUE, asd_msg->mac, asd_msg->vlanId);

	return ret;
}

int npd_asd_user_log_off(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId, pvid;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	
	syslog_ax_asd_dbg("Asd del user\r\n");
#ifdef HAVE_PORTAL
	if(asd_msg->type == PORTAL)
	{
		npd_asd_portal_sta_set(FALSE, asd_msg->ipaddr);
		return ret;
	}
#endif
	syslog_ax_asd_dbg("Dot1x mode is Mac-based\r\n");
	if( NPD_FDB_ERR_NODE_EXIST != npd_fdb_check_static_authen_entry_exist(asd_msg->mac,\
							asd_msg->vlanId, &ifIndex)) {
		syslog_ax_asd_dbg("User doesn't exist\r\n");					
		ret = ASD_RETURN_CODE_SUCCESS;
	}

	syslog_ax_asd_dbg("Del user as static FDB entry\r\n");
	ret = npd_fdb_static_authen_entry_del(asd_msg->mac, asd_msg->vlanId );

	if(0 == npd_vlan_port_pvid_get(ifIndex, &pvid) && pvid != vlanId)
		ret = npd_vlan_assoc_mac_set(FALSE, asd_msg->mac, asd_msg->vlanId);

	return ret;
}

extern hash_table_index_t *npd_arpsnp_haship_index ;
int npd_asd_sta_arp_delete(struct npd_mng_asd * asd_msg)
{
	struct arp_snooping_item_s arp_item;
	int ret = 0;
	
	syslog_ax_asd_dbg("Npd asd del sta arp\r\n");

	while(0 == ret)
	{
    	memset(&arp_item, 0, sizeof(arp_item));
    	ret = npd_arp_snooping_find_item_bymac(asd_msg->mac, &arp_item);
    	if(ret != ASD_RETURN_CODE_SUCCESS){
    		syslog_ax_asd_dbg("Fail to find sta arp:"MACSTR"\r\n", MAC2STR(asd_msg->mac));
    		return ASD_RETURN_CODE_ERROR;
    	}
    #ifdef HAVE_CAPWAP_ENGINE
    	ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_STA_ARP_DEL, &arp_item);
    	if(ASD_RETURN_CODE_SUCCESS != ret){
    		syslog_ax_asd_err("func:%s del arp event process error %d\r\n", __func__, ret);
    	}
    #endif
    	syslog_ax_asd_dbg("Del station arp entry\r\n");
    	if(arp_item.isStatic) {
    		npd_arp_snooping_del_static(npd_arpsnp_haship_index, &arp_item, TRUE);
    	}
    	else {
    		npd_arp_snooping_del(npd_arpsnp_haship_index, &arp_item, TRUE);
    	}
	}
	
	return ret;
}


int npd_asd_port_enable_dot1x(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	
	syslog_ax_asd_dbg("Asd port Enable dot1x\r\n");
	/*If msg doesn't take vlan ID parameter, then get default vlan Id of the port*/
	if(vlanId == 0){
		ret = npd_vlan_port_pvid_get(ifIndex, &vlanId);
		if(ret < 0){
			syslog_ax_asd_err("Get port %d vlan ID failed\r\n", ifIndex);
			return ret;
		}
		syslog_ax_asd_dbg("Get port default vlanId:%d\r\n", vlanId);
	}
	
	/*Set default dot1x auth mode on the port & vlan*/
	ret = npd_asd_set_authMode_by_ifindex( ifIndex, vlanId,NPD_ASD_AUTH_PORT_MODE );
	if(ret != 0){
		syslog_ax_asd_err("Set dot1x mode failed\r\n");
		return ret;
	}
	
	syslog_ax_asd_dbg("Enable dot1x return:%d\r\n", ret);
	return ret;
}

int npd_asd_port_disable_dot1x(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	syslog_ax_asd_dbg("Asd port Disable dot1x\r\n");
	if(vlanId == 0){
		ret = npd_vlan_port_pvid_get(ifIndex, &vlanId);
		if(ret < 0){
			syslog_ax_asd_err("Get port %d vlan ID failed\r\n", ifIndex);
			return ret;
		}
		syslog_ax_asd_dbg("Get port default vlanId:%d\r\n", vlanId);
	}
	
	syslog_ax_asd_dbg("Disable dot1x\r\n");
	ret = npd_asd_set_dot1x_by_ifindex( ifIndex, vlanId, FALSE);

	syslog_ax_asd_dbg("Disable dot1x return:%d\r\n", ret);
	return ret;
}


int npd_asd_set_port_mac_auth(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	syslog_ax_asd_dbg("Asd port Enable mac-based dot1x\r\n");
	if(vlanId == 0){
		ret = npd_vlan_port_pvid_get(ifIndex, &vlanId);
		if(ret < 0){
			syslog_ax_asd_err("Get port %d vlan ID failed\r\n", ifIndex);
			return ret;
		}
		syslog_ax_asd_dbg("Get port default vlanId:%d\r\n", vlanId);
	}
	/*Set port & vlan dot1x auth modes*/
	syslog_ax_asd_dbg("Set dot1x auth mode Mac-based\r\n");
	ret = npd_asd_set_authMode_by_ifindex( ifIndex, vlanId,NPD_ASD_AUTH_MAC_MODE );

	return ret;
}

int npd_asd_set_port_mab_auth(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;
	
	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;
	
	syslog_ax_asd_dbg("Asd port Enable dot1x\r\n");
	if(vlanId == 0){
		ret = npd_vlan_port_pvid_get(ifIndex, &vlanId);
		if(ret < 0){
			syslog_ax_asd_err("Get port %d vlan ID failed\r\n", ifIndex);
			return ret;
		}
		syslog_ax_asd_dbg("Get port default vlanId:%d\r\n", vlanId);
	}
	
	ret = npd_asd_set_authMode_by_ifindex( ifIndex, vlanId, NPD_ASD_AUTH_MAB_MODE );
	if(ret != 0){
		syslog_ax_asd_err("Set dot1x mode failed\r\n");
		return ret;
	}
	
	syslog_ax_asd_dbg("Enable dot1x return:%d\r\n", ret);
	return ret;
}


static int npd_asd_mv_port_to_vlan(unsigned int ifIndex, unsigned short vlanId)
{
	int ret= ASD_RETURN_CODE_SUCCESS;

	
	syslog_ax_asd_dbg("Move port %d to vlan %d untag\r\n", ifIndex, vlanId);
	if (npd_vlan_check_port_membership(vlanId, ifIndex, 0)){
		syslog_ax_asd_dbg("Port %d already in vlan %d untag\r\n", ifIndex, vlanId);
		return ASD_RETURN_CODE_SUCCESS;
	}
	
	if (npd_vlan_check_port_membership(vlanId, ifIndex, 1)){
		syslog_ax_asd_err("Port %d already in vlan %d tag\r\n", ifIndex, vlanId);
		return ASD_RETURN_CODE_PORT_MEMBERSHIP_ERR;
	}

	if(!npd_vlan_check_port_membership(DEFAULT_VLAN_ID, ifIndex, 0)){
		if(npd_netif_free_alluntag_vlan(ifIndex)){
			syslog_ax_asd_err("Fail to reset port %d to default vlan untag\r\n", ifIndex);
			return ASD_RETURN_CODE_PORT_MEMBERSHIP_ERR;
		}
	}
	
	/*Check if destination vlan is default VLAN*/
	if(vlanId != DEFAULT_VLAN_ID){
		ret = npd_netif_allow_vlan( ifIndex, vlanId, 0, NPD_TRUE );
	}
	
	return ret;
}


int npd_asd_mv_port_access_vlan_deny(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId, curr_vlan;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;
	
	if(npd_vlan_port_pvid_get(ifIndex, &curr_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
		goto reply;
	}

	if(npd_fdb_static_entry_del_by_vlan_port(curr_vlan, ifIndex) != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_err("Fail to del static fdb on port %d vlan %d\r\n", ifIndex, curr_vlan);
		ret = ASD_RETURN_CODE_DEL_STATIC_FDB_FAIL;
		goto reply;
	}
	
	
	if(vlanId != 0 && vlanId != curr_vlan){	
		ret = npd_asd_mv_port_to_vlan(ifIndex, vlanId);
		if(ret != ASD_RETURN_CODE_SUCCESS){
			syslog_ax_asd_err("Move port %d to vlan %d ERROR\r\n", ifIndex, vlanId);
			ret = ASD_RETURN_CODE_CHG_VLAN_FAIL;
			goto reply;
		}
	}
	else{
		vlanId = curr_vlan;
		syslog_ax_asd_dbg("Port %d is already in access vlan\r\n", ifIndex);
	}
	
	/*Disable fdb auto learn of the port*/
	syslog_ax_asd_dbg("Disable auto learn\r\n");
	ret = npd_asd_set_port_auth_state(ifIndex, vlanId, FALSE);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		ret = ASD_RETURN_CODE_SET_AUTH_STAT_FAIL;
	}
reply:	
	return npd_asd_send_port_vlan(ifIndex, vlanId, NPD_ASD_SET_RESULT, ret);
}

int npd_asd_mv_port_guest_vlan(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;
	unsigned short configed_access_vlan = DEFAULT_VLAN_ID;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	if(npd_vlan_port_pvid_get(ifIndex, &configed_access_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
		goto reply;
	}

	if(npd_fdb_static_entry_del_by_vlan_port(configed_access_vlan, ifIndex) != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_err("Fail to del static fdb on port %d vlan %d\r\n", ifIndex, configed_access_vlan);
		ret = ASD_RETURN_CODE_DEL_STATIC_FDB_FAIL;
		goto reply;
	}
	
	syslog_ax_asd_dbg("Move port %d to vlan %d untag\r\n", ifIndex, vlanId);
	/*check if the port already in the specified vlan untag*/
	ret = npd_asd_mv_port_to_vlan(ifIndex, vlanId);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		ret = ASD_RETURN_CODE_CHG_VLAN_FAIL;
		goto reply;
	}

	/*Enable fdb auto learn of the port*/
	syslog_ax_asd_dbg("Dot1x mode is port based, Enable auto learn\r\n");
	ret = npd_asd_set_port_auth_state(ifIndex, vlanId, TRUE);
	if(ret != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_dbg("Enable dot1x return:%d\r\n", ret);
		ret = ASD_RETURN_CODE_ERROR;
	}
	
	syslog_ax_asd_dbg("guest vlan:Send configured access vlan %d to asd\r\n",configed_access_vlan);
reply:
	return  npd_asd_send_port_vlan(ifIndex, configed_access_vlan, SET_CONFIGED_ACCESS_VLAN, ret);
}

int npd_asd_mv_port_auth_fail_vlan(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId;
	int ret;
	unsigned short configed_access_vlan = DEFAULT_VLAN_ID;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	if(npd_vlan_port_pvid_get(ifIndex, &configed_access_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
		goto reply;
	}
	if(npd_fdb_static_entry_del_by_vlan_port(configed_access_vlan, ifIndex) != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_err("Fail to del static fdb on port %d vlan %d\r\n", ifIndex, configed_access_vlan);
		ret = ASD_RETURN_CODE_DEL_STATIC_FDB_FAIL;
		goto reply;
	}
	
	syslog_ax_asd_dbg("Move port %d to vlan %d untag\r\n", ifIndex, vlanId);
	if(vlanId !=0 && vlanId != configed_access_vlan){
		ret = npd_asd_mv_port_to_vlan(ifIndex, vlanId);
		if(ret != ASD_RETURN_CODE_SUCCESS){
			ret = ASD_RETURN_CODE_CHG_VLAN_FAIL;
			goto reply;
		}
	}
	
	syslog_ax_asd_dbg("Dot1x mode is port based, Enable auto learn\r\n");
	ret = npd_asd_set_port_auth_state(ifIndex, vlanId, TRUE);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		ret = ASD_RETURN_CODE_SET_AUTH_STAT_FAIL;
	}
	
reply:	
	//syslog_ax_asd_dbg("auth fail:Send configured access vlan %d to asd\r\n",configed_access_vlan);
	return npd_asd_send_port_vlan(ifIndex, configed_access_vlan, SET_CONFIGED_ACCESS_VLAN,ret);
	
}


int npd_asd_mv_port_access_vlan_allow(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId, curr_vlan;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;
	if(npd_vlan_port_pvid_get(ifIndex, &curr_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
		goto reply;
	}

	if(npd_fdb_static_entry_del_by_vlan_port(curr_vlan, ifIndex) != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_err("Fail to del static fdb on port %d vlan %d\r\n", ifIndex, curr_vlan);
		ret = ASD_RETURN_CODE_ERROR;
		goto reply;
	}
	
	syslog_ax_asd_dbg("Move port %d to vlan %d untag\r\n", ifIndex, vlanId);
	/*check if port already in the vlan untag*/
	if(vlanId !=0 && vlanId != curr_vlan){
		ret = npd_asd_mv_port_to_vlan(ifIndex, vlanId);
		if(ret != ASD_RETURN_CODE_SUCCESS){
			ret = ASD_RETURN_CODE_CHG_VLAN_FAIL;
			goto reply;
		}
	}
	else{
		vlanId = curr_vlan;
	}

	syslog_ax_asd_dbg("Dot1x mode is port based, Enable auto learn\r\n");
	ret = npd_asd_set_port_auth_state(ifIndex, vlanId, TRUE);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		ret = ASD_RETURN_CODE_SET_AUTH_STAT_FAIL;
	}
reply:	
	return npd_asd_send_port_vlan(ifIndex, vlanId, NPD_ASD_SET_RESULT,ret);
}

int npd_asd_mv_port_access_vlan_reinit(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId, curr_vlan;
	int ret;

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;
	if(npd_vlan_port_pvid_get(ifIndex, &curr_vlan)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
		goto reply;
	}

	if(npd_fdb_static_entry_del_by_vlan_port(curr_vlan, ifIndex) != FDB_RETURN_CODE_SUCCESS){
		syslog_ax_asd_err("Fail to del static fdb on port %d vlan %d\r\n", ifIndex, curr_vlan);
		ret = ASD_RETURN_CODE_DEL_STATIC_FDB_FAIL;
		goto reply;
	}
	
	
	syslog_ax_asd_dbg("Move port %d to vlan %d untag\r\n", ifIndex, vlanId);
	/*check if port already in the vlan untag*/
	if((vlanId !=0 ) && (vlanId != curr_vlan)){
		ret = npd_asd_mv_port_to_vlan(ifIndex, vlanId);
		if(ret != ASD_RETURN_CODE_SUCCESS){
			ret = ASD_RETURN_CODE_CHG_VLAN_FAIL;
			goto reply;
		}
	}
	else{
		vlanId = curr_vlan;
	}
	
	syslog_ax_asd_dbg("Disable dot1x, Enable auto learn\r\n");
	ret = npd_asd_set_port_auth_state(ifIndex, vlanId, TRUE);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		ret = ASD_RETURN_CODE_SET_AUTH_STAT_FAIL;
	}
	
reply:	
	return npd_asd_send_port_vlan(ifIndex, vlanId, NPD_ASD_SET_RESULT,ret);

}

int npd_asd_get_port_info(struct npd_mng_asd * asd_msg)
{
	unsigned int ifIndex;
	unsigned short vlanId = DEFAULT_VLAN_ID;
	int ret;

	ifIndex = asd_msg->ifIndex;
	if(npd_vlan_port_pvid_get(ifIndex, &vlanId)!= VLAN_RETURN_CODE_ERR_NONE){
		syslog_ax_asd_err("Fail to get port %d's access vlan\r\n", ifIndex);
		ret = ASD_RETURN_CODE_GET_PVID_FAIL;
	}	
		
	return npd_asd_send_port_vlan(ifIndex, vlanId, WIRED_JOIN,ret);
}

#ifdef HAVE_PORTAL
#define PORTAL_ACL_CREATE  1
#define PORTAL_ACL_DESTORY 2

void npd_asd_inf_portal_refresh(unsigned int netif_index)
{
    unsigned int ret = 0;
	unsigned int ifindex = 0;
    NPD_L3INTF_ADDR intfAddr;
	unsigned char intfname[32] = {0};
    unsigned char ipaddress[16] = {0};
    char command_str[256];
	
	ret = npd_intf_gindex_exist_check(netif_index, &ifindex);
	if(ret != NPD_TRUE)
	{
		return INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST;
	}

	npd_netif_index_to_name(netif_index, intfname);
    intfAddr.ifindex = ifindex;
    for((ret = dbtable_hash_head_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index)); 
    	0 == ret ;
    	(ret = dbtable_hash_next_key(l3intf_addr_if_index, &intfAddr, &intfAddr, npd_l3_intf_addr_filter_by_index)))
    {
        lib_get_string_from_ip(ipaddress, intfAddr.ipAddr);
        memset(command_str, 0, 256);
	    sprintf(command_str, "2>/dev/null sudo /sbin/iptables -t nat -D PREROUTING -i %s -p tcp --dport 80 -j DNAT --to-destination %s:8001",intfname,ipaddress);
	    system(command_str);
    }   
}

int npd_asd_portal_acl_set_all(unsigned int flag, unsigned char *name)
{ 
	int ret = 0; 
	switch_port_db_t switch_port = {0};
    npd_key_database_lock();
	ret = dbtable_hash_head(switch_ports_hash, NULL, &switch_port, NULL);
	while(0 == ret)
	{
		if(PORTAL_ACL_CREATE == flag)
			service_policy_create(name, ACL_DIRECTION_INGRESS_E, switch_port.global_port_ifindex);
		else if(PORTAL_ACL_DESTORY == flag)
			service_policy_destroy(name, ACL_DIRECTION_INGRESS_E, switch_port.global_port_ifindex);

		ret = dbtable_hash_next(switch_ports_hash, &switch_port, &switch_port, NULL);
	}
    npd_key_database_unlock();
	return 0;
}

int npd_portal_srv_ip_set(unsigned char isEnable, unsigned int srvID, unsigned int ipaddr)
{	
	struct class_map_index_s class_map = {0};
    policy_map_index_t policy_map = {0};
    int ret = 0;
    char name[32]={0};
    char ip_str[32]={0};
	
    sprintf(name,"PORTAL_SRV_%d",srvID);
    if(isEnable)
    {
        /*create permit acl for PORTAL server and deploy it*/
        class_map_create(name);
        class_map_find_by_name(name, &class_map);
        lib_get_string_from_ip(ip_str, ipaddr);
        class_map_add_match(&class_map, "dstip",ip_str, "0xffff");
        policy_map_create(name);
        policy_map_class(name, name);
        policy_map_find_by_name(name, &policy_map);
        policy_map_add_set(&policy_map, "drop", "0"); 
		npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name);
    }
    else
    {   
    	npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name);
        policy_map_delete(name);
        class_map_delete(name);        
    }

    return 0;
}

int npd_intf_portal_set(char isEnable, int netif_index)
{
	unsigned short vid = 0;
	struct class_map_index_s class_map = {0};
    policy_map_index_t policy_map = {0};
    int ret = 0;
    char name[32]={0}, name_tmp[32] = {0};
    char ip_str[32]={0};
	char vlan_str[32] = {0};
	
    sprintf(name,"PORTAL_INTF_%.8x",netif_index);
	if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
	{
		vid = npd_netif_vlan_get_vid(netif_index);		
		sprintf(vlan_str, "%d", vid);
		if(isEnable)
		{
			/*add trap-to-cpu for http packet*/
			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_1", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
			class_map_add_match(&class_map, "protocol", "6", "0xffff");
    		class_map_add_match(&class_map, "dstl4port", "80", "0xffff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "trap", "0xffff");
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);
			
			/*add permit for DHCP packet*/
			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_2", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
			class_map_add_match(&class_map, "protocol", "17", "0xffff");
    		class_map_add_match(&class_map, "dstl4port", "67", "0xffff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "drop", "0");   
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);

			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_3", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
			class_map_add_match(&class_map, "protocol", "17", "0xffff");
    		class_map_add_match(&class_map, "dstl4port", "68", "0xffff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "drop", "0");   
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);

			/*add permit for DNS packet*/
			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_4", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
			class_map_add_match(&class_map, "protocol", "17", "0xffff");
			class_map_add_match(&class_map, "dstl4port", "53", "0xffff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "drop", "0");      
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);

			/*add permit for ARP packet*/
            memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_5", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
			class_map_add_match(&class_map, "ethertype", "arp", "0xffff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "drop", "0");      
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);

			/*add discard for all packet*/
			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_6", name);
			class_map_create(name_tmp);
	        class_map_find_by_name(name_tmp, &class_map);
	        class_map_add_match(&class_map, "outer-vlan", vlan_str, "0xfff");
	        policy_map_create(name_tmp);
	        policy_map_class(name_tmp, name_tmp);
	        policy_map_find_by_name(name_tmp, &policy_map);
	        policy_map_add_set(&policy_map, "drop", "1");      
			npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name_tmp);
		}
		else
		{
			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_1", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp); 

			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_2", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp); 

			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_3", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp); 

			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_4", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp); 

            memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_5", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp); 

			memset(name_tmp, 0, 32);
			sprintf(name_tmp, "%s_6", name);
			npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name_tmp);
	        policy_map_delete(name_tmp);
	        class_map_delete(name_tmp);
		}
	}

	return 0;
}

int npd_asd_portal_sta_set(unsigned int isAdd, unsigned int sta_ipaddr)
{
	struct class_map_index_s class_map = {0};
    policy_map_index_t policy_map = {0};
    int ret = 0;
    char name[32]={0};
    char ip_str[32]={0};
	
    sprintf(name,"PORTAL_STA_%.8x",sta_ipaddr);
    if(isAdd)
    {
        /*create permit acl for PORTAL server and deploy it*/
        class_map_create(name);
        class_map_find_by_name(name, &class_map);
        lib_get_string_from_ip(ip_str, sta_ipaddr);
        class_map_add_match(&class_map, "srcip",ip_str, "0xffff");
        policy_map_create(name);
        policy_map_class(name, name);
        policy_map_find_by_name(name, &policy_map);
        policy_map_add_set(&policy_map, "drop", "0");      
		npd_asd_portal_acl_set_all(PORTAL_ACL_CREATE, name);     
    }
    else
    {        
        npd_asd_portal_acl_set_all(PORTAL_ACL_DESTORY, name);
        policy_map_delete(name);
        class_map_delete(name);        
    }

    return 0;
}


int npd_asd_set_portal(struct npd_mng_asd *asd_msg)
{
	unsigned int srvID, changed = 0;	
	npd_asd_portal_srv portalsrvConf;

	srvID = asd_msg->ifIndex;
	if(srvID < MAX_PORTAL_SERVER_NUM)
	{			
		npd_asd_portal_conf_get(srvID, &portalsrvConf);
		portalsrvConf.enable = asd_msg->enable;
		portalsrvConf.srv_addr = asd_msg->ipaddr;
		portalsrvConf.srv_port = asd_msg->l4port;
		npd_asd_portal_conf_set(srvID, &portalsrvConf);

		npd_portal_srv_ip_set(asd_msg->enable, srvID, asd_msg->ipaddr);
	}
	
	return 0;
}

int npd_asd_del_portal(struct npd_mng_asd *asd_msg)
{
	int srvID, changed = 0;	

	srvID = asd_msg->ifIndex;
	if(srvID < MAX_PORTAL_SERVER_NUM)
	{
		npd_asd_portal_conf_del(srvID);
	}

	npd_portal_srv_ip_set(FALSE, srvID, asd_msg->ipaddr);
	
	return 0;
}

DBusMessage * npd_dbus_asd_set_portal_client_bypass(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	unsigned int ret = MATCH_RETURN_CODE_SUCCESS;
    unsigned int enable;
    unsigned int ipv4;
    portal_user_info_s user1;
    portal_user_info_s user2;
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                 DBUS_TYPE_UINT32, &enable,
								 DBUS_TYPE_UINT32, &ipv4,
	                             DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if(enable == TRUE)
    {
        ret = npd_asd_portal_sta_set(TRUE, ipv4);
        
        user1.ipaddr = ipv4;
        ret = dbtable_hash_search(npd_asd_portal_bypass_name, &user1, NULL, &user2);
        if(0 == ret)
        {
            ret = dbtable_hash_update(npd_asd_portal_bypass_name, NULL, &user1);
        }
        else 
            ret = dbtable_hash_insert(npd_asd_portal_bypass_name, &user1);     
    }
    else
    {
        ret = npd_asd_portal_sta_set(FALSE, ipv4);

        user1.ipaddr = ipv4;
        ret = dbtable_hash_search(npd_asd_portal_bypass_name, &user1, NULL, &user2);
        
        ret = dbtable_hash_delete(npd_asd_portal_bypass_name, &user1, &user1);
    }
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32, &ret);
    return reply;
} 

DBusMessage * npd_dbus_asd_show_portal_bypass(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
	int ret = -1;
    unsigned int num;
	unsigned int rc = INTERFACE_RETURN_CODE_SUCCESS;
    portal_user_info_s   userinfo;
    
	memset(&userinfo, 0, sizeof(portal_user_info_s));
    dbus_error_init(&err);
    num = dbtable_hash_count(npd_asd_portal_bypass_name);
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rc);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &num);
    ret = dbtable_hash_head(npd_asd_portal_bypass_name, NULL, &userinfo, NULL);
    while(ret == 0)
    {		      
		if(userinfo.ipaddr)
		{
            dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &userinfo.ipaddr);                    
		}
        
		ret = dbtable_hash_next(npd_asd_portal_bypass_name, &userinfo, &userinfo, NULL);
    }
	
    return reply;
} 

DBusMessage * npd_dbus_asd_portal_showrunning(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
	int ret = -1;
	portal_user_info_s   userinfo;
	char    netif_name[MAX_IFNAME_LEN];
	char *show_buffer = NULL;
    char *cursor = NULL;
    unsigned char ipaddr[32];
    int length = 0;
	show_buffer = malloc(MAX_L3INTF_NUMBER*512);
	if(show_buffer == NULL)
	{
		return NULL;
	}

    cursor = show_buffer;
	memset(show_buffer, 0, MAX_L3INTF_NUMBER*512);
	memset(&userinfo, 0, sizeof(portal_user_info_s));
	ret = dbtable_hash_head(npd_asd_portal_bypass_name, NULL, &userinfo, NULL);
    while(ret == 0)
    {
		if(userinfo.ipaddr)	
		{
            memset(ipaddr, 0, sizeof(ipaddr));
            lib_get_string_from_ip(ipaddr, userinfo.ipaddr);
            length += sprintf(cursor, "portal client-bypass %s\n", ipaddr);
		    cursor = show_buffer + length;
		}
		ret = dbtable_hash_next(npd_asd_portal_bypass_name, &userinfo, &userinfo, NULL);
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &show_buffer);
	
    free(show_buffer);
    length = 0;
    
    return reply;
}

#endif

#ifdef HAVE_CAPWAP_ENGINE

npd_wlan_client_key_t wlan_client_key;
npd_wlan_client_t 	wlan_client;


int npd_asd_db_set_ufdb_entry(struct npd_asd_ufdb_s * ufdb_entry)
{
	int ret = 0;
	struct npd_asd_ufdb_s pre_conf;

	if( ufdb_entry == NULL )
		return CAPWAP_RETURN_CODE_ERROR;

	if( 0 == dbtable_hash_search(npd_asd_ufdb_hash_index,ufdb_entry, NULL, &pre_conf))
	{
		ret = dbtable_hash_update(npd_asd_ufdb_hash_index, &pre_conf, ufdb_entry);
	}
	else
	{
		ret = dbtable_hash_insert(npd_asd_ufdb_hash_index, ufdb_entry);
	}

	return ret;
}

int npd_asd_db_del_ufdb_entry(struct npd_asd_ufdb_s *ufdb_entry)
{
	if( ufdb_entry == NULL )
		return CAPWAP_RETURN_CODE_ERROR;

	return dbtable_hash_delete(npd_asd_ufdb_hash_index, ufdb_entry, ufdb_entry);
}
int npd_asd_db_get_ufdb_entry(struct npd_asd_ufdb_s *ufdb_entry)
{
	return dbtable_hash_search(npd_asd_ufdb_hash_index, ufdb_entry, NULL, ufdb_entry);
}

int npd_asd_db_get_first_entry(struct npd_asd_ufdb_s *ufdb_entry)
{
	return dbtable_hash_head(npd_asd_ufdb_hash_index, ufdb_entry, ufdb_entry, NULL);
}

int npd_asd_db_get_next_entry(struct npd_asd_ufdb_s *ufdb_entry)
{
	return dbtable_hash_next(npd_asd_ufdb_hash_index, ufdb_entry, ufdb_entry, NULL);
}


int npd_asd_get_station_on_ap(unsigned int ap_id, unsigned int *station_num)
{
	struct npd_asd_ufdb_s ufdb_entry;
	unsigned int count= 0;
	unsigned int ap_index = 0;
	int ret = 0;


	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	ret = dbtable_hash_head(npd_asd_ufdb_hash_index, &ufdb_entry, &ufdb_entry, NULL);
	while(0 == ret){
		ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
		if(ap_index == ap_id){
			count++;
		}
		ret = dbtable_hash_next(npd_asd_ufdb_hash_index, &ufdb_entry, &ufdb_entry, NULL);
	}

	*station_num = count;

	return 0;
}


int npd_asd_add_ufdb_entry(struct npd_mng_asd * asd_msg)
{
	unsigned int 			unit_id = 0;
	struct npd_asd_ufdb_s	ufdb_entry;
	npd_wlan_client_key_t 	key;
	npd_wlan_client_t		client;
	unsigned short			pvid = 0;
	unsigned int			ap_index;
    struct arp_snooping_item_s          dbItem;
    struct npd_capwap_global_conf_s     global_conf ;           
    int ret = ASD_RETURN_CODE_SUCCESS;


	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	memset(&key, 	  	0, sizeof(key));
	memset(&client,   	0, sizeof(client));
	
    npd_capwap_get_global_conf(0, &global_conf);
    
	memcpy(ufdb_entry.mac, asd_msg->mac, MAC_ADDR_LEN);
	/*add ufdb software entry*/
	ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
	if(ASD_RETURN_CODE_SUCCESS == ret){
        if (global_conf.global_status == NPD_TRUE){
    		/*initialize the key to ufdb table*/
    		memcpy(key.mac, asd_msg->mac, MAC_ADDR_LEN);
    		key.vid = ufdb_entry.pvid_when_add;

    		/*del ufdb  table hardware entry*/
    		syslog_ax_asd_dbg("func:%s, line:%d\n", __func__, __LINE__);
    		nam_wlan_client_del(unit_id, &key);
	    }
		ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
		ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_STA_LOGOUT, (void*)ap_index);
	}

	ufdb_entry.netif_index		= asd_msg->ifIndex;
	memcpy(ufdb_entry.mac, asd_msg->mac, MAC_ADDR_LEN);
	ufdb_entry.link_state 		= asd_msg->link_state;


	npd_capwap_get_pvid_by_netif(asd_msg->ifIndex, &pvid);
	ufdb_entry.pvid_when_add	= pvid;
    /*if arp is already exist, notify arp to update*/
    memset(&dbItem, 0, sizeof(struct arp_snooping_item_s));

    if (0 == npd_arp_snooping_find_item_bymac(ufdb_entry.mac, &dbItem))
    {
        /*tunnel mode*/
        if (global_conf.global_status == NPD_TRUE)
        {
            dbItem.ifIndex = asd_msg->ifIndex;
        }
        /*local_bridge mode*/
        else
        {
            nam_fdb_get_index_bymac(ufdb_entry.mac, ufdb_entry.pvid_when_add, &(dbItem.ifIndex));
        }
        /*此处似应该更新arp的物理出接口,高通芯片没有问题，但其他芯片在此处会出现问题，
        但也可以在FDB学习的时候进行处理，那样更通用一些*/
        //npd_arp_snooping_learning(&dbItem);        
    }
	/*add ufdb software entry*/
	ret = npd_asd_db_set_ufdb_entry(&ufdb_entry);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		syslog_ax_asd_dbg("Fail to add ufdb entry\r\n");
		return ASD_RETURN_CODE_SUCCESS;
	}
	
	{
        if (global_conf.global_status == NPD_TRUE)
        {
            /*get bssid table entry tunnel id*/
    		npd_capwap_get_bssid_by_netif(ufdb_entry.netif_index, client.wtp);
			client.cflag = asd_msg->flags & FAL_WLAN_FLAG_CLIENT_ISO_EN;
    		
    		/*initialize the key to ufdb table*/
    		memcpy(key.mac, asd_msg->mac, MAC_ADDR_LEN);
    		key.vid = pvid;

    		/*add ufdb  table hardware entry*/
    		syslog_ax_asd_dbg("func:%s, line:%d\n", __func__, __LINE__);
    		nam_wlan_client_add(unit_id, &key, &client);
        }
	}
	
	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
	ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_STA_LOGIN, (void*)ap_index);
	syslog_ax_asd_dbg("%s Wifi event process result: %d\n", __func__, ret);


	return ASD_RETURN_CODE_SUCCESS;
	
}

int npd_asd_del_ufdb_entry(struct npd_mng_asd * asd_msg)
{
	struct npd_asd_ufdb_s 	ufdb_entry;
	npd_wlan_client_key_t	key;
	unsigned int 			unit_id	= 0;
	unsigned short			pvid = 0;
	int 					ret = 0;
	unsigned int			ap_index;

	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	memcpy(ufdb_entry.mac, asd_msg->mac, MAC_ADDR_LEN);

	/*delete software entry*/
	ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		syslog_ax_asd_dbg("ufdb entry not exist\r\n");
		return ASD_RETURN_CODE_SUCCESS;
	}
	
	/*if link state is true, then delete hardware entry*/
	if(TRUE == ufdb_entry.link_state){	
		/*initialize the key to ufdb table*/
		memcpy(key.mac, asd_msg->mac, MAC_ADDR_LEN);
		key.vid = ufdb_entry.pvid_when_add;

		/*delete ufdb table entry according to mac address*/
		nam_wlan_client_del(unit_id, &key);
	}
	
	/*delete software entry*/
	npd_asd_db_del_ufdb_entry(&ufdb_entry);	
	
	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
	ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_STA_LOGOUT, (void*)ap_index);
	syslog_ax_asd_dbg("%s Wifi event process result: %d\n", __func__, ret);

	return ASD_RETURN_CODE_SUCCESS;
	
	
}

int npd_capwap_get_netif_by_mac_vid(unsigned char *mac, unsigned short *vid, unsigned int *netif_index )
{
	int ret = CAPWAP_RETURN_CODE_SUCCESS;
	struct npd_asd_ufdb_s  ufdb_entry;

	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	memcpy(ufdb_entry.mac, mac, MAC_ADDR_LEN);

	/*delete software entry*/
	ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
	if(ret != ASD_RETURN_CODE_SUCCESS){
		*netif_index = -1;
		npd_syslog_dbg("func:%s line:%d netif_index:%d\r\n", __func__, __LINE__, *netif_index);
		return ret;
	}

	*netif_index = ufdb_entry.netif_index;
	npd_capwap_get_pvid_by_netif(ufdb_entry.netif_index, vid);
	npd_syslog_dbg("func:%s line:%d netif_index:%d\r\n", __func__, __LINE__, *netif_index);
	return CAPWAP_RETURN_CODE_SUCCESS;
}



#endif //HAVE_CAPWAP_ENGINE

/*******************************************************************************
 * npd_asd_recvmsg_proc
 *
 * DESCRIPTION:
 *		 config dev according to running result of asd protocol
 *
 * INPUTS:
 * 		asd_msg - ASD notify message
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		ASD_RETURN_CODE_SUCCESS - config success
 *
 * COMMENTS:
 *  
 **
 ********************************************************************************/
unsigned int npd_asd_recvmsg_proc(struct npd_mng_asd * asd_msg)
{
	unsigned int ret = ASD_RETURN_CODE_SUCCESS;
	unsigned int ifIndex = 0;
	unsigned short vlanId = 0;

	syslog_ax_asd_dbg("npd receive message from asd:get action: %d, vlan_id %d, ifindex 0x%x, \
						mac %02x:%02x:%02x:%02x:%02x:%02x\n",
						asd_msg->action, asd_msg->vlanId, asd_msg->ifIndex, \
						asd_msg->mac[0], asd_msg->mac[1], asd_msg->mac[2], \
						asd_msg->mac[3], asd_msg->mac[4], asd_msg->mac[5]);

	ifIndex = asd_msg->ifIndex;
	vlanId = asd_msg->vlanId;

	switch(asd_msg->action)
	{
		case NPD_MSG_ASD_ENABLE_SYSTEM_AUTH_CONTROL:
			ret = npd_asd_system_auth_control_on(asd_msg);
			break;
		case NPD_MSG_ASD_DISABLE_SYSTEM_AUTH_CONTROL:
			ret = npd_asd_system_auth_control_off(asd_msg);
			break;
		case NPD_MSG_ASD_SWITCHOVER:
			ret = npd_asd_switchover(asd_msg);
			break;	
		case NPD_MNG_ASD_ADD_USER:
			ret = npd_asd_user_log_on(asd_msg);
			break;			
		case NPD_MNG_ASD_DEL_USER:
			ret = npd_asd_user_log_off(asd_msg);
			break;
		case NPD_MNG_ASD_STA_ARP_ADD:
			break;
		case NPD_MNG_ASD_STA_ARP_DELETE:
			ret = npd_asd_sta_arp_delete(asd_msg);
			break;
		case NPD_MNG_ASD_ENABLE_DOT1X:
			ret = npd_asd_port_enable_dot1x(asd_msg);
			break;
		case NPD_MNG_ASD_DISABLE_DOT1X:
			ret = npd_asd_port_disable_dot1x(asd_msg);
			break;
		case NPD_MNG_ASD_SET_MAC_AUTH:
 			ret = npd_asd_set_port_mac_auth(asd_msg);
			break;
		case NPD_MNG_ASD_SET_PORT_AUTH:
			ret = npd_asd_port_enable_dot1x(asd_msg);
			break;
        case NPD_MNG_ASD_SET_MAB_AUTH:
			ret = npd_asd_set_port_mab_auth(asd_msg);
			break; 
		case NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_DENY:
			ret = npd_asd_mv_port_access_vlan_deny(asd_msg);
			break;
		case NPD_MNG_ASD_MV_PORT_TO_GUEST_VLAN:
			ret = npd_asd_mv_port_guest_vlan(asd_msg);
			break;
		case NPD_MNG_ASD_MV_PORT_TO_AUTH_FAIL_VLAN:
			ret = npd_asd_mv_port_auth_fail_vlan(asd_msg);
			break;
		case NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_ALLOW:
			ret = npd_asd_mv_port_access_vlan_allow(asd_msg);
			break;
		case NPD_MNG_ASD_MV_PORT_TO_CONFIGED_VLAN_AND_REINIT:
			ret = npd_asd_mv_port_access_vlan_reinit(asd_msg);
			break;
#ifdef HAVE_CAPWAP_ENGINE			
		case NPD_MNG_ASD_UFDB_ENTRY_ADD:
			ret = npd_asd_add_ufdb_entry(asd_msg);
			break;
		case NPD_MNG_ASD_UFDB_ENTRY_DEL:
			ret = npd_asd_del_ufdb_entry(asd_msg);
			break;
#endif //HAVE_CAPWAP_ENGINE	
		case NPD_MNG_ASD_GET_PORT_INFO:
			ret = npd_asd_get_port_info(asd_msg);
			break;
#ifdef HAVE_PORTAL
		case NPD_MNG_ASD_SET_PORTAL:
			ret = npd_asd_set_portal(asd_msg);
			break;
		case NPD_MNG_ASD_DEL_PORTAL:
			ret = npd_asd_del_portal(asd_msg);
			break;
#endif
		default :
		{
			syslog_ax_asd_err("npd can NOT proccess the running result of Protocol");
			break;
		}
	}
	return ret;
}

int npd_asd_msg_handler(char *msg, int len)
{
	WIRED_TableMsg *recvMsg = (WIRED_TableMsg *)msg;
	struct npd_mng_asd asd_msg;
	if(npd_asd_pid < 0)
	{
    	npd_asd_pid = syscall(SYS_gettid);
	}
	
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return 0;
	}
	memset(&asd_msg, 0, sizeof(struct npd_mng_asd));
	npd_asd_get_convert_mng(&asd_msg, recvMsg);
	npd_asd_recvmsg_proc(&asd_msg);
	return 0;
}

 int npd_asd_table_init()
 {
 	int ret;
	db_table_t *db;
	
	ret = create_dbtable( NPD_ASD_HASHTBL_NAME, NPD_ASD_TABLE_SIZE, sizeof(struct npd_asd_item_s),\
					npd_asd_dbtbl_handle_update, 
					NULL,
					npd_asd_dbtbl_handle_insert, 
					npd_asd_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(db));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd igmp snp database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ifindex", db,NPD_ASD_HASH_IFIDX_SIZE, npd_asd_keyifidx_generate,\
													npd_asd_compare, &npd_asd_hashifidx_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd igmp snp hash table fail\n");
		return NPD_FAIL;
	}	

#ifdef HAVE_CAPWAP_ENGINE
	ret = create_dbtable( NPD_ASD_UFDB_HASHTBL_NAME, NPD_ASD_UFDB_TABLE_SIZE, sizeof(struct npd_asd_ufdb_s),\
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(db));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd asd ufdb database fail\n");
		return NPD_FAIL;
	}
	ret = dbtable_create_hash_index("ufdb_mac", db,NPD_ASD_UFDB_HASH_SIZE, npd_asd_ufdb_mac_generate,\
													npd_asd_ufdb_compare, &npd_asd_ufdb_hash_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd asd ufdb hash table fail\n");
		return NPD_FAIL;
	}	
#endif //HAVE_CAPWAP_ENGINE

#ifdef HAVE_PORTAL
	ret = create_dbtable( "npdAsdPortalDb", 4, sizeof(npd_asd_portal_srv),\
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL, NULL,
					NULL, NULL, NULL, 
					DB_SYNC_ALL,
					&(db));
	
	ret = dbtable_create_array_index( "npdAsdPortalIndex" ,db, &npd_asd_portal_index);

    ret = create_dbtable( "npdAsdPortalClientBypassDb", 256, sizeof(portal_user_info_s),\
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL, NULL,
					NULL, NULL, NULL, 
					DB_SYNC_ALL,
					&(db));
	
	//ret = dbtable_create_array_index( "npdAsdPortalClientBypassIndex" ,db, &npd_asd_portal_bypass_index);

    ret = dbtable_create_hash_index("npdAsdPortalClientBypassName", db, 256, npd_asd_portal_bypass_ip_generate,\
													npd_asd_portal_bypass_ip_compare, &npd_asd_portal_bypass_name);
#endif

	syslog_ax_asd_dbg("Register npd asd netif event notifier\n");
	register_netif_notifier(&npd_asd_netif_notifier);
	return NPD_OK;
 }


/**********************************************************************************
*npd_asd_init()
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*	
*DESCRIPTION:
*	ASD init Global Flag 
*	and user table database
*
***********************************************************************************/
void npd_asd_init(void)
{
	
	npd_asd_table_init();
}

/**********************************************************************************
*npd_asd_mng_thread_dgram()
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*	
*DESCRIPTION:
*	ASD command message handle thread
*
***********************************************************************************/
int npd_asd_msg_init(void)
{
	int sock = 0;
#if 0
	int recv_len = 0;
	struct npd_mng_asd *asd_msg = NULL;
	char buf[ASD_MNG_MAX_SIZE];
	fd_set rfds;
	int ret;
#endif
	npd_asd_init();

	/*create socket communication*/
	if(ASD_RETURN_CODE_SUCCESS != npd_asd_sock_init(&sock))
	{
		syslog_ax_asd_err("Failed to create asd manage thread socket.\r\n");
		return -1;
	}

	syslog_ax_asd_dbg("Create ads msg socket fd %d ok\n",sock);
	
	npd_asd_fd = sock;

	npd_app_msg_socket_register(sock, "asdMsg", npd_asd_msg_handler, 1024);
#if 0
	asd_msg = (struct npd_mng_asd* )buf;
	while(1)
	{
		memset(buf,0,sizeof(char)* ASD_MNG_MAX_SIZE);
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		switch(select(sock+1,&rfds,NULL,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				break;
			default:
				if(FD_ISSET(sock,&rfds))
				{
					ret = npd_asd_recv_info(asd_msg,ASD_MNG_MAX_SIZE,&recv_len);					
					if( 0 == recv_len ){
						syslog_ax_asd_err("ASD:recieve msg len is 0\r\n");
						break;
					}
					
					if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
						syslog_ax_asd_dbg("The slot is not Active master\r\n");	
						break;
					}

					ret = npd_asd_recvmsg_proc(asd_msg);					
				}
		}
		if(0 == recv_len)
			break;
	}
	close(sock);
	return NULL;
#endif
    return 0;
}


#ifdef __cplusplus
}
#endif
#endif

