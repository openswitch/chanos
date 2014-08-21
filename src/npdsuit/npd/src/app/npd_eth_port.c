
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 *UPDATE:
 *04/26/2010              zhanwei@autelan.com          Unifying netif index formate with vlan and port-channel
 *05/10/2010              zhanwei@autelan.com          Using DB.
 *06/11/2010              zhanwei@autelan.com          L3 interface supported.
 *06/21/2010              chengjun@autelan.com           Add switcport.
 *07/14/2010              chengjun@autelan.com           Add REMOVED state.
 *08/18/2010              anph@autelan.com                Fix attribute setting and displaying bugs.
 *09/27/2010              chengjun@autelan.com           Add Q-in-Q supporting.
 *  FILE REVISION NUMBER:
 *  		$Revision: 1.256 $
*******************************************************************************/
/*
  Network Platform Daemon Ethernet Port Management
*/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd_dbus.h"

#include "nbm/nbm_cpld.h"
#include "npd_eth_port.h"
#include "npd_lacp.h"

NPD_ETHPORT_ATTRIBUTE_S	ethport_default_attr_mib[] = {	\
	[ETH_INVALID] = {0,0,0,0,0,0,0,0,0,0,0},
	[ETH_FE_TX] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_100_E,	\
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,	\
		ETH_ATTR_ON,ETH_ATTR_OFF,COMBO_PHY_MEDIA_PREFER_NONE
	},
	[ETH_FE_FIBER] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_100_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,  \
		ETH_ATTR_DISABLE,ETH_ATTR_ENABLE,COMBO_PHY_MEDIA_PREFER_NONE
	},
	[ETH_GTX] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_1000_E,	\
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,	\
		ETH_ATTR_ON,ETH_ATTR_ON,COMBO_PHY_MEDIA_PREFER_NONE
	},
	[ETH_GE_FIBER] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_1000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_ENABLE,COMBO_PHY_MEDIA_PREFER_NONE
	},
	[ETH_GE_SFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_1000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_ENABLE,COMBO_PHY_MEDIA_PREFER_NONE
	},
	[ETH_GE_COMBO] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_1000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_ENABLE,COMBO_PHY_MEDIA_PREFER_FIBER
	},
	[ETH_XGE_XFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_10000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_XGTX] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_10000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_ON,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_XGE_FIBER] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_10000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_XGE_SFPPLUS] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_10000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_XGE_QSFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_10000_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_40G_QSFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_40G_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_40G_CFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_40G_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_100G_CFP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_100G_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_100G_CXP] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_100G_E,  \
		ETH_ATTR_DISABLE,ETH_ATTR_DISABLE,ETH_ATTR_OFF,ETH_ATTR_ON,  \
		ETH_ATTR_ON,ETH_ATTR_DISABLE,COMBO_PHY_MEDIA_PREFER_NONE	
	},
	[ETH_MNG] = {
		ETH_ATTR_ENABLE,1522,PORT_FULL_DUPLEX_E,PORT_SPEED_1000_E,	\
		ETH_ATTR_ENABLE,ETH_ATTR_DISABLE,ETH_ATTR_ON,ETH_ATTR_ON,	\
		ETH_ATTR_ON,ETH_ATTR_ON,COMBO_PHY_MEDIA_PREFER_NONE
	},	
};


db_table_t *eth_ports_db;
sequence_table_index_t *g_eth_ports;

array_table_index_t *npd_eth_cfg_index = NULL;

db_table_t         *npd_eth_cfgtbl = NULL;

int npd_eth_port_rate_poll_enable = 1;

extern void (*netif_notify_remote_event_callback)(void *private_data, int private_data_len);
NPD_ETH_PORT_NOTIFIER_FUNC	portNotifier = NULL;
npd_msg_list_t *npd_port_event_list = NULL;

/* port array need to poll fiber state*/
unsigned int g_eth_port_rate_poll[MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT];
unsigned int g_eth_port_rateInput[MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT]= {0};
unsigned long long g_ethport_input_bytes[MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT][6] = {{0}};
unsigned  int g_eth_port_rateOutput[MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT] = {0};
unsigned long long g_ethport_output_bytes[MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT][6] = {{0}};

static unsigned int scType = ETH_PORT_STREAM_BPS_E;
eth_port_stats_t 	g_eth_port_counter[MAX_SLOT_SUBBOARD_COUNT][MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT];
struct eth_port_counter_s 	g_stack_port_counter[MAX_SLOT_SUBBOARD_COUNT][MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT];

extern int manu_testing;

extern hash_table_index_t *npd_route_haship_index;
extern hash_table_index_t *ports_stp;
extern hash_table_index_t *npd_mroute_haship_index;
extern hash_table_index_t *npd_dhcp_snp_dbhash_mac_index;
extern hash_table_index_t *npd_fdb_hashmac_index;


struct npd_ethport_change_rate_s
{
	int fast_count;
	int auto_shutdown;
	enum PORT_NOTIFIER_ENT	last_event;
    struct eth_port_s private;
}
npd_ethport_change_fast_detect[MAX_ETHPORT_PER_SYSTEM] = {{0, 0, PORT_NOTIFIER_TYPE_MAX, {0,}},};
int global_portup_event_delay_count = 0;

int npd_set_port_sfp_laser
(
    unsigned int eth_g_index,
    unsigned int status
);

NPD_ETHPORT_ATTRIBUTE_S	*ethport_attr_default
(
	unsigned int port_type	
)
{
	return &ethport_default_attr_mib[port_type];
}


unsigned int eth_port_sfp_type_check(struct eth_port_s *portInfo);

int npd_thread_port_event_init();

void npd_eth_port_master_notifier
(
	unsigned int	eth_g_index,
	enum PORT_NOTIFIER_ENT	event,
	char *private, int len
);

void npd_eth_port_relate_notifier
(
	unsigned int	trunk_index,
	unsigned int    eth_g_index,
	enum PORT_RELATE_ENT	event,
	char *private, int len
);
int npd_eth_port_local_member_add
(
    unsigned int eth_g_index
);

netif_event_notifier_t eth_port_notifier = 
{
    .netif_event_handle_f = npd_eth_port_master_notifier,
    .netif_relate_handle_f = npd_eth_port_relate_notifier
};

/**********************************************************************************
 *  npd_init_eth_ports
 *
 *	DESCRIPTION:
 * 		this routine initialize all ethernet ports
 *
 *	INPUT:
 *		NULL
 *
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 *
 **********************************************************************************/
unsigned int npd_port_db_index(unsigned int index)
{
    return eth_port_array_index_from_ifindex(index);
}
unsigned int npd_port_key(void *port)
{
    return((struct eth_port_s*) port)->eth_port_ifindex;
}

int npd_port_cmp(void * port1, void *port2)
{
    return (((struct eth_port_s *) port1)->eth_port_ifindex
            == ((struct eth_port_s *)port2)->eth_port_ifindex);
}

long npd_eth_port_update(void *newdata, void *olddata);
long npd_eth_port_insert(void *data);
long npd_eth_port_delete(void *data);
void npd_eth_port_linkup_update(void *private_date, int len)
{
	unsigned int netif_index = 0;
	struct eth_port_s *eth_attr = (struct eth_port_s *)private_date;
    struct eth_port_s *current_eth;

   if(len != sizeof(struct eth_port_s))
	{
		syslog_ax_eth_port_err("Remote linkup event private date length error.\r\n");
	}
	syslog_ax_eth_port_dbg("Netif index: 0x%x, attibute bitmap: %x.\r\n", eth_attr->eth_port_ifindex, eth_attr->attr_bitmap);
	netif_index = eth_attr->eth_port_ifindex;
	/*\B6\D4ethernet port, trunk, switch port, vlan\B5?\C7\CA\FD\BE?\E2\B5\C4update/insert/delete\B2\D9\D7\F7\B1\D8\D0?\CF\C2\C1\D0
	   ?\D0\F2\D7\F6\CB\F8\A3\AC\D2??\D2\FD\C6\F0\CA\FD\BE?\BB?\D6\C2\CE\CA\CC\E2*/
	npd_key_database_lock();
    current_eth = npd_get_port_by_index(netif_index);
    if(NULL == current_eth)
    {
    	npd_key_database_unlock();
        return;
    }
	current_eth->state = PORT_NORMAL;
    current_eth->attr_bitmap &= (~ETH_ATTR_SPEED_MASK);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_SPEED_MASK);
    current_eth->attr_bitmap &= (~ETH_ATTR_LINK_STATUS);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_LINK_STATUS);
    current_eth->attr_bitmap &= (~ETH_ATTR_DUPLEX);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_DUPLEX);
    current_eth->attr_bitmap &= (~ETH_ATTR_FLOWCTRL);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_FLOWCTRL);
    current_eth->attr_bitmap &= (~ETH_ATTR_REAL_COPPER_MEDIA);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_REAL_COPPER_MEDIA);
    current_eth->attr_bitmap &= (~ETH_ATTR_REAL_FIBER_MEDIA);
    current_eth->attr_bitmap |= (eth_attr->attr_bitmap & ETH_ATTR_REAL_FIBER_MEDIA);
    current_eth->real_speed = eth_attr->real_speed;

	dbtable_sequence_update(g_eth_ports, netif_index, NULL, current_eth);
    npd_key_database_unlock();
    free(current_eth);
    return;
}

int npd_eth_port_ntoh(void *data)
{	
	int i = 0;
    struct eth_port_s *eth_port = (struct eth_port_s*)data;

	eth_port->eth_port_ifindex = ntohl(eth_port->eth_port_ifindex);
	eth_port->port_type = ntohl(eth_port->port_type);
    eth_port->state = ntohl(eth_port->state);
	eth_port->attr_bitmap = ntohl(eth_port->attr_bitmap);
	eth_port->mtu = ntohl(eth_port->mtu);
	eth_port->ipg = ntohl(eth_port->ipg);
	for(i=0;i<2;i++)
	    eth_port->bandwidth[i] = ntohl(eth_port->bandwidth[i]); 

	for(i=0;i<2;i++)
	    eth_port->burstsize[i] = ntohl(eth_port->burstsize[i]); 
	eth_port->lastLinkChange = ntohl(eth_port->lastLinkChange);
    eth_port->switch_port_index = ntohl(eth_port->switch_port_index);
    eth_port->trunkid = ntohl(eth_port->trunkid);
    eth_port->sc.bcast.value.pps = ntohl(eth_port->sc.bcast.value.pps);
	eth_port->sc.dlf.value.pps = ntohl(eth_port->sc.dlf.value.pps);
	eth_port->sc.mcast.value.pps = ntohl(eth_port->sc.mcast.value.pps);
    eth_port->forward_mode = ntohl(eth_port->forward_mode); 
	eth_port->vct_isable = ntohl(eth_port->vct_isable);
    eth_port->vlan_ingress_filter = ntohl(eth_port->vlan_ingress_filter);
    eth_port->eee = ntohl(eth_port->eee);
    eth_port->fdb_learning_mode = ntohl(eth_port->fdb_learning_mode);
	
	return 0;
}

int npd_eth_port_hton(void *data)
{	
	int i = 0;
	struct eth_port_s *eth_port = (struct eth_port_s*)data;

	eth_port->eth_port_ifindex = htonl(eth_port->eth_port_ifindex);
	eth_port->port_type = htonl(eth_port->port_type);
	eth_port->state = htonl(eth_port->state);
	eth_port->attr_bitmap = htonl(eth_port->attr_bitmap);
	eth_port->mtu = htonl(eth_port->mtu);
	eth_port->ipg = htonl(eth_port->ipg);
	for(i=0;i<2;i++)
		eth_port->bandwidth[i] = htonl(eth_port->bandwidth[i]); 

	for(i=0;i<2;i++)
		eth_port->burstsize[i] = htonl(eth_port->burstsize[i]); 
	eth_port->lastLinkChange = htonl(eth_port->lastLinkChange);
	eth_port->switch_port_index = htonl(eth_port->switch_port_index);
	eth_port->trunkid = htonl(eth_port->trunkid);
	eth_port->sc.bcast.value.pps = htonl(eth_port->sc.bcast.value.pps);
	eth_port->sc.dlf.value.pps = htonl(eth_port->sc.dlf.value.pps);
	eth_port->sc.mcast.value.pps = htonl(eth_port->sc.mcast.value.pps);
	eth_port->forward_mode = htonl(eth_port->forward_mode); 
	eth_port->vct_isable = htonl(eth_port->vct_isable);
	eth_port->vlan_ingress_filter = htonl(eth_port->vlan_ingress_filter);
	eth_port->eee = htonl(eth_port->eee);
	eth_port->fdb_learning_mode = htonl(eth_port->fdb_learning_mode);
	
	return 0;
}

int npd_eth_cfgtbl_handle_ntoh(void *data)
{
    struct npd_eth_cfg_s *ethCfg = (struct npd_eth_cfg_s *)data;
    ethCfg->rate_poll_enable = ntohl(ethCfg->rate_poll_enable);
    return 0;
}

int npd_eth_cfgtbl_handle_hton(void *data)
{
    struct npd_eth_cfg_s *ethCfg = (struct npd_eth_cfg_s *)data;
    ethCfg->rate_poll_enable = htonl(ethCfg->rate_poll_enable);
    return 0;
}


long npd_eth_cfgtbl_handle_update(void *newItem, void *oldItem)
{
    struct npd_eth_cfg_s* npd_eth_cfg_new = (struct npd_eth_cfg_s*)newItem;
    struct npd_eth_cfg_s* npd_eth_cfg_old = (struct npd_eth_cfg_s*)oldItem;

    if (npd_eth_cfg_new->rate_poll_enable != npd_eth_cfg_old->rate_poll_enable)
    {
        npd_eth_port_rate_poll_enable = npd_eth_cfg_new->rate_poll_enable;
    }
    return 0;
}

long npd_eth_cfgtbl_handle_insert(void *newItem)
{
    struct npd_eth_cfg_s* npd_eth_cfg_new = (struct npd_eth_cfg_s*)newItem;
	npd_eth_port_rate_poll_enable = npd_eth_cfg_new->rate_poll_enable;
    return 0;
}

void npd_init_eth_ports(void)
{
    int ret = 0;
	int npd_eth_cfg_global_no = 0;
    char name[16];
	struct npd_eth_cfg_s npd_eth_cfg_default;
    strcpy(name, "ETH_PORT_DB");
    create_dbtable(name, MAX_ETH_GLOBAL_INDEX, sizeof(struct eth_port_s),
                   npd_eth_port_update, NULL, npd_eth_port_insert,  npd_eth_port_delete, NULL, NULL, NULL, 
                   npd_eth_port_ntoh, npd_eth_port_hton, DB_SYNC_ALL, &eth_ports_db);
    dbtable_create_sequence_index("netif_index", eth_ports_db, MAX_ETH_GLOBAL_INDEX, &npd_port_db_index,
                                  &npd_port_key, &npd_port_cmp, &g_eth_ports);

    if (NULL == g_eth_ports)
    {
        syslog_ax_eth_port_dbg("memory alloc error for eth port init!!!\n");
        return;
    }

    ret = create_dbtable("npdEthCfg", 1, sizeof(struct npd_eth_cfg_s),\
                         npd_eth_cfgtbl_handle_update,
                         NULL,
                         npd_eth_cfgtbl_handle_insert,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         npd_eth_cfgtbl_handle_ntoh,
                         npd_eth_cfgtbl_handle_hton,
                         DB_SYNC_ALL,
                         &npd_eth_cfgtbl);

    if (0 != ret || npd_eth_cfgtbl == NULL)
    {
        syslog_ax_fdb_err("create npd fdb configuration table fail\n");
        return NPD_FAIL;
    }

    ret = dbtable_create_array_index("eth_cfg", npd_eth_cfgtbl, &npd_eth_cfg_index);

    if (0 != ret)
    {
        syslog_ax_fdb_err("create npd ethernet port configuration table index fail\n");
        return NPD_FAIL;
    }

    npd_eth_cfg_default.rate_poll_enable = npd_eth_port_rate_poll_enable;
    ret = dbtable_array_insert(npd_eth_cfg_index, &npd_eth_cfg_global_no, &npd_eth_cfg_default);

    if (ret != 0)
    {
        syslog_ax_fdb_err("Insert ethernet prot default configuration failed.\n");
        return NPD_FAIL;
    }

    register_netif_notifier(&eth_port_notifier);
	netif_notify_remote_event_callback = npd_eth_port_linkup_update;
    memset(g_eth_port_counter, 0, sizeof(g_eth_port_counter));
	memset(g_stack_port_counter, 0, sizeof(g_stack_port_counter));


	npd_thread_port_event_init();
	nam_thread_create("rate_moniter", (void *)eth_port_rate_poll_thread, NULL, NPD_TRUE, NPD_FALSE);
    return;
}

unsigned int eth_port_local_and_master_check(unsigned int eth_g_index)
{
    unsigned int ret = 0;
    NPD_NETIF_INDEX_U eth_ifindex;
	unsigned char tmp_devNum, tmp_portNum;
	int plane_port = -1;
    eth_ifindex.netif_index = eth_g_index;

    if (eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
    {
        return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
	
	ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);
	if(ret != 0)
	{
	    return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
	}
	if(SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
	{
        plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE,
                       tmp_devNum, tmp_portNum);
    	if(plane_port == -1 || BOARD_INNER_CONN_PORT == plane_port)
    	{
    		return ETHPORT_RETURN_CODE_ERR_NONE;
    	}
	}
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        return ETHPORT_RETURN_CODE_ERR_NONE;
    }

    return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
}

unsigned int eth_port_local_check(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
    eth_ifindex.netif_index = eth_g_index;

    if (eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
    {
        return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    if(PRODUCT_IS_BOX)
    {
        if (eth_ifindex.eth_if.slot != SYS_LOCAL_CHASSIS_INDEX)
        {
            return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        if (ETH_LOCAL_PORTNO_ISLEGAL(eth_ifindex.eth_if.sub_slot, eth_ifindex.eth_if.port + 1))
        {
            return ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
	else
	{
        if (eth_ifindex.eth_if.chassis != SYS_LOCAL_CHASSIS_INDEX || eth_ifindex.eth_if.slot != SYS_LOCAL_MODULE_SLOT_INDEX)
        {
            return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        if (ETH_LOCAL_PORTNO_ISLEGAL(eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.port + 1))
        {
            return ETHPORT_RETURN_CODE_ERR_NONE;
        }
	}

    return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
}

unsigned int eth_port_legal_check(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
    eth_ifindex.netif_index = eth_g_index;

    if (eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
    {
        return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    if(PRODUCT_IS_BOX)
    {
        if (ETH_LOCAL_PORTNO_ISLEGAL(eth_ifindex.eth_if.sub_slot, eth_ifindex.eth_if.port + 1))
        {
            return ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
	else
	{
        if (ETH_LOCAL_PORTNO_ISLEGAL(eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.port + 1))
        {
            return ETHPORT_RETURN_CODE_ERR_NONE;
        }
	}
    return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    
}
/**********************************************************************************
 *  npd_get_port_by_index
 *
 *	DESCRIPTION:
 * 		get ethernet port info by global eth index
 *
 *	INPUT:
 *		eth_g_index - ethernet port global index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL  - if parameters given are wrong
 *		portInfo	- if port found
 *
 **********************************************************************************/
struct eth_port_s * npd_get_port_by_index
(
    unsigned int eth_g_index
)
{
    struct eth_port_s *portInfo = NULL;
    int ret;

    portInfo = malloc(sizeof(struct eth_port_s));

    if (NULL == portInfo)
        return NULL;
    portInfo->eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, portInfo);

    if (0 != ret)
    {
        free(portInfo);
        return NULL;
    }

    return portInfo;
}

int npd_put_port(struct eth_port_s* portInfo)
{
    struct eth_port_s port;
    int ret;

    if (NULL == portInfo)
        return NPD_SUCCESS;
    npd_key_database_lock();
    port.eth_port_ifindex = portInfo->eth_port_ifindex;
    ret = dbtable_sequence_search(g_eth_ports, portInfo->eth_port_ifindex, &port);

    if (0 != memcmp(portInfo, &port, sizeof(struct eth_port_s)))
        ret = dbtable_sequence_update(g_eth_ports, portInfo->eth_port_ifindex, &port, portInfo);
    npd_key_database_unlock();
    free(portInfo);
    //pthread_mutex_unlock(&port_lock);
    return NPD_SUCCESS;
}

int npd_switch_port_exist_check(unsigned int netif_index)
{
    struct switch_port_db_s *portInfo = NULL;

    portInfo = npd_get_switch_port_by_index(netif_index);
    if (NULL == portInfo)
    {
        return -1;
    }
    else
    {
        free(portInfo);
    }

    return 0;
}

int npd_check_ethport_exist(unsigned int eth_g_index)
{
    struct eth_port_s *port;
    port = npd_get_port_by_index(eth_g_index);

    if (port != NULL)
    {
        free(port);
        return TRUE;
    }
    else
        return  FALSE;
}

/**********************************************************************************
 *  npd_create_eth_port
 *
 *	DESCRIPTION:
 * 		this routine create ethernet port with <slot,port> and
 *		attach port info to global data structure.
 *
 *	INPUT:
 *		slot_index - slot number(index is the same as panel slot number)
 *		eth_local_index - port index(index may different from panel number)
 *
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 *
 **********************************************************************************/
void npd_create_eth_port
(
    unsigned int chassis,
    unsigned int slot_index,
    unsigned int sub_slot,
    unsigned int eth_local_index,
    unsigned int sub_port,
    int state
)
{
    int ret;
    unsigned int link_status;
    unsigned int eth_port_ifindex = 0;
    struct eth_port_s *eth_ports = NULL;
    int port_type;
    int port_driver_type;

	/*\B6\D4\D3?\D0?\C9??\A3\ACnetif_index\D6°Ï?\AB\C8\EB\B5\C4slot?\BC\CA\C9\CF\CA\C7subslot*/
	if(PRODUCT_IS_BOX)
        eth_port_ifindex = eth_port_generate_ifindex(slot_index, sub_slot, 0, eth_local_index, sub_port);
	else
        eth_port_ifindex = eth_port_generate_ifindex(chassis, slot_index, sub_slot, eth_local_index, sub_port);
		
    syslog_ax_eth_port_dbg("Chassis %d, slot %d, sub slot %d, port %d!\r\n",chassis, slot_index, sub_slot, eth_local_index);
    eth_ports = (struct eth_port_s *)malloc(sizeof(struct eth_port_s));

    if (NULL == eth_ports)
    {
        syslog_ax_eth_port_err("memory alloc error when create ether port %d/%d!!!\n",slot_index,eth_local_index);
        return;
    }

    memset((void *)eth_ports,0,sizeof(struct eth_port_s));
    eth_ports->lastLinkChange = 0;
    eth_ports->eth_port_ifindex = eth_port_ifindex;
    eth_ports->state = state;
    eth_ports->trunkid = -1;
	
    if(PRODUCT_IS_BOX)
    {
        eth_ports->port_type = npd_get_port_type(
                         MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot),
                         ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
                         );
    }
	else
	{
        eth_ports->port_type = npd_get_port_type(
                         MODULE_TYPE_ON_SLOT_INDEX(slot_index),
                         ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
                         );
	}
    port_type = eth_ports->port_type;
    port_driver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_port_ifindex);
    
    /* admin status bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->admin_state << ETH_ADMIN_STATUS_BIT);
    /* port speed bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->speed<< ETH_SPEED_BIT);
    /* Auto-Nego bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->autoNego << ETH_AUTONEG_BIT);
    /* duplex mode bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->duplex<< ETH_DUPLEX_BIT);
    /* flow-control bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->fc<< ETH_FLOWCTRL_BIT);
    /* back-pressure bitmap*/
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->bp<< ETH_BACKPRESSURE_BIT);
    eth_ports->attr_bitmap |= (ethport_attr_default(port_type)->mediaPrefer<<ETH_PREFERRED_COPPER_MEDIA_BIT);
    eth_ports->mtu = ethport_attr_default(port_type)->mtu;
    eth_ports->ipg = ETH_ATTR_DEFAULT_MINIMUM_IPG;
    eth_ports->switch_port_index = netif_array_index_from_ifindex(eth_port_ifindex);
	npd_key_database_lock();
    dbtable_sequence_insert(g_eth_ports,  eth_port_ifindex, eth_ports);
    npd_create_switch_port(eth_port_ifindex, "ethernet", &(eth_ports->switch_port_index), FALSE);
    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_CREATE);
    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_L2CREATE);
#ifdef HAVE_POE
	if (IS_POE_PORT(slot_index, sub_slot, eth_local_index))
	{
		npd_create_poe_intf(eth_port_ifindex);
	}
#endif

    if (PORT_ONLINE_REMOVED == state)
    {
        netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_REMOVE);
    }
    else if(npd_startup_end)
    {
        ret = npd_get_port_link_status(eth_port_ifindex, (int *)&link_status);

        if (NPD_SUCCESS == ret)
        {
            {
                if (link_status == ETH_ATTR_ON)
                    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_LINKUP_E);
                else
                    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_LINKDOWN_E);
            }
        }
    }
	npd_key_database_unlock();

    free(eth_ports);
	
	if (port_type == ETH_MNG)
	{
		unsigned int ifindex;
		npd_syslog_dbg("creat l3 intf for 0x%x.\n", eth_port_ifindex);
		ret = npd_l3_intf_create(eth_port_ifindex, &ifindex);
		if (ret != 0)
		{			
			syslog_ax_eth_port_dbg("create index 0x%x error.\n",eth_port_ifindex );
		}
	}

    return;
}

void npd_delete_eth_port(
    unsigned int chassis,
    unsigned int slot_index,
    unsigned int sub_slot,
    unsigned int eth_local_index,
    unsigned int sub_port,
    int state
)
{
    int ret;
    unsigned int eth_port_ifindex = 0;
    struct eth_port_s *eth_ports = NULL;

	
	/*\B6\D4\D3?\D0?\C9??\A3\ACnetif_index\D6°Ï?\AB\C8\EB\B5\C4slot?\BC\CA\C9\CF\CA\C7subslot*/
	if(PRODUCT_IS_BOX)
        eth_port_ifindex = eth_port_generate_ifindex(slot_index, sub_slot, 0, eth_local_index, sub_port);
	else
        eth_port_ifindex = eth_port_generate_ifindex(chassis, slot_index, sub_slot, eth_local_index, sub_port);
    eth_ports = (struct eth_port_s *)malloc(sizeof(struct eth_port_s));

    if (NULL == eth_ports)
    {
        syslog_ax_eth_port_err("memory alloc error when create ether port %d/%d!!!\n",slot_index,eth_local_index);
        return;
    }
    npd_key_database_lock();
	
    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_DELETE);
    netif_app_notify_event(eth_port_ifindex, PORT_NOTIFIER_DELETE, NULL, 0);
	
    memset((void *)eth_ports,0,sizeof(struct eth_port_s));
    eth_ports->eth_port_ifindex = eth_port_ifindex;
	
    ret = dbtable_sequence_search(g_eth_ports,  eth_port_ifindex, eth_ports);

    if (-1 == ret)
    {
        goto error;
    }

    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_L2DELETE);
    netif_app_notify_event(eth_port_ifindex, PORT_NOTIFIER_L2DELETE, NULL, 0);

    if (-1 != eth_ports->switch_port_index)
        npd_delete_switch_port(eth_ports->switch_port_index);

    ret = dbtable_sequence_delete(g_eth_ports,  eth_port_ifindex, eth_ports, eth_ports);
error:
    npd_key_database_unlock();
    if (eth_ports)
        free(eth_ports);

    return;
}

