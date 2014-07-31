/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_trunk.c
*
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for TRUNK module.
*
* DATE:
*		02/21/2010	
*
*UPDATE:
*04/26/2010              zhengzw@autelan.com          Unifying netif index formate with vlan and port-channel
*05/10/2010              zhengzw@autelan.com          Using DB.
*06/11/2010              zhengzw@autelan.com          L3 interface supported.
*08/12/2010              chengjun@autelan.com           Add switchport.
*10/19/2010              zhengzw@autelan.com          Master port check when link change or port join, port leave.
*  FILE REVISION NUMBER:
*  		$Revision: 1.77 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "name_hash_index.h"
#include "npd_lacp.h"
#include "npd_trunk.h"

void npd_trunk_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
);
void npd_trunk_handle_port_relate_event
(
    unsigned int trunk_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
);


netif_event_notifier_t trunk_port_notifier =
{
    .netif_event_handle_f = &npd_trunk_handle_port_event,
    .netif_relate_handle_f = &npd_trunk_handle_port_relate_event
};

char *trkLBalanc[LOAD_BANLC_MAX] = {
	 "based-mac",
	 "based-port",
	 "based-ip",
	 "based-L4",
	 "mac+ip",
	 "mac+L4",
	 "ip+L4",
	 "mac+ip+L4"
};

char *aggregator_mode[3] = {
	"manual",
	"static",
	"dynamic"
};
unsigned int g_loadBalanc = LOAD_BANLC_SRC_DEST_MAC;
db_table_t *trunk_db = NULL;
sequence_table_index_t * g_trunks = NULL;

extern hash_table_index_t *proto_vlanport_hash;
extern array_table_index_t *proto_vlan_array;

extern hash_table_index_t *vlan_xlate_table_hash;

unsigned int trunk_index(unsigned int index)
{
    return index;
}
int npd_trunk_dhcp_trap_set(
    int vid,
    unsigned int netif_index,
    int flags
)
{
    return nam_asic_dhcp_trap_set_by_devport(vid,
            netif_index, flags);
}

unsigned int trunk_key(void *data)
{
    struct trunk_s *trunk = (struct trunk_s*)data;

    return trunk->trunk_id;
}

int trunk_cmp(void *data1, void *data2)
{
    struct trunk_s *trunk1 = (struct trunk_s*)data1;
    struct trunk_s *trunk2 = (struct trunk_s*)data2;

    return (trunk1->trunk_id == trunk2->trunk_id);
}
long npd_trunk_update(void *new, void* old);
long npd_trunk_insert(void* new);
long npd_trunk_delete(void *data);

int npd_trunk_ntoh(void *data)
{
    struct trunk_s *trunk = (struct trunk_s *) data;

	trunk->trunk_id = ntohl(trunk->trunk_id);
    trunk->g_index = ntohl(trunk->g_index);
    trunk->mtu = ntohl(trunk->mtu);
    trunk->linkstate = ntohl(trunk->linkstate);
    trunk->master_port_index = ntohl(trunk->master_port_index);
    trunk->switch_port_index = ntohl(trunk->switch_port_index);
    trunk->load_balance_mode = ntohl(trunk->load_balance_mode);
    trunk->forward_mode = ntohl(trunk->forward_mode);
    trunk->aggregator_mode = ntohs(trunk->aggregator_mode);
	NPD_PBMP_PORT_NTOH(trunk->ports);

	return 0;
}

int npd_trunk_hton(void *data)
{
	struct trunk_s *trunk = (struct trunk_s *) data;

	trunk->trunk_id = htonl(trunk->trunk_id);
	trunk->g_index = htonl(trunk->g_index);
	trunk->mtu = htonl(trunk->mtu);
	trunk->linkstate = htonl(trunk->linkstate);
	trunk->master_port_index = htonl(trunk->master_port_index);
	trunk->switch_port_index = htonl(trunk->switch_port_index);
	trunk->load_balance_mode = htonl(trunk->load_balance_mode);
	trunk->forward_mode = htonl(trunk->forward_mode);
	trunk->aggregator_mode = htons(trunk->aggregator_mode);
	NPD_PBMP_PORT_HTON(trunk->ports);

	return 0;
}

void npd_init_trunks
(
	void
)
{
    char name[] = "LAG_DB";
    create_dbtable(name, NPD_TRUNK_NUMBER_MAX, sizeof(struct trunk_s), 
        &npd_trunk_update, NULL, &npd_trunk_insert, &npd_trunk_delete, 
        NULL, NULL, NULL, npd_trunk_ntoh, npd_trunk_hton, DB_SYNC_ALL, &trunk_db);
    if(NULL == trunk_db)
	{
		syslog_ax_trunk_err("memory alloc error for trunk init!!\n");
		return;
	}
    dbtable_create_sequence_index("tid", trunk_db, NPD_TRUNK_NUMBER_MAX, &trunk_index,
         &trunk_key, &trunk_cmp, &g_trunks);
	if(NULL == g_trunks)
	{
		syslog_ax_trunk_err("memory alloc error for trunk init!!\n");
		return;
	}
    register_netif_notifier(&trunk_port_notifier);
	return;
}
/* check a trunk node has exists in the list or not*/
/* we use TRUNKiD as a index for searching for the node*/
/**********************************************************************************
 *  npd_check_trunk_exist
 *
 *	DESCRIPTION:
 * 		Check out if specified TRUNK has been created or not
 * 		TRUNK ID used as an index to find trunk structure.
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUNK_BADPARAM  - if parameters given are wrong
 *		NPD_TRUNK_EXISTS	- if trunk has been created before 
 *		NPD_TRUNK_NOTEXISTS	- if trunk doesn't exist,or hasn't been created before
 *
 **********************************************************************************/
unsigned int npd_check_trunk_exist
(
	unsigned short trunkId
)
{
    int ret;
    struct trunk_s *data;
	/*trunkId should be in range of (0,127]*/
	if((trunkId == 0)||(trunkId >= NPD_TRUNK_NUMBER_MAX))
	{
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
	}
    data = malloc(sizeof(struct trunk_s));
    if(NULL == data)
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
    data->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, data);

	if(0 != ret) {
		ret =  TRUNK_RETURN_CODE_TRUNK_NOTEXISTS; /*MACRO =24*/
	}
	else
		ret =  TRUNK_RETURN_CODE_TRUNK_EXISTS; /*MACRO =23*/
    free(data);
    return ret;
}

int npd_trunk_check_port_in_trunk
(
	unsigned int   eth_index,
	unsigned int *trunk_id,
	unsigned int *netif_index
)
{
	int ret = 0;
    unsigned short pvid;

    ret = npd_eth_port_get_ptrunkid(eth_index, &pvid);
    if(0 != ret)
        return -1;
    else
    {
        *trunk_id = pvid;
        *netif_index = npd_netif_trunk_get_index(pvid);
        return 0;
    }

	return -1;
}

unsigned short npd_get_valid_trunk_id()
{
	unsigned short i = 0;
    int ret;
    struct trunk_s *data;
    data = malloc(sizeof(struct trunk_s));
    if(NULL == data)
		return 0;  
    
	for(i = 1; i < NPD_TRUNK_NUMBER_MAX; i++)
	{
        data->trunk_id = i;
        ret = dbtable_sequence_search(g_trunks, i, data);
	    if(0 != ret)
	    {
            free(data);
			return i;
	    }
	}
    free(data);
	return 0;
}

/**********************************************************************************
 *  npd_find_trunk
 *
 *	DESCRIPTION:
 * 		Check out if specified trunk has been created or not
 * 		TRUNK ID used as an index to find trunk structure.
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL  - if parameters given are wrong
 *		trunkNode	- if trunk has been created before 
 *		
 **********************************************************************************/
int npd_find_trunk
(
	unsigned short trunkId,
	struct trunk_s * trunk
)
{
    int ret = -1;

	if( trunk == NULL )
		return ret; 
	
    trunk->trunk_id = trunkId;
	ret = dbtable_sequence_search(g_trunks, trunkId, trunk);
    return ret;
}

/**********************************************************************************
 *  npd_find_trunk_by_name
 *
 *	DESCRIPTION:
 * 		Check out if specified trunk has been created or not
 * 		name is used to compare with each trunk exists.
 *
 *	INPUT:
 *		name - trunk name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		pointer to trunk found.
 *
 **********************************************************************************/
int npd_find_trunk_by_name
(
	char *name,
	struct trunk_s * trunk

)
{
	int iter = 0;
    int ret;

	for(iter = 0;iter < NPD_TRUNK_NUMBER_MAX; iter++) {
        trunk->trunk_id = iter;
		ret = dbtable_sequence_search(g_trunks, iter, trunk);
		if(0 != ret)
			continue;	/*trunk null*/
		else if(strcmp(name,trunk->name)) {			
			continue;	/*trunk name not match*/
		}
		else {
			return 0;
		}
	}
    memset(trunk, 0, sizeof(struct trunk_s));
	return -1;
}
/**********************************************************************************
 *  npd_check_trunk_status
 *
 *	DESCRIPTION:
 * 		Check out if specified trunk is UP or DOWN
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		TRUNK_STATE_UP_E  - if trunk is up
 *		TRUNK_STATE_DOWN_E	- if trunk is down
 *		-NPD_TRUNK_NOTEXISTS	- if trunk doesn't exist,or hasn't been created before
 *
 **********************************************************************************/
int npd_check_trunk_status
(
	unsigned short trunkId
)
{
	unsigned int ret = TRUNK_RETURN_CODE_ERR_NONE;
    struct trunk_s *data;
	/*trunkId should be in range of (0,127]*/
	if((trunkId == 0)||(trunkId >= NPD_TRUNK_NUMBER_MAX))
	{
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
	}
    data = malloc(sizeof(struct trunk_s));
    if(NULL == data)
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
    data->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, data);

	if(0 != ret) {
		ret =  TRUNK_RETURN_CODE_TRUNK_NOTEXISTS; /*MACRO =24*/
	}
	else
	{

		if(TRUNK_STATE_UP_E == data->linkstate) {
			ret = TRUNK_STATE_UP_E;
		}
		else {
			ret = TRUNK_STATE_DOWN_E;
		}
	}
    free(data);
	return ret;
}

/**********************************************************************************
 *  npd_check_trunk_member_exist
 *
 *	DESCRIPTION:
 * 		Check out if specified trunk have member ports.
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		TRUNK_RETURN_CODE_PORT_EXISTS        - if trunk have member ports
 *		TRUNK_RETURN_CODE_PORT_NOTEXISTS	- if trunk have not member ports
 *		TRUNK_RETURN_CODE_BADPARAM             - paramter invalid
 *		TRUNK_RETURN_CODE_TRUNK_NOTEXISTS  - if trunk is not exist
 *
 **********************************************************************************/

int npd_check_trunk_member_exist
(
	unsigned short trunkId
)
{
    unsigned int ret = TRUNK_RETURN_CODE_PORT_EXISTS;
    struct trunk_s *data;
	/*trunkId should be in range of (0,127]*/
	if((trunkId == 0)||(trunkId >= NPD_TRUNK_NUMBER_MAX))
	{
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
	}
    data = malloc(sizeof(struct trunk_s));
    if(NULL == data)
		return TRUNK_RETURN_CODE_BADPARAM;  /*MACRO =12*/
    data->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, data);

	if(0 != ret) {
		ret =  TRUNK_RETURN_CODE_TRUNK_NOTEXISTS; /*MACRO =24*/
	}
	else
	{
		if(NPD_PBMP_IS_NULL(data->ports)) {			
			ret = TRUNK_RETURN_CODE_PORT_NOTEXISTS;
		}
		else {
			ret = TRUNK_RETURN_CODE_PORT_EXISTS;
		}
	}
    free(data);
	return ret;
}
unsigned int npd_delete_trunk
(
	unsigned short trunkId
)
{
	struct trunk_s* node = NULL;
	int retval = TRUNK_RETURN_CODE_ERR_NONE;

    node = malloc(sizeof(struct trunk_s));
    if(NULL == node)
        return -1;
	retval = npd_find_trunk(trunkId, node);

	if (0 != retval) {
	 	retval = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
	}
	else 
    {
		syslog_ax_trunk_dbg("free trunk %d node\n",trunkId);
        dbtable_sequence_delete(g_trunks, trunkId, node, node);
	}
    free(node);
	return retval;
}


unsigned int npd_eth_port_get_ptrunkid
(
	unsigned int   eth_g_index,
	unsigned short *trunkId
)
{
	unsigned int ret = NPD_SUCCESS;
	
	struct eth_port_s portInfo;

    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, (void *)&portInfo);
    if(0 != ret)
        return -1;
	if(portInfo.trunkid == -1)
	{
        *trunkId = -1;
		return -1;
	}
	
    *trunkId = portInfo.trunkid;
    return ret;
}



