/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
* CREATOR:
*		zhengzw@autelan.com
*
* DESCRIPTION:
*		NPD definition for LACP module.
*
* DATE:
*		11/08/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $	
*******************************************************************************/
#ifdef HAVE_LACP
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_lacp.h"
#include "chassis_man_app.h"

extern int npd_packet_rx_socket_init(int type, int service_priority);
extern int nam_packet_tx_socket_init(int type, int service_priority, int (*tx_packet_handler)(int sock, char *buff, int len, void *private_data), int buff_len);
extern int npd_packet_rx_handle_register
(
int type, 
char *desc, 
int flags, 
int (*protocol_filter)(unsigned char  *packetBuff), 
int (*protocol_handler)(int packet_type,
                        unsigned char *packetBuffs, 
		                unsigned long buffLen, 
		                unsigned int netif_index,
		                unsigned int son_netif_index,
		                unsigned short vid,
		                unsigned char isTagged,
		                int flag
                        )
);
extern int eth_port_array_index_to_ifindex(unsigned int netif_index);
extern int eth_port_array_index_from_ifindex(unsigned int netif_index);
extern int nam_packet_tx_unicast_by_netif(int packet_type,unsigned int netif_index,unsigned int vid,unsigned int isTagged,unsigned char* data,unsigned int dataLen);
extern unsigned char *npd_packet_alloc(unsigned int size);
extern unsigned int npd_packet_free(unsigned char *data);
extern unsigned short npd_get_valid_trunk_id();
/* ================= ad code helper functions ==================*/
/*needed by dot3ad_rx_machine(...)*/
static void __record_pdu(lacpdu_t *lacpdu, lacp_port_actor_t *lacp_port_actor, lacp_port_partner_t *port, lacp_port_sm_t *lacp_port_sm);
static int __record_default(lacp_port_partner_t *port);
static void __update_selected(lacpdu_t *lacpdu, lacp_port_partner_t *port, lacp_port_sm_t *lacp_port_sm);
static int __update_default_selected(lacp_port_partner_t *port);
static void __choose_matched(lacpdu_t *lacpdu, lacp_port_actor_t *port, lacp_port_sm_t *lacp_port_sm);
static void __update_ntt(lacpdu_t *lacpdu, lacp_port_actor_t *port, lacp_port_sm_t *lacp_port_sm);

/*needed for dot3ad_mux_machine(..)*/
//static int __attach_bond_to_agg(unsigned int netif_index);
//static int __detach_bond_from_agg(unsigned int netif_index);
static int __agg_ports_are_ready(unsigned int netif_index);
static int __set_agg_ports_ready(unsigned int netif_index, int val);



/*needed for dot3ad_agg_selection_logic(...)*/
//static long __get_agg_bandwidth(unsigned int netif_index);

/* ================= main 802.3ad protocol functions ==================*/
static int dot3ad_mux_machine(unsigned int netif_index);
static int dot3ad_rx_machine(lacpdu_t *lacpdu, unsigned int netif_index);
static int dot3ad_tx_machine(unsigned int netif_index);
static int dot3ad_periodic_machine(unsigned int netif_index);
static int dot3ad_port_selection_logic(unsigned int netif_index);
static void dot3ad_clear_agg(aggregator_t *aggregator);
int npd_lacp_aggregator_init(unsigned int netif_index, aggregator_mode_t mode);
 int dot3ad_initialize_port(unsigned int netif_index, int lacp_fast, unsigned long lacp_mode, unsigned long enabled);
static void dot3ad_initialize_lacpdu(lacpdu_t *Lacpdu);
static int dot3ad_enable_collecting_distributing(unsigned int netif_index);
static int dot3ad_disable_collecting_distributing(unsigned int netif_index);
static int  dot3ad_rx_indication(unsigned short vlan_id, unsigned int netif_index, unsigned int son_netif_index, lacpdu_t *lacpdu, unsigned short length);
int dynamic_aggregator_leave(unsigned int trunk_netif_index, unsigned int eth_netif_index);
static int dot3ad_enable_lacp_port(unsigned int netif_index);
static int dot3ad_disable_lacp_port(unsigned int netif_index);
int npd_lacp_aggregator_delete(unsigned int netif_index);
int dynamic_aggregator_join(unsigned int trunk_netif_index, unsigned int eth_netif_index);
int dot3ad_state_machine_handler();
static int npd_lacp_enable_disable_config(unsigned int lacp_enable);
int npd_packet_rx_lacp(int packet_type,unsigned char *packetBuffs, unsigned long buffLen, unsigned int netif_index, unsigned int son_netif_index,
                               unsigned short vid,unsigned char isTagged,int flag);
int nam_packet_tx_lacp(unsigned int netif_index);
int static_aggregator_update(unsigned int trunk_netif_index);

static  mac_addr_t null_mac_addr = {{0, 0, 0, 0, 0, 0}};
static unsigned short dot3ad_ticks_per_sec = 1;

static unsigned int g_lacp_enable = 0;
static unsigned short aggregator_identifier;
unsigned int lacp_cfg_global_no = 0;

db_table_t         *npd_lacp_cfgtbl = NULL;
array_table_index_t *npd_lacp_cfg_index = NULL;

db_table_t* lacpport_actor_db = NULL;
sequence_table_index_t* lacpport_actor_index = NULL;

db_table_t* lacpport_partner_db = NULL;
sequence_table_index_t* lacpport_partner_index = NULL;

db_table_t* lacpport_sm_db = NULL;
sequence_table_index_t* lacpport_sm_index = NULL;

db_table_t* aggregator_db = NULL;
hash_table_index_t* aggregator_index = NULL;
//hash_table_index_t* aggregator_id = NULL;
sequence_table_index_t * aggregator_id = NULL;
pthread_mutex_t lacp_act_sync=PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

int eth_port_array_index_from_ifindex(unsigned int  netif_index);

void npd_lacp_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
);
void npd_lacp_handle_port_relate_event
(
    unsigned int trunk_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
);
netif_event_notifier_t lacp_port_notifier =
{
    .netif_event_handle_f = &npd_lacp_handle_port_event,
    .netif_relate_handle_f = &npd_lacp_handle_port_relate_event
};

unsigned int npd_lacpport_netif_index(unsigned int index)
{
    return eth_port_array_index_from_ifindex(index);
}
unsigned int npd_lacpport_actor_netif_index_key(void *port)
{
    return((lacp_port_actor_t*) port)->netif_index;
}

int npd_lacpport_actor_netif_index_cmp(void * port1, void *port2)
{
    return (((lacp_port_actor_t *) port1)->netif_index
            == ((lacp_port_actor_t *)port2)->netif_index);
}

unsigned int npd_lacpport_partner_netif_index_key(void *port)
{
    return((lacp_port_partner_t*) port)->netif_index;
}

int npd_lacpport_partner_netif_index_cmp(void * port1, void *port2)
{
    return (((lacp_port_partner_t *) port1)->netif_index
            == ((lacp_port_partner_t *)port2)->netif_index);
}

unsigned int npd_lacpport_sm_netif_index_key(void *port)
{
    return((lacp_port_sm_t*) port)->netif_index;
}

int npd_lacpport_sm_netif_index_cmp(void * port1, void *port2)
{
    return (((lacp_port_sm_t *) port1)->netif_index
            == ((lacp_port_sm_t *)port2)->netif_index);
}

unsigned int npd_aggregator_netif_index(void *data)
{
	aggregator_t *aggregator_v = (aggregator_t *)data;
	unsigned int netif_index = aggregator_v->netif_index;
	int hash_index = 0;
	hash_index = netif_index >> 28;
	hash_index = hash_index << 6;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
	return hash_index%MAX_ETH_GLOBAL_INDEX;
}

unsigned int npd_aggregator_cmp_netif_index(void *data_a, void *data_b)
{
	aggregator_t *aggregator_v_a = (aggregator_t *)data_a;
	aggregator_t *aggregator_v_b = (aggregator_t *)data_b;
	return (aggregator_v_a->netif_index == aggregator_v_b->netif_index);
}

unsigned int npd_aggregator_id_key(void *aggregator)
{
    return((aggregator_t*) aggregator)->aggregator_identifier;
}

int npd_aggregator_id_cmp(void * aggregator_a, void *aggregator_b)
{
    return (((aggregator_t *) aggregator_a)->aggregator_identifier
            == ((aggregator_t *)aggregator_b)->aggregator_identifier);
}

unsigned int npd_aggregator_id_index(unsigned int agg_id)
{
    return agg_id;
}

unsigned int npd_lacp_thread_main(void *dummy_data)
{ 
    int service_type = 7000;
    osal_register_timer(1, service_type, dot3ad_state_machine_handler,NULL,1);
    osal_thread_master_run(service_type);
    return 0;
}
int nam_packet_tx_lacp_tipc(int sock, char *buff, int len, void *private_data)
{
	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	unsigned int tx_len = 0;
	unsigned int netif_index = 0;
	unsigned int e_netif_index = 0;
	packet_sync_ctrl *lacpPktBuff = NULL;	
	NAM_PACKET_TX_SESSION *nam_packet_tx_session = (NAM_PACKET_TX_SESSION *)private_data;
	npd_syslog_dbg("LACP packet TX ...\r\n");
	if(len < 0)
	{
		if(nam_packet_tx_session)
		{
		    nam_packet_tx_session_dma_free(nam_packet_tx_session);
		}
		npd_syslog_err("packet len error\r\n");
		return -1;
	}
	if(nam_packet_tx_session == NULL)
	{
		npd_syslog_err("TX session NULL\r\n");
		return 0;
	}
	lacpPktBuff = (packet_sync_ctrl *)buff;
	data_buff = (buff + NPD_PKT_RESERVED_LEN);
	netif_index = lacpPktBuff->netif_index;
	vid = lacpPktBuff->vid;
	tx_len = len - NPD_PKT_RESERVED_LEN;
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
		rc = npd_trunk_master_port_get(npd_netif_trunk_get_tid(netif_index), &e_netif_index);
		if(rc == NPD_FALSE)
		{
    		npd_syslog_err("No master port of port-channel %d!\r\n",
    								npd_netif_trunk_get_tid(netif_index));
			return -1;
		}
    	rc = npd_vlan_check_contain_trunk(vid, npd_netif_trunk_get_tid(netif_index), &istagged);
    	if (rc == NPD_FALSE)
    	{
    		npd_syslog_err("Port-channel (netif index 0x%x) is not in VLAN %d!\r\n",
    								e_netif_index, vid);
    		return -1;
    	}
	}
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
	}
	else
	{
		npd_syslog_err("global index 0x%x is unknown netif type!\r\n",
    								netif_index);
		return -1;
	}
	npd_system_get_basemac(data_buff + 6, 6);
    nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_LACP_E, e_netif_index, 
				                           vid, istagged, 
				                           data_buff, tx_len);
	return 0;
}

void npd_lacp_init()
{
    int ret;
    struct npd_lacp_cfg_s npd_lacp_cfg_default;

    ret = create_dbtable( NPD_LACP_CFGTBL_NAME, 1, sizeof(struct npd_lacp_cfg_s),\
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL, 
					NULL,
					NULL, NULL, NULL,
					DB_SYNC_MCU,
					&npd_lacp_cfgtbl);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("Create npd lacp configuration table fail\n");
		return;
	}

    create_dbtable("lacpport_actor", MAX_ETH_GLOBAL_INDEX, sizeof(lacp_port_actor_t),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   NULL, NULL, NULL, 
                   NULL, NULL, DB_SYNC_ALL, &lacpport_actor_db);
    dbtable_create_sequence_index("netif_index", lacpport_actor_db, MAX_ETH_GLOBAL_INDEX, &npd_lacpport_netif_index, 
        &npd_lacpport_actor_netif_index_key, &npd_lacpport_actor_netif_index_cmp, &lacpport_actor_index);
	
    create_dbtable("lacpport_partner", MAX_ETH_GLOBAL_INDEX, sizeof(lacp_port_partner_t),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   NULL, NULL, NULL, 
                   NULL, NULL, DB_SYNC_NONE, &lacpport_partner_db);
    dbtable_create_sequence_index("netif_index", lacpport_partner_db, MAX_ETH_GLOBAL_INDEX, &npd_lacpport_netif_index, 
        &npd_lacpport_partner_netif_index_key, &npd_lacpport_partner_netif_index_cmp, &lacpport_partner_index);
	
    create_dbtable("lacpport_sm", MAX_ETH_GLOBAL_INDEX, sizeof(lacp_port_sm_t),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   NULL, NULL, NULL, 
                   NULL, NULL, DB_SYNC_NONE, &lacpport_sm_db);
    dbtable_create_sequence_index("netif_index", lacpport_sm_db, MAX_ETH_GLOBAL_INDEX, &npd_lacpport_netif_index, 
        &npd_lacpport_sm_netif_index_key, &npd_lacpport_sm_netif_index_cmp, &lacpport_sm_index);
	
    create_dbtable("aggregator", MAX_ETH_GLOBAL_INDEX, sizeof(aggregator_t),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   NULL, NULL, NULL, 
                   NULL, NULL, DB_SYNC_NONE, &aggregator_db);
    dbtable_create_hash_index("netif_index", aggregator_db, MAX_ETH_GLOBAL_INDEX, 
		&npd_aggregator_netif_index, &npd_aggregator_cmp_netif_index, &aggregator_index);
    dbtable_create_sequence_index("agg_id", aggregator_db, MAX_ETH_GLOBAL_INDEX, &npd_aggregator_id_index,  
        &npd_aggregator_id_key, &npd_aggregator_id_cmp, &aggregator_id);

    if(app_slot_work_mode_get())
    {
        npd_syslog_dbg("Create LACP thread\n");
	    nam_thread_create("lacp", npd_lacp_thread_main, NULL, NPD_TRUE, NPD_FALSE);
    }
    register_netif_notifier(&lacp_port_notifier);
    

	ret = dbtable_create_array_index("lacp_cfg", npd_lacp_cfgtbl, &npd_lacp_cfg_index);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("Create npd LACP configuration table index fail\n");
		return;
	}
	npd_lacp_cfg_default.lacp_enable = g_lacp_enable;
	ret = dbtable_array_insert(npd_lacp_cfg_index, &lacp_cfg_global_no, &npd_lacp_cfg_default);
	if(ret != 0)
	{
		syslog_ax_fdb_err("Insert lacp default configuration failed.\n");
		return;
	}
    npd_packet_rx_handle_register(NAM_PACKET_TYPE_LACP_E, "LACP", NAM_PACKET_RX_TO_MCU, npd_packet_type_is_lacp, npd_packet_rx_lacp);
    npd_packet_rx_socket_init(NAM_PACKET_TYPE_LACP_E, 38);
	nam_packet_tx_socket_init(NAM_PACKET_TYPE_LACP_E, 52, nam_packet_tx_lacp_tipc, NAM_TX_MAX_PACKET_LEN);
    return;
}
static unsigned short npd_aggregator_id_get(void)
{
    int ret = -1; 
    unsigned int Id = 0;
    unsigned short aggId = 0;
    aggregator_t *aggregator_p = NULL;
    
    aggregator_p = (aggregator_t *)malloc(sizeof(aggregator_t));
	if(NULL == aggregator_p)
	{
		goto error;
	}
    for(Id = 1; Id < AGGREGATOR_ID; Id++)
    {
        aggregator_p->aggregator_identifier = Id;    	
    	ret = dbtable_sequence_search(aggregator_id, Id, aggregator_p);
    	if(ret != 0)
    	{  
    	    if(Id <= aggregator_identifier)
            {   
                aggId = Id;
            }
            else
            {
                aggId = ++aggregator_identifier;
            }
            break;
    	}
        continue;
    }
    npd_syslog_dbg("assign a identifier %d to this new aggregator",aggId);
error:
   
    if(aggregator_p)
        free(aggregator_p);
	return aggId;
}
static unsigned short npd_aggregator_get_max_id(void)
{
    int ret = -1; 
    unsigned int Id = 0;
    unsigned short aggId = 0;
    aggregator_t *aggregator_p = NULL;
    
    aggregator_p = (aggregator_t *)malloc(sizeof(aggregator_t));
	if(NULL == aggregator_p)
	{
		goto error;
	}
    
    Id = aggregator_identifier;
    aggregator_p->aggregator_identifier = Id;    	
	ret = dbtable_sequence_search(aggregator_id, Id, aggregator_p);
	if(ret != 0)
	{  
	    if(Id == 0)
        {
            aggId = ++aggregator_identifier;
        }
        else
        {
            aggId = Id;
        }
	}
    else
    {
        aggId = ++aggregator_identifier;
    }
    
    npd_syslog_dbg("assign a identifier %d to this trunk\n",aggId);
error:
   
    if(aggregator_p)
        free(aggregator_p);
	return aggId;
}

#if 1
/**
 * __disable_port - disable the port's slave
 * @port: the port we're looking at
 *
 */
static inline void __disable_port(unsigned int netif_index)
{
	int ret = 0;
    ret = npd_set_port_admin_status(netif_index, 0);
    if(ret != 0)
    {
	    npd_syslog_dbg("lacp (port netif_index = 0X %d)set admin status failed\n",netif_index);
    }
}
/**
 * __enable_port - enable the port's slave, if it's up
 * @port: the port we're looking at
 *
 */
static inline int __enable_port(unsigned int netif_index)
{   
    int ret = -1;
    ret = npd_set_port_admin_status(netif_index, 1);
    if(ret != 0)
    {
	    npd_syslog_dbg("lacp set admin status failed\n");
    }
    return ret;
}  

#endif
/**
 * __port_is_enabled - check if the port's slave is in active state
 * @port: the port we're looking at
 *
 */
static inline int __port_is_enabled(unsigned int netif_index)
{
    int ret;
	unsigned long port_status;
	ret = nam_get_port_en_dis(netif_index,&port_status);
	if(ret != 0)
	{
		return FALSE;
	}
	if(port_status == 1)
	{
		return TRUE;
	}
    return FALSE;
}
#if  0
/**
 * __get_agg_selection_mode - get the aggregator selection mode
 * @port: the port we're looking at
 *
 * Get the aggregator selection mode. Can be %BANDWIDTH or %COUNT.
 */
static inline unsigned long __get_agg_selection_mode(unsigned int netif_index)
{

    TRUNK_P_DATA_S *bond = __get_bond_by_port(port);

    if (bond == NULL)
    {
        return AD_BANDWIDTH;
    }

    return BOND_AD_INFO(bond).agg_select_mode;

}
#endif 

/**
 * __get_rx_machine_lock - lock the port's RX machine
 * @port: the port we're looking at
 *
 */
static inline void __get_rx_machine_lock(unsigned int netif_index)
{

   // spin_lock(&(port->rx_machine_lock));
}

/**
 * __release_rx_machine_lock - unlock the port's RX machine
 * @port: the port we're looking at
 *
 */
static inline void __release_rx_machine_lock(unsigned int netif_index)
{
    //spin_unlock(&(port->rx_machine_lock));
}

/**
 * __get_link_speed - get a port's speed
 * @port: the port we're looking at
 *
 * Return @port's speed in 802.3ad bitmask format. i.e. one of:
 *     0,
 *     %AD_LINK_SPEED_BITMASK_10MBPS,
 *     %AD_LINK_SPEED_BITMASK_100MBPS,
 *     %AD_LINK_SPEED_BITMASK_1000MBPS,
 *     %AD_LINK_SPEED_BITMASK_10000MBPS
 */
static unsigned short __get_link_speed(unsigned int netif_index)
{
    unsigned short speed = 0;
    int ulBaud;

    ulBaud = eth_port_sw_speed_get(netif_index);

    switch (ulBaud)
    {
        case 10:
            speed = AD_LINK_SPEED_BITMASK_10MBPS;
            break;
        case 100:
            speed = AD_LINK_SPEED_BITMASK_100MBPS;
            break;
        case 1000:
            speed = AD_LINK_SPEED_BITMASK_1000MBPS;
            break;
        case 10000:
            speed = AD_LINK_SPEED_BITMASK_10000MBPS;
            break;
        default:
            speed = 0; /* unknown speed value from ethtool. shouldn't happen*/
            break;
    }
    npd_syslog_dbg("Port 0x%x Received link speed %d update from dapter\r\n", netif_index, speed);

    return speed;
}

/**
 * __get_duplex - get a port's duplex
 * @port: the port we're looking at
 *
 * Return @port's duplex in 802.3ad bitmask format. i.e.:
 *     0x01 if in full duplex
 *     0x00 otherwise
 */
static unsigned char __get_duplex(unsigned int netif_index)
{
    unsigned int duplex;
	unsigned char retVal = 0;

    duplex = eth_port_sw_duplex_get(netif_index);
    switch (duplex)
    {
        case ETH_ATTR_DUPLEX_FULL:
            retVal = 0x1;
			break;
        case ETH_ATTR_DUPLEX_HALF:
            retVal = 0x0;
			break;
        default:
            retVal = 0x0;
			break;
    }
    return retVal;
}

/*conversions*/
/**
 * __htons_lacpdu - convert the contents of a LACPDU to network byte order
 * @lacpdu: the speicifed lacpdu
 *
 * For each multi-byte field in the lacpdu, convert its content
 */
static void __htons_lacpdu(lacpdu_t *lacpdu)
{
    if (lacpdu)
    {
        lacpdu->actor_system_priority =   htons(lacpdu->actor_system_priority);
        lacpdu->actor_key =               htons(lacpdu->actor_key);
        lacpdu->actor_port_priority =     htons(lacpdu->actor_port_priority);
        lacpdu->actor_port =              htons(lacpdu->actor_port);
        lacpdu->partner_system_priority = htons(lacpdu->partner_system_priority);
        lacpdu->partner_key =             htons(lacpdu->partner_key);
        lacpdu->partner_port_priority =   htons(lacpdu->partner_port_priority);
        lacpdu->partner_port =            htons(lacpdu->partner_port);
        lacpdu->collector_max_delay =     htons(lacpdu->collector_max_delay);
    }
}