void npd_remove_eth_port(
    unsigned int chassis,
    unsigned int slot_index,
    unsigned int sub_slot,
    unsigned int eth_local_index,
    unsigned int sub_port,
    int state
)
{
    int ret;
    unsigned int eth_port_ifindex = 0;
    struct eth_port_s *eth_ports = NULL;
	
    int array_id = 0;

	/*\B6\D4\D3?\D0?\C9??\A3\ACnetif_index\D6°Ï?\AB\C8\EB\B5\C4slot?\BC\CA\C9\CF\CA\C7subslot*/
	if(PRODUCT_IS_BOX)
        eth_port_ifindex = eth_port_generate_ifindex(slot_index, sub_slot, 0, eth_local_index, sub_port);
	else
        eth_port_ifindex = eth_port_generate_ifindex(chassis, slot_index, sub_slot, eth_local_index, sub_port);
    eth_ports = (struct eth_port_s *)malloc(sizeof(struct eth_port_s));

    if (NULL == eth_ports)
    {
        syslog_ax_eth_port_err("memory alloc error when create ether port %d/%d!!!\n",slot_index,eth_local_index);
        return;
    }

    memset((void *)eth_ports,0,sizeof(struct eth_port_s));
    eth_ports->eth_port_ifindex = eth_port_ifindex;
    npd_key_database_lock();
    ret = dbtable_sequence_search(g_eth_ports,  eth_port_ifindex, eth_ports);

    if (-1 == ret)
    {
        goto error;
    }
	array_id = netif_array_index_from_ifindex(eth_port_ifindex);
    npd_ethport_change_fast_detect[array_id].last_event = PORT_NOTIFIER_TYPE_MAX;
    eth_ports->state = PORT_ONLINE_REMOVED;
	eth_ports->attr_bitmap &= ~((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
    ret = dbtable_sequence_update(g_eth_ports,  eth_port_ifindex, NULL, eth_ports);
    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_REMOVE);
error:
    npd_key_database_unlock();
    if (eth_ports)
        free(eth_ports);

    return;
}

void npd_insert_eth_port(
    unsigned int chassis,
    unsigned int slot_index,
    unsigned int sub_slot,
    unsigned int eth_local_index,
    unsigned int sub_port,
    int state
)
{
    int ret;
    int link_status;
    unsigned int eth_port_ifindex = 0;
    struct eth_port_s *eth_ports = NULL;

	/*\B6\D4\D3?\D0?\C9??\A3\ACnetif_index\D6°Ï?\AB\C8\EB\B5\C4slot?\BC\CA\C9\CF\CA\C7subslot*/
	if(PRODUCT_IS_BOX)
        eth_port_ifindex = eth_port_generate_ifindex(slot_index, sub_slot, 0, eth_local_index, sub_port);
	else
        eth_port_ifindex = eth_port_generate_ifindex(chassis, slot_index, sub_slot, eth_local_index, sub_port);
    syslog_ax_eth_port_dbg("Chassis %d, slot %d, sub slot %d, port %d!\r\n",chassis, slot_index, sub_slot, eth_local_index);
    eth_ports = (struct eth_port_s *)malloc(sizeof(struct eth_port_s));

    if (NULL == eth_ports)
    {
        syslog_ax_eth_port_err("memory alloc error when create ether port %d/%d!!!\n",slot_index,eth_local_index);
        return;
    }

    memset((void *)eth_ports,0,sizeof(struct eth_port_s));
    eth_ports->eth_port_ifindex = eth_port_ifindex;
    npd_key_database_lock();
    ret = dbtable_sequence_search(g_eth_ports,  eth_port_ifindex, eth_ports);

    if (-1 == ret)
    {
        goto error;
    }
    eth_ports->lastLinkChange = 0;
    eth_ports->state = PORT_NORMAL;
    dbtable_sequence_update(g_eth_ports, eth_port_ifindex, NULL, eth_ports);
    netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_INSERT);
    ret = npd_get_port_link_status(eth_port_ifindex, (int *)&link_status);

    if (NPD_SUCCESS == ret)
    {
        {
            if (link_status == ETH_ATTR_ON)
                netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_LINKUP_E);
            else
                netif_notify_event(eth_port_ifindex, PORT_NOTIFIER_LINKDOWN_E);
        }
    }

error:
    npd_key_database_unlock();
    if (eth_ports)
        free(eth_ports);

    return;
}

/**********************************************************************************
 *  npd_init_subslot_eth_ports
 *
 *	DESCRIPTION:
 * 		this routine initialize all ethernet ports
 *
 *	INPUT:
 *		NULL
 *
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 *
 **********************************************************************************/
void npd_init_subslot_eth_ports(int chassis, int slot, int subslot, int state)
{
    int j = 0;
    syslog_ax_eth_port_dbg("Creating eth-port on chassis %d slot %d sub-slot %d module %s\r\n",	\
                           chassis, slot, subslot, module_id_str(MODULE_TYPE_ON_SUBSLOT_INDEX(slot, subslot)));

    for (j = 0; j < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, subslot); j++)
    {
        npd_create_eth_port(chassis, slot, subslot, j, 0,state);
    }

    return;
}

void npd_remove_subslot_eth_ports(int chassis, int slot, int subslot, int state)
{
    int i = 0, j = 0;
    syslog_ax_eth_port_dbg("Removing eth-port on chassis %d slot %d sub-slot %d module %s\r\n",	\
                           chassis, slot, subslot, module_id_str(MODULE_TYPE_ON_SUBSLOT_INDEX(slot, subslot)));

    for (i = 0; i < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, subslot); i++)
    {
        for(j = 0; j < MAX_SUBPORT_PER_ETHPORT; j++)
        {
            npd_remove_eth_port(chassis, slot, subslot, i, j, state);
        }
    }

    return;
}

void npd_delete_subslot_eth_ports(int chassis, int slot, int subslot, int state)
{
    int i = 0, j = 0;
    syslog_ax_eth_port_dbg("Deleting eth-port on chassis %d slot %d sub-slot %d module %s\r\n",	\
                           chassis, slot, subslot, module_id_str(MODULE_TYPE_ON_SUBSLOT_INDEX(slot, subslot)));

    for (i = 0; i < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, subslot); i++)
    {
        for(j = 0; j < MAX_SUBPORT_PER_ETHPORT; j++)
        {
            npd_delete_eth_port(chassis, slot, subslot, i, j, state);
        }
    }

    return;
}

void npd_insert_subslot_eth_ports(int chassis, int slot, int subslot, int state)
{
    int j = 0;
	
    syslog_ax_eth_port_dbg("Inserting eth-port on chassis %d slot %d sub-slot %d module %s\r\n",	\
                           chassis, slot, subslot, module_id_str(MODULE_TYPE_ON_SUBSLOT_INDEX(slot, subslot)));

    for (j = 0; j < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, subslot); j++)
    {
        npd_insert_eth_port(chassis, slot, subslot, j, 0, state);
    }

    return;
}

/**********************************************************************************
 *  npd_get_port_type
 *
 *	DESCRIPTION:
 * 		this routine get port type from module type and port number
 *
 *	INPUT:
 *		module_type - module type,such as CRSMU,6GTX,6GE SFP etc.
 *		eth_port_index - ethernet port index
 *
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 *
 **********************************************************************************/
enum eth_port_type_e npd_get_port_type
(
    int module_type,
    int panel_port
)
{
    return PPAL_PANEL_PORT_TYPE(module_type, panel_port);
}
void npd_set_port_type
(
    int module_type,
    int panel_port,
    enum eth_port_type_e port_type
)
{
    PPAL_PANEL_PORT_TYPE(module_type, panel_port) = port_type;
}
void npd_port_xg_to_4xg(unsigned int netif_index)
{
    int i = 0;
    unsigned int chassis;
    unsigned int slot_index;
    unsigned int sub_slot;
    unsigned int eth_local_index;
    chassis = npd_netif_eth_get_chassis(netif_index);
    slot_index = npd_netif_eth_get_slot(netif_index);
	sub_slot = npd_netif_eth_get_subslot(netif_index);
    eth_local_index = npd_netif_eth_get_port(netif_index);
	
    /*first delete the 4 XG ports from database*/
	for(i = 0; i < 4; i++)
	{
	    npd_delete_eth_port(chassis, slot_index, sub_slot, eth_local_index, i, 0);
	}
	
    if(PRODUCT_IS_BOX)
    {
        npd_set_port_type(
                         MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot),
                         ETH_LOCAL_INDEX2NO(slot_index, eth_local_index),
                         ETH_40G_QSFP
                         );
    }
	else
	{
        npd_set_port_type(
                         MODULE_TYPE_ON_SLOT_INDEX(slot_index),
                         ETH_LOCAL_INDEX2NO(slot_index, eth_local_index),
                         ETH_40G_QSFP
                         );
	}
	
    /*then add the a 4XG port to database*/
	npd_create_eth_port(chassis, slot_index, sub_slot, eth_local_index, 0, PORT_NORMAL);
}

void npd_port_4xg_to_xg(unsigned int netif_index)
{
    int i = 0;
    unsigned int chassis;
    unsigned int slot_index;
    unsigned int sub_slot;
    unsigned int eth_local_index;
    chassis = npd_netif_eth_get_chassis(netif_index);
    slot_index = npd_netif_eth_get_slot(netif_index);
	sub_slot = npd_netif_eth_get_subslot(netif_index);
    eth_local_index = npd_netif_eth_get_port(netif_index);
	
    /*first delete the 4 XG ports from database*/
	npd_delete_eth_port(chassis, slot_index, sub_slot, eth_local_index, 0, 0);

    if(PRODUCT_IS_BOX)
    {
        npd_set_port_type(
                         MODULE_TYPE_ON_SUBSLOT_INDEX(slot_index, sub_slot),
                         ETH_LOCAL_INDEX2NO(slot_index, eth_local_index),
                         ETH_XGE_QSFP
                         );
    }
	else
	{
        npd_set_port_type(
                         MODULE_TYPE_ON_SLOT_INDEX(slot_index),
                         ETH_LOCAL_INDEX2NO(slot_index, eth_local_index),
                         ETH_XGE_QSFP
                         );
	}
    /*then add the 4 XG ports to database*/
	for(i = 0; i < 4; i++)
	{
	    npd_create_eth_port(chassis, slot_index, sub_slot, eth_local_index, i, PORT_NORMAL);
	}
}

int npd_get_port_driver_type(unsigned int eth_g_index)
{
    int slot_index;
    int panel_port;
    int module_type;
    if(PRODUCT_IS_BOX)
    {
		slot_index = 0;
    }
	else
	{
        slot_index = npd_netif_eth_get_slot(eth_g_index);
	}
    panel_port = npd_netif_eth_get_port(eth_g_index);
    panel_port	= ETH_LOCAL_INDEX2NO(slot_index,panel_port);
    module_type = MODULE_TYPE_ON_SLOT_INDEX(slot_index);
    if(module_type == PPAL_BOARD_TYPE_NONE)
        return MODULE_DRIVER_NONE;
    else
        return PPAL_PANEL_PORT_DRVIER_TYPE(module_type, panel_port);
}



/*\BE\A1\C1\BF\B2\BB??\BD\D3?\D3\C3?\BE?\E4\C1\BFg_eth_ports*/
unsigned long eth_port_sw_link_time_change_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (-1 == ret)
    {
        return 0;
    }

    return portInfo.lastLinkChange;
}
unsigned int eth_port_sw_type_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (0 != ret)
    {
        return 0;
    }

    return portInfo.port_type;
}

unsigned int eth_port_sw_attr_bitmap_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (0 != ret)
    {
        return 0;
    }

    return portInfo.attr_bitmap;
}

unsigned int eth_port_sw_mtu_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (ret != 0)
    {
        return 0;
    }

    return portInfo.mtu;
}

unsigned int eth_port_sw_ipg_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (ret != 0)
    {
        return 0;
    }

    return portInfo.ipg;
}

int eth_port_sw_duplex_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int duplex;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (ret != 0)
    {
        return 0;
    }
    duplex = (portInfo.attr_bitmap&ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;

    return duplex;
}

int eth_port_sw_admin_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int adminStatus;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (ret != 0)
    {
        return NPD_FAIL;
    }
    adminStatus = (portInfo.attr_bitmap&ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;

    return adminStatus;
}
int eth_port_sw_speed_get(unsigned int eth_g_index)
{
    struct eth_port_s portInfo ;
    int speed;
    int ret;
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (ret != 0)
    {
        return 0;
    }
    speed = (portInfo.attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

    switch(speed)
    {
        case PORT_SPEED_10_E:
            return 10;
        case PORT_SPEED_100_E:
            return 100;
        case PORT_SPEED_1000_E:
            return 1000;
        case PORT_SPEED_10000_E:
            return 100000;
        default:
            return 0;
    }
       
}

int eth_port_sw_attr_update(unsigned int eth_g_index, unsigned int type, unsigned int value)
{
    int ret = 0;
    struct eth_port_s* g_ptr = NULL;
    unsigned int eee_state = 0;
	npd_key_database_lock();
    g_ptr = npd_get_port_by_index(eth_g_index);
    if (g_ptr)
    {
        switch (type)
        {
            case ADMIN:
                value = value&0x1;
                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_ADMIN_STATUS_BIT) & ETH_ATTR_ADMIN_STATUS);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_ADMIN_STATUS_BIT) & ETH_ATTR_ADMIN_STATUS));
                }

                break;
            case ETH_SPEED:
				ret = ETHPORT_RETURN_CODE_ERR_NONE;
#if 0
				if (value == 40000)
				{
				    if(g_ptr->port_type == ETH_XGE_QSFP)/*change 10G to 40G*/
				    {
				        npd_port_xg_to_4xg(eth_g_index);
						ret = ETHPORT_RETURN_CODE_ERR_NONE;
                        npd_key_database_unlock();
                        return ret;
				    }
					else if(g_ptr->port_type == ETH_40G_QSFP)
					{
					    break;
					}
					else
					{
                        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                        break; 
					}
				}
				else if (value == 10000)
				{
				    if(g_ptr->port_type == ETH_40G_QSFP)/*change 40G to 10G*/
				    {
				        npd_port_4xg_to_xg(eth_g_index);
						ret = ETHPORT_RETURN_CODE_ERR_NONE;
                        npd_key_database_unlock();
                        return ret;
				    }
					else if(g_ptr->port_type == ETH_XGE_QSFP)
					{
					    break;
					}
				}