/**********************************************************************************
 *  npd_trunk_check_port_membership
 *
 *	DESCRIPTION:
 * 		check out if a port is a port member of specified TRUNK
 *
 *	INPUT:
 *		trunkId - trunk id
 *		eth_g_index - global ethernet port index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - if port is a member of trunk
 *		NPD_FALSE - if port is not a member of trunk
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_check_port_membership
(
	unsigned short trunkId,
	unsigned int   eth_index
)
{
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int ret;

	if(NULL == trunkNode) {
		return NPD_FALSE;
	}
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FALSE;
        goto error;
    }

    ret = NPD_PBMP_MEMBER(trunkNode->ports, eth_port_array_index_from_ifindex(eth_index));

error:
    free(trunkNode);

    return ret;
    
}


/**********************************************************************************
 *  npd_trunk_member_port_index_get_all
 *
 *	DESCRIPTION:
 * 		check out if a port is a port member of specified trunk
 *
 *	INPUT:
 *		trunkId - trunk id
 *		eth_g_index - global ethernet port index
 *		mbr_count - trunk port member counts
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - if port is a member of trunk
 *		NPD_FALSE - if port is not a member of trunk
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_member_port_index_get_all
(
	unsigned short	trunkId,
	unsigned int	eth_g_index[],
	unsigned int	*mbr_count
)
{
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int mbrCnt = 0;
    unsigned int port;
    int ret;
    
	if(NULL == trunkNode) {
		return COMMON_RETURN_CODE_NO_RESOURCE;
	}
    
	ret = npd_find_trunk(trunkId, trunkNode);
    
    if(0 != ret)
    {
        ret = NPD_FALSE;
        goto error;
    }

    NPD_PBMP_ITER(trunkNode->ports, port)
    {
        eth_g_index[mbrCnt] = eth_port_array_index_to_ifindex(port);
        mbrCnt++;
    }

	if(mbrCnt >0){
		*mbr_count = mbrCnt;
		ret =  NPD_TRUE;
	}
error:
    free(trunkNode);
	return ret;
}


/**********************************************************************************
 *  npd_trunk_master_port_config
 *
 *	DESCRIPTION:
 * 		config a port tobe a master port member of specified TRUNK
 *
 *	INPUT:
 *		trunkId - trunk id
 *		eth_g_index - global ethernet port index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - if port is a member of trunk
 *		NPD_FALSE - if port is not a member of trunk
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_master_port_config
(
	unsigned short trunkId,
	unsigned int   eth_index
)
{
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int ret;

	if(NULL == trunkNode) {
		return  NPD_FALSE;
	}

    npd_key_database_lock();
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret =  NPD_FALSE;
        goto error;
    }

	ret = NPD_PBMP_MEMBER(trunkNode->ports, 
        eth_port_array_index_from_ifindex(eth_index));

    if(0 == ret)
    {
        ret =  NPD_FALSE;
        goto error;
    }

    trunkNode->master_port_index = eth_index;
    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);

    ret = NPD_TRUE;
error:
	npd_key_database_unlock();
    free(trunkNode);
	return ret;
}


/***************************************************
 * npd_trunk_load_balanc_set
 *
 * params :
 *		trunkId
 *		loadBalancMode
 *
 ***************************************************/
 unsigned int npd_trunk_load_balanc_set(unsigned short trunkId,unsigned int lbMode)
 {
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int ret = TRUNK_RETURN_CODE_ERR_NONE;

	if(NULL == trunkNode) {
		return NPD_FAIL;
	}

    npd_key_database_lock();
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }
    
	trunkNode->load_balance_mode= lbMode;
    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
error:
	npd_key_database_unlock();
    free(trunkNode);
	return ret;
 }
/**********************************************************************************
 *  npd_trunk_master_port_get
 *
 *	DESCRIPTION:
 * 		find out a master port member of specified TRUNK
 *
 *	INPUT:
 *		trunkId - trunk id
 *		eth_g_index - global ethernet port index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUE  - find a master port
 *		NPD_FALSE - no exsit
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_master_port_get
(
	unsigned short trunkId,
	unsigned int   *eth_index
)
{
    struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int ret;

    if(NULL == trunkNode) 
    {
        return NPD_FALSE;
    }
    
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FALSE;
        goto error;
    }
    if(trunkNode->master_port_index == 0)
    {
        ret = NPD_FALSE;
        goto error;
    }
    *eth_index = trunkNode->master_port_index;

    free(trunkNode);
	return ret = NPD_TRUE;;
    
error:
    free(trunkNode);
	return ret;
}

/**********************************************************************************
 *  npd_trunk_status_get
 *
 *	DESCRIPTION:
 * 		get trunk status:state up or state down
 *
 *	INPUT:
 *		trunkId - trunk id		
 *	
 *	OUTPUT:
 *		status - trunk status
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_status_get
(
	unsigned int trunkId,
	unsigned int *status
)
{
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int ret;

	if(NULL == trunkNode) {
		return NPD_FAIL;
	}
    
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }


    *status = trunkNode->linkstate;
error:
    free(trunkNode);
	return ret;
}

/**********************************************************************************
 *  npd_get_port_route_mode
 *
 *	DESCRIPTION:
 * 		get switch port route mode
 *
 *	INPUT:
 *		netif_index - index of trunk mode 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *			INTERFACE_RETURN_CODE_SUCCESS
 *			INTERFACE_RETURN_CODE_ERROR
 *
 **********************************************************************************/
int npd_get_trunk_route_mode(unsigned int netif_index,unsigned char *mode)
{
	unsigned int trunkId = 0;
	int ret = INTERFACE_RETURN_CODE_SUCCESS;
	struct trunk_s* portInfo = malloc(sizeof(struct trunk_s));

    if(!portInfo)
        return COMMON_RETURN_CODE_NO_RESOURCE;
    trunkId = npd_netif_trunk_get_tid(netif_index);
    portInfo->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, portInfo);
	if(0 == ret)
	{
        if(PORT_IP_INTF == portInfo->forward_mode)
        {
            *mode = 1;
        }
		else
		{
			*mode = 0;
		}
	}
	else 
		ret = INTERFACE_RETURN_CODE_ERROR;

    free(portInfo);
	return ret;
	
}

/**********************************************************************************
 *  npd_del_port_switch_mode
 *
 *	DESCRIPTION:
 * 		del switch port running mode
 *
 *	INPUT:
 *		eth_g_index - index of port's mode 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *			INTERFACE_RETURN_CODE_SUCCESS
 *			INTERFACE_RETURN_CODE_ERROR
 *
 **********************************************************************************/
int npd_set_trunk_route_mode(unsigned int trunkId,unsigned int mode)
{
	int ret = INTERFACE_RETURN_CODE_SUCCESS;
	struct trunk_s* portInfo = malloc(sizeof(struct trunk_s));

    if(!portInfo)
        return COMMON_RETURN_CODE_NO_RESOURCE;

    portInfo->trunk_id = trunkId;
	npd_key_database_lock();
    ret = dbtable_sequence_search(g_trunks, trunkId, portInfo);
	if(0 == ret) {
        if(PORT_IP_INTF != portInfo->forward_mode)
        {
			if(npd_intf_table_is_full() == TRUE)
			{
				ret = INTERFACE_RETURN_CODE_ERROR;
				free(portInfo);
				npd_key_database_unlock();
				return ret;
			}
			else
			{
                netif_notify_event(npd_netif_trunk_get_index(trunkId), 
                    PORT_NOTIFIER_L2DELETE);
                netif_app_notify_event(npd_netif_trunk_get_index(trunkId), PORT_NOTIFIER_L2DELETE, NULL, 0);
                npd_delete_switch_port(portInfo->switch_port_index);
			}
        }
        ret = dbtable_sequence_search(g_trunks, trunkId, portInfo);
        portInfo->forward_mode = PORT_IP_INTF;
        portInfo->switch_port_index = -1;
        dbtable_sequence_update(g_trunks, trunkId, NULL, portInfo);
	}
	else 
		ret = INTERFACE_RETURN_CODE_ERROR;
    npd_key_database_unlock();
    free(portInfo);
	return ret;
	
}

/**********************************************************************************
 *  npd_del_port_route_mode
 *
 *	DESCRIPTION:
 * 		del route port running mode
 *
 *	INPUT:
 *		eth_g_index - index of port's mode 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		INTERFACE_RETURN_CODE_SUCCESS 
 *		COMMON_RETURN_CODE_NULL_PTR   -  NULL POINT  error
 *		INTERFACE_RETURN_CODE_FD_ERROR  - fd is invalidate
 *		INTERFACE_RETURN_CODE_IOCTL_ERROR   -  ioctl FAILED
 *		INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *		INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST
 *		INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *		INTERFACE_RETURN_CODE_ERROR
 *
 **********************************************************************************/
int npd_del_trunk_route_mode(unsigned int trunkId)
{
	int ret=INTERFACE_RETURN_CODE_SUCCESS;
	struct trunk_s* portInfo = malloc(sizeof(struct trunk_s));
    unsigned int link_status;

    if(!portInfo)
        return COMMON_RETURN_CODE_NO_RESOURCE;

    portInfo->trunk_id = trunkId;
	npd_key_database_lock();
    ret = dbtable_sequence_search(g_trunks, trunkId, portInfo);

	if(0 == ret) {
        if(PORT_SWITCH_PORT != portInfo->forward_mode)
        {
            portInfo->forward_mode = PORT_SWITCH_PORT;            
            link_status = npd_check_trunk_status(trunkId);
            npd_create_switch_port(npd_netif_trunk_index(trunkId),
                "LAG",
                &portInfo->switch_port_index,
                link_status
                );
			dbtable_sequence_update(g_trunks, trunkId, NULL , portInfo);
            netif_notify_event(npd_netif_trunk_index(trunkId), 
                PORT_NOTIFIER_L2CREATE);
            if(link_status)
                netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_LINKUP_E);
            else
                netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_LINKDOWN_E);
        }
	}
	else 
		ret = INTERFACE_RETURN_CODE_ERROR;
	npd_key_database_unlock();
    free(portInfo);
	
	return ret;
}

/**********************************************************************************
 *  npd_set_port_switch_mode
 *
 *	DESCRIPTION:
 * 		set switch port running mode
 *
 *	INPUT:
 *		eth_g_index - index of port's mode 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *			INTERFACE_RETURN_CODE_SUCCESS
 *			INTERFACE_RETURN_CODE_ERROR
 *
 **********************************************************************************/
int npd_set_trunk_switch_mode(unsigned int trunkId,unsigned int oldMode)
{
	return npd_del_trunk_route_mode(trunkId);	
}

/**********************************************************************************
 *  npd_set_port_route_mode
 *
 *	DESCRIPTION:
 * 		set route port running mode
 *
 *	INPUT:
 *		index - index of port's mode 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		INTERFACE_RETURN_CODE_SUCCESS 
 *		COMMON_RETURN_CODE_NULL_PTR
 *		INTERFACE_RETURN_CODE_ERROR
 *		INTERFACE_RETURN_CODE_NAM_ERROR 
 *		INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *		INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *		INTERFACE_RETURN_CODE_FD_ERROR
 *		INTERFACE_RETURN_CODE_IOCTL_ERROR
 *		INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 **********************************************************************************/
int npd_del_trunk_switch_mode(unsigned int trunkId)
{
    return npd_set_trunk_route_mode(trunkId, 0);
}