/**
 * __dot3ad_timer_to_ticks - convert a given timer type to AD module ticks
 * @timer_type:	which timer to operate
 * @par: timer parameter. see below
 *
 * If @timer_type is %current_while_timer, @par indicates long/short timer.
 * If @timer_type is %periodic_timer, @par is one of %FAST_PERIODIC_TIME,
 *						    %SLOW_PERIODIC_TIME.
 */
static unsigned short __dot3ad_timer_to_ticks(unsigned short timer_type, unsigned short par)
{
    unsigned short retval=0;	 /*to silence the compiler*/

    switch (timer_type)
    {
        case AD_CURRENT_WHILE_TIMER:   /* for rx machine usage*/

            if (par)  	      /* for short or long timeout*/
            {
                retval = (AD_SHORT_TIMEOUT_TIME*dot3ad_ticks_per_sec); /* short timeout*/
            }
            else
            {
                retval = (AD_LONG_TIMEOUT_TIME*dot3ad_ticks_per_sec); /* long timeout*/
            }

            break;
        case AD_ACTOR_CHURN_TIMER:	    /* for local churn machine*/
            retval = (AD_CHURN_DETECTION_TIME*dot3ad_ticks_per_sec);
            break;
        case AD_PERIODIC_TIMER:	    /* for periodic machine*/
            retval = (par*dot3ad_ticks_per_sec); /* long timeout*/
            break;
        case AD_PARTNER_CHURN_TIMER:   /* for remote churn machine*/
            retval = (AD_CHURN_DETECTION_TIME*dot3ad_ticks_per_sec);
            break;
        case AD_WAIT_WHILE_TIMER:	    /* for selection machine*/
            retval = (AD_AGGREGATE_WAIT_TIME*dot3ad_ticks_per_sec);
            break;
    }

    return retval;
}

/**
 * __record_pdu - record parameters from a received lacpdu
 * @lacpdu: the lacpdu we've received
 * @port: the port we're looking at
 *
 * Record the parameter values for the Actor carried in a received lacpdu as
 * the current partner operational parameter values and sets
 * actor_oper_port_state.defaulted to FALSE.
 */
static void __record_pdu(lacpdu_t *lacpdu, lacp_port_actor_t *lacp_port_actor, lacp_port_partner_t *lacp_port_partner, lacp_port_sm_t *lacp_port_sm)
{
    /* validate lacpdu and port*/
    if (lacpdu && lacp_port_partner && lacp_port_sm)
    {
        /* record the new parameter values for the partner operational*/
        lacp_port_partner->partner_oper_port_number = ntohs(lacpdu->actor_port);
        lacp_port_partner->partner_oper_port_priority = ntohs(lacpdu->actor_port_priority);
        lacp_port_partner->partner_oper_system = lacpdu->actor_system;
        lacp_port_partner->partner_oper_system_priority = ntohs(lacpdu->actor_system_priority);
        lacp_port_partner->partner_oper_key = ntohs(lacpdu->actor_key);
        /* zero partener's lase states*/
        lacp_port_partner->partner_oper_port_state = 0;
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_LACP_ACTIVITY);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_LACP_TIMEOUT);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_AGGREGATION);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_SYNCHRONIZATION);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_COLLECTING);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_DISTRIBUTING);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_DEFAULTED);
        lacp_port_partner->partner_oper_port_state |= (lacpdu->actor_state & AD_STATE_EXPIRED);
        /* set actor_oper_port_state.defaulted to FALSE*/
        lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DEFAULTED;
        /* set the partner sync. to on if the partner is sync. and the port is matched*/
        if ((lacp_port_sm->sm_vars & AD_PORT_MATCHED) && (lacpdu->actor_state & AD_STATE_SYNCHRONIZATION))
        {
            lacp_port_partner->partner_oper_port_state |= AD_STATE_SYNCHRONIZATION;
        }
        else
        {
            lacp_port_partner->partner_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;
        }
    }
}

/**
 * __record_default - record default parameters
 * @port: the port we're looking at
 *
 * This function records the default parameter values for the partner carried
 * in the Partner Admin parameters as the current partner operational parameter
 * values and sets actor_oper_port_state.defaulted to TRUE.
 */
static int __record_default(lacp_port_partner_t *lacp_port_partner)
{
    int state = 0;
    /* validate the port*/
    if (lacp_port_partner)
    {
        /* record the partner admin parameters*/
        lacp_port_partner->partner_oper_port_number = lacp_port_partner->partner_admin_port_number;
        lacp_port_partner->partner_oper_port_priority = lacp_port_partner->partner_admin_port_priority;
        lacp_port_partner->partner_oper_system = lacp_port_partner->partner_admin_system;
       // memcpy(&( lacp_port_partner->partner_oper_system.mac_addr_value), &(lacp_port_partner->partner_admin_system.mac_addr_value), 6);
        lacp_port_partner->partner_oper_system_priority = lacp_port_partner->partner_admin_system_priority;
        lacp_port_partner->partner_oper_key = lacp_port_partner->partner_admin_key;
        lacp_port_partner->partner_oper_port_state = lacp_port_partner->partner_admin_port_state;
        /* set actor_oper_port_state.defaulted to true*/
        state = 1;
        //port->actor_oper_port_state |= AD_STATE_DEFAULTED;
    }
    return state;
}

/**
 * __update_selected - update a port's Selected variable from a received lacpdu
 * @lacpdu: the lacpdu we've received
 * @port: the port we're looking at
 *
 * Update the value of the selected variable, using parameter values from a
 * newly received lacpdu. The parameter values for the Actor carried in the
 * received PDU are compared with the corresponding operational parameter
 * values for the ports partner. If one or more of the comparisons shows that
 * the value(s) received in the PDU differ from the current operational values,
 * then selected is set to FALSE and actor_oper_port_state.synchronization is
 * set to out_of_sync. Otherwise, selected remains unchanged.
 */
static void __update_selected(lacpdu_t *lacpdu, lacp_port_partner_t *lacp_port_partner, lacp_port_sm_t *lacp_port_sm)
{
    /* validate lacpdu and port*/
    if (lacpdu && lacp_port_partner && lacp_port_sm)
    {
        /* check if any parameter is different*/
        if ((ntohs(lacpdu->actor_port) != lacp_port_partner->partner_oper_port_number) ||
                (ntohs(lacpdu->actor_port_priority) != lacp_port_partner->partner_oper_port_priority) ||
                MAC_ADDRESS_COMPARE(&(lacpdu->actor_system), &(lacp_port_partner->partner_oper_system)) ||
                (ntohs(lacpdu->actor_system_priority) != lacp_port_partner->partner_oper_system_priority) ||
                (ntohs(lacpdu->actor_key) != lacp_port_partner->partner_oper_key) ||
                ((lacpdu->actor_state & AD_STATE_AGGREGATION) != (lacp_port_partner->partner_oper_port_state & AD_STATE_AGGREGATION))
           )
        {
            /* update the state machine Selected variable*/
            lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
        }
    }
}
/**
 * __update_default_selected - update a port's Selected variable from Partner
 * @port: the port we're looking at
 *
 * This function updates the value of the selected variable, using the partner
 * administrative parameter values. The administrative values are compared with
 * the corresponding operational parameter values for the partner. If one or
 * more of the comparisons shows that the administrative value(s) differ from
 * the current operational values, then Selected is set to FALSE and
 * actor_oper_port_state.synchronization is set to OUT_OF_SYNC. Otherwise,
 * Selected remains unchanged.
 */
static int __update_default_selected(lacp_port_partner_t *lacp_port_partner)
{
    int state = 0;
    /* validate the port*/
    if (lacp_port_partner)
    {   
        
        /* check if any parameter is different*/
        if ((lacp_port_partner->partner_admin_port_number != lacp_port_partner->partner_oper_port_number) ||
                (lacp_port_partner->partner_admin_port_priority != lacp_port_partner->partner_oper_port_priority) ||
                MAC_ADDRESS_COMPARE(&(lacp_port_partner->partner_admin_system), &(lacp_port_partner->partner_oper_system)) ||
                (lacp_port_partner->partner_admin_system_priority != lacp_port_partner->partner_oper_system_priority) ||
                (lacp_port_partner->partner_admin_key != lacp_port_partner->partner_oper_key) ||
                ((lacp_port_partner->partner_admin_port_state & AD_STATE_AGGREGATION) != (lacp_port_partner->partner_oper_port_state & AD_STATE_AGGREGATION))
           )
        {
            /* update the state machine Selected variable*/
            state = 1;
            //port->sm_vars &= ~AD_PORT_SELECTED;
        }
    }
    return state;
}

/**
 * __choose_matched - update a port's matched variable from a received lacpdu
 * @lacpdu: the lacpdu we've received
 * @port: the port we're looking at
 *
 * Update the value of the matched variable, using parameter values from a
 * newly received lacpdu. Parameter values for the partner carried in the
 * received PDU are compared with the corresponding operational parameter
 * values for the actor. Matched is set to TRUE if all of these parameters
 * match and the PDU parameter partner_state.aggregation has the same value as
 * actor_oper_port_state.aggregation and lacp will actively maintain the link
 * in the aggregation. Matched is also set to TRUE if the value of
 * actor_state.aggregation in the received PDU is set to FALSE, i.e., indicates
 * an individual link and lacp will actively maintain the link. Otherwise,
 * matched is set to FALSE. LACP is considered to be actively maintaining the
 * link if either the PDU's actor_state.lacp_activity variable is TRUE or both
 * the actor's actor_oper_port_state.lacp_activity and the PDU's
 * partner_state.lacp_activity variables are TRUE.
 */
static void __choose_matched(lacpdu_t *lacpdu, lacp_port_actor_t *lacp_port_actor, lacp_port_sm_t *lacp_port_sm)
{
    /* validate lacpdu and port*/
    if (lacpdu && lacp_port_actor && lacp_port_sm)
    {
        /* check if all parameters are alike*/
        if (((ntohs(lacpdu->partner_port) == lacp_port_actor->actor_port_number) &&
                (ntohs(lacpdu->partner_port_priority) == lacp_port_actor->actor_port_priority) &&
                !MAC_ADDRESS_COMPARE(&(lacpdu->partner_system), &(lacp_port_actor->actor_system)) &&
                (ntohs(lacpdu->partner_system_priority) == lacp_port_actor->actor_system_priority) &&
                (ntohs(lacpdu->partner_key) == lacp_port_actor->actor_oper_port_key) &&
                ((lacpdu->partner_state & AD_STATE_AGGREGATION) == (lacp_port_actor->actor_oper_port_state & AD_STATE_AGGREGATION))) ||
                /* or this is individual link(aggregation == FALSE)*/
                ((lacpdu->actor_state & AD_STATE_AGGREGATION) == 0)
           )
        {
            /* update the state machine Matched variable*/
            lacp_port_sm->sm_vars |= AD_PORT_MATCHED;
        }
        else
        {
        }
    }
}

/**
 * __update_ntt - update a port's ntt variable from a received lacpdu
 * @lacpdu: the lacpdu we've received
 * @port: the port we're looking at
 *
 * Updates the value of the ntt variable, using parameter values from a newly
 * received lacpdu. The parameter values for the partner carried in the
 * received PDU are compared with the corresponding operational parameter
 * values for the Actor. If one or more of the comparisons shows that the
 * value(s) received in the PDU differ from the current operational values,
 * then ntt is set to TRUE. Otherwise, ntt remains unchanged.
 */
static void __update_ntt(lacpdu_t *lacpdu, lacp_port_actor_t *lacp_port_actor, lacp_port_sm_t *lacp_port_sm)
{
    /* validate lacpdu and port*/
    if (lacpdu && lacp_port_actor && lacp_port_sm)
    {
        /* check if any parameter is different*/
        if ((ntohs(lacpdu->partner_port) != lacp_port_actor->actor_port_number) ||
                (ntohs(lacpdu->partner_port_priority) != lacp_port_actor->actor_port_priority) ||
                MAC_ADDRESS_COMPARE(&(lacpdu->partner_system), &(lacp_port_actor->actor_system)) ||
                (ntohs(lacpdu->partner_system_priority) != lacp_port_actor->actor_system_priority) ||
                (ntohs(lacpdu->partner_key) != lacp_port_actor->actor_oper_port_key) ||
                ((lacpdu->partner_state & AD_STATE_LACP_ACTIVITY) != (lacp_port_actor->actor_oper_port_state & AD_STATE_LACP_ACTIVITY)) ||
                ((lacpdu->partner_state & AD_STATE_LACP_TIMEOUT) != (lacp_port_actor->actor_oper_port_state & AD_STATE_LACP_TIMEOUT)) ||
                ((lacpdu->partner_state & AD_STATE_SYNCHRONIZATION) != (lacp_port_actor->actor_oper_port_state & AD_STATE_SYNCHRONIZATION)) ||
                ((lacpdu->partner_state & AD_STATE_AGGREGATION) != (lacp_port_actor->actor_oper_port_state & AD_STATE_AGGREGATION))
           )
        {
            /* set ntt to be TRUE*/
            lacp_port_sm->ntt = TRUE;
        }
    }
}
static int __agg_ports_are_ready(unsigned int trunk_netif_index)
{
   int ret = -1;
   int retval = 1;
   unsigned int trunkId = 0;
   unsigned int array_port = 0;
   unsigned int eth_g_index = 0;
   struct trunk_s *trunkNode = NULL;
   lacp_port_sm_t *lacp_port_sm = NULL;

   trunkNode = malloc(sizeof(struct trunk_s));
   if(NULL == trunkNode)
   {
       retval = -1;
       goto error;
   }
   lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
   if(NULL == lacp_port_sm)
   {
       retval = -1;
       goto error;
   }

   if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(trunk_netif_index))
    {
        trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
        ret = dbtable_sequence_search(g_trunks, trunkId, trunkNode);
        {  
            retval = -1;
            goto error;
        }
        NPD_PBMP_ITER(trunkNode->ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            lacp_port_sm->netif_index = eth_g_index;
            memset(lacp_port_sm, 0, sizeof(lacp_port_sm_t));
            ret = dbtable_sequence_search(lacpport_sm_index, eth_g_index, lacp_port_sm);
            if(ret != 0)
            {  
                retval = -1;
                goto error;
            }
            if (!(lacp_port_sm->sm_vars & AD_PORT_READY_N))
            {
                retval = 0;
                goto error;;
            }
           
        }
    }
error:
    
    if(trunkNode)
        free(trunkNode);
    if(lacp_port_sm)
        free(lacp_port_sm);
   
    return retval;
}


/**
 * __set_agg_ports_ready - set value of Ready bit in all ports of an aggregator
 * @aggregator: the aggregator we're looking at
 * @val: Should the ports' ready bit be set on or off
 *
 */
static int __set_agg_ports_ready(unsigned int trunk_netif_index, int val)
{
   int ret = -1;
   unsigned int trunkId = 0;
   unsigned int array_port = 0;
   unsigned int eth_g_index = 0;
   struct trunk_s *trunkNode = NULL;
   lacp_port_sm_t *lacp_port_sm = NULL;
   
   trunkNode = malloc(sizeof(struct trunk_s));
   if(NULL == trunkNode)
   {
       goto error;
   }
   lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
   if(lacp_port_sm == NULL)
   {
       goto error;
   }
   if(val == -1)
   {
       goto error;
   }
   
   if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(trunk_netif_index))
    {
        trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
        ret = dbtable_sequence_search(g_trunks, trunkId, trunkNode);

        NPD_PBMP_ITER(trunkNode->ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            lacp_port_sm->netif_index = eth_g_index;
            memset(lacp_port_sm, 0, sizeof(lacp_port_sm_t));
            ret = dbtable_sequence_search(lacpport_sm_index, eth_g_index, lacp_port_sm);
            if(ret != 0)
            {  
                goto error;
            }
            if (val)
            {
                lacp_port_sm->sm_vars |= AD_PORT_READY;
            }
            else
            {
                lacp_port_sm->sm_vars &= ~AD_PORT_READY;
            }
           ret = dbtable_sequence_update(lacpport_sm_index, eth_g_index, lacp_port_sm, lacp_port_sm); 
           
        }
    }
error:

    if(trunkNode)
        free(trunkNode);
    if(lacp_port_sm)
        free(lacp_port_sm);
    return ret;
}

/**
 * __get_agg_bandwidth - get the total bandwidth of an aggregator
 * @aggregator: the aggregator we're looking at
 *
 */
static  long __get_agg_bandwidth(unsigned int trunk_netif_index)
{
    long ret = -1;
    long bandwidth = -1;
    unsigned long basic_speed = 0;
    aggregator_t   *aggregator = NULL;

    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
        goto error;
	}
    
    aggregator->netif_index = trunk_netif_index;
	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
	if(ret != 0)
	{  
	   goto error;
	}

    if (aggregator->num_of_ports)
    {
        basic_speed = aggregator->actor_oper_aggregator_key & 0x3E;

        switch (basic_speed)
        {
            case AD_LINK_SPEED_BITMASK_1MBPS:
                bandwidth = aggregator->num_of_ports;
                break;
            case AD_LINK_SPEED_BITMASK_10MBPS:
                bandwidth = aggregator->num_of_ports * 10;
                break;
            case AD_LINK_SPEED_BITMASK_100MBPS:
                bandwidth = aggregator->num_of_ports * 100;
                break;
            case AD_LINK_SPEED_BITMASK_1000MBPS:
                bandwidth = aggregator->num_of_ports * 1000;
                break;
            case AD_LINK_SPEED_BITMASK_10000MBPS:
                bandwidth = aggregator->num_of_ports * 10000;
                break;
            default:
                bandwidth=0; /* to silent the compilor ....*/
        }
    }

    
error:
    
   if(aggregator)
       free(aggregator);
   return bandwidth;
}

/**
 * __update_lacpdu_from_port - update a port's lacpdu fields
 * @port: the port we're looking at
 *
 */
