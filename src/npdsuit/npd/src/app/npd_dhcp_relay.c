
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dhcp_relay.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		dhcp relay for NPD module.
*
* DATE:
*		12/08/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.0 $	

*******************************************************************************/
#ifdef HAVE_DHCP_RELAY
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

#include "npd_dhcp_relay.h"
/*********************************************************
*	global variable define											*
**********************************************************/

unsigned int dhcp_relay_max_hops = NPD_DHCP_RELAY_DEF_HOPS_LIMIT;

unsigned int dhcp_relay_global_no = 0;

int dhcpRelayToServer = 0, dhcpRelayToAgent = 0;

db_table_t *npd_dhcpr_server_db = NULL;

hash_table_index_t *npd_dhcpr_server_hash_index = NULL;

#define NPD_DHCP_RELAY_GLOBAL_STATUS_LEN    1024

#define NPD_DHCP_RELAY_GLOBAL_STATUS_NAME "dhcp_relay_global_table"
#define NPD_DHCP_RELAY_GLOBAL_STATUS_SIZE 1

db_table_t   *npd_dhcp_relay_global_status_table = NULL;
#define NPD_DHCP_RELAY_GLOBAL_STATUS_INDEX_NAME "npd_dhcp_relay_global_index"

array_table_index_t *npd_dhcp_relay_global_status_index = NULL;

void npd_dhcpr_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);

void npd_dhcpr_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);


netif_event_notifier_t dhcpr_netif_notifier =
{
    .netif_event_handle_f  = &npd_dhcpr_notify_event,
    .netif_relate_handle_f = &npd_dhcpr_relate_event
};

/*********************************************************
*	extern variable												*
**********************************************************/
extern hash_table_index_t *npd_dhcpr_server_hash_index;

/*********************************************************
*	dhcp relay database                								       *
**********************************************************/

unsigned int npd_dhcp_relay_global_db_initialize()
{
	unsigned int ret;
	ret = create_dbtable( NPD_DHCP_RELAY_GLOBAL_STATUS_NAME, 
							NPD_DHCP_RELAY_GLOBAL_STATUS_SIZE, 
							sizeof(struct npd_dhcp_relay_global_status),
							NULL, 
							NULL,
							NULL, 
							NULL, 
							NULL,
							NULL, NULL, 
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&(npd_dhcp_relay_global_status_table));
	if( 0 != ret )
	{
		syslog_ax_dhcp_snp_err("create npd relay global dbtable fail\n");
		return NPD_FAIL;
	}


    ret = dbtable_create_array_index(NPD_DHCP_RELAY_GLOBAL_STATUS_INDEX_NAME, 
									npd_dhcp_relay_global_status_table,  
									&npd_dhcp_relay_global_status_index);
	
	if( 0  != ret )
	{
		syslog_ax_dhcp_snp_err("create dhcp relay status item fail\n");
		return NPD_FAIL;
	}	
	npd_syslog_dbg("npd dhcp relay global table successfully.\n");
	return DHCP_SNP_RETURN_CODE_OK;
}

unsigned int npd_dhcp_relay_global_status_get
(
	struct npd_dhcp_relay_global_status* user
)
{
    return (0 == dbtable_array_get(npd_dhcp_relay_global_status_index, dhcp_relay_global_no, user)) \
        ? DHCP_RELAY_RETURN_CODE_SUCCESS : DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
}

unsigned int npd_dhcp_relay_global_status_set
(
	struct npd_dhcp_relay_global_status* user
)
{
    return (0 == dbtable_array_update(npd_dhcp_relay_global_status_index, dhcp_relay_global_no, NULL, user)) \
        ? DHCP_RELAY_RETURN_CODE_SUCCESS : DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
}


unsigned int npd_dhcp_relay_global_status_insert
(	
	struct npd_dhcp_relay_global_status *user
)
{
    return (0 == dbtable_array_insert(npd_dhcp_relay_global_status_index, &dhcp_relay_global_no, user)) \
        ? DHCP_RELAY_RETURN_CODE_SUCCESS : DHCP_RELAY_RETURN_CODE_GENERAL_ERROR; 
}

unsigned int npd_dhcpr_server_genkey
(	
	void * temp
)
{
	NPD_DHCPR_SERVER *data;
	unsigned int key = 0;
	if(NULL == temp) {
		return FALSE;
	}
	data = (NPD_DHCPR_SERVER *)temp;


	key = data->netifIndex >> 14;

	key %= NPD_DHCPR_SERVER_HASH_SIZE;

	return key;
}

unsigned int npd_dhcpr_server_compare
(
	void * temp1,
	void * temp2	
)
{
	NPD_DHCPR_SERVER *data1;
	NPD_DHCPR_SERVER *data2;
	unsigned int ret = TRUE;

	if(NULL == temp1 || NULL == temp2 ) {
		return FALSE;
	}
	data1 = (NPD_DHCPR_SERVER *)temp1;
	data2 = (NPD_DHCPR_SERVER *)temp2;

	if( data1->netifIndex != data2->netifIndex)
		ret = FALSE;

	return ret;
}

int npd_dhcpr_server_get_byIndex(unsigned int index, NPD_DHCPR_SERVER *dhcprServer)
{
	NPD_DHCPR_SERVER tmpServer = {0};
	npd_syslog_event("npd_dhcpr_server_get_byIndex index=%d",index);
	tmpServer.netifIndex = index;
	
	return dbtable_hash_search(npd_dhcpr_server_hash_index, &tmpServer, NULL, dhcprServer);
}

int npd_dhcpr_server_set(NPD_DHCPR_SERVER *dhcprServer)
{
	NPD_DHCPR_SERVER tmpServer;
	npd_syslog_event("DHCPR server update: intf %s index 0x%x enDis", dhcprServer->ifName,
												dhcprServer->netifIndex, dhcprServer->enDis );

	if( 0 == dbtable_hash_search(npd_dhcpr_server_hash_index, dhcprServer, NULL, &tmpServer) )
	{
		return dbtable_hash_update(npd_dhcpr_server_hash_index, dhcprServer, dhcprServer);
	}

	return dbtable_hash_insert(npd_dhcpr_server_hash_index, dhcprServer);
}