int  npd_trunk_master_check(int trunkId)
{
    long ret = 0;
    unsigned int port = 0;
    unsigned int eth_g_index = 0;
    unsigned int  master_eth_index = 0;
    unsigned int  orig_master_eth_index = 0;
    unsigned long check_priority_max = 0;
    unsigned long check_priority_current = 0;
    struct trunk_s *trunkdata = NULL;
    unsigned long temp = 0;
    unsigned int speed = 0;
    unsigned int link = 0;
	unsigned int attr_bitmap = 0;

    trunkdata = malloc(sizeof(struct trunk_s));
    if(NULL == trunkdata)
    {
        ret = TRUNK_RETURN_CODE_BADPARAM;
        goto error;
    }
    
    ret = npd_find_trunk(trunkId, trunkdata);
    if(0 != ret)
    {   
        goto error;
    }
   
    orig_master_eth_index = trunkdata->master_port_index;
    NPD_PBMP_ITER(trunkdata->ports, port)
    {
        check_priority_current = 0;
        eth_g_index = eth_port_array_index_to_ifindex(port);
		attr_bitmap = eth_port_sw_attr_bitmap_get(eth_g_index);
        link = (attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT;
        if(link == ETH_ATTR_LINKUP)
        {
            check_priority_current = 0x1<<30;
        }
        if ((attr_bitmap & ETH_ATTR_DUPLEX ) == ETH_ATTR_DUPLEX_FULL)
        {
            check_priority_current |= (0x1<<29);
        }
        speed = (attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;
        switch (speed)
        {
            case ETH_ATTR_SPEED_10M:
                check_priority_current |= (0x1<<15);
                break;
            case ETH_ATTR_SPEED_100M:
                check_priority_current |= (0x2<<15);
                break;
            case ETH_ATTR_SPEED_1000M:
                check_priority_current |= (0x3<<15);
                break;
            case ETH_ATTR_SPEED_10G:
                check_priority_current |= (0x4<<15);
                break;
            case ETH_ATTR_SPEED_12G:
                check_priority_current |= (0x5<<15);
                break;
            case ETH_ATTR_SPEED_2500M:
                check_priority_current |= (0x6<<15);
                break;
            case ETH_ATTR_SPEED_5G:
                check_priority_current |= (0x7<<15);
                break;
            case ETH_ATTR_SPEED_40G:
                check_priority_current |= (0x8<<15);
                break;
            case ETH_ATTR_SPEED_100G:
                check_priority_current |= (0x9<<15);
                break;
            default:
                break;
        }
        temp = (unsigned long)(4096 - eth_port_array_index_from_ifindex(eth_g_index));
        
        check_priority_current += temp;
        if (check_priority_current > check_priority_max)
        {
            check_priority_max = check_priority_current;
            master_eth_index = eth_g_index;
        }
    }
    /*if the master port has changed, set the new one*/
    if(orig_master_eth_index != master_eth_index)
    {
        if(NPD_TRUE == npd_trunk_master_port_config(trunkId, master_eth_index))
        {
            syslog_ax_trunk_dbg("debug: set master(0x%x) success!\n", master_eth_index);
    		ret = TRUNK_RETURN_CODE_ERR_NONE;
        }
        else
        {
            ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        }
    }
error:
     if(trunkdata)
        free(trunkdata);
     return ret;
}


/**********************************************************************************
 *
 * change trunk status down if all ports down in the trunk
 *
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_SUCCESS
 *		NPD_FAIL
 *		
 **********************************************************************************/
void npd_trunk_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
)
{
    struct eth_port_s *eth_port = malloc(sizeof(struct eth_port_s));
	struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    unsigned int trunkId;
    int ret;
    unsigned int port;
    unsigned int flag = 0;
    int unlock = TRUE;

	syslog_ax_trunk_dbg("trunk handle port event: port_index 0x%x, event %d\n", eth_g_index, event );

    if((NULL == eth_port) || (NULL == trunkNode))
    {
        ret = NPD_FAIL;
        goto error;
    }

	if( NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index) )
	{
		ret = NPD_FAIL;
		goto error;
	}

	memset(eth_port, 0, sizeof(struct eth_port_s));
	memset(trunkNode, 0, sizeof(struct trunk_s));

    unlock = FALSE;
	eth_port->eth_port_ifindex = eth_g_index;

    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, eth_port);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }
    
    trunkId = eth_port->trunkid;
    ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }

    switch(event)
    {
        case PORT_NOTIFIER_CREATE:
            /*todo*/
            break;
        case PORT_NOTIFIER_DELETE:
            unlock = TRUE;
			npd_trunk_port_del(trunkId, eth_g_index);
            break;
        case PORT_NOTIFIER_INSERT:
            /*todo*/
            break;
        case PORT_NOTIFIER_REMOVE:
        case PORT_NOTIFIER_LINKDOWN_E:
            if(TRUNK_STATE_UP_E == trunkNode->linkstate)
            {
                trunkNode->linkstate = TRUNK_STATE_DOWN_E;
                NPD_PBMP_ITER(trunkNode->ports, port)
                {
                    unsigned int g_index = eth_port_array_index_to_ifindex(port);
					if (g_index == eth_g_index) // because db eth port state is up
						continue;
					
                    flag = npd_check_eth_port_status(g_index);
                    if(flag == ETH_ATTR_LINKUP)
                    {
                        if(trunkNode->master_port_index == eth_g_index)
                        {
                            npd_trunk_master_check(trunkId);
                        }
                        trunkNode->linkstate = TRUNK_STATE_UP_E;
                        break;
                    }
                }
                if(TRUNK_STATE_DOWN_E == trunkNode->linkstate)
                {
                    ret = npd_find_trunk(trunkId, trunkNode);
                    trunkNode->linkstate = TRUNK_STATE_DOWN_E;
				    dbtable_sequence_update(g_trunks, trunkId, trunkNode, trunkNode);
                    unlock = TRUE;
                    netif_notify_event(trunkNode->g_index, PORT_NOTIFIER_LINKDOWN_E);
                }
                
            }

            break;
        case PORT_NOTIFIER_LINKUP_E:
            npd_trunk_master_check(trunkId);
            npd_find_trunk(trunkId, trunkNode);
            if(TRUNK_STATE_DOWN_E == trunkNode->linkstate)
            {
                trunkNode->linkstate = TRUNK_STATE_UP_E;
				dbtable_sequence_update(g_trunks, trunkId, trunkNode, trunkNode);
                unlock = TRUE;
                
                netif_notify_event(trunkNode->g_index, PORT_NOTIFIER_LINKUP_E);
            }

            break;
        default:
            break;
    }
    ret = NPD_SUCCESS;

error:
    if(eth_port)
        free(eth_port);
    if(trunkNode)
        free(trunkNode);      
}


void npd_trunk_handle_port_relate_event
(
    unsigned int trunk_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
)
{
    struct eth_port_s *eth_port = malloc(sizeof(struct eth_port_s));
    struct trunk_s *trunkNode = malloc(sizeof(struct trunk_s));
    int trunkId;
    int ret;
    unsigned int port;
    unsigned int flag;

    if((NULL == eth_port) || (NULL == trunkNode))
    {
        ret = NPD_FAIL;
        goto error;
    }

    if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_index))
        || (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
        ret = NPD_FAIL;
        goto error;
    }
	
    eth_port->eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, eth_port);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }
    
    trunkId = npd_netif_trunk_get_tid(trunk_index);
    ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
        
    }

    switch(event)
    {
        case PORT_NOTIFIER_JOIN:
            npd_trunk_master_check(trunkId);
            npd_find_trunk(trunkId, trunkNode);
            if(TRUNK_STATE_DOWN_E == trunkNode->linkstate)
            {
                flag = npd_check_eth_port_status(eth_g_index);
                if(flag == ETH_ATTR_LINKUP)
                {
                    trunkNode->linkstate = TRUNK_STATE_UP_E;
				    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
                    netif_notify_event(trunkNode->g_index, PORT_NOTIFIER_LINKUP_E);
                }
            }
            break;
        case PORT_NOTIFIER_LEAVE:
            npd_trunk_master_check(trunkId);
            npd_find_trunk(trunkId, trunkNode);
            if(TRUNK_STATE_UP_E == trunkNode->linkstate)
            {
                trunkNode->linkstate = TRUNK_STATE_DOWN_E;
                NPD_PBMP_ITER(trunkNode->ports, port)
                {
                    unsigned int flag;
                    unsigned int g_index = eth_port_array_index_to_ifindex(port);
                    if(g_index == eth_g_index)
                        continue;
                    flag = npd_check_eth_port_status(g_index);
                    if(flag == ETH_ATTR_LINKUP)
                    {
                        trunkNode->linkstate = TRUNK_STATE_UP_E;
                        break;
                    }
                }
                if(TRUNK_STATE_DOWN_E == trunkNode->linkstate)
                {
                    trunkNode->linkstate = TRUNK_STATE_DOWN_E;
				    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
                    netif_notify_event(trunkNode->g_index, PORT_NOTIFIER_LINKDOWN_E);
                }
                
            }
            break;
        default:
            break;
    }
error:
    if(eth_port)
        free(eth_port);
    if(trunkNode)
        free(trunkNode);      
}


int npd_trunk_port_qinq_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{
	switch_port_db_t switch_port_trunk;
	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;
	int ret;

   	if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}	
	
	memset(&switch_port_trunk, 0, sizeof(switch_port_db_t));

	switch_port_trunk.global_port_ifindex = trunk_netif_index;
	ret = dbtable_hash_search(switch_ports_hash, &switch_port_trunk,NULL, &switch_port_trunk);
	if(0 != ret)
		return ret;
	
	
	ret = npd_port_access_qinq_enable(eth_g_index, switch_port_trunk.access_qinq);
	if (0 != ret)
		return ret;
	
	return NPD_SUCCESS;	
}

/* update added eth port isolation to other port in private vlan */
int npd_trunk_port_isolate_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{

	switch_port_db_t switch_port_trunk;
	switch_port_db_t switch_port_egress;
	struct vlan_s vlan_loop;
	npd_vbmp_t bmp;
	int vlan;
    
	npd_pbmp_t bmp_port;		
	int port;

	port_driver_t* driver;
	int vlan_ret;
	int ret = NPD_FAIL;

	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;
	
	
    if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}
		
	memset(&switch_port_trunk, 0, sizeof(switch_port_db_t));
	memset(&switch_port_egress, 0, sizeof(switch_port_db_t));

	switch_port_trunk.global_port_ifindex = trunk_netif_index;
	ret = dbtable_hash_search(switch_ports_hash, &switch_port_trunk,NULL, &switch_port_trunk);
	if(0 != ret)
		return ret;

	if(!switch_port_trunk.vlan_private_mode) //if trunk is promisucous port, then not update
		return NPD_SUCCESS;

	driver = port_driver_get(netif_index);
	if ((NULL == driver->port_isolate_add)
		|| (NULL == driver->port_isolate_del))
		return NPD_FAIL;

	NPD_VBMP_ASSIGN(bmp, switch_port_trunk.allow_untag_vlans);
	NPD_VBMP_OR(bmp, switch_port_trunk.allow_tag_vlans);
	NPD_VBMP_ITER(bmp, vlan)
	{  
	    vlan_loop.vid = vlan;
	    vlan_ret = dbtable_sequence_search(g_vlans, vlan, &vlan_loop);
	    if(0 != vlan_ret)
	    {
	        continue;
	    }


		if(0 == vlan_loop.pvlan_type)
		{
			continue;
		}
		
		NPD_PBMP_ASSIGN(bmp_port, vlan_loop.untag_ports);
		NPD_PBMP_OR(bmp_port, vlan_loop.tag_ports);
	    NPD_PBMP_ITER(bmp_port, port)	
	    {
	        ret = dbtable_array_get(switch_ports, port, &switch_port_egress);
		    if (0 != ret)
		    {
				npd_syslog_dbg("ret is not 0: %s(%d): %s", 
					__FILE__, __LINE__, "dbtable_array_get");
		    }

		    if (switch_port_egress.switch_port_index == switch_port_trunk.switch_port_index) 
			{
				continue;
				//return NPD_SUCCESS;
			}	
			
			switch(switch_port_trunk.vlan_private_mode)
			{
				case SWITCH_PORT_PRIVLAN_COMMUNITY:
					{
						if(switch_port_egress.vlan_private_mode == SWITCH_PORT_PRIVLAN_ISOLATED)
						{
							if (added)
							{
								ret = (*driver->port_isolate_add)(switch_port_egress.global_port_ifindex,
					                            eth_g_index);
							}
							else
							{
								ret = (*driver->port_isolate_del)(switch_port_egress.global_port_ifindex,
					                            eth_g_index);								
							}
						}
					}
					break;
				case SWITCH_PORT_PRIVLAN_ISOLATED:
					{
						if(switch_port_egress.vlan_private_mode != SWITCH_PORT_PRIVLAN_PROMI)
						{
							if (added)
							{
								ret = (*driver->port_isolate_add)(switch_port_egress.global_port_ifindex,
					                            eth_g_index);
							}
							else
							{
								ret = (*driver->port_isolate_del)(switch_port_egress.global_port_ifindex,
					                            eth_g_index);								
							}
						}
					}
					break;
			}
		}	
	}

	return ret;

}

int npd_trunk_port_protocol_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{
	proto_vlan_port_t protovlan;
	protobase_vlan_t proto_vlan_base;

	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;
	
	int i;
	int op_ret;
	int ret;

   	if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}	

	for(i = 0; i <= NPD_MAX_PROTO_VLAN_ID; i++)
	{
	    protovlan.netif_index = trunk_netif_index;
	    protovlan.proto_group_index = i;
	
	    op_ret = dbtable_hash_search(proto_vlanport_hash, &protovlan, NULL, &protovlan);
	    if(0 != op_ret)
	    {
	    	continue;
	    }		
	    
	    ret = dbtable_array_get(proto_vlan_array, protovlan.proto_group_index, 
	                   &proto_vlan_base);

		if (added)
		{
		    ret = nam_proto_vlan_port_add(eth_g_index, 
				protovlan.proto_group_index,  
				protovlan.vid, 
	        	proto_vlan_base.ether_frame, 
	        	proto_vlan_base.eth_type);

			ret = npd_port_vlan_filter(eth_g_index, 0);	/* NONE */
			
		}
		else
		{
		    ret = nam_proto_vlan_port_del(eth_g_index, 
				protovlan.proto_group_index,
				protovlan.vid, 
	        	proto_vlan_base.ether_frame, 
	        	proto_vlan_base.eth_type);	
			
			ret = npd_port_vlan_filter(eth_g_index, 3);	/* Ingress And Egress */
			
		}
	                   
	}
	return ret;
	
}

int npd_trunk_port_macvlan_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{
	switch_port_db_t switch_port_trunk;
	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;	
	int ret;

   	if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}	
	
	memset(&switch_port_trunk, 0, sizeof(switch_port_db_t));

	switch_port_trunk.global_port_ifindex = trunk_netif_index;
	ret = dbtable_hash_search(switch_ports_hash, &switch_port_trunk,NULL, &switch_port_trunk);
	if(0 != ret)
		return ret;

	if (FALSE == switch_port_trunk.mac_vlan_flag)
		return NPD_SUCCESS;

	if (added)
	{
		ret = npd_port_mac_vlan_enable(eth_g_index, switch_port_trunk.mac_vlan_flag);
		ret = npd_port_vlan_filter(eth_g_index, 0);	/* NONE */
	}
	else
	{
		ret = npd_port_mac_vlan_enable(eth_g_index, FALSE);		
		ret = npd_port_vlan_filter(eth_g_index, 3);	/* Ingress And Egress */
	}
	
	return ret;
}


int npd_trunk_port_subnetvlan_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{
	switch_port_db_t switch_port_trunk;

	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;
	int ret;

   	if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}	

	memset(&switch_port_trunk, 0, sizeof(switch_port_db_t));

	switch_port_trunk.global_port_ifindex = trunk_netif_index;
	ret = dbtable_hash_search(switch_ports_hash, &switch_port_trunk,NULL, &switch_port_trunk);
	if(0 != ret)
		return ret;

	if (FALSE == switch_port_trunk.subnet_vlan_flag)
		return NPD_SUCCESS;

	if (added)
	{
		ret = npd_port_subnet_vlan_enable(eth_g_index, switch_port_trunk.subnet_vlan_flag);
		ret = npd_port_vlan_filter(eth_g_index, 0);	/* NONE */
	}
	else
	{
		ret = npd_port_subnet_vlan_enable(eth_g_index, FALSE);		
		ret = npd_port_vlan_filter(eth_g_index, 3);	/* Ingress And Egress */
	}

	return ret;
	
}