static int __update_lacpdu_from_port(unsigned int netif_index, lacpdu_t *lacpdu)
{
	int ret = -1;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{
		goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
		goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
		goto error;
	}
	lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{
		goto error;
	}

    dot3ad_initialize_lacpdu(lacpdu);

    lacpdu->actor_system_priority = lacp_port_actor->actor_system_priority;
    lacpdu->actor_system = lacp_port_actor->actor_system;
    lacpdu->actor_key = lacp_port_actor->actor_oper_port_key;
    lacpdu->actor_port_priority = lacp_port_actor->actor_port_priority;
    lacpdu->actor_port = lacp_port_actor->actor_port_number;
    lacpdu->actor_state = lacp_port_actor->actor_oper_port_state;
    /* lacpdu->reserved_3_1              initialized
     * lacpdu->tlv_type_partner_info     initialized
     * lacpdu->partner_information_length initialized
     */
    lacpdu->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
    lacpdu->partner_system = lacp_port_partner->partner_oper_system;
    lacpdu->partner_key = lacp_port_partner->partner_oper_key;
    lacpdu->partner_port_priority = lacp_port_partner->partner_oper_port_priority;
    lacpdu->partner_port = lacp_port_partner->partner_oper_port_number;
    lacpdu->partner_state = lacp_port_partner->partner_oper_port_state;
    /* Convert all non unsigned char parameters to Big Endian for transmit */
    __htons_lacpdu(lacpdu);
error:
    
    if(lacp_port_actor)
	    free(lacp_port_actor);
    if(lacp_port_partner)
	    free(lacp_port_partner);
	return ret;
}
int npd_packet_rx_lacp
(
int packet_type,
unsigned char *packetBuffs, 
unsigned long buffLen,
unsigned int netif_index,
unsigned int son_netif_index,
unsigned short vid,
unsigned char isTagged,
int flag
)
{
	lacpdu_header_t *rxPacket = (lacpdu_header_t *)(packetBuffs+NPD_PKT_RESERVED_LEN);
	if(son_netif_index == 0)
	{
        npd_syslog_dbg("LACP packet from 0x%x\r\n", netif_index);
	}
	else
	{
		npd_syslog_dbg("LACP packet from LAG netif 0x%x (ethernet netif 0x%x)\r\n", netif_index, son_netif_index);
	}
    pthread_mutex_lock(&lacp_act_sync);
    dot3ad_rx_indication(vid, netif_index, son_netif_index, &rxPacket->lacpdu, buffLen);
    pthread_mutex_unlock(&lacp_act_sync);

	return NAM_PACKET_RX_COMPLETE;
}
int nam_packet_tx_lacp(unsigned int netif_index)
{

	int ret = -1;
	unsigned int rc = 0;
	unsigned char *data_buff = NULL;
	unsigned char istagged = 0;
	unsigned short vid = 0;
	lacpdu_header_t *lacpdu = NULL;
	unsigned int e_netif_index = 0;
    unsigned int trunk_index = 0;
    unsigned int trunkId = 0;
    mac_addr_t lacpdu_multicast_address = AD_MULTICAST_LACPDU_ADDR;
	npd_syslog_dbg("lacp packet TX ...\r\n");
	lacpdu = (lacpdu_header_t *)npd_packet_alloc(sizeof(lacpdu_header_t));
	if(lacpdu == NULL)
	{
		goto error;
	}
    lacpdu->dot3ad_header.destination_address = lacpdu_multicast_address;
	npd_system_get_basemac(lacpdu->dot3ad_header.source_address.mac_addr_value, 6);
    //lacpdu->dot3ad_header.dot1q_tag = 0x81000201;/*fixme*/
    lacpdu->dot3ad_header.length_type = PKT_TYPE_LACPDU;
    ret = __update_lacpdu_from_port(netif_index , &lacpdu->lacpdu);
	if(ret != 0)
	{
		goto error;
	}
	e_netif_index = netif_index;
	if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
	{
		rc = npd_vlan_port_pvid_get(netif_index, &vid);
		if(rc != 0)
		{    
		    struct eth_port_s portInfo ;
            portInfo.eth_port_ifindex = netif_index;
            ret = dbtable_sequence_search(g_eth_ports, netif_index, &portInfo);
            if (0 != ret)
            {
                ret = -1;
        	    goto error;
            }
            trunkId = portInfo.trunkid;
            trunk_index = 0x1<<30;
            trunk_index |= trunkId<<16;
		    if(npd_netif_type_get(trunk_index) == NPD_NETIF_TRUNK_TYPE)
        	{
        		rc = npd_vlan_port_pvid_get(trunk_index, &vid);
        		if(rc != 0)
        		{
        			npd_syslog_dbg("Ethernet port (netif index 0x%x) is not in VLAN %d!\r\n",netif_index, vid);
                    ret = -1;
        			goto error;
        		}
        	}
            else
            {
    			npd_syslog_dbg("Ethernet port (netif index 0x%x) is not in VLAN %d!\r\n",netif_index, vid);
                ret = -1;
    			goto error;
            }
		}
	}
	else
	{
		npd_syslog_dbg("Only ethernet netif support LACP packet TX!\r\n");
        ret = -1;
		goto error;
	}
	data_buff = (unsigned char *)lacpdu;
	nam_packet_tx_unicast_by_netif(NAM_PACKET_TYPE_LACP_E, e_netif_index, vid, istagged, data_buff, sizeof(lacpdu_header_t));
error:
    
    if(lacpdu)
        npd_packet_free((unsigned char*)lacpdu);
	return ret;
}
 int dot3a_band_aggregator_to_trunk(unsigned int netif_index_a,unsigned int netif_index_b)
{   
    int ret = NPD_FAIL;
    unsigned short trunkId = 0;
    char* trunkName = NULL;
    lacp_port_actor_t *lacp_port_actor_a = NULL;
    lacp_port_actor_t *lacp_port_actor_b = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;
    lacp_port_sm_t  *lacp_port_sm_a = NULL;
    lacp_port_sm_t  *lacp_port_sm_b = NULL;
    aggregator_t    *agg_dynamic = NULL;
    struct trunk_s*	trunkNode = malloc(sizeof(struct trunk_s));
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
    lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
    lacp_port_actor_a = malloc(sizeof(lacp_port_actor_t));
    lacp_port_actor_b = malloc(sizeof(lacp_port_actor_t));
    lacp_port_sm_a = malloc(sizeof(lacp_port_sm_t));
    lacp_port_sm_b = malloc(sizeof(lacp_port_sm_t));
    agg_dynamic  = malloc(sizeof(aggregator_t));
    if((NULL == trunkName) || (NULL == trunkNode) || (lacp_port_partner == NULL) || (lacp_port_actor_a == NULL) ||
        (lacp_port_actor_b == NULL) || (lacp_port_sm_a == NULL) || (lacp_port_sm_b ==  NULL) || (agg_dynamic == NULL))
    {
        goto error;
    }
    lacp_port_actor_a->netif_index = netif_index_a;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index_a, lacp_port_actor_a);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_actor_b->netif_index = netif_index_b;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index_b, 	lacp_port_actor_b);
	if(ret != 0)
	{  
        goto error;
	}
    npd_syslog_dbg("netif_index_a = 0X%x netif_index_b = 0X%x", netif_index_a, netif_index_b);
	memset(trunkName,0,ALIAS_NAME_SIZE);
    trunkId = npd_get_valid_trunk_id();
    
    sprintf(trunkName, "port_channel%.3d", trunkId);
    ret = npd_find_trunk_by_name(trunkName, trunkNode);
    if(0 == ret) 
    {
        int iter = 0;
	    for(iter = 1;iter < NPD_TRUNK_NUMBER_MAX; iter++) 
        {   
            memset(trunkName, 0, ALIAS_NAME_SIZE);
            memset(trunkNode, 0, sizeof(struct trunk_s));
            sprintf(trunkName, "port_channel%.3d", iter);
            trunkNode->trunk_id = iter;
    		ret = dbtable_sequence_search(g_trunks, iter, trunkNode);
    		if(0 != ret)
            {   
    			continue;	/*trunk null*/
            }
    		else if(strcmp(trunkName, trunkNode->name)) 
            {           
                break;	    /*trunk name  not match*/
    		}
    		else 
            {
                if(iter == NPD_TRUNKID_END)
                {
                    goto error;
                }
                continue;	/*trunk name match*/
    		}
	    }
    }
	npd_key_database_lock();
    ret = npd_trunk_activate(trunkId, trunkName);
    if(0 != ret) 
    {
        goto error;
    }
    memset(trunkNode, 0, sizeof(struct trunk_s));
    ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }
    trunkNode->aggregator_mode = DYNAMIC_MODE;
    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
    
    ret = npd_trunk_port_add(trunkId, netif_index_b);
    if(0 != ret)
    {
        goto error;
    }
    ret = npd_trunk_port_add(trunkId, netif_index_a);
    if(0 != ret)
    {
        goto error;
    }

    ret = npd_lacp_aggregator_init(trunkNode->g_index, DYNAMIC_MODE);
    agg_dynamic->netif_index = trunkNode->g_index;
	ret = dbtable_hash_search(aggregator_index, agg_dynamic, NULL, agg_dynamic);
	if(ret != 0)
	{  
        goto error;
    }
    
    memset(trunkNode, 0,  sizeof(struct trunk_s));
    ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        ret = NPD_FAIL;
        goto error;
    }
    
    ret = npd_lacp_aggregator_delete(netif_index_b);
    if(0 != ret) 
    {
        goto error;
    }
    
    ret = npd_lacp_aggregator_delete(netif_index_a);
    if(0 != ret) 
    {
        goto error;
    }
    //更新aggregator及lacp_port_actor、lacp_port_sm
    lacp_port_actor_a->netif_index = netif_index_a;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index_a, lacp_port_actor_a);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_actor_b->netif_index = netif_index_b;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index_b, 	lacp_port_actor_b);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_partner->netif_index = netif_index_a;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index_a, lacp_port_partner);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_sm_a->netif_index = netif_index_a;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index_a, lacp_port_sm_a);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_sm_b->netif_index = netif_index_b;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index_b, lacp_port_sm_b);
	if(ret != 0)
	{  
        goto error;
	}
    agg_dynamic->netif_index = trunkNode->g_index;
	ret = dbtable_hash_search(aggregator_index, agg_dynamic, NULL, agg_dynamic);
	if(ret != 0)
	{  
        goto error;
    }
    
    agg_dynamic->num_of_ports++;
    agg_dynamic->num_of_ports++;
    agg_dynamic->actor_admin_aggregator_key = lacp_port_actor_a->actor_admin_port_key;
    agg_dynamic->actor_oper_aggregator_key = lacp_port_actor_a->actor_oper_port_key;
    agg_dynamic->partner_system = lacp_port_partner->partner_oper_system;
    agg_dynamic->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
    agg_dynamic->partner_oper_aggregator_key = lacp_port_partner->partner_oper_key;
    agg_dynamic->receive_state = 1;
    agg_dynamic->transmit_state = 1;
    
    lacp_port_actor_a->actor_oper_port_state |= (AD_STATE_COLLECTING | AD_STATE_DISTRIBUTING | AD_STATE_AGGREGATION  | AD_STATE_SYNCHRONIZATION | AD_STATE_LACP_ACTIVITY);
   	lacp_port_actor_a->actor_port_aggregator_identifier = agg_dynamic->aggregator_identifier;
    lacp_port_actor_a->is_lacp_enable = TRUE;
    
    lacp_port_actor_a->trunk_netif_index = trunkNode->g_index;
    lacp_port_sm_a->sm_tx_timer_counter = dot3ad_ticks_per_sec/AD_MAX_TX_IN_SECOND;
    lacp_port_sm_a->sm_vars |= AD_PORT_LACP_ENABLED | AD_PORT_SELECTED;
    
    lacp_port_actor_b->actor_oper_port_state |= (AD_STATE_COLLECTING | AD_STATE_DISTRIBUTING | AD_STATE_AGGREGATION  | AD_STATE_SYNCHRONIZATION | AD_STATE_LACP_ACTIVITY);
   	lacp_port_actor_b->actor_port_aggregator_identifier = agg_dynamic->aggregator_identifier;
    lacp_port_actor_b->is_lacp_enable = TRUE;
    lacp_port_actor_b->trunk_netif_index = trunkNode->g_index;
    
    lacp_port_sm_b->sm_tx_timer_counter = dot3ad_ticks_per_sec/AD_MAX_TX_IN_SECOND;
    lacp_port_sm_b->sm_vars |= AD_PORT_LACP_ENABLED | AD_PORT_SELECTED;
    //
    lacp_port_sm_b->sm_rx_timer_counter++;
 
    dbtable_hash_update(aggregator_index, agg_dynamic, agg_dynamic);
    dbtable_sequence_update(lacpport_actor_index, netif_index_a, lacp_port_actor_a, lacp_port_actor_a);
    dbtable_sequence_update(lacpport_actor_index, netif_index_b, lacp_port_actor_b, lacp_port_actor_b);
    dbtable_sequence_update(lacpport_sm_index, netif_index_a, lacp_port_sm_a, lacp_port_sm_a);
    dbtable_sequence_update(lacpport_sm_index, netif_index_b, lacp_port_sm_b, lacp_port_sm_b);

error:
    npd_key_database_unlock();
    if(trunkNode)
        free(trunkNode);
    if(trunkName)
        free(trunkName);
    if(agg_dynamic)
        free(agg_dynamic);
    if(lacp_port_actor_a)
        free(lacp_port_actor_a);
    if(lacp_port_actor_b)
        free(lacp_port_actor_b);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(lacp_port_sm_a)
        free(lacp_port_sm_a);
    if(lacp_port_sm_b)
        free(lacp_port_sm_b);
    return ret;  
}
#if 0
/**
 * dot3ad_mux_machine - handle a port's mux state machine
 * @port: the port we're looking at
 *
 */
static int dot3ad_mux_machine(unsigned int netif_index)
{
    int ret = -1;
    mux_states_t last_state;
    aggregator_t    *aggregator = NULL;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;
    
    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
        goto error;
	}
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}

    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
    aggregator->netif_index = lacp_port_actor->trunk_netif_index;
    ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);	
    if(ret != 0)
	{  
        goto error;;
	}

    /* keep current State Machine state to compare later if it was changed*/
    last_state = lacp_port_sm->sm_mux_state;

    if (lacp_port_sm->sm_vars & AD_PORT_BEGIN)
    {
        lacp_port_sm->sm_mux_state = AD_MUX_DETACHED;		 /* next state*/
    }
    else
    {
        switch (lacp_port_sm->sm_mux_state)
        {
            case AD_MUX_DETACHED:

                if ((lacp_port_sm->sm_vars & AD_PORT_SELECTED) || (lacp_port_sm->sm_vars & AD_PORT_STANDBY))   /* if SELECTED or STANDBY*/
                {
                    lacp_port_sm->sm_mux_state = AD_MUX_WAITING; /* next state*/
                }

                break;
            case AD_MUX_WAITING:

                /* if SELECTED == FALSE return to DETACH state*/
                if (!(lacp_port_sm->sm_vars & AD_PORT_SELECTED))   /* if UNSELECTED*/
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_READY_N;
                    /* in order to withhold the Selection Logic to check all ports READY_N value*/
                    /* every callback cycle to update ready variable, we check READY_N and update READY here*/
                    __set_agg_ports_ready(aggregator->netif_index, __agg_ports_are_ready(aggregator->netif_index));
                    lacp_port_sm->sm_mux_state = AD_MUX_DETACHED;	 /* next state*/
                    break;
                }

                /* check if the wait_while_timer expired*/
                if (lacp_port_sm->sm_mux_timer_counter && !(--lacp_port_sm->sm_mux_timer_counter))
                {
                    lacp_port_sm->sm_vars |= AD_PORT_READY_N;
                }

                /* in order to withhold the selection logic to check all ports READY_N value*/
                /* every callback cycle to update ready variable, we check READY_N and update READY here*/
                __set_agg_ports_ready(aggregator->netif_index, __agg_ports_are_ready(aggregator->netif_index));

                /* if the wait_while_timer expired, and the port is in READY state, move to ATTACHED state*/
                if ((lacp_port_sm->sm_vars & AD_PORT_READY) && !lacp_port_sm->sm_mux_timer_counter)
                {
                    lacp_port_sm->sm_mux_state = AD_MUX_ATTACHED;	 /* next state*/
                }

                break;
            case AD_MUX_ATTACHED:

                /* check also if agg_select_timer expired(so the edable port will take place only after this timer)*/
                if ((lacp_port_sm->sm_vars & AD_PORT_SELECTED) && (lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION))
                {
                    lacp_port_sm->sm_mux_state = AD_MUX_COLLECTING_DISTRIBUTING;/* next state*/
                }
                else if (!(lacp_port_sm->sm_vars & AD_PORT_SELECTED) || (lacp_port_sm->sm_vars & AD_PORT_STANDBY))  	  /* if UNSELECTED or STANDBY*/
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_READY_N;
                    /* in order to withhold the selection logic to check all ports READY_N value*/
                    /* every callback cycle to update ready variable, we check READY_N and update READY here*/
                    __set_agg_ports_ready(aggregator->netif_index, __agg_ports_are_ready(aggregator->netif_index));
                    lacp_port_sm->sm_mux_state = AD_MUX_DETACHED;/* next state*/
                }

                break;
            case AD_MUX_COLLECTING_DISTRIBUTING:

                if (!(lacp_port_sm->sm_vars & AD_PORT_SELECTED) || (lacp_port_sm->sm_vars & AD_PORT_STANDBY) ||
                        !(lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION)
                   )
                {
                    lacp_port_sm->sm_mux_state = AD_MUX_ATTACHED;/* next state*/
                }
                else
                {
                    /* if port state hasn't changed make*/
                    /* sure that a collecting distributing*/
                    /* port in an active aggregator is enabled*/
                }

                break;
            default:    /*to silence the compiler*/
                break;
        }
    }

    /* check if the state machine was changed*/
    if (lacp_port_sm->sm_mux_state != last_state)
    {
        switch (lacp_port_sm->sm_mux_state)
        {
            case AD_MUX_DETACHED:

                dynamic_aggregator_leave(netif_index, netif_index);
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;
                dot3ad_disable_collecting_distributing(netif_index);
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
                lacp_port_sm->ntt = TRUE;
                dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
                break;
            case AD_MUX_WAITING:
                lacp_port_sm->sm_mux_timer_counter = __dot3ad_timer_to_ticks(AD_WAIT_WHILE_TIMER, 0);
                break;
            case AD_MUX_ATTACHED:

                lacp_port_actor->actor_oper_port_state |= AD_STATE_SYNCHRONIZATION;
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
                dot3ad_disable_collecting_distributing(netif_index);
                lacp_port_sm->ntt = TRUE;
                dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
                break;
            case AD_MUX_COLLECTING_DISTRIBUTING:
                lacp_port_actor->actor_oper_port_state |= AD_STATE_COLLECTING;
                lacp_port_actor->actor_oper_port_state |= AD_STATE_DISTRIBUTING;
                dot3ad_enable_collecting_distributing(netif_index);
                lacp_port_sm->ntt = TRUE;
                dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
                break;
            default:    /*to silence the compiler*/
                break;
        }
    }

    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    //dbtable_sequence_update(lacpport_actor_index, netif_index, NULL, lacp_port_actor);
    
error:
    
    if(aggregator)
        free(aggregator);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    return ret;
}
#endif
/**
 * dot3ad_rx_machine - handle a port's rx State Machine
 * @lacpdu: the lacpdu we've received
 * @port: the port we're looking at
 *
 
 * If lacpdu arrived, stop previous timer (if exists) and set the next state as
 * CURRENT. If timer expired set the state machine in the proper state.
 * In other cases, this function checks if we need to switch to other state.
 */
static int dot3ad_rx_machine(lacpdu_t *lacpdu,unsigned int netif_index)
{
    int ret = -1;
    unsigned int aggregator_en = 0;
    unsigned char last_partner_syc = 0;
    rx_states_t   last_state;
    aggregator_t   *aggregator = NULL;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;

    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
        goto error;
	}
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
 
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
	lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{
        goto error;
	}
    //aggregator->netif_index = lacp_port_actor->trunk_netif_index;
    aggregator->netif_index = netif_index;
	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
	if(ret == 0)
	{  
	    aggregator_en = 1;
	}
    last_partner_syc = (lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION);

	npd_syslog_dbg("1> Rx sm: 0x%x, timer counter: %d.\r\n", lacp_port_sm->sm_rx_state, lacp_port_sm->sm_rx_timer_counter);
    /* Lock to prevent 2 instances of this function to run simultaneously(rx interrupt and periodic machine callback)*/
    __get_rx_machine_lock(netif_index);
    /* keep current State Machine state to compare later if it was changed*/
    last_state = lacp_port_sm->sm_rx_state;
    /* check if state machine should change state*/
    /* first, check if port was reinitialized*/
    if (lacp_port_sm->sm_vars & AD_PORT_BEGIN)
    {
       npd_syslog_dbg("AD_PORT_BEGIN");
       lacp_port_sm->sm_rx_state = AD_RX_INITIALIZE;		    /* next state*/
    }
    /* check if port is not enabled ???    */
    else if (!(lacp_port_sm->sm_vars & AD_PORT_BEGIN) && !lacp_port_actor->is_enabled && !(lacp_port_sm->sm_vars & AD_PORT_MOVED))
    {
        npd_syslog_dbg("AD_RX_PORT_DISABLED");
        lacp_port_sm->sm_rx_state = AD_RX_PORT_DISABLED;	    /* next state*/
    }
    /* check if new lacpdu arrived*/
    else if (lacpdu && ((lacp_port_sm->sm_rx_state == AD_RX_EXPIRED) || (lacp_port_sm->sm_rx_state == AD_RX_DEFAULTED) || (lacp_port_sm->sm_rx_state == AD_RX_CURRENT)))
    {
        npd_syslog_dbg("AD_RX_EXPIRED or AD_RX_DEFAULTED or AD_RX_CURRENT");
        lacp_port_sm->sm_rx_timer_counter = 0; /* zero timer*/
        lacp_port_sm->sm_rx_state = AD_RX_CURRENT;
    }
    else
    {
        /* if timer is on, and if it is expired*/
		npd_syslog_dbg("2> Rx sm: 0x%x, timer counter: %d.\r\n", lacp_port_sm->sm_rx_state, lacp_port_sm->sm_rx_timer_counter);
        if (lacp_port_sm->sm_rx_timer_counter == 1)/*FIXME.*/
        {
			lacp_port_sm->sm_rx_timer_counter--;
            switch (lacp_port_sm->sm_rx_state)
            {
                case AD_RX_EXPIRED:
                    lacp_port_sm->sm_rx_state = AD_RX_DEFAULTED;		/* next state*/
                    break;
                case AD_RX_CURRENT:
                    lacp_port_sm->sm_rx_state = AD_RX_EXPIRED;	    /* next state*/
                    break;
                default:    /*to silence the compiler*/
                    break;
            }
        }
        else
        {
			if(lacp_port_sm->sm_rx_timer_counter)
			{
			    lacp_port_sm->sm_rx_timer_counter--;
			}
            /* if no lacpdu arrived and no timer is on*/
            switch (lacp_port_sm->sm_rx_state)
            {
                case AD_RX_PORT_DISABLED:

                    if (lacp_port_sm->sm_vars & AD_PORT_MOVED)
                    {
                        lacp_port_sm->sm_rx_state = AD_RX_INITIALIZE;	    /* next state*/
                    }
                    else if (lacp_port_actor->is_enabled && (lacp_port_sm->sm_vars & AD_PORT_LACP_ENABLED))
                    {
                        lacp_port_sm->sm_rx_state = AD_RX_EXPIRED;	/* next state*/
                    }
                    else if (lacp_port_actor->is_enabled && ((lacp_port_sm->sm_vars & AD_PORT_LACP_ENABLED) == 0))
                    {
                        lacp_port_sm->sm_rx_state = AD_RX_LACP_DISABLED;    /* next state*/
                    }
                    break;
                default:    /*to silence the compiler*/
                    break;
            }
        }
    }

    /* check if the State machine was changed or new lacpdu arrived*/
    if ((lacp_port_sm->sm_rx_state != last_state) || (lacpdu))
    {
        switch (lacp_port_sm->sm_rx_state)
        {
            case AD_RX_INITIALIZE:
                npd_syslog_dbg("change: AD_RX_INITIALIZE, netif_index : 0x%x", netif_index);

                if (!(lacp_port_actor->actor_oper_port_key & AD_DUPLEX_KEY_BITS))
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_LACP_ENABLED;
                     npd_syslog_dbg("~AD_PORT_LACP_ENABLED, netif_index : 0x%x", netif_index);
                }
                else
                {
                    lacp_port_sm->sm_vars |= AD_PORT_LACP_ENABLED;
                     npd_syslog_dbg("|= AD_PORT_LACP_ENABLED, netif_index : 0x%x", netif_index);
                }

                lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
               
                if(__record_default(lacp_port_partner) == 1)
                {
                    lacp_port_actor->actor_oper_port_state |= AD_STATE_DEFAULTED;
                }
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_EXPIRED;
                lacp_port_sm->sm_vars &= ~AD_PORT_MOVED;
                lacp_port_sm->sm_rx_state = AD_RX_PORT_DISABLED;	/* next state*/
                /*- Fall Through -*/
            case AD_RX_PORT_DISABLED:
                npd_syslog_dbg("change: AD_RX_PORT_DISABLED, netif_index : 0x%x", netif_index);
                lacp_port_sm->sm_vars &= ~AD_PORT_MATCHED;
                break;
            case AD_RX_LACP_DISABLED:
                npd_syslog_dbg("change: AD_RX_LACP_DISABLED, netif_index : 0x%x", netif_index);
                lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
                if(__record_default(lacp_port_partner) == 1)
                {
                    lacp_port_actor->actor_oper_port_state |= AD_STATE_DEFAULTED;
                }
                lacp_port_partner->partner_oper_port_state &= ~AD_STATE_AGGREGATION;
                lacp_port_sm->sm_vars |= AD_PORT_MATCHED;
                //lacp_port_sm->sm_vars &= ~AD_PORT_MATCHED;
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_EXPIRED;
                break;
            case AD_RX_EXPIRED:
                npd_syslog_dbg("change: AD_RX_EXPIRED, netif_index : 0x%x", netif_index);
                /*Reset of the Synchronization flag. (Standard 43.4.12)*/
                /*This reset cause to disable this port in the COLLECTING_DISTRIBUTING state of the*/
                /*mux machine in case of EXPIRED even if LINK_DOWN didn't arrive for the port.*/
                lacp_port_partner->partner_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;
                lacp_port_sm->sm_vars &= ~AD_PORT_MATCHED;
                //lacp_port_partner->partner_oper_port_state |= AD_SHORT_TIMEOUT;
                lacp_port_partner->partner_oper_port_state |= AD_STATE_LACP_TIMEOUT;
                lacp_port_sm->sm_rx_timer_counter = __dot3ad_timer_to_ticks(AD_CURRENT_WHILE_TIMER, (unsigned short)(AD_SHORT_TIMEOUT));
                lacp_port_actor->actor_oper_port_state |= AD_STATE_EXPIRED;
                break;
            case AD_RX_DEFAULTED:
                npd_syslog_dbg("change: AD_RX_DEFAULTED, netif_index : 0x%x", netif_index);
                if(__update_default_selected(lacp_port_partner) == 1)
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
                }
                if(__record_default(lacp_port_partner) == 1)
                {
                    lacp_port_actor->actor_oper_port_state |= AD_STATE_DEFAULTED;
                }
                lacp_port_sm->sm_vars |= AD_PORT_MATCHED;
               // lacp_port_sm->sm_vars &= ~AD_PORT_MATCHED;
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_EXPIRED;
                break;
            case AD_RX_CURRENT:
                npd_syslog_dbg("change: AD_RX_CURRENT, netif_index : 0x%x", netif_index);

                /* detect loopback situation*/
                if (!MAC_ADDRESS_COMPARE(&(lacpdu->actor_system), &(lacp_port_actor->actor_system)))
                {
                    /* INFO_RECEIVED_LOOPBACK_FRAMES*/
                    npd_syslog_dbg("lacp: An illegal loopback occurred on adapter (netif_index = 0X%x). Check the configuration to verify that all Adapters are connected to 802.3ad compliant switch ports\r\n",netif_index);

                    goto error;
                }
                /* lacpdu->reserved_3_1              initialized
                          * lacpdu->tlv_type_partner_info     initialized
                          * lacpdu->partner_information_length initialized
                           */
             
                __update_selected(lacpdu, lacp_port_partner, lacp_port_sm);
                __update_ntt(lacpdu, lacp_port_actor, lacp_port_sm);
                __record_pdu(lacpdu, lacp_port_actor, lacp_port_partner, lacp_port_sm);
                __choose_matched(lacpdu, lacp_port_actor, lacp_port_sm);
				npd_syslog_dbg("lacp_port_actor->LACP_TIMEOUT : %d.\n", (lacp_port_actor->actor_oper_port_state & AD_STATE_LACP_TIMEOUT));
                lacp_port_sm->sm_rx_timer_counter = __dot3ad_timer_to_ticks(AD_CURRENT_WHILE_TIMER, (unsigned short)(lacp_port_actor->actor_oper_port_state & AD_STATE_LACP_TIMEOUT));
                lacp_port_actor->actor_oper_port_state &= ~AD_STATE_EXPIRED;
                /* verify that if the aggregator is enabled, the port is enabled too.*/
                /*(because if the link goes down for a short time, the 802.3ad will not*/

                /* catch it, and the port will continue to be disabled) ??? move  */
             #if  0
             if (aggregator_en && !__port_is_enabled(netif_index))
                {
                    __enable_port(netif_index);
                }
             #endif
                break;
            default:    /*to silence the compiler*/
                
                break;
        }
        //the aggregator of port should synchronize with the partner
        if(aggregator_en )
        {
            aggregator->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
            aggregator->partner_oper_aggregator_key = lacp_port_partner->partner_oper_key;
            aggregator->partner_system = lacp_port_partner->partner_oper_system;
            dbtable_hash_update(aggregator_index,  aggregator,  aggregator);
        }
        dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
        dbtable_sequence_update(lacpport_partner_index, netif_index, lacp_port_partner, lacp_port_partner);
    }
    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