int npd_dhcpr_server_del(NPD_DHCPR_SERVER *dhcprServer)
{
	NPD_DHCPR_SERVER tmpServer;
	if( 0 == dbtable_hash_search(npd_dhcpr_server_hash_index, dhcprServer, NULL, &tmpServer) )
	{
		return dbtable_hash_delete(npd_dhcpr_server_hash_index, dhcprServer, dhcprServer);
	}

	return 0;
}

unsigned int npd_dhcpr_table_init()
{
	int ret; 
		
	ret = create_dbtable( NPD_DHCPR_SERVER_DB, NPD_DHCPR_SERVER_SIZE, sizeof(NPD_DHCPR_SERVER),\
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
					&(npd_dhcpr_server_db));
	if( 0 != ret )
	{
		syslog_ax_route_err("create npd route database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index(NPD_DHCPR_SERVER_HASH_NAME,npd_dhcpr_server_db,NPD_DHCPR_SERVER_HASH_SIZE,
								npd_dhcpr_server_genkey, npd_dhcpr_server_compare, &npd_dhcpr_server_hash_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd route hash table fail\n");
		return NPD_FAIL;
	}

	npd_syslog_dbg("npd dhcp relay init table successfully.\n");
	
	return NPD_OK;
}


/*********************************************************
*	dhcp relay functions             								      *
**********************************************************/
/**********************************************************************************
 * npd_dhcpr_check_global_status
 *		check DHCP_Relay enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_ENABLE_GBL		- global status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	- global status is disable
 **********************************************************************************/
unsigned int npd_dhcpr_check_global_status
(
	void
)
{
	struct npd_dhcp_relay_global_status user;
	memset(&user,0,sizeof(user));

    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user))
	{
		if (user.dhcp_relay_endis == NPD_DHCP_RELAY_ENABLE) {
			npd_syslog_err("dhcp relay global enable ");
			return TRUE;
		}
		npd_syslog_err("dhcp relay global disable ");
		return FALSE;
	}
	return FALSE;
}