int npd_trunk_port_prefersubnet_update(
	unsigned int father_netif_index, 
	unsigned int netif_index, 
	unsigned int added)
{
	switch_port_db_t switch_port_trunk;
	int trunk_netif_index = father_netif_index;
	int eth_g_index = netif_index;
	int ret;

   	if((NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(trunk_netif_index))
        && (NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index)))
    {
		return NPD_FAIL;
	}	

	memset(&switch_port_trunk, 0, sizeof(switch_port_db_t));

	switch_port_trunk.global_port_ifindex = trunk_netif_index;
	ret = dbtable_hash_search(switch_ports_hash, &switch_port_trunk,NULL, &switch_port_trunk);
	if(0 != ret)
		return ret;

	if (FALSE == switch_port_trunk.prefer_subnet)
		return NPD_SUCCESS;

	if (added)
	{
		ret = npd_port_prefer_subnet_enable(eth_g_index, switch_port_trunk.prefer_subnet);
	}
	else
	{
		ret = npd_port_prefer_subnet_enable(eth_g_index, FALSE);		
	}

	return ret;
	
}

long npd_trunk_insert(void* new)
{
    struct trunk_s *trunk = (struct trunk_s*)new;
    int ret = NPD_SUCCESS, all_ret = NPD_SUCCESS;
    unsigned int port;
	unsigned char tmp_devNum, tmp_portNum; 
	
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));
    if(switch_port == NULL)
    {
		return -1;
    }
    ret = nam_asic_trunk_entry_active(trunk->trunk_id);
    /*trunk load balance mode*/
    if(ret == 0)
    {
        ret = nam_asic_trunk_load_balanc_set(trunk->trunk_id, trunk->load_balance_mode);
        if(NPD_SUCCESS != ret)
            all_ret = -1;
    }
	
	NPD_PBMP_ITER(trunk->ports, port)
    {
        unsigned int eth_index = eth_port_array_index_to_ifindex(port);
        int enable;
        unsigned int vid;
		int port_link_state;

        enable = TRUE;

		port_link_state = npd_check_eth_port_status(eth_index);
        if(port_link_state == ETH_ATTR_LINKUP)
            enable = TRUE;
        else
            enable = FALSE;

		/* Only the port is Linkup can add TRUNK */
    	ret = nam_asic_trunk_ports_add(eth_index, trunk->trunk_id, enable);
        if(NPD_SUCCESS != ret)
            all_ret = -1;


		ret = npd_get_devport_by_global_index(eth_index, &tmp_devNum, &tmp_portNum);

        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
            continue;

		if(trunk->forward_mode == PORT_SWITCH_PORT)
		{
            if(!switch_port)
            {
                all_ret = -1;
                continue;
            }
            switch_port->global_port_ifindex = npd_netif_trunk_get_index(trunk->trunk_id);
            ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
            if(0 != ret)
            {
                all_ret = -1;
                continue;
            }
            ret = npd_port_set_pvid(eth_index, switch_port->pvid);
            if(NPD_SUCCESS != ret)
                all_ret = -1;
			
            NPD_VBMP_ITER(switch_port->allow_tag_vlans, vid)
            {
				ret = npd_trunk_allow_vlan(switch_port->global_port_ifindex, vid, TRUE);
                ret = nam_asic_vlan_entry_ports_add(eth_index, vid, TRUE);
                if(NPD_SUCCESS != ret)
                    all_ret = -1;
                npd_intf_vlan_add_eth_hw_handler(vid, eth_index);
            }

            NPD_VBMP_ITER(switch_port->allow_untag_vlans, vid)
            {
				ret = npd_trunk_allow_vlan(switch_port->global_port_ifindex, vid, FALSE);
                ret = nam_asic_vlan_entry_ports_add(eth_index, vid, FALSE);
                if(NPD_SUCCESS != ret)
                    all_ret = -1;
                npd_intf_vlan_add_eth_hw_handler(vid, eth_index);
            }
		
			npd_trunk_port_protocol_update(switch_port->global_port_ifindex,eth_index, TRUE);
			npd_trunk_port_macvlan_update(switch_port->global_port_ifindex,eth_index, TRUE);
			npd_trunk_port_subnetvlan_update(switch_port->global_port_ifindex,eth_index, TRUE);
			npd_trunk_port_prefersubnet_update(switch_port->global_port_ifindex,eth_index, TRUE);
			npd_trunk_port_isolate_update(switch_port->global_port_ifindex,eth_index, TRUE);
			npd_trunk_port_qinq_update(switch_port->global_port_ifindex,eth_index, TRUE);
		}
		else if(trunk->forward_mode == PORT_IP_INTF)
		{
			ret = npd_port_set_pvid(eth_index, NPD_PORT_L3INTF_VLAN_ID);					
			if(NPD_SUCCESS != ret)
				all_ret = -1;
			
			ret = nam_asic_vlan_entry_ports_add(eth_index, NPD_PORT_L3INTF_VLAN_ID, FALSE);
            if(NPD_SUCCESS != ret)
                all_ret = -1;
            npd_intf_trunk_add_eth_hw_handler(trunk->trunk_id, eth_index);
		}
    }
	free(switch_port);
    return ret;
}

long npd_trunk_delete(void* new)
{
    struct trunk_s *trunk = (struct trunk_s*)new;
    int ret = NPD_SUCCESS;

    ret = nam_asic_trunk_delete(trunk->trunk_id);
    
    return ret;
}

long npd_trunk_update(void *new, void* old)
{
    struct trunk_s *new_trunk = (struct trunk_s *) new;
    struct trunk_s *old_trunk = (struct trunk_s *) old;
	unsigned char tmp_devNum, tmp_portNum; 	
    int ret = NPD_SUCCESS, all_ret = NPD_SUCCESS;
    switch_port_db_t *switch_port = malloc(sizeof(switch_port_db_t));


    npd_syslog_dbg("LAG db handle update\n");
    /*trunk port member update*/
    if(!NPD_PBMP_EQ(new_trunk->ports, old_trunk->ports))
    {
        unsigned int port;
        npd_pbmp_t bmp;

        NPD_PBMP_ASSIGN(bmp, new_trunk->ports);

        NPD_PBMP_XOR(bmp, old_trunk->ports);

        NPD_PBMP_ITER(bmp, port)
        {
            unsigned int eth_index = eth_port_array_index_to_ifindex(port);
            int enable;
            unsigned int vid;
			int port_link_state;

            if(NPD_PBMP_MEMBER(new_trunk->ports, port))
            {
                enable = TRUE;

				port_link_state = npd_check_eth_port_status(eth_index);
		        if(port_link_state == ETH_ATTR_LINKUP)
		            enable = TRUE;
		        else
		            enable = FALSE;

				/* Only the port is Linkup can add TRUNK */
            	ret = nam_asic_trunk_ports_add(eth_index, new_trunk->trunk_id, enable);
                if(NPD_SUCCESS != ret)
                    all_ret = -1;

			
				ret = npd_get_devport_by_global_index(eth_index, &tmp_devNum, &tmp_portNum);

                if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
                    continue;

				if(new_trunk->forward_mode == PORT_SWITCH_PORT)
				{
	                if(!switch_port)
	                {
	                    all_ret = -1;
	                    continue;
	                }
	                switch_port->global_port_ifindex = npd_netif_trunk_get_index(new_trunk->trunk_id);
	                ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
	                if(0 != ret)
	                {
	                    all_ret = -1;
	                    continue;
	                }
	                ret = npd_port_set_pvid(eth_index, switch_port->pvid);
	                if(NPD_SUCCESS != ret)
	                    all_ret = -1;
					
	                NPD_VBMP_ITER(switch_port->allow_tag_vlans, vid)
	                {
						ret = npd_trunk_allow_vlan(switch_port->global_port_ifindex, vid, TRUE);
	                    ret = nam_asic_vlan_entry_ports_add(eth_index, vid, TRUE);
	                    if(NPD_SUCCESS != ret)
	                        all_ret = -1;
	                    npd_intf_vlan_add_eth_hw_handler(vid, eth_index);
	                }

	                NPD_VBMP_ITER(switch_port->allow_untag_vlans, vid)
	                {
						ret = npd_trunk_allow_vlan(switch_port->global_port_ifindex, vid, FALSE);
	                    ret = nam_asic_vlan_entry_ports_add(eth_index, vid, FALSE);
	                    if(NPD_SUCCESS != ret)
	                        all_ret = -1;
	                    npd_intf_vlan_add_eth_hw_handler(vid, eth_index);
	                }
				
					npd_trunk_port_protocol_update(switch_port->global_port_ifindex,eth_index, TRUE);
					npd_trunk_port_macvlan_update(switch_port->global_port_ifindex,eth_index, TRUE);
					npd_trunk_port_subnetvlan_update(switch_port->global_port_ifindex,eth_index, TRUE);
					npd_trunk_port_prefersubnet_update(switch_port->global_port_ifindex,eth_index, TRUE);
					npd_trunk_port_isolate_update(switch_port->global_port_ifindex,eth_index, TRUE);
					npd_trunk_port_qinq_update(switch_port->global_port_ifindex,eth_index, TRUE);
				}
				else if(new_trunk->forward_mode == PORT_IP_INTF)
				{
					ret = npd_port_set_pvid(eth_index, NPD_PORT_L3INTF_VLAN_ID);					
					if(NPD_SUCCESS != ret)
						all_ret = -1;
					
					ret = nam_asic_vlan_entry_ports_add(eth_index, NPD_PORT_L3INTF_VLAN_ID, FALSE);
	                if(NPD_SUCCESS != ret)
	                    all_ret = -1;
                    npd_intf_trunk_add_eth_hw_handler(new_trunk->trunk_id, eth_index);
				}
            }
            else
            {
            	ret = nam_asic_trunk_ports_del(eth_index, new_trunk->trunk_id);
                if(NPD_SUCCESS != ret)
                    all_ret = -1;
	
				ret = npd_get_devport_by_global_index(eth_index, &tmp_devNum, &tmp_portNum);

                if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
                    continue;
				
				if(new_trunk->forward_mode == PORT_SWITCH_PORT)
				{
	                if(!switch_port)
	                {
	                    all_ret = -1;
	                    continue;
	                }
	                switch_port->global_port_ifindex = npd_netif_trunk_get_index(new_trunk->trunk_id);
	                ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);
	                if(0 != ret)
	                {
	                    all_ret = -1;
	                    continue;
	                }

	                NPD_VBMP_ITER(switch_port->allow_tag_vlans, vid)
	                {
	                    npd_intf_vlan_del_eth_hw_handler(vid, eth_index);
	                    ret = nam_asic_vlan_entry_ports_del(vid, eth_index,  TRUE);
	                    if(NPD_SUCCESS != ret)
	                        all_ret = -1;
	                }

	                NPD_VBMP_ITER(switch_port->allow_untag_vlans, vid)
	                {
	                    npd_intf_vlan_del_eth_hw_handler(vid, eth_index);
	                    ret = nam_asic_vlan_entry_ports_del(vid, eth_index, FALSE);
	                    if(NPD_SUCCESS != ret)
	                        all_ret = -1;
	                }
					npd_trunk_port_protocol_update(switch_port->global_port_ifindex,eth_index, FALSE);
					npd_trunk_port_macvlan_update(switch_port->global_port_ifindex,eth_index, FALSE);
					npd_trunk_port_subnetvlan_update(switch_port->global_port_ifindex,eth_index, FALSE);
					npd_trunk_port_prefersubnet_update(switch_port->global_port_ifindex,eth_index, FALSE);
					npd_trunk_port_isolate_update(switch_port->global_port_ifindex,eth_index, FALSE);
					npd_trunk_port_qinq_update(switch_port->global_port_ifindex,eth_index, FALSE);

				}
				else if(new_trunk->forward_mode == PORT_IP_INTF)
				{
					ret = nam_asic_vlan_entry_ports_del(NPD_PORT_L3INTF_VLAN_ID, eth_index, TRUE);
	               	if(NPD_SUCCESS != ret)
	                   	all_ret = -1;
                   	npd_intf_trunk_del_eth_hw_handler(new_trunk->trunk_id, eth_index);				
				}
            }
        }        
    }

    /*trunk master port update*/
    if(new_trunk->master_port_index != old_trunk->master_port_index)
    {
        ret = nam_asic_trunk_master_port_set(new_trunk->trunk_id, new_trunk->master_port_index); 
        if(NPD_SUCCESS != ret)
            all_ret = -1;

    }
    
    /*trunk load balance mode*/
    if(new_trunk->load_balance_mode != old_trunk->load_balance_mode)
    {
        ret = nam_asic_trunk_load_balanc_set(new_trunk->trunk_id, new_trunk->load_balance_mode);
        if(NPD_SUCCESS != ret)
            all_ret = -1;
    }

    if(new_trunk->linkstate != old_trunk->linkstate)
    {
    	ret = nam_asic_trunk_linkstate_set(new_trunk->trunk_id, new_trunk->linkstate);
		
        if(new_trunk->linkstate == PORT_LINK_DOWN)
            ret = nam_fdb_table_delete_entry_with_trunk(new_trunk->trunk_id);
    }

	if(new_trunk->forward_mode != old_trunk->forward_mode)
	{
		ret = nam_asic_trunk_forward_mode_set(new_trunk->trunk_id, new_trunk->forward_mode);		
	}

    if(switch_port)
        free(switch_port);
    return all_ret;
           
}