error:
    
    __release_rx_machine_lock(netif_index);
    if(aggregator)
        free(aggregator);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    return ret;
}

/**
 * dot3ad_tx_machine - handle a port's tx state machine
 * @port: the port we're looking at
 *
 */
static int dot3ad_tx_machine(unsigned int netif_index)
{
    int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{   
	    goto error;
	}
    /* check if tx timer expired, to verify that we do not send more than 3 packets per second*/
    // if (lacp_port_sm->sm_tx_timer_counter && !(--lacp_port_sm->sm_tx_timer_counter))
    // {
        if (lacp_port_sm->ntt && (lacp_port_sm->sm_vars & AD_PORT_LACP_ENABLED))
        {
            /* send the lacpdu*/
            if (nam_packet_tx_lacp(netif_index) == 0)
            {  
                /* mark ntt as false, so it will not be sent again until demanded*/
                lacp_port_sm->ntt = FALSE;
            }
        }

        /* restart tx timer(to verify that we will not exceed AD_MAX_TX_IN_SECOND*/
        lacp_port_sm->sm_tx_timer_counter = dot3ad_ticks_per_sec/AD_MAX_TX_IN_SECOND;
        ret = dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
     // }
error:
 
    if(lacp_port_sm)
        free(lacp_port_sm);
    return ret;
    
}


/**
 * dot3ad_periodic_machine - handle a port's periodic state machine
 * @port: the port we're looking at
 *
 * Turn ntt flag on priodically to perform periodic transmission of lacpdu's.
 */
static int dot3ad_periodic_machine(unsigned int netif_index)
{
    int ret = -1;
    periodic_states_t last_state;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
        goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
	lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{
        goto error;
	}

    /* keep current state machine state to compare later if it was changed*/
    last_state = lacp_port_sm->sm_periodic_state;
    /* check if port was reinitialized*/
    if (((lacp_port_sm->sm_vars & AD_PORT_BEGIN) || !(lacp_port_sm->sm_vars & AD_PORT_LACP_ENABLED) || !lacp_port_actor->is_enabled) ||
            (!(lacp_port_actor->actor_oper_port_state & AD_STATE_LACP_ACTIVITY) && !(lacp_port_partner->partner_oper_port_state & AD_STATE_LACP_ACTIVITY))
       )
    {
        lacp_port_sm->sm_periodic_state = AD_NO_PERIODIC;	     /* next state*/
    }
    /* check if state machine should change state*/
    else if (lacp_port_sm->sm_periodic_timer_counter)
    {
        /* If not expired, check if there is some new timeout parameter from the partner state*/
        switch (lacp_port_sm->sm_periodic_state)
        {
            case AD_FAST_PERIODIC:
                if (!(lacp_port_partner->partner_oper_port_state & AD_STATE_LACP_TIMEOUT))
                {
                    lacp_port_sm->sm_periodic_state = AD_SLOW_PERIODIC;  /* next state*/
                }

                break;
            case AD_SLOW_PERIODIC:

                if ((lacp_port_partner->partner_oper_port_state & AD_STATE_LACP_TIMEOUT))
                {
                    /* stop current timer*/
                    lacp_port_sm->sm_periodic_timer_counter = 0;
                    lacp_port_sm->sm_periodic_state = AD_PERIODIC_TX;	 /* next state*/
                }

                break;
            default:    /*to silence the compiler*/
                break;
        }
        
        if(lacp_port_sm->sm_periodic_state == last_state)
        {
            /* check if periodic state machine expired*/
            if (!(--lacp_port_sm->sm_periodic_timer_counter))
            {
                /* if expired then do tx*/
                lacp_port_sm->sm_periodic_state = AD_PERIODIC_TX;    /* next state*/
            }
        }
    }
    else
    {
        switch (lacp_port_sm->sm_periodic_state)
        {
            case AD_NO_PERIODIC:
                lacp_port_sm->sm_periodic_state = AD_FAST_PERIODIC;	 /* next state*/
                break;
            case AD_PERIODIC_TX:

                if (!(lacp_port_partner->partner_oper_port_state & AD_STATE_LACP_TIMEOUT))
                {
                    lacp_port_sm->sm_periodic_state = AD_SLOW_PERIODIC;  /* next state*/
                }
                else
                {
                    lacp_port_sm->sm_periodic_state = AD_FAST_PERIODIC;  /* next state*/
                }

                break;
            default:    /*to silence the compiler*/
                break;
        }
    }

    /* check if the state machine was changed*/
    if (lacp_port_sm->sm_periodic_state != last_state)
    {
        switch (lacp_port_sm->sm_periodic_state)
        {
            case AD_NO_PERIODIC:
                lacp_port_sm->sm_periodic_timer_counter = 0;	   /* zero timer*/
                break;
            case AD_FAST_PERIODIC:
                lacp_port_sm->sm_periodic_timer_counter = __dot3ad_timer_to_ticks(AD_PERIODIC_TIMER, (unsigned short)(AD_FAST_PERIODIC_TIME));//-1; /* decrement 1 tick we lost in the PERIODIC_TX cycle*/
                break;
            case AD_SLOW_PERIODIC:
                lacp_port_sm->sm_periodic_timer_counter = __dot3ad_timer_to_ticks(AD_PERIODIC_TIMER, (unsigned  short)(AD_SLOW_PERIODIC_TIME)-3);//-1; /* decrement 1 tick we lost in the PERIODIC_TX cycle*/
                break;
            case AD_PERIODIC_TX:
                lacp_port_sm->ntt = TRUE;
                if (!(lacp_port_partner->partner_oper_port_state & AD_STATE_LACP_TIMEOUT))
                {
                    lacp_port_sm->sm_periodic_state = AD_SLOW_PERIODIC;  /* next state*/
                    lacp_port_sm->sm_periodic_timer_counter = __dot3ad_timer_to_ticks(AD_PERIODIC_TIMER, (unsigned  short)(AD_SLOW_PERIODIC_TIME)-3);
                }
                else
                {
                    lacp_port_sm->sm_periodic_state = AD_FAST_PERIODIC;  /* next state*/
                    lacp_port_sm->sm_periodic_timer_counter = __dot3ad_timer_to_ticks(AD_PERIODIC_TIMER, (unsigned short)(AD_FAST_PERIODIC_TIME));
                }
                break;
            default:    /*to silence the compiler*/
                break;
        }
    }
    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
error:
   
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    return ret;
}

/**
 * dot3ad_port_selection_logic - select aggregation groups
 * @port: the port we're looking at
 *
 * Select aggregation groups, and assign each port for it's aggregetor. The
 * selection logic is called in the inititalization (after all the handshkes),
 * and after every lacpdu receive (if selected is off).
 */
static int dot3ad_port_selection_logic(unsigned int netif_index)
{
    int ret = -1;
    unsigned char select = 0;
    unsigned char found = 0;
    unsigned char port_leave_trunk = 0;
    unsigned char type = 0; 
    unsigned char actor_sta_default = 0;
    unsigned char partner_sta_lacp_en = 0;
    unsigned int bind_agg = NPD_FALSE;
    unsigned int join_agg = NPD_FALSE;
    unsigned int aggId = 0;
    unsigned int agg_id_port = 0;
    unsigned int trunk_netif_index = 0;
    aggregator_t    *aggregator = NULL;
    aggregator_t item, item_out;
    aggregator_t    *best_aggregator = NULL;
    lacp_port_sm_t    *lacp_port_sm = NULL;
    lacp_port_sm_t    *lacp_port_sm_match = NULL;
	lacp_port_actor_t  *lacp_port_actor = NULL;
	lacp_port_partner_t  *lacp_port_partner = NULL;

    aggregator = malloc(sizeof(aggregator_t));
    best_aggregator = malloc(sizeof(aggregator_t));
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
    lacp_port_sm_match = malloc(sizeof(lacp_port_sm_t));
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
    lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
    memset(best_aggregator, 0 , sizeof(aggregator_t));
    memset(&item, 0, sizeof(struct aggregator));
    memset(&item_out, 0, sizeof(struct aggregator));
	if(aggregator == NULL || lacp_port_sm == NULL || lacp_port_sm_match == NULL || lacp_port_actor == NULL || 
       lacp_port_partner == NULL || best_aggregator == NULL)
	{
        goto error;
	}
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "lacp_port_sm");
        goto error;
	}
    /* if the  lacp  is disable, do nothing*/
    if((lacp_port_sm->sm_vars & AD_PORT_LACP_ENABLED) == NPD_FALSE)
    {
        npd_syslog_dbg("error:AD_PORT_LACP_ENABLED = NPD_FALSE");
        goto error;
    }
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "lacp_port_actor");
        goto error;
	}
	lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{
	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "lacp_port_partner");
        goto error;
	}
    aggregator->netif_index = netif_index;
	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
	if(ret == 0)
	{  
	    if(aggregator->aggregator_mode != DYNAMIC_MODE)
        {
            npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "aggregator->aggregator_mode != DYNAMIC_MODE");
            goto error; 
        }   
          
    }
    if((lacp_port_actor->actor_oper_port_state & AD_STATE_DEFAULTED) == AD_STATE_DEFAULTED)
    {
        actor_sta_default = 1;
    }
    if(lacp_port_partner->partner_oper_port_state & 0x01)
    {
        partner_sta_lacp_en = 1;
    }
   
    agg_id_port = aggregator->aggregator_identifier;

    /* if the port is already Selected, do nothing*/
    if (lacp_port_sm->sm_vars & AD_PORT_SELECTED)
    {
        select = 1;
        if((lacp_port_actor->actor_oper_port_state & AD_STATE_SYNCHRONIZATION) == NPD_FALSE)
        {
            lacp_port_actor->actor_oper_port_state |= AD_STATE_SYNCHRONIZATION;
            dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
        }
    }
    /*Find the aggregator of this port */
    trunk_netif_index = lacp_port_actor->trunk_netif_index;
    npd_syslog_dbg("trunk_netif_index = 0x%x", trunk_netif_index);
    type = npd_netif_type_get(trunk_netif_index);
    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        aggregator->netif_index = trunk_netif_index;
    	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
    	if(ret != 0)
    	{ 
    	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "aggregator");
            goto error;   
    	}
        /* only dynamic aggregator can be selected*/
        if(aggregator->aggregator_mode == STATIC_MODE)
		{
			ret = -1;
            if((select) && (!partner_sta_lacp_en))
            {
                lacp_port_sm->ntt = TRUE;
                lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
                dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
            }
            else if((!select) && (partner_sta_lacp_en))
            {
                lacp_port_sm->ntt = TRUE;
                lacp_port_sm->sm_vars |= AD_PORT_SELECTED;
                dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
                if((lacp_port_actor->actor_oper_port_state & AD_STATE_SYNCHRONIZATION) == NPD_FALSE)
                {
                    lacp_port_actor->actor_oper_port_state |= AD_STATE_SYNCHRONIZATION;
                    dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
                }
            }
            
            npd_syslog_dbg("DEBUG: only dynamic aggregator can be selected");
            goto error;
		}
        
        if((lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION) == FALSE)
        {
            npd_syslog_dbg("DEBUG:(1) the port is connected to other aggregator, detach it");
            ret = dynamic_aggregator_leave(trunk_netif_index, netif_index);
            if(ret != 0)
        	{  
                goto error;  
        	}
            port_leave_trunk = 1;
        }
    }
	if(port_leave_trunk == 1)
    {
    	lacp_port_actor->netif_index = netif_index;
    	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
        if(ret != 0)
        {  
            goto error;  
        }
    }
    
    if(actor_sta_default)
    {
        unsigned int update = 0; 
        if(lacp_port_actor->actor_oper_port_state & AD_STATE_SYNCHRONIZATION)
        {
            update = 1;
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;  
        }
        if(lacp_port_actor->actor_oper_port_state & AD_STATE_DISTRIBUTING)
        {
            update = 1;
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
           
        }
        if(lacp_port_actor->actor_oper_port_state & AD_STATE_COLLECTING)
        {
            update = 1;
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
           
        }
        if(update)
        {
            dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
        }
        goto error;
    }
 
    /* if the port is a member of dynamic aggregator , do nothing*/
    if((port_leave_trunk == NPD_FALSE) && (NPD_NETIF_TRUNK_TYPE == type))
    {
        goto error;
    }
    /* search on all aggregators for a suitable aggregator for this port*/
    if((lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION))
    {
        unsigned int eth_index = 0;
        while(1)
        {  
            memset(aggregator, 0, sizeof(aggregator_t));
            ret = dbtable_hash_next(aggregator_index, &item, &item_out, NULL);
            if(0 != ret)
            {
                break;
            }
            
            item = item_out;

            memcpy(aggregator, &item, sizeof(aggregator_t));
            aggId = aggregator->aggregator_identifier;
            //不能和自己比较
            if(agg_id_port == aggId)
            {
                continue;
            }
            if (((aggregator->actor_oper_aggregator_key == lacp_port_actor->actor_oper_port_key) && /* if all parameters match AND*/
                    !MAC_ADDRESS_COMPARE(&(aggregator->partner_system), &(lacp_port_partner->partner_oper_system)) &&
                    (aggregator->partner_system_priority == lacp_port_partner->partner_oper_system_priority) &&
                    (aggregator->partner_oper_aggregator_key == lacp_port_partner->partner_oper_key)
                ) &&
                    ((MAC_ADDRESS_COMPARE(&(lacp_port_partner->partner_oper_system), &(null_mac_addr)) && /* partner answers*/
                      !aggregator->is_individual)  /* but is not individual OR*/
                    )
               )
            {
                npd_syslog_dbg("DEBUG: find match agg (%d)", aggId);
                if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(aggregator->netif_index))
                {
                   if(best_aggregator->num_of_ports < aggregator->num_of_ports)
                   {
                       npd_syslog_dbg("DEBUG: match trunk agg (%d)", aggId);
                       //best_aggregator->aggregator_identifier = aggId;
                       best_aggregator->num_of_ports = aggregator->num_of_ports;
                       best_aggregator->netif_index = aggregator->netif_index;
                       join_agg = NPD_TRUE;
                   }
                }
                else if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(aggregator->netif_index))
                {
                    eth_index = aggregator->netif_index;
                    lacp_port_sm_match->netif_index = eth_index;
                	ret = dbtable_sequence_search(lacpport_sm_index, eth_index, lacp_port_sm_match);
                	if(ret != 0)
                	{  
                        goto error;
                	}
                    memset(lacp_port_partner, 0, sizeof(lacp_port_partner_t));
                    lacp_port_partner->netif_index = eth_index;
        	        ret = dbtable_sequence_search(lacpport_partner_index, eth_index, lacp_port_partner);
              	    if(ret != 0)
              	    {
                        goto error;
              	    }
                    if((lacp_port_sm_match->sm_vars & AD_PORT_SELECTED) && (lacp_port_partner->partner_oper_port_state & AD_STATE_SYNCHRONIZATION))
                    {
                        npd_syslog_dbg("DEBUG: match eth agg (%d)", aggId);
                        best_aggregator->netif_index = aggregator->netif_index;
                        bind_agg = NPD_TRUE;
                    }
               
                    #if 0
                    if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(best_aggregator->netif_index))
                    {
                    }
                    #endif
                }
               
            }
        }
        /* attach to the founded aggregator*/
        if(join_agg == NPD_TRUE)
        {
            trunk_netif_index = best_aggregator->netif_index;
            ret = dynamic_aggregator_join(trunk_netif_index, netif_index);
            if(ret != 0)
            {
                goto error;
            }
            
            found = 1;
        }
        else if(bind_agg == NPD_TRUE)
        {
             /* the two aggregators band together for a new aggregator and create a new trunk*/
             ret = dot3a_band_aggregator_to_trunk(best_aggregator->netif_index,netif_index);
             if(ret != 0)
             {
                 goto error;
             }
    
             found = 1;
        }
    }
    
    if((!found) && (!select))
    {
    	lacp_port_actor->netif_index = netif_index;
    	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
    	if(ret != 0)
    	{
    	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                                   __FILE__, __LINE__, "lacp_port_actor");
            goto error;
    	}
        lacp_port_sm->netif_index = netif_index;
    	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
    	if(ret != 0)
    	{  
    	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                                   __FILE__, __LINE__, "lacp_port_sm");
            goto error;
    	}
        lacp_port_sm->ntt = TRUE;
        lacp_port_sm->sm_vars |= AD_PORT_SELECTED;
        lacp_port_actor->actor_oper_port_state |= AD_STATE_SYNCHRONIZATION;
        lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
        lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
        dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
        dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
    }
error:
    if(aggregator)
        free(aggregator);
    if(best_aggregator)
        free(best_aggregator);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_sm_match)
        free(lacp_port_sm_match);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
   
    return ret;
}
static int dot3ad_enable_lacp_port(unsigned int netif_index)
{
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    lacp_port_actor_t *lacp_port_actor = NULL;
    struct eth_port_s *eth_ports = NULL;
    
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
        ret = COMMON_RETURN_CODE_NO_RESOURCE;
	    goto error;
	}

    npd_syslog_dbg("find a lacp_port_actor of the port ");
	npd_key_database_lock();
    eth_ports = npd_get_port_by_index(netif_index);
    if(eth_ports->trunkid > 0)
    {
        npd_syslog_dbg("Erro: port is a member of the trunk!");
        ret = TRUNK_RETURN_CODE_PORT_EXISTS;
        goto error;
    }
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
	     npd_syslog_dbg("enable lacp");
	    //没有找到netif_index的lacp_port_actor，则创建一个新的aggregator
	    dot3ad_initialize_port(netif_index, FALSE, AD_ACTIVE, TRUE);
        npd_lacp_aggregator_init(netif_index, DYNAMIC_MODE);
        ret = 0;
        goto error;
	}
    #if 0
    if(!lacp_port_actor->is_lacp_enable)
    {
        lacp_port_actor->is_lacp_enable = 1;
        dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
        npd_syslog_dbg(" Lacp is enable");
    }
    #endif