unsigned int npd_dhcp_information_relay_opt82_status_get
(
	unsigned char *relay_opt82_status
)
{
   	struct npd_dhcp_relay_global_status user_r;
    
    memset(&user_r, 0, sizeof(user_r));
    
    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user_r))
	{
	    *relay_opt82_status = user_r.dhcp_relay_opt82_enable;
	}
    else
    {
        return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
    }

    return DHCP_RELAY_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 * npd_dhcp_relay_set_opt82_status
 *		set ip dhcp relay information enable/disable option82 status
 *
 *	INPUT:
 *		unsigned char isEnable
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_relay_set_opt82_status
(
	unsigned char isEnable
)
{
	struct npd_dhcp_relay_global_status user_r;
	memset(&user_r, 0, sizeof(struct npd_dhcp_relay_global_status));

    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user_r))
	{
		user_r.dhcp_relay_opt82_enable = isEnable ? NPD_DHCP_RELAY_OPT82_ENABLE : NPD_DHCP_RELAY_OPT82_DISABLE;
		return ((DHCP_RELAY_RETURN_CODE_SUCCESS == dbtable_array_update(npd_dhcp_relay_global_status_index, dhcp_relay_global_no, NULL, &user_r)) ? \
           DHCP_SNP_RETURN_CODE_OK : DHCP_SNP_RETURN_CODE_ERROR);
	}
	
	return DHCP_SNP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 * npd_dhcp_relay_opt82_enable
 *		set ip dhcp relay information option82 enable global status, and init 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_relay_opt82_enable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = npd_dhcp_relay_set_opt82_status(NPD_DHCP_RELAY_OPT82_ENABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret)
	{
		syslog_ax_dhcp_snp_err("set ip dhcp relay information option82 status enable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_relay_opt82_disable
 *		set ip dhcp relay information enable option82 status, and init 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_relay_opt82_disable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = npd_dhcp_relay_set_opt82_status(NPD_DHCP_SNP_OPT82_DISABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret)
	{
		syslog_ax_dhcp_snp_err("set DHCP-Snooping option82 status disable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 * npd_dhcp_relay_get_opt82_status
 *		get ip dhcp relay information enable/disable option82 status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned char *status
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int npd_dhcp_relay_get_opt82_status
(
	unsigned char *status
)
{
   	struct npd_dhcp_relay_global_status user_r;

   	memset(&user_r, 0, sizeof(struct npd_dhcp_relay_global_status));

	if (!status)
	{
		syslog_ax_dhcp_snp_err("get ip dhcp relay information option82 status error, parameters is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user_r))
	{
		*status = user_r.dhcp_relay_opt82_enable;
	}
    else
    {
        return DHCP_SNP_RETURN_CODE_ERROR;
    }
    
    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * npd_dhcp_relay_check_vlan_status
 *		check DHCP_Relay enable/disable status on special vlan
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_EN_VLAN			- vlan status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_EN_VLAN		- vlan status is disable
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	- check fail, vlan not exist
 **********************************************************************************/
unsigned char npd_dhcpr_check_intf_status
(
	unsigned int netif_index
)
{
	unsigned char ret = FALSE;
	NPD_DHCPR_SERVER tmpSvr;

	if( 0 == npd_dhcpr_server_get_byIndex(netif_index, &tmpSvr) )
	{
		ret = tmpSvr.enDis;
	}

	return ret;
}



int npd_dhcpr_del_iphelper_by_ifindex( unsigned int ifindex )
{
	NPD_DHCPR_SERVER dhcprServer;

	if( 0 == npd_dhcpr_server_get_byIndex(ifindex, &dhcprServer))
	{
		return npd_dhcpr_server_del(&dhcprServer);
	}

	return DHCP_RELAY_RETURN_CODE_SUCCESS;
}


int npd_dhcpr_add_iphelper_by_ifindex( unsigned int ifindex )
{
	NPD_DHCPR_SERVER dhcprServer;
	memset(&dhcprServer,0,sizeof(dhcprServer));
	npd_syslog_dbg("Enter npd_dhcpr_add_iphelper_by_ifindex");
	
	dhcprServer.netifIndex = ifindex;
    /*TODO: fill interface name*/
	if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(ifindex) ) {
		dhcprServer.vid = npd_netif_vlan_get_vid(ifindex);
	}
	else {
		dhcprServer.vid = NPD_PORT_L3INTF_VLAN_ID;
	}	
	if( 0 == npd_dhcpr_server_set(&dhcprServer) )
		return DHCP_RELAY_RETURN_CODE_SUCCESS;

	return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;

}

void npd_dhcpr_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
)
{
	syslog_ax_arpsnooping_dbg("npd notify DHCP Relay index event: index 0x%x event %d\n", netif_index, evt);

    switch(evt)
    {
	    case PORT_NOTIFIER_L3DELETE:
			npd_dhcpr_del_iphelper_by_ifindex(netif_index);
			break;
		case PORT_NOTIFIER_L3CREATE:
			//npd_dhcpr_add_iphelper_by_ifindex(netif_index);
			break;
	    default:
	        break;
    }

    return;
}

void npd_dhcpr_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
)
{
	syslog_ax_arpsnooping_dbg("npd notify DHCP Relay relate event: vlan 0x%x index 0x%x event %d\n", \
											vlan_index, netif_index, event);

	return;

}

unsigned int npd_dhcpr_init()
{
	int ret = 0;
	int on = 1;
	struct sockaddr_in addr;
	struct npd_dhcp_relay_global_status user;
	memset(&user,0,sizeof(user));
	npd_dhcpr_table_init();

	npd_dhcp_relay_global_db_initialize();
	npd_dhcp_relay_global_status_insert(&user);

	dhcpRelayToServer = socket(AF_INET, SOCK_DGRAM, 0);
	
	if( dhcpRelayToServer == -1 )
	{
		npd_syslog_dbg("npd_dhcpr_init: create server socket fail, ret is %d",dhcpRelayToServer);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	
	ret = setsockopt(dhcpRelayToServer, SOL_SOCKET, SO_REUSEADDR,( void * )&on, sizeof(on));
	if( ret < 0 )
	{
		npd_syslog_dbg("npd_dhcpr_init: set socket REUSE fail, ret is %d", ret);
		close(dhcpRelayToServer);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	
	ret = fcntl(dhcpRelayToServer, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		npd_syslog_dbg("npd_dhcpr_init: set socket NONBLOACK fail, ret is %d", ret);
		close(dhcpRelayToServer);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
		
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons(NPD_DHCPR_CLIENT_PORT);
	ret = bind (dhcpRelayToServer, (struct sockaddr *)&addr, sizeof(addr));
	if( ret == -1 )
	{
		npd_syslog_dbg("npd_dhcpr_init: bind socket to addr fail, ret is %d", ret);
		close(dhcpRelayToServer);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	dhcpRelayToAgent = socket(AF_INET, SOCK_DGRAM, 0);
	
	if( dhcpRelayToAgent == -1 )
	{
		npd_syslog_dbg("npd_dhcpr_init: create agent socket fail, ret is %d",dhcpRelayToAgent);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	
	ret = setsockopt(dhcpRelayToAgent, SOL_SOCKET, SO_REUSEADDR,( void * )&on, sizeof(on));
	if( ret == -1 )
	{
		npd_syslog_dbg("npd_dhcpr_init: set socket REUSE fail, ret is %d");
		close(dhcpRelayToAgent);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	ret = fcntl(dhcpRelayToAgent, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		npd_syslog_dbg("npd_dhcpr_init: set socket NONBLOACK fail, ret is %d", ret);
		close(dhcpRelayToAgent);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
		
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons(NPD_DHCPR_SERVER_PORT);
	ret = bind (dhcpRelayToAgent, (struct sockaddr *)&addr, sizeof(addr));
	if( ret == -1 )
	{
		npd_syslog_dbg("npd_dhcpr_init: bind agent socket to addr fail, ret is %d", ret);
		close(dhcpRelayToAgent);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	register_netif_notifier(&dhcpr_netif_notifier);

	return DHCP_RELAY_RETURN_CODE_SUCCESS;
}



/*********************************************************
*	Support API    											      *
**********************************************************/

int npd_dhcp_relay_add_iphelper(unsigned char *ifName, unsigned int ipAddr, unsigned int port)
{
	int i = 0, found = 0, num = -1;
	unsigned int netif_index = 0;
	NPD_DHCPR_SERVER dhcprServer;
	
	if( NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, ifName))
	{
		npd_syslog_err("get netif index by l3 index %s fail", ifName);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	if( 0 != npd_dhcpr_server_get_byIndex(netif_index, &dhcprServer) )
	{
		npd_syslog_dbg("A new dhcpr server is added");
		memset(&dhcprServer, 0, sizeof(NPD_DHCPR_SERVER));
		memcpy(dhcprServer.ifName, ifName, strlen((const char*)ifName));
		dhcprServer.netifIndex = netif_index;
		if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index) ) {
			dhcprServer.vid = npd_netif_vlan_get_vid(netif_index);
		}
		else {
			dhcprServer.vid = NPD_PORT_L3INTF_VLAN_ID;
		}	
	}

	for(i=0; i<MAX_DHCPR_IP_NUM; i++)
	{
		if( dhcprServer.svrAddr[i].sin_addr.s_addr == 0 && num == -1 )
			num = i;
		if( dhcprServer.svrAddr[i].sin_addr.s_addr == ipAddr )
			found = 1;
	}

	if( -1 == num ) {
		return DHCP_RELAY_RETURN_CODE_SERVER_FULL;
	}
	else if( found )
	{
		return DHCP_RELAY_RETURN_CODE_HELPER_ADDRESS_EXIST;
	}
	else {
		dhcprServer.svrAddr[num].sin_family = AF_INET;
		dhcprServer.svrAddr[num].sin_addr.s_addr = ipAddr;
		dhcprServer.svrAddr[num].sin_port = port;
	}
	
	npd_syslog_dbg("Add ip helper: intf %s index 0x%x ip 0x%x port %d", ifName, netif_index, ntohl(ipAddr), port);
	
	if( 0 != npd_dhcpr_server_set( &dhcprServer )) {
		npd_syslog_err("set dhcpr server fail");
		return DHCP_RELAY_RETURN_CODE_SERVER_FULL;
	}

	return DHCP_RELAY_RETURN_CODE_SUCCESS;
}

int npd_dhcp_relay_del_iphelper(unsigned char *ifName, unsigned int ipAddr)
{
	int i = 0, num = 0, del_num = -1;
	unsigned int netif_index = 0;
	NPD_DHCPR_SERVER dhcprServer;
	
	if( NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, ifName))
	{
		npd_syslog_err("get netif index by l3 index %s fail", ifName);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	if( 0 != npd_dhcpr_server_get_byIndex(netif_index, &dhcprServer) )
	{
		return DHCP_RELAY_RETURN_CODE_SERVER_NOT_EXIST;
	}

	for(i=0; i<MAX_DHCPR_IP_NUM; i++)
	{
		if( dhcprServer.svrAddr[i].sin_addr.s_addr == ipAddr )
			del_num = i;
		else if( dhcprServer.svrAddr[i].sin_addr.s_addr != 0 )
			num++;
		
	}

	if( -1 == del_num )
	{
		return DHCP_RELAY_RETURN_CODE_SERVER_NOT_EXIST;
	}
	
	npd_syslog_dbg("Del ip helper: intf %s index 0x%x ip 0x%x port %s", ifName, dhcprServer.netifIndex, 
								ntohl(dhcprServer.svrAddr[del_num].sin_addr.s_addr), dhcprServer.svrAddr[del_num].sin_port );

	if( num == 0 ) {
		memset(dhcprServer.svrAddr,0,sizeof(dhcprServer.svrAddr));
	}
	else {
		memset(&(dhcprServer.svrAddr[del_num]), 0, sizeof(struct sockaddr_in));
	}
	
	if( 0 != npd_dhcpr_server_set( &dhcprServer )) 
	{
		npd_syslog_err("set dhcpr server fail");
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	
	return DHCP_RELAY_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_dhcpr_check_global_status
 *		check DHCP_Relay enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_ENABLE_GBL		- global status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	- global status is disable
 **********************************************************************************/
unsigned int npd_dhcp_relay_enable_global_status
(
	unsigned char enDis
)
{
	struct npd_dhcp_relay_global_status user;
	memset(&user,0,sizeof(user));
    
    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user))
	{

		if (enDis)
		{
			user.dhcp_relay_endis = NPD_DHCP_RELAY_ENABLE;
		}
		else
        {
			user.dhcp_relay_endis = NPD_DHCP_RELAY_DISABLE;
		}

        return npd_dhcp_relay_global_status_set(&user);
	}	

	return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
}


/*******************************************************************************
 * npd_dhcpr_vlan_endis_config
 *
 * DESCRIPTION:
 *   		enable/disable DHCP_Relay in vlan 
 *
 * INPUTS:
 * 		isAdd - enalbe or disable flag 
 *		vid - vlan id
 *
 * OUTPUTS:
 *    	null	
 *
 * RETURNS:
 *		DHCP_RELAY_RETURN_CODE_SUCCESS - config endis success
 *		DHCP_SNP_RETURN_CODE_VLAN_NOT_EXIST - can not find the vlan 
 *		DHCP_RELAY_RETURN_CODE_GENERAL_ERROR_HW - config hardware error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int npd_dhcp_relay_enable_intf_status
(
	unsigned char *ifName,
	unsigned char enDis
)
{
	unsigned int netif_index = 0;
	NPD_DHCPR_SERVER tmpSvr;
	memset(&tmpSvr,0,sizeof(tmpSvr));
	npd_syslog_dbg("Enter npd_dhcp_relay_enable_intf_status: intf %s endis %d\n", ifName, enDis);

	if( ifName == NULL ) {
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}
	else if( NPD_FALSE == npd_intf_netif_get_by_name(&netif_index, ifName) )
	{
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	if( 0 == npd_dhcpr_server_get_byIndex(netif_index, &tmpSvr) )
	{
		if( enDis == tmpSvr.enDis )
		{
			return (enDis == FALSE)? DHCP_RELAY_RETURN_CODE_NOT_VLAN_ENABLE: DHCP_RELAY_RETURN_CODE_VLAN_ENABLE ;
		}
		else
		{
			tmpSvr.enDis = enDis;
		}
	}
	else
	{
		memset(&tmpSvr, 0, sizeof(NPD_DHCPR_SERVER) );
		tmpSvr.netifIndex = netif_index;
		tmpSvr.enDis = enDis;
		memcpy(tmpSvr.ifName,ifName,sizeof(tmpSvr.ifName));
		if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index) ) {
			tmpSvr.vid = npd_netif_vlan_get_vid(netif_index);
		}
		else {
			tmpSvr.vid = NPD_PORT_L3INTF_VLAN_ID;
		}	
	}

	npd_syslog_dbg("Enter npd_dhcp_relay_enable_intf_status: intf 0x%x endis %d\n", tmpSvr.netifIndex, enDis);

	if( 0 == npd_dhcpr_server_set(&tmpSvr) )
		return DHCP_RELAY_RETURN_CODE_SUCCESS;

	return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
}

/**********************************************************************************
 * npd_dhcp_relay_show_running_intf_config
 *		get string of DHCP Snooping show running vlan config
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
void npd_dhcp_relay_show_running_intf_config
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	int i, enter = 0;
	char ifName[32];
	int length = 0;
	char *showStr = NULL;
	char *current = NULL;
	unsigned int ret = 0;
	NPD_DHCPR_SERVER server;
	memset(&server,0,sizeof(server));
	memset(ifName, 0, 32);
	if (NULL == buf || NULL == enDis) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, parameter is null error\n");
		return ;
	}

	showStr = (char*)buf;
	current = showStr;
	npd_syslog_dbg("enter !");
	
	ret = npd_dhcpr_check_global_status();
	if (TRUE == ret)
	{
		if( 0 != dbtable_hash_head(npd_dhcpr_server_hash_index, NULL, &server, NULL) )
		{
			return;
		}

		do{
			if ((length + sizeof("interface %s\n")) < bufLen) 
			{
				npd_netif_index_to_l3intf_name(server.netifIndex, ifName);
				length += sprintf(current, "interface %s\n", ifName);
				current = showStr+ length;
				enter = 1;
			}
			if ((length + sizeof(" ip dhcp relay enable\n")) < bufLen)
			{
				if( server.enDis )
				{
					length += sprintf(current, " ip dhcp relay enable\n");
					current = showStr + length;
				}
			}
			for(i=0;i<MAX_DHCPR_IP_NUM;i++)
			{
				if ((length + sizeof(" ip helper dhcp 255.255.255.255 123456\n")) < bufLen)
				{
					if( server.svrAddr[i].sin_addr.s_addr != 0 )
					{
						char ipstring[16];
						lib_get_string_from_ip(ipstring,server.svrAddr[i].sin_addr.s_addr);
						length += sprintf(current, " ip helper dhcp %s %d\n",ipstring, server.svrAddr[i].sin_port);
						current = showStr + length;
					}
				}
			}
			if(enter)
			{
				if ((length + sizeof(" exit\n")) < bufLen) 
				{
					length += sprintf(current, " exit\n");
					current = showStr+ length;
					enter = 0;
				}
			}
		}
		while( 0 == dbtable_hash_next(npd_dhcpr_server_hash_index,&server,&server, NULL) );
	}
	
	return;
}

/*********************************************************
*	DBUS functions  											      *
**********************************************************/
	
/**********************************************************************************
 * npd_dbus_dhcp_relay_add_iphelper
 *		add DHCP-relay server
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
 *		DHCP_RELAY_RETURN_CODE_SUCCESS
 *		DHCP_RELAY_RETURN_CODE_GENERAL_ERROR
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_add_iphelper
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_RELAY_RETURN_CODE_SUCCESS;
	unsigned char *ifName;
	unsigned int serverIp = 0, serverPort = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &ifName,
								DBUS_TYPE_UINT32, &serverIp,
								DBUS_TYPE_UINT32, &serverPort,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_dhcpr_check_global_status();
	if (ret == TRUE)
	{
		ret = npd_dhcp_relay_add_iphelper(ifName, serverIp, serverPort);
	}
	else { 
		npd_syslog_dbg("check DHCP-Relay global status not enabled.\n");
		ret = DHCP_RELAY_RETURN_CODE_NOT_GLOBAL_ENABLE;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

	
/**********************************************************************************
 * npd_dbus_dhcp_relay_del_iphelper
 *		del DHCP-relay server
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
 *		DHCP_RELAY_RETURN_CODE_SUCCESS
 *		DHCP_RELAY_RETURN_CODE_GENERAL_ERROR
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_del_iphelper
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_RELAY_RETURN_CODE_SUCCESS;
	unsigned char *ifName;
	unsigned int serverIp = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &ifName,
								DBUS_TYPE_UINT32, &serverIp,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_dhcpr_check_global_status();
	if (ret == TRUE)
	{
		ret = npd_dhcp_relay_del_iphelper(ifName, serverIp);
	}
	else { 
		npd_syslog_dbg("check DHCP-Relay global status not enabled global.\n");
		ret = DHCP_RELAY_RETURN_CODE_NOT_GLOBAL_ENABLE;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

DBusMessage *npd_dbus_dhcp_relay_show_iphelper
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter, iter_array, iter_detail_array;
	DBusError err;

	int j = 0;
	unsigned int ret = DHCP_RELAY_RETURN_CODE_SUCCESS;
	unsigned int serverCount=0;
	NPD_DHCPR_SERVER server;
    NPD_DHCPR_SERVER next_server;
    char* ifname = NULL;
    char* pifname = NULL;
    char* pnext_ifname = NULL;
	
	memset(&server, 0, sizeof(server));
    memset(&next_server, 0, sizeof(next_server));
    
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &ifname,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_dbg("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_dbg("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_dbg("return error caused dbus.\n");
		return NULL;
	}

    if ('\0' == ifname[0])
    {
        ret = dbtable_hash_head(npd_dhcpr_server_hash_index, NULL, &server, NULL);
        (void)dbtable_hash_next(npd_dhcpr_server_hash_index, &server, &next_server, NULL);
    	serverCount = dbtable_hash_count(npd_dhcpr_server_hash_index);
    }
    else
	{
        if (NPD_TRUE == npd_intf_netif_get_by_name(&(server.netifIndex), ifname))
        {
            ret = dbtable_hash_search(npd_dhcpr_server_hash_index, &server, NULL, &server);
            (void)dbtable_hash_next(npd_dhcpr_server_hash_index, &server, &next_server, NULL);
        }
        else
        {
		    ret = DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
        }
	}

    if (0 != ret)
    {
        ret = DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
    }
	
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	
	dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_UINT32,
								 &serverCount);
	
	if(DHCP_RELAY_RETURN_CODE_SUCCESS == ret)
	{
		/*get arp count*/
        pifname = (server.ifName);
        dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_STRING,
								 &pifname);
        pnext_ifname = (next_server.ifName);
	    dbus_message_iter_append_basic (&iter,
								 DBUS_TYPE_STRING,
								 &pnext_ifname);

		dbus_message_iter_open_container(&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
								DBUS_TYPE_UINT32_AS_STRING //WTPID
								DBUS_TYPE_BYTE_AS_STRING //wtp_mat_p										
								DBUS_TYPE_ARRAY_AS_STRING
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
								DBUS_TYPE_UINT32_AS_STRING //wlanid
								DBUS_TYPE_UINT16_AS_STRING //tx_bytes
								DBUS_STRUCT_END_CHAR_AS_STRING 
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_array);			
		
			
		DBusMessageIter iter_struct;

		dbus_message_iter_open_container( &iter_array, DBUS_TYPE_STRUCT, NULL, &iter_struct);				
		
		dbus_message_iter_append_basic( &iter_struct, DBUS_TYPE_UINT32, &(server.netifIndex));  /* interface*/								

		dbus_message_iter_append_basic( &iter_struct, DBUS_TYPE_BYTE, &(server.enDis));   /*enDis*/

		npd_syslog_dbg("helper intf %s index 0x%x enable %d", server.ifName,server.netifIndex, server.enDis);

		dbus_message_iter_open_container(&iter_struct,
							DBUS_TYPE_ARRAY,
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING
							DBUS_TYPE_UINT32_AS_STRING //IP address
							DBUS_TYPE_UINT16_AS_STRING //Port
							DBUS_STRUCT_END_CHAR_AS_STRING,
							&iter_detail_array);

		for(j = 0; j < MAX_DHCPR_IP_NUM; j++ ) {

			DBusMessageIter iter_detail_struct;

			dbus_message_iter_open_container( &iter_detail_array, DBUS_TYPE_STRUCT, NULL, &iter_detail_struct);										
			
			dbus_message_iter_append_basic( &iter_detail_struct, DBUS_TYPE_UINT32, &(server.svrAddr[j].sin_addr.s_addr));

			dbus_message_iter_append_basic( &iter_detail_struct, DBUS_TYPE_UINT16, &(server.svrAddr[j].sin_port));

			dbus_message_iter_close_container(&iter_detail_array, &iter_detail_struct);	

			if( server.svrAddr[j].sin_addr.s_addr != 0 )
			{
				npd_syslog_dbg("   Server %d IP 0x%x Port %d", j, server.svrAddr[j].sin_addr.s_addr, server.svrAddr[j].sin_port);
			}
		}
			
		dbus_message_iter_close_container(&iter_struct, &iter_detail_array);
		
		dbus_message_iter_close_container(&iter_array, &iter_struct);

		dbus_message_iter_close_container (&iter, &iter_array);
    }
        
	return reply;
}

/**********************************************************************************
 * npd_dbus_dhcp_relay_enable_global_status
 *		set DHCP-Relay enable/disable global status
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
 *		DHCP_RELAY_RETURN_CODE_SUCCESS
 * 	 	DHCP_RELAY_RETURN_CODE_GENERAL_ERROR
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_enable_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_RELAY_RETURN_CODE_SUCCESS;
	unsigned char isEnable = 0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}
	ret = npd_dhcpr_check_global_status();

	if(ret == isEnable)
	{
		npd_syslog_err("DHCP ALREADY SET %s",isEnable?"enable":"disable");
		ret = DHCP_RELAY_RETURN_CODE_DHCP_RELAY_SET;
	}
	else{
		
		ret = npd_dhcp_relay_enable_global_status(isEnable);		
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * npd_dbus_dhcp_relay_enable_intf_status
 *		enable/disable DHCP-Snooping on special vlan
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
 * 	 	DHCP_RELAY_RETURN_CODE_SUCCESS
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_enable_intf_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_RELAY_RETURN_CODE_SUCCESS;
	unsigned char *ifName;
	unsigned char isEnable = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &ifName,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		npd_syslog_err("return error caused dbus.\n");
		return NULL;
	}

	ret = npd_dhcpr_check_global_status();

	if (ret == TRUE)
	{
		ret = npd_dhcp_relay_enable_intf_status(ifName, isEnable );
	}
	else {
		npd_syslog_dbg("check DHCP-Relay global status not enabled global.\n");
        ret = DHCP_RELAY_RETURN_CODE_NOT_GLOBAL_ENABLE;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

DBusMessage* npd_dbus_dhcp_relay_enable_opt82
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_INIT_0;	
	unsigned char opt82_status = NPD_DHCP_RELAY_OPT82_DISABLE;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	if (npd_dhcpr_check_global_status() == TRUE)
	{
		ret = npd_dhcp_relay_get_opt82_status(&opt82_status);
		if (ret != DHCP_SNP_RETURN_CODE_OK)
		{
			syslog_ax_dhcp_snp_err("check ip dhcp relay information option82 error\n");
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
        else
        {
			syslog_ax_dhcp_snp_dbg("check ip dhcp relay informaiton option82 status %s.\n",
								opt82_status ? "enable" : "disable");
			
			if (opt82_status == isEnable)
			{
				syslog_ax_dhcp_snp_err("ip dhcp relay information option82 status already set %s\n",
									opt82_status ? "enable" : "disable");
				ret = DHCP_SNP_RETURN_CODE_IP_DHCP_RELAY_INFO_ALREADY_SET;
			}
            else
			{
				if (isEnable)
				{
					ret = npd_dhcp_relay_opt82_enable();
				}
                else
                {
					ret = npd_dhcp_relay_opt82_disable();
				}
			}
		}
	}
	else
	{
		syslog_ax_dhcp_snp_dbg("check DHCP relay global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_IP_DHCP_RELAY_INFO_NOT_EN_OPT82;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * npd_dbus_dhcp_relay_show_running_intf_config
 *		DHCP relay show running vlan config
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
 *			DHCP_RELAY_RETURN_CODE_OK				- success
 *			DHCP_RELAY_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_show_running_intf_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_dbg("DHCP relay show running intf config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/**************************************** 
	  * save DHCP Snooping vlan config information
	  ***************************************/
	npd_dhcp_relay_show_running_intf_config(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, &en_dis);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}


/**********************************************************************************
 * npd_dbus_dhcp_relay_show_running_global_config
 *		DHCP Relay show running global config
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
 *			DHCP_RELAY_RETURN_CODE_OK				- success
 *			DHCP_RELAY_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *npd_dbus_dhcp_relay_show_running_global_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	char *showStr = NULL;
    struct npd_dhcp_relay_global_status user_r;
    int length = 0;

	syslog_ax_dhcp_snp_err("enter into npd_dbus_dhcp_snp_show_running_global_config func!\n");
	showStr = (char*)malloc(NPD_DHCP_RELAY_GLOBAL_STATUS_LEN);
	if (NULL == showStr)
	{
		syslog_ax_dhcp_snp_err("DHCP relay show running global config, memory malloc error\n");
		return NULL;
	}
    
	memset(showStr, 0, NPD_DHCP_RELAY_GLOBAL_STATUS_LEN);
	memset(&user_r, 0, sizeof(user_r));
    
    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user_r))
	{
        if (NPD_DHCP_RELAY_ENABLE == user_r.dhcp_relay_endis)
        {
            length = sprintf(showStr, "ip dhcp relay enable\n");
            if (NPD_DHCP_RELAY_OPT82_ENABLE == user_r.dhcp_relay_opt82_enable)
            {
               sprintf(showStr + length, "ip dhcp relay information enable\n");
            }
        }
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
    {
	    showStr = NULL;
	}

	return reply;
}

DBusMessage *npd_dbus_ip_dhcp_relay_check_global
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
    unsigned char   dhcpr_endis = 0;
    unsigned char   dhcpr_opt82_endis = 0;
    struct npd_dhcp_relay_global_status user_r;

    memset(&user_r, 0, sizeof(user_r));
    syslog_ax_dhcp_snp_dbg("get in npd_dbus_ip_dhcp_relay_check_global!\n");
        
    if (DHCP_RELAY_RETURN_CODE_SUCCESS == npd_dhcp_relay_global_status_get(&user_r))
	{
        dhcpr_endis = user_r.dhcp_relay_endis;
        dhcpr_opt82_endis = user_r.dhcp_relay_opt82_enable;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &dhcpr_endis);
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &dhcpr_opt82_endis);
	return reply;
}

/*
int npd_dhcp_server_flag_check
(
	unsigned int netifindex,
	unsigned int *status
)
{
	NPD_DHCPR_SERVER tmpSvr;

	if( 0 == npd_dhcpr_server_get_byIndex(netifindex, &tmpSvr) )
	{
		if( TRUE == tmpSvr.dhcp_server_status )
		{
			npd_syslog_dbg("DHCP-SERVER has set enable");
			*status = TRUE ;
			return DHCP_RELAY_RETURN_CODE_SUCCESS;
		}
		else
		{
			*status =  FALSE ;
			return DHCP_RELAY_RETURN_CODE_SUCCESS;
		}
	}
	return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR ;
}
*/

unsigned int npd_dhcpr_relay_server_send
(
	unsigned char *packetBuffs,
	unsigned long buffLen,
	unsigned int netifIndex,
	unsigned short vid
)
{
	int i = 0;
	int ret = 0 ;
	unsigned int gatewayIp=0,agent_count = 0;
	NPD_DHCPR_SERVER dhcprAgent;
	NPD_DHCP_MESSAGE_T *data = NULL;
	int pktLen = 0;

	npd_syslog_dbg("npd_dhcpr_send_to_server:start! ");
	
	if( vid == NPD_PORT_L3INTF_VLAN_ID )
	{
		npd_syslog_dbg("npd_dhcpr_send_to_server:vid == NPD_PORT_L3INTF_VLAN_ID  vid=%d ",vid);

		dhcprAgent.netifIndex = netifIndex;
	}
	else if( vid != 0 ){
		npd_syslog_dbg("npd_dhcpr_send_to_server:vid == NPD_PORT_L3INTF_VLAN_ID  vid=%d ",vid);
		dhcprAgent.netifIndex = npd_netif_vlan_get_index(vid);
	}
	else {
		
		return 0;
	}

	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 
	pktLen = buffLen - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct udphdr);

	npd_syslog_dbg("dhcprAgent.netifIndex=%d", dhcprAgent.netifIndex);


	if( 0 != npd_dhcpr_server_get_byIndex( dhcprAgent.netifIndex, &dhcprAgent) )
	{
		npd_syslog_dbg("No ip helper address on interface 0x%x", dhcprAgent.netifIndex);
		return 0;
	}
	else if( NPD_FALSE == npd_intf_ip_get_by_netif(&gatewayIp, dhcprAgent.netifIndex))
	{
		npd_syslog_dbg("npd_dhcpr_send_to_server: not find IP address for interface %s", dhcprAgent.ifName );
		return 0;
	}
	
	for(i=0; i< MAX_DHCPR_IP_NUM; i++ )
	{
		if( dhcprAgent.svrAddr[i].sin_addr.s_addr == 0 )
		{
			continue;
		}
		npd_syslog_dbg("npd_dhcpr_send_to_server: agent index 0x%x ipAddr 0x%x", 
																					dhcprAgent.netifIndex, 
																					ntohl(gatewayIp));
		if (ntohl(data->giaddr) == 0)
		{
			data->giaddr = gatewayIp;
		}
        dhcprAgent.svrAddr[i].sin_port = htons(dhcprAgent.svrAddr[i].sin_port);
		ret = sendto(dhcpRelayToServer, (char *)data, pktLen, 0, (struct sockaddr *)&(dhcprAgent.svrAddr[i]), 
							sizeof(struct sockaddr_in)) ;
		if( -1 == ret )
		{
			npd_syslog_err("send buf from socket %d for server 0x%x fail, ret is %d", dhcpRelayToServer, 
																	dhcprAgent.svrAddr[i].sin_addr.s_addr, ret);
		}
		else {
			agent_count++;
		}
	}
	npd_syslog_err("agent_count =%d!", agent_count); 
	return agent_count;
}


unsigned int npd_dhcpr_send_to_agent
(
	unsigned char *data,
	unsigned long pktLen,
	unsigned int ipAddr, /* must be network byte order */
	unsigned int port   /* must be network byte order */
)
{
	int ret = 0;
	struct sockaddr_in serverAddr;
	
	npd_syslog_dbg("npd_dhcpr_send_to_agent: send to server 0x%x ", ntohl(ipAddr));
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ipAddr; /* htonl(ipAddr); */
	serverAddr.sin_port = port; /* htonl(port); */

	ret = sendto(dhcpRelayToAgent, (char *)data, pktLen, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if( -1 == ret )
	{
		npd_syslog_err("send buf from socket %d to server 0x%x port %d fail, ret is %d", dhcpRelayToAgent, ipAddr, port, ret);
		return DHCP_RELAY_RETURN_CODE_GENERAL_ERROR;
	}

	return DHCP_RELAY_RETURN_CODE_SUCCESS;
}

unsigned int npd_dhcp_relay_message_replace
(
	unsigned char *packetBuffs,
	unsigned char *isBroadcast,
	unsigned int *output_index,
	unsigned long *buffLen,
	unsigned short *vlanId
)
{
	struct ethhdr *ethHdr = NULL;
	struct iphdr  *ipHdr = NULL;
	struct iphdr tempip;
	struct udphdr *udpHdr = NULL;
	NPD_DHCP_MESSAGE_T *data = NULL;
	unsigned int l3Index = 0 ;
	unsigned short dataLen = 0;
    unsigned char hops = 0;
	unsigned int  gateway = 0;

	ethHdr = (struct ethhdr *)packetBuffs;
	ipHdr  = (struct iphdr *)(packetBuffs + sizeof(struct ethhdr));
	udpHdr = (struct udphdr *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr));
	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

    hops = data->hops;
	gateway = data->giaddr;
	syslog_ax_dhcp_snp_dbg("npd DHCP send to client: for gateway %#x, recalc checksum\n", gateway);
	
	data->hops   = 0;
	data->giaddr = 0;
	
	memcpy(&tempip, ipHdr, sizeof(struct iphdr));
	memset(ipHdr, 0, sizeof(struct iphdr));	

	ipHdr->protocol = IPPROTO_UDP;	
	ipHdr->tot_len = udpHdr->len; /* cheat on the psuedo-header */	
	ipHdr->daddr = data->yiaddr;
	if(data->flags)
	{
		(ipHdr->daddr) |= ~0UL;
	}
	ipHdr->saddr = gateway; 

	udpHdr->source = htons(67);
	udpHdr->dest   = htons(68);
	if(udpHdr->check)
	{
		udpHdr->check  = 0;
		udpHdr->check  = npd_dhcp_snp_checksum(ipHdr, ntohs(udpHdr->len) + sizeof(struct iphdr));
	}

	memcpy(ipHdr, &tempip, sizeof(struct iphdr));
	ipHdr->daddr = data->yiaddr;
	ipHdr->saddr = gateway;
	ipHdr->check = 0;
	if(data->flags)
	{
		(ipHdr->daddr) |= ~0UL;
	}
	ipHdr->check = npd_dhcp_snp_checksum(ipHdr, sizeof(struct iphdr));
	
	memcpy(ethHdr->h_source, PRODUCT_MAC_ADDRESS, MAC_ADDR_LEN);
	memcpy(ethHdr->h_dest, data->chaddr, MAC_ADDR_LEN );
	if(data->flags)
	{
		memset(ethHdr->h_dest,0xFF,6);
	}

	/*get dhcp relay broadcast vlan*/
	if( NPD_TRUE == npd_intf_netif_get_by_ip(&l3Index, gateway) )		
	{
		if(NPD_TRUE == npd_intf_netif_get_by_ifindex(l3Index, output_index) )
		{
			syslog_ax_dhcp_snp_dbg("npd dhcp snp get dhcp relay interface %#x\n", *output_index);
            switch (npd_netif_type_get(*output_index))
            {
                case NPD_NETIF_VLAN_TYPE:
                {
                    *vlanId = npd_netif_vlan_get_vid(*output_index);
    				*isBroadcast = 1;
    				return DHCP_SNP_RETURN_CODE_OK;
                }
                case NPD_NETIF_ETH_TYPE:
                {
                    *vlanId = NPD_PORT_L3INTF_VLAN_ID;
				    return DHCP_SNP_RETURN_CODE_OK;
                }
                case NPD_NETIF_TRUNK_TYPE:
                {
                    *vlanId = NPD_PORT_L3INTF_VLAN_ID;
    				return DHCP_SNP_RETURN_CODE_OK;
                }
                default :
                {
                    syslog_ax_dhcp_snp_dbg("npd dhcp relay message replace: not found netif.");	
				    return DHCP_SNP_RETURN_CODE_PKT_DROP;
                }
            }
		}
		else
		{
			*isBroadcast = 1;
		}
	}
	else {
		data->hops = ++hops;
        data->giaddr = gateway;
		npd_syslog_dbg("The packet is not relay from here, send to ip 0x%x", ntohl(data->giaddr));
		dataLen = *buffLen-sizeof(struct ethhdr)-sizeof(struct iphdr)-sizeof(struct udphdr);
		npd_dhcpr_send_to_agent(data, (unsigned long)dataLen, data->giaddr ,(unsigned int)udpHdr->dest);
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
	}
    
	return DHCP_SNP_RETURN_CODE_PKT_DROP;			
}

#ifdef __cplusplus
}
#endif
#endif