/**********************************************************************************
 *
 * create trunk by TRUNK ID on both SW and HW side
 *
 *
 *	INPUT:
 *		trunk ID - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUNK_ERR_GENERAL - if error occurred when create trunk in SW side 
 *		NPD_TRUNK_ERR_HW - if error occurred when create trunk in HW side
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_activate
(
	unsigned short trunkId,
	char* name
)
{
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
    unsigned int 	op_ret = 0 ;
	struct trunk_s	*trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int    trkLinkStat = TRUNK_STATE_DOWN_E;
	if(NULL == name){
		ret = TRUNK_RETURN_CODE_BADPARAM;
		goto error;
	}

    if(NULL == trunkNode)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
		goto error;
    }
    npd_key_database_lock();
    
	ret = npd_check_trunk_exist(trunkId);
	if(ret == TRUNK_RETURN_CODE_TRUNK_EXISTS) 
	{
        op_ret = npd_find_trunk(trunkId, trunkNode);
        if(op_ret != 0) 
        {
            ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
            goto error;
        }

        if (strcmp(name,trunkNode->name)==0)
        {
            ret = TRUNK_RETURN_CODE_ERR_NONE;
        }
        goto error;
	}
	else if (ret == TRUNK_RETURN_CODE_BADPARAM)
	{
		goto error;
	}
	else 
    {
		ret = npd_find_trunk_by_name(name, trunkNode);
		if(0 ==  ret) {
			 syslog_ax_trunk_dbg("trunk name conflict.\n");
			ret = TRUNK_RETURN_CODE_NAME_CONFLICT;
            goto error;
		}
        memset(trunkNode, 0, sizeof(*trunkNode));
		trunkNode->trunk_id= trunkId;
		trunkNode->g_index = npd_netif_trunk_index(trunkId);
		trunkNode->load_balance_mode= g_loadBalanc;
		trunkNode->linkstate = trkLinkStat;
        trunkNode->forward_mode = PORT_SWITCH_PORT;
		trunkNode->aggregator_mode = MANUAL_MODE;
		trunkNode->mtu = 1522;
        strncpy(trunkNode->name, name, 30);
        ret = dbtable_sequence_insert(g_trunks, trunkId, trunkNode);
        if(-1 == ret)
        {
			syslog_ax_trunk_dbg("trunk create error.\n");
			ret = TRUNK_RETURN_CODE_ERR_GENERAL;  
            goto error;
        }
        npd_create_switch_port(npd_netif_trunk_index(trunkId), "lag", &(trunkNode->switch_port_index), FALSE);

        ret = dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
        if(0 != ret)
        {
			syslog_ax_trunk_dbg("trunk create error.\n");
			ret = TRUNK_RETURN_CODE_ERR_GENERAL;  
            goto error;
        }
        
        netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_L2CREATE);
	}
error:
	npd_key_database_unlock();
    if(trunkNode)
    {
		free(trunkNode);
    }
	return ret;
}
unsigned int npd_trunk_port_add
(
	unsigned short trunkId, 
	unsigned int eth_index
)
{
    int ret;
    struct trunk_s *trunkdata = NULL;
    struct eth_port_s *eth_port = NULL;
	
	syslog_ax_trunk_dbg("trunkId %d,global_index 0x%x\n",trunkId,eth_index);
    

    trunkdata = malloc(sizeof(struct trunk_s));
    if(NULL == trunkdata)
    {
        ret = TRUNK_RETURN_CODE_BADPARAM;
        goto error;
    }

    eth_port = malloc(sizeof(struct eth_port_s));
    if(NULL == eth_port)
    {
        ret = TRUNK_RETURN_CODE_BADPARAM;
        goto error;
    }
	npd_key_database_lock();

    trunkdata->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, trunkdata);
    if(0 != ret)
    {
		syslog_ax_trunk_dbg("trunk %d not exist.\n",trunkId);
		ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;/*can NOT directed Return NPD_TRUNK_BADPARAM.*/
        goto error;
        
    }
    eth_port->eth_port_ifindex = eth_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_index, eth_port);
    if(0 != ret)
    {
		syslog_ax_trunk_dbg("eth port 0x%x not exist.\n",eth_index);
		ret = TRUNK_RETURN_CODE_PORT_NOTEXISTS;/*can NOT directed Return NPD_TRUNK_BADPARAM.*/
        goto error;
    }
    
    ret = NPD_PBMP_MEMBER(trunkdata->ports, eth_port_array_index_from_ifindex(eth_index));
	if (TRUE == ret) {
		syslog_ax_trunk_dbg("port %x has already been the member of trunk % d\n",eth_index,trunkId);
		ret = TRUNK_RETURN_CODE_PORT_EXISTS;
        goto error;
	}

    if((eth_port->trunkid != -1)
        && (eth_port->trunkid != trunkId))
    {
		syslog_ax_trunk_dbg("port %x has already been the member of trunk % d\n",
            eth_index,eth_port->trunkid);
		ret = TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT;
        goto error;
    }

    if(eth_port->forward_mode == PORT_IP_INTF)
    {
		syslog_ax_trunk_dbg("port %x has been configured as ip port\n",
            eth_index);
		ret = TRUNK_RETURN_CODE_PORT_L3_INTFG;
        goto error;
        
    }

    {
        int count = 0;
        unsigned int array_port;
        int board_type[16] = {0};
        int trunk_slot[16] = {0};
		/*add port to trunk_port_list and set port trunkId*/
        NPD_PBMP_ITER(trunkdata->ports, array_port)
        {
            unsigned int netif_index = netif_array_index_to_ifindex(array_port);
            int slot = npd_netif_eth_get_slot(netif_index);
            int slot_type = MODULE_TYPE_ON_SLOT_INDEX(slot);
            
            count++;
            if(count > MAX_PORT_PER_TRUNK-1)
            {
                ret = TRUNK_RETURN_CODE_PORT_MBRS_FULL;
                goto error;
            }
            
            board_type[slot] = slot_type;
            trunk_slot[slot] = 1;
		}
        {
            int slot_i;
			int slot = npd_netif_eth_get_slot(eth_index);
            board_type[slot] = MODULE_TYPE_ON_SLOT_INDEX(slot);

            for(slot_i = 0; slot_i < 16; slot_i++)
            {
                if(board_type[slot_i] != 0)
                {
                    if(SYS_MODULE_SDK_DIFFERENT(board_type[slot], board_type[slot_i]))
                    {
                        ret = TRUNK_RETURN_CODE_UNSUPPORT;
                        goto error;
                    }
                }
            }
            if(!SYS_MODULE_SUPPORT_CROSSBOARD_TRUNK(board_type[slot]))
            {
                for(slot_i = 0; slot_i < 16; slot_i++)
                {
                    if(slot_i == slot)
                        continue;
                    if(trunk_slot[slot_i] != 0)
                    {
                        ret = TRUNK_RETURN_CODE_UNSUPPORT;
                        goto error;
                    }
                }
            }
        }
        netif_notify_event(eth_port->eth_port_ifindex, PORT_NOTIFIER_L2DELETE);
        netif_app_notify_event(eth_port->eth_port_ifindex, PORT_NOTIFIER_L2DELETE, NULL, 0);
        npd_delete_switch_port(eth_port->switch_port_index);
        ret = dbtable_sequence_search(g_trunks, trunkId, trunkdata);
        if(0 != ret)
        {
            ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
            goto error;
        }
        ret = dbtable_sequence_search(g_eth_ports, eth_index, eth_port);
        eth_port->trunkid = trunkId;
        eth_port->switch_port_index = -1;
        dbtable_sequence_update(g_eth_ports, eth_index, NULL, eth_port);
        NPD_PBMP_PORT_ADD(trunkdata->ports, eth_port_array_index_from_ifindex(eth_index));
        dbtable_sequence_update(g_trunks, trunkId, NULL, trunkdata);
        
        netif_notify_relate_event(npd_netif_trunk_get_index(trunkId), 
            eth_index, PORT_NOTIFIER_JOIN);
        netif_app_notify_relate_event(npd_netif_trunk_get_index(trunkId),
            eth_index, PORT_NOTIFIER_JOIN, NULL, 0);
    }

error:
	npd_key_database_unlock();
    if(trunkdata)
        free(trunkdata);
    if(eth_port)
        free(eth_port);
	return ret;
}

unsigned int npd_trunk_port_del
(
	unsigned short	trunkId,
	unsigned int	eth_index
)
{
    int ret;
    struct trunk_s *trunkdata = NULL;
    struct eth_port_s *eth_port = NULL;
	
	syslog_ax_trunk_dbg("trunkId %d,global_index %x\n",trunkId,eth_index);
    
    trunkdata = malloc(sizeof(struct trunk_s));
    if(NULL == trunkdata)
    {
        ret = TRUNK_RETURN_CODE_BADPARAM;
        goto error;
    }

    eth_port = malloc(sizeof(struct eth_port_s));
    if(NULL == eth_port)
    {
        ret = TRUNK_RETURN_CODE_BADPARAM;
        goto error;
    }
    eth_port->eth_port_ifindex = eth_index;
	npd_key_database_lock();
    ret = dbtable_sequence_search(g_eth_ports, eth_index, eth_port);
    if(0 != ret)
    {
		syslog_ax_trunk_dbg("Ethernet port %d not exist.\n",trunkId);
		ret = TRUNK_RETURN_CODE_PORT_NOTEXISTS;/*can NOT directed Return NPD_TRUNK_BADPARAM.*/
        goto error;
    }
    
    trunkdata->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, trunkdata);
    if(0 != ret)
    {
		syslog_ax_trunk_dbg("trunk %d not exist.\n",trunkId);
		ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;/*can NOT directed Return NPD_TRUNK_BADPARAM.*/
        goto error;
        
    }
    
    ret = NPD_PBMP_MEMBER(trunkdata->ports, eth_port_array_index_from_ifindex(eth_index));
	if (FALSE == ret) {
		syslog_ax_trunk_dbg("port %x is not the member of trunk % d\n",eth_index,trunkId);
		ret = TRUNK_RETURN_CODE_PORT_NOTEXISTS;
        goto error;
	}
    ret = TRUNK_RETURN_CODE_ERR_NONE;
    {
		/*remove port to trunk_port_list and set port trunkId*/
        NPD_PBMP_PORT_REMOVE(trunkdata->ports, eth_port_array_index_from_ifindex(eth_index));
        dbtable_sequence_update(g_trunks, trunkId, NULL, trunkdata);
        netif_notify_relate_event(npd_netif_trunk_index(trunkId), 
                eth_index, PORT_NOTIFIER_LEAVE);
        netif_app_notify_relate_event(npd_netif_trunk_get_index(trunkId),
                eth_index, PORT_NOTIFIER_LEAVE, NULL, 0);
        dbtable_sequence_search(g_eth_ports, eth_index, &eth_port);
        npd_create_switch_port(eth_index, "ethernet", &eth_port->switch_port_index, npd_check_eth_port_status(eth_index));
        eth_port->trunkid = -1;
        dbtable_sequence_update(g_eth_ports, eth_index, NULL, eth_port);
        netif_notify_event(eth_index, PORT_NOTIFIER_L2CREATE);
        /*
        if(npd_check_eth_port_status(eth_index))
            netif_notify_event(eth_index, PORT_NOTIFIER_LINKUP_E);
        else
            netif_notify_event(eth_index, PORT_NOTIFIER_LINKDOWN_E);
        */
    }

error:
    npd_key_database_unlock();
    if(trunkdata)
        free(trunkdata);
    if(eth_port)
        free(eth_port);
	return ret;
}


/*the following is the driver for l2 and l3*/
int npd_trunk_allow_vlan
(
    unsigned int netif_index,
    unsigned int vid,
    int isTaged
)
{
    int ret;
	unsigned short trunkId = 0;
	if(npd_netif_type_get(netif_index) != NPD_NETIF_TRUNK_TYPE)
	{
		return TRUNK_RETURN_CODE_BADPARAM;
	}
	
	trunkId = npd_netif_trunk_get_tid(netif_index);
    npd_syslog_dbg("LAG %d allow vlan %d with tag mode %d\n",
                   trunkId, vid, isTaged);
    ret = nam_vlan_add_trunk(trunkId, vid, isTaged);
    return ret;
}

int npd_trunk_free_vlan
(
    unsigned int netif_index,
    unsigned int vid,
    int isTaged
)
{
    int ret;
	unsigned short trunkId = 0;
	if(npd_netif_type_get(netif_index) != NPD_NETIF_TRUNK_TYPE)
	{
		return TRUNK_RETURN_CODE_BADPARAM;
	}
	
	trunkId = npd_netif_trunk_get_tid(netif_index);
    npd_syslog_dbg("LAG %d free vlan %d with tag mode %d\n",
                   trunkId, vid, isTaged);
    ret = nam_vlan_del_trunk(trunkId, vid, isTaged);
    return ret;
}

unsigned int npd_trunk_master_port_set
(
	unsigned short trunkId, 
	unsigned int eth_index
)
{
	unsigned int  eth_index_orgl = 0, ret = 0;

	/*if NOT Steps into the (trunk-config CMD node),trunk exists or not Test is wanted.*/
	/*if Steps into (trunk-config CMD node), it does NOT need test trunk node exist or not,because*/
	ret = npd_check_trunk_exist(trunkId);
	if (TRUNK_RETURN_CODE_TRUNK_EXISTS != ret) {
		syslog_ax_trunk_dbg("npd_trunk_add_del:trunk %d NOT exist.\n",trunkId);
		return TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
	}
	/*here MUST check the ETH-port is already member of the TRUNK or NOT*/
	ret = npd_trunk_check_port_membership(trunkId,eth_index);
	if (NPD_TRUE != ret) {
		syslog_ax_trunk_dbg("port %d is not the member of trunk % d\n",eth_index,trunkId);
		return TRUNK_RETURN_CODE_PORT_NOTEXISTS;
	}
	/*find the original Master port*/
	ret = npd_trunk_master_port_get(trunkId, &eth_index_orgl);

    if(eth_index != eth_index_orgl)
    {
    	if(NPD_TRUE == npd_trunk_master_port_config(trunkId, eth_index))
        {
    		return TRUNK_RETURN_CODE_ERR_NONE;
        }
        else
        {
            return TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        }
    }
	
	return TRUNK_RETURN_CODE_ERR_NONE;
}