error:
    npd_key_database_unlock();
    if(lacp_port_actor)
        free(lacp_port_actor);
     if(eth_ports)
        free(eth_ports);
    return ret;
}
int npd_port_aggregator_delete(unsigned int netif_index)
{   
    int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    npd_syslog_dbg("delete the relative date of this port ! ");
    //删除netif_index  自身的lacp_port_actor、lacp_port_sm、lacp_port_partner、aggregator
    lacp_port_actor->netif_index = netif_index;
    ret = dbtable_sequence_delete(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
    if(ret != 0)
	{  
        goto error;
	}
    lacp_port_partner->netif_index = netif_index;
    ret = dbtable_sequence_delete(lacpport_partner_index, netif_index, lacp_port_partner, lacp_port_partner);
    if(ret != 0)
	{  
        goto error;
	}
    lacp_port_sm->netif_index = netif_index;
    ret = dbtable_sequence_delete(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    if(ret != 0)
	{  
        goto error;
	}
    ret = npd_lacp_aggregator_delete(netif_index);
    if(ret == 0)
    {
        npd_syslog_dbg("port_aggregator_delete success! ");
    }
error:

    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    return ret;
}
static int dot3ad_disable_lacp_port(unsigned int netif_index)
{
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned int trunk_netif_index;
    aggregator_t *aggregator = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;

    aggregator = malloc(sizeof(aggregator_t));
    if(NULL == aggregator)
    {
        goto error;
    }
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    npd_syslog_dbg("port: lacp disable ");
    
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
	    npd_syslog_dbg("lacp_port_actor : no find exit! ");
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
        goto error;
	}
    
    trunk_netif_index = lacp_port_actor->trunk_netif_index;
    if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(trunk_netif_index))
    {
        npd_syslog_dbg("Port is a member of the trunk!");
        aggregator->netif_index = trunk_netif_index;
    	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
    	if(ret != 0)
    	{  
    	    goto error;
    	}
    
        //只能对动态的aggregator 端口进行disable 操作
        if(aggregator->aggregator_mode == MANUAL_MODE)
        {
            ret = TRUNK_RETURN_CODE_PORT_EXISTS;
            goto error;
        }
        if(aggregator->aggregator_mode == STATIC_MODE)
        {
            npd_syslog_dbg("The port of static trunk can not be disable");
            ret = TRUNK_RETURN_CODE_PORT_EXISTS;
            goto error;
        }
        //从trunk的AGG中移除端口
       
        dynamic_aggregator_leave(trunk_netif_index, netif_index);

    }

    ret = npd_port_aggregator_delete(netif_index);
    if(ret == 0)
    {
        npd_syslog_dbg("lacp disable success! ");
    }
error:

    if(aggregator)
        free(aggregator);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
}
/**
 * dot3ad_agg_selection_logic - select an aggregation group for a team
 * @aggregator: the aggregator we're looking at
 *
 * It is assumed that only one aggregator may be selected for a team.
 * The logic of this function is to select (at first time) the aggregator with
 * the most ports attached to it, and to reselect the active aggregator only if
 * the previous aggregator has no more ports related to it.
 *
 * FIXME: this function MUST be called with the first agg in the bond, or
 * __get_active_agg() won't work correctly. This function should be better
 * called with the bond itself, and retrieve the first agg from it.
 */
    
    /*get current active aggregator*/

    /* search for the aggregator with the most ports attached to it.*/
        /* count how many candidate lag's we have*/


                /*The reasons for choosing new best aggregator:*/
                /* 1. if current agg is NOT individual and the best agg chosen so far is individual OR*/
                /* current and best aggs are both individual or both not individual, AND*/
                /* 2a.  current agg partner reply but best agg partner do not reply OR*/
                /* 2b.  current agg partner reply OR current agg partner do not reply AND best agg partner also do not reply AND*/
                /*      current has more ports/bandwidth, or same amount of ports but current has faster ports, THEN*/
                /*      current agg become best agg so far*/

                /*if current agg is NOT individual and the best agg chosen so far is individual change best_aggregator*/
                /* current and best aggs are both individual or both not individual*/
                    /*  current and best aggs are both individual or both not individual AND*/
                    /*  current agg partner reply but best agg partner do not reply*/
                    /*  current agg partner reply OR current agg partner do not reply AND best agg partner also do not reply*/


    /* if we have new aggregator selected, don't replace the old aggregator if it has an answering partner,*/

    /* or if both old aggregator and new aggregator don't have answering partner*/
            /* if new aggregator has link, and old aggregator does not, replace old aggregator.(do nothing)*/
            /* -> don't replace otherwise.*/

    /* if there is new best aggregator, activate it*/

        /* check if any partner replys*/

        /* check if there are more than one aggregator*/


        /* disable the ports that were related to the former active_aggregator*/

    /* if the selected aggregator is of join individuals(partner_system is NULL), enable their ports*/


       
/**
 * dot3ad_clear_agg - clear a given aggregator's parameters
 * @aggregator: the aggregator we're looking at
 *
 */
static void dot3ad_clear_agg(aggregator_t *aggregator)
{
    if (aggregator)
    {
        aggregator->is_individual = 0;
        aggregator->actor_admin_aggregator_key = 0;
        aggregator->actor_oper_aggregator_key = 0;
        aggregator->partner_system = null_mac_addr;
        aggregator->partner_system_priority = 0;
        aggregator->partner_oper_aggregator_key = 0;
        aggregator->receive_state = 0;
        aggregator->transmit_state = 0;
        aggregator->is_active = 0;
        aggregator->num_of_ports = 0;
    }
}


/*port no. as port priority, vlan no. as system priority*/
unsigned short actor_system_priority = 32768;

/*
端口LACP禁止时删除与之对应的aggregator
或者端口聚合时,删除其中一个端口上的aggregator
或者在手工删除port-channel时删除一个与trunk netif index对应的aggregator
*/
int npd_lacp_aggregator_delete(unsigned int netif_index)
{
	int ret = -1;
	aggregator_t *aggregator_p = NULL;
	aggregator_p = (aggregator_t *)malloc(sizeof(aggregator_t));
	if(NULL == aggregator_p)
	{
		return ret;
	}

    npd_syslog_dbg("Delete aggregator!");
	aggregator_p->netif_index = netif_index;
	ret = dbtable_hash_delete(aggregator_index, aggregator_p, aggregator_p);
    if(ret == 0)
    {
        npd_syslog_dbg("The aggregator deletes success");
    }
	free(aggregator_p);
	return ret;
}


/**
 * dot3ad_initialize_agg - initialize a given aggregator's parameters
 * @aggregator: the aggregator we're looking at
 *
 */
static void dot3ad_initialize_agg(aggregator_t *aggregator)
{
    unsigned int netif_index = 0;
    netif_index = aggregator->netif_index;
    npd_syslog_dbg("dot3ad_initialize_agg\n");
    if (aggregator)
    {
        dot3ad_clear_agg(aggregator);
        aggregator->aggregator_mac_address = null_mac_addr;
        if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
        {
            aggregator->aggregator_identifier = npd_aggregator_id_get();
        }
        else
        {
           aggregator->aggregator_identifier = npd_aggregator_get_max_id();
        }
    }
}

/*
端口LACP使能时产生一个与之对应的aggregator
或者在手工创建port-channel时生成一个与trunk netif index对应的aggregator
*/
int npd_lacp_aggregator_init(unsigned int netif_index, aggregator_mode_t mode)
{
	int ret = -1; 
	lacp_port_actor_t  *lacp_port_actor = NULL;
	lacp_port_partner_t  *lacp_port_partner = NULL;
    aggregator_t *aggregator_p = NULL;
  
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    aggregator_p = (aggregator_t *)malloc(sizeof(aggregator_t));
	if(NULL == aggregator_p)
	{
		goto error;
	}
    memset(aggregator_p, 0, sizeof(aggregator_t));
    
    aggregator_p->netif_index = netif_index;
    dot3ad_initialize_agg(aggregator_p);
  
    aggregator_p->aggregator_mode = mode;
	npd_system_get_basemac(aggregator_p->aggregator_mac_address.mac_addr_value, 6);
	aggregator_p->netif_index = netif_index;

    if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
    {
        lacp_port_actor->netif_index = netif_index;
    	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
    	if(ret != 0)
    	{
            goto error;
    	}
    	lacp_port_partner->netif_index = netif_index;
    	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
    	if(ret != 0)
    	{
            goto error;
    	}
        
        lacp_port_actor->actor_port_aggregator_identifier = aggregator_p->aggregator_identifier;
        /* update the new aggregator's parameters*/

        /* if port was responsed from the end-user*/
        if (lacp_port_actor->actor_oper_port_key & AD_DUPLEX_KEY_BITS)  /* if port is full duplex*/
        { 
            aggregator_p->is_individual = 0;
        }
        else
        {
            aggregator_p->is_individual = 1;
        }
        aggregator_p->actor_admin_aggregator_key = lacp_port_actor->actor_admin_port_key;
        aggregator_p->actor_oper_aggregator_key = lacp_port_actor->actor_oper_port_key;
        aggregator_p->partner_system = lacp_port_partner->partner_oper_system;
        aggregator_p->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
        aggregator_p->partner_oper_aggregator_key = lacp_port_partner->partner_oper_key;
        aggregator_p->receive_state = 1;
        aggregator_p->transmit_state = 1;
        aggregator_p->num_of_ports++;
        ret = dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
    }
	npd_syslog_dbg("insert an aggregator_p!\n");
	ret = dbtable_hash_insert(aggregator_index, aggregator_p);
    if(ret == 0)
    {
         npd_syslog_dbg("aggregator init success!\n");
    }
error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(aggregator_p)
        free(aggregator_p);
	return ret;
}

/**
 * dot3ad_initialize_port - initialize a given port's parameters
 * @aggregator: the aggregator we're looking at
 * @lacp_fast: boolean. whether fast periodic should be used
 *
 */
int dot3ad_initialize_port(unsigned int netif_index, int lacp_fast, unsigned long lacp_mode, unsigned long enabled)
{
	int ret = -1;
    unsigned int port_link_state;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;
    struct eth_port_s *eth_ports = NULL;

    lacp_port_sm = (lacp_port_sm_t *)malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = (lacp_port_actor_t *)malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
        goto error;
	}
	lacp_port_partner = (lacp_port_partner_t *)malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    eth_ports = malloc(sizeof(struct eth_port_s));
    if(eth_ports == NULL)
	{
        goto error;
	}

    memset(lacp_port_sm, 0, sizeof(lacp_port_sm_t));
    memset(lacp_port_actor, 0, sizeof(lacp_port_actor_t));
    memset(lacp_port_partner, 0, sizeof(lacp_port_partner_t));
    memset(eth_ports, 0, sizeof(struct eth_port_s));
    if (npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
    {
        lacp_port_actor->actor_port_number = (unsigned short)eth_port_array_index_from_ifindex(netif_index);
        lacp_port_actor->actor_port_priority = (unsigned short)eth_port_array_index_from_ifindex(netif_index);
        npd_system_get_basemac(lacp_port_actor->actor_system.mac_addr_value, 6);
        lacp_port_actor->actor_system_priority = actor_system_priority;/*FIXME...*/
        lacp_port_actor->actor_port_aggregator_identifier = 0;
        lacp_port_sm->ntt = FALSE;
        lacp_port_actor->actor_admin_port_key = 1;
        lacp_port_actor->actor_oper_port_key = 0;
        lacp_port_actor->actor_oper_port_key  |= __get_duplex( netif_index);
        lacp_port_actor->actor_oper_port_key  |= (__get_link_speed(netif_index) << 1);
        lacp_port_actor->actor_admin_port_state = 0;
		if(enabled)
		{
            lacp_port_actor->actor_admin_port_state |= AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY;
            lacp_port_actor->actor_oper_port_state  |= AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY;
			lacp_port_actor->is_lacp_enable = TRUE;
		}
		else
		{
            lacp_port_actor->actor_admin_port_state |= AD_STATE_AGGREGATION;
            lacp_port_actor->actor_oper_port_state  |= AD_STATE_AGGREGATION;
			lacp_port_actor->is_lacp_enable = FALSE;
		}
        
        if (lacp_fast)
        {
            lacp_port_actor->actor_oper_port_state |= AD_STATE_LACP_TIMEOUT;
        }

        lacp_port_partner->partner_admin_system = null_mac_addr;
        lacp_port_partner->partner_oper_system  = null_mac_addr;
        lacp_port_partner->partner_admin_system_priority = 0xffff;
        lacp_port_partner->partner_oper_system_priority  = 0xffff;
        lacp_port_partner->partner_admin_key = 1; // 1
        lacp_port_partner->partner_oper_key  = 0; // 1
        lacp_port_partner->partner_admin_port_number = 1;  // 1
        lacp_port_partner->partner_oper_port_number  = 1;  // 1
        lacp_port_partner->partner_admin_port_priority = 0xff;
        lacp_port_partner->partner_oper_port_priority  = 0xff;
        lacp_port_partner->partner_admin_port_state = 0;  // 1
        lacp_port_partner->partner_oper_port_state  = 0;  // 1

       
        eth_ports->eth_port_ifindex = netif_index;
        npd_key_database_lock();
        ret = dbtable_sequence_search(g_eth_ports, netif_index, eth_ports);
        if(ret != 0) 
        {
            npd_key_database_unlock();
			goto error;
		}
        port_link_state = (eth_ports->attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT;
        if(port_link_state == ETH_ATTR_LINKUP)
        {
            lacp_port_actor->is_enabled = 1; 
        }
        else
        {
            lacp_port_actor->is_enabled = 0; 
        }
        /* ****** private parameters *******/
        lacp_port_sm->sm_vars = AD_PORT_BEGIN;
		if(enabled)
		{
            lacp_port_sm->sm_vars |= AD_PORT_LACP_ENABLED;
		}
        lacp_port_sm->sm_rx_state = 0;
        lacp_port_sm->sm_rx_timer_counter = 0;
        lacp_port_sm->sm_periodic_state = 0;
        lacp_port_sm->sm_periodic_timer_counter = 0;
        lacp_port_sm->sm_mux_state = 0;
        lacp_port_sm->sm_mux_timer_counter = 0;
        lacp_port_sm->sm_tx_state = 0;
        lacp_port_sm->sm_tx_timer_counter = 0;
		lacp_port_actor->lacp_mode = lacp_mode;
        lacp_port_sm->netif_index = netif_index;
        dbtable_sequence_insert(lacpport_sm_index, netif_index, lacp_port_sm);
        lacp_port_actor->netif_index = netif_index;
        dbtable_sequence_insert(lacpport_actor_index, netif_index, lacp_port_actor);
        lacp_port_partner->netif_index = netif_index;
        dbtable_sequence_insert(lacpport_partner_index, netif_index, lacp_port_partner);
        npd_key_database_unlock();
    }
error:
    
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(eth_ports)
        free(eth_ports);
    return ret;
}
/**
 * dot3ad_enable_collecting_distributing - enable a port's transmit/receive
 * @port: the port we're looking at
 *
 * Enable @port if it's in an active aggregator
 */
static int dot3ad_enable_collecting_distributing(unsigned int  netif_index)
{  
    int ret = -1;
    aggregator_t   *aggregator = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;

    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    aggregator->netif_index = netif_index;
    ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);	
    if(ret != 0)
	{  
	    goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}

    npd_syslog_dbg("lacp:Enabling port %d(LAG %d)\r\n", lacp_port_actor->actor_port_number, aggregator->aggregator_identifier);
    __enable_port( netif_index);
    
    dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
error:
    
    if(aggregator)
        free(aggregator);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
}

/**
 * dot3ad_disable_collecting_distributing - disable a port's transmit/receive
 * @port: the port we're looking at
 *
 */
static int dot3ad_disable_collecting_distributing(unsigned int netif_index)
{ 
    int ret = -1;
    aggregator_t    *aggregator = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
    
    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    aggregator->netif_index = netif_index;
    ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);	
    if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}

    if (MAC_ADDRESS_COMPARE(&(aggregator->partner_system), &(null_mac_addr)))
    {
        npd_syslog_dbg("Disabling port netif_index = 0x%d(LAG %d)\r\n", 
        netif_index,aggregator->aggregator_identifier);
        __disable_port(netif_index);
    }
error:
    
    if(aggregator)
        free(aggregator);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
   
}

/**
 * dot3ad_initialize_lacpdu - initialize a given lacpdu structure
 * @lacpdu: lacpdu structure to initialize
 *
 */
static void dot3ad_initialize_lacpdu(lacpdu_t *lacpdu)
{
    unsigned short index;
    /* initialize lacpdu data*/
    lacpdu->subtype = 0x01;
    lacpdu->version_number = 0x01;
    lacpdu->tlv_type_actor_info = 0x01;
    lacpdu->actor_information_length = 0x14;
    /* lacpdu->actor_system_priority    updated on send*/
    /* lacpdu->actor_system             updated on send*/
    /* lacpdu->actor_key                updated on send*/
    /* lacpdu->actor_port_priority      updated on send*/
    /* lacpdu->actor_port               updated on send*/
    /* lacpdu->actor_state              updated on send*/
    lacpdu->tlv_type_partner_info = 0x02;
    lacpdu->partner_information_length = 0x14;

    for (index=0; index<=2; index++)
    {
        lacpdu->reserved_3_1[index]=0;
    }

    /* lacpdu->partner_system_priority  updated on send*/
    /* lacpdu->partner_system           updated on send*/
    /* lacpdu->partner_key              updated on send*/
    /* lacpdu->partner_port_priority    updated on send*/
    /* lacpdu->partner_port             updated on send*/

    /* lacpdu->partner_state            updated on send*/
    for (index=0; index<=2; index++)
    {
        lacpdu->reserved_3_2[index]=0;
    }

    lacpdu->tlv_type_collector_info = 0x03;
    lacpdu->collector_information_length= 0x10;
    lacpdu->collector_max_delay = AD_COLLECTOR_MAX_DELAY;

    for (index=0; index<=11; index++)
    {
        lacpdu->reserved_12[index]=0;
    }

    lacpdu->tlv_type_terminator = 0x00;
    lacpdu->terminator_length = 0;

    for (index=0; index<=49; index++)
    {
        lacpdu->reserved_50[index]=0;
    }
}

#define AD_AGGREGATOR_SELECTION_TIMER  8
//static unsigned short aggregator_identifier;

/**
 * dot3ad_state_machine_handler - handle state machines timeout
 * @bond: bonding struct to work on
 *
 * The state machine handling concept in this module is to check every tick
 * which state machine should operate any function. The execution order is
 * round robin, so when we have an interaction between state machines, the
 * reply of one to each other might be delayed until next tick.
 *
 * This function also complete the initialization when the agg_select_timer
 * times out, and it selects an aggregator for the ports that are yet not
 * related to any aggregator, and selects the active aggregator for a bond.
 */
int dot3ad_state_machine_handler()
{
    int ret = -1;
    aggregator_t *aggregator = NULL;
    aggregator_t item, item_out;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_sm_t *lacp_port_sm = NULL;
    unsigned int trunkId = 0;
    unsigned int eth_g_index = 0;
    unsigned int array_port = 0;
    unsigned int type = 0;
    struct trunk_s *trunkNode = NULL;
    struct npd_lacp_cfg_s npd_lacp_cfg_get;

    if(!SYS_MODULE_ISMASTERACTIVE(SYS_LOCAL_MODULE_SLOT_INDEX))
        return 0;
    ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_lacp_enable");
		 goto error;
	}
    g_lacp_enable = npd_lacp_cfg_get.lacp_enable;
    if(g_lacp_enable == NPD_FALSE)
    {
        goto error;
    }
    trunkNode = malloc(sizeof(trunk_t));
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if((NULL == trunkNode) || (lacp_port_actor == NULL) \
        || (lacp_port_sm == NULL))
	{   
	    goto error;
	}
    memset(&item, 0, sizeof(struct aggregator));
    memset(&item_out, 0, sizeof(struct aggregator));

	pthread_mutex_lock(&lacp_act_sync);

    while(1)
    {
        ret = dbtable_hash_next(aggregator_index, &item, &item_out, NULL);
        if(0 != ret)
        {
            break;
        }
        
        item = item_out;
        aggregator = &item ;
        type = npd_netif_type_get(aggregator->netif_index);
        npd_syslog_dbg("type = %d agg->netif_index = 0X%x",type, aggregator->netif_index);
        if(NPD_NETIF_TRUNK_TYPE == type)
        { 
            memset(trunkNode, 0, sizeof(struct trunk_s));
			trunkId = npd_netif_trunk_get_tid(aggregator->netif_index);
            trunkNode->trunk_id = trunkId;
            ret = dbtable_sequence_search(g_trunks, trunkId, trunkNode);
            if(ret != 0)
            {  
                continue;
            }
            if(trunkNode->aggregator_mode == MANUAL_MODE)
            {
                continue; 
            }
            npd_syslog_dbg("trunkId = %d mode = %d ", trunkId, trunkNode->aggregator_mode);
           // npd_syslog_dbg("debug: aggId=%d,trunkNode->aggregator_mode =%d", aggId, trunkNode->aggregator_mode);
            NPD_PBMP_ITER(trunkNode->ports, array_port)
            {
               
                eth_g_index = eth_port_array_index_to_ifindex(array_port);
                eth_g_index &= 0xFFFFC000;
                npd_syslog_dbg("debug: trunkNode eth_g_index = 0x%x" , eth_g_index);
                memset(lacp_port_actor, 0, sizeof(lacp_port_actor_t));
                lacp_port_actor->netif_index = eth_g_index;
                ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, lacp_port_actor);
                if(ret != 0)
                { 
                    goto error;
                }
               
                dot3ad_rx_machine(NULL, eth_g_index);
                dot3ad_periodic_machine(eth_g_index);
                dot3ad_port_selection_logic(eth_g_index);
                //dot3ad_mux_machine(eth_g_index);
                dot3ad_tx_machine(eth_g_index);
                lacp_port_sm->netif_index = eth_g_index;
                ret = dbtable_sequence_search(lacpport_sm_index, eth_g_index, lacp_port_sm);
                if(ret != 0)
                { 
                    goto error;
                }
    
                /* turn off the BEGIN bit, since we already handled it*/
                if (lacp_port_sm->sm_vars & AD_PORT_BEGIN)
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_BEGIN;
                    dbtable_sequence_update(lacpport_sm_index, eth_g_index, lacp_port_sm, lacp_port_sm);
                    
                }
            }
        }
        else if(NPD_NETIF_ETH_TYPE  == type)
        {
            eth_g_index = aggregator->netif_index;
            memset(lacp_port_actor, 0, sizeof(lacp_port_actor_t));
            lacp_port_actor->netif_index = eth_g_index;
            ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, lacp_port_actor);
            if(ret != 0)
            { 
                goto error;
            }
            if(lacp_port_actor->is_lacp_enable == NPD_TRUE)
			{
                dot3ad_rx_machine(NULL, eth_g_index);
                dot3ad_periodic_machine(eth_g_index);
                dot3ad_port_selection_logic(eth_g_index);
                //dot3ad_mux_machine(eth_g_index);
                dot3ad_tx_machine(eth_g_index);
                lacp_port_sm->netif_index = eth_g_index;
                ret = dbtable_sequence_search(lacpport_sm_index, eth_g_index, lacp_port_sm);
                if(ret != 0)
                { 
                    goto error;
                }
                /* turn off the BEGIN bit, since we already handled it*/
                if (lacp_port_sm->sm_vars & AD_PORT_BEGIN)
                {
                    lacp_port_sm->sm_vars &= ~AD_PORT_BEGIN;
                    dbtable_sequence_update(lacpport_sm_index, eth_g_index, lacp_port_sm, lacp_port_sm); 
                }
			}
        }
    }
    