#endif
				switch (value)
				{
					case 10:
						if(g_ptr->port_type != ETH_FE_TX && g_ptr->port_type != ETH_GTX
							&& g_ptr->port_type != ETH_GE_COMBO)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
							npd_key_database_unlock();
                            return ret;
						}
						g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
                        g_ptr->attr_bitmap &= 0xFFFF0FFF;
                        g_ptr->attr_bitmap |= ((PORT_SPEED_10_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
						break;
					case 100:
						if(g_ptr->port_type != ETH_FE_TX && g_ptr->port_type != ETH_GTX
							&& g_ptr->port_type != ETH_GE_COMBO && g_ptr->port_type != ETH_GE_SFP
							&& g_ptr->port_type != ETH_FE_FIBER)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
							npd_key_database_unlock();
                            return ret;
						}
						g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
                        g_ptr->attr_bitmap &= 0xFFFF0FFF;
                        g_ptr->attr_bitmap |= ((PORT_SPEED_100_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
					    break;
					case 1000:
						if(g_ptr->port_type != ETH_GTX && g_ptr->port_type != ETH_GE_COMBO 
							&& g_ptr->port_type != ETH_GE_SFP && g_ptr->port_type != ETH_XGTX
							&& g_ptr->port_type != ETH_XGE_SFPPLUS && g_ptr->port_type != ETH_XGE_XFP)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
							npd_key_database_unlock();
                            return ret;
						}
						if(g_ptr->port_type == ETH_GTX || g_ptr->port_type == ETH_GE_COMBO)
						{
						    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
						}
						else if(g_ptr->port_type == ETH_GE_SFP || g_ptr->port_type == ETH_XGE_SFPPLUS)
						{
						    g_ptr->attr_bitmap |= ((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG);
						}
						
                        g_ptr->attr_bitmap &= 0xFFFF0FFF;
                        g_ptr->attr_bitmap |= ((PORT_SPEED_1000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
						break;
					case 10000:
						if(g_ptr->port_type != ETH_XGE_XFP && g_ptr->port_type != ETH_XGTX 
							&& g_ptr->port_type != ETH_XGE_SFPPLUS && g_ptr->port_type != ETH_XGE_FIBER
							&& g_ptr->port_type != ETH_XGE_QSFP && g_ptr->port_type != ETH_40G_QSFP)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
							npd_key_database_unlock();
                            return ret;
						}
    				    if(g_ptr->port_type == ETH_40G_QSFP)/*change 40G to 10G*/
    				    {
    				        npd_port_4xg_to_xg(eth_g_index);
    						ret = ETHPORT_RETURN_CODE_ERR_NONE;
                            npd_key_database_unlock();
                            return ret;
    				    }
                        g_ptr->attr_bitmap &= 0xFFFF0FFF;
                        g_ptr->attr_bitmap |= ((PORT_SPEED_10000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
						break;
					case 40000:
						if(g_ptr->port_type != ETH_40G_QSFP && g_ptr->port_type != ETH_40G_CFP
							&& g_ptr->port_type != ETH_XGE_QSFP)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
							npd_key_database_unlock();
                            return ret;
						}
    				    if(g_ptr->port_type == ETH_XGE_QSFP)/*change 10G to 40G*/
    				    {
    				        npd_port_xg_to_4xg(eth_g_index);
    						ret = ETHPORT_RETURN_CODE_ERR_NONE;
                            npd_key_database_unlock();
                            return ret;
    				    }
						break;
					default:
						ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
						break;
				}
#if 0
				if(g_ptr->attr_bitmap & ETH_ATTR_AUTONEG)
				{
                    if (value == 10000)
                    {
                        if(g_ptr->port_type == ETH_XGE_SFPPLUS || g_ptr->port_type == ETH_XGE_FIBER 
                        || g_ptr->port_type == ETH_XGE_XFP || g_ptr->port_type == ETH_XGE_QSFP)
                        {
                           g_ptr->attr_bitmap &= 0xFFFF0FFF;
                           g_ptr->attr_bitmap |= ((PORT_SPEED_10000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
						   break;
                        }
                    }
					else if(value == 1000)
					{
						if((g_ptr->port_type == ETH_GE_FIBER) || (g_ptr->port_type == ETH_GE_SFP)
							|| (g_ptr->port_type == ETH_XGE_SFPPLUS || g_ptr->port_type == ETH_XGE_FIBER 
                        || g_ptr->port_type == ETH_XGE_XFP || g_ptr->port_type == ETH_XGE_QSFP))
						{
                           g_ptr->attr_bitmap &= 0xFFFF0FFF;
                           g_ptr->attr_bitmap |= ((PORT_SPEED_1000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
						   break;
						}
						if(g_ptr->port_type == ETH_GTX)
						{
							ret = ETHPORT_RETURN_CODE_ERR_1000M_AN_DISABLED;
							break;
						}
					}
					/*
                    eee_state = g_ptr->eee;
                	if (eee_state != ETH_ATTR_EEE_ENABLE)
                	{
                        ret = ETHPORT_RETURN_CODE_ERR_OPERATE;
    					break;
                	}
                	*/
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
				}
                if ((g_ptr->attr_bitmap&ETH_ATTR_AUTONEG)==0)
                {
                    if (value == 10)
                    {
                        if(g_ptr->port_type == ETH_FE_TX || g_ptr->port_type == ETH_GTX 
                        || g_ptr->port_type == ETH_GE_COMBO || g_ptr->port_type == ETH_FE_FIBER)
                        {
                            value = PORT_SPEED_10_E;
                        }
                        else
                        {
                            ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
    					    break;
                        }   
                       
                    }
                    else if (value == 100)
                    {
                        if(g_ptr->port_type == ETH_XGE_XFP || g_ptr->port_type == ETH_XGTX 
                        || g_ptr->port_type == ETH_XGE_FIBER ||g_ptr->port_type == ETH_XGE_SFPPLUS)
                        {
                            ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                            break; 
                        }
                        else
                        {
                            value = PORT_SPEED_100_E;
                        }
                    }
                    else if (value == 1000)
                    {
						if(g_ptr->port_type == ETH_FE_FIBER || g_ptr->port_type == ETH_FE_TX)
						{
						    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                            break;
						}
						else
						{
                            value = PORT_SPEED_1000_E;
						}
                    }
                    else if (value == 10000)
                    {
                        if(g_ptr->port_type == ETH_XGE_SFPPLUS || g_ptr->port_type == ETH_XGE_FIBER 
                        || g_ptr->port_type == ETH_XGTX || g_ptr->port_type == ETH_XGE_XFP || g_ptr->port_type == ETH_XGE_QSFP
                        || g_ptr->port_type == ETH_40G_QSFP)
                        {
                            value = PORT_SPEED_10000_E;
                        }
                        else
                        {
                            ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                            break;
                        }
                    }
                    else if (value == 40000)
                    {
                        if(g_ptr->port_type == ETH_40G_QSFP)
                        {
                            value = PORT_SPEED_40G_E;
                        }
                        else
                        {
                            ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                            break;
                        }
                    }
                    if(ETHPORT_RETURN_CODE_ERR_NONE == ret)
                    {
                        g_ptr->attr_bitmap &= 0xFFFF0FFF;
                        g_ptr->attr_bitmap |= ((value << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
                    }
                }

#endif
                break;

            case DUMOD:
                value = value&0x1;
				if(g_ptr->attr_bitmap & ETH_ATTR_AUTONEG)
				{
					/*
                    eee_state = g_ptr->eee;
                	if (eee_state != ETH_ATTR_EEE_ENABLE)
                	{
                        ret = ETHPORT_RETURN_CODE_ERR_OPERATE;
    					break;
                	}
					*/
				}

                if (value)/*FULL DUPLEX*/
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
                    g_ptr->attr_bitmap &= (~((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE));
                    g_ptr->attr_bitmap &= (~((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX));
					if(((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_1000_E)
						&& (g_ptr->port_type == ETH_GTX))
					{
                           g_ptr->attr_bitmap &= 0xFFFF0FFF;
                           g_ptr->attr_bitmap |= ((PORT_SPEED_100_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
					}
                }
                else
                {
					if(((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_1000_E)
					|| ((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_10000_E))
					{
                        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
						break;
					}
					
					if(((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_100_E)
						&& (g_ptr->port_type == ETH_FE_FIBER || g_ptr->port_type == ETH_GE_FIBER || g_ptr->port_type == ETH_GE_SFP))
					{
						ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
						break;
					}
                    g_ptr->attr_bitmap &= (~((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL));
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
                    g_ptr->attr_bitmap |= ((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX);
                }

                break;
            case BACKPRE:
                value = value&0x1;
/*
				if(g_ptr->attr_bitmap & ETH_ATTR_AUTONEG)
				{
                    eee_state = g_ptr->eee;
                	if (eee_state != ETH_ATTR_EEE_ENABLE)
                	{
                        ret = ETHPORT_RETURN_CODE_ERR_OPERATE;
    					break;
                	}
					
				}
*/
                if (value)
                {
                    if((g_ptr->port_type != ETH_GTX) && (g_ptr->port_type != ETH_FE_TX))
                    {
						ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
						break;
                    }
					if(((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_1000_E)
					|| ((g_ptr->attr_bitmap&ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT == PORT_SPEED_10000_E))
					{
                       g_ptr->attr_bitmap &= 0xFFFF0FFF;
                       g_ptr->attr_bitmap |= ((PORT_SPEED_100_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
					}
					
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
                    g_ptr->attr_bitmap |= ((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX);
                    g_ptr->attr_bitmap |= ((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE));
                }
                break;            
			case FLOWCTRL:
                value = value&0x1;
				if(g_ptr->attr_bitmap & ETH_ATTR_DUPLEX)
				{
					return ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF;
				}
                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL));
                }

                break;

            case AUTONEGT:
                value = value&0x1;

                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG);
                    
                    if((g_ptr->port_type == ETH_GE_FIBER) || (g_ptr->port_type == ETH_GTX)
						|| (g_ptr->port_type == ETH_GE_SFP)||(g_ptr->port_type == ETH_GE_COMBO))
                    {
                         g_ptr->attr_bitmap &= 0xFFFF0FFF;
                         g_ptr->attr_bitmap |= ((PORT_SPEED_1000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK); /*SPEED 1000*/
                    }
				#if 0
                    else if(g_ptr->port_type == ETH_XGE_SFPPLUS || g_ptr->port_type == ETH_XGE_FIBER 
                        || g_ptr->port_type == ETH_XGE_XFP)
                    {
                       g_ptr->attr_bitmap &= 0xFFFF0FFF;
                       g_ptr->attr_bitmap |= ((PORT_SPEED_10000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
                    }
				#endif
                    g_ptr->attr_bitmap &= (~((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX));                  /*FULL DUPLEX*/
                    g_ptr->attr_bitmap &= (~((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE));      /*back pressure disable*/                 /*FULL DUPLEX*/
                    
                }
                else
                {
                    eee_state = g_ptr->eee;
                	if (eee_state != ETH_ATTR_EEE_ENABLE)
                	{
                    	g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG));
						#if 0
                    	g_ptr->attr_bitmap &= 0xFFFF0FFF;
                    	if(g_ptr->port_type == ETH_XGE_SFPPLUS || g_ptr->port_type == ETH_XGE_FIBER 
                    	|| g_ptr->port_type == ETH_XGTX || g_ptr->port_type == ETH_XGE_XFP)
                    	{
                       		g_ptr->attr_bitmap |= ((PORT_SPEED_10000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK); /*SPEED 10000*/
                    	}
						else 
						if((g_ptr->port_type == ETH_GE_FIBER) || (g_ptr->port_type == ETH_GE_SFP))
						{
                       		g_ptr->attr_bitmap |= ((PORT_SPEED_1000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK); /*SPEED 10000*/
						}
                    	else
                    	{
                        	g_ptr->attr_bitmap |= ((PORT_SPEED_100_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK); /*SPEED 100*/
                    	}
						#endif
                    	//g_ptr->attr_bitmap &= (~((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL));           /*DISABLE FLOWCTRL*/
                    	g_ptr->attr_bitmap &= (~((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX));                  /*FULL DUPLEX*/
                    	g_ptr->attr_bitmap &= (~((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE));   /*DISABLEBACKPRESSURE*/
                    	// g_ptr->attr_bitmap |= ((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE);      /*ENABLEBACKPRESSURE*/
                	}
					else 
					{
						ret = ETHPORT_RETURN_CODE_ERR_OPERATE;
					}
                }

                break;
            case AUTONEGTS:
                value = value&0x1;

                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_AUTONEG_SPEED_BIT) & ETH_ATTR_AUTONEG_SPEED);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_SPEED_BIT) & ETH_ATTR_AUTONEG_SPEED));
                }

                break;
            case AUTONEGTD:
                value = value&0x1;

                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_AUTONEG_DUPLEX_BIT) & ETH_ATTR_AUTONEG_DUPLEX);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_DUPLEX_BIT) & ETH_ATTR_AUTONEG_DUPLEX));
                }

                break;
            case AUTONEGTF:
                value = value&0x1;

                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_AUTONEG_FLOWCTRL_BIT) & ETH_ATTR_AUTONEG_FLOWCTRL);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_AUTONEG_FLOWCTRL_BIT) & ETH_ATTR_AUTONEG_FLOWCTRL));
                }

                break;            case LINKS:
                value = value&0x1;

                if (value)
                {
                    g_ptr->attr_bitmap |= ((1 << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
                }
                else
                {
                    g_ptr->attr_bitmap &= (~((1 << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS));
                }

                break;
            case CFGMTU:
                g_ptr->mtu = value;
                break;
            case CFGMEDIA:

                if (g_ptr->port_type != ETH_GE_COMBO)
                    ret = ETHPORT_RETURN_CODE_UNSUPPORT;
                else
                {
                    value = value&0x3;
                    g_ptr->attr_bitmap &= 0xCFFFFFFF;
                    g_ptr->attr_bitmap |= (value << ETH_PREFERRED_COPPER_MEDIA_BIT);
                }

                break;
            default:
                break;
        }

        npd_put_port(g_ptr);
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
    npd_key_database_unlock();
    return ret;
}

extern int nam_vct_phy_enable
(
	unsigned int netif_index,
	unsigned int state
);

int npd_vct_set(unsigned int netif_index, unsigned int state)
{
    int ret = 0;
	unsigned long portattr;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x vct set %d\n",netif_index, state);
	
	if(state == 0)
	{
	    syslog_ax_eth_port_dbg("npd_vct_set_state = 0");
		
	    return ret;
	}
	
	ret = nam_get_port_link_state(netif_index, &portattr);
	syslog_ax_eth_port_dbg("\n linkget_ret = %d, portattr = %d\n", ret, portattr);
	if(ret != 0)
	{   
		return DIAG_RETURN_CODE_ERROR;
    }
    if(portattr == 1)
    {
        ret = nam_vct_phy_enable(netif_index, 1);
    }
	else
	{
	    ret = nam_vct_phy_enable(netif_index, 0);
	}
	
	if (0 != ret)
    {
        ret = DIAG_RETURN_CODE_ERROR;
    }
	
  
    return ret;
}


int npd_set_eth_port_ipg(unsigned int eth_g_index, int port_type, unsigned int eth_ipg)
{
    int ret = -1;
    enum module_driver_type_e module_driver_type = MODULE_DRIVER_NONE;
    module_driver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);
    int speed;

    switch (module_driver_type)
    {
        case MODULE_DRIVER_NAM_BCM:
        case MODULE_DRIVER_NAM_CPSS:
        case MODULE_DRIVER_NAM_CTC:
	    case MODULE_DRIVER_NAM_ATHEROS:
            speed = ethport_attr_default(port_type)->speed;
            ret = nam_set_ethport_ipg(eth_g_index, speed, eth_ipg);

            if (0 != ret)
            {
                ret = ETHPORT_RETURN_CODE_ERR_HW;
            }
            else
            {
                ret = ETHPORT_RETURN_CODE_ERR_NONE;
            }

            break;
        default:
            ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
            break;
    }

    return ret;
}

/**********************************************************************************
 *  npd_eth_port_combo_media_check
 *
 *	DESCRIPTION:
 * 		this function check a port whether defaule media type or not
 *
 *	INPUT:
 *		module_id - module type,such as CRSMU,6GTX,6GE SFP etc.
 *		eth_port_no - ethernet port number
 *
 *
 *	OUTPUT:
 *		type - whether defaule media type or not
 *
 * 	RETURN:
 *		NPD_SUCCESS  - if no error occurred
 *		NPD_FAIL	 - if error occurred
 *
 **********************************************************************************/
int npd_set_port_media_type(
    unsigned int eth_g_index,
    unsigned int media
)
{
    int ret;
    syslog_ax_eth_port_dbg("Set gindex 0x%x media type %d\n",eth_g_index,media);
    ret = nam_set_eth_port_trans_media(eth_g_index,media);

    if (NPD_DBUS_ERROR_UNSUPPORT == ret)
    {
        ret = ETHPORT_RETURN_CODE_UNSUPPORT;
    }
    else if (NPD_DBUS_ERROR == ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == ret)
    {
        ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
    }

    return ret;
}

int npd_eth_port_combo_check
(
	unsigned int eth_g_index
)
{
	unsigned char port_type = 0;
	int slot_index = eth_port_get_slot_by_ifindex(eth_g_index);
	int eth_local_index = eth_port_get_portno_by_ifindex(eth_g_index);
	
    if(PRODUCT_IS_BOX)
    {
	    port_type = PPAL_PANEL_PORT_TYPE(
							MODULE_TYPE_ON_SUBSLOT_INDEX(0, slot_index),
	                        ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
	                       );
    }
	else
	{
	    port_type = PPAL_PANEL_PORT_TYPE(
							MODULE_TYPE_ON_SLOT_INDEX(slot_index),
	                        ETH_LOCAL_INDEX2NO(slot_index,eth_local_index)
	                       );
	}
	if (ETH_GE_COMBO == port_type)
		return NPD_SUCCESS;
	else
		return NPD_FAIL;	
}
int npd_port_media_get
(
    unsigned int eth_g_index,
    enum eth_port_type_e *portMedia
)
{
    unsigned int ret = 0;
    enum eth_port_type_e tmpPortMedia = ETH_INVALID;
    ret = nam_port_phy_port_media_type(eth_g_index,&tmpPortMedia);

    if (0 == ret)
    {
        syslog_ax_eth_port_dbg("get port(%x) mediaType :: %d\n",eth_g_index,tmpPortMedia);
        *portMedia = tmpPortMedia;
        return NPD_TRUE;
    }
    else
        return NPD_FALSE;

}



int npd_eth_port_hw_sw_sync(void *data)
{
    struct eth_port_s *new_port = (struct eth_port_s*)data;
    int retval, ret;
    {
        unsigned char an_state = 0,an_fc = 0,fc = 0,an_duplex = 0,duplex = 0,an_speed = 0;
        unsigned char admin_status = 0;
        PORT_SPEED_ENT speed = PORT_SPEED_10_E;
        {
            unsigned int hw_admin_status;
            admin_status = (new_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
            ret = npd_get_port_admin_status(new_port->eth_port_ifindex, &hw_admin_status);

            if ((NPD_SUCCESS != ret)
                    || (hw_admin_status != admin_status))
            {
                ret = npd_set_port_admin_status(new_port->eth_port_ifindex, admin_status);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;
            }
        }
#if 0

        {
            unsigned int hw_bp;
            bp = (new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
            ret = npd_get_port_backPressure_state(new_port->eth_port_ifindex, &hw_bp);

            if ((NPD_SUCCESS != ret)
                    || (hw_bp != bp))
            {
                ret = npd_set_port_backPressure_state(new_port->eth_port_ifindex, bp);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;
            }
        }
#endif
        {
            unsigned int hw_an_state;
            an_state = (new_port->attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;
            ret = npd_get_port_autoNego_status(new_port->eth_port_ifindex, &hw_an_state);

            if ((NPD_SUCCESS != ret)
                    || (hw_an_state != an_state))
            {
                ret = npd_set_port_autoNego_status(new_port->eth_port_ifindex, an_state);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;
            }
        }
        {
            an_speed = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT;
            ret = npd_set_port_autoNego_speed(new_port->eth_port_ifindex, an_speed);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
        {
            PORT_SPEED_ENT hw_speed;
            speed = (new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;
            ret = npd_get_port_speed(new_port->eth_port_ifindex, &hw_speed);

            if ((NPD_SUCCESS != ret)
                    || (speed != hw_speed))
            {
                ret = npd_set_port_speed(new_port->eth_port_ifindex, speed);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;
            }
        }
        {
            an_duplex = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT;
            ret = npd_set_port_autoNego_duplex(new_port->eth_port_ifindex, an_duplex);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
        {
            duplex = (new_port->attr_bitmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;
            ret = npd_set_port_duplex_mode(new_port->eth_port_ifindex, duplex);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
        {
            an_fc = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT;
            ret = npd_set_port_autoNego_flowctrl(new_port->eth_port_ifindex, an_fc);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
        {
            fc = (new_port->attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
            ret = npd_set_port_flowCtrl_state(new_port->eth_port_ifindex, fc);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
        {
            unsigned int media;
            media = (new_port->attr_bitmap & (ETH_ATTR_PREFERRED_COPPER_MEDIA|ETH_ATTR_PREFERRED_FIBER_MEDIA))>> ETH_PREFERRED_COPPER_MEDIA_BIT;
            ret = npd_set_port_media_type(new_port->eth_port_ifindex, media);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;
        }
    }
    {
        ret = npd_set_port_mru(new_port->eth_port_ifindex, new_port->mtu);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
    }
    {
        ret = npd_set_eth_port_ipg(new_port->eth_port_ifindex, new_port->port_type, new_port->ipg);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
    }
    {
        int portType;
        int portflg;
        portType = new_port->port_type;
        portflg = ((ETH_FE_TX == portType)||(ETH_FE_FIBER == portType))? 0:1;

        if (new_port->sc.dlf.bpsValid)
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_DLF,
                                          new_port->sc.dlf.value.bps,portflg);
        }
        else if (new_port->sc.dlf.ppsValid)
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_DLF,
                                          new_port->sc.dlf.value.pps,portflg);
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if (new_port->sc.bcast.bpsValid)
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_BCAST,
                                          new_port->sc.dlf.value.bps,portflg);
        }
        else if (new_port->sc.bcast.ppsValid)
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_BCAST,
                                          new_port->sc.dlf.value.pps,portflg);
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if (new_port->sc.mcast.bpsValid)
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_MCAST,
                                          new_port->sc.dlf.value.bps,portflg);
        }
        else if (new_port->sc.mcast.ppsValid)
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_MCAST,
                                          new_port->sc.dlf.value.pps,portflg);
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
    }

    if (new_port->vct_isable)
    {
        ret = npd_vct_set(new_port->eth_port_ifindex, 1);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
    }

    return retval;
}

extern int nam_eth_port_set_bandwidth
(
    unsigned int eth_g_index,
    int type,
    unsigned int bandwidth,
    unsigned int burstsize
);

int npd_set_port_bandwidth(
    unsigned int eth_g_index,
    int type,
    unsigned int bandwidth,
    unsigned int burstsize
    )
{
    return nam_eth_port_set_bandwidth(eth_g_index,type,bandwidth,burstsize);
}

int npd_set_port_eee
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
	unsigned long autoNego_status = 0;
    enum module_driver_type_e module_drver_type;

	ret = nam_get_port_autoneg_state(eth_g_index, &autoNego_status);
	if (0 != ret)
	{
		return ret;
	}
	else if (ETH_ATTR_AUTONEG_DONE != autoNego_status)
	{
		return ETHPORT_RETURN_CODE_ERR_OPERATE;
	}
	
    module_drver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);
    syslog_ax_eth_port_dbg("Set gindex 0x%x Energy Efficient Ethernet(EEE) states %d\n",eth_g_index,status);

    if (MODULE_DRIVER_NAM_BCM == module_drver_type)
    {
        ret = nam_set_ethport_eee(eth_g_index,attr);
    }
    return ret;
}
int npd_get_port_eee
(
    unsigned int eth_g_index,
    unsigned int *state
)
{
    unsigned int attr = 0;
    int ret = NPD_FAIL;
    ret = nam_get_ethport_eee(eth_g_index,&attr);

    if (ret != 0)
    {
        return ret;
    }

    if (ETH_ATTR_EEE_ENABLE == attr)
    {
        *state = ETH_ATTR_EEE_ENABLE;
    }
    else if (ETH_ATTR_EEE_DISABLE == attr)
    {
        *state = ETH_ATTR_EEE_DISABLE;
    }

    return ret;
}

int npd_get_port_loopback
(
    unsigned int eth_g_index,
    int *state
)
{
    int ret = NPD_FAIL;
    ret = nam_get_port_loopback(eth_g_index,state);

    return ret;
}

long npd_eth_port_insert(void *data)
{
    struct eth_port_s *new_port = (struct eth_port_s*)data;
    int ret = 0, retval = 0;
    int port_type = new_port->port_type;
    unsigned int link_status;
	unsigned char tmp_devNum, tmp_portNum;
	
    /*insert eth to hardware, for special handle*/
	ret = nam_eth_port_insert(new_port->eth_port_ifindex);
	
    if (ETH_ATTR_LINKDOWN == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
    {
		ret = nam_set_ethport_linkstate(new_port->eth_port_ifindex, ETH_ATTR_LINKDOWN);
		if(new_port->trunkid != -1)
		{
			ret = nam_asic_trunk_ports_hwdel(new_port->eth_port_ifindex, new_port->trunkid);
		}
    }
	else if (ETH_ATTR_LINKUP == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
	{
		ret = nam_set_ethport_linkstate(new_port->eth_port_ifindex, ETH_ATTR_LINKUP);
		if(new_port->trunkid != -1)
		{
            ret = nam_asic_trunk_ports_hwadd(new_port->eth_port_ifindex, new_port->trunkid, TRUE);
		}
	}

	/*if the port is not in local board, than only return success*/

	ret = npd_get_devport_by_global_index(new_port->eth_port_ifindex, &tmp_devNum, &tmp_portNum);


    if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
        return NPD_SUCCESS;
	
    npd_eth_port_local_member_add(new_port->eth_port_ifindex);
	
    {
        unsigned char an_state = 0,fc = 0,duplex = 0;
        unsigned char admin_status = 0;
        PORT_SPEED_ENT speedMode = PORT_SPEED_10_E, speed = 10;
        an_state = (new_port->attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;
        {
            admin_status = (new_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
			
            if(!npd_startup_end)
                ret = npd_set_port_admin_status(new_port->eth_port_ifindex, ETH_ATTR_DISABLE);
            else
                ret = npd_set_port_admin_status(new_port->eth_port_ifindex, admin_status);

			if (ETHPORT_RETURN_CODE_ERR_NONE == eth_port_sfp_type_check(new_port))
			{
				ret = npd_set_port_sfp_laser(new_port->eth_port_ifindex, admin_status);
			}

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_admin_status");
            }
        }
		if (port_type == ETH_GE_COMBO)
        {
            unsigned int media;
            media = (new_port->attr_bitmap & (ETH_ATTR_PREFERRED_COPPER_MEDIA|ETH_ATTR_PREFERRED_FIBER_MEDIA))>> ETH_PREFERRED_COPPER_MEDIA_BIT;
            ret = npd_set_port_media_type(new_port->eth_port_ifindex, media);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_media_type");
            }
        }
        {   
            ret = npd_set_port_autoNego_status(new_port->eth_port_ifindex, an_state);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_autoNego_status");
            }
        }
        {
                fc = (new_port->attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
                ret = npd_set_port_flowCtrl_state(new_port->eth_port_ifindex, fc);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_flowCtrl_state");
                }
        }
        
        if (an_state != ETH_ATTR_AUTONEG_DONE )
        {
            {
                duplex = (new_port->attr_bitmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;
                ret = npd_set_port_duplex_mode(new_port->eth_port_ifindex, duplex);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_duplex_mode");
                }
            }
            {
                speedMode = (new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

                if (speedMode == PORT_SPEED_10_E)
                    speed = 10;
                else if (speedMode == PORT_SPEED_100_E)
                    speed = 100;
                else if (speedMode == PORT_SPEED_1000_E)
                    speed = 1000;
                else if (speedMode == PORT_SPEED_10000_E)
                    speed = 10000;
                else if (speedMode == PORT_SPEED_40G_E)
                    speed = 40000;
                else if (speedMode == PORT_SPEED_100G_E)
                    speed = 100000;


                ret = npd_set_port_speed(new_port->eth_port_ifindex, speed);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_speed");
                }
            }
            {
                fc = (new_port->attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
                ret = npd_set_port_flowCtrl_state(new_port->eth_port_ifindex, fc);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_flowCtrl_state");
                }
            }
            {
                unsigned int bp = (new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
                ret = npd_set_port_backPressure_state(new_port->eth_port_ifindex, bp);
    
                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;
    
                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_backPressure_state");
                }
            }        
		}
        else
		{
            speedMode = (new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

            if (speedMode == PORT_SPEED_1000_E)
            {
                speed = 1000;
            }
            else if (speedMode == PORT_SPEED_10000_E)
            {
                speed = 10000;
            }
            else if (speedMode == PORT_SPEED_40G_E)
            {
                speed = 40000;
            }
            else if (speedMode == PORT_SPEED_100G_E)
            {
                speed = 100000;
            }
            if(speed >= 1000)
            {
                ret = npd_set_port_speed(new_port->eth_port_ifindex, speed);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_speed");
                }
            }
		}
        ret = npd_get_port_link_status(new_port->eth_port_ifindex, (int *)&link_status);

		/* Added by wangquan */
		ret = eth_port_local_and_master_check(new_port->eth_port_ifindex);
		
        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_get_port_link_status");
        }
        else
        {
            if (link_status != (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
            {
                if (!SYS_LOCAL_MODULE_ISMASTERACTIVE && npd_startup_end)
                {
#ifdef HAVE_CHASSIS_SUPPORT	
    				new_port->state = PORT_NORMAL;
                    if (link_status == ETH_ATTR_LINKDOWN)
                        netif_remote_notify_event(SYS_MASTER_ACTIVE_SLOT_INDEX,
                                                  new_port->eth_port_ifindex, PORT_NOTIFIER_LINKDOWN_E, NULL, 0);
    
                    if (link_status == ETH_ATTR_LINKUP)
                    {
                        ret = npd_get_port_speed(new_port->eth_port_ifindex, &speed);
    			        new_port->attr_bitmap &= (~ETH_ATTR_SPEED_MASK);
                        new_port->attr_bitmap |= (speed << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK;
                        netif_remote_notify_event(SYS_MASTER_ACTIVE_SLOT_INDEX,
                                                  new_port->eth_port_ifindex, PORT_NOTIFIER_LINKUP_E, new_port, sizeof(struct eth_port_s));
                    }
#endif					
                }
            }
    		if(ETH_ATTR_LINKUP == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
    		{
    			if (new_port->port_type == ETH_GE_COMBO)
    			{
    				ret = SYS_LOCAL_MODULE_LED_LIGHT(new_port->eth_port_ifindex);
    			}			
    		}
        }
    }
	if(new_port->ipg)
	{
        ret = npd_set_eth_port_ipg(new_port->eth_port_ifindex, new_port->port_type, new_port->ipg);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_set_eth_port_ipg");
        }
	}
    if (new_port->bandwidth[0])
    {
        npd_set_port_bandwidth(new_port->eth_port_ifindex, 0,
            new_port->bandwidth[0], new_port->burstsize[0]);
            
    }
    if (new_port->bandwidth[1])
    {
        npd_set_port_bandwidth(new_port->eth_port_ifindex, 1,
            new_port->bandwidth[1], new_port->burstsize[1]);
            
    }
    {
        ret = npd_set_port_mru(new_port->eth_port_ifindex, new_port->mtu);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_set_port_mru");
        }
    }

    {
        ret = nam_ethport_set_ipsg(new_port->eth_port_ifindex, new_port->ip_sg);
    }

    {
    	ret = npd_set_port_eee(new_port->eth_port_ifindex, new_port->eee);
        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
            
        if(ret != NPD_SUCCESS)
        {
           npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_eee");
        }
    }
    
    return 0;
}


long npd_eth_port_delete(void *data)
{
	int ret = 0;
	struct eth_port_s *del_port = (struct eth_port_s *)data;
	
    /*insert eth to hardware, for special handle*/
	ret = nam_eth_port_remove(del_port->eth_port_ifindex);

	return ret;
}

extern int npd_fdb_entry_del_by_port( int ifIndex );

int npd_eth_port_update_admin(
     sequence_table_index_t *index,
     void* data,
     unsigned int flag
     )
{
    unsigned char admin_status = 0;
    struct eth_port_s *new_port = (struct eth_port_s*)data;

    admin_status = (new_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;

    /*\B3\F5?\BB\AF|?\BD\E1\CA\F8\A3\AC\B2\BBenable\B6?\DA*/
    if((admin_status == ETH_ATTR_ENABLE) && (!npd_startup_end))
        ;
    else
        npd_set_port_admin_status(new_port->eth_port_ifindex, admin_status);
	return NPD_SUCCESS;
}
     
int npd_eth_port_startup_end_update()
{
	/*\D3\C3traversal\BB\E1\B7\E2\CB\F8\CA\FD\BE?\E2\BD?\A4?\BC?\D4\DAQUALCOMM??\C9\CF\D3°Ï®¢\C9\C4?\BC\D6\C2\D6\D8\C6\F4*/
	int ret;
	struct eth_port_s eth_port = {0};

	ret = dbtable_sequence_traverse_next(g_eth_ports, -1, &eth_port);
	while(0 == ret)
	{
		int local_ret;
        local_ret = eth_port_local_check(eth_port.eth_port_ifindex);
		if(local_ret == ETHPORT_RETURN_CODE_ERR_NONE)
		{
    		npd_eth_port_update_admin(g_eth_ports, &eth_port, 0);
    		/*?10ms\A3\AC\D2??\C0\ED\CD\EA\B6?\DAup\D6°Ï?\CF*/
    		usleep(10000);
		}
		ret = dbtable_sequence_traverse_next(g_eth_ports, eth_port.eth_port_ifindex, &eth_port);
	}
	/*
    return dbtable_sequence_traversal(g_eth_ports, 0, NULL, NULL, npd_eth_port_update_admin);
    */
    return 0;
}

long npd_eth_port_update(void *newdata, void *olddata)
{
    struct eth_port_s *new_port = (struct eth_port_s*)newdata;
    struct eth_port_s *old_port = (struct eth_port_s*)olddata;
    int link_status;
    int retval = NPD_SUCCESS, ret = NPD_SUCCESS;
	unsigned char tmp_devNum, tmp_portNum; 
		
    /*notify eth port link change, not only handle local ports*/
	if((new_port->attr_bitmap & ETH_ATTR_LINK_STATUS) != (old_port->attr_bitmap & ETH_ATTR_LINK_STATUS))
	{
        if (ETH_ATTR_LINKDOWN == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
        {
			ret = nam_set_ethport_linkstate(new_port->eth_port_ifindex, ETH_ATTR_LINKDOWN);
			if(new_port->trunkid != -1)
			{
				ret = nam_asic_trunk_ports_hwdel(new_port->eth_port_ifindex, new_port->trunkid);
			}
        }
		else if (ETH_ATTR_LINKUP == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
		{
			ret = nam_set_ethport_linkstate(new_port->eth_port_ifindex, ETH_ATTR_LINKUP);
			if(new_port->trunkid != -1)
			{
				ret = nam_asic_trunk_ports_hwadd(new_port->eth_port_ifindex, new_port->trunkid, TRUE);
			}
		}
	}

	if(new_port->forward_mode != old_port->forward_mode )
	{	
		ret = nam_eth_port_forward_mode_set(new_port->eth_port_ifindex, new_port->forward_mode);
	}
		
	if (new_port->mtu != old_port->mtu)
   {
	   ret = npd_set_port_mru(new_port->eth_port_ifindex, new_port->mtu);

	   if (NPD_SUCCESS != ret)
		   retval = NPD_FAIL;

	   if (NPD_SUCCESS != ret)
	   {
		   npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
						  __FILE__, __LINE__, "npd_set_port_mru");
	   }
   }
	/*if the port is not in local board, than only return success*/

	ret = npd_get_devport_by_global_index(new_port->eth_port_ifindex, &tmp_devNum, &tmp_portNum);

    if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
        return NPD_SUCCESS;

    if (new_port->attr_bitmap != old_port->attr_bitmap)
    {
        unsigned char an_state = 0,an_fc = 0,fc = 0,an_duplex = 0,duplex = 0,an_speed = 0;
        unsigned char admin_status = 0;
        PORT_SPEED_ENT speedMode = PORT_SPEED_10_E, speed = 10;
		
        an_state = (new_port->attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;

        if ((new_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS) !=
                (old_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS))
        {
            admin_status = (new_port->attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
            if((admin_status == ETH_ATTR_ENABLE) && (!npd_startup_end))
                ret = npd_set_port_admin_status(new_port->eth_port_ifindex, 0);
            else
                ret = npd_set_port_admin_status(new_port->eth_port_ifindex, admin_status);

			if (ETHPORT_RETURN_CODE_ERR_NONE == eth_port_sfp_type_check(new_port))
			{
				ret = npd_set_port_sfp_laser(new_port->eth_port_ifindex, admin_status);
			}
			
            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_admin_status");
            }
        }
#if 0
        if ((new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) !=
                (old_port->attr_bitmap &ETH_ATTR_BACKPRESSURE))
        {
            bp = (new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
            ret = npd_set_port_backPressure_state(new_port->eth_port_ifindex, bp);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_backPressure_state");
            }
        }
#endif
		if ((new_port->attr_bitmap & (ETH_ATTR_PREFERRED_COPPER_MEDIA|ETH_ATTR_PREFERRED_FIBER_MEDIA))
                != (old_port->attr_bitmap & (ETH_ATTR_PREFERRED_COPPER_MEDIA|ETH_ATTR_PREFERRED_FIBER_MEDIA)))
        {
            unsigned int media;
            media = (new_port->attr_bitmap & (ETH_ATTR_PREFERRED_COPPER_MEDIA|ETH_ATTR_PREFERRED_FIBER_MEDIA))>> ETH_PREFERRED_COPPER_MEDIA_BIT;
            ret = npd_set_port_media_type(new_port->eth_port_ifindex, media);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_media_type");
            }
        }
        if ((new_port->attr_bitmap & ETH_ATTR_AUTONEG) !=
                (old_port->attr_bitmap & ETH_ATTR_AUTONEG))
        {
            ret = npd_set_port_autoNego_status(new_port->eth_port_ifindex, an_state);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_autoNego_status");
            }
        }

        if ((new_port->attr_bitmap & ETH_ATTR_AUTONEG_SPEED) !=
                (old_port->attr_bitmap & ETH_ATTR_AUTONEG_SPEED))
        {
            an_speed = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT;
            ret = npd_set_port_autoNego_speed(new_port->eth_port_ifindex, an_speed);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_autoNego_speed");
            }
        }

        if ((new_port->attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX)
                != (old_port->attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX))
        {
            an_duplex = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT;
            ret = npd_set_port_autoNego_duplex(new_port->eth_port_ifindex, an_duplex);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_autoNego_duplex");
            }
        }

        if ((new_port->attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL)
                != (old_port->attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL))
        {
            an_fc = (new_port->attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT;
            ret = npd_set_port_autoNego_flowctrl(new_port->eth_port_ifindex, an_fc);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_autoNego_flowctrl");
            }
        }

        if ((new_port->attr_bitmap & ETH_ATTR_FLOWCTRL)      //add
                != (old_port->attr_bitmap & ETH_ATTR_FLOWCTRL))
        {
            fc = (new_port->attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
            ret = npd_set_port_flowCtrl_state(new_port->eth_port_ifindex, fc);

            if (NPD_SUCCESS != ret)
                retval = NPD_FAIL;

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_flowCtrl_state");
            }
        }

        if (an_state != ETH_ATTR_AUTONEG_DONE)
        {
            if ((new_port->attr_bitmap & ETH_ATTR_DUPLEX)
                    != (old_port->attr_bitmap & ETH_ATTR_DUPLEX))
            {
                duplex = (new_port->attr_bitmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;
                ret = npd_set_port_duplex_mode(new_port->eth_port_ifindex, duplex);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_duplex_mode");
                }
            }

            if ((new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) !=
                    (old_port->attr_bitmap & ETH_ATTR_SPEED_MASK))
            {
                speedMode = (new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

                if (speedMode == PORT_SPEED_10_E)
                    speed = 10;
                else if (speedMode == PORT_SPEED_100_E)
                    speed = 100;
                else if (speedMode == PORT_SPEED_1000_E)
                {
                    speed = 1000;
                }
                else if (speedMode == PORT_SPEED_10000_E)
                    speed = 10000;
                else if (speedMode == PORT_SPEED_40G_E)
                    speed = 40000;
                else if (speedMode == PORT_SPEED_100G_E)
                    speed = 100000;

                ret = npd_set_port_speed(new_port->eth_port_ifindex, speed);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_speed");
                }
            }
            if ((new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) !=           //add
                    (old_port->attr_bitmap &ETH_ATTR_BACKPRESSURE))
            {
                unsigned int bp = (new_port->attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
                ret = npd_set_port_backPressure_state(new_port->eth_port_ifindex, bp);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_backPressure_state");
                }
            }
            if ((new_port->attr_bitmap & ETH_ATTR_FLOWCTRL)
                    != (old_port->attr_bitmap & ETH_ATTR_FLOWCTRL))
            {
                fc = (new_port->attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
                ret = npd_set_port_flowCtrl_state(new_port->eth_port_ifindex, fc);

                if (NPD_SUCCESS != ret)
                    retval = NPD_FAIL;

                if (NPD_SUCCESS != ret)
                {
                    npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                   __FILE__, __LINE__, "npd_set_port_flowCtrl_state");
                }
            }
        }
		else
		{
            if ((new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) !=
                    (old_port->attr_bitmap & ETH_ATTR_SPEED_MASK))
            {
                speedMode = (new_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

                if (speedMode == PORT_SPEED_1000_E)
                {
                    speed = 1000;
                }
                else if (speedMode == PORT_SPEED_10000_E)
                {
                    speed = 10000;
                }
                else if (speedMode == PORT_SPEED_40G_E)
                {
                    speed = 40000;
                }
                else if (speedMode == PORT_SPEED_100G_E)
                {
                    speed = 100000;
                }
                if(speed >= 1000)
                {
                    ret = npd_set_port_speed(new_port->eth_port_ifindex, speed);
    
                    if (NPD_SUCCESS != ret)
                        retval = NPD_FAIL;
    
                    if (NPD_SUCCESS != ret)
                    {
                        npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                       __FILE__, __LINE__, "npd_set_port_speed");
                    }
                }
            }
		}
        ret = npd_get_port_link_status(new_port->eth_port_ifindex, &link_status);

		/* Added by wangquan */
		ret = eth_port_local_and_master_check(new_port->eth_port_ifindex);
		
        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_get_port_link_status");
        }
        else
        {
            if (link_status != (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
            {
#ifdef HAVE_CHASSIS_SUPPORT	
                if (!SYS_LOCAL_MODULE_ISMASTERACTIVE && npd_startup_end)
                {
                    if (link_status == ETH_ATTR_LINKDOWN)
                        netif_remote_notify_event(SYS_MASTER_ACTIVE_SLOT_INDEX,
                                                  new_port->eth_port_ifindex, PORT_NOTIFIER_LINKDOWN_E, NULL, 0);
    
                    if (link_status == ETH_ATTR_LINKUP)
                        netif_remote_notify_event(SYS_MASTER_ACTIVE_SLOT_INDEX,
                                                  new_port->eth_port_ifindex, PORT_NOTIFIER_LINKUP_E, NULL, 0);
                }
#endif				
            }
        }
		if((new_port->attr_bitmap & ETH_ATTR_LINK_STATUS) != (old_port->attr_bitmap & ETH_ATTR_LINK_STATUS))
		{
            if (ETH_ATTR_LINKDOWN == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
            {
                port_driver_t *driver;
                driver = port_driver_get(new_port->eth_port_ifindex);
                if(driver->fdb_delete_by_port)
                ret = (*driver->fdb_delete_by_port)(new_port->eth_port_ifindex);
            }
			else if (ETH_ATTR_LINKUP == (new_port->attr_bitmap & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)
			{
				if (new_port->port_type == ETH_GE_COMBO)
				{
					ret = SYS_LOCAL_MODULE_LED_LIGHT(new_port->eth_port_ifindex);
				}
				
			}
		}
    }

    if (new_port->bandwidth[0] != old_port->bandwidth[0])
    {
        npd_set_port_bandwidth(new_port->eth_port_ifindex, 0,
            new_port->bandwidth[0], new_port->burstsize[0]);
            
    }
    if (new_port->bandwidth[1] != old_port->bandwidth[1])
    {
        npd_set_port_bandwidth(new_port->eth_port_ifindex, 1,
            new_port->bandwidth[1], new_port->burstsize[1]);
    }
	
	if(new_port->ip_sg != old_port->ip_sg)
    {
        ret = nam_ethport_set_ipsg(new_port->eth_port_ifindex, new_port->ip_sg);
    }
    

    if (new_port->ipg != old_port->ipg)
    {
        ret = npd_set_eth_port_ipg(new_port->eth_port_ifindex, new_port->port_type, new_port->ipg);

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_set_eth_port_ipg");
        }
    }

    if (0 != memcmp(&(new_port->sc), &(old_port->sc), sizeof(new_port->sc)))
    {
        unsigned int portType;
        unsigned int portflg;
        portType = new_port->port_type;
        portflg = ((ETH_FE_TX == portType)||(ETH_FE_FIBER == portType))? 0:1;

        if ((new_port->sc.dlf.bpsValid)||(new_port->sc.dlf.bpsValid != old_port->sc.dlf.bpsValid))
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_DLF,
                                          new_port->sc.dlf.value.bps,portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_bps_set");
            }
        }
        else if ((new_port->sc.dlf.ppsValid)||(new_port->sc.dlf.ppsValid != old_port->sc.dlf.ppsValid))
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_DLF,
                                          new_port->sc.dlf.value.pps,portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_pps_set");
            }
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if ((new_port->sc.bcast.bpsValid)||(new_port->sc.bcast.bpsValid != old_port->sc.bcast.bpsValid))
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_BCAST,
                                          new_port->sc.bcast.value.bps,portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_bps_set");
            }
        }
        else if ((new_port->sc.bcast.ppsValid)||(new_port->sc.bcast.ppsValid != old_port->sc.bcast.ppsValid))
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_BCAST,
                                          new_port->sc.bcast.value.pps,portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_pps_set");
            }
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;

        if ((new_port->sc.mcast.bpsValid)||(new_port->sc.mcast.bpsValid != old_port->sc.mcast.bpsValid))
        {
            ret = nam_eth_port_sc_bps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_MCAST,
                                          new_port->sc.mcast.value.bps,portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_bps_set");
            }
        }
        else if ((new_port->sc.mcast.ppsValid)||(new_port->sc.mcast.ppsValid != old_port->sc.mcast.ppsValid))
        {
            ret = nam_eth_port_sc_pps_set(new_port->eth_port_ifindex,
                                          PORT_STORM_CONTROL_STREAM_MCAST,
                                          new_port->sc.mcast.value.pps, portflg);

            if (NPD_SUCCESS != ret)
            {
                npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "nam_eth_port_sc_pps_set");
            }
        }

        if (NPD_SUCCESS != ret)
            retval = NPD_FAIL;
    }
    /* Modified by liuxi 20120917 */
    if (new_port->vct_isable != old_port->vct_isable)
    {
        unsigned int state =new_port->vct_isable;
        syslog_ax_eth_port_dbg("Set gindex 0x%x vct set %d\n",new_port->eth_port_ifindex, state);
		/*if state == 1 enable the vct port*/
        ret = npd_vct_set(new_port->eth_port_ifindex, state);
        
        if (NPD_SUCCESS != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                           __FILE__, __LINE__, "npd_vct_set");
			retval = NPD_FAIL;
        }
    }

    if(new_port->eee != old_port->eee)
    {
        unsigned int value = new_port->eee;
        unsigned int eth_g_index = new_port->eth_port_ifindex;
        
    	ret = npd_set_port_eee(eth_g_index, value);
        if(ret != NPD_SUCCESS)
        {
           npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                               __FILE__, __LINE__, "npd_set_port_eee");
        }
    }

	if(new_port->loopback != old_port->loopback)
	{
		ret = nam_set_port_loopback(new_port->eth_port_ifindex, new_port->loopback);
	}
    return retval;
}

eth_port_stats_t *npd_get_port_counter_by_index
(
    unsigned int eth_g_index
)
{
    eth_port_stats_t *port_counter_info = NULL;
    unsigned int sub_slot = 0;
    unsigned int portno = 0, sub_port = 0;
	unsigned int slot_index = 0, sub_slot_index = 0;
	int module_type;
	unsigned char tmp_devNum, tmp_portNum; 


	if (npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum)
		== ETHPORT_RETURN_CODE_ERR_NONE)
    {
    	slot_index 		= SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
		sub_slot_index = SUBSLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
		if(sub_slot_index)
		{
			module_type = SYS_LOCAL_MODULE_TYPE;
		}
		else
		{
		    module_type 	= MODULE_TYPE_ON_SLOT_INDEX(slot_index);
		}
		if (!SYS_MODULE_ISHAVEPP(module_type))
		{
			sub_slot = npd_netif_eth_get_subslot(eth_g_index);
			portno = tmp_portNum + 24;
			port_counter_info = &g_eth_port_counter[sub_slot][portno];
		}
		else
		{
	        sub_slot = npd_netif_eth_get_subslot(eth_g_index);
	        portno = npd_netif_eth_get_port(eth_g_index);
			sub_port = npd_netif_eth_get_sub_port(eth_g_index);
	        port_counter_info = &g_eth_port_counter[sub_slot][portno*MAX_SUBPORT_PER_ETHPORT + sub_port];
		}
    }

    return port_counter_info;
}

struct eth_port_counter_s *npd_get_stack_port_counter_by_slotno_portno
(
    unsigned int slot_no,
    unsigned int port_no
)
{
    struct eth_port_counter_s *port_counter_info = NULL;
	unsigned int slot_index;
	unsigned int port_index;

	if (SYS_LOCAL_MODULE_SLOT_INDEX == CHASSIS_SLOT_NO2INDEX(slot_no))
	{
		slot_index = slot_no -1;
		port_index = port_no -1;
		port_counter_info = &g_stack_port_counter[0][port_index];
	}

    return port_counter_info;
}

int npd_stack_port_counter_statistics
(
	unsigned int slot_no, 
	unsigned int port_no,
	struct eth_port_counter_s *portPtr
)
{
	struct eth_port_counter_s *port_counter_info = NULL;
    port_counter_info = npd_get_stack_port_counter_by_slotno_portno(slot_no, port_no);
	if (port_counter_info)
	{
		port_counter_info->rx.goodframes += portPtr->rx.goodframes;
        
        port_counter_info->rx.totalbytes += portPtr->rx.totalbytes;
      
        port_counter_info->rx.uncastframes += portPtr->rx.uncastframes;

        port_counter_info->rx.mcastframes += portPtr->rx.mcastframes;
        
        port_counter_info->rx.bcastframes += portPtr->rx.bcastframes;
        
        port_counter_info->rx.errorframes += portPtr->rx.errorframes; 
        
        port_counter_info->rx.discardframes += portPtr->rx.discardframes;
     
        port_counter_info->tx.goodframe += portPtr->tx.goodframe;
       
        port_counter_info->tx.totalbyte += portPtr->tx.totalbyte;
        
        port_counter_info->tx.uncastframe += portPtr->tx.uncastframe;
        
        port_counter_info->tx.mcastframe += portPtr->tx.mcastframe;
        
        port_counter_info->tx.bcastframe += portPtr->tx.bcastframe;
        
        port_counter_info->tx.errorframe += portPtr->tx.errorframe ; 
        
        port_counter_info->tx.discardframe += portPtr->tx.discardframe;
     
        return NPD_SUCCESS;
	}
	else
    {
        return NPD_FAIL;
    }
}

/**********************************************************************************
 *  npd_eth_port_counter_statistics
 *
 *	DESCRIPTION:
 * 		statistics packets info
 *
 *	INPUT:
 *		eth_g_index - port_index
 *		portPtr - pakets info
 *
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCESS
 *
 *
 **********************************************************************************/
int npd_eth_port_counter_statistics
(
    unsigned int eth_g_index,
    eth_port_stats_t *portPtr
)
{
    eth_port_stats_t *port_counter_info = NULL;
	int i;
    port_counter_info = npd_get_port_counter_by_index(eth_g_index);

	if(port_counter_info == NULL)
		return 0;

	for(i = 0; i < 64; i++)
	{
		port_counter_info->values[i] += portPtr->values[i];
	}

    return NPD_SUCCESS;
}


/**********************************************************************************
 *  npd_get_port_admin_status
 *
 *	DESCRIPTION:
 * 		get ethernet port admin status:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		status - admin status
 *
 * 	RETURN:
 *		0:OK
 *		else: fail
 *
 **********************************************************************************/
int npd_get_port_admin_status
(
    unsigned int eth_g_index,
    unsigned int *status
)
{
    unsigned long attr = 0;
    int ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    ret = nam_get_port_en_dis(eth_g_index,&attr);

    if (ret != 0)
    {
        return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    if (ETH_ATTR_ENABLE == attr)
    {
        *status = ETH_ATTR_ENABLE;
    }
    else if (ETH_ATTR_DISABLE == attr)
    {
        *status = ETH_ATTR_DISABLE;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_admin_status
 *
 *	DESCRIPTION:
 * 		set ethernet port admin status:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -admin status enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 *	RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 *
 **********************************************************************************/
int npd_set_port_admin_status
(
    IN unsigned int eth_g_index,
    IN unsigned int status
)
{
    unsigned long attr = status;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    syslog_ax_eth_port_dbg("Set gindex 0x%x states %d\n",eth_g_index,status);

    ret = eth_port_local_check(eth_g_index);
    if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
    {
        if(!(SYS_LOCAL_MODULE_ISMASTERACTIVE))
        {
            attr = 0;
        }
    }
	if(ETH_ATTR_DISABLE == attr)
	{
		/*\B9?\D5\C1\B4?\B2\BB\CE?\A8\B5?\EC\B2\E2\D7?\AFshutdown\B6??\FA\D6\C6*/
		int array_id = netif_array_index_from_ifindex(eth_g_index);
        npd_ethport_change_fast_detect[array_id].auto_shutdown = 0;		
	}
	
    ret = nam_set_ethport_enable(eth_g_index,attr);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_HW;
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
    }
    return ret;
}

/**********************************************************************************
 *  npd_set_port_sfp_laser
 *
 *	DESCRIPTION:
 * 		set ethernet port sfp-laser status:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -sfp status enable or disable
 *
 *	RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 *
 **********************************************************************************/

int npd_set_port_sfp_laser
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned char portNum = 0;
	unsigned char tmp_devNum, tmp_portNum; 
	

	ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);


    if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
        return NPD_SUCCESS;
    portNum = npd_netif_eth_get_port(eth_g_index);
	syslog_ax_eth_port_dbg("set sfp port %d.\n", portNum);
	
	ret = nbm_sfp_light_set(portNum, attr);
	if (ret != NPD_SUCCESS)
	{
		ret = ETHPORT_RETURN_CODE_ERR_HW;
	}
	else
	{
		ret = ETHPORT_RETURN_CODE_ERR_NONE;
	}

	return ret;
}

/**********************************************************************************
 *  npd_get_port_link_status
 *
 *	DESCRIPTION:
 * 		get ethernet port link status:link up or link down
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		status - admin status
 *
 * 	RETURN:
 *		NULL
 *
 *
 **********************************************************************************/
int npd_get_port_link_status
(
    IN unsigned int eth_g_index,
    OUT int *status
)
{
    unsigned long attr = 0;
    unsigned int ret = NPD_SUCCESS;
    ret = nam_get_port_link_state(eth_g_index,&attr);

    if (NPD_SUCCESS != ret)
    {
        return ret;
    }

    if (ETH_ATTR_LINKUP == attr)
    {
        *status = ETH_ATTR_LINKUP;
    }
    else if (ETH_ATTR_LINKDOWN == attr)
    {
        *status = ETH_ATTR_LINKDOWN;
    }

    return ret;
}
/**********************************************************************************
 *  npd_set_port_link_status
 *
 *	DESCRIPTION:
 * 		set ethernet port link status:link up or link down
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -linkstatus enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		ETHPORT_RETURN_CODE_ERR_GENERAL
 *		ETHPORT_RETURN_CODE_ERR_NONE
 *      ETHPORT_RETURN_CODE_ERR_HW
 *		ETHPORT_RETURN_CODE_UNSUPPORT
 *
 **********************************************************************************/
int npd_set_port_link_status
(
    IN unsigned int eth_g_index,
    IN unsigned int status
)
{
    unsigned long attr = status;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
	
    if (1 == attr)
    {
        ret = nam_set_ethport_force_linkup(eth_g_index,1);

        if (0 != ret)
        {
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else if (0 == attr)
    {
        ret = nam_set_ethport_force_linkdown(eth_g_index,1);

        if (0 != ret)
        {
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else if (2 == attr)
    {
        ret = nam_set_ethport_force_auto(eth_g_index,0);

        if (0 != ret)
        {
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    return ret;
}
/**********************************************************************************
 *  npd_get_port_autoNego_status
 *
 *	DESCRIPTION:
 * 		get ethernet port auto-negotiation status:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		status - auto-negotiation status
 *
 * 	RETURN:
 *		0: OK
 *
 *
 **********************************************************************************/
int npd_get_port_autoNego_status
(
    unsigned int eth_g_index,
    unsigned int *status
)
{
    unsigned long attr = 0;
    int ret = -1;
    ret = nam_get_port_autoneg_state(eth_g_index,&attr);

    if (ret != 0)
    {
        return ret;
    }

    if (ETH_ATTR_AUTONEG_DONE == attr)
    {
        *status = ETH_ATTR_AUTONEG_DONE;
    }
    else if (ETH_ATTR_AUTONEG_NOT_DONE== attr)
    {
        *status = ETH_ATTR_AUTONEG_NOT_DONE;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_autoNego_status
 *
 *	DESCRIPTION:
 * 		set ethernet port auto-negotiation status:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -autoNego status enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FALI
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_autoNego_status
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    unsigned long portState = 0;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    enum module_driver_type_e module_driver_type;
	int adminStatus = 0;
    module_driver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);
    syslog_ax_eth_port_dbg("Set gindex 0x%x autoNeg states %d\n",eth_g_index,status);

    switch (module_driver_type)
    {
        case MODULE_DRIVER_NAM_BCM:
        case MODULE_DRIVER_NAM_CPSS:
        case MODULE_DRIVER_NAM_CTC:
		case MODULE_DRIVER_NAM_ATHEROS:
			adminStatus = eth_port_sw_admin_get(eth_g_index);
            if (NPD_FAIL == adminStatus)
                return ETHPORT_RETURN_CODE_ERR_GENERAL;
            if (ETH_ATTR_ENABLE == adminStatus)
            {
            	ret = nam_get_port_en_dis(eth_g_index, &portState);
				if (ETH_ATTR_ENABLE == portState)
				{
					ret |= nam_set_ethport_enable(eth_g_index,0);
				}
                
                ret |= nam_set_ethport_autoneg_state(eth_g_index,attr);
				if ((ETH_ATTR_ENABLE == portState) && npd_startup_end)
				{
					ret |= nam_set_ethport_enable(eth_g_index,1);
				}
                

                if (0 != ret)
                {
                    syslog_ax_eth_port_err("set port autoNego status failed in 3052\n");
                    return ETHPORT_RETURN_CODE_ERR_HW;
                }
                else
                {
                    ret = ETHPORT_RETURN_CODE_ERR_NONE;
                }
            }
            else
            {
                ret |= nam_set_ethport_autoneg_state(eth_g_index,attr);
				if (0 != ret)
                {
                    syslog_ax_eth_port_err("set port autoNego status failed in 3052\n");
                    return ETHPORT_RETURN_CODE_ERR_HW;
                }
                else
                {
                    ret = ETHPORT_RETURN_CODE_ERR_NONE;
                }
                //return ETHPORT_RETURN_CODE_ERR_OPERATE;
            }

            break;
        default:
            return ETHPORT_RETURN_CODE_NOT_SUPPORT;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_autoNego_duplex
 *
 *	DESCRIPTION:
 * 		set ethernet port auto-negotiation duplex:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -autoNego duplex enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FALI
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_autoNego_duplex
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    unsigned long portState = 0;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned int speed = 0;
	int adminStatus;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x autoNeg duplex states %d\n",eth_g_index,status);

    if ((ret = nam_get_port_speed(eth_g_index,&speed)) != 0)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    adminStatus = eth_port_sw_admin_get(eth_g_index);
    if (NPD_FAIL == adminStatus)
            return ETHPORT_RETURN_CODE_ERR_GENERAL;		

    
    if (ETH_ATTR_ENABLE == adminStatus)
    {
			
        ret = nam_set_ethport_force_linkdown(eth_g_index, 1);
        ret = nam_set_ethport_duplex_autoneg(eth_g_index,attr);

        if (ETHPORT_RETURN_CODE_NOT_SUPPORT == ret)
        {
            return ret;
        }

        ret = nam_set_ethport_force_auto(eth_g_index, 0);

        if (NPD_FAIL == ret)
        {
            syslog_ax_eth_port_err("set port duplex autoNego status failed\n");
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else if (ETH_ATTR_DISABLE== portState)
    {
        return ETHPORT_RETURN_CODE_ERR_OPERATE;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_autoNego_speed
 *
 *	DESCRIPTION:
 * 		set ethernet port auto-negotiation speed:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -autoNego speed enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FALI
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_autoNego_speed
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    unsigned long portState = 0;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
	int adminStatus;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x autoNeg speed states %d\n",eth_g_index,status);

    adminStatus = eth_port_sw_admin_get(eth_g_index);
    if (NPD_FAIL == adminStatus)
        return ETHPORT_RETURN_CODE_ERR_GENERAL;
            
    if (ETH_ATTR_ENABLE == adminStatus)
    {
        ret = nam_set_ethport_force_linkdown(eth_g_index,1);
        ret = nam_set_ethport_speed_autoneg(eth_g_index,attr);

        if (ETHPORT_RETURN_CODE_NOT_SUPPORT == ret)
        {
            return ret;
        }

        //ret = nam_set_phy_ethport_autoneg(eth_g_index,attr);
        ret = nam_set_ethport_force_auto(eth_g_index,0);

        if (NPD_FAIL == ret)
        {
            syslog_ax_eth_port_err("set port speed autoNego status failed\n");
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else if (ETH_ATTR_DISABLE== portState)
    {
        return ETHPORT_RETURN_CODE_ERR_OPERATE;
    }
    return ret;
}

/**********************************************************************************
 *  npd_set_port_autoNego_flowctrl
 *
 *	DESCRIPTION:
 * 		set ethernet port auto-negotiation flowcontrol:enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status -autoNego speed enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FALI
 *		NPD_SUCCESS
 *
 **********************************************************************************/

int npd_set_port_autoNego_flowctrl
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    int ret = 0;
#if 0    
    unsigned long attr = status;
    unsigned long portState = 0;
    unsigned int pauseAdvertise = 1;
	int adminStatus;
    syslog_ax_eth_port_dbg("Set gindex 0x%x autoNeg fc states %d\n",eth_g_index,status);

    nam_get_port_en_dis(eth_g_index,&portState);
    adminStatus = eth_port_sw_admin_get(eth_g_index);
    if (NPD_FAIL == adminStatus)
        return ETHPORT_RETURN_CODE_ERR_GENERAL;
            
    if (ETH_ATTR_ENABLE == adminStatus)
    {
        ret = nam_set_ethport_force_linkdown(eth_g_index,1);
        pauseAdvertise = attr ? 1 : 0;
        ret = nam_set_ethport_fc_autoneg(eth_g_index,attr,pauseAdvertise);

        if (ETHPORT_RETURN_CODE_NOT_SUPPORT == ret)
        {
            return ret;
        }

        ret = nam_set_ethport_force_auto(eth_g_index,0);

        if (NPD_FAIL == ret)
        {
            syslog_ax_eth_port_err("set port fc autoNego status failed\n");
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }

        return ret;
    }
    else if (ETH_ATTR_DISABLE == portState)
    {
        return ETHPORT_RETURN_CODE_ERR_OPERATE;
    }
#endif	
    return ret;
}

/**********************************************************************************
 *  npd_get_port_duplex_mode
 *
 *	DESCRIPTION:
 * 		get ethernet port duplex status:half or full
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		mode - duplex half or duplex full
 *
 * 	RETURN:
 *		0: OK
 *
 *
 **********************************************************************************/
int npd_get_port_duplex_mode
(
    unsigned int eth_g_index,
    unsigned int *mode
)
{
    unsigned int attr = 0;
    int ret = -1;
    ret = nam_get_port_duplex_mode(eth_g_index,&attr);

    if (ret != 0)
    {
        return ret;
    }

    if (ETH_ATTR_DUPLEX_FULL == attr)
    {
        *mode = ETH_ATTR_DUPLEX_FULL;
    }
    else if (ETH_ATTR_DUPLEX_HALF == attr)
    {
        *mode = ETH_ATTR_DUPLEX_HALF;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_duplex_mode
 *
 *	DESCRIPTION:
 * 		set ethernet port duplex status:half or full
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		mode - full or half
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_duplex_mode
(
    unsigned int eth_g_index,
    unsigned int mode
)
{
    unsigned int attr = mode;
    unsigned int slot_index = 0;
    int ret = 0;
    unsigned long anduplex = 0;
    unsigned int speed = 0;
    unsigned long flowControl = 0, anDone = 0;
	
    slot_index = SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x duplex mode states %d\n",eth_g_index,mode);

    ret = nam_get_ethport_duplex_autoneg(eth_g_index,&anduplex);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    ret = nam_get_port_autoneg_state(eth_g_index,&anDone);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    ret = nam_get_port_speed(eth_g_index,&speed);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    ret = nam_get_port_flowCtrl_state(eth_g_index,&flowControl);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
#if 0
    ret = nam_get_port_backPres_state(eth_g_index,&backpressure);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
#endif
    {
        if (0 == anDone)
        {
            attr = mode ? PORT_HALF_DUPLEX_E : PORT_FULL_DUPLEX_E;
            #if 0
            if ((PORT_HALF_DUPLEX_E == attr) && (ETH_ATTR_FC_ENABLE == flowControl))
            {
                ret = ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF;
                return ret;
            }
            
            if ((PORT_FULL_DUPLEX_E == attr) && (ETH_ATTR_BP_ENABLE == backpressure))
            {
                ret = ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL;
                return ret;
            }
			#endif
            ret = nam_set_ethport_duplex_mode(eth_g_index,attr);

            if (NPD_FAIL == ret)
            {
                ret = ETHPORT_RETURN_CODE_ERR_HW;
                syslog_ax_eth_port_err("set port duplex mode failed \n");
            }
            else
            {
                ret = ETHPORT_RETURN_CODE_ERR_NONE;
            }
        }
        else
        {
            syslog_ax_eth_port_dbg("set port duplex mode %s,anduplex %d ,speed %d\n",attr? "Half":"Full",anduplex,speed);
            ret = ETHPORT_RETURN_CODE_DUPLEX_NODE;
        }
    }
    return ret;
}

/**********************************************************************************
 *  npd_get_port_speed
 *
 *	DESCRIPTION:
 * 		get ethernet port speed :10M ,100M or 1000M etc.
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		speed - port speed 10M or 100M or 1000M
 *
 * 	RETURN:
 *		0:OK
 *
 *
 **********************************************************************************/
int npd_get_port_speed
(
    unsigned int eth_g_index,
    unsigned int *speed
)
{
    unsigned int ret = 0;
    unsigned int attr = 0;
    ret = nam_get_port_speed(eth_g_index,&attr);

    if (ret != 0)
    {
        *speed = 0;
    }
    else
    {
        *speed = attr;
    }

    return ret;
}

int npd_get_port_swspeed
(
    unsigned int eth_g_index,
    int *speed
)
{
    *speed  = eth_port_sw_speed_get(eth_g_index);

    return 0;
}

int npd_get_port_swduplex
(
    unsigned int eth_g_index,
    int *duplex
)
{
    *duplex  = eth_port_sw_duplex_get(eth_g_index);

    return 0;
}

/**********************************************************************************
 *  npd_set_port_speed
 *
 *	DESCRIPTION:
 * 		set ethernet port speed :10M ,100M or 1000M etc.
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		speed 10M,100M,1000M
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_speed
(
    unsigned int eth_g_index,
    unsigned int speed
)
{
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned long anspeed = 0, anDone = 0;
    unsigned int attr = PORT_SPEED_1000_E;
	unsigned int mode;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x port speed %d\n",eth_g_index,speed);

    if (speed == 1000)
        attr = PORT_SPEED_1000_E;
    else if (speed == 100)
        attr = PORT_SPEED_100_E;
    else if (speed ==10)
        attr = PORT_SPEED_10_E;
    else if (speed == 10000)
        attr = PORT_SPEED_10000_E;
    else if (speed == 40000)
        attr = PORT_SPEED_40G_E;
    else if (speed == 100000)
        attr = PORT_SPEED_100G_E;


    ret = nam_get_port_autoneg_state(eth_g_index,&anDone);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    ret = nam_get_ethport_speed_autoneg(eth_g_index,&anspeed);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    ret = nam_get_port_duplex_mode(eth_g_index,&mode);

    if (ret != 0)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    if ((0 == anDone))
    {
        ret = nam_set_ethport_speed(eth_g_index,attr);

        if (NPD_FAIL == ret)
        {
            syslog_ax_eth_port_err("set port speed failed \n");
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else
    {
        if(speed > 1000)
        {
            ret = nam_set_ethport_speed(eth_g_index,attr);
    
            if (NPD_FAIL == ret)
            {
                syslog_ax_eth_port_err("set port speed failed \n");
                ret = ETHPORT_RETURN_CODE_ERR_HW;
            }
            else
            {
                ret = ETHPORT_RETURN_CODE_ERR_NONE;
            }
        }
		else
		{
            syslog_ax_eth_port_dbg("%s ::auto neg speed %d\n",__func__,anspeed);
            ret = ETHPORT_RETURN_CODE_SPEED_NODE;
		}
    }
    return ret;
}

/**********************************************************************************
 *  npd_get_port_flowCtrl_state
 *
 *	DESCRIPTION:
 * 		get ethernet port flow control status: enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		status  - flow control is enable or disable
 *
 * 	RETURN:
 *		0:OK
 *
 *
 **********************************************************************************/
int npd_get_port_flowCtrl_state
(
    unsigned int eth_g_index,
    unsigned int *status
)
{
    unsigned long attr = 0;
    int ret = -1;
    ret = nam_get_port_flowCtrl_state(eth_g_index,&attr);

    if (ret != 0)
    {
        return ret;
    }

    if (ETH_ATTR_FC_ENABLE == attr)
    {
        *status = ETH_ATTR_FC_ENABLE;
    }
    else if (ETH_ATTR_FC_DISABLE == attr)
    {
        *status = ETH_ATTR_FC_DISABLE;
    }

    return ret;
}

/**********************************************************************************
 *  npd_set_port_flowCtrl_state
 *
 *	DESCRIPTION:
 * 		set ethernet port flow control status: enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status - enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_flowCtrl_state
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned long anfc = 0;
    enum module_driver_type_e module_drver_type;
    module_drver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);
    syslog_ax_eth_port_dbg("Set gindex 0x%x flowCtrl states %d\n",eth_g_index,status);


    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }

    if (MODULE_DRIVER_NAM_BCM == module_drver_type || 
		MODULE_DRIVER_NAM_CPSS == module_drver_type|| 
		MODULE_DRIVER_NAM_CTC == module_drver_type || 
		MODULE_DRIVER_NAM_ATHEROS == module_drver_type 
		)
    {
        if (0 == anfc)
        {
            ret = nam_set_ethport_flowCtrl(eth_g_index,attr);
        }
        else
            ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    return ret;
}

/**********************************************************************************
 *  npd_get_port_backPressure_state
 *
 *	DESCRIPTION:
 * 		get ethernet port back pressure status: enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		status  - back pressure is enable or disable
 *
 * 	RETURN:
 *		0:OK
 *
 *
 **********************************************************************************/
int npd_get_port_backPressure_state
(
    unsigned int eth_g_index,
    unsigned int *status
)
{
    unsigned long attr = 0;
    int ret = -1;
    ret = nam_get_port_backPres_state(eth_g_index,&attr);

    if (ret != 0)
    {
        return ret;
    }

    if (ETH_ATTR_BP_ENABLE == attr)
    {
        *status = ETH_ATTR_BP_ENABLE;
    }
    else if (ETH_ATTR_BP_DISABLE == attr)
    {
        *status = ETH_ATTR_BP_DISABLE;
    }

    return ret;
}
/**********************************************************************************
 *  npd_set_port_backPressure_state
 *
 *	DESCRIPTION:
 * 		set ethernet port back pressure status: enable or disable
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		status - enable or disable
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_backPressure_state
(
    unsigned int eth_g_index,
    unsigned int status
)
{
    unsigned long attr = status;
    unsigned long backpressure = 0;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned int duplex = 0;
    unsigned char bp_set =0;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x backPressure states %d\n",eth_g_index,status);

    ret = nam_get_port_backPres_state(eth_g_index,&backpressure);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    else
    {
        if ((backpressure == ETH_ATTR_BP_ENABLE) && (attr == ETH_ATTR_BP_DISABLE))
        {
            bp_set = NPD_TRUE;
        }
    }

    ret = nam_get_port_duplex_mode(eth_g_index,&duplex);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    else if (ETH_ATTR_DUPLEX_HALF == duplex || bp_set)
    {
        bp_set = NPD_FALSE;
        ret = nam_set_ethport_backPres(eth_g_index,attr);

        if (0 != ret)
        {
            ret = ETHPORT_RETURN_CODE_ERR_HW;
        }
        else
        {
            ret = ETHPORT_RETURN_CODE_ERR_NONE;
        }
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_BACKPRE_NODE;
        syslog_ax_eth_port_dbg("%s ::duplex  %d\n",__func__,duplex);
    }
    return ret;
}
/**********************************************************************************
 *  npd_get_port_mru
 *
 *	DESCRIPTION:
 * 		get ethernet port MRU
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *
 *	OUTPUT:
 *		mru  - Maximum Receive Unit value
 *
 * 	RETURN:
 *		0:OK
 *
 *
 **********************************************************************************/
int npd_get_port_mru
(
    unsigned int eth_g_index,
    unsigned int *mru
)
{
    unsigned int ret = 0;
    unsigned int attr = 0;
    ret = nam_get_port_mru(eth_g_index,&attr);

    if (ret != 0)
    {
        *mru = 0;
    }
    else
    {
        *mru = attr;
    }

    return 0;
}

/**********************************************************************************
 *  npd_set_port_mru
 *
 *	DESCRIPTION:
 * 		set ethernet port MRU
 *
 *	INPUT:
 *		eth_g_index - ethernet global index
 *		mru <64-8912>
 *		isBoard - port is on main board or not( 1 - main board port, 0 - slave board port)
 *
 * RETURN:
 *		NPD_FAIL
 *		NPD_SUCCESS
 *
 **********************************************************************************/
int npd_set_port_mru
(
    unsigned int eth_g_index,
    unsigned int mru
)
{
    unsigned int attr = mru;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
	
    syslog_ax_eth_port_dbg("Set gindex 0x%x port mru %d\n",eth_g_index,mru);

    ret = nam_set_ethport_mru(eth_g_index,attr);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_HW;
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
    }
    return ret;
}

int npd_get_eth_port_drv_info(unsigned int eth_g_index, struct eth_port_s *portInfo)
{
    int ret = -1;
    int speed = 0;
    int port_type = 0;
    portInfo->port_type = ETH_PORT_TYPE_FROM_GLOBAL_INDEX(eth_g_index);
    portInfo->attr_bitmap = ETH_PORT_ATTRBITMAP_FROM_GLOBAL_INDEX(eth_g_index);
    portInfo->mtu = ETH_PORT_MTU_FROM_GLOBAL_INDEX(eth_g_index);

    port_type = portInfo->port_type;
    speed = ethport_attr_default(port_type)->speed;
    ret = nam_read_eth_port_info(eth_g_index, speed, portInfo);
    if (ret != 0)
    {
        syslog_ax_eth_port_dbg("Eth port interface 0x%x attr get FAIL. \n", eth_g_index);
        ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
    }

    return ret;
}

int npd_get_eth_port_ipg(unsigned int eth_g_index, int port_type, unsigned int *eth_ipg)
{
    int ret = -1;
    unsigned char ipg;
    int speed;
    enum module_driver_type_e module_driver_type = MODULE_DRIVER_NONE;

	unsigned char tmp_devNum, tmp_portNum; 
	ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);


    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        module_driver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);

        switch (module_driver_type)
        {
            case MODULE_DRIVER_NBM:
                /*for the panel port do not connect to asic*/
                *eth_ipg = 0;
                ret = ETHPORT_RETURN_CODE_BOARD_IPG;
                break;
            case MODULE_DRIVER_NAM_BCM:
            case MODULE_DRIVER_NAM_CPSS:
            case MODULE_DRIVER_NAM_CTC:
			case MODULE_DRIVER_NAM_ATHEROS:
            {
                speed = ethport_attr_default(port_type)->speed;
                ret = nam_get_ethport_ipg(eth_g_index, speed, &ipg);

                if (0 == ret)
                {
                    *eth_ipg = ipg;
                }
            }
            break;
            default:
                ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                break;
        }
    }

    return ret;
}
int npd_get_eth_port_rate(unsigned int eth_g_index, unsigned int * inbandwidth,unsigned int *outbandwidth)
{
    int i = 0;
	int ret = 0;
	
	for (i = 0; i < MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT; i++)
    {
        if (eth_g_index == g_eth_port_rate_poll[i])
        {
            *inbandwidth = g_eth_port_rateInput[i];
			*outbandwidth = g_eth_port_rateOutput[i];
			return NPD_SUCCESS;
        }
        
    }

	ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    return ret;
}
int npd_get_eth_port_stat(unsigned int eth_g_index, eth_port_stats_t *ptr)
{
    unsigned char portNum = 0, devNum = 0;
    int ret = 0;
    enum module_driver_type_e module_driver_type = MODULE_DRIVER_NONE;
	unsigned char tmp_devNum, tmp_portNum; 
	ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);


    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        {
            module_driver_type = MODULE_DRIVER_TYPE_BY_IFINDEX(eth_g_index);

            switch (module_driver_type)
            {
                case MODULE_DRIVER_NAM_BCM:
                case MODULE_DRIVER_NAM_CPSS:
                case MODULE_DRIVER_NAM_CTC:
			    case MODULE_DRIVER_NAM_ATHEROS:
                    ret = nam_asic_port_pkt_statistic(eth_g_index,ptr);

                    break;
                default:
                    ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
                    break;
            }

            if (0 != ret)
            {
                syslog_ax_eth_port_dbg("Error:port_index 0x%x,devNum %d,portNum %d,ret %d\n",eth_g_index,devNum,portNum,ret);
                ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
            }
            else
            {
                ret = npd_eth_port_counter_statistics(eth_g_index, ptr);

                if (0 != ret)
                {
                    syslog_ax_eth_port_dbg("save counter info error");
                    ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
                }
                else
                {
                    //clear counter
                    ret = nam_asic_clear_port_pkt_stat(eth_g_index);
                    if (0 != ret)
                    {
                        syslog_ax_eth_port_dbg("Failed to clear statistics of port 0x%x, return code %d\n",eth_g_index,ret);
                        ret = ETHPORT_RETURN_CODE_ERR_HW;
                    }
                    ret = ETHPORT_RETURN_CODE_ERR_NONE;
                }
            }
        }
    }

    return ret;
}

int npd_stack_port_counter_statistics
(
	unsigned int slot_no, 
	unsigned int port_no,
	struct eth_port_counter_s *portPtr
);
int npd_stack_port_counter_statistics
(
	unsigned int slot_no, 
	unsigned int port_no,
	struct eth_port_counter_s *portPtr
);

int npd_get_stack_port_stat(
	unsigned int slotno,
	unsigned int portno,
	struct eth_port_counter_s *ptr)
{
	unsigned int devNum;
	unsigned int devPort;
	int module_type;
	int ret;	

	if (SYS_LOCAL_MODULE_SLOT_INDEX != CHASSIS_SLOT_NO2INDEX(slotno))
	{
		return  ETHPORT_RETURN_CODE_NO_SUCH_PORT;
	}
	module_type = MODULE_TYPE_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slotno));
	if (module_type == 0)
	{
		return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
	}
	devNum = PPAL_PLANE_2_UNIT(module_type, portno-1);
	devPort = PPAL_PLANE_2_PORT(module_type, portno-1);
	
	ret = nam_asic_stack_port_pkt_statistic(devNum, devPort, ptr);
	if (0 != ret)
	{
		syslog_ax_eth_port_dbg("Error:devNum %d,portNum %d,ret %d.\n",devNum,devPort,ret);
		ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
	}
	else
	{
		syslog_ax_eth_port_dbg("save counter info\n");
		ret = npd_stack_port_counter_statistics(slotno, portno, ptr);		
		if (0 != ret)
		{
			syslog_ax_eth_port_dbg("save counter info error");
			ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		}
		else
		{
			//clear counter
			ret = nam_asic_clear_stack_port_pkt_stat(devNum, devPort);
			if (0 != ret)
			{
				syslog_ax_eth_port_dbg("slotno %d, portno %d, ret %d\n", slotno, portno, ret);
				syslog_ax_eth_port_dbg("nam_asic_port_pkt_statistic ERROR\n");
				ret = ETHPORT_RETURN_CODE_ERR_HW;
			}
			ret = ETHPORT_RETURN_CODE_ERR_NONE;
		}
	}	
	return ret;
}




/********************************************************************************
 *  npd_check_eth_port_status
 *
 *	DESCRIPTION:
 * 		check ethernet port link stauts
 *
 *	INPUT:
 *		eth_g_index - ethernet port global index
 *
 *	OUTPUT:
 *
 *
 * 	RETURN:
 *		NPD_FAIL - if some errors occur
 *		ETH_ATTR_LINKUP
 *		ETH_ATTR_LINKDOWN
 *
 *
 **********************************************************************************/
int npd_check_eth_port_status
(
    unsigned int eth_g_index
)
{
    struct eth_port_s* portInfo = NULL;
    int ret;
    portInfo = npd_get_port_by_index(eth_g_index);

    if (NULL != portInfo)
    {
        if (portInfo->attr_bitmap & ETH_ATTR_LINK_STATUS)
        {
            ret = ETH_ATTR_LINKUP;
        }
        else
        {
            ret = ETH_ATTR_LINKDOWN;
        }

        free(portInfo);
    }
    else
        ret = NPD_FAIL;

    return ret;
}

int npd_get_eth_port_route_mode(unsigned int eth_g_index, unsigned char *mode)
{
    int ret = NPD_SUCCESS;
    struct eth_port_s* portInfo = NULL;
    portInfo = npd_get_port_by_index(eth_g_index);

    if (NULL != portInfo)
    {
        if (PORT_IP_INTF != portInfo->forward_mode)
        {
            *mode = 0;
        }
        else
        {
            *mode = 1;
        }

        free(portInfo);
    }
    else
    {
        ret = NPD_FAIL;
    }

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
int npd_set_ethport_route_mode(unsigned int eth_g_index,unsigned int mode)
{
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
    struct eth_port_s* portInfo = NULL;
	switch_port_db_t switch_port;

    npd_key_database_lock();
    portInfo = npd_get_port_by_index(eth_g_index);

    if (NULL != portInfo)
    {
		switch_port.global_port_ifindex = eth_g_index;
        ret = dbtable_hash_search(switch_ports_hash, &switch_port, NULL, &switch_port);
        if(ret != 0)
        {
			if(PORT_IP_INTF == portInfo->forward_mode)
			{
			    ret = INTERFACE_RETURN_CODE_ALREADY_THIS_MODE;
			}
			else
			{
				ret = INTERFACE_RETURN_CODE_NOT_SWITCHPORT;
			}
			free(portInfo);
			goto error;
        }
        if (PORT_IP_INTF != portInfo->forward_mode)
        {
			if(npd_intf_table_is_full() == TRUE)
			{
				ret = INTERFACE_RETURN_CODE_ERROR;
			}
			else
			{
                netif_notify_event(eth_g_index, PORT_NOTIFIER_L2DELETE);
                netif_app_notify_event(eth_g_index, PORT_NOTIFIER_L2DELETE, NULL, 0);
                portInfo->forward_mode = PORT_IP_INTF;
                npd_delete_switch_port(portInfo->switch_port_index);
                portInfo->switch_port_index = -1;
			}
        }
		else
		{
			ret = INTERFACE_RETURN_CODE_ALREADY_THIS_MODE;
		}
        npd_put_port(portInfo);
    }
    else
    {
        ret = INTERFACE_RETURN_CODE_ERROR;
    }
error:
    npd_key_database_unlock();
    
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
int npd_del_ethport_route_mode(unsigned int eth_g_index)
{
    int ret=INTERFACE_RETURN_CODE_SUCCESS;
    struct eth_port_s* portInfo = NULL;
    unsigned int link_status;

    npd_key_database_lock();
    portInfo = npd_get_port_by_index(eth_g_index);
    if (NULL != portInfo)
    {
        if (portInfo->attr_bitmap & ETH_ATTR_LINK_STATUS)
        {
            link_status = ETH_ATTR_LINKUP;
        }
        else
        {
            link_status = ETH_ATTR_LINKDOWN;
        }

        if (PORT_SWITCH_PORT != portInfo->forward_mode)
        {
            npd_create_switch_port(portInfo->eth_port_ifindex,
                                   "ethernet",
                                   &portInfo->switch_port_index,
                                   link_status
                                  );
            portInfo->forward_mode = PORT_SWITCH_PORT;
            npd_put_port(portInfo);
            netif_notify_event(eth_g_index, PORT_NOTIFIER_L2CREATE);
            
/* ?\B8\C3?\D3°Ï®∞\D8?\A3\AC\CF\E0\B9??\A6\C0\ED\D4\DAL2CREATE\CA?\FE\D6\D0\D2?\AD\D7\F6\C1?\AC\B6\F8\C7?\C9\C4\DC\D2\FD\B7\A2\CA\FD\BE?\BB?\D6?\C4\CE\CA\CC\E2
            if(link_status)
                netif_notify_event(eth_g_index, PORT_NOTIFIER_LINKUP_E);
            else
                netif_notify_event(eth_g_index, PORT_NOTIFIER_LINKDOWN_E);
*/
        }
        else
        {
            free(portInfo);
        }
    }
    else
    {
        ret = INTERFACE_RETURN_CODE_ERROR;
    }
    npd_key_database_unlock();
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
int npd_set_port_switch_mode(unsigned int eth_g_index,unsigned int oldMode)
{
    return npd_del_ethport_route_mode(eth_g_index);
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
int npd_del_port_switch_mode(unsigned int eth_g_index)
{
    return npd_set_ethport_route_mode(eth_g_index, 0);
}

int npd_set_eth_port_mode(unsigned int eth_g_index,unsigned int mode)
{
    int ret= ETHPORT_RETURN_CODE_NOT_SUPPORT;

    switch (mode)
    {
        case ETH_PORT_FUNC_BRIDGE:
            ret = npd_set_port_switch_mode(eth_g_index, mode);
            break;
        case ETH_PORT_FUNC_IPV4:
            ret = npd_set_ethport_route_mode(eth_g_index, mode);
        default:
            break;
    }

    return ret;
}


/**********************************************************************************
 *  npd_check_port_is_mode_type
 *
 *	DESCRIPTION:
 * 		check port swicth mode
 *
 *	INPUT:
 *		eth_g_index - port index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *			TRUE
 *			FALSE
 *
 **********************************************************************************/
int npd_check_port_switch_mode(unsigned int eth_g_index)
{
    int ret;
    struct eth_port_s* portInfo = NULL;
    portInfo = npd_get_port_by_index(eth_g_index);

    if (NULL != portInfo)
    {
        if (PORT_SWITCH_PORT == (portInfo->forward_mode))
            ret = TRUE;
        else
            ret =  FALSE;

        free(portInfo);
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}


/**********************************************************************************
 *  npd_eth_port_notifier
 *
 *	DESCRIPTION:
 * 		ethernet port link event process
 *
 *	INPUT:
 *		eth_g_index - ethernet port global index
 *		event - port link up/down event
 *
 *	OUTPUT:
 *
 *
 * 	RETURN:
 *		NPD_SUCCESS  - if no error occur
 *		NPD_FAIL - if some errors occur
 *
 *
 **********************************************************************************/
int npd_eth_port_thread_notifier
(
    unsigned int	eth_g_index,
    enum ETH_PORT_NOTIFIER_ENT	event
)
{
    struct eth_port_s 	ethPort = {0};
    int ret = 0;
    enum eth_port_type_e portMedia = ETH_INVALID;
    int  old_state = 0, new_state = 0;
    unsigned int  duplex_state = 0;
    unsigned int  flowcon_state = 0;
    eth_port_stats_t *port_counter_info = NULL;
    unsigned long duplex = 0;
    unsigned long flowControl = 0;
    int speed;
	int local;
    struct timeval tnow;
    struct timezone tzone;
    char name[20] = {0};
    int array_id;

    if(manu_testing)
        return 0;

    ethPort.eth_port_ifindex = eth_g_index;
	npd_key_database_lock();
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &ethPort);
    if (0 != ret)
    {
        goto error;
    }

	local = eth_port_local_check(eth_g_index);
	if(local == ETHPORT_RETURN_CODE_ERR_NONE)
	{
        old_state = npd_check_eth_port_status(eth_g_index);
		/*\C8\E7\B9\FB\CA?\BE\B0\E5\B6??\AC\D0\E8?\B6\D4ethPort\B5\C4\CA\FD\BE?\F8\D0°Ï®≤\FC\D0\C2*/
        ret = npd_get_port_link_status(eth_g_index,&new_state);

        if (NPD_SUCCESS != ret)
        {
            npd_netif_index_to_user_fullname(eth_g_index, name);
            /*\C8\E7\B9\FB\B1\BE\B0çI\CA\C7\D6\F7\BF\D8\C7???\BB\CA?\BE\B0\E5\BD?\DA, \B2\BB\D7\F6\B4\A6\C0\ED*/
			syslog_ax_eth_port_dbg("Can't get link status of ethernet port %s(%x).\r\n", name, eth_g_index);
            goto error;
        }
        port_counter_info = npd_get_port_counter_by_index(eth_g_index);
    
        if (NULL == port_counter_info)
        {
            syslog_ax_eth_port_err("port event: counter info for %#x node null\n",eth_g_index);
            goto error;
        }
    
        //get the duplex state
        ret = nam_get_port_duplex_mode(eth_g_index, (unsigned int*)&duplex);
        if (0 != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                      __FILE__, __LINE__, "nam_get_port_duplex_mode");
        }
        else
        {   	
            if(duplex == ETH_ATTR_DUPLEX_HALF)
            {
                duplex_state = 1;
            }
            else
            {
                duplex_state = 0;
            }
            
        }
    	
        ret = nam_get_port_flowCtrl_state(eth_g_index,&flowControl); 
        if (0 != ret)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s\r\n",
                                      __FILE__, __LINE__, "nam_get_port_flowCtrl_state");
        }
        else
        {
            if(flowControl == ETH_ATTR_FC_ENABLE)
            {
                flowcon_state = 1;
            }
            else
            {
                flowcon_state = 0;
            }
        }
        if (old_state == new_state)
        {
	/*\D3\C9\D3??\BC?DB\B2\D9\D7\F7\B2\BB\CA\C7?\D7?\D9\D7\F7\A3\AC\CB\F9\D2?\E6\D4?\C9\C4\DC\C1\AC\D0\F8\C9?\A8\CA?\FE\A3\AC
	\B5\AB\CA\C7DB\D6°Ï?\C4???\D3°Ï®≤?\E4
            goto error;
        */
        syslog_ax_eth_port_dbg("Link change trigged but state not changed.\r\n");
        }
        else if (ETH_PORT_NOTIFIER_LINKPOLL_E == event)  /*find state change when polling*/
        {
            if(old_state == new_state)
            {
                goto error;
            }
            event = (ETH_ATTR_LINKUP == new_state) ? ETH_PORT_NOTIFIER_LINKUP_E :
                    (ETH_ATTR_LINKDOWN == new_state) ? ETH_PORT_NOTIFIER_LINKDOWN_E : event;
            syslog_ax_eth_port_dbg("poll found port %#x link change %s\n",	\
                                   eth_g_index, (ETH_ATTR_LINKUP == new_state) ? "UP":"DOWN");
        }
    
        /* event not match with actual port status*/
        if (((ETH_ATTR_LINKUP == new_state) && (ETH_PORT_NOTIFIER_LINKDOWN_E == event))||
                ((ETH_ATTR_LINKDOWN == new_state) && (ETH_PORT_NOTIFIER_LINKUP_E == event)))
        {
			
            syslog_ax_eth_port_dbg("link %s event but port %#x current state %s error\n",		\
                                   (ETH_PORT_NOTIFIER_LINKUP_E == event) ? "UP":"DOWN",eth_g_index,	\
                                   (ETH_ATTR_LINKUP == new_state) ? "UP":"DOWN");
            /*\B4?\A6\B2\BB\C4?\BB\B4\A6\C0\ED\A3\AC\CF\E0?\CA?\FE\CF\C2\C3\E6\B4\FA\C2\EB\BB?\C0\ED,\BD\F6\D0\E8?\B6\D4event\B8\B3\D2\D4\D5\FD?\B5\C4?*/
            event = (ETH_ATTR_LINKUP == new_state) ? ETH_PORT_NOTIFIER_LINKUP_E :
                    (ETH_ATTR_LINKDOWN == new_state) ? ETH_PORT_NOTIFIER_LINKDOWN_E : event;
		    /*
			goto error;
			*/
        }	
	}
            
    switch (event)
    {
        case PORT_NOTIFIER_LINKUP_E:
			if(local == ETHPORT_RETURN_CODE_ERR_NONE)
			{
				/*\C8\E7\B9\FB\CA?\BE\B0\E5\B6??\AC\D0\E8?\B6\D4ethPort\CA\FD\BE\DD\D7\F6\B8\FC\D0\C2*/
                ret = npd_port_media_get(eth_g_index,&portMedia);
    
                if (NPD_TRUE == ret)
                {
                    if(portMedia == ETH_GE_SFP)
                        ethPort.attr_bitmap |= (ETH_ATTR_REAL_FIBER_MEDIA);
                    else
                        ethPort.attr_bitmap |= ETH_ATTR_REAL_COPPER_MEDIA;
                    syslog_ax_eth_port_dbg("eth_g_index::%d,port_type %d.\n",eth_g_index,portMedia);
                }
    
                ret = npd_get_port_speed(eth_g_index, (unsigned int*)&speed);
    			if(speed == ETH_ATTR_SPEED_1000M)
    				ethPort.real_speed = 1000;
    			else if(speed == ETH_ATTR_SPEED_100M)
    				ethPort.real_speed = 100;
    			else if(speed == ETH_ATTR_SPEED_10M)
    				ethPort.real_speed = 10;
    			else if(speed == ETH_ATTR_SPEED_10G)
    				ethPort.real_speed = 10000;
    			else if(speed == ETH_ATTR_SPEED_40G)
    				ethPort.real_speed = 40000;
    			else if(speed == ETH_ATTR_SPEED_100G)
    				ethPort.real_speed = 100000;
    			ethPort.attr_bitmap &= (~ETH_ATTR_SPEED_MASK);
                ethPort.attr_bitmap |= (speed << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK;
                port_counter_info->snmp_stats.linkupcount = port_counter_info->snmp_stats.linkupcount + 1;
    
    		    ethPort.attr_bitmap |= ((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
    
                if(duplex_state == NPD_TRUE)
                {
                    ethPort.attr_bitmap |= ((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX);
                }
                else
                {
                    ethPort.attr_bitmap &= (~((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX));
                }
    
                if(flowcon_state == NPD_TRUE)
                {
                    ethPort.attr_bitmap |= ((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL); 
                }
                else
                {
                    ethPort.attr_bitmap &= (~((1 << ETH_FLOWCTRL_BIT) & ETH_ATTR_FLOWCTRL));
                }
			}
            break;
        case PORT_NOTIFIER_LINKDOWN_E:
			if(local == ETHPORT_RETURN_CODE_ERR_NONE)
			{
                port_counter_info->snmp_stats.linkdowncount = port_counter_info->snmp_stats.linkdowncount + 1;
                ethPort.attr_bitmap &= ~((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
			}
            break;
        default:
            break;
    }
    if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        array_id = netif_array_index_from_ifindex(eth_g_index);
        if(event == npd_ethport_change_fast_detect[array_id].last_event)
    	{
    		/*\B9\E6\B1\DC\C1\AC\D0\F8\B5\C4\CF\E0?\CA?\FE\C9?\A8*/
            goto error;
    	}
        if(event == PORT_NOTIFIER_LINKDOWN_E)
        {
            int arpCount = npd_arp_snooping_count_all();
            int routeCount = dbtable_hash_count(npd_route_haship_index);
            int stpCount = dbtable_hash_count(ports_stp);
            int mrouteCount = dbtable_hash_count(npd_mroute_haship_index);
            int dhcpCount = dbtable_hash_count(npd_dhcp_snp_dbhash_mac_index);
            int fdbCount = dbtable_hash_count(npd_fdb_hashmac_index);
            int heavyCount = arpCount+routeCount+stpCount+mrouteCount+dhcpCount+fdbCount;
            if(heavyCount > 2000)
            {
                /*\C8\E7\B9\FB??\B4\A6\D3\DA\D6\D8\D4\D8\C7\E9\BF\F6\A3\AC\B6?\DAUP\CA?\FE\C9?\A8\D0\E8?\B6\EE\CD\E2\D1??\AC
                  \B9\E6\B1\DC\D2\F2CPU\B4\A6\C0\ED\C4\DC\C1\A6\B2\BB\B9\BB\B5\BC\D6?\C4??\D2?,??\D4?\A820\C3\EB*/
                global_portup_event_delay_count = 4;
            }
        }
        
    	npd_ethport_change_fast_detect[array_id].last_event = event;
    	memcpy(&npd_ethport_change_fast_detect[array_id].private, &ethPort, sizeof(struct eth_port_s));
        

        memset(&tnow, 0, sizeof(struct timeval));
        memset(&tzone, 0, sizeof(struct timezone));
        gettimeofday(&tnow, &tzone);
        if ((unsigned int)(tnow.tv_sec - ethPort.lastLinkChange) < 1)
        {
    		npd_ethport_change_fast_detect[array_id].fast_count++;
    		if(npd_ethport_change_fast_detect[array_id].fast_count > 10)
    		{
                npd_netif_index_to_user_fullname(eth_g_index, name);
                npd_syslog_official_event("Interface %s link state change very often. \r\n"
    				                      "The link  copper or fiber maybe damaged or the hardware wrong.\r\n"
    				                      /*"The interface is automatically shutdown.\r\n"*/, name);
    			npd_ethport_change_fast_detect[array_id].auto_shutdown = 1;
				npd_ethport_change_fast_detect[array_id].fast_count = 1;
                /*ethPort.attr_bitmap	 &= (~((1 << ETH_ADMIN_STATUS_BIT) & ETH_ATTR_ADMIN_STATUS));*/
    		}
			else
			    goto error;
        }
        else if(global_portup_event_delay_count)
        {
            /*\C8\E7\B9\FB??\B4\A6\D3\DA\D6\D8\D4\D8\C7\E9\BF\F6\A3\AC\B6?\DAUP\CA?\FE\C9?\A8\D0\E8?\B6\EE\CD\E2\D1??\AC
              \B9\E6\B1\DC\D2\F2CPU\B4\A6\C0\ED\C4\DC\C1\A6\B2\BB\B9\BB\B5\BC\D6?\C4??\D2?,??\D4?\A820\C3\EB*/
            if(event == PORT_NOTIFIER_LINKUP_E)
            {
    		    npd_ethport_change_fast_detect[array_id].fast_count++;
                goto error;
            }
        }
    	else
    	{
    		npd_ethport_change_fast_detect[array_id].fast_count = 0;
    	}
    	
    
        ethPort.lastLinkChange = tnow.tv_sec;
    
        switch (event)
        {
            case PORT_NOTIFIER_LINKUP_E:
                ethPort.attr_bitmap |= ((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
                dbtable_sequence_update(g_eth_ports, eth_g_index, NULL,  &ethPort);
                break;
            case PORT_NOTIFIER_LINKDOWN_E:
            case PORT_NOTIFIER_REMOVE:
                ethPort.attr_bitmap &= ~((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
                dbtable_sequence_update(g_eth_ports, eth_g_index, NULL,  &ethPort);
                break;
            default:
                break;
        }
		
		netif_notify_event(eth_g_index, event);
    }
#ifdef HAVE_CHASSIS_SUPPORT	
    else
    {
        syslog_ax_eth_port_dbg("Eth port(0x%x) event(%d) to MCU slot %d.\r\n", eth_g_index, event, SYS_MASTER_ACTIVE_SLOT_INDEX + 1);
        netif_remote_notify_event(SYS_MASTER_ACTIVE_SLOT_INDEX, eth_g_index, event, &ethPort, sizeof(struct eth_port_s));
    }
#endif
error:
    npd_key_database_unlock();
    return 0; 
}


unsigned npd_thread_port_event_handling(void *arg)
{
    npd_msg_list_t *msg_list = (npd_msg_list_t*)arg;
	npd_port_event_t *event = NULL;
    struct list_head *node;
    char name[50];

    sprintf(name, "npdPortEvent");
	npd_init_tell_whoami(name,0);

    while(1)
    {
        npd_msg_list_lock(msg_list);
        npd_msg_list_wait(msg_list);
		npd_msg_list_remove_head(msg_list, &node);
        event = (npd_port_event_t*)list_entry(node, npd_port_event_t, list);
		npd_msg_list_unlock(msg_list);
		while(event)
		{
            npd_eth_port_thread_notifier(event->netif_index, event->event);
			free(event);
            npd_msg_list_lock(msg_list);
		    npd_msg_list_remove_head(msg_list, &node);
            event = (npd_port_event_t*)list_entry(node, npd_port_event_t, list);
            npd_msg_list_unlock(msg_list);
		}
    }
    return 0;
}
int npd_thread_port_event_init()
{
    char name[20] = {0};
	int i;

    sprintf(name, "npdPortEvent");

    npd_create_msg_list(&npd_port_event_list);
        
    nam_thread_create(name, npd_thread_port_event_handling, npd_port_event_list,
            NPD_TRUE, NPD_FALSE);

	for(i = 0; i < MAX_ETHPORT_PER_SYSTEM; i++)
	{
		npd_ethport_change_fast_detect[i].last_event = PORT_NOTIFIER_TYPE_MAX;
	}
    return 0;
}

int npd_eth_port_notifier
(
    unsigned int	eth_g_index,
    enum ETH_PORT_NOTIFIER_ENT	event
)
{
    npd_msg_list_t *queue = npd_port_event_list;
	npd_port_event_t *evt = NULL;
    char name[20] = {0};

    evt = malloc(sizeof(npd_port_event_t));
	if(NULL == evt)
		return 0;

	memset(evt, 0, sizeof(*evt));
	evt->netif_index = eth_g_index;
	evt->event = event;
    npd_msg_list_lock(queue);
    npd_msg_list_add_tail(queue, (struct list_head*)evt);
    npd_msg_list_unlock(queue);
    return 0; 
}

void npd_eth_port_handle_delay_event()
{
	struct eth_port_s eth_port;
    struct timeval tnow;
    struct timezone tzone;
	int ret;

	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		return;
    memset(&tnow, 0, sizeof(struct timeval));
    memset(&tzone, 0, sizeof(struct timezone));
    gettimeofday(&tnow, &tzone);

    global_portup_event_delay_count--;
    if(global_portup_event_delay_count < 0)
        global_portup_event_delay_count = 0;
	npd_key_database_lock();
	ret = dbtable_sequence_traverse_next(g_eth_ports, -1, &eth_port);
	while(0 == ret)
	{
        int array_id = netif_array_index_from_ifindex(eth_port.eth_port_ifindex);
		int event = npd_ethport_change_fast_detect[array_id].last_event;
		if((npd_ethport_change_fast_detect[array_id].auto_shutdown == 1)
			&&(((unsigned int)tnow.tv_sec - (unsigned int)eth_port.lastLinkChange) > 300)
			)
		{
            eth_port.attr_bitmap |= ((1 << ETH_ADMIN_STATUS_BIT) & ETH_ATTR_ADMIN_STATUS);								  
            dbtable_sequence_update(g_eth_ports, eth_port.eth_port_ifindex, NULL,  &eth_port);
			npd_ethport_change_fast_detect[array_id].auto_shutdown = 0;
		}
        if((global_portup_event_delay_count > 0)
             && (event == PORT_NOTIFIER_LINKUP_E))
        {
	        ret = dbtable_sequence_traverse_next(g_eth_ports, eth_port.eth_port_ifindex, &eth_port);
            continue;
        }
		if((npd_ethport_change_fast_detect[array_id].fast_count > 0)
			&& (((unsigned int)tnow.tv_sec - (unsigned int)eth_port.lastLinkChange) > 1))
		{
			/*?\B4|???\C4\C4\DA\C8?\FC\D0\C2\D4\DA\CA?\FE\C9?\A8?\D2?\AD\CD\EA\B3\C9*/
            eth_port.lastLinkChange = tnow.tv_sec;
        
            switch (event)
            {
                case PORT_NOTIFIER_LINKUP_E:
                    eth_port.attr_bitmap |= ((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
                    dbtable_sequence_update(g_eth_ports, eth_port.eth_port_ifindex, NULL,  &eth_port);
    		        netif_notify_event(eth_port.eth_port_ifindex, event);
                    break;
                case PORT_NOTIFIER_LINKDOWN_E:
                case PORT_NOTIFIER_REMOVE:
                    eth_port.attr_bitmap &= ~((ETH_ATTR_LINKUP << ETH_LINK_STATUS_BIT) & ETH_ATTR_LINK_STATUS);
                    dbtable_sequence_update(g_eth_ports, eth_port.eth_port_ifindex, NULL,  &eth_port);
    		        netif_notify_event(eth_port.eth_port_ifindex, event);
                    break;
                default:
                    break;
            }
    		npd_ethport_change_fast_detect[array_id].fast_count = 0;
			
		}
	    ret = dbtable_sequence_traverse_next(g_eth_ports, eth_port.eth_port_ifindex, &eth_port);
	}
	npd_key_database_unlock();
	
}


void npd_eth_port_master_notifier
(
    unsigned int	eth_g_index,
    enum PORT_NOTIFIER_ENT	event,
    char *private, int len
)
{
    struct eth_port_s 	ethPort = {0};
    int ret = 0;
    char name[20] = {0};
	int slot, current_slot;

    ethPort.eth_port_ifindex = eth_g_index;
   
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &ethPort);

    if (-1 == ret)
    {
        syslog_ax_eth_port_err("port event: index %#x node null\n",eth_g_index);
        return;
    }
    npd_netif_index_to_user_fullname(eth_g_index, name);

   /*for snmp trap, the ifIndex must get*/
	current_slot = npd_netif_eth_get_slot(eth_g_index);
	ethPort.switch_port_index = 0;
	for(slot = 0; slot < current_slot; slot++)
	{
		ethPort.switch_port_index += ETH_LOCAL_PORT_COUNT(slot);
	}
	ethPort.switch_port_index += npd_netif_eth_get_port(eth_g_index);

    switch (event)
    {
        case PORT_NOTIFIER_LINKUP_E:
			/*\B8\FC\D0\C2\D2?\AD\D4\DA?\C3\E6\CD\EA\B3\C9*/
			netif_app_notify_event(eth_g_index, PORT_NOTIFIER_LINKUP_E, &ethPort, sizeof(ethPort));
            npd_syslog_official_event("Interface %s link state is UP.\r\n", name);
            break;
        case PORT_NOTIFIER_LINKDOWN_E:
            npd_syslog_official_event("Interface %s link state is DOWN.\r\n", name);
        case PORT_NOTIFIER_REMOVE:
			netif_app_notify_event(eth_g_index, PORT_NOTIFIER_LINKDOWN_E, &ethPort, sizeof(ethPort));
            break;
        case NOTIFIER_SWITCHOVER:
            {
                int i;
            	for(i = 0; i < MAX_ETHPORT_PER_SYSTEM; i++)
            	{
            		npd_ethport_change_fast_detect[i].last_event = PORT_NOTIFIER_TYPE_MAX;
            	}
            }
            break;
        default:
            break;
    }
    return;
}


void npd_eth_port_relate_notifier
(
    unsigned int	trunk_index,
    unsigned int    eth_g_index,
    enum PORT_RELATE_ENT	event,
    char *private, int len
)
{
    if (npd_netif_type_get(trunk_index) != NPD_NETIF_TRUNK_TYPE)
        return;

    if (npd_netif_type_get(eth_g_index) != NPD_NETIF_ETH_TYPE)
        return;

    switch (event)
    {
        case PORT_NOTIFIER_JOIN:
            /*npd_set_port_attr_default(eth_g_index);*/
            break;
        default:
            break;
    }

    return;
}

/**********************************************************************************
 *  npd_eth_port_register_notifier_hook
 *
 *	DESCRIPTION:
 * 		Register ethernet port event notifier callback function
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *
 *
 **********************************************************************************/
int npd_eth_port_register_notifier_hook
(
    void
)
{
    portNotifier = npd_eth_port_notifier;
    return 0;
}
int npd_eth_port_local_member_add
(
    unsigned int eth_g_index
)
{
    int i = 0;
    unsigned int ret = NPD_SUCCESS;
	
    for (; i < MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT; i++)
    {
        if (0 == g_eth_port_rate_poll[i])
        {
            g_eth_port_rate_poll[i] = eth_g_index;
            return NPD_SUCCESS;
        }
        else if (g_eth_port_rate_poll[i] == eth_g_index)
            return NPD_SUCCESS;
    }

    if (MAX_ETH_GLOBAL_INDEX == i)
    {
        return NPD_FAIL;
    }

    return ret;
}

unsigned int npd_eth_port_sc_global_cfg
(
    unsigned int modeType
)
{
    if ((ETH_PORT_STREAM_PPS_E != modeType)&& (ETH_PORT_STREAM_BPS_E != modeType))
    {
        return NPD_ERR;
    }
    else
    {
        scType = modeType;
        return NPD_OK;
    }
}
/**********************************************************************************
 *  npd_eth_port_get_sc_cfg
 *
 *	DESCRIPTION:
 * 		This method get dlf/mcast/bcast stream type (pps/bps) and values.
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *           strmType -     dlf/mcast/bcast
 *	OUTPUT:
 *		rateTYpe - pps/bps
 *          value      -  pps value or bps value
 * 	RETURN:
 *		NPD_OK - get success
 *          -NPD_FAIL - get failed
 **********************************************************************************/

unsigned int npd_eth_port_get_sc_cfg
(
    unsigned int eth_g_index,
    unsigned int strmType,
    unsigned int* rateType,
    unsigned int* value
)
{
    struct eth_port_s* eth_port = NULL;
    eth_port_sc_ctrl_t* port_sc = NULL;

    if (!rateType || !value)
    {
        return -NPD_FAIL;
    }

    eth_port = npd_get_port_by_index(eth_g_index);

    if (NULL == eth_port)
    {
        *value = 0;
        *rateType = ETH_PORT_STREAM_INVALID_E;
        return -NPD_FAIL;
    }

    port_sc = &eth_port->sc;

    if ((PORT_STORM_CONTROL_STREAM_DLF == strmType)&&(port_sc->dlf.ppsValid))
    {
        *value = port_sc->dlf.value.pps;
        *rateType = ETH_PORT_STREAM_PPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else if ((PORT_STORM_CONTROL_STREAM_DLF == strmType)&&(port_sc->dlf.bpsValid))
    {
        *value = port_sc->dlf.value.bps;
        *rateType = ETH_PORT_STREAM_BPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else if ((PORT_STORM_CONTROL_STREAM_BCAST == strmType)&&(port_sc->bcast.ppsValid))
    {
        *value = port_sc->bcast.value.pps;
        *rateType = ETH_PORT_STREAM_PPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else if ((PORT_STORM_CONTROL_STREAM_BCAST == strmType)&&(port_sc->bcast.bpsValid))
    {
        *value = port_sc->bcast.value.bps;
        *rateType = ETH_PORT_STREAM_BPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else if ((PORT_STORM_CONTROL_STREAM_MCAST == strmType)&&(port_sc->mcast.ppsValid))
    {
        *value = port_sc->mcast.value.pps;
        *rateType = ETH_PORT_STREAM_PPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else if ((PORT_STORM_CONTROL_STREAM_MCAST == strmType)&&(port_sc->mcast.bpsValid))
    {
        *value = port_sc->mcast.value.bps;
        *rateType = ETH_PORT_STREAM_BPS_E;
		free(eth_port);
		return NPD_OK;
    }
    else
    {
        *value = 0;
        *rateType = ETH_PORT_STREAM_INVALID_E;
        free(eth_port);
        return -NPD_FAIL;
    }

    free(eth_port);
    return NPD_FAIL;
}

int npd_eth_port_sc_cfg
(
    unsigned int eth_g_index,
    unsigned int scMode,
    unsigned int sctype,
    unsigned int scvalue
)
{
    unsigned int  valid= 0,portSpeed = 0;
    int ret = NPD_SUCCESS;
    enum eth_port_type_e portType = ETH_INVALID;
    struct eth_port_s*  eth_port = NULL;
    eth_port_sc_ctrl_t* port_sc = NULL;

	npd_key_database_lock();
    eth_port = npd_get_port_by_index(eth_g_index);

    if (NULL == eth_port)
    {
        syslog_ax_eth_port_err("g_port(%#x) struct not exist!\n",eth_g_index);
        ret =  NPD_DBUS_ERROR_NO_SUCH_PORT;
    }
    else
    {
        portType = eth_port->port_type;
        portSpeed = (eth_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;
        switch (portSpeed)
        {
			case 0:
				break;
			case PORT_SPEED_100_E:
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 148810)
					{
						syslog_ax_eth_port_err("Port working at 100Mbps does not support value larger than 1488100!\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				else
				{
					if(scvalue > 100000000)
					{
						syslog_ax_eth_port_err("Port working at 100Mbps does not support value larger than 100000000!\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				break;
			case PORT_SPEED_1000_E:
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 1488100)
					{
						syslog_ax_eth_port_err("Port working at 10Mbps does not support value larger than 1488100\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				else
				{
					if(scvalue > 1000000000)
					{
						syslog_ax_eth_port_err("Port working at 10Mbps does not support value larger than 10000000!\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				break;
			case PORT_SPEED_10000_E:
		    /*\D0?\C4?\B2\BB\D4\CA\D0\ED\C5\E4\D6?\F3\D3\DA1G\B5??\D2\D6\D6\C6\C1\F7\C1\BF*/
#if 0				
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 14881000)
					{
						syslog_ax_eth_port_err("Port working at 10Mbps does not support value larger than 1488100\n");
						return NPD_DBUS_ERROR_NOT_SUPPORT;
					}
				}
				else
				{
					if(scvalue > 10000000000)
					{
						syslog_ax_eth_port_err("Port working at 10Mbps does not support value larger than 10000000!\n");
						return NPD_DBUS_ERROR_NOT_SUPPORT;
					}
				}
#endif				
				break;
			default:
				syslog_ax_eth_port_err("Port working at unknow speed!\n");
				ret = NPD_DBUS_ERROR_NOT_SUPPORT;
				goto retcode;
        }
		
        switch (portType)
        {
			case ETH_FE_TX:
			case ETH_FE_FIBER:
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 148810)
					{
						syslog_ax_eth_port_err("Port working at 100Mbps does not support value larger than 148810!\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				else
				{
					if(scvalue > 100000000)
					{
						syslog_ax_eth_port_err("Port working at 100Mbps does not support value larger than 100000000!\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
				}
				break;
			case ETH_GTX:
			case ETH_GE_FIBER:
			case ETH_GE_SFP:
			case ETH_GE_COMBO:
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 1488100)
					{
						syslog_ax_eth_port_err("Port working at 1000Mbps does not support value larger than 1488100\n");
						ret = NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
					else if(scvalue == 1488100)
					{
						valid = 0;
					}
					else
					{
						valid = 1;
					}
				}
				else
				{
					if(scvalue > 1000000000)
					{
						syslog_ax_eth_port_err("Port working at 1000Mbps does not support value larger than 100000000!\n");
						ret =NPD_DBUS_ERROR_NOT_SUPPORT;
						goto retcode;
					}
					else if(scvalue == 1000000000)
					{
						valid = 0;
					}
					else
					{
						valid = 1;
					}
				}
				break;
			case ETH_XGTX:
			case ETH_XGE_FIBER:
			case ETH_XGE_SFPPLUS:
			case ETH_XGE_XFP:
		    /*\D0?\C4?\B2\BB\D4\CA\D0\ED\C5\E4\D6?\F3\D3\DA1G\B5??\D2\D6\D6\C6\C1\F7\C1\BF*/
#if 0				
				if(ETH_PORT_STREAM_PPS_E == scMode)
				{
					if(scvalue > 14881000)
					{
						syslog_ax_eth_port_err("Port working at 10000Mbps does not support value larger than 14881000\n");
						return NPD_DBUS_ERROR_NOT_SUPPORT;
					}
					else if(scvalue == 14881000)
					{
						valid = 0;
					}
					else
					{
						valid = 1;
					}
				}
				else
				{
					if(scvalue > 10000000000)
					{
						syslog_ax_eth_port_err("Port working at 10000Mbps does not support value larger than 10000000000!\n");
						return NPD_DBUS_ERROR_NOT_SUPPORT;
					}
					else if(scvalue == 10000000000)
					{
						valid = 0;
					}
					else
					{
						valid = 1;
					}
				}
#endif				
				break;
			default:
				syslog_ax_eth_port_err("Port working at unknow speed!\n");
				ret = NPD_DBUS_ERROR_NOT_SUPPORT;
				goto retcode;
        }
		port_sc = &eth_port->sc;
	
        if (PORT_STORM_CONTROL_STREAM_DLF == sctype)
        {
            if (ETH_PORT_STREAM_PPS_E == scMode)
            {
                port_sc->dlf.bpsValid = 0;
                port_sc->dlf.ppsValid = valid;
                port_sc->dlf.value.pps = scvalue;
            }
            else if (ETH_PORT_STREAM_BPS_E == scMode)
            {
                port_sc->dlf.ppsValid = 0;
                port_sc->dlf.bpsValid = valid;
                port_sc->dlf.value.bps = scvalue;
            }
        }
        else if (PORT_STORM_CONTROL_STREAM_MCAST == sctype)
        {
            if (ETH_PORT_STREAM_PPS_E == scMode)
            {
                port_sc->mcast.bpsValid = 0;
                port_sc->mcast.ppsValid = valid;
                port_sc->mcast.value.pps = scvalue;
            }
            else if (ETH_PORT_STREAM_BPS_E == scMode)
            {
                port_sc->mcast.ppsValid = 0;
                port_sc->mcast.bpsValid = valid;
                port_sc->mcast.value.bps = scvalue;
            }
        }
        else if (PORT_STORM_CONTROL_STREAM_BCAST == sctype)
        {
            if (ETH_PORT_STREAM_PPS_E == scMode)
            {
                port_sc->bcast.bpsValid = 0;
                port_sc->bcast.ppsValid = valid;
                port_sc->bcast.value.pps = scvalue;
            }
            else if (ETH_PORT_STREAM_BPS_E == scMode)
            {
                port_sc->bcast.ppsValid = 0;
                port_sc->bcast.bpsValid = valid;
                port_sc->bcast.value.bps = scvalue;
            }
        }
		npd_put_port(eth_port);
		
retcode:
	    npd_key_database_unlock();
		return ret;
#if 0
        if ((ETH_FE_TX == portType)||(ETH_FE_FIBER == portType))
        {
            portType = ETH_PORT_STREAM_FE_E;
        }
        else
        {
			if((ETH_GE_COMBO == portType)
				||(ETH_GTX == portType)
				||(ETH_GE_FIBER == portType)
				||(ETH_GE_SFP == portType))
			{
                portType = ETH_PORT_STREAM_GE_E;
			}
			else if((ETH_XGE_FIBER == portType)
				||(ETH_XGE_SFPPLUS == portType)
				||(ETH_XGE_XFP == portType)
				||(ETH_XGTX == portType))
			{
                portType = ETH_PORT_STREAM_XE_E;
			}
        }

        portSpeed = (eth_port->attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;

        /*GE port support <0-1488100>,FE port support <0-148810>*/
        if (((PORT_SPEED_100_E == portSpeed)
			||((0 == portSpeed)
			&&(ETH_PORT_STREAM_FE_E == portType)))
			&&(ETH_PORT_STREAM_PPS_E == scMode)
			&&(scvalue > 148810))
        {
            syslog_ax_eth_port_err("FE port or 100Mbps port does not support value larger than 148810!\n");
            ret = NPD_DBUS_ERROR_NOT_SUPPORT;
        }
        else if (((PORT_SPEED_100_E == portSpeed)
			||((0 == portSpeed)
			&&(ETH_PORT_STREAM_FE_E == portType)))
			&&(ETH_PORT_STREAM_BPS_E == scMode)
			&&(scvalue > 100000000))
        {
            syslog_ax_eth_port_err("FE port or 100Mbps portdoes not support value larger than 100000000!\n");
            ret = NPD_DBUS_ERROR_NOT_SUPPORT;
        }
        else
        {
            if (ETH_PORT_STREAM_PPS_E == scMode)
            {
                valid = ((((portSpeed == PORT_SPEED_100_E)
					||((0 == portSpeed)
					&&(ETH_PORT_STREAM_FE_E == portType)))
					&&(scvalue == 148810))
					||(((portSpeed == PORT_SPEED_10000_E)
					||(portSpeed == PORT_SPEED_1000_E)
					||((0 == portSpeed)
					&&(ETH_PORT_STREAM_GE_E == portType)))
					&&(scvalue == 1488100)))? \
                        0 : 1;
            }
            else if (ETH_PORT_STREAM_BPS_E == scMode)
            {
                valid = ((((portSpeed == PORT_SPEED_100_E)
					||((0 == portSpeed)
					&&(ETH_PORT_STREAM_FE_E == portType)))
					&&(scvalue == 100000000))
					||(((portSpeed == PORT_SPEED_10000_E)
					||(portSpeed == PORT_SPEED_1000_E)
					||((0 == portSpeed)
					&&(ETH_PORT_STREAM_GE_E == portType)))
					&&(scvalue == 1000000000)))? \
                        0 : 1;
            }

            port_sc = &eth_port->sc;

            if (PORT_STORM_CONTROL_STREAM_DLF == sctype)
            {
                if (ETH_PORT_STREAM_PPS_E == scMode)
                {
                    port_sc->dlf.bpsValid = 0;
                    port_sc->dlf.ppsValid = 1;
                    port_sc->dlf.value.pps = scvalue;
                }
                else if (ETH_PORT_STREAM_BPS_E == scMode)
                {
                    port_sc->dlf.ppsValid = 0;
                    port_sc->dlf.bpsValid = 1;
                    port_sc->dlf.value.bps = scvalue;
                }
            }
            else if (PORT_STORM_CONTROL_STREAM_MCAST == sctype)
            {
                if (ETH_PORT_STREAM_PPS_E == scMode)
                {
                    port_sc->mcast.bpsValid = 0;
                    port_sc->mcast.ppsValid = 1;
                    port_sc->mcast.value.pps = scvalue;
                }
                else if (ETH_PORT_STREAM_BPS_E == scMode)
                {
                    port_sc->mcast.ppsValid = 0;
                    port_sc->mcast.bpsValid = 1;
                    port_sc->mcast.value.bps = scvalue;
                }
            }
            else if (PORT_STORM_CONTROL_STREAM_BCAST == sctype)
            {
                if (ETH_PORT_STREAM_PPS_E == scMode)
                {
                    port_sc->bcast.bpsValid = 0;
                    port_sc->bcast.ppsValid = 1;
                    port_sc->bcast.value.pps = scvalue;
                }
                else if (ETH_PORT_STREAM_BPS_E == scMode)
                {
                    port_sc->bcast.ppsValid = 0;
                    port_sc->bcast.bpsValid = 1;
                    port_sc->bcast.value.bps = scvalue;
                }
            }
        }

        npd_put_port(eth_port);
#endif
    }

    return ret;
}

int npd_ethport_desc(unsigned int netif_index, char *desc)
{
    struct eth_port_s *portInfo = NULL;

    portInfo = npd_get_port_by_index(netif_index);

    if (NULL == portInfo)
    {
        return ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    strncpy(portInfo->desc, desc, 63);
    npd_put_port(portInfo);
    return COMMON_SUCCESS;
}

DBusMessage * npd_dbus_config_eth_port_rate_poll(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* 	reply = NULL;
    DBusMessageIter	iter = {0};
    DBusError 		err;
    unsigned int	ret = NPD_DBUS_ERROR;
    int 	rate_poll_enable;
    struct npd_eth_cfg_s npd_eth_cfg_set = {0};
	
    syslog_ax_acl_dbg("Entering config ethernet port rate poll!\n");
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&rate_poll_enable,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    if (npd_eth_port_rate_poll_enable != rate_poll_enable)
    {
        npd_eth_cfg_set.rate_poll_enable = rate_poll_enable;
        ret = dbtable_array_update(npd_eth_cfg_index, 0, &npd_eth_cfg_set, &npd_eth_cfg_set);
        if (ret != 0)
        {
            syslog_ax_eth_port_err("Failed to %s rate poll: %d\n",rate_poll_enable? "enable": "disable", ret);
            ret=NPD_DBUS_ERROR;
        }
        else
        {
            ret = NPD_DBUS_SUCCESS;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&ret);
    return reply;
}
DBusMessage * npd_dbus_show_eth_port_rate_poll(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* 	reply = NULL;
    DBusMessageIter	iter = {0};
    DBusError 		err;
    unsigned int	ret = NPD_DBUS_ERROR;
    int 	rate_poll_enable;
    struct npd_eth_cfg_s npd_eth_cfg_set = {0};

    dbus_error_init(&err);

    ret = dbtable_array_get(npd_eth_cfg_index, 0, &npd_eth_cfg_set);

    if (ret != 0)
    {
        rate_poll_enable = 0;
    }
    else
    {
        rate_poll_enable = npd_eth_cfg_set.rate_poll_enable;
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&rate_poll_enable);
    return reply;
}

int combo_port_active_medium_get(unsigned int eth_g_index, int *active_medium)
{
	int ret = 0;
    struct eth_port_s *port = NULL;

    port = npd_get_port_by_index(eth_g_index);

    if(NULL != port)
    {
        if(port->attr_bitmap & ETH_ATTR_REAL_COPPER_MEDIA)
        {
		    *active_medium = COMBO_PHY_MEDIA_PREFER_COPPER;
            ret = NPD_SUCCESS;
        }
        else if(port->attr_bitmap & ETH_ATTR_REAL_FIBER_MEDIA)
        {
 		    *active_medium = COMBO_PHY_MEDIA_PREFER_FIBER;
            ret = NPD_SUCCESS;
        }
        else
            ret = NPD_FAIL;
        free(port);
    }
    return ret;
}


int npd_ethport_show_running(void *data, char *string, int* size)
{
    struct eth_port_s *portInfo = (struct eth_port_s*)data;
    unsigned int eth_g_index;
    unsigned int local_port_attrmap = 0, local_port_mtu = 0,local_port_ipg = 0, local_eee_state = 0;
    unsigned char an = 0,an_state = 0,an_fc = 0,fc = 0,an_duplex = 0,duplex = 0,an_speed = 0;
    unsigned char admin_status = 0,bp = 0,media = 0;
    PORT_SPEED_ENT speed = PORT_SPEED_10_E;
    int local_port_type;
    unsigned int rateType;
    unsigned int rateValue;
    char *tmpBuf;
    char tmpCommand[256];
    char name[50];
    int length = 0;
    int enter_node = 0;

#ifdef HAVE_LACP
    lacp_port_actor_t lacp_port_actor;
#endif
	int ret = 0;
	
    tmpBuf = malloc(*size);

    if (NULL == tmpBuf)
        return -1;

    *tmpBuf = 0;

    if (portInfo == NULL)
    {
        free(tmpBuf);
        return -1;
    }

    eth_g_index = portInfo->eth_port_ifindex;
    local_port_attrmap  = portInfo->attr_bitmap;
    local_port_mtu		= portInfo->mtu;
    local_port_ipg      = portInfo->ipg;
    local_port_type     = portInfo->port_type;
	local_eee_state     = portInfo->eee;
    admin_status = (local_port_attrmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
    bp = (local_port_attrmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
    an_state = (local_port_attrmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;
    an = (local_port_attrmap & ETH_ATTR_AUTONEG_CTRL) >> ETH_AUTONEG_CTRL_BIT;
    an_speed = (local_port_attrmap & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT;
    speed = (local_port_attrmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;
    an_duplex = (local_port_attrmap & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT;
    duplex = (local_port_attrmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;
    an_fc = (local_port_attrmap & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT;
    fc = (local_port_attrmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
    media = (local_port_attrmap & (ETH_ATTR_PREFERRED_FIBER_MEDIA|ETH_ATTR_PREFERRED_COPPER_MEDIA)) >> ETH_PREFERRED_COPPER_MEDIA_BIT;
    npd_netif_index_to_user_fullname(eth_g_index, name);
    sprintf(tmpCommand, "interface %s\n", name);
    length = length+strlen(tmpCommand);

    if (length < *size)
        strcat(tmpBuf, tmpCommand);
    else
        goto error;

	if(0 != strlen(portInfo->desc))
    {
        enter_node = 1;
        sprintf(tmpCommand," description %s\n", portInfo->desc);
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
		
    if (local_port_mtu != ethport_attr_default(local_port_type)->mtu)
    {
        enter_node = 1;
        sprintf(tmpCommand," mtu %d\n",local_port_mtu);
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

    if (local_port_ipg != ETH_ATTR_DEFAULT_MINIMUM_IPG)
    {
        enter_node = 1;
        sprintf(tmpCommand," minimum-ipg %d\n",local_port_ipg);
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

    /*  Auto-Nego - all AN options disable we need one command control*/
	if(local_port_type == ETH_XGE_SFPPLUS)
	{
	    if(speed != ethport_attr_default(local_port_type)->speed)
	    {
    		if(speed == PORT_SPEED_1000_E)
    		{
                enter_node = 1;
                sprintf(tmpCommand," speed 1000\n");
                length = length+strlen(tmpCommand);
        
                if (length < *size)
                    strcat(tmpBuf, tmpCommand);
                else
                    goto error;
    		}
    		if(an_state != ETH_ATTR_ON)
    		{
                enter_node = 1;
                sprintf(tmpCommand," no auto-negotiation\n");
                length = length+strlen(tmpCommand);
        
                if (length < *size)
                    strcat(tmpBuf, tmpCommand);
                else
                    goto error;
    		}
	    }
	}
	else if(local_port_type == ETH_GE_SFP)
	{
		if(speed == PORT_SPEED_100_E)
		{
            enter_node = 1;
            sprintf(tmpCommand," speed 100\n");
            length = length+strlen(tmpCommand);
    
            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
		}
		else
		{
    		if(an_state != ETH_ATTR_ON)
    		{
                enter_node = 1;
                sprintf(tmpCommand," no auto-negotiation\n");
                length = length+strlen(tmpCommand);
        
                if (length < *size)
                    strcat(tmpBuf, tmpCommand);
                else
                    goto error;
    		}
		}
	}
	else
	{
        if (an_state != ethport_attr_default(local_port_type)->autoNego)
        {
            enter_node = 1;
            sprintf(tmpCommand, an_state ? " auto-negotiation\n": " no auto-negotiation\n");
            length = length+strlen(tmpCommand);
    
            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    
        if (an_state != ETH_ATTR_ON)
        {
            enter_node = 1;
    		if(speed == PORT_SPEED_1000_E)
    		{
                sprintf(tmpCommand," speed 1000\n");
    		}
    		else if(speed == PORT_SPEED_100_E)
    		{
                sprintf(tmpCommand," speed 100\n");
    		}
    		else if(speed == PORT_SPEED_10_E)
    		{
                sprintf(tmpCommand," speed 10\n");
    		}
    		else if(speed == PORT_SPEED_10000_E)
    		{
                sprintf(tmpCommand," speed 10000\n");
    		}
    		else if(speed == PORT_SPEED_40G_E)
    		{
                sprintf(tmpCommand," speed 40G\n");
    		}
    		else if(speed == PORT_SPEED_100G_E)
    		{
                sprintf(tmpCommand," speed 100G\n");
    		}
            length = length+strlen(tmpCommand);
    
            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    	else
    	{
    		if(speed == PORT_SPEED_1000_E)
    		{
                enter_node = 1;
                sprintf(tmpCommand," speed 1000\n");
                length = length+strlen(tmpCommand);
    
                if (length < *size)
                    strcat(tmpBuf, tmpCommand);
                else
                    goto error;
    		}
    	}
	}
    /* duplex - if not auto-nego duplex and duplex mode not default*/
    if ((an_state != ETH_ATTR_ON) && (duplex != ethport_attr_default(local_port_type)->duplex))
    {
        enter_node = 1;
        sprintf(tmpCommand," duplex mode %s\n", duplex ? "half":"full");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

    /* back-pressure  - back pressure only relevant when port in half-duplex mode*/
    if ((an_state != ETH_ATTR_ON) && (bp != ethport_attr_default(local_port_type)->bp) 
              && (duplex == ETH_ATTR_DUPLEX_HALF))
    {
        enter_node = 1;
        sprintf(tmpCommand," back-pressure %s\n", bp ? "on":"off");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

    /* flow-control - if not auto-nego fc and fc mode not default*/
    if ((fc != ethport_attr_default(local_port_type)->fc) 
          && (duplex == ETH_ATTR_DUPLEX_FULL))
    {
        enter_node = 1;
        sprintf(tmpCommand," flow-control %s\n", fc ? "on":"off");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

	if(portInfo->loopback)
	{
        enter_node = 1;
        sprintf(tmpCommand," loopback on\n");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
	}

	/* eee_state - if  auto-nego is on and eee_state mode not default*/
	/*default eee_state is disable*/
    if ((local_eee_state != ETH_ATTR_OFF) && (an_state == ETH_ATTR_ON))
    {
        enter_node = 1;
        sprintf(tmpCommand," eee %s\n", local_eee_state ? "enable":"disable");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
#ifdef HAVE_LACP
	{
		lacp_port_actor.netif_index = eth_g_index;
		ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, &lacp_port_actor);
		if(ret == 0)
		{

        	enter_node = 1;
        	sprintf(tmpCommand," lacp enable\n");
	        length = length+strlen(tmpCommand);
	        if (length < *size)
	            strcat(tmpBuf, tmpCommand);

		}
	}
#endif
    /*media priority*/
    if (media != ethport_attr_default(local_port_type)->mediaPrefer)
    {
        enter_node = 1;
        switch (media)
        {
            case COMBO_PHY_MEDIA_PREFER_NONE:
                sprintf(tmpCommand," media preferred none\n");
                break;
            case COMBO_PHY_MEDIA_PREFER_FIBER:
                sprintf(tmpCommand, " media preferred fiber\n");
                break;
            case COMBO_PHY_MEDIA_PREFER_COPPER:
                sprintf(tmpCommand, " media preferred copper\n");
                break;
            default:
				memset(tmpCommand, 0, 256);
                break;
        }
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }

    if(portInfo->bandwidth[0] != 0)
    {
        enter_node = 1;
        sprintf(tmpCommand," ingress bandwidth %u\n",
            portInfo->bandwidth[0]);
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
    if(portInfo->bandwidth[1] != 0)
    {
        enter_node = 1;
        sprintf(tmpCommand," egress bandwidth %u\n",
            portInfo->bandwidth[1]);
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
    /* eth-port storm control*/
    if (NPD_OK == npd_eth_port_get_sc_cfg(eth_g_index,PORT_STORM_CONTROL_STREAM_DLF,&rateType,&rateValue))
    {
      
		if(rateType != ETH_PORT_STREAM_INVALID_E)
        {
            enter_node = 1;
            sprintf(tmpCommand," storm-control dlf %s %d\n",(ETH_PORT_STREAM_PPS_E == rateType)? "pps" : "bps",rateValue);
            length = length+strlen(tmpCommand);

            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    }

    if (NPD_OK == npd_eth_port_get_sc_cfg(eth_g_index,PORT_STORM_CONTROL_STREAM_MCAST,&rateType,&rateValue))
    {
    
        if(rateType != ETH_PORT_STREAM_INVALID_E)
        {
            enter_node = 1;
            sprintf(tmpCommand," storm-control multicast %s %d\n",(ETH_PORT_STREAM_PPS_E == rateType)? "pps" : "bps",rateValue);
            length = length+strlen(tmpCommand);

            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    }

    if (NPD_OK == npd_eth_port_get_sc_cfg(eth_g_index,PORT_STORM_CONTROL_STREAM_BCAST,&rateType,&rateValue))
    {  
  
        if(rateType != ETH_PORT_STREAM_INVALID_E)
        {
            enter_node = 1;
            sprintf(tmpCommand," storm-control broadcast %s %d\n",(ETH_PORT_STREAM_PPS_E == rateType)? "pps" : "bps",rateValue);
            length = length+strlen(tmpCommand);

            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    }

    /* admin status*/
    if (admin_status != ethport_attr_default(local_port_type)->admin_state)
    {
		int array_id = netif_array_index_from_ifindex(eth_g_index);
		/*\B2\BB\B4?\D2\F2?\B6?\DA\C1\B4?\B2\BB\CE?\A8\B5\BC\D6?\C4shutdown*/
        if(0 == npd_ethport_change_fast_detect[array_id].auto_shutdown)		
        {
            enter_node = 1;
            sprintf(tmpCommand,admin_status ?" no shutdown\n":" shutdown\n");
            length = length+strlen(tmpCommand);
    
            if (length < *size)
                strcat(tmpBuf, tmpCommand);
            else
                goto error;
        }
    }
    if(portInfo->ip_sg)
    {
        enter_node = 1;
        sprintf(tmpCommand," ip-source-guard\n");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
    if(portInfo->forward_mode == PORT_IP_INTF)
    {
        enter_node = 1;
        sprintf(tmpCommand," no switchport\n");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;
    }
	else
	{
        if (npd_switch_port_show_running(portInfo->switch_port_index,
                                         tmpBuf+length-1, *size-length+1) == 1)
        {
            enter_node = 1;
        }
	}

    if ((enter_node) || (length < strlen(tmpBuf)))  /* port configured*/
    {
        length = strlen(tmpBuf);
        sprintf(tmpCommand," exit\n");
        length = length+strlen(tmpCommand);

        if (length < *size)
            strcat(tmpBuf, tmpCommand);
        else
            goto error;

        strcat(string, tmpBuf);
    }

    *size = *size - length;
    free(tmpBuf);
    return 0;
error:
    free(tmpBuf);
    return -1;
}

DBusMessage *npd_dbus_ethports_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusError err;
    char *showStr = NULL;
    int totalLen = 0;
    unsigned int eth_g_index;
    struct eth_port_s eth_port;
    int ret;
	npd_syslog_dbg("Ethernet port show running!\n");
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID))) {
		 npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
    showStr = (char*)malloc(NPD_ETHPORT_SHOWRUN_CFG_SIZE);

    if (NULL == showStr)
    {
        syslog_ax_eth_port_err("memory malloc error\n");
        return NULL;
    }

    memset(showStr,0,NPD_ETHPORT_SHOWRUN_CFG_SIZE);
    totalLen = NPD_ETHPORT_SHOWRUN_CFG_SIZE;
    eth_port.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &eth_port);
    if(ret != 0)
        goto retcode;

	/* for mng port ,can't show in the running  */
	if (eth_port.port_type == ETH_MNG)
	{
		goto retcode;
		
	}
    
    npd_ethport_show_running(&eth_port, showStr, &totalLen);

retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_STRING,
                                   &showStr);
    free(showStr);
    showStr = NULL;
    return reply;
}

DBusMessage *npd_dbus_get_next_portindex(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int 	eth_g_index = 0;
    int ret = 0;
    DBusError err;
    struct eth_port_s eth_port;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        npd_syslog_err("Unable to get input args\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    npd_syslog_dbg("To get the next netif index of port 0x%x\n", eth_g_index);
	
    if(eth_g_index == 0)
        eth_g_index = -1;
    ret = dbtable_sequence_traverse_next(g_eth_ports, eth_g_index, &eth_port);

    if (-1 == ret)
        eth_g_index = 0;
    else
        eth_g_index = eth_port.eth_port_ifindex;

    npd_syslog_dbg("The next netif index is 0x%x\n", eth_g_index);
	
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &eth_g_index);
    return reply;
}

DBusMessage *  npd_dbus_switchport_exist(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int netif_index = 0;
    struct switch_port_db_s port= {0};
    unsigned int ret = 0;
    DBusError err;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To check if port 0x%x is a switch port.\n", netif_index);
    port.global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &port, NULL, &port);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage *  npd_dbus_show_switchport(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int netif_index = 0;
    struct switch_port_db_s port= {0};
    char *untag_vlan_str = NULL;
    char *tag_vlan_str = NULL;
    unsigned int ret = 0;
    DBusError err;
    untag_vlan_str = malloc(256);
	if(untag_vlan_str == NULL)
	{
		return NULL;
	}
    memset(untag_vlan_str, 0, 256);
	
    tag_vlan_str = malloc(256);
	if(tag_vlan_str == NULL)
	{
		free(untag_vlan_str);
		return NULL;
	}
    memset(tag_vlan_str, 0, 256);
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            npd_syslog_dbg("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        free(untag_vlan_str);
        free(tag_vlan_str);
        return NULL;
    }

    npd_syslog_dbg("Show switch-port info for netif 0x%x.\n", netif_index);
    port.global_port_ifindex = netif_index;
    ret = dbtable_hash_search(switch_ports_hash, &port, NULL, &port);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &port.state);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &port.vlan_access_mode);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &port.vlan_private_mode);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(port.fdb_limit));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(port.pvid));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(port.stp_flag));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &port.fdb_learning_mode);
    vbmp_2_vlanlist_str(port.allow_untag_vlans, untag_vlan_str);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_STRING,
                                   &untag_vlan_str);
    vbmp_2_vlanlist_str(port.allow_tag_vlans, tag_vlan_str);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_STRING,
                                   &tag_vlan_str);
    free(untag_vlan_str);
    free(tag_vlan_str);
    return reply;
}

/*NEW NPD DBUS API FOR EHTERNET USING ETHERNET PORT INDEX AS INPUT PARAMETER.*/
DBusMessage * npd_dbus_show_ethport_attr(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int eth_g_index = 0;
    struct eth_port_s portInfo = {0};
    unsigned int ret = 0;
    unsigned int op_ret = 0;
#ifdef HAVE_LACP
    unsigned char lacp_sta = 0;
    lacp_port_actor_t lacp_port_actor;
#endif
    char *desc;

    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;
    }

#ifdef HAVE_LACP
    lacp_port_actor.netif_index = eth_g_index;
	op_ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, &lacp_port_actor);
	if(op_ret == 0)
	{
        lacp_sta = NPD_TRUE;
	}
#endif

    if (PORT_ONLINE_REMOVED == portInfo.state)
    {
        goto retcode;
    }

    if (portInfo.port_type == ETH_MNG)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;        
    }	

    ret = eth_port_local_check(eth_g_index);

    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        ret = npd_get_eth_port_drv_info(eth_g_index, &portInfo);
    }
retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(portInfo.port_type));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(portInfo.state));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &(portInfo.attr_bitmap));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &portInfo.mtu);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &portInfo.lastLinkChange);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &portInfo.eee);
#ifdef HAVE_LACP
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_BYTE,
                                   &lacp_sta);
#endif
    desc = strdup(portInfo.desc);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_STRING,
                                   &desc);
	dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &portInfo.forward_mode);

	dbus_message_iter_append_basic(&iter,
		                           DBUS_TYPE_UINT32,
		                           &portInfo.loopback);
	dbus_message_iter_append_basic(&iter,
		                           DBUS_TYPE_UINT32,
		                           &portInfo.bandwidth[0]);
	dbus_message_iter_append_basic(&iter,
		                           DBUS_TYPE_UINT32,
		                           &portInfo.bandwidth[1]);
	
    if(desc)
        free(desc);
    
    return reply;
}

DBusMessage * npd_dbus_show_ethport_ipg(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    int eth_g_index = 0;
    unsigned int ret = 0;
    unsigned char ipg;
    struct eth_port_s portInfo = {0};
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;
    }
    
    ipg = (unsigned char)portInfo.ipg;

retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_BYTE,
                                   &ipg);
    return reply;
}
#ifdef HAVE_CHASSIS_SUPPORT
DBusMessage *npd_dbus_clear_stack_stat_by_slotno_and_portno
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	unsigned char slot_no = 0;
	unsigned char port_no = 0;
    struct eth_port_counter_s *ptr, portPktCount;
    unsigned int ret = 0;
    DBusError err;
    struct eth_port_counter_s *port_counter_info = NULL;
	int module_type = 0;
    ptr = &portPktCount;
    unsigned char devNum = 0;
    unsigned char devPort = 0;
    memset(ptr,0,sizeof(struct eth_port_counter_s));

	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_BYTE, &slot_no,
                                DBUS_TYPE_BYTE, &port_no,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args\n");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	if (!CHASSIS_SLOTNO_ISLEGAL(slot_no))
	{
		ret = BOARD_RETURN_CODE_NO_SUCH_SLOT;
		goto retcode;
	}
	
	if (SYS_LOCAL_MODULE_SLOT_INDEX != CHASSIS_SLOT_NO2INDEX(slot_no))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	module_type = MODULE_TYPE_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slot_no));	
	if (module_type == 0)
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	if(! ETH_STACK_PORTNO_ISLEGAL(slot_no, port_no))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	devNum = PPAL_PLANE_2_UNIT(module_type, port_no-1);
	devPort = PPAL_PLANE_2_PORT(module_type, port_no-1);
	ret = nam_asic_stack_port_pkt_statistic(devNum, devPort, ptr);
    syslog_ax_eth_port_dbg("get stack port detail of devNum:%d, devPort:%d, ret:%d!\n",devNum, devPort, ret);
    if (ret != 0)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
    }
	ret = nam_asic_clear_stack_port_pkt_stat(devNum, devPort);
	syslog_ax_eth_port_dbg("clear stack port detail ret:%d!\n",ret);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_ERR_HW;
    }
    else
    {
		port_counter_info = npd_get_stack_port_counter_by_slotno_portno(slot_no, port_no);

        if (NULL == port_counter_info)
        {
            syslog_ax_eth_port_dbg("get port counter info errors\n");
            ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        else
        {
        	memset(port_counter_info, 0, sizeof(struct eth_port_counter_s));
        }
    }
retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}
DBusMessage * npd_dbus_show_port_stat_by_slotno_and_portno
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter, iter_array, iter_struct;
	unsigned char slot_no = 0;
	unsigned char port_no = 0;
    unsigned int linkuptimes = 0, linkdowntimes = 0;
    struct eth_port_counter_s *ptr, portPktCount;
    unsigned int ret = 0, i = 0;
    unsigned long long tmp = 0;
    DBusError err;
    struct eth_port_counter_s *port_counter_info = NULL;
	int module_type = 0;
    ptr = &portPktCount;
    unsigned int link_state = 0;
    unsigned char devNum = 0;
    unsigned char devPort = 0;
    unsigned char modNum = 0;
	
    memset(ptr,0,sizeof(struct eth_port_counter_s));
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_BYTE, &slot_no,
                                DBUS_TYPE_BYTE, &port_no,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args\n");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	if (!CHASSIS_SLOTNO_ISLEGAL(slot_no))
	{
		ret = BOARD_RETURN_CODE_NO_SUCH_SLOT;
		goto retcode;
	}
	
	if (SYS_LOCAL_MODULE_SLOT_INDEX != CHASSIS_SLOT_NO2INDEX(slot_no))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	module_type = MODULE_TYPE_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slot_no));	
	if (module_type == 0)
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	if(! ETH_STACK_PORTNO_ISLEGAL(slot_no, port_no))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	ret = npd_get_stack_port_stat(slot_no, port_no, ptr);
    syslog_ax_eth_port_dbg("Show stack port detail of slotno:%d, port_no:%d, ret:%d!\n",slot_no, port_no, ret);
    if (ret != 0)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
    else
    {
		port_counter_info = npd_get_stack_port_counter_by_slotno_portno(slot_no, port_no);

        if (NULL == port_counter_info)
        {
            syslog_ax_eth_port_dbg("get port counter info errors\n");
            ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        else
        {
            linkuptimes = port_counter_info->linkupcount;
            linkdowntimes = port_counter_info->linkdowncount;
	     	devNum = PPAL_PLANE_2_UNIT(module_type, port_no-1);
	     	devPort = PPAL_PLANE_2_PORT(module_type, port_no-1);
			modNum = (unsigned char)UNIT_2_MODULE(module_type, CHASSIS_SLOT_NO2INDEX(slot_no), devNum, devPort);
	     	ret = nam_get_stack_port_link_state(devNum, devPort, (unsigned long *)&link_state);
        }
    }
retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);

    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        dbus_message_iter_open_container(&iter,
                                         DBUS_TYPE_ARRAY,
                                         DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                         DBUS_TYPE_UINT64_AS_STRING
                                         DBUS_STRUCT_END_CHAR_AS_STRING,
                                         &iter_array);

        for (i = 0; i< 26; i++)
        {
            dbus_message_iter_open_container(&iter_array,
                                             DBUS_TYPE_STRUCT,
                                             NULL,
                                             &iter_struct);

            if (NULL == port_counter_info)
            {
                tmp = 0;
                dbus_message_iter_append_basic
                (&iter_struct,
                 DBUS_TYPE_UINT64,
                 &(tmp));
            }
            else
            {
                tmp = *((unsigned long long*)(port_counter_info) + i);
                dbus_message_iter_append_basic
                (&iter_struct,
                 DBUS_TYPE_UINT64,
                 &(tmp));
            }

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }

        dbus_message_iter_close_container(&iter, &iter_array);
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &linkuptimes);
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &linkdowntimes);
		dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &link_state);

		dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &modNum);
		dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &devNum);
		dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &devPort);
		
    }
    return reply;
}

DBusMessage * npd_dbus_get_next_stack_port
(
	DBusConnection *conn, 
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	unsigned char slot_no = 0;
	unsigned char port_no = 0;
    unsigned int ret = 0;
    DBusError err;	
	DBusMessageIter iter;
		
	int module_type = 0;
	int devNum = 0;
	int devPort = 0;
	int module_id = 0;
	
    dbus_error_init(&err);
	
    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_BYTE, &slot_no,
                                DBUS_TYPE_BYTE, &port_no,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args\n");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	
	if (slot_no == 0)
		slot_no = 1;
	port_no += 1;
	
	if (!CHASSIS_SLOTNO_ISLEGAL(slot_no))
	{
		ret = BOARD_RETURN_CODE_NO_SUCH_SLOT;
		goto retcode;
	}
	module_type = MODULE_TYPE_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slot_no));	
	if (module_type == 0)
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}	
	if(!ETH_STACK_PORTNO_ISLEGAL(slot_no, port_no))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	if ((MODULE_STATUS_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slot_no)) == RMT_BOARD_NOEXIST)
		||(MODULE_STATUS_ON_SLOT_INDEX(CHASSIS_SLOT_NO2INDEX(slot_no)) == RMT_BOARD_HWINSERTED))
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	devNum = PPAL_PLANE_2_UNIT(module_type, port_no-1);
	devPort = PPAL_PLANE_2_PORT(module_type, port_no-1);						
	module_id = UNIT_2_MODULE(module_type, CHASSIS_SLOT_NO2INDEX(slot_no), devNum, devPort);
		
retcode:
	reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
    	dbus_message_iter_append_basic(&iter, 
									   DBUS_TYPE_BYTE, &slot_no);
		dbus_message_iter_append_basic(&iter, 
									   DBUS_TYPE_BYTE, &port_no);
		dbus_message_iter_append_basic(&iter, 
									   DBUS_TYPE_INT32, &devNum);
		dbus_message_iter_append_basic(&iter,
									   DBUS_TYPE_INT32, &devPort);
		dbus_message_iter_append_basic(&iter, 
			                           DBUS_TYPE_INT32, &module_id);
    }
	return reply;	
}
#endif
DBusMessage * npd_dbus_show_ethport_stat
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
)
{
    DBusMessage* reply;
    DBusMessageIter  iter, iter_array, iter_struct;
    unsigned int eth_g_index = 0;
    unsigned int linkuptimes = 0, linkdowntimes = 0;
    unsigned int ret = 0, i = 0;
    unsigned long long tmp = 0;
    DBusError err;
	struct eth_port_s portInfo;
    eth_port_stats_t portCnt;
	eth_port_stats_t *port_counter_info = NULL;
	
    memset(&portInfo, 0, sizeof(struct eth_port_s));
	memset(&portCnt, 0, sizeof(eth_port_stats_t));
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args\n");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

   
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;
    }

	ret = eth_port_local_and_master_check(eth_g_index);
	if (0 != ret)
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	
    syslog_ax_eth_port_dbg("Show ether port detail of 0x%x !\n",eth_g_index);
    ret = npd_get_eth_port_stat(eth_g_index, &portCnt);

    if (ret != 0)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
    else
    {
        port_counter_info = npd_get_port_counter_by_index(eth_g_index);

        if (NULL == port_counter_info)
        {
            syslog_ax_eth_port_dbg("get port counter info errors\n");
            ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        else
        {
            linkuptimes = port_counter_info->snmp_stats.linkupcount;
            linkdowntimes = port_counter_info->snmp_stats.linkdowncount;
        }
    }

retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);

    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        dbus_message_iter_open_container(&iter,
                                         DBUS_TYPE_ARRAY,
                                         DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                         DBUS_TYPE_UINT64_AS_STRING
                                         DBUS_STRUCT_END_CHAR_AS_STRING,
                                         &iter_array);

        for (i = 0; i< 64; i++)
        {
            dbus_message_iter_open_container(&iter_array,
                                             DBUS_TYPE_STRUCT,
                                             NULL,
                                             &iter_struct);

            if (NULL == port_counter_info)
            {
                tmp = 0;
                dbus_message_iter_append_basic
                (&iter_struct,
                 DBUS_TYPE_UINT64,
                 &(tmp));
            }
            else
            {
                tmp = *((unsigned long long*)(port_counter_info) + i);
                dbus_message_iter_append_basic
                (&iter_struct,
                 DBUS_TYPE_UINT64,
                 &(tmp));
            }

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }

        dbus_message_iter_close_container(&iter, &iter_array);
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &linkuptimes);
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &linkdowntimes);
    }

    return reply;
}

DBusMessage * npd_dbus_show_ethport_rate
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int eth_g_index = 0;
    unsigned int ret = 0;
    DBusError err;
	struct eth_port_s portInfo;
	unsigned int inbandwidth = 0, outbandwidth = 0;
	
    memset(&portInfo, 0, sizeof(struct eth_port_s));
	
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args\n");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

   
    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);
    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;
    }

	ret = eth_port_local_and_master_check(eth_g_index);
	if (0 != ret)
	{
		ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
		goto retcode;
	}
	ret = npd_get_eth_port_rate(eth_g_index, &inbandwidth, &outbandwidth);
	//printf("%s: inbandwidthyte = %d,outbandwidth =%d\n",__func__,inbandwidth,outbandwidth);
	if (ret != 0)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
	dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &inbandwidth);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &outbandwidth);
	
    return reply;
}
unsigned int eth_port_sfp_type_check(struct eth_port_s *portInfo)
{
	if (portInfo->port_type == ETH_FE_FIBER 
		|| portInfo->port_type == ETH_GE_FIBER
		|| portInfo->port_type == ETH_GE_SFP
		|| portInfo->port_type == ETH_GE_COMBO
		|| portInfo->port_type == ETH_XGE_XFP
		|| portInfo->port_type == ETH_XGE_FIBER
		|| portInfo->port_type == ETH_XGE_SFPPLUS)
	{
		return ETHPORT_RETURN_CODE_ERR_NONE;
	}

	return ETHPORT_RETURN_CODE_ERR_SFPTYPE;
}