/* delete trunk node*/
/* by RE-set the struct trunk_s to be ZERO*/
/**********************************************************************************
 *
 *  delete trunk by TRUNK ID on both SW and HW side
 *
 *
 *	INPUT:
 *		trunkId - trunk id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NPD_TRUNK_ERR_GENERAL - if error occurred when create trunk in SW side 
 *		NPD_TRUNK_ERR_HW - if error occurred when create trunk in HW side
 *		
 *
 **********************************************************************************/
unsigned int npd_trunk_destroy_node
(
	unsigned short trunkId
)
{
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
    unsigned int array_port_id;
    struct trunk_s *trunk;

    trunk = malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }

	npd_key_database_lock();
    netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_DELETE);
    netif_app_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_DELETE, NULL, 0);

    trunk->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, trunk);
    if(0 != ret)
    {
		syslog_ax_trunk_dbg("Delete a not exists LAG %d.", trunkId);
        ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        goto error;
    }

    NPD_PBMP_ITER(trunk->ports, array_port_id)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);

        ret = npd_trunk_port_del(trunkId,eth_g_index);

        if(ret == TRUNK_RETURN_CODE_TRUNK_NOTEXISTS)
        {
			syslog_ax_trunk_dbg("Delete ethernet port from a non-exist LAG %d.", trunkId);
            goto error;
        }
        else if(ret == TRUNK_RETURN_CODE_PORT_NOTEXISTS)
        {
			syslog_ax_trunk_dbg("Delete a non-exist ethernet port from LAG %d.", trunkId);
            goto error;
        }
        else if(ret == TRUNK_RETURN_CODE_BADPARAM)
        {
			syslog_ax_trunk_dbg("Delete  ethernet port from LAG %d unknown error.", trunkId);
            goto error;
        }
        
    }
    netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_L2DELETE); 
    netif_app_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_L2DELETE, NULL, 0);
    syslog_ax_trunk_dbg("Delete switchport %d for LAG %d.", trunk->switch_port_index, trunkId);
    npd_delete_switch_port(trunk->switch_port_index);
    trunk->switch_port_index = -1;

    ret = dbtable_sequence_delete(g_trunks, trunkId, trunk, trunk);
    
error:
	npd_key_database_unlock();
    if(trunk)
        free(trunk);
	return ret;
}

unsigned int npd_trunk_destroy_node_by_name
(
	char *trunk_name
)
{
	unsigned short trunkId= 0;
    unsigned int array_port_id;
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
    struct trunk_s *trunk;
    if(trunk_name == NULL)
    {
		return TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
    }
    trunk = malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	npd_key_database_lock();
    ret = npd_find_trunk_by_name(trunk_name, trunk);
	if(ret != 0)
	{
		ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
		goto error;
	}

    trunkId = trunk->trunk_id;
    NPD_PBMP_ITER(trunk->ports, array_port_id)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);

        ret = npd_trunk_port_del(trunkId,eth_g_index);

        if(ret == TRUNK_RETURN_CODE_TRUNK_NOTEXISTS)
        {
             goto error;
        }
        else if(ret == TRUNK_RETURN_CODE_PORT_NOTEXISTS)
        {
             goto error;
        }
        else if(ret == TRUNK_RETURN_CODE_BADPARAM)
        {
             goto error;
        }
        
    }
    netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_L2DELETE);
    netif_app_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_L2DELETE, NULL, 0);
        
    npd_delete_switch_port(trunk->switch_port_index);
    trunk->switch_port_index = -1;

    netif_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_DELETE);
    netif_app_notify_event(npd_netif_trunk_index(trunkId), PORT_NOTIFIER_DELETE, NULL, 0);

    ret = dbtable_sequence_delete(g_trunks, trunkId, trunk, trunk);
    
error:
	npd_key_database_unlock();
    if(trunk)
        free(trunk);
	return ret;
}




/******************************************************
 *  show trunk port membership by read Software record.
 *
 *****************************************************/
int npd_trunk_get_mbr_bmp_via_sw
(
	unsigned short trunkId,
	unsigned int *mbrBmp_sp,
	unsigned int *disMbrBmp_sp
)
{

	return NPD_SUCCESS;
}


/******************************************************
 *  show trunk port membership by read Software record.
 *
 *****************************************************/

void npd_save_trunk_cfg(char* buf,int bufLen)
{
	char *showStr = buf;
	int totalLen = 0;
	int i = 0;
    int ret;
	struct trunk_s* node = NULL;
    unsigned int port;
	unsigned int eth_g_index = 0;
    char tempbuf[128];
	
    node = malloc(sizeof(struct trunk_s));
    if(NULL == node)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }

	for(i = 1; i <= NPD_MAX_TRUNK_ID; i++) {
        node->trunk_id = i;
        ret = dbtable_sequence_search(g_trunks, i, node);
        if(0 != ret)
            continue;
        sprintf(tempbuf, "interface port-channel %s %d\n", node->name,i);
        totalLen += strlen(tempbuf);
        
        if(totalLen < bufLen)
        {
            strcat(showStr, tempbuf);
        }
        else
        {
            goto error;
        }
        if(node->load_balance_mode != g_loadBalanc)
        {
            sprintf(tempbuf, " load-balance %s\n", trkLBalanc[node->load_balance_mode]);
            totalLen += strlen(tempbuf);
            if(totalLen < bufLen)
            {
                strcat(showStr, tempbuf);
            }
            else
            {
                goto error;
            }
        }
        if(node->aggregator_mode != MANUAL_MODE)
        {
            sprintf(tempbuf, " port-channel mode %s\n", aggregator_mode[node->aggregator_mode]);
            totalLen += strlen(tempbuf);
            if(totalLen < bufLen)
            {
                strcat(showStr, tempbuf);
            }
            else
            {
                goto error;
            }
        }		
        NPD_PBMP_ITER(node->ports, port)
        {
            char name[50];
			eth_g_index = eth_port_array_index_to_ifindex(port);
            parse_eth_index_to_name(eth_g_index, (unsigned char *)name);            
            sprintf(tempbuf, " addport %s\n", name);
            totalLen += strlen(tempbuf);
            if(totalLen < bufLen)
            {
                strcat(showStr, tempbuf);
            }
            else
            {
                goto error;
            }
        }

        if(node->forward_mode == PORT_IP_INTF)
        {
            sprintf(tempbuf," no switchport\n");
            totalLen += strlen(tempbuf);
    
            if (totalLen < bufLen)
                strcat(showStr, tempbuf);
            else
                goto error;
        }
    	else
    	{
    		npd_switch_port_show_running(node->switch_port_index, 
                    showStr, bufLen-totalLen);
    	}
		
        sprintf(tempbuf, " exit\n");
        totalLen += strlen(tempbuf);
        if(totalLen < bufLen)
        {
            strcat(showStr, tempbuf);
        }
	}
error:
    if(node)
        free(node);
    return;
		
}

/*****************************************************************************************
 *		NPD dbus operation
 *
 *****************************************************************************************/
 DBusMessage * npd_dbus_check_trunk_exist
 (
 	DBusConnection *conn,
 	DBusMessage *msg,
 	void *user_data
 )
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError error;
	
	struct trunk_s *trunk = NULL;
	unsigned short trunk_id = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_UINT16, &trunk_id,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return NULL;
	}
	trunk = (struct trunk_s *)malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
         return NULL;
    }
	memset(trunk, 0, sizeof(struct trunk_s));
	ret = npd_find_trunk(trunk_id, trunk);
	free(trunk);
	if (ret == 0)
		op_ret = 1;
	else
		op_ret = 0;
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
							DBUS_TYPE_UINT32, &op_ret);
	return reply;
	
	
}


/*create trunk entity with trunkId,on Both Hw &Sw.*/
DBusMessage * npd_dbus_trunk_create_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;
	char*	name = NULL;
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_trunk_dbg("To create a LAG %d named %s.\n", trunkId, name);
	
	ret = npd_trunk_activate(trunkId,name);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	return reply;
	
}

/*enter trunk configure node*/
DBusMessage * npd_dbus_trunk_config_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;
	char trunkName[31] = {0};
	struct trunk_s*	trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int	ret = 0,op_ret = TRUNK_RETURN_CODE_ERR_NONE;
    int route = 0;
    unsigned int l3_ifindex;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&trunkId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(trunkNode)
		{
			free(trunkNode);
		}
		return NULL;
	}
	/* call npd_check_trunk_exist, return tha trunk exist or NOT:*/
	
	if(NULL == trunkNode) {
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
	}
    else
    {
    	syslog_ax_trunk_dbg("get lag index %d\n",trunkId);
    	ret = npd_find_trunk(trunkId, trunkNode);
    	if(0 != ret) {
			sprintf(trunkName,"lag%d", trunkId);
    		op_ret = npd_trunk_activate(trunkId,trunkName);
    		if (TRUNK_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			syslog_ax_trunk_dbg("npd_dbus101:: add trunk tid %d\n",trunkId);
    		}
    	}
    	else {
            
    		 syslog_ax_trunk_dbg("trunk %s 's Id %d\n",trunkName,trunkId);
    	}
		free(trunkNode);
    }

    route = npd_intf_exist_check(npd_netif_trunk_index(trunkId), &l3_ifindex);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &route);
    
	return reply;
}

DBusMessage * npd_dbus_trunk_config_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;
	char*	trunkName = NULL;
	struct trunk_s*	trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int	op_ret = TRUNK_RETURN_CODE_ERR_NONE;
    int route = 0;
    unsigned int l3_ifindex;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING,&trunkName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(trunkNode)
		{
			free(trunkNode);
		}
		return NULL;
	}
    if(NULL == trunkNode)
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
    else
    {
        int ret;
    	syslog_ax_trunk_dbg("get lag name %s\n",trunkName);
    	ret = npd_find_trunk_by_name(trunkName, trunkNode);
    	if(0 != ret) {
    		trunkId = npd_get_valid_trunk_id();
    		if(trunkId == 0)
    		{
        		op_ret = VLAN_RETURN_CODE_ERR_NONE;  
    		}
    		else
    		{
        		op_ret = npd_trunk_activate(trunkId,trunkName);
        		if (VLAN_RETURN_CODE_ERR_NONE == op_ret)
        		{
        			syslog_ax_vlan_dbg("npd_dbus101:: add trunk tid %d\n",trunkId);
        		}
    		}
    	}
    	else {
    		trunkId = trunkNode->trunk_id;
    		 syslog_ax_trunk_dbg("trunk %s 's Id %d\n",trunkName,trunkId);
    	}
		free(trunkNode);
    }
    route = npd_intf_exist_check(npd_netif_trunk_index(trunkId), &l3_ifindex);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT16,
									 &trunkId);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &route);
	
	return reply;
	
}

/*enter trunk configure node*/
DBusMessage * npd_dbus_trunk_config_update_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;
	char*	trunkName = NULL;
	struct trunk_s*	trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int	op_ret = TRUNK_RETURN_CODE_ERR_NONE;
    unsigned int netif_index;
    DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
        DBUS_TYPE_UINT32, &netif_index,
		DBUS_TYPE_STRING,&trunkName,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(trunkNode)
		{
			free(trunkNode);
		}
		return NULL;
	}
    if(NULL == trunkNode)
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
    else
    {
        int ret;
    	syslog_ax_trunk_dbg("get lag name %s\n",trunkName);
    	ret = npd_find_trunk_by_name(trunkName, trunkNode);
    	if(0 == ret) {
            if(trunkNode->g_index == netif_index)
    		{
        		op_ret = TRUNK_RETURN_CODE_ERR_NONE;  
    		}
    		else
    		{
       			op_ret = TRUNK_RETURN_CODE_NAME_CONFLICT;   
    		}
    	}
    	else {
            trunkId = npd_netif_trunk_get_tid(netif_index);
            npd_key_database_lock();
            ret = npd_find_trunk(trunkId, trunkNode);
            if(0 != ret)
                op_ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;

            strcpy(trunkNode->name, trunkName);
            dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
            npd_key_database_unlock();
    	}
		free(trunkNode);
    }
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
	
}


DBusMessage * npd_dbus_trunk_add_delete_port_member
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;

	unsigned char	isAdd = 0;
	unsigned short	trunkId = 0;	
	unsigned int	eth_g_index = 0;
	unsigned int 	ret = TRUNK_RETURN_CODE_ERR_NONE;
	struct trunk_s *trunk;
    trunk = malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	DBusError err;	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_BYTE,&isAdd,
							DBUS_TYPE_UINT32,&eth_g_index,
							DBUS_TYPE_UINT16,&trunkId,
							DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(trunk)
		{
			free(trunk);
		}
		return NULL;
	}
    trunk->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, trunk);
    if(0 != ret)
    {
        ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        goto error;
    }
    if(trunk->aggregator_mode == DYNAMIC_MODE)
    {
        ret = TRUNK_RETURN_CODE_REFUSE_ERR;
        goto error;
    }
    
	if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
		if(isAdd){
			ret = npd_trunk_port_add(trunkId,eth_g_index);
		}
		else{
			ret = npd_trunk_port_del(trunkId,eth_g_index);
		}
	}
error:
    if(trunk)
        free(trunk);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}
DBusMessage * npd_dbus_trunk_delete_one
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
	DBusError err;
    struct trunk_s *trunk;
    trunk = malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	
	 syslog_ax_trunk_dbg("Entering delete trunk one!\n");
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&trunkId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(trunk);
		return NULL;
	}
	syslog_ax_trunk_dbg("delete trunk entry %d.\n",trunkId);
    
    trunk->trunk_id = trunkId;
    ret = dbtable_sequence_search(g_trunks, trunkId, trunk);
    if(0 != ret)
    {
        ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        goto error;
    }
    if(trunk->aggregator_mode == DYNAMIC_MODE)
    {
        ret = TRUNK_RETURN_CODE_REFUSE_ERR;
        goto error;
    }
	ret = npd_trunk_destroy_node(trunkId);