error:
	pthread_mutex_unlock(&lacp_act_sync);
    
    if(trunkNode)
        free(trunkNode);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
}

/**
 * dot3ad_rx_indication - handle a received frame
 * @lacpdu: received lacpdu
 * @slave: slave struct to work on
 * @length: length of the data received
 *
 * It is assumed that frames that were sent on this NIC don't returned as new
 * received frames (loopback). Since only the payload is given to this
 * function, it check for loopback.
 */
static int  dot3ad_rx_indication(unsigned short vlan_id, unsigned int netif_index, unsigned int son_netif_index, lacpdu_t *lacpdu, unsigned short length)
{
    int ret = -1;
    unsigned int eth_g_index = 0;
    aggregator_t  *aggregator= NULL;
    lacp_port_actor_t   *lacp_port_actor = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;
    struct trunk_s *trunkNode = NULL;
    struct eth_port_s *eth_ports = NULL;
    struct npd_lacp_cfg_s npd_lacp_cfg_get;
    ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_lacp_enable");
		 goto error;
	}
    g_lacp_enable = npd_lacp_cfg_get.lacp_enable;
    if(g_lacp_enable == NPD_FALSE)
    {
        goto error;
    }
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
    aggregator = malloc(sizeof(aggregator_t));
    trunkNode = malloc(sizeof(trunk_t));
    eth_ports = malloc(sizeof(struct eth_port_s));
    lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
    if((NULL == trunkNode) || (aggregator == NULL) || (lacp_port_actor == NULL) 
       || (lacp_port_partner == NULL) || (NULL == eth_ports))
	{   
	    goto error;
	}
    if(npd_netif_type_get(netif_index) == NPD_NETIF_ETH_TYPE)
    {
		eth_g_index = netif_index;
        npd_syslog_dbg("LACP rx indication from netif 0x%x\r\n", eth_g_index);
    }
	else if(npd_netif_type_get(netif_index) == NPD_NETIF_TRUNK_TYPE)
	{
        npd_syslog_dbg("LACP rx indication from netif 0x%x, son netif 0x%x\r\n", netif_index, son_netif_index);
		if(npd_netif_type_get(son_netif_index) == NPD_NETIF_ETH_TYPE)
		{
			eth_g_index = son_netif_index;
		}
		else
		{
		    goto error;
		}
	}
	
    lacp_port_actor->netif_index = eth_g_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, lacp_port_actor);
	if(ret != 0)
	{   
        goto error;
	}
    
    if (lacp_port_actor->is_lacp_enable == FALSE)
    {
        ret = 0;
		goto error;
    }

    if (length >= sizeof(lacpdu_t))
    {
        switch (lacpdu->subtype)
        {
            case AD_TYPE_LACPDU:
                dot3ad_rx_machine(lacpdu, eth_g_index);
                break;
            case AD_TYPE_MARKER:
                break;
                /* No need to convert fields to Little Endian since we don't use the marker's fields.*/
                #if 0
                switch (((struct marker *)lacpdu)->tlv_type)
                {
                    case AD_MARKER_INFORMATION_SUBTYPE:
                        npd_syslog_dbg("Received Marker Information on port %d\r\n", lacp_port_actor->actor_port_number);
                        //dot3ad_marker_info_received((struct marker *)lacpdu, port);
                        break;
                    case AD_MARKER_RESPONSE_SUBTYPE:
                        npd_syslog_dbg("Received Marker Response on port %d\r\n", lacp_port_actor->actor_port_number);
                        //dot3ad_marker_response_received((struct marker *)lacpdu, port);
                        break;
                    default:
                        npd_syslog_dbg("Received an unknown Marker subtype on slot %d\r\n", lacp_port_actor->actor_port_number);
                }
                #endif
        }
    }
    else
    {
        npd_syslog_dbg("Bad LACP packet length: %d\r\n", length);
    }
error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(aggregator)
        free(aggregator);
    if(trunkNode)
        free(trunkNode);
    if(eth_ports)
       free(eth_ports);
    if(lacp_port_partner)
        free(lacp_port_partner);
    return ret;
}

/**
 * dot3ad_adapter_speed_changed - handle a slave's speed change indication
 * @slave: slave struct to work on
 *
 * Handle reselection of aggregator (if needed) for this port.
 */
static int dot3ad_adapter_speed_changed(unsigned int netif_index)
{
    int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
    
    lacp_port_actor->actor_admin_port_key &= ~AD_SPEED_KEY_BITS;
    lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= (__get_link_speed(netif_index) << 1);
    npd_syslog_dbg("Port (netif_index = %x) changed speed\r\n", netif_index);
    /* there is no need to reselect a new aggregator, just signal the*/
    /* state machines to reinitialize*/
    lacp_port_sm->sm_vars |= AD_PORT_BEGIN;
    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
error:
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
}

/**
 * dot3ad_adapter_duplex_changed - handle a slave's duplex change indication
 * @slave: slave struct to work on
 *
 * Handle reselection of aggregator (if needed) for this port.
 */
int  dot3ad_adapter_duplex_changed(unsigned long netif_index)
{
    int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
	
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
    
    lacp_port_actor->actor_admin_port_key &= ~AD_DUPLEX_KEY_BITS;
    lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= __get_duplex(netif_index);
    npd_syslog_dbg("Port (netif_index = %x) changed duplex\r\n", netif_index);
    /* there is no need to reselect a new aggregator, just signal the*/
    /* state machines to reinitialize*/
    lacp_port_sm->sm_vars |= AD_PORT_BEGIN;
    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
error:
    
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    return ret;
}
#if 1
/**
 * dot3ad_handle_link_change - handle a slave's link status change indication
 * @slave: slave struct to work on
 * @status: whether the link is now up or down
 *
 * Handle reselection of aggregator (if needed) for this port.
 */
int dot3ad_handle_link_change(unsigned int netif_index, enum PORT_NOTIFIER_ENT event)
{
    int ret = -1;
    int op_ret = 0;
    unsigned int    actor_update_flag = 0;
    unsigned int    trunk_g_index = 0;
    unsigned int     eth_agg = 1;
    aggregator_t    *aggregator = NULL;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;
 
    aggregator = malloc(sizeof(aggregator_t));
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	
    if((lacp_port_sm == NULL) || (lacp_port_actor == NULL) || (lacp_port_partner == NULL) || (aggregator == NULL))
    {
        goto error;
    }
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
	lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
	lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{
        goto error;
	}
   
    aggregator->netif_index = netif_index;
	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
	if(ret != 0)
	{  
        trunk_g_index = lacp_port_actor->trunk_netif_index;
        if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(trunk_g_index)) 
        {
            eth_agg = 0;
        }
    }
   
  npd_syslog_dbg("dot3ad_handle_link_change  event = %d", event);
  if(event == PORT_NOTIFIER_LINKUP_E)
   {   
        lacp_port_actor->is_enabled = 1;
        lacp_port_actor->actor_admin_port_key = 0;
        lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= __get_duplex(netif_index);
        //lacp_port_actor->actor_admin_port_key &= ~AD_SPEED_KEY_BITS;
        lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= (__get_link_speed(netif_index) << 1);
        lacp_port_actor->actor_oper_port_state &= ~AD_STATE_LACP_TIMEOUT;
        //only the aggregator of port can be updated
        if(eth_agg)
        {
            aggregator->actor_oper_aggregator_key = lacp_port_actor->actor_oper_port_key;
        }
        npd_syslog_dbg("Port(netif_index = %x)link up\r\n", netif_index);
    }
    else if(event == PORT_NOTIFIER_LINKDOWN_E)
    {
        /* link has failed */
        lacp_port_actor->is_enabled = 0;
       
        lacp_port_actor->actor_admin_port_key = 0;
        lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= __get_duplex(netif_index);
       
        lacp_port_actor->actor_oper_port_key = lacp_port_actor->actor_admin_port_key |= (__get_link_speed(netif_index) << 1);
        lacp_port_actor->actor_oper_port_state = 0;
        lacp_port_actor->actor_oper_port_state |= (AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY);
        lacp_port_actor->actor_oper_port_state &= ~AD_STATE_LACP_TIMEOUT;
        lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;

        lacp_port_partner->partner_admin_system = null_mac_addr;
        lacp_port_partner->partner_oper_system  = null_mac_addr;
        lacp_port_partner->partner_admin_system_priority = 0xffff;
        lacp_port_partner->partner_oper_system_priority  = 0xffff;
        lacp_port_partner->partner_admin_key = 1;
        lacp_port_partner->partner_oper_key  = 0;
        lacp_port_partner->partner_admin_port_number = 1;
        lacp_port_partner->partner_oper_port_number  = 1;
        lacp_port_partner->partner_admin_port_priority = 0xff;
        lacp_port_partner->partner_oper_port_priority  = 0xff;
        lacp_port_partner->partner_admin_port_state = 0;  // 1
        lacp_port_partner->partner_oper_port_state  = 0;
        //only the aggregator of port can be updated
        if(eth_agg)
        {
            aggregator->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
            aggregator->actor_oper_aggregator_key = lacp_port_actor->actor_oper_port_key;
            aggregator->partner_oper_aggregator_key = lacp_port_partner->partner_oper_key;
        }
        npd_syslog_dbg("Port (netif_index = 0X%x)link failed\r\n", netif_index);
    }
    /*BOND_PRINT_DBG(("Port %d changed link status to %s", port->actor_port_number, ((link == BOND_LINK_UP)?"UP":"DOWN")));*/
    /* there is no need to reselect a new aggregator, just signal the*/
    /* state machines to reinitialize*/
    lacp_port_sm->sm_vars |= AD_PORT_BEGIN;
   
    if(eth_agg)
    {
        /* if port was responsed from the end-user*/
        if (lacp_port_actor->actor_oper_port_key & AD_DUPLEX_KEY_BITS)  /* if port is full duplex*/
        { 
            aggregator->is_individual = 0;
        }
        else
        {
            npd_syslog_dbg("is_individual = 1");
            aggregator->is_individual = 1;
        }
        dbtable_hash_update(aggregator_index,  aggregator,  aggregator);
    }
    else
    {
        //update  static aggregator
        npd_syslog_dbg("link change: static aggregator");
        memset(aggregator, 0, sizeof(aggregator_t));
        trunk_g_index = lacp_port_actor->trunk_netif_index;
        aggregator->netif_index = trunk_g_index;
    	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
    	if(ret != 0)
    	{  
            goto error;
        }
        if(STATIC_MODE == aggregator->aggregator_mode)
        {
            npd_syslog_dbg("link change: update actor state");
            lacp_port_actor->actor_oper_port_state |= (AD_STATE_COLLECTING  | AD_STATE_DISTRIBUTING);
            dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
            actor_update_flag = 1;
            op_ret = static_aggregator_update(trunk_g_index);
            if(op_ret == TRUNK_RETURN_CODE_ERR_GENERAL)
            {
               ret = -1;
               npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "static_aggregator_update");
               goto error;
            }
      
        }
    }
    dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    if(!actor_update_flag)
    {
        dbtable_sequence_update(lacpport_actor_index, netif_index, lacp_port_actor, lacp_port_actor);
    }
    dbtable_sequence_update(lacpport_partner_index, netif_index, lacp_port_partner, lacp_port_partner);
error:

    if(aggregator)
        free(aggregator);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);   
    return ret;
}

#endif
#if 0
int dot3ad_xmit_xor(struct sk_buff *skb, struct net_device *dev)
{

    return 0;
}
#endif 

#if 0 
int dot3ad_lacpdu_recv(struct sk_buff *skb)
{
    int ret = 0;
    //dot3ad_rx_indication(skb->vlan.v_vid, skb->unPhyIfIndex, (lacpdu_t *) skb->data, skb->len);
    return ret;
}
#endif 
#if 0
int manual_aggregator_init(unsigned long trunk_netif_index)
{  
    int ret = -1;
	ret = npd_lacp_aggregator_init(trunk_netif_index, MANUAL_MODE);
	return ret;
}

int manual_aggregator_join(unsigned long trunk_netif_index, unsigned long eth_netif_index)
{
    int ret = -1;
    int trunkId = 0;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_manual = NULL ;
    aggregator_t    *agg_original = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_actor_t *lacp_master_port_actor = NULL;
    lacp_port_sm_t *lacp_port_sm = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;
    unsigned int    eth_master_netif_index;

    agg_manual = malloc(sizeof(aggregator_t));
	if(agg_manual == NULL)
	{
        goto error;
	}
    agg_original = malloc(sizeof(aggregator_t));
	if(agg_original == NULL)
	{
        goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    lacp_master_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_master_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {
        goto error;
	}

    //删除eth_netif_index 原来的aggregator  及lacp_port_actor、lacp_port_sm、lacp_port_partner
    agg_original->netif_index = eth_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_original, NULL, agg_original);
	if(ret == 0)
	{  
	    ret = npd_lacp_aggregator_delete(eth_netif_index);
        if(ret !=0)
        {
            goto error;
        }
        
	    
	}
    lacp_port_actor->netif_index = eth_netif_index;
    dbtable_sequence_delete(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    lacp_port_partner->netif_index = eth_netif_index;
    dbtable_sequence_delete(lacpport_partner_index, eth_netif_index, lacp_port_partner, lacp_port_partner);
    lacp_port_sm->netif_index = eth_netif_index;
    dbtable_sequence_delete(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm);

    //if the aggregator of trunk does not exist,create it.
    agg_manual->netif_index = trunk_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);
	if(ret != 0)
	{  
	    ret = manual_aggregator_init(trunk_netif_index);
        if(ret != 0)
        {
            goto error;
        }
        ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);
        if(ret != 0)
        {
            goto error;
        }
	}
    
   //把端口加入trunk中并初始化
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        goto error;
    }
    ret = npd_trunk_port_add(trunkId,eth_netif_index);
    if(ret != 0)
    {
        goto error;
    }
    //更新aggregator及lacp_port_actor
    dot3ad_initialize_port(eth_netif_index, TRUE, AD_ACTIVE, FALSE);
    lacp_port_actor->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
    eth_master_netif_index = trunkNode->master_port_index;
    ret = dbtable_sequence_search(lacpport_actor_index, eth_master_netif_index, lacp_master_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
	agg_manual->num_of_ports++;
    agg_manual->actor_admin_aggregator_key = lacp_master_port_actor->actor_admin_port_key;
    memcpy(agg_manual->aggregator_mac_address.mac_addr_value, lacp_port_actor->actor_system.mac_addr_value , 6);
	if(agg_manual->num_of_ports == 1)
	{
		agg_manual->is_individual = TRUE;
	}
    lacp_port_actor->actor_oper_port_state |= (AD_STATE_COLLECTING | AD_STATE_DISTRIBUTING | AD_STATE_AGGREGATION);
   	lacp_port_actor->actor_port_aggregator_identifier = agg_manual->aggregator_identifier;
    lacp_port_actor->is_lacp_enable = FALSE;
    dbtable_hash_update(aggregator_index, agg_manual, agg_manual);
    dbtable_sequence_update(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    
error:

    if(agg_original)
        free(agg_original);
    if(agg_manual)
        free(agg_manual);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_master_port_actor)
        free(lacp_master_port_actor);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(trunkNode)
        free(trunkNode);
	return ret;
}

int manual_aggregator_leave(unsigned long trunk_netif_index, unsigned long eth_netif_index)
{
	int ret = -1;
    int trunkId;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_manual = NULL;
    lacp_port_sm_t  *lacp_port_sm = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_partner_t *lacp_port_partner= NULL;

    agg_manual = malloc(sizeof(aggregator_t));
	if(agg_manual == NULL)
	{
        goto error;
	}
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{   
	    goto error;
	}
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{   
	    goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {
        goto error;
	}

    agg_manual->netif_index = trunk_netif_index;
    ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);	
    if(ret != 0)
	{  
        goto error;
	}
    //从trunk中删除端口
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        goto error;
    }
    ret = npd_trunk_port_del(trunkId,eth_netif_index);
    if(ret != 0)
    {
        goto error;
    }
   //删除eth_netif_index 的lacp_port_actor、lacp_port_sm、lacp_port_partner
    lacp_port_actor->netif_index = eth_netif_index;
    ret = dbtable_sequence_delete(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    if(ret != 0)
	{  
        goto error;
	}
    lacp_port_partner->netif_index = eth_netif_index;
    ret = dbtable_sequence_delete(lacpport_partner_index, eth_netif_index, lacp_port_partner, lacp_port_partner);
    if(ret != 0)
	{  
        goto error;
	}
    lacp_port_sm->netif_index = eth_netif_index;
    ret = dbtable_sequence_delete(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm);
    if(ret != 0)
	{  
        goto error;
	}
    //更新aggregator
	agg_manual->num_of_ports--;
	if(agg_manual->num_of_ports == 1)
	{
		agg_manual->is_individual = TRUE;
	}
    if(agg_manual->num_of_ports == 0)
	{
		ret = npd_lacp_aggregator_delete(trunk_netif_index);
        if(ret != 0)
	    {  
            goto error;
	    }
     
	}
    dbtable_hash_update(aggregator_index, agg_manual, agg_manual);
error:
    
    if(agg_manual)
        free(agg_manual);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(trunkNode)
        free(trunkNode);
	return ret;
}
#endif
unsigned short port_stats_machine_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short value;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        return -1;
	}
    
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{   
        free(lacp_port_sm);
		return -1;
	}
    
	value = lacp_port_sm->sm_vars;
    free(lacp_port_sm);
	return value;
}

int static_aggregator_join(unsigned int trunk_netif_index, unsigned int eth_netif_index)
{
	int ret = -1;
    int trunkId;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_manual = NULL;
    aggregator_t    *agg_original = NULL;
    lacp_port_sm_t  *lacp_port_sm = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_actor_t *lacp_master_port_actor = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;
    unsigned int   master_netif_index;
    unsigned int   found = 1;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{   
        goto error;
	}
    agg_manual = malloc(sizeof(aggregator_t));
	if(agg_manual == NULL)
	{   
	   	goto error;
	}
    agg_original = malloc(sizeof(aggregator_t));
	if(agg_original == NULL)
	{
        goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    lacp_master_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_master_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {   
        goto error;
	}
    //delete the aggregator of  the port
    agg_original->netif_index = eth_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_original, NULL, agg_original);
	if(ret == 0)
	{  
	    ret = npd_lacp_aggregator_delete(eth_netif_index);
        if(ret !=0)
        {
            goto error;
        }
	}
    //if the aggregator of trunk does not exist,create it.
    agg_manual->netif_index = trunk_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);
	if(ret != 0)
	{   
        ret = npd_lacp_aggregator_init(trunk_netif_index, STATIC_MODE);
        if(ret != 0)
        {
            goto error;
        }
        ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);
        if(ret != 0)
        {
            goto error;
        }
	}
    //find the trunk 
    npd_key_database_lock();
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
		npd_key_database_unlock();
        goto error;
    }
    //对非master的端口进行初始化
    master_netif_index = trunkNode->master_port_index;
    if(master_netif_index != eth_netif_index)
    {
        //删除eth_netif_index 的lacp_port_actor、lacp_port_sm、lacp_port_partner
        lacp_port_actor->netif_index = eth_netif_index;
        dbtable_sequence_delete(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
        lacp_port_partner->netif_index = eth_netif_index;
        dbtable_sequence_delete(lacpport_partner_index, eth_netif_index, lacp_port_partner, lacp_port_partner);
        lacp_port_sm->netif_index = eth_netif_index;
        dbtable_sequence_delete(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm);
        dot3ad_initialize_port(eth_netif_index, FALSE, AD_ACTIVE, TRUE);
    }
    //如果找不到master，则初始化
    lacp_master_port_actor->netif_index = master_netif_index;
    ret = dbtable_sequence_search(lacpport_actor_index, master_netif_index, lacp_master_port_actor);
	if(ret != 0)
	{  
        dot3ad_initialize_port(master_netif_index, FALSE, AD_ACTIVE, TRUE);

        found = 0;
	}
    
    //更新aggregator及lacp_port_actor、lacp_port_sm
    lacp_port_sm->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, eth_netif_index, lacp_port_sm);
	if(ret != 0)
	{  
		npd_key_database_unlock();
        goto error;
	}
    lacp_port_actor->netif_index = eth_netif_index;
    ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
		npd_key_database_unlock();
        goto error;
	}

    if(!found)
    {
        lacp_master_port_actor->netif_index = master_netif_index;
        ret = dbtable_sequence_search(lacpport_actor_index, master_netif_index, lacp_master_port_actor);
    	if(ret != 0)
    	{  
    	    npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "The master port do not exists.");
		    npd_key_database_unlock();
            goto error;
    	}
    }
	agg_manual->num_of_ports++;
    agg_manual->actor_admin_aggregator_key = lacp_master_port_actor->actor_admin_port_key;
    npd_system_get_basemac(agg_manual->aggregator_mac_address.mac_addr_value, 6);
	if(agg_manual->num_of_ports == 1)
	{
		agg_manual->is_individual = TRUE;
	}
    lacp_port_actor->actor_oper_port_state |= (AD_STATE_COLLECTING | AD_STATE_DISTRIBUTING | AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY);
   	lacp_port_actor->actor_port_aggregator_identifier = agg_manual->aggregator_identifier;
    lacp_port_actor->is_lacp_enable = TRUE;
    lacp_port_actor->trunk_netif_index = trunk_netif_index;
    lacp_port_sm->sm_tx_timer_counter = dot3ad_ticks_per_sec/AD_MAX_TX_IN_SECOND;
    lacp_port_sm->sm_vars |= AD_PORT_BEGIN | AD_PORT_LACP_ENABLED;
    
    dbtable_hash_update(aggregator_index, agg_manual, agg_manual);
    dbtable_sequence_update(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm);
    dbtable_sequence_update(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    trunkNode->aggregator_mode = STATIC_MODE;
    dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
    ret = npd_trunk_port_add(trunkId,eth_netif_index);
    npd_key_database_unlock();
error:
    
    if(agg_manual)
        free(agg_manual);
    if(agg_original)
        free(agg_original);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_master_port_actor)
        free(lacp_master_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(trunkNode)
        free(trunkNode);
	return ret;
}