int nbm_sfp_presence_get(int index, int *presence_state);
int nbm_sfp_light_get(int index, int * light_state);

DBusMessage * npd_dbus_show_ethport_sfp
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
)
{
    DBusMessage* reply;
    DBusMessageIter  iter;

    unsigned int eth_g_index = 0;
    eth_port_sfp sfp_info;
    struct eth_port_s portInfo = {0};
    unsigned int ret = 0;
    unsigned int op_ret = 0;
	unsigned int sfp_index = 0;
	NPD_NETIF_INDEX_U eth_ifindex;

    DBusError err;
    dbus_error_init(&err);
    
    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    portInfo.eth_port_ifindex = eth_g_index;
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &portInfo);

    if (0 != ret)
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        goto retcode;
    }

    if (PORT_ONLINE_REMOVED == portInfo.state)
    {
        goto retcode;
    }
	ret = eth_port_sfp_type_check(&portInfo);
	if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
    {
		syslog_ax_eth_port_err("port %d is not sfp type.\n",eth_g_index);
        goto retcode;
    }

    ret = eth_port_local_and_master_check(eth_g_index);
    if (ETHPORT_RETURN_CODE_ERR_NONE != ret)
    {
        goto retcode;
    }    
    ret = npd_get_eth_port_drv_info(eth_g_index, &portInfo);


	eth_ifindex.netif_index = eth_g_index;
	sfp_index  = eth_ifindex.eth_if.port;
	
	syslog_ax_eth_port_dbg("get sfp slot %d port %d info.\n", 
		eth_ifindex.eth_if.slot, sfp_index);

	/* operate sfp type*/
	op_ret = nbm_sfp_presence_get(sfp_index, (int*)&sfp_info.presense);
	if (op_ret != 0)
	{
		syslog_ax_eth_port_err("port %d read presense state error.\n", eth_g_index);
		ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	op_ret = nbm_sfp_light_get(sfp_index, (int*)&sfp_info.laser);
	if (op_ret != 0)
	{
		syslog_ax_eth_port_err("port %d read laser state error.\n", eth_g_index);
		ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	            
retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &sfp_info.presense);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &sfp_info.laser);
	
    return reply;
}