error:
    if(trunk)
        free(trunk);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	
	return reply;
	
}


DBusMessage * npd_dbus_trunk_master_port_set
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned short	trunkId = 0;	

	unsigned int	eth_g_index = 0;
	unsigned int 	ret = TRUNK_RETURN_CODE_ERR_NONE;
	DBusError err;
	/*enum module_id_e module_type;*/
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT32,&eth_g_index,
							DBUS_TYPE_UINT16,&trunkId,
							DBUS_TYPE_INVALID))) {
		syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = npd_trunk_master_port_set(trunkId,eth_g_index);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

/*******************************
 *
 *
 ******************************/

DBusMessage * npd_dbus_trunk_load_banlc_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int	loadBanlc = LOAD_BANLC_MAX;
	unsigned short	trunkId = 0;	
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;

	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT16,&trunkId,
							DBUS_TYPE_UINT32,&loadBanlc,
							DBUS_TYPE_INVALID))) {
		syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	syslog_ax_trunk_dbg("get trunk %d parameter,loadBalanc %d.\n",trunkId,loadBanlc);
	/*check trunk exist*/
	ret = npd_check_trunk_exist(trunkId);
	if(TRUNK_RETURN_CODE_TRUNK_EXISTS == ret)
	{
		npd_trunk_load_balanc_set(trunkId,loadBanlc);
		/*we need to get the ret code of nam*/
		ret = nam_asic_trunk_load_balanc_set(trunkId, loadBanlc);
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	return reply;
}

DBusMessage * npd_dbus_trunk_delete_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char*			trunkName = NULL;
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
	DBusError err;
    struct trunk_s *trunk;
    trunk = malloc(sizeof(struct trunk_s));
    if(NULL == trunk)
    {
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING,&trunkName,
		DBUS_TYPE_INVALID))) {
		syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(trunk);
		return NULL;
	}

	ret = npd_find_trunk_by_name(trunkName, trunk);
	if(ret == 0)
	{
        if(trunk->aggregator_mode == DYNAMIC_MODE)
        {
            ret = TRUNK_RETURN_CODE_REFUSE_ERR;
            goto error;
        }
	    ret = npd_trunk_destroy_node_by_name(trunkName);
	}
error:
    if(trunk)
        free(trunk);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &ret);
	
	return reply;
}

/*show a special trunk member slot_port*/
DBusMessage * npd_dbus_trunk_show_one_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	
	DBusMessage* reply = NULL;
	DBusMessageIter	iter;
	DBusError err;
	unsigned short	trunkId =0;
	char*			trunkName = NULL;
	struct trunk_s	*trunkNode = NULL;
	unsigned int 	m_eth_index = 0, op_ret = 0 ;
	unsigned char 	mstFlag = 0;
	unsigned int 	loadBalanc = 0x0f;
	unsigned int    forwardMode = 0;
	unsigned int 	trkLinkStat = 0;
    unsigned int    enport[MAX_PORT_PER_TRUNK] = {0};
    unsigned int    disport[MAX_PORT_PER_TRUNK] = {0};
    int i;

	trunkName = (char*)malloc(NPD_TRUNK_IFNAME_SIZE+1);
	if(NULL == trunkName) {
		return reply;
	}
	memset(trunkName,0,NPD_TRUNK_IFNAME_SIZE+1);

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT16,&trunkId,
		DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		free(trunkName);	
		trunkName = NULL;
		return NULL;
	}
	 syslog_ax_trunk_dbg("show trunk %d. \n",trunkId);
	if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == npd_check_trunk_exist(trunkId)) {
		op_ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
		 syslog_ax_trunk_dbg("trunkId =%d NOT exists.\n",trunkId);
	}
	else {
        unsigned int array_port_id;
        int en_count = 0, dis_count = 0;

        trunkNode = malloc(sizeof(*trunkNode));
        if(NULL == trunkNode)
        {
            op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
            goto error;
        }
		op_ret = npd_find_trunk(trunkId, trunkNode);
		if(op_ret != 0) {
			 syslog_ax_trunk_dbg("find trunk on Sw Error!\n");
             goto error;
		}

		syslog_ax_trunk_dbg("find trunkNode %p\n",trunkNode);
		memcpy(trunkName,trunkNode->name, ALIAS_NAME_SIZE);
		loadBalanc = trunkNode->load_balance_mode;
		trkLinkStat = trunkNode->linkstate;
		forwardMode = trunkNode->forward_mode;
        /*find out trunk ports*/
		 syslog_ax_trunk_dbg("trunkName %s,load-balance %d.\n",trunkName,loadBalanc);

		/*get trunk master port*/
		op_ret = npd_trunk_master_port_get(trunkId,&m_eth_index);
		if(NPD_TRUE == op_ret){
			mstFlag = 1;
			op_ret = TRUNK_RETURN_CODE_ERR_NONE;
		}

        memset(enport, 0xff, sizeof(enport));
        memset(disport, 0xff, sizeof(disport));
        NPD_PBMP_ITER(trunkNode->ports, array_port_id)
        {
            unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);
            int link_status;

            link_status = npd_check_eth_port_status(eth_g_index);
            if(ETH_ATTR_LINKUP == link_status)
                enport[en_count++] = eth_g_index;
            else
                disport[dis_count++] = eth_g_index;
        }
		
	}
error:
   if(trunkNode)
   {
        free(trunkNode);
        trunkNode=NULL;
   }
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32,&op_ret);
   if(op_ret == TRUNK_RETURN_CODE_ERR_NONE)
   {    
       dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT16,
                                                   &trunkId);
       dbus_message_iter_append_basic(&iter,
       							DBUS_TYPE_STRING,&trunkName);
       dbus_message_iter_append_basic(&iter,
       							DBUS_TYPE_BYTE,&mstFlag);
       dbus_message_iter_append_basic(&iter,
       							DBUS_TYPE_UINT32,&m_eth_index);
       for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
       {
       	dbus_message_iter_append_basic(&iter,
       						        DBUS_TYPE_UINT32,&enport[i]);
       }
       for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
       {
           dbus_message_iter_append_basic(&iter,
       						        DBUS_TYPE_UINT32,&disport[i]);
       }
       dbus_message_iter_append_basic(&iter,
       							DBUS_TYPE_UINT32,&loadBalanc);
       dbus_message_iter_append_basic(&iter,
       					    		DBUS_TYPE_UINT32,&trkLinkStat);
	   dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&forwardMode);
   }
   free(trunkName);
      trunkName = NULL;
        
   return reply;
}

/*show a special trunk member slot_port*/
DBusMessage * npd_dbus_trunk_show_by_name_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	
	DBusMessage* reply;
	DBusMessageIter	iter;
	DBusError err;
	unsigned short	trunkId = 0;
	char*			trunkName = NULL;
	struct trunk_s* trunkNode = malloc(sizeof(struct trunk_s));
	unsigned int	op_ret = TRUNK_RETURN_CODE_ERR_NONE;
	unsigned int 	m_eth_index = 0;
	unsigned char 	mstFlag = 0;
	unsigned int 	loadBalanc = 0x0f;
	unsigned int   forwardMode = 0;
	unsigned int 	trkLinkStat = 0;
    unsigned int    enport[MAX_PORT_PER_TRUNK];
    unsigned int    disport[MAX_PORT_PER_TRUNK];
    int i;
    
	syslog_ax_trunk_dbg("Entering show trunk port members!\n");

    if(NULL == trunkNode)
    {
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_STRING,&trunkName,
								DBUS_TYPE_INVALID))) {
		 syslog_ax_trunk_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(trunkNode)
		{
			free(trunkNode);
		}
		return NULL;
	}
	syslog_ax_trunk_dbg("show trunk %s. \n",trunkName);
	op_ret = npd_find_trunk_by_name(trunkName, trunkNode);
	if(0 != op_ret){
		op_ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
        goto error;
	}
	else 
    {
        int array_port_id;
        int dis_count = 0;
        int en_count = 0;
		trunkId = trunkNode->trunk_id;
		memcpy(trunkName,trunkNode->name, ALIAS_NAME_SIZE);
		loadBalanc = trunkNode->load_balance_mode;
		trkLinkStat = trunkNode->linkstate;
		forwardMode = trunkNode->forward_mode;
		/*find out trunk ports*/
		 syslog_ax_trunk_dbg("trunkName %s,load-balance %d.\n",trunkName,loadBalanc);

		/*get trunk master port*/
		op_ret = npd_trunk_master_port_get(trunkId,&m_eth_index);
		if(NPD_TRUE == op_ret){
			mstFlag = 1;
		}
		op_ret = TRUNK_RETURN_CODE_ERR_NONE;
        memset(enport, 0xff, sizeof(enport));
        memset(disport, 0xff, sizeof(disport));
        NPD_PBMP_ITER(trunkNode->ports, array_port_id)
        {
            unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);
            int link_status;

            link_status = npd_check_eth_port_status(eth_g_index);
            if(ETH_ATTR_LINKUP == link_status)
                enport[en_count++] = eth_g_index;
            else
                disport[dis_count++] = eth_g_index;
        }
	}
error:
    if(trunkNode)
        free(trunkNode);
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&op_ret);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT16,
                                     &trunkId);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_STRING,&trunkName);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_BYTE,&mstFlag);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&m_eth_index);
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
    	dbus_message_iter_append_basic(&iter,
    								DBUS_TYPE_UINT32,&enport[i]);
    }
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
	    dbus_message_iter_append_basic(&iter,
							    DBUS_TYPE_UINT32,&disport[i]);
    }
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&loadBalanc);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&trkLinkStat);	
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&forwardMode);
	return reply;
}

#if 1
/*original*/
DBusMessage * npd_dbus_trunk_show_trunklist_port_member_v1
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
    DBusMessage* reply;
    DBusMessageIter		iter;
    DBusError err;
    
    unsigned short	trunkId =0;
    char*			trunkName = NULL;
    struct trunk_s	*trunkNode;
    unsigned int 	m_eth_index = 0 ;
    unsigned char 	mstFlag = 0;
    unsigned int 	loadBalanc = 0x0f;
	unsigned int   forwardMode = 0;
    unsigned int 	trkLinkStat = 0;
    unsigned int        enport[MAX_PORT_PER_TRUNK];
    unsigned int        disport[MAX_PORT_PER_TRUNK];
    int i, op_ret = -1;
    
    trunkName = (char*)malloc(NPD_TRUNK_IFNAME_SIZE+1);
    if(NULL == trunkName) {
    	return reply;
    }
    memset(trunkName,0,NPD_TRUNK_IFNAME_SIZE+1);
    
    dbus_error_init(&err);
    
    if (!(dbus_message_get_args ( msg, &err,
    	DBUS_TYPE_UINT16,&trunkId,
    	DBUS_TYPE_INVALID))) {
    	 syslog_ax_trunk_err("Unable to get input args ");
    	if (dbus_error_is_set(&err)) {
    		 syslog_ax_trunk_err("%s raised: %s",err.name,err.message);
    		dbus_error_free(&err);
    	}
    	free(trunkName);	
    	trunkName = NULL;
    	return NULL;
    }
    trunkNode = malloc(sizeof(*trunkNode));
    if(NULL == trunkNode)
    {
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
    syslog_ax_trunk_dbg("show trunk %d. \n",trunkId);
    trunkId++;
    for (; trunkId <= CHASSIS_TRUNK_RANGE_MAX; trunkId++)
    {   
        unsigned int array_port_id;
        int en_count = 0, dis_count = 0;

        op_ret = npd_find_trunk(trunkId, trunkNode);
        if(-1 == op_ret)
        {   
            continue;
        }

    	syslog_ax_trunk_dbg("find trunkNode %p\n",trunkNode);
    	memcpy(trunkName,trunkNode->name, ALIAS_NAME_SIZE);
    	loadBalanc = trunkNode->load_balance_mode;
    	trkLinkStat = trunkNode->linkstate;
        forwardMode = trunkNode->forward_mode;
            /*find out trunk ports*/
    	 syslog_ax_trunk_dbg("trunkName %s,load-balance %d.\n",trunkName,loadBalanc);
    
    	/*get trunk master port*/
    	op_ret = npd_trunk_master_port_get(trunkId,&m_eth_index);
    	if(NPD_TRUE == op_ret)
        {
    	    mstFlag = 1;
    	}
    	op_ret = TRUNK_RETURN_CODE_ERR_NONE;

        memset(enport, 0xff, sizeof(enport));
        memset(disport, 0xff, sizeof(disport));
        NPD_PBMP_ITER(trunkNode->ports, array_port_id)
        {
            unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);
            int link_status;

            link_status = npd_check_eth_port_status(eth_g_index);
            if(ETH_ATTR_LINKUP == link_status)
                enport[en_count++] = eth_g_index;
            else
                disport[dis_count++] = eth_g_index;
        }
        
        break;
    } 
    if(CHASSIS_TRUNK_RANGE_MAX < trunkId)
        op_ret = TRUNK_RETURN_CODE_TRUNK_NOTEXISTS;