int static_aggregator_leave(unsigned int trunk_netif_index, unsigned int eth_netif_index)
{
	int ret = -1;
    int trunkId;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_manual = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    
    agg_manual = malloc(sizeof(aggregator_t));
	if(agg_manual == NULL)
	{   
	   	goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {   
        goto error;
	}
    agg_manual->netif_index = trunk_netif_index;
    ret = dbtable_hash_search(aggregator_index, agg_manual, NULL, agg_manual);	
    if(ret != 0)
	{  
        goto error;
    }
    if(agg_manual->aggregator_mode != STATIC_MODE)
    {
        goto error;
    }
    //从trunk中删除端口
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        goto error;
    }
    npd_trunk_port_del(trunkId,eth_netif_index);
    lacp_port_actor->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
  
    lacp_port_actor->actor_oper_port_state |= AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY;
    //从trunk的aggregator中移除端口并生成一个新的aggregator；
    lacp_port_actor->is_lacp_enable = TRUE;
    lacp_port_actor->trunk_netif_index= FALSE;
    dbtable_sequence_update(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    ret = npd_lacp_aggregator_init(eth_netif_index, DYNAMIC_MODE);
    if(ret != 0)
    {
        goto error;
    }
    //更新aggregator
	agg_manual->num_of_ports--;
    if(agg_manual->num_of_ports == 1)
	{ 
        agg_manual->is_individual = TRUE;
	}
    
    if(agg_manual->num_of_ports == 0)
	{
		ret = npd_lacp_aggregator_delete(trunk_netif_index);
        if(ret != 0)
	    {  
	        npd_syslog_dbg("The aggregator deletes fail");
	    }
        goto error;
	}
    ret = dbtable_hash_update(aggregator_index, agg_manual, agg_manual);
error:
    
    if(agg_manual)
        free(agg_manual);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(trunkNode)
        free(trunkNode);
	return ret;
}
int static_aggregator_update(unsigned int trunk_netif_index)
{ 
    int ret = -1;
    int  op_ret = TRUNK_RETURN_CODE_ERR_NONE;
    unsigned int master_g_index = 0;
    unsigned short  trunk_id = 0;
    unsigned int    update = 0;
    aggregator_t    *aggregator = NULL;
	lacp_port_actor_t *lacp_port_actor = NULL;
	lacp_port_partner_t *lacp_port_partner = NULL;
 

    aggregator = malloc(sizeof(aggregator_t));
	lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	
    if((lacp_port_actor == NULL) || (lacp_port_partner == NULL) || (aggregator == NULL))
    {
        op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
        goto error;
    }
     npd_syslog_dbg("ENTER: static_aggregator_update"); 
    aggregator->netif_index = trunk_netif_index;
	ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
	if(ret != 0)
	{  
	    op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
         npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "aggregator"); 
        goto error;
    }
    if(STATIC_MODE != aggregator->aggregator_mode)
    {
        op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
        npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "aggregator->aggregator_mode = STATIC");
        goto error;    
    }
    
    //if the master port changes, update the aggregator.
    trunk_id = npd_netif_trunk_get_tid(trunk_netif_index); 
    ret = npd_trunk_master_port_get(trunk_id, &master_g_index);
    if(ret == NPD_FALSE)
    {
        op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
        npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "npd_trunk_master_port_get");
        goto error;
    }
    lacp_port_actor->netif_index = master_g_index;
	ret = dbtable_sequence_search(lacpport_actor_index, master_g_index, lacp_port_actor);
	if(ret != 0)
	{
	    op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
        npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "lacp_port_actor");
        goto error;
	}
    lacp_port_partner->netif_index = master_g_index;
    ret = dbtable_sequence_search(lacpport_partner_index, master_g_index, lacp_port_partner);
    if(ret != 0)
    {
        op_ret = TRUNK_RETURN_CODE_ERR_GENERAL;
         npd_syslog_dbg("ret is not 0: %s(%d): %s",
                               __FILE__, __LINE__, "lacp_port_partner");
        goto error;
    }
    if(aggregator->partner_system_priority != lacp_port_partner->partner_oper_system_priority)
    {
        aggregator->partner_system_priority = lacp_port_partner->partner_oper_system_priority;
        update = 1;
    }
    if(aggregator->actor_oper_aggregator_key != lacp_port_actor->actor_oper_port_key)
    {
        aggregator->actor_oper_aggregator_key = lacp_port_actor->actor_oper_port_key;
        update = 1;
    }
    if(aggregator->partner_oper_aggregator_key != lacp_port_partner->partner_oper_key)
    {
        aggregator->partner_oper_aggregator_key = lacp_port_partner->partner_oper_key;
        update = 1;
    }
    if(MAC_ADDRESS_COMPARE(&(aggregator->partner_system), &(lacp_port_partner->partner_oper_system)))
    {
        aggregator->partner_system = lacp_port_partner->partner_oper_system;
        update = 1;
    }
    
    #if 0
    if (lacp_port_actor->actor_oper_port_key & AD_DUPLEX_KEY_BITS)  /* if port is full duplex*/
    { 
        aggregator->is_individual = 0;
    }
    else
    {
        npd_syslog_dbg("is_individual = 1");
        aggregator->is_individual = 1;
    }
    #endif
    
    if(update)
    {
        dbtable_hash_update(aggregator_index,  aggregator,  aggregator);
    }
error:

    if(aggregator)
        free(aggregator);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
     
    return op_ret;       
	
}
int dynamic_aggregator_join(unsigned int trunk_netif_index, unsigned int eth_netif_index)
{
    int ret = -1;
    int trunkId = 0;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_dynamic = NULL;
    aggregator_t    *agg_original = NULL;
    lacp_port_sm_t  *lacp_port_sm = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_partner_t *lacp_port_partner = NULL;

    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{   
        goto error;
	}
    agg_dynamic = malloc(sizeof(aggregator_t));
	if(agg_dynamic == NULL)
	{   
	   	goto error;
	}
    agg_original = malloc(sizeof(aggregator_t));
	if(agg_original == NULL)
	{
        goto error;
	}
	lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {   
        goto error;
	}
    
    lacp_port_actor->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_partner->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, eth_netif_index, lacp_port_partner);
	if(ret != 0)
	{  
        goto error;
	}
    //delete the aggregator of this port
    agg_original->netif_index = eth_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_original, NULL, agg_original);
	if(ret == 0)
	{  
	    ret = npd_lacp_aggregator_delete(eth_netif_index);
        if(ret !=0)
        {
            goto error;
        }   
	}
    //把端口加入trunk中并初始化
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        goto error;
    }
    ret = npd_trunk_port_add(trunkId,eth_netif_index);
    if(ret != 0)
    {
        goto error;
    }
    //if the aggregator of trunk does not exist,return false.
    agg_dynamic->netif_index = trunk_netif_index;
	ret = dbtable_hash_search(aggregator_index, agg_dynamic, NULL, agg_dynamic);
	if(ret != 0)
	{  
        goto error;
    }
    lacp_port_sm->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, eth_netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
    //更新aggregator及lacp_port_actor、lacp_port_sm
    if(agg_dynamic->aggregator_identifier > agg_original->aggregator_identifier)
    {
        lacp_port_sm->sm_rx_timer_counter++;
    }

	agg_dynamic->num_of_ports++;
	
    lacp_port_actor->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_actor->actor_oper_port_state |= (AD_STATE_COLLECTING | AD_STATE_DISTRIBUTING | AD_STATE_AGGREGATION  | AD_STATE_SYNCHRONIZATION | AD_STATE_LACP_ACTIVITY);
   	lacp_port_actor->actor_port_aggregator_identifier = agg_dynamic->aggregator_identifier;
    lacp_port_actor->is_lacp_enable = TRUE;
    lacp_port_actor->trunk_netif_index = trunk_netif_index;
    lacp_port_sm->sm_tx_timer_counter = dot3ad_ticks_per_sec/AD_MAX_TX_IN_SECOND;
    lacp_port_sm->sm_vars |= AD_PORT_LACP_ENABLED | AD_PORT_SELECTED;       //AD_PORT_BEGIN |
    dbtable_hash_update(aggregator_index, agg_dynamic, agg_dynamic);
    dbtable_sequence_update(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm);
    dbtable_sequence_update(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);

error:

    if(agg_dynamic)
        free(agg_dynamic);
    if(agg_original)
        free(agg_original);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(lacp_port_partner)
        free(lacp_port_partner);
    if(lacp_port_sm)
        free(lacp_port_sm);
    if(trunkNode)
        free(trunkNode);
	return ret;
}

int dynamic_aggregator_leave(unsigned int trunk_netif_index, unsigned int eth_netif_index)
{
	int ret = -1;
    int trunkId = 0;
    unsigned int eth_g_index = 0;
    unsigned int array_port = 0;
    struct trunk_s  *trunkNode = NULL;
    aggregator_t    *agg_dynamic = NULL;
    lacp_port_actor_t *lacp_port_actor = NULL;
    lacp_port_sm_t    *lacp_port_sm = NULL;

    agg_dynamic = malloc(sizeof(aggregator_t));
	if(agg_dynamic == NULL)
	{   
	   	goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
	if(NULL == trunkNode) 
    {   
        goto error;
	}
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
    lacp_port_sm->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, eth_netif_index, lacp_port_sm);
	if(ret != 0)
	{  
        goto error;
	}
    agg_dynamic->netif_index = trunk_netif_index;
    ret = dbtable_hash_search(aggregator_index, agg_dynamic, NULL, agg_dynamic);	
    if(ret != 0)
	{  
        goto error;
    }
    if(agg_dynamic->aggregator_mode != DYNAMIC_MODE)
    {
        goto error;
    }
    //delete the port from the trunk
    trunkId = npd_netif_trunk_get_tid(trunk_netif_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        npd_syslog_dbg("%s %d: Port-channel %d not found\r\n" , __func__, __LINE__, trunkId);
    }
    ret = npd_trunk_port_del(trunkId,eth_netif_index);
    if(ret != 0)
    {
        npd_syslog_dbg("%s %d: Delete port 0x%x from port-channel %d failed.r\n" , __func__, __LINE__, eth_netif_index, trunkId);
    }
    //update lacp_port_actor
    lacp_port_actor->netif_index = eth_netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_netif_index, lacp_port_actor);
	if(ret != 0)
	{  
        goto error;
	}
    lacp_port_actor->actor_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;
    lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
    lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
    lacp_port_actor->actor_oper_port_state |= (AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY);
    lacp_port_actor->is_lacp_enable = TRUE;
    lacp_port_actor->trunk_netif_index = 0;
    lacp_port_sm->ntt = TRUE;
    #if 0
    lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
    #endif
    //update aggregator
	agg_dynamic->num_of_ports--;
    dbtable_hash_update(aggregator_index, agg_dynamic, agg_dynamic);
    dbtable_sequence_update(lacpport_actor_index, eth_netif_index, lacp_port_actor, lacp_port_actor);
    dbtable_sequence_update(lacpport_sm_index, eth_netif_index, lacp_port_sm, lacp_port_sm); 
    //create a aggregator for the port 
    ret = npd_lacp_aggregator_init(eth_netif_index, DYNAMIC_MODE);
    if(ret != 0)
    {
        goto error;
    }
    if(agg_dynamic->num_of_ports == 1)
	{
        NPD_PBMP_ITER(trunkNode->ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            eth_g_index &= 0xFFFFC000;
            if(eth_netif_index == eth_g_index)
            {
                continue;
            }
            ret = npd_lacp_aggregator_init(eth_g_index, DYNAMIC_MODE);
            if(ret != 0)
            {
                goto error;
            }
            ret = npd_trunk_destroy_node(trunkId);
            if(ret != 0)
            {
                goto error;
            }
            ret = npd_lacp_aggregator_delete(trunk_netif_index);
            if(ret != 0)
            {
                goto error;
            }
            
            memset(lacp_port_actor, 0, sizeof(lacp_port_actor_t));
            memset(lacp_port_sm, 0, sizeof(lacp_port_sm_t));
            lacp_port_actor->netif_index = eth_g_index;
        	ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, lacp_port_actor);
        	if(ret != 0)
        	{  
				npd_syslog_dbg("%s %d: Actor for 0x%x not found\r\n" , __func__, __LINE__, eth_g_index);
                goto error;
        	}
            lacp_port_sm->netif_index = eth_g_index;
        	ret = dbtable_sequence_search(lacpport_sm_index, eth_g_index, lacp_port_sm);
        	if(ret != 0)
        	{  
				npd_syslog_dbg("%s %d: State machine for 0x%x not found\r\n" , __func__, __LINE__, eth_g_index);
                goto error;
        	}
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_SYNCHRONIZATION;
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_COLLECTING;
            lacp_port_actor->actor_oper_port_state &= ~AD_STATE_DISTRIBUTING;
            lacp_port_actor->actor_oper_port_state |= (AD_STATE_AGGREGATION | AD_STATE_LACP_ACTIVITY);
            lacp_port_actor->is_lacp_enable = TRUE;
            lacp_port_actor->trunk_netif_index = 0;
            lacp_port_sm->ntt = TRUE;
            dbtable_sequence_update(lacpport_actor_index, eth_g_index, lacp_port_actor, lacp_port_actor);
            dbtable_sequence_update(lacpport_sm_index, eth_g_index, lacp_port_sm, lacp_port_sm);   
			npd_syslog_dbg("%s %d: Update actor for 0x%x\r\n" , __func__, __LINE__, eth_g_index);  
        }
       
	} 
error:
    if(agg_dynamic)
        free(agg_dynamic);
    if(lacp_port_actor)
        free(lacp_port_actor);
    if(trunkNode)
        free(trunkNode);
    if(lacp_port_sm)
        free(lacp_port_sm);
	return ret;
}
unsigned short netif_manual_port_key_get(unsigned int netif_index)
{
    unsigned short value = 0;
    
    value |= __get_duplex(netif_index);
    value |= (__get_link_speed(netif_index) << 1);

    return value;
}
unsigned int netif_is_link_selected(unsigned int netif_index) 
{
	int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
    
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{   
        goto error;
	}
    
    if(lacp_port_sm->sm_vars & AD_PORT_SELECTED)
	{
	    free(lacp_port_sm);
        npd_syslog_dbg("Select the port 0X%x ", netif_index);
        return TRUE;
	}
	else
	{   
	    free(lacp_port_sm);
        npd_syslog_dbg("Unselect the port 0X%x ", netif_index);
		return FALSE;
	}
    
error:
    
    if(lacp_port_sm)
        free(lacp_port_sm);
    return ret;
}
unsigned short netif_link_priority_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short value = 0;
    lacp_port_actor_t *lacp_port_actor = NULL;
	
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{
        goto error;
	}
    
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_actor->actor_port_priority;
    npd_syslog_dbg("get the actor's port(0X%x) priority  %d ", netif_index, value);
error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
	return value;
}

unsigned short netif_link_actor_key_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short value = 0;
    lacp_port_actor_t   *lacp_port_actor = NULL;
	
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{
        goto error;
	}
    
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_actor->actor_oper_port_key;
    npd_syslog_dbg("Get the actor's port(0X%x ) key %d ", netif_index, value);
error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
	return value;
}

unsigned char netif_link_actor_state_get(unsigned int netif_index)
{
	int ret = -1;
     unsigned short value = 0;
    lacp_port_actor_t   *lacp_port_actor = NULL;
	
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{
        goto error;
	}
    
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_actor->actor_oper_port_state;
    npd_syslog_dbg("Get the actor's port(0X%x) state  %d ", netif_index, value);
 error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
	return value;
}

unsigned char* netif_link_actor_state_string(unsigned int netif_index)
{
	int i = 0, j = 0;
    int ret = -1;
	static unsigned char state_str[16];
	char alp_str[8] = {'A','B','C','D','E','F','G','H'};
    lacp_port_actor_t   *lacp_port_actor= NULL;
	
    lacp_port_actor= malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{
        goto error;
	}
    
    lacp_port_actor->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_actor_index, netif_index, lacp_port_actor);
	if(ret != 0)
	{   
        goto error;
	}

    memset(state_str, 0, 16);
	state_str[j++] = '{';
	for(i = 0; i < 8; i++)
	{
		if((lacp_port_actor->actor_oper_port_state >> i) & 0x1)
		{
			state_str[j++] = alp_str[i];
		}
	}
	state_str[j] = '}';
error:
    
    if(lacp_port_actor)
        free(lacp_port_actor);
	return state_str;
}

unsigned short netif_link_partner_key_get(unsigned int netif_index)
{ 
    int ret = -1;
    unsigned short value = 0;
    lacp_port_partner_t   *lacp_port_partner= NULL;
	
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_partner->partner_oper_key;
    npd_syslog_dbg("Get the partner's port(0X%x ) key %d ", netif_index, value);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return value;
}

unsigned short netif_link_partner_sys_priority_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short value = 0;
    lacp_port_partner_t   *lacp_port_partner= NULL;
	
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_partner->partner_oper_system_priority;
    npd_syslog_dbg("get the partner's port(0X%x) priority  %d ", netif_index, value);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return value;
	
}
int  netif_link_partner_sys_mac_get(unsigned int netif_index, mac_addr_t *value)
{
	int ret = -1;
    lacp_port_partner_t   *lacp_port_partner= NULL;
    
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    
    memset(value, 0, sizeof(mac_addr_t));
    if(NULL == value)
    {
        goto error;

    }
    memcpy(value, &(lacp_port_partner->partner_oper_system), 6);
    npd_syslog_dbg("Get the partner's port(0X%x) priority  mac ", netif_index);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return ret;
}

int  netif_link_partner_sys_mac_get_v1(unsigned int netif_index, unsigned char*value)
{
	int ret = -1; 
    lacp_port_partner_t   *lacp_port_partner= NULL;
    
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    
    memcpy(value, &(lacp_port_partner->partner_oper_system), 6);
    npd_syslog_dbg("Get the partner's port(0X%x) priority  mac ", netif_index);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return ret;
}
unsigned short netif_link_partner_port_priority_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short  value = 0;
    lacp_port_partner_t   *lacp_port_partner= NULL;
	
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_partner->partner_oper_port_priority;

    npd_syslog_dbg("Get the partner's port(0X%x) priority  %d ", netif_index, value);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return value;
}

unsigned short netif_link_partner_number_get(unsigned int netif_index)
{
	int ret = -1;
    unsigned short  value = 0;
    lacp_port_partner_t   *lacp_port_partner= NULL;
	
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_partner->partner_oper_port_number;
    npd_syslog_dbg("Get the partner's port(0X%x) number  %d ", netif_index, value);
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return value;
}

unsigned char netif_link_partner_state_get(unsigned int netif_index)
{
	int ret = -1;
     unsigned short value = 0;
    lacp_port_partner_t   *lacp_port_partner = NULL;
	
    lacp_port_partner = malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner == NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
        goto error;
	}
    value = lacp_port_partner->partner_oper_port_state;
    npd_syslog_dbg("Get the partner's port(0X%x) state  %d ", netif_index, value);
 error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return value;
}
unsigned char* netif_link_partner_state_string(unsigned int netif_index)
{
	int i = 0, j = 0;
    int ret = -1;
	static unsigned char state_str[16];
	char alp_str[8] = {'A','B','C','D','E','F','G','H'};
    lacp_port_partner_t   *lacp_port_partner= NULL;
	
    lacp_port_partner= malloc(sizeof(lacp_port_partner_t));
	if(lacp_port_partner== NULL)
	{
        goto error;
	}
    
    lacp_port_partner->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_partner_index, netif_index, lacp_port_partner);
	if(ret != 0)
	{   
         goto error;
	}

    memset(state_str, 0, 16);
	state_str[j++] = '{';
	for(i = 0; i < 8; i++)
	{
		if((lacp_port_partner->partner_oper_port_state >> i) & 0x1)
		{
			state_str[j++] = alp_str[i];
		}
	}
	state_str[j] = '}';