DBusMessage * npd_dbus_clear_ethport_stat(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int eth_g_index = 0;
    struct eth_port_s* g_ptr = NULL;
    unsigned int ret = 0;
    DBusError err;
    eth_port_stats_t *port_counter_info = NULL;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To clear ethport(0x%x) stats!\n", eth_g_index);

    ret = eth_port_local_and_master_check(eth_g_index);
    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        /*get eth port stat*/
        g_ptr = npd_get_port_by_index(eth_g_index);

        if (NULL == g_ptr)
        {
            syslog_ax_eth_port_dbg("get port info errors\n");
            ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
        }
        else
        {
            ret = nam_asic_clear_port_pkt_stat(eth_g_index);

            if (0 != ret)
            {
                syslog_ax_eth_port_dbg("port_index %d,ret %d\n",eth_g_index,ret);
                syslog_ax_eth_port_dbg("nam_asic_port_pkt_statistic ERROR\n");
                ret = ETHPORT_RETURN_CODE_ERR_HW;
            }
            else
            {
                syslog_ax_eth_port_dbg("clear counter info\n");
                port_counter_info = npd_get_port_counter_by_index(eth_g_index);

                if (NULL == port_counter_info)
                {
                    syslog_ax_eth_port_dbg("get port counter info errors\n");
                    ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
                }
                else
                {
                    memset(port_counter_info, 0, sizeof(eth_port_stats_t));
                }
            }

            free(g_ptr);
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_config_ethport_one(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int netif_index = 0, ret = 0;
    int route;
    unsigned int l3_ifindex;
    DBusError err;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To check ether port 0x%x!\n",netif_index);
    ret = npd_netif_check_exist(netif_index);
    route = npd_intf_exist_check(netif_index, &l3_ifindex);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &netif_index);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &route);
    return reply;
}