error:
    if(trunkNode)
    {
        free(trunkNode);
    }
    reply = dbus_message_new_method_return(msg);
    	
    dbus_message_iter_init_append (reply, &iter);
    	
    dbus_message_iter_append_basic (&iter,
    								DBUS_TYPE_UINT32,&op_ret);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT16,
                                 &trunkId);
    dbus_message_iter_append_basic(&iter,
    							DBUS_TYPE_STRING,&trunkName);
    dbus_message_iter_append_basic(&iter,
    							DBUS_TYPE_BYTE,&mstFlag);
    dbus_message_iter_append_basic(&iter,
    							DBUS_TYPE_UINT32,&m_eth_index);
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
    	dbus_message_iter_append_basic(&iter,
    								DBUS_TYPE_UINT32,&enport[i]);
    }
    for(i = 0; i < MAX_PORT_PER_TRUNK; i++)
    {
	    dbus_message_iter_append_basic(&iter,
							    DBUS_TYPE_UINT32,&disport[i]);
    }
    dbus_message_iter_append_basic(&iter,
    							DBUS_TYPE_UINT32,&loadBalanc);
    dbus_message_iter_append_basic(&iter,
    							DBUS_TYPE_UINT32,&trkLinkStat);	
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,&forwardMode);
    free(trunkName);
    trunkName = NULL;
    return reply;
      
}
#endif

DBusMessage * npd_dbus_trunk_show_running_config
(	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *showStr = NULL;

	showStr = (char*)malloc(NPD_TRUNK_RUNNING_CFG_MEM);
	if(NULL == showStr) {
		 syslog_ax_trunk_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr,0,NPD_TRUNK_RUNNING_CFG_MEM);
	/*save trunk cfg*/
	npd_save_trunk_cfg(showStr,NPD_TRUNK_RUNNING_CFG_MEM);
	syslog_ax_trunk_dbg("trunk config:%s\n",showStr);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr);	
	free(showStr);
	showStr = NULL;
	return reply;
}

/*the following is the driver*/
int npd_trunk_driver_allow_vlan(
    unsigned int netif_index,
    int vid,
    int isTagged
    )
{
    int ret, all_ret = TRUNK_RETURN_CODE_ERR_NONE;
    unsigned int array_port;
    struct trunk_s node = {0};

    npd_syslog_dbg_internal("LAG 0x%x allow vlan %d with tag mode %d\n",
        netif_index, vid, isTagged);

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

	ret = npd_trunk_allow_vlan(netif_index, vid, isTagged);
    if(ret == 0)
    {
        NPD_PBMP_ITER(node.ports, array_port)
        {
            unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
            ret = npd_port_allow_vlan(eth_g_index, vid, isTagged);
            if(0 != ret)
            {
                npd_syslog_err("Eth port 0x%x allow vlan %d with tag mode %d failed. (int trunk driver)\n",
                        eth_g_index, vid, isTagged);
                all_ret = ret;
            }
        }
    }
    npd_intf_vlan_add_trunk_hw_handler(vid, node.trunk_id);
    return all_ret;

}

int npd_trunk_driver_free_vlan(
    unsigned int netif_index,
    int vid,
    int isTagged
    )
{
    int ret, all_ret = 0;
    unsigned int array_port;
    struct trunk_s node = {0};

    npd_syslog_dbg_internal("LAG 0x%x free vlan %d with tag mode %d\n",
        netif_index, vid, isTagged);

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
    npd_intf_vlan_del_trunk_hw_handler(vid, node.trunk_id);
    nam_fdb_table_delete_entry_with_vlan_trunk(vid, node.trunk_id);
    npd_fdb_dynamic_entry_del_by_vlan_port(vid, netif_index);
    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_port_free_vlan(eth_g_index, vid, isTagged);
        if(0 != ret)
            all_ret = ret;
    }
	ret = npd_trunk_free_vlan(netif_index, vid, isTagged);
	
    return all_ret;
}

int npd_trunk_driver_set_pvid(
    unsigned int netif_index,
    int pvid
    )
{
    int ret, all_ret = 0;
    unsigned int array_port;
    struct trunk_s node = {0};


    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
	
    npd_syslog_dbg_internal("LAG %d set pvid %d\n", node.trunk_id, pvid);
	
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_port_set_pvid(eth_g_index, pvid);
        if(0 != ret)
            all_ret = ret;
    }
   npd_intf_vlan_add_trunk_hw_handler(pvid, node.trunk_id);
   return all_ret;

}

int npd_trunk_driver_set_inner_pvid(
    unsigned int netif_index,
    int pvid
    )
{
    int ret, all_ret;
    unsigned int array_port;
    struct trunk_s node = {0};

    npd_syslog_dbg_internal("LAG 0x%x set pvid %d\n",
        netif_index, pvid);

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_port_set_inner_pvid(eth_g_index, pvid);
        if(0 != ret)
            all_ret = ret;
    }
   //npd_intf_vlan_add_trunk_hw_handler(pvid, node.trunk_id);
   return all_ret;

}

int npd_trunk_driver_set_vlan_pri(
    unsigned int netif_index,
    int vid,
    int pri, 
    int cfi
    )
{
    int ret, all_ret;
    unsigned int array_port;
    struct trunk_s node = {0};

    npd_syslog_dbg("LAG 0x%x set vlan %d pri %d cfi %d\n",
        netif_index, vid, pri, cfi);

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_port_set_vlan_pri(eth_g_index, vid, pri, cfi);
        if(0 != ret)
            all_ret = ret;
    }
    return all_ret;
}

int npd_trunk_vlan_mode_set(unsigned int netif_index, int mode)
{
	int ret, all_ret;
	unsigned int array_port;
	struct trunk_s node = {0};

	npd_syslog_dbg("LAG 0x%x set vlan mode %d\n", netif_index, mode);

	node.trunk_id = npd_netif_trunk_get_tid(netif_index);
	ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

	ret = nam_asic_trunk_vlan_mode_set(node.trunk_id, mode);
	if( 0 != ret)
		all_ret = ret;

	NPD_PBMP_ITER(node.ports, array_port)
	{
		unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
		ret = npd_port_vlan_mode_set(eth_g_index, mode);
		if(0 != ret)
			all_ret = ret;
	}
	
	return all_ret;
}


int npd_trunk_driver_set_fdb_limit(
    unsigned int netif_index, 
    int fdb_limit
    )
{
	unsigned int trunkno = npd_netif_trunk_get_tid(netif_index);
	unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;
	nam_learn_limit_t limit;
    unsigned char devno, portno;
    unsigned int array_port;
    struct trunk_s node = {0};
    
	memset(&limit,0,sizeof(nam_learn_limit_t));

    npd_syslog_dbg("LAG 0x%x set fdb limit %d \n",
        netif_index, fdb_limit);


    node.trunk_id = trunkno;
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
    if(ret != 0)
    {
		return ret;
    }
    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_get_devport_by_global_index(eth_g_index, &devno, &portno);
        if(0 != ret)
        {
		    continue;
        }
        limit.trunk = trunkno;
        limit.flags = NAM_L2_LEARN_LIMIT_TRUNK;
        limit.port = portno;
		limit.limit = fdb_limit;
        ret = nam_fdb_limit_set(devno, limit);
    }
    return ret;
}

int npd_trunk_driver_set_fdb_limit_byvlantrunk(
    unsigned int netif_index, 
    int vid, 
    int fdb_limit
    )
{
    return 0;
}

int npd_trunk_driver_fdb_add(unsigned char mac[], int vid, unsigned int g_ifindex)
{
    int tid = npd_netif_trunk_get_tid(g_ifindex);
    return nam_static_fdb_entry_mac_vlan_trunk_set(mac, vid, tid);
}

int npd_trunk_driver_fdb_delete_by_port(unsigned int g_ifindex)
{
    int tid = npd_netif_trunk_get_tid(g_ifindex);
    nam_fdb_table_delete_entry_with_trunk(tid);
    return 0;
}

int npd_trunk_driver_fdb_delete_by_vlan_port(unsigned int g_ifindex, int vid)
{
    int tid = npd_netif_trunk_get_tid(g_ifindex);
    nam_fdb_table_delete_entry_with_vlan_trunk(vid, tid);
    return 0;
}

/*status : 0                      not learning and discard unknown SA
                   1                      auto learning and forward 
                   2                      protect mode. CPU learning and discard packet when mac limit full
*/
int npd_trunk_driver_fdb_learning_mode(
    unsigned int netif_index,
    int mode
    )
{
    int ret, all_ret;
    unsigned int array_port;
    struct trunk_s node = {0};

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_port_fdb_learning_mode(eth_g_index, mode);
        if(0 != ret)
            all_ret = ret;
    }
    return all_ret;
}

int npd_trunk_driver_speed(
    unsigned int netif_index,
    int *speed
    )
{
    int ret;
    unsigned int array_port;
    struct trunk_s node = {0};
    int ret_speed = 0;

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    *speed = 0;
    NPD_PBMP_ITER(node.ports, array_port)
    {
        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
        ret = npd_get_port_swspeed(eth_g_index, &ret_speed);
        *speed += ret_speed;

    }
    return 0;
    
}

int npd_trunk_driver_duplex(
    unsigned int netif_index,
    int *duplex
    )
{
    int ret= 0;
    struct trunk_s node = {0};

    node.trunk_id = npd_netif_trunk_get_tid(netif_index);
    ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);

    *duplex = PORT_FULL_DUPLEX_E;
    return ret;
    
}

int npd_trunk_driver_isolate_add(
    unsigned int src_netif_index,
    unsigned int dst_netif_index	/* only the trunk type port */
    )
{
	return npd_port_isolate_add(src_netif_index, dst_netif_index);
}

int npd_trunk_driver_isolate_del(
    unsigned int src_netif_index,
    unsigned int dst_netif_index
    )
{
	return npd_port_isolate_del(src_netif_index, dst_netif_index);
}

int npd_trunk_driver_dhcp_trap_set
(
	int vid,
	unsigned int netif_index,
	int flags
)
{
	int ret = 0, all_ret = 0;
    unsigned int array_port;
	unsigned short trunkId = 0;
    struct trunk_s node = {0};

    npd_syslog_dbg_internal("npd_trunk_driver_dhcp_trap_set: vlan %d netifindex 0x%x set flag %d\n",
        vid, netif_index, flags);

	
    trunkId = npd_netif_trunk_get_tid(netif_index);
	if( 0 == npd_find_trunk( trunkId, &node) )
	{
		NPD_PBMP_ITER(node.ports, array_port)
	    {
	        unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port);
	        ret = npd_port_dhcp_trap_set(vid, eth_g_index, flags);
	        if(0 != ret)
	            all_ret = ret;
	    }
	}
	else {
		all_ret = -1;
	}

    return all_ret;
}

int npd_trunk_subnet_vlan_enable(
    unsigned int netif_index,
    int flags)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_subnet_vlan_enable(eth_g_index, flags);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_mac_vlan_enable(
    unsigned int netif_index,
    int flags)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_mac_vlan_enable(eth_g_index, flags);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_prefer_subnet_enable(
    unsigned int netif_index,
    int flags)
{
	
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_prefer_subnet_enable(eth_g_index, flags);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_vlan_filter(
    unsigned int netif_index,
    int flags)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_vlan_filter(eth_g_index, flags);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_access_qinq_enable(
    unsigned int netif_index,
    int flags)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_access_qinq_enable(eth_g_index, flags);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_tpid_set(
    unsigned int netif_index,
    unsigned short value)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_tpid_set(eth_g_index, value);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}

int npd_trunk_inner_tpid_set(
    unsigned int netif_index,
    unsigned short value)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(netif_index);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = npd_port_inner_tpid_set(eth_g_index, value);
            if(0 != ret)
                all_ret = ret;			
		}
	}
	else
		ret = NPD_FAIL;
    
    return ret;
}


struct port_driver_s trunk_switchport_driver =
{
    .type = NPD_NETIF_TRUNK_TYPE,
    .set_pvid = npd_trunk_driver_set_pvid,
    .set_inner_pvid = npd_trunk_driver_set_inner_pvid,    
    .allow_vlan = npd_trunk_driver_allow_vlan,
    .remove_vlan = npd_trunk_driver_free_vlan,
    .vlan_mode_set = npd_trunk_vlan_mode_set,
    .fdb_limit_set = npd_trunk_driver_set_fdb_limit,
    .fdb_learning_mode = npd_trunk_driver_fdb_learning_mode,
    .fdb_add = npd_trunk_driver_fdb_add,
    .fdb_delete_by_port = npd_trunk_driver_fdb_delete_by_port,
    .fdb_delete_by_vlan_port = npd_trunk_driver_fdb_delete_by_vlan_port,
    .port_isolate_add = npd_trunk_driver_isolate_add,
    .port_isolate_del = npd_trunk_driver_isolate_del,
    .port_speed = npd_trunk_driver_speed,
    .port_duplex_mode = npd_trunk_driver_duplex,
#if 0
    /*stp*/
    int (*mstp_port_enable)(unsigned int g_ifindex);
    int (*mstp_port_disable)(unsigned int g_ifindex);
    int (*mstp_set_port_state)(unsigned int g_ifindex, int mstid, int state);

    /*igmp snooping*/
    int (*igmp_trap_set)(int vlan, unsigned int g_ifindex, int flags);
    int (*l2mc_entry_add_port)(void *l2_mc_entry);
    int (*l2mc_entry_del_port)(void *l2_mc_entry);
#endif
	.dhcp_trap_set = npd_trunk_driver_dhcp_trap_set,
    .mac_vlan_enable = npd_trunk_mac_vlan_enable,
    .subnet_vlan_enable = npd_trunk_subnet_vlan_enable,
    .prefer_subnet_enable = npd_trunk_prefer_subnet_enable,	
    .port_vlan_filter = npd_trunk_vlan_filter,
	.access_qinq_enable = npd_trunk_access_qinq_enable,
	.tpid_set = npd_trunk_tpid_set,
   	.inner_tpid_set = npd_trunk_inner_tpid_set,
#if 0	
    /*qinq*/
    int (*dtag_mode_set)(unsigned int g_ifindex, int mode);
    
    /*layer 3 intf*/
#endif
};


#ifdef __cplusplus
}
#endif