error:
    
    if(lacp_port_partner)
        free(lacp_port_partner);
	return state_str;
}

unsigned int netif_link_selected_set(unsigned int netif_index, unsigned int selected)
{
	int ret = -1;
    lacp_port_sm_t   *lacp_port_sm = NULL;
	
    lacp_port_sm = malloc(sizeof(lacp_port_sm_t));
	if(lacp_port_sm == NULL)
	{
        goto error;
	}
    
    lacp_port_sm->netif_index = netif_index;
	ret = dbtable_sequence_search(lacpport_sm_index, netif_index, lacp_port_sm);
	if(ret != 0)
	{   
        goto error;
	}
    
	if(selected == TRUE)
	{
		lacp_port_sm->sm_vars |= AD_PORT_SELECTED;
        npd_syslog_dbg("The value of Selected is set to SELECTED ");
	}
	else
	{
		lacp_port_sm->sm_vars &= ~AD_PORT_SELECTED;
        npd_syslog_dbg("The value of Selected is set to UNSELECTED ");
	}
    ret = dbtable_sequence_update(lacpport_sm_index, netif_index, lacp_port_sm, lacp_port_sm);
    if(ret != 0)
	{   
		goto error;
	}
    
error:
    
    if(lacp_port_sm)
        free(lacp_port_sm);
	return ret;
}

void npd_lacp_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
)
{
    int ret;
    lacp_port_actor_t *lacp_port_actor = NULL;
    
    npd_syslog_dbg("lacp handle port event: port_index 0x%x, event %d\n", eth_g_index, event );
    if( NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index) )
	{
		ret = NPD_FAIL;
		goto error;
	}
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    lacp_port_actor->netif_index = eth_g_index;
	ret = dbtable_sequence_search(lacpport_actor_index, eth_g_index, lacp_port_actor);
	if(ret != 0)
	{
        goto error;
	}
    
    switch(event)
    {
        case PORT_NOTIFIER_CREATE:
            /*todo*/
            break;
        case PORT_NOTIFIER_DELETE:
            /*todo*/
            break;
        case PORT_NOTIFIER_INSERT:
            /*todo*/
            break;
        case PORT_NOTIFIER_REMOVE:
        case PORT_NOTIFIER_LINKDOWN_E:
            npd_syslog_dbg("PORT_NOTIFIER_LINKDOWN_E");
            dot3ad_handle_link_change(eth_g_index, PORT_NOTIFIER_LINKDOWN_E);
            break;
        case PORT_NOTIFIER_LINKUP_E:
            npd_syslog_dbg("PORT_NOTIFIER_LINKUP_E:");
            dot3ad_handle_link_change(eth_g_index, PORT_NOTIFIER_LINKUP_E);
            break;
        default:
            break;
    }
    ret = NPD_SUCCESS;

error:  
    if(lacp_port_actor)
        free(lacp_port_actor);
	return;    
}
void npd_lacp_handle_port_relate_event
(
    unsigned int trunk_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
)
{
    int ret = 0;
    int trunkId;
    aggregator_mode_t mode = MANUAL_MODE;
    lacp_port_actor_t *lacp_port_actor = NULL;
    struct trunk_s *trunkNode = NULL;
    int op_ret = TRUNK_RETURN_CODE_ERR_NONE;

    trunkNode = malloc(sizeof(struct trunk_s));
    if(NULL == trunkNode)
    {
        ret = NPD_FAIL;
        goto error;
    }
    lacp_port_actor = malloc(sizeof(lacp_port_actor_t));
	if(lacp_port_actor == NULL)
	{   
	    goto error;
	}
    if (npd_netif_type_get(trunk_index) != NPD_NETIF_TRUNK_TYPE)
    {
        ret = NPD_FAIL;
        goto error;
    }
    if (npd_netif_type_get(eth_g_index) != NPD_NETIF_ETH_TYPE)
    {
        ret = NPD_FAIL;
        goto error;
    }
    npd_syslog_dbg("join :relate_event");
    memset(trunkNode, 0, sizeof(struct trunk_s));
    trunkId = npd_netif_trunk_get_tid(trunk_index);
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {   
        goto error;
    }
    mode = trunkNode->aggregator_mode;
    npd_syslog_dbg("debug :mode = %d",mode);
    switch(event)
    {
        case PORT_NOTIFIER_JOIN:
            if(STATIC_MODE == mode)
            {
                ret = static_aggregator_join(trunk_index, eth_g_index);
                if(0 != ret)
                {
                    ret = NPD_FAIL;
                    goto error;
                }

                op_ret = static_aggregator_update(trunk_index);
                if(op_ret == TRUNK_RETURN_CODE_ERR_GENERAL)
                {
                   ret = NPD_FAIL;
                   goto error;
                }
            }
            else if(DYNAMIC_MODE == mode)
            {
             /*todo*/ 
            }
            else if(MANUAL_MODE == mode)
            {
                npd_port_aggregator_delete(eth_g_index); 
            }
            break;
        case PORT_NOTIFIER_LEAVE:
            if(STATIC_MODE == mode)
            {
                npd_syslog_dbg("PORT_NOTIFIER_LEAVE");
                ret = static_aggregator_leave(trunk_index, eth_g_index);
                if(0 != ret)
                {
                    ret = NPD_FAIL;
                    goto error;
                }

                op_ret = static_aggregator_update(trunk_index);
                if(op_ret == TRUNK_RETURN_CODE_ERR_GENERAL)
                {
                   ret = NPD_FAIL;
                   goto error;
                }
            }
            else if(DYNAMIC_MODE == mode)
            {
             /*todo*/
            }
            break;
        default:
            break;
    }
error:

    if(trunkNode)
        free(trunkNode);
    if(lacp_port_actor)
        free(lacp_port_actor);
	return;
}
static int npd_lacp_enable_disable_config(unsigned int lacp_enable)
{
	int ret = ETHPORT_RETURN_CODE_ERR_NONE;
    struct npd_lacp_cfg_s npd_lacp_cfg_set;
	npd_lacp_cfg_set.lacp_enable= lacp_enable;	
	pthread_mutex_lock(&lacp_act_sync);
    ret = dbtable_array_update(npd_lacp_cfg_index, 0, &npd_lacp_cfg_set, &npd_lacp_cfg_set);
	pthread_mutex_unlock(&lacp_act_sync);
	return ret;
}
DBusMessage* npd_dbus_config_all_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned int value = 0;
    unsigned int array_port_id;
    unsigned int trunk_netif_index;
    unsigned int last_port =0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned short trunkId = 0;
    unsigned short agg_port = 0;
    struct trunk_s *trunkNode;
    aggregator_t *aggregator;
    struct npd_lacp_cfg_s npd_lacp_cfg_get;
    trunkNode = malloc(sizeof(struct trunk_s));
    if(trunkNode == NULL)
    {
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
    aggregator = malloc(sizeof(aggregator_t));
    if(aggregator == NULL)
    {
        op_ret = COMMON_RETURN_CODE_NO_RESOURCE;
        goto error;
    }
	npd_syslog_dbg("Entering config ports lacp!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
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
    
	npd_syslog_dbg("Configure all the eth_port lacp %s\n",value ? "enable" : "disable");

    op_ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(op_ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_lacp_enable");
		 goto error;
	}
    g_lacp_enable = npd_lacp_cfg_get.lacp_enable;
    
    if(value == g_lacp_enable)
    {
        goto error;
    }
    else
    {
        if(value == AD_LACP_ENABLE)
        {
            op_ret = npd_lacp_enable_disable_config(value);
	        if (op_ret != ETHPORT_RETURN_CODE_ERR_NONE) 
            {
		        npd_syslog_dbg("npd_lacp_enable_disable_config::  set %d ERROR. \n",value);
		        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
	        }
            
        }
        else if(value == AD_LACP_DISABLE)
        {
            op_ret = npd_lacp_enable_disable_config(value);
	        if (op_ret != ETHPORT_RETURN_CODE_ERR_NONE) 
            {
		        npd_syslog_dbg("npd_lacp_enable_disable_config::  set %d ERROR. \n",value);
		        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
	        }
            
            for (; trunkId <= CHASSIS_TRUNK_RANGE_MAX; trunkId++)
            {   
                op_ret = npd_find_trunk(trunkId, trunkNode);
                if(-1 == op_ret)
                {   
                    syslog_ax_trunk_dbg("Find trunk on Sw Error!\n");
                    continue;
                }
                if(trunkNode->aggregator_mode == MANUAL_MODE)
                {
                    continue;
                }
                
                trunk_netif_index = trunkNode->g_index;
                aggregator->netif_index = trunk_netif_index;
                op_ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);	
                if(op_ret != 0)
            	{  
            	    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
                    goto error;
                }
                agg_port = aggregator->num_of_ports;
                if(trunkNode->aggregator_mode == DYNAMIC_MODE)/*delete all the dynamic trunks which had been created automatically */
                {
                    NPD_PBMP_ITER(trunkNode->ports, array_port_id)
                    { 
                        ++last_port;
                        if((agg_port) != last_port)
                        {
                            unsigned int eth_g_index = eth_port_array_index_to_ifindex(array_port_id);
                            dynamic_aggregator_leave(trunk_netif_index, eth_g_index);
                        }
                        
                    } 
                }
                else if(trunkNode->aggregator_mode == STATIC_MODE)
                {
                    /*todo*/
                }
                else
                {
                    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
                }
            }
        }
        else
        {
           op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        }
    }
error:
    if(trunkNode)
        free(trunkNode);
    if(aggregator)
        free(aggregator);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}    

DBusMessage* npd_dbus_config_port_lacp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter  iter;

	unsigned int eth_g_index = 0;
	unsigned int value = 0;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
	struct npd_lacp_cfg_s npd_lacp_cfg_get;

	memset(&npd_lacp_cfg_get, 0, sizeof(struct npd_lacp_cfg_s));
	
	npd_syslog_dbg("Entering config ports lacp!\n");

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
    
	npd_syslog_dbg("configure eth_port 0x%x,lacp %s\n",eth_g_index,value ? "enable" : "disable");
	pthread_mutex_lock(&lacp_act_sync);

	op_ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(op_ret != 0)
	{
	    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}
	if (!npd_lacp_cfg_get.lacp_enable)
	{
		op_ret = TRUNK_RETURN_CODE_GLOBAL_LACP_DISABLE;
		goto error;
	}
    if(value == AD_LACP_ENABLE)
    {
	    op_ret = dot3ad_enable_lacp_port(eth_g_index);
        if(op_ret != ETHPORT_RETURN_CODE_ERR_NONE)
        {
             goto error;
        }
    }
    else if(value == AD_LACP_DISABLE)
    {
        op_ret = dot3ad_disable_lacp_port(eth_g_index);
        if(op_ret != ETHPORT_RETURN_CODE_ERR_NONE)
        {
             goto error;
        }
    }
    else
    {
        op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
    }
    
error:
	pthread_mutex_unlock(&lacp_act_sync);
    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}    
DBusMessage * npd_dbus_aggregator_mode_change
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
    unsigned int array_port = 0;
    unsigned int trunk_netif_index = 0;
    unsigned int eth_g_index = 0;
	unsigned short trunkId = 0;
    struct trunk_s *trunkNode = NULL;
    aggregator_t   *aggregator = NULL;
	int isEnable = FALSE;
	int ret = -1;
    
    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
	    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
	}
    trunkNode = malloc(sizeof(struct trunk_s));
    if(NULL == trunkNode) 
    {
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
    }
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID))) {
		 npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
		goto error;
	}
	npd_syslog_dbg("lacp set trunk %d mode : %s trunk \n",trunkId, isEnable ? "static" : "manual" );
    pthread_mutex_lock(&lacp_act_sync);

	npd_key_database_lock();
	ret = npd_find_trunk(trunkId, trunkNode);
    if(0 != ret)
    {
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
    }
    trunk_netif_index = trunkNode->g_index;

    npd_syslog_dbg("Curent trunk %d mode : %s trunk \n", trunkId, trunkNode->aggregator_mode ? "static" : "manual" );

    if(trunkNode->aggregator_mode == isEnable)
    {
        npd_syslog_dbg("  %s mode of trunk has already set up\n",isEnable ? "Static" : "Manual" );
        
        op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
        goto error;
    }
    else
    {
        if(MANUAL_MODE == isEnable)
        {
            npd_syslog_dbg(" The trunk is changed into manual trunk! \n" );
            npd_lacp_aggregator_delete(trunk_netif_index);
            trunkNode->aggregator_mode = MANUAL_MODE;
            dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
            NPD_PBMP_ITER(trunkNode->ports, array_port)
            {
                eth_g_index = eth_port_array_index_to_ifindex(array_port);
                eth_g_index &= 0xFFFFC000;
                npd_port_aggregator_delete(eth_g_index); 
            }
        }
        else if(STATIC_MODE == isEnable)
        {
            npd_syslog_dbg(" The trunk is changed into static trunk! \n" );
            aggregator->netif_index = trunk_netif_index;
            ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);
        	if(ret == 0)
        	{  
        	   
        	    ret = npd_lacp_aggregator_delete(trunk_netif_index);
                if(ret !=0)
                {
                    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
                    goto error;
                }
        	}
			/*****create aggregator for trunk********************/
        	ret = npd_lacp_aggregator_init(trunk_netif_index, STATIC_MODE);
        	if(ret != 0)
        	{
				op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
            	goto error;
        	}
            NPD_PBMP_ITER(trunkNode->ports, array_port)
            {
                eth_g_index = eth_port_array_index_to_ifindex(array_port);
                eth_g_index &= 0xFFFFC000;
                npd_syslog_dbg("find: eth_g_index = 0X%x! type = %d\n" ,eth_g_index,npd_netif_type_get(eth_g_index));
                
                static_aggregator_join(trunk_netif_index, eth_g_index);
            }
            
            trunkNode->aggregator_mode = STATIC_MODE;
            dbtable_sequence_update(g_trunks, trunkId, NULL, trunkNode);
            
        }
    } 
error:
	npd_key_database_unlock();
    pthread_mutex_unlock(&lacp_act_sync);

    if(aggregator)
        free(aggregator);
    if(trunkNode)
        free(trunkNode);
    
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	return reply;
	
}

DBusMessage *npd_dbus_show_lacp_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	char *showStr = NULL;
	char*	cursor = NULL;
	struct npd_lacp_cfg_s npd_lacp_cfg_get;
	unsigned int op_ret = 0;
	unsigned int lacp_sta = 0;
	
	showStr = (char *)malloc(50);
	if(NULL == showStr) 
	{
		npd_syslog_err("memory malloc for show lacp running config error\n");
		return NULL;
	}
	memset(showStr, 0, 50);
	op_ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(op_ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_lacp_enable");
		 return NULL;
	}
    lacp_sta = npd_lacp_cfg_get.lacp_enable;
    
	cursor = showStr;

	if (0 != lacp_sta)
	{
		sprintf(cursor, "lacp enable\n");
		
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
	
}


DBusMessage *npd_dbus_aggregator_show_by_trunkId(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusError err;

    unsigned char  i = 0;
    unsigned char j = 0;
    unsigned int en_count = 0, dis_count = 0;
    int ret = 0;
    
    struct trunk_s *trunkNode = NULL;
    unsigned short    trunkId = 0;
    
    unsigned short aggId = 0;
    unsigned char  mode = 0;
    unsigned char  status_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned short priority_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned char  flag_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned short key_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned char  dis_status_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned short dis_priority_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned char  dis_flag_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned short dis_key_actor[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned int   array_port = 0;
    unsigned short key_port_manual[MAX_PORT_PER_TRUNK] = {0}; 
    
    unsigned int    eth_g_index = 0;
    aggregator_t *aggregator = NULL;
    unsigned short port_number_partner[MAX_PORT_PER_TRUNK] = {0}; 
    unsigned short priority_partner[MAX_PORT_PER_TRUNK] = {0};    
    unsigned short key_partner[MAX_PORT_PER_TRUNK] = {0};         
    unsigned char flag_partner[MAX_PORT_PER_TRUNK] = {0};         
    unsigned int op_ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    unsigned char *mac_str = NULL;
    unsigned int lacp_enable_state = 0;
    mac_str = malloc(48);
    memset(mac_str, 0, 48);
    struct npd_lacp_cfg_s npd_lacp_cfg_get;
    ret = dbtable_array_get(npd_lacp_cfg_index, 0, &npd_lacp_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_lacp_enable");
		 goto error;
	}
    lacp_enable_state = npd_lacp_cfg_get.lacp_enable;
    
    trunkNode = malloc(sizeof(struct trunk_s));
    if(NULL == trunkNode) 
    {
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
    }
    aggregator = malloc(sizeof(aggregator_t));
	if(aggregator == NULL)
	{
	    op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
	}

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&trunkId,
                                DBUS_TYPE_INVALID)))
    {
        npd_syslog_err("Unable to get input args\n ");

        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        goto error;
    }
    npd_syslog_dbg("enter:npd_dbus_aggregator_show_by_trunkId");
     npd_syslog_dbg("trunkId = %d\n" ,trunkId);
    trunkNode->trunk_id = trunkId;
	ret = dbtable_sequence_search(g_trunks, trunkId, trunkNode);
	if(0 != ret)
    {
        op_ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
        goto error;
	}
    
    if(trunkNode->aggregator_mode == MANUAL_MODE) 
    {
        NPD_PBMP_ITER(trunkNode->ports, array_port)
        {
            eth_g_index = eth_port_array_index_to_ifindex(array_port);
            eth_g_index &= 0xFFFFC000;
            
            key_port_manual[i] = netif_manual_port_key_get(eth_g_index);
            i++;
        }
        aggId = 0;
        goto error;
    }
    aggregator->netif_index = trunkNode->g_index;
    ret = dbtable_hash_search(aggregator_index, aggregator, NULL, aggregator);	
    if(ret != 0)
	{  
	    op_ret = ETHPORT_RETURN_CODE_ERR_NONE;
        aggId = 0;
	    goto error;
	}
    npd_syslog_dbg("aggregator_show_by_trunkId %d\n", trunkNode->trunk_id);
    NPD_PBMP_ITER(trunkNode->ports, array_port)
    {
        unsigned int k = 0;
        int link_status = 0;
        if(en_count != 0)
        {
            k = ((en_count * 6));
        }
        npd_syslog_dbg("en_count=%d k = %d" , en_count, k);
        eth_g_index = eth_port_array_index_to_ifindex(array_port);
        eth_g_index &= 0xFFFFC000;
   
        
        link_status = npd_check_eth_port_status(eth_g_index);
        if(ETH_ATTR_LINKUP == link_status)
        {
             //actor
            status_actor[en_count] =  netif_is_link_selected(eth_g_index);
            priority_actor[en_count] = netif_link_priority_get(eth_g_index);
            key_actor[en_count] = netif_link_actor_key_get(eth_g_index);
            flag_actor[en_count] = netif_link_actor_state_get(eth_g_index);
            //partner
            port_number_partner[en_count] = netif_link_partner_number_get(eth_g_index);
            priority_partner[en_count] = netif_link_partner_sys_priority_get(eth_g_index);
            key_partner[en_count] = netif_link_partner_key_get(eth_g_index);
            netif_link_partner_sys_mac_get_v1(eth_g_index, &mac_str[k]);
            npd_syslog_dbg("%02x:%02x:%02x:%02x:%02x:%02x ", mac_str[k],mac_str[k + 1],mac_str[k + 2], mac_str[k + 3], 
            mac_str[k + 4], mac_str[k + 5]);
            flag_partner[en_count] = netif_link_partner_state_get(eth_g_index);
            en_count++;
        }
        else
        {
            //actor
            dis_status_actor[dis_count] =  netif_is_link_selected(eth_g_index);
            dis_priority_actor[dis_count] = netif_link_priority_get(eth_g_index);
            dis_key_actor[dis_count] = netif_link_actor_key_get(eth_g_index);
            dis_flag_actor[dis_count] = netif_link_actor_state_get(eth_g_index);
            dis_count++;
        }
       
      
    }
    //aggregator
    mode = aggregator->aggregator_mode;
    aggId = aggregator->aggregator_identifier;
    
error:
  
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &op_ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &lacp_enable_state);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT16,
                                   &aggId);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_BYTE,
                                   &mode);
    for(j = 0; j < MAX_PORT_PER_TRUNK; j++)
    {
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &status_actor[j]);
  
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT16,
                                       &priority_actor[j]);
  
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT16,
                                       &key_actor[j]);
  
        dbus_message_iter_append_basic(&iter,
                                      DBUS_TYPE_UINT16,
                                       &key_partner[j]);

        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &flag_actor[j]);
 
        dbus_message_iter_append_basic(&iter,
                                          DBUS_TYPE_BYTE,
                                          &dis_status_actor[j]);
        dbus_message_iter_append_basic(&iter,
                                           DBUS_TYPE_UINT16,
                                           &dis_priority_actor[j]);
        dbus_message_iter_append_basic(&iter,
                                           DBUS_TYPE_UINT16,
                                           &dis_key_actor[j]);
        dbus_message_iter_append_basic(&iter,
                                           DBUS_TYPE_UINT16,
                                           &dis_flag_actor[j]);

        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT16,
                                       &port_number_partner[j]);
   
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT16,
                                       &priority_partner[j]);
   
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_BYTE,
                                       &flag_partner[j]);
   
        dbus_message_iter_append_basic(&iter,
                                       DBUS_TYPE_UINT16,
                                       &key_port_manual[j]);
    }
    for(j = 0; j < 48; j++)
    {
        dbus_message_iter_append_basic(&iter,
    			                       DBUS_TYPE_BYTE,
    			                       &(mac_str[j]));
    }
    if(mac_str)
        free(mac_str);
    if(aggregator)
        free(aggregator);
    if(trunkNode)
        free(trunkNode);
    
    return reply;
}

#ifdef __cplusplus
}
#endif
#endif