DBusMessage * npd_dbus_config_ethport_desc(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int netif_index = 0, ret = 0;
    char *desc;
    DBusError err;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&netif_index,
                                DBUS_TYPE_STRING, &desc,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To set description for ether port 0x%x!\n",netif_index);

    ret = npd_ethport_desc(netif_index, desc);
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}


DBusMessage* npd_dbus_config_ethport_ipg(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned char port_ipg = 0xf;
    unsigned int eth_g_index = 0;
    unsigned int ret = 0;
    struct eth_port_s* g_ptr = NULL;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_BYTE,&port_ipg,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    g_ptr = npd_get_port_by_index(eth_g_index);

    if (g_ptr)
    {
        g_ptr->ipg = port_ipg;
        npd_put_port(g_ptr);
    }
    else
    {
        ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}
DBusMessage* npd_dbus_eth_link_state_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned int  eth_g_index;
    unsigned int value = 0,ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&value,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }


    ret = eth_port_local_and_master_check(eth_g_index);
    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        syslog_ax_eth_port_dbg("npd_set_port_link_status value %d \n",value);
        ret = npd_set_port_link_status(eth_g_index,value);

        if (ret != NPD_SUCCESS)
            syslog_ax_eth_port_dbg("set link status failed\n");
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&eth_g_index);
    return reply;
}

DBusMessage* npd_dbus_config_eth_port_attr(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned int type = 0;
    unsigned int eth_g_index = 0;
    unsigned int value = 0;
    int ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&type,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&value,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To set ether port 0x%x,type: %d, value: %d\n", eth_g_index, type, value);
    ret = eth_port_sw_attr_update(eth_g_index, type, value);

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &eth_g_index);
    return reply;
}

DBusMessage* npd_dbus_config_eth_port_ratelimit(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    struct eth_port_s eth_port = {0};
    unsigned int eth_g_index = 0;
    unsigned int bandwidth = 0;
    unsigned int burstsize = 0;
    int flow_dir;
    int port_speed;
    int ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32, &flow_dir,
                                DBUS_TYPE_UINT32,&bandwidth,
                                DBUS_TYPE_UINT32,&burstsize,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To set rate-limit for ether port 0x%x, bandwidth %u burstsize %u\n",
         eth_g_index,bandwidth,burstsize);
    port_speed = eth_port_sw_speed_get(eth_g_index);
    /*port_speed is Mbps, translate to Kbps*/
    if((bandwidth > port_speed*1000)
        || (burstsize > port_speed*1000))
    {
        ret = INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND;
        goto dbus_reply;
    }
    eth_port.eth_port_ifindex = eth_g_index;
    npd_key_database_lock();
    ret = dbtable_sequence_search(g_eth_ports, eth_g_index, &eth_port);
    if(0 != ret)
    {
        ret = INTERFACE_RETURN_CODE_NO_SUCH_PORT;
        goto dbus_reply;
    }
    
    eth_port.bandwidth[flow_dir] = bandwidth;
    eth_port.burstsize[flow_dir] = burstsize;
    ret = dbtable_sequence_update(g_eth_ports, eth_g_index, NULL, &eth_port);
    if (0 != ret)
    {
        ret = COMMON_ERROR;
        goto dbus_reply;
    }

dbus_reply:
	npd_key_database_unlock();
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage* npd_dbus_config_ports_vct(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned int g_index= ~0;
	unsigned int mode = 0;
    int ret = 0;/* 1 -if main board port, 0 - slave board port */
	int media_prefered = 0;
    struct eth_port_s *portPtr = NULL;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&g_index,
                                DBUS_TYPE_UINT32,&mode,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    if(VCT_IS_AVAILABLE == FALSE){
        ret = NPD_DBUS_ERROR_DISENABLE_PLATFORM;
        goto error; 
    }
	
    portPtr = npd_get_port_by_index(g_index);
	syslog_ax_eth_port_dbg("\n vctisablebefore = %d\n", portPtr->vct_isable);
		
    if (NULL == portPtr)
    {
        ret = NPD_DBUS_ERROR;
    }
    else if ((portPtr->port_type == ETH_GE_SFP)
             || (portPtr->port_type == ETH_GE_FIBER)
             || (portPtr->port_type == ETH_XGE_SFPPLUS))
    {
        /*only ETH_GTX can operate*/
        free(portPtr);
        ret = NPD_DBUS_ERROR_UNSUPPORT;
    }
	else if(portPtr->port_type == ETH_GE_COMBO)
	{
	    ret = combo_port_active_medium_get(g_index, &media_prefered);
		if(media_prefered == COMBO_PHY_MEDIA_PREFER_FIBER)
		{
		    free(portPtr);
            ret = NPD_DBUS_ERROR_UNSUPPORT;
		}
		else
		{
			portPtr->vct_isable = mode;/*mode = 1 is enable, mode = 0 is dsable*/
	        npd_put_port(portPtr);
		}
	}
    else
    {
        portPtr->vct_isable = mode;/*mode = 1 is enable, mode = 0 is dsable*/
        npd_put_port(portPtr);
    }

error:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &g_index);
    return reply;
}

DBusMessage* npd_dbus_get_ethport_vct(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned int g_index= ~0, len = 0;
    int ret = 0;/* 1 -if main board port, 0 - slave board port */
    struct eth_port_s *portPtr = NULL;
    unsigned short state = 0;
	unsigned int result = 0;
	unsigned int vct_enable = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }


    ret = eth_port_local_and_master_check(g_index);
    if (ret == ETHPORT_RETURN_CODE_ERR_NONE)
    {
        portPtr = npd_get_port_by_index(g_index);

        if (NULL == portPtr)
        {
            ret = NPD_DBUS_ERROR;
        }
        else if ((portPtr->port_type == ETH_GE_SFP)
                 || (portPtr->port_type == ETH_GE_FIBER))
        {
            /*only ETH_GTX can operate*/
            free(portPtr);
            ret = NPD_DBUS_ERROR;
        }
        else
        {
            vct_enable = portPtr->vct_isable;
			free(portPtr);
			if(vct_enable == 0)
			{
			    goto retcode;
			}
				
            ret = nam_vct_phy_read(g_index, &state,&len,&result);
        }
    }
    else
    {
        ret = NPD_DBUS_ERROR;
    }
retcode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &g_index);
	dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &vct_enable);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT16,
                                   &state);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &len);
	dbus_message_iter_append_basic(&iter,                                   
								   DBUS_TYPE_UINT32,                                   
								   &result);
    return reply;
}
/****************************************************
 *
 * RETURN:
 *		INTERFACE_RETURN_CODE_SUCCESS
 *		INTERFACE_RETURN_CODE_ERROR
 *		INTERFACE_RETURN_CODE_NO_SUCH_PORT
 *		INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND
 *		INTERFACE_RETURN_CODE_ALREADY_THIS_MODE
 *   no interface
 *		INTERFACE_RETURN_CODE_PORT_HAS_SUB_IF
 *	advanced-routing disable
 *		COMMON_RETURN_CODE_NULL_PTR
 *		INTERFACE_RETURN_CODE_PORT_HAS_SUB_IF
 *		INTERFACE_RETURN_CODE_NAM_ERROR
 *		INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *		INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *		INTERFACE_RETURN_CODE_FD_ERROR
 *		INTERFACE_RETURN_CODE_IOCTL_ERROR
 *		INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *	advanced-routing enable
 *		INTERFACE_RETURN_CODE_DEFAULT_VLAN_IS_L3_VLAN
 *******************************************************/
DBusMessage * npd_dbus_config_port_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int	mode;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned int  eth_g_index = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&mode,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    ret = INTERFACE_RETURN_CODE_SUCCESS;

    if (INTERFACE_RETURN_CODE_SUCCESS == ret)
    {
        ret = npd_set_eth_port_mode(eth_g_index,mode);

        if (INTERFACE_RETURN_CODE_SUCCESS == ret || INTERFACE_RETURN_CODE_ALREADY_THIS_MODE == ret)
        {
			ret = INTERFACE_RETURN_CODE_SUCCESS;
        }
		else
        {
            syslog_ax_eth_port_err("npd_port_type_deal err re_value is %#x \n",ret);
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,	&ret);
    dbus_message_iter_init_append(reply, &iter);
    return reply;
}

/****************************************************
 *
 * RETURN:
 *		COMMON_RETURN_CODE_NULL_PTR
 *		INTERFACE_RETURN_CODE_SUCCESS
 *		INTERFACE_RETURN_CODE_ERROR
 *		INTERFACE_RETURN_CODE_NO_SUCH_PORT
 *		INTERFACE_RETURN_CODE_PORT_HAS_SUB_IF
 *		INTERFACE_RETURN_CODE_NAM_ERROR
 *		INTERFACE_RETURN_CODE_CHECK_MAC_ERROR
 *		INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *		INTERFACE_RETURN_CODE_FD_ERROR
 *		INTERFACE_RETURN_CODE_IOCTL_ERROR
 *		INTERFACE_RETURN_CODE_MAC_GET_ERROR
 *
 *******************************************************/

DBusMessage * npd_dbus_config_port_interface_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    int ret = INTERFACE_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned int  eth_g_index = 0;
    unsigned int ifIndex = ~0UI;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    ret = INTERFACE_RETURN_CODE_SUCCESS;

    if (INTERFACE_RETURN_CODE_SUCCESS == ret)
    {
        ret = npd_set_ethport_route_mode(eth_g_index, 0);
    }
    if(INTERFACE_RETURN_CODE_SUCCESS == ret || INTERFACE_RETURN_CODE_ALREADY_THIS_MODE == ret)
    {
		ret = INTERFACE_RETURN_CODE_SUCCESS;
    }
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,	&ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,	&ifIndex);
    dbus_message_iter_init_append(reply, &iter);
    return reply;
}

DBusMessage *npd_dbus_config_storm_control(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int  scMode= 0,sctype = 0,scvalue = 0;
    unsigned int	eth_g_index = 0;
    int ret = NPD_SUCCESS;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&scMode,
                                DBUS_TYPE_UINT32,&sctype,
                                DBUS_TYPE_UINT32,&scvalue,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    ret = npd_eth_port_sc_cfg(eth_g_index,scMode,sctype,scvalue);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage* npd_dbus_get_eth_port_eee(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned int eth_g_index = 0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
	unsigned int eee_state = 0;
	int ret  = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}


    ret = eth_port_local_and_master_check(eth_g_index);
	if (ret == ETHPORT_RETURN_CODE_ERR_NONE)
    {
		ret = npd_get_port_eee(eth_g_index, &eee_state);
		if (ret != NPD_SUCCESS)
			op_ret = ETHPORT_RETURN_CODE_UNSUPPORT;
    }
	else
	{
		op_ret = NPD_DBUS_ERROR;
	}
error:
  
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);
	return reply;
} 


DBusMessage* npd_dbus_config_eth_port_eee(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned char an_state = 0;
	unsigned int eth_g_index = 0;
	unsigned int value = 0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
    struct eth_port_s *eth_port = NULL;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}
    
	eth_port = npd_get_port_by_index(eth_g_index);
    if (eth_port)
    { 
		an_state = (eth_port->attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;
		
		if (an_state != ETH_ATTR_AUTONEG_DONE)
		{
            eth_port->attr_bitmap |= ((1 << ETH_AUTONEG_BIT) & ETH_ATTR_AUTONEG);
            
            if((eth_port->port_type == ETH_GE_FIBER) || (eth_port->port_type == ETH_GTX)
				|| (eth_port->port_type == ETH_GE_SFP))
            {
                 eth_port->attr_bitmap &= 0xFFFF0FFF;
                 eth_port->attr_bitmap |= ((PORT_SPEED_1000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK); /*SPEED 1000*/
            }
            else if(eth_port->port_type == ETH_XGE_SFPPLUS || eth_port->port_type == ETH_XGE_FIBER 
                || eth_port->port_type == ETH_XGE_XFP)
            {
               eth_port->attr_bitmap &= 0xFFFF0FFF;
               eth_port->attr_bitmap |= ((PORT_SPEED_10000_E << ETH_SPEED_BIT) & ETH_ATTR_SPEED_MASK);/*bit12~15 represent 16 kinds of speed*/
            }
            eth_port->attr_bitmap &= (~((1 << ETH_DUPLEX_BIT) & ETH_ATTR_DUPLEX));                  /*FULL DUPLEX*/
            eth_port->attr_bitmap &= (~((1 << ETH_BACKPRESSURE_BIT) & ETH_ATTR_BACKPRESSURE));      /*back pressure disable*/                 /*FULL DUPLEX*/
    			
		}
		eth_port->eee = value;
        npd_put_port(eth_port);
    }
    else
    {
        op_ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
error:
  
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
} 

DBusMessage* npd_dbus_config_eth_port_loopback(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned int eth_g_index = 0;
	unsigned int value = 0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
    struct eth_port_s *eth_port = NULL;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}
    
	npd_syslog_dbg("To %s LOOPBACK at port 0x%x\n",value ? "enable" : "disable", eth_g_index);

	eth_port = npd_get_port_by_index(eth_g_index);
    if (eth_port)
    { 
		eth_port->loopback = value;
        npd_put_port(eth_port);
    }
    else
    {
        op_ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT;
    }
error:
  
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
} 

DBusMessage* npd_dbus_get_eth_port_loopback(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned int eth_g_index = 0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
	int loopback_state = 0;
	int ret  = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}


    ret = eth_port_local_and_master_check(eth_g_index);
	if (ret == ETHPORT_RETURN_CODE_ERR_NONE)
    {
		ret = npd_get_port_loopback(eth_g_index, &loopback_state);
		if (ret != NPD_SUCCESS)
			op_ret = ETHPORT_RETURN_CODE_UNSUPPORT;
    }
	else
	{
		op_ret = NPD_DBUS_ERROR;
	}
error:
  
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);
	return reply;
} 


#define NPD_PACKET_TYPE_OTHER 31
DBusMessage * npd_dbus_clear_cpu_stats(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int eth_g_index = 0;
    unsigned int ret = 0;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_eth_port_dbg("To clear stats from ethport(0x%x) to cpu!\n", eth_g_index);

    ret = eth_port_local_and_master_check(eth_g_index);
    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
		ret = npd_packet_netif_type_stats_clear(eth_g_index);
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_show_cpu_stats(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int eth_g_index = 0;
    unsigned int ret = 0, i = 0;
	unsigned int rx_type_stats[NPD_PACKET_TYPE_OTHER];
	unsigned int rx_type_pps[NPD_PACKET_TYPE_OTHER];
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_eth_port_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_eth_port_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    
    ret = eth_port_local_check(eth_g_index);
    memset(rx_type_stats, 0, sizeof(rx_type_stats));
    memset(rx_type_pps, 0, sizeof(rx_type_pps));
    if (ETHPORT_RETURN_CODE_ERR_NONE == ret)
    {
        ret = npd_packet_netif_type_stats_get(eth_g_index, rx_type_stats, rx_type_pps);
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
	if(ret == 0)
	{
    	for(i = 0; i < NPD_PACKET_TYPE_OTHER; i++)
    	{
            dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &(rx_type_stats[i]));
    	}
    	for(i = 0; i < NPD_PACKET_TYPE_OTHER; i++)
    	{
            dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT32,
                                       &(rx_type_pps[i]));
    	}
	}
    return reply;
}



/*the following is the driver for l2 and l3*/
int npd_port_allow_vlan(
    unsigned int eth_g_index,
    int vid,
    int isTagged
)
{
    int ret;
    npd_syslog_dbg("Ethernet port 0x%x allow vlan %d with tag mode %d\n",
                   eth_g_index, vid, isTagged);
    ret = nam_asic_vlan_entry_ports_add(eth_g_index, vid, isTagged);
    npd_intf_vlan_add_eth_hw_handler(vid, eth_g_index);
    return ret;
}

int npd_port_free_vlan(
    unsigned int eth_g_index,
    int vid,
    int isTagged
)
{
    int ret;
    npd_syslog_dbg("Ethernet port 0x%x free vlan %d with tag mode %d\n",
                   eth_g_index, vid, isTagged);
    npd_intf_vlan_del_eth_hw_handler(vid, eth_g_index);
    ret = nam_asic_vlan_entry_ports_del(vid, eth_g_index, isTagged);
	if(ret != 0)
	{
		return ret;
	}
    ret = nam_fdb_table_delete_entry_with_vlan_port(vid, eth_g_index);
    return ret;
}

int npd_port_set_pvid(
    unsigned int eth_g_index,
    int pvid
)
{
    int ret;
    npd_syslog_dbg("Ethernet port 0x%x pvid set %d\n",
                   eth_g_index, pvid);
    ret = nam_asic_set_port_pvid(eth_g_index, pvid);
    npd_intf_vlan_add_eth_hw_handler(pvid, eth_g_index);
    return ret;
}

int npd_port_set_inner_pvid(
    unsigned int eth_g_index,
    int pvid
)
{
    int ret;
    npd_syslog_dbg("Ethernet port inner 0x%x pvid set %d\n",
                   eth_g_index, pvid);
    ret = nam_asic_set_port_inner_pvid(eth_g_index, pvid);
    //npd_intf_vlan_add_eth_hw_handler(pvid, eth_g_index);
    return ret;
}



int npd_port_set_vlan_pri(
    unsigned int eth_g_index,
    int vid,
    int pri,
    int cfi
)
{
    return 0;
}

int npd_port_vlan_mode_set(unsigned int eth_g_index, int mode)
{
	int ret = 0;
	npd_syslog_dbg("Ethernet port 0x%x vlan mode set %d\n", eth_g_index, mode);
	ret = nam_port_vlan_mode_set(eth_g_index, mode);

	return ret;
}

int npd_port_set_fdb_limit(
    unsigned int eth_g_index,
    int fdb_limit
)
{
    unsigned char devNum = 0,portNum = 0;
    unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;
    int 		 retVal = 0;
    nam_learn_limit_t limit;
    memset(&limit,0,sizeof(nam_learn_limit_t));
    npd_syslog_dbg("Ethernet port 0x%x set fdb limit %d \n",
                   eth_g_index, fdb_limit);
    ret = npd_get_devport_by_global_index(eth_g_index,&devNum,&portNum);

    if (NPD_SUCCESS != ret)
    {
        return NPD_SUCCESS;
    }

    /*clear the fdb entry in hw*/
	if(fdb_limit >= 0)
	{
        nam_fdb_table_delete_entry_with_port(eth_g_index);
	}
    limit.limit = fdb_limit;
    limit.port = portNum;
    limit.flags = NAM_L2_LEARN_LIMIT_PORT;
    retVal = nam_fdb_limit_set(devNum, limit);

    if (0 != retVal)
    {
        syslog_ax_fdb_err("set fdb limit failed for %d\n",ret);
        ret = FDB_RETURN_CODE_OCCUR_HW;
    }

    return ret;
}

int npd_port_set_fdb_limit_byvlanport(
    unsigned int eth_g_index,
    int vid,
    int fdb_limit
)
{
    return 0;
}

int npd_port_fdb_delete_by_port(unsigned int g_ifindex)
{
    nam_fdb_table_delete_entry_with_port(g_ifindex);
    return 0;
}

int npd_port_fdb_delete_by_vlan_port(unsigned int g_ifindex, int vid)
{
    nam_fdb_table_delete_entry_with_vlan_port(vid, g_ifindex);
    return 0;
}

int npd_port_fdb_add(unsigned char mac[], int vid, unsigned int g_ifindex)
{
    return nam_static_fdb_entry_mac_vlan_port_set(mac, vid, g_ifindex);
}

/*status : 0                      not learning and discard unknown SA
                   1                      auto learning and forward 
                   2                      protect mode. CPU learning and discard packet when mac limit full
*/
int npd_port_fdb_learning_mode(
    unsigned int eth_g_index,
    int mode
)
{
    unsigned char devNum = 0,portNum = 0;
    unsigned int ret = VLAN_RETURN_CODE_ERR_NONE;

    npd_syslog_dbg("Ethernet port 0x%x set fdb learning mode %d \n",
                   eth_g_index, mode);
    ret = npd_get_devport_by_global_index(eth_g_index,&devNum,&portNum);

    if (NPD_SUCCESS != ret)
    {
        return NPD_SUCCESS;
    }

    ret = nam_fdb_port_learn_status_set(eth_g_index, mode);
    return ret;
}

int npd_port_isolate_add(
    unsigned int src_netif_index,
    unsigned int dst_netif_index
)
{
    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(dst_netif_index))
	{
		ret = nam_port_isolate_add(src_netif_index, dst_netif_index);
	}	
	else if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(dst_netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(dst_netif_index);

        dbtable_sequence_lock(g_trunks);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = nam_port_isolate_add(src_netif_index, eth_g_index);
            if(0 != ret)
                all_ret = ret;			
		}
        dbtable_sequence_unlock(g_trunks);
	}
	else
		ret = NPD_FAIL;	
    //unsigned int ret = NPD_FDB_ERR_NONE;
    //ret = nam_port_isolate_add(src_netif_index, dst_netif_index);
    return ret;
}

int npd_port_isolate_del(
    unsigned int src_netif_index,
    unsigned int dst_netif_index
)
{

    int ret, all_ret;
    unsigned int array_port;
	unsigned int eth_g_index;
    struct trunk_s node = {0};
	
	if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(dst_netif_index))
	{
		ret = nam_port_isolate_del(src_netif_index, dst_netif_index);
	}		
	if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(dst_netif_index))
	{
		node.trunk_id = npd_netif_trunk_get_tid(dst_netif_index);
        dbtable_sequence_lock(g_trunks);
		ret = dbtable_sequence_search(g_trunks, node.trunk_id, &node);
		
		NPD_PBMP_ITER(node.ports, array_port)
		{
			eth_g_index = eth_port_array_index_to_ifindex(array_port);
			ret = nam_port_isolate_del(src_netif_index, eth_g_index);
            if(0 != ret)
                all_ret = ret;			
		}
        dbtable_sequence_unlock(g_trunks);
	}
	else
		ret = NPD_FAIL;
    //unsigned int ret = NPD_FDB_ERR_NONE;
    //ret = nam_port_isolate_del(src_netif_index, dst_netif_index);    
    return ret;
}

int npd_port_igmp_trap_set(
    int vid,
    unsigned int netif_index,
    int flags
)
{
    return nam_asic_igmp_trap_set_by_devport(vid,
            netif_index, flags);
}

int npd_port_mld_trap_set(
	int vid,
	unsigned int netif_index,
	int flags
)
{
	return nam_asic_mld_trap_set_by_devport(vid,
			netif_index, flags);
}

int npd_port_dhcp_trap_set(
    int vid,
    unsigned int netif_index,
    int flags
)
{
    return nam_asic_dhcp_trap_set_by_devport(vid,
            netif_index, flags);
}
int npd_port_access_qinq_enable(
    unsigned int netif_index,
    int flags)
{
    return nam_port_qinq_enable(netif_index, flags);
}

int npd_port_qinq_drop_miss(
    unsigned int netif_index,
    int flags)
{
    return nam_port_qinq_drop_miss_enable(netif_index, flags);
}

int npd_port_subnet_vlan_enable(
    unsigned int netif_index,
    int flags)
{
    return nam_port_subnet_vlan_enable(netif_index, flags);
}

int npd_port_mac_vlan_enable(
    unsigned int netif_index,
    int flags)
{
    return nam_port_mac_vlan_enable(netif_index, flags);
}

int npd_port_prefer_subnet_enable(
    unsigned int netif_index,
    int flags)
{
    return nam_port_subnet_prefer(netif_index, flags);
}

int npd_port_vlan_filter(
    unsigned int netif_index,
    int flags)
{
	return nam_port_vlan_filter(netif_index, flags);
}


int npd_port_tpid_set(
    unsigned int netif_index,
    unsigned short value)
{
	return nam_port_tpid_set(netif_index, value);
}

int npd_port_inner_tpid_set(
    unsigned int netif_index,
    unsigned short value)
{
	return nam_port_inner_tpid_set(netif_index, value);
}

struct port_driver_s eth_switchport_driver =
{
    .type = NPD_NETIF_ETH_TYPE,
    .set_pvid = npd_port_set_pvid,
    .set_inner_pvid = npd_port_set_inner_pvid,
    .allow_vlan = npd_port_allow_vlan,
    .remove_vlan = npd_port_free_vlan,
    .vlan_mode_set = npd_port_vlan_mode_set,
    .fdb_limit_set = npd_port_set_fdb_limit,
    .fdb_learning_mode = npd_port_fdb_learning_mode,
    .fdb_add = npd_port_fdb_add,
    .fdb_delete_by_port = npd_port_fdb_delete_by_port,
    .fdb_delete_by_vlan_port = npd_port_fdb_delete_by_vlan_port,
    .port_isolate_add = npd_port_isolate_add,
    .port_isolate_del = npd_port_isolate_del,
    .port_speed = npd_get_port_swspeed,
    .port_duplex_mode = npd_get_port_swduplex,
    .port_link_status = npd_get_port_link_status,
    .igmp_trap_set = npd_port_igmp_trap_set,
    .dhcp_trap_set = npd_port_dhcp_trap_set,
    .access_qinq_enable = npd_port_access_qinq_enable,
    .qinq_drop_miss_enable = npd_port_qinq_drop_miss,
 	.tpid_set = npd_port_tpid_set,
	.inner_tpid_set = npd_port_inner_tpid_set,   
    .mac_vlan_enable = npd_port_mac_vlan_enable,
    .subnet_vlan_enable = npd_port_subnet_vlan_enable,
    .prefer_subnet_enable = npd_port_prefer_subnet_enable,
    .port_vlan_filter = npd_port_vlan_filter,
#if 0
    /*stp*/
    int (*mstp_port_enable)(unsigned int g_ifindex);
    int (*mstp_port_disable)(unsigned int g_ifindex);
    int (*mstp_set_port_state)(unsigned int g_ifindex, int mstid, int state);

    /*igmp snooping*/
    int (*l2mc_entry_add_port)(void *l2_mc_entry);
    int (*l2mc_entry_del_port)(void *l2_mc_entry);

    /*qinq*/
    int (*dtag_mode_set)(unsigned int g_ifindex, int mode);
    
    /*layer 3 intf*/
#endif
	.mld_trap_set = npd_port_mld_trap_set,
};

int npd_dhcp_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_dhcp_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_dhcp_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_dhcp_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_dhcp_packet_trap_cpu(netif_index, enable);
    }
    return 0;
}

int npd_igmp_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_igmp_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_igmp_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_igmp_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_igmp_packet_trap_cpu(netif_index, enable);
    }
    return 0;
}

int npd_arp_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_arp_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_arp_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_arp_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_arp_packet_trap_cpu(netif_index, enable);
    }
    return 0;
}

int npd_bpdu_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_bpdu_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_bpdu_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_bpdu_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_bpdu_packet_trap_cpu(netif_index, enable);
    }
    return 0;
}

int npd_8021x_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_8021x_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_8021x_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_8021x_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_8021x_packet_trap_cpu(netif_index, enable);
    }
    return 0;

}

int npd_nd_packet_enable_netif(
    unsigned int netif_index,
    int enable)
{
    int type = npd_netif_type_get(netif_index);
    int ret;

    if(NPD_NETIF_VLAN_TYPE == type)
    {
        struct vlan_s vlan = {0};
        int array_port;
        unsigned int eth_g_index;

        vlan.vid = npd_netif_vlan_get_vid(netif_index);
        ret = dbtable_sequence_search(g_vlans, vlan.vid, &vlan);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(vlan.untag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_nd_packet_trap_cpu(eth_g_index, enable);
        }
        NPD_PBMP_ITER(vlan.tag_ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_nd_packet_trap_cpu(eth_g_index, enable);
        }
        
    }
    else if(NPD_NETIF_TRUNK_TYPE == type)
    {
        struct trunk_s trunk = {0};
        int array_port;
        unsigned int eth_g_index;

        trunk.trunk_id = npd_netif_trunk_get_tid(netif_index);
        ret = dbtable_sequence_search(g_trunks, trunk.trunk_id, &trunk);
        if(0 != ret)
            return 0;

        NPD_PBMP_ITER(trunk.ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            nam_nd_packet_trap_cpu(eth_g_index, enable);
        }
    }
    else if(NPD_NETIF_ETH_TYPE == type)
    {
        nam_nd_packet_trap_cpu(netif_index, enable);
    }
    return 0;
}


int npd_dhcp_packet_enable(
    int enable)
{
    return nam_dhcp_packet_trap_cpu_global(enable);
}


int npd_igmp_packet_enable(
    unsigned int netif_index,
    int enable)
{
    return nam_igmp_packet_trap_cpu_global(enable);
}


int npd_arp_packet_enable(
    unsigned int netif_index,
    int enable)
{
    return nam_arp_packet_trap_cpu_global(enable);

}

int npd_bpdu_packet_enable(
    unsigned int netif_index,
    int enable)
{
    return nam_bpdu_packet_trap_cpu_global(enable);

}

int npd_8021x_packet_enable(
    unsigned int netif_index,
    int enable)
{
    return nam_8021x_packet_trap_cpu_global(enable);
}

int npd_nd_packet_enable(
    unsigned int netif_index,
    int enable)
{
    return nam_nd_packet_trap_cpu_global(enable);
}


void* eth_port_rate_poll_thread(void)
{
    int j;
	int ret;
	unsigned int eth_index = 0;
	eth_port_stats_t portCnt;
	unsigned int rateInput,rateOutput;
	eth_port_stats_t *port_counter_info = NULL;
	unsigned int minute_roll = 0;
    static unsigned int eth_port_delay_event_count = 0;

    while(!npd_startup_end)
		sleep(1);
	
	memset(&portCnt, 0, sizeof(eth_port_stats_t));
	while(1)
	{
		eth_port_delay_event_count++;
		if(eth_port_delay_event_count%60 == 1 && (npd_eth_port_rate_poll_enable == 1))
		{
            for (j = 0; j < MAX_ETHPORT_PER_BOARD*MAX_SUBPORT_PER_ETHPORT; j++)
            {
                
                if(g_eth_port_rate_poll[j] == 0)
    				continue;
                eth_index = g_eth_port_rate_poll[j];
    		    ret = npd_get_eth_port_stat(eth_index, &portCnt);
    			if(ret == 0)
    			{
    			    port_counter_info = npd_get_port_counter_by_index(eth_index);
    			}
    			else
    			    continue;
    			
    			
    			g_ethport_input_bytes[j][minute_roll] = port_counter_info->snmp_stats.ibytes + port_counter_info->snmp_stats.etherinoerrors *12;
    			g_ethport_output_bytes[j][minute_roll] = port_counter_info->snmp_stats.obytes + port_counter_info->snmp_stats.etheronoerrors*12;
    			
    		    rateInput = (g_ethport_input_bytes[j][minute_roll]
    				 - g_ethport_input_bytes[j][(minute_roll+1)%6]) * 8 / (5*60*1000);/*5 minute kbps*/
    		    rateOutput = (g_ethport_output_bytes[j][minute_roll]
    				 - g_ethport_output_bytes[j][(minute_roll+1)%6]) * 8 / (5*60*1000);/*5 minute bps*/
    			if((rateInput < 0)||(rateOutput < 0))
    			{
    			    continue;
    			}
                g_eth_port_rateInput[j] = rateInput;
    			g_eth_port_rateOutput[j] = rateOutput;
    		}
    		minute_roll++;
    		minute_roll = minute_roll%6;
		}
		/*\B4\A6\C0\ED\B6?\DA\CA?\FE\B9\FD\BF\EC\B5\C4\D1?\D9\C9?\A8\B6?\DA\CA?\FE*/
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			if((eth_port_delay_event_count%5) == 0)
			{
			    npd_eth_port_handle_delay_event();
			}
		}
		sleep(1);
	}
}


#ifdef __cplusplus
}
#endif

