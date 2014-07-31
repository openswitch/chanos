/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_qos.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		API used in NPD dbus process for QOS  module
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.71 $	
*******************************************************************************/
#ifdef HAVE_QOS
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_qos.h"

db_table_t  *npd_qos_dbtbl = NULL;
db_table_t  *npd_policymap_dbtbl = NULL;
db_table_t  *npd_policer_dbtbl = NULL;
db_table_t  *npd_counter_dbtbl = NULL;
db_table_t  *npd_qos_netif_cfg_dbtbl = NULL;
db_table_t *cpu_flow_control_cfg_db = NULL;
hash_table_index_t *cpu_flow_control_cfg_index = NULL;

array_table_index_t  *npd_qos_arr_index = NULL;
array_table_index_t  *npd_policymap_arr_index = NULL;
array_table_index_t  *npd_policer_arr_index = NULL;
array_table_index_t  *npd_counter_arr_index = NULL;
sequence_table_index_t *npd_qos_netif_cfg_seq_index = NULL;
array_table_index_t  *npd_qos_global_parm_arr_index = NULL;

extern array_table_index_t* acl_group_index;
extern hash_table_index_t* acl_group_name;

int npd_qos_netif_cfg_init(unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr);


netif_event_notifier_t qos_port_notifier =
{
    .netif_event_handle_f = npd_qos_handle_port_event,
    .netif_relate_handle_f = npd_qos_handle_port_relate_event
};

unsigned int npd_qos_port_index(unsigned int index)
{
    return eth_port_array_index_from_ifindex(index);
}

unsigned int npd_qos_port_key(void *cfg)
{
    return((QOS_PORT_CFG_STC*) cfg)->ifIndex;
}


unsigned int cpu_flow_control_cfg_hash_key(void *data)
{
	int i = 0;
	int protocol_index = 0;

    cpu_flow_control_t *cpu_flow_control_cfg = (cpu_flow_control_t *)data;
	
	for(i = 0; i < strlen(cpu_flow_control_cfg->protocol); i++) {
		protocol_index += cpu_flow_control_cfg->protocol[i];
	}
	
    return protocol_index % MAX_CPU_FLOW_CONTROL_PROTOCOL_NUM;
}

unsigned int cpu_flow_control_cfg_hash_cmp(void *data1, void *data2)
{
	return (((struct cpu_flow_control_s *) data1)->protocol[0]
			   == ((struct cpu_flow_control_s *)data2)->protocol[0]);
}

long cpu_flow_control_handle_insert(void *newdata)
{
	int ret = 0, retval = 0;
	cpu_flow_control_t *cfctl_cfg = (cpu_flow_control_t *)newdata;
	
	ret = nam_qos_cpu_flow_cortrol(cfctl_cfg->protocol, cfctl_cfg->bandwith, cfctl_cfg->priority);
	if (0 != ret)
		retval = -1;

	return retval;
}

long cpu_flow_control_handle_update(void * newdata, void *olddata)
{
	int ret = 0, retval = 0;
	cpu_flow_control_t *new_cfctl_cfg = (cpu_flow_control_t *)newdata;
    cpu_flow_control_t *old_cfctl_cfg = (cpu_flow_control_t *)olddata;
    
	if (new_cfctl_cfg->bandwith != old_cfctl_cfg->bandwith) {
		ret = nam_qos_cpu_flow_cortrol(new_cfctl_cfg->protocol, new_cfctl_cfg->bandwith, new_cfctl_cfg->priority);
		if (0 != ret)
			retval = -1;
	}

	if (new_cfctl_cfg->priority != old_cfctl_cfg->priority) {
		ret = nam_qos_cpu_flow_cortrol(new_cfctl_cfg->protocol, new_cfctl_cfg->bandwith, new_cfctl_cfg->priority);
		if (0 != ret)
			retval = -1;
	}

	return retval;
}

long npd_qos_netif_cfg_insert(void *data)
{
	QOS_PORT_CFG_STC*	item = NULL;
	int					ret = 0;
	int					i = 0;
	int					mode = 1; 
	unsigned int		weight[MAX_COS_QUEUE_NUM];
	
	if( data == NULL )
	{
		return -1;
	}

	memset(weight, 0, sizeof(weight));
	item = (QOS_PORT_CFG_STC *)data;

	/* for port queue traffic */
	for(i=0; i<MAX_COS_QUEUE_NUM; i++ ) 
	{
		weight[i] = item->cosCfg.queue[i].weight;
	}

	ret |= nam_qos_queue_set(item->ifIndex, (unsigned int)item->cosCfg.queue_type, weight);

	/* for port shape */
    if(item->tcCfg.portEnable == 1)
    {
    	ret |= nam_qos_traffic_shape_port(item->ifIndex, item->tcCfg.kmstate, item->tcCfg.burstSize, item->tcCfg.Maxrate);
    	if(ret!=QOS_RETURN_CODE_SUCCESS)
    	{
    		npd_syslog_err("nam_qos_traffic_shape_port fail!\r\n"); 
    	}
    }
	else
	{
    	ret |= nam_qos_traffic_shape_port(item->ifIndex, item->tcCfg.kmstate, item->tcCfg.burstSize, item->tcCfg.Maxrate);
	}
	/* for port bandwidth */
	for( i=0; i<MAX_COS_QUEUE_NUM; i++) 
	{
		if( item->tcCfg.queue[i].queueEnable == 1) 
		{
			ret |=nam_qos_traffic_shape_queue_port(item->ifIndex,item->tcCfg.queue[i].kmstate, i, item->tcCfg.queue[i].burstSize, \
															item->tcCfg.queue[i].Maxrate);
		}
		else 
		{
			ret |=nam_qos_del_traffic_shape_queue_port(item->ifIndex, mode, i);
			if(ret!=QOS_RETURN_CODE_SUCCESS) 
			{
				npd_syslog_err("clear1!!!!nam_qos_del_traffic_shape_queue_port fail!\r\n");
			}
		}
//		nam_qos_queue_drop(item->ifIndex, item->cosCfg.queue_drop_mode);
	}

    nam_qos_port_trust(item->ifIndex, item->trust);
    nam_qos_port_default_qos_profile(item->ifIndex, item->qosProfileIndex);
    nam_qos_port_eg_remark(item->ifIndex, item->egressRemark);
    nam_qos_port_in_remark(item->ifIndex, item->ingressRemark);
    
    
	return ret;
}

long npd_qos_netif_cfg_update(void *data, void *old_data)
{
	QOS_PORT_CFG_STC *updateItem = NULL, *origItem = NULL;
	int ret = 0;
	int mode = 1, i = 0;
	unsigned int weight[MAX_COS_QUEUE_NUM];

	if( data == NULL || old_data == NULL )
	{
		return -1;
	}

	updateItem = (QOS_PORT_CFG_STC *)data;
	origItem = (QOS_PORT_CFG_STC *)old_data;

	{
		mode = updateItem->tcCfg.kmstate;
		if( updateItem->tcCfg.portEnable != origItem->tcCfg.portEnable ||
			updateItem->tcCfg.burstSize != origItem->tcCfg.burstSize ||
			updateItem->tcCfg.Maxrate != origItem->tcCfg.Maxrate)
		{
			ret = nam_qos_traffic_shape_port(updateItem->ifIndex, mode, updateItem->tcCfg.burstSize, updateItem->tcCfg.Maxrate);
			if(ret!=QOS_RETURN_CODE_SUCCESS)
			{
				npd_syslog_err("nam_qos_traffic_shape_port fail!"); 
			}
		}
		
		for( i=0; i<MAX_COS_QUEUE_NUM; i++) 
		{
			if( memcmp( &(updateItem->tcCfg.queue[i]), &(origItem->tcCfg.queue[i]), sizeof(QOS_SHAPER_QUEUE)) )
			{
				if( updateItem->tcCfg.queue[i].queueEnable == 1) 
				{
					ret =nam_qos_traffic_shape_queue_port(updateItem->ifIndex, mode,i, updateItem->tcCfg.queue[i].burstSize, \
																	updateItem->tcCfg.queue[i].Maxrate);
				}
				else 
				{
					ret =nam_qos_del_traffic_shape_queue_port(updateItem->ifIndex,mode,i);
					if(ret!=QOS_RETURN_CODE_SUCCESS) 
					{
						npd_syslog_err("clear1!!!!nam_qos_del_traffic_shape_queue_port fail!");
					}
				}
			}
		}
	}

	{		
		if(updateItem->cosCfg.queue_type != origItem->cosCfg.queue_type ||
		   memcmp(&(updateItem->cosCfg.queue), &(origItem->cosCfg.queue), sizeof(QOS_WRR_TX_WEIGHT_E)*MAX_COS_QUEUE_NUM) )
		{
			for(i=0; i<MAX_COS_QUEUE_NUM; i++ ) 
			{
				weight[i] = updateItem->cosCfg.queue[i].weight;
			}
			ret = nam_qos_queue_set(updateItem->ifIndex, updateItem->cosCfg.queue_type, weight);
			if(ret != QOS_RETURN_CODE_SUCCESS)
			{
				npd_syslog_err("nam_qos_queue_set fail,ret %d\n",ret);		
			}
		}
	}
    {
        nam_qos_port_default_qos_profile(updateItem->ifIndex, updateItem->qosProfileIndex);
    }
    
    nam_qos_port_in_remark(updateItem->ifIndex, updateItem->ingressRemark);
    nam_qos_port_eg_remark(updateItem->ifIndex, updateItem->egressRemark);
    nam_qos_port_trust(updateItem->ifIndex, updateItem->trust);
  
	return ret;
}

long npd_qos_netif_cfg_delete(void *data)

{
	QOS_PORT_CFG_STC *item = NULL;
	int ret = 0;
	
	if( data == NULL )
	{
		return -1;
	}

	item = (QOS_PORT_CFG_STC *)data;

	return ret;
}

long npd_qos_policer_update(void* new_data, void* old_data)
{
	QOS_POLICER_STC* update_item = (QOS_POLICER_STC *)new_data;
	QOS_POLICER_STC* orig_item = (QOS_POLICER_STC *)old_data;
	int ret = 0;

	if (NULL == update_item || NULL == orig_item )
	{
		return -1;
	}
	if (0 != memcmp(update_item, orig_item, sizeof(*update_item)))
	{
		ret |= nam_qos_policer_set_cir_cbs(update_item->index, update_item);
	}
	
	return ret;
}

long npd_qos_policer_insert(void* data)
{
	QOS_POLICER_STC* data_item = (QOS_POLICER_STC *)data;
	int ret = 0;

	if (NULL == data_item)
	{
		return -1;
	}

	if (0 != data_item->index)
	{
		ret |= nam_qos_policer_create(&data_item->index, data_item->direction);
		ret |= nam_qos_policer_set_cir_cbs(data_item->index, data_item);
	}
	
	return ret;
}

long npd_qos_policer_delete(void* data)
{
	QOS_POLICER_STC* data_item = (QOS_POLICER_STC *)data;
	int ret = 0;

	if (NULL == data_item)
	{
		return -1;
	}

	if (0 != data_item->index)
	{
		ret |= nam_qos_policer_destroy(data_item->index, data_item->direction);
	}

	return ret;
}

long npd_qos_counter_update(void *data, void *old_data)
{
	QOS_COUNTER_STC *updateItem = NULL, *origItem = NULL;
	int ret = 0;
	
	if( data == NULL || old_data == NULL)
	{
		return -1;
	}
	
	updateItem = (QOS_COUNTER_STC *)data;
	origItem = (QOS_COUNTER_STC *)old_data;

	if( updateItem->inProfileBytesCnt != origItem->inProfileBytesCnt ||
		updateItem->outOfProfileBytesCnt != updateItem->outOfProfileBytesCnt )
	{
		ret = nam_qos_set_counter(updateItem->index, updateItem->inProfileBytesCnt, updateItem->outOfProfileBytesCnt);
		if(ret!=QOS_RETURN_CODE_SUCCESS) 
		{
			npd_syslog_err("fail to set counter!\n");
			ret = -1;		
		}
	}

	return ret;
}

long npd_qos_counter_insert(void *data)
{
	QOS_COUNTER_STC *item = NULL;
	int ret = 0;

	if( data == NULL )
	{
		return -1;
	}

	item = (QOS_COUNTER_STC *)data;
	
	ret = nam_qos_set_counter(item->index, item->inProfileBytesCnt, item->outOfProfileBytesCnt);
	if(ret!=QOS_RETURN_CODE_SUCCESS) 
	{
		npd_syslog_err("fail to set counter!\n");
		ret = -1;
	}

	return ret;
}

long npd_qos_counter_delete(void *data)
{
	QOS_COUNTER_STC *item = NULL;
	int ret = 0;

	if( data == NULL )
	{
		return -1;
	}

	item = (QOS_COUNTER_STC *)data;
	
	ret = nam_qos_set_counter(item->index, 0, 0);
	if(ret!=QOS_RETURN_CODE_SUCCESS) 
	{
		npd_syslog_err("fail to set counter!\n");
		ret = -1;
	}

	return ret;

}

long npd_qos_profile_insert(void *data)
{
	
	QOS_PROFILE_STC *updateItem = NULL;	
	int ret = 0;
	
	if( data == NULL)
		return -1;
	
	updateItem = (QOS_PROFILE_STC *)data;
	
	{
		ret=nam_qos_profile_entry_set(updateItem->index, updateItem);
		if(ret!=QOS_RETURN_CODE_SUCCESS) 
		{
			npd_syslog_dbg("qos profile entry set fail");
		}
	}
	return 0;
}

long npd_qos_profile_update(void *data, void *old_data)
{
	
	QOS_PROFILE_STC *updateItem = NULL, *origItem = NULL;
	int ret = 0;
	if( data == NULL || old_data == NULL)
		return -1;
	
	updateItem = (QOS_PROFILE_STC *)data;
	origItem = (QOS_PROFILE_STC *)old_data;	

	if( memcmp(updateItem, origItem, sizeof(QOS_PROFILE_STC)))
	{
		ret=nam_qos_profile_entry_set(updateItem->index, updateItem);
		if(ret!=QOS_RETURN_CODE_SUCCESS) 
		{
			npd_syslog_dbg("qos profile entry set fail");
		}
	}
	return 0;
}

long npd_qos_policymap_insert(void* data)
{
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC*	item = NULL;

	if (data == NULL)
	{
		return -1;
	}
	
	item = (QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *)data;
	return 0;
}

long npd_qos_policymap_update(void *data, void *old_data)
{

	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC*	item = NULL;
	int 		ret = 0;

	if (data == NULL)
	{
		return -1;
	}
	
	item = (QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *)data;

	
	{
		ret = nam_qos_port_bind_policy_map(item, 0);
	}
	return 0;
}

int npd_qosprofile_get_by_index(unsigned int index, QOS_PROFILE_STC *profilePtr)
{
	if( profilePtr== NULL )
		return -1;
	
	return dbtable_array_get( npd_qos_arr_index, index, profilePtr );
}

int npd_qosprofile_set_by_index(unsigned int index, QOS_PROFILE_STC *profilePtr)
{
	QOS_PROFILE_STC tmpProfile;

	if( profilePtr== NULL )
		return -1;
	
	if( 0 == dbtable_array_get( npd_qos_arr_index, index, &tmpProfile )){
		return dbtable_array_update( npd_qos_arr_index, index, &tmpProfile, profilePtr);
	}

	return dbtable_array_insert_byid( npd_qos_arr_index, index, profilePtr);
}

int npd_qosprofile_del_by_index( unsigned int index )
{
	int ret = -1;
	QOS_PROFILE_STC tmpProfile;
	
	if( 0 == dbtable_array_get( npd_qos_arr_index, index, &tmpProfile )){
		ret = dbtable_array_delete( npd_qos_arr_index, index, &tmpProfile);
	}

	return ret;
}

int npd_policymap_get_by_index( unsigned int index, QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *profilePtr )
{
	if( profilePtr== NULL )
		return -1;

	return dbtable_array_get( npd_policymap_arr_index, index, profilePtr );
}

int npd_policymap_set_by_index( unsigned int index, QOS_PORT_POLICY_MAP_ATTRIBUTE_STC *profilePtr )
{
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC tmpProfile;

	if( profilePtr== NULL )
		return -1;
	
	if( 0 == dbtable_array_get( npd_policymap_arr_index, index, &tmpProfile )){
		return dbtable_array_update( npd_policymap_arr_index, index, &tmpProfile, profilePtr);
	}

	return dbtable_array_insert_byid( npd_policymap_arr_index, index, profilePtr);
}

int npd_policymap_del_by_index( unsigned int index )
{
	int ret = -1;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC tmpProfile;
	
	if( 0 == dbtable_array_get( npd_policymap_arr_index, index, &tmpProfile )){
		ret = dbtable_array_delete( npd_policymap_arr_index, index, &tmpProfile);
	}

	return ret;
}

int npd_policer_get_by_index( unsigned int index, QOS_POLICER_STC *profilePtr )
{
	if( profilePtr== NULL )
		return -1;

	return dbtable_array_get( npd_policer_arr_index, index, profilePtr );
}

int npd_policer_set_by_index( unsigned int index, QOS_POLICER_STC *profilePtr )
{
	QOS_POLICER_STC tmpProfile;

	if( profilePtr== NULL )
		return -1;
	
	if( 0 == dbtable_array_get( npd_policer_arr_index, index, &tmpProfile )){
		return dbtable_array_update( npd_policer_arr_index, index, &tmpProfile, profilePtr);
	}

	return dbtable_array_insert_byid( npd_policer_arr_index, index, profilePtr);
}

int npd_policer_del_by_index( unsigned int index )
{
	int ret = -1;
	QOS_POLICER_STC tmpProfile;
	
	if( 0 == dbtable_array_get( npd_policer_arr_index, index, &tmpProfile )){
		ret = dbtable_array_delete( npd_policer_arr_index, index, &tmpProfile);
	}

	return ret;
}

int npd_counter_get_by_index( unsigned int index, QOS_COUNTER_STC *profilePtr )
{
	if( profilePtr== NULL )
		return -1;

	return dbtable_array_get( npd_counter_arr_index, index, profilePtr );
}

int npd_counter_set_by_index( unsigned int index, QOS_COUNTER_STC *profilePtr )
{
	QOS_COUNTER_STC tmpProfile;

	if( profilePtr== NULL )
		return -1;
	
	if( 0 == dbtable_array_get( npd_counter_arr_index, index, &tmpProfile )){
		return dbtable_array_update( npd_counter_arr_index, index, &tmpProfile, profilePtr);
	}

	return dbtable_array_insert_byid( npd_counter_arr_index, index, profilePtr);
}

int npd_counter_del_by_index( unsigned int index )
{	
	int ret = -1;
	QOS_COUNTER_STC tmpProfile;
	
	if( 0 == dbtable_array_get( npd_counter_arr_index, index, &tmpProfile )){
		ret = dbtable_array_delete( npd_counter_arr_index, index, &tmpProfile);
	}

	return ret;
}

int npd_qos_netif_cfg_init(unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr)
{
	int i;
	
	memset(profilePtr, 0, sizeof(QOS_PORT_CFG_STC));
	profilePtr->ifIndex = netifIndex;
	profilePtr->cosCfg.queue_type = QOS_PORT_TX_SP_ARB_GROUP_E;
    profilePtr->poMapCfg.poMapId = 0;
	for (i=0; i<MAX_COS_QUEUE_NUM; i++) 
	{
		profilePtr->cosCfg.queue[i].groupFlag = QOS_PORT_TX_SP_ARB_GROUP_E;
		profilePtr->cosCfg.queue[i].weight = 0;
	}
	
	return QOS_RETURN_CODE_SUCCESS;
}



int npd_qos_netif_cfg_get_by_index( unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr )
{
	if( profilePtr == NULL )
		return -1;

	profilePtr->ifIndex = netifIndex;
	return dbtable_sequence_search( npd_qos_netif_cfg_seq_index, netifIndex, profilePtr );
}

int npd_qos_netif_cfg_set_by_index( unsigned int netifIndex, QOS_PORT_CFG_STC *profilePtr )
{
	if( profilePtr == NULL )
		return -1;
	
	return dbtable_sequence_insert( npd_qos_netif_cfg_seq_index, netifIndex, profilePtr);
}

int npd_qos_netif_cfg_del_by_index( unsigned int netifIndex )
{	
	QOS_PORT_CFG_STC tmpProfile;
		
	tmpProfile.ifIndex = netifIndex;
	if (0 == dbtable_sequence_search( npd_qos_netif_cfg_seq_index, netifIndex, &tmpProfile)) 
	{
		return dbtable_sequence_delete( npd_qos_netif_cfg_seq_index, netifIndex, &tmpProfile, &tmpProfile);
	}

	return 0;
}

int npd_qos_netif_cmp( void *itemA, void *itemB)
{
	QOS_PORT_CFG_STC *dataA = NULL, *dataB= NULL;

	if( itemA == NULL || itemB == NULL )
	{
		return NPD_FALSE;
	}

	dataA = (QOS_PORT_CFG_STC *)itemA;
	dataB = (QOS_PORT_CFG_STC *)itemB;

	if( dataA->ifIndex == dataB->ifIndex )
	{
		return NPD_TRUE;
	}
	
	return  NPD_FALSE;	
}

#define DSCP_2_DP(dscp) (((dscp)&0x7)>>1)
#define DSCP_2_UP(dscp)    (((dscp)&0x38)>>3)

int npd_qos_dbtbl_init(void)
{
	int ret = 0;
    int i;
	ret = create_dbtable( "npdQosProfTbl", MAX_PROFILE_NUMBER, sizeof(QOS_PROFILE_STC),
			    npd_qos_profile_update,
			    NULL,
			    npd_qos_profile_insert,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
				NULL,
				NULL,
			    DB_SYNC_ALL,
			    &npd_qos_dbtbl);
	ret = dbtable_create_array_index("prof", npd_qos_dbtbl, &npd_qos_arr_index );


	ret = create_dbtable( "npdQosCnterTbl", MAX_COUNTER_NUM, sizeof(QOS_COUNTER_STC),
				npd_qos_counter_update,
				NULL,
				npd_qos_counter_insert,
				npd_qos_counter_delete,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				DB_SYNC_ALL,
				&npd_counter_dbtbl);
	ret = dbtable_create_array_index("counter", npd_counter_dbtbl, &npd_counter_arr_index );

	ret = create_dbtable( "npdPolicerTbl", MAX_POLICER_NUM, sizeof(QOS_POLICER_STC),
			    npd_qos_policer_update,
			    NULL,
			    npd_qos_policer_insert,
			    npd_qos_policer_delete,
			    NULL,
			    NULL,
			    NULL,
				NULL,
				NULL,
			    DB_SYNC_ALL,
			    &npd_policer_dbtbl);
	ret = dbtable_create_array_index("policer", npd_policer_dbtbl, &npd_policer_arr_index );

	ret = create_dbtable( "npdPoMapTbl", MAX_POLICY_MAP_NUM, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC),
			    npd_qos_policymap_update,
			    NULL,
			    npd_qos_policymap_insert,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
				NULL,
				NULL,
			    DB_SYNC_ALL,
			    &npd_policymap_dbtbl);
	ret = dbtable_create_array_index("policymap", npd_policymap_dbtbl, &npd_policymap_arr_index );

	class_qos_init();
    
	ret = create_dbtable( "npdQosNetifTbl", MAX_ETH_GLOBAL_INDEX, sizeof(QOS_PORT_CFG_STC),
				npd_qos_netif_cfg_update,
				NULL,
				npd_qos_netif_cfg_insert,
				npd_qos_netif_cfg_delete,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				DB_SYNC_ALL,
				&npd_qos_netif_cfg_dbtbl);

	ret = dbtable_create_sequence_index("qosnetif", npd_qos_netif_cfg_dbtbl, MAX_ETH_GLOBAL_INDEX, 
				npd_qos_port_index,
		        npd_qos_port_key,
		        npd_qos_netif_cmp, 
		        &npd_qos_netif_cfg_seq_index);


	create_dbtable("cpu_flow_control_dbtable", MAX_CPU_FLOW_CONTROL_PROTOCOL_NUM, sizeof(struct cpu_flow_control_s),
					&cpu_flow_control_handle_update, NULL, &cpu_flow_control_handle_insert, 
					NULL, NULL, NULL, NULL, 
					NULL, NULL, DB_SYNC_ALL, &cpu_flow_control_cfg_db);

	dbtable_create_hash_index("cpu_flow_control_index", cpu_flow_control_cfg_db, MAX_CPU_FLOW_CONTROL_PROTOCOL_NUM, 
	 	 					  &cpu_flow_control_cfg_hash_key, &cpu_flow_control_cfg_hash_cmp,
	  						  &cpu_flow_control_cfg_index);	


	ret = nam_qos_policer_init();
	if(QOS_RETURN_CODE_SUCCESS != ret) 
	{
		npd_syslog_err("policer init error!");
	}
	
	ret = nam_qos_traffic_queue_set();
	if(QOS_RETURN_CODE_SUCCESS != ret) 
	{
		npd_syslog_err("traffic queue set error!");
	}

    nam_qos_init();

	{
	    int ret;
		cpu_flow_control_t cfctl;
		
		memset(&cfctl, 0, sizeof(cfctl));
	
	    strcpy(cfctl.protocol, "total");
		cfctl.bandwith = 4000;
		cfctl.priority = -1;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);
	
		strcpy(cfctl.protocol, "bpdu");
		cfctl.bandwith = 192;
		cfctl.priority = 3;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "lldp");
		cfctl.bandwith = 64;
		cfctl.priority = 1;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "igmp");
		cfctl.bandwith = 256;
		cfctl.priority = 1;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);


		strcpy(cfctl.protocol, "rip");
		cfctl.bandwith = 320;
		cfctl.priority = 2;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "ospf");
		cfctl.bandwith = 3000;
		cfctl.priority = 2;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);


		strcpy(cfctl.protocol, "pim");
		cfctl.bandwith = 640;
		cfctl.priority = 2;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "bgp");
		cfctl.bandwith = 64;
		cfctl.priority = 2;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "arp");
		cfctl.bandwith = 320;
		cfctl.priority = 1;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "dhcp");
		cfctl.bandwith = 320;
		cfctl.priority = 1;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "ip-to-me");
		cfctl.bandwith = 1024;
		cfctl.priority = 0;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);

		strcpy(cfctl.protocol, "unknow");
		cfctl.bandwith = 100;
		cfctl.priority = 0;
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);
	}

    for(i = 1; i <= 32; i++)
	{
        QOS_PROFILE_STC profilePtr;
		memset(&profilePtr, 0, sizeof(QOS_PROFILE_STC));
		profilePtr.index = i;
        profilePtr.dropPrecedence = ((i-1)%4);
        profilePtr.userPriority = (i-1)/4;
        profilePtr.trafficClass = (i-1)/4;
        profilePtr.dscp = ((i-1)/4)*8 + ((i-1)%4)*2;
		if( 0 != npd_qosprofile_set_by_index( i, &profilePtr))
		{
			ret = QOS_RETURN_CODE_ERROR;
		}
	}
    {
        QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
        int ni;
		memset(&poMap, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));
		poMap.index = SYSTEM_QOS_POLICY;
        poMap.domain = 0;
		poMap.modifyDscp = DISABLE_E;
		poMap.modifyUp	 = DISABLE_E;

		for (ni = 0; ni < MAX_UP_PROFILE_NUM; ni++)
		{
            poMap.upMapQosProfile[ni].profileIndex = ni*4+1;
		}
        for (ni = 0; ni < MAX_DSCP_DSCP_NUM; ni++)
        {
            poMap.dscpMapQosProfile[ni].profileIndex = (ni/8)*4+1;
        }
		npd_policymap_set_by_index(SYSTEM_QOS_POLICY, &poMap);
    }


    {
        char name[30] = {0};
        policy_map_index_t policy_map;
        memset(&policy_map, 0, sizeof(policy_map_index_t));
        sprintf(name, "VOICE VLAN");
        class_map_create(name);
        policy_map_create(name);
        policy_map_class(name, name);
        policy_map_add_set(&policy_map, "qos-profile", "24");
        policy_map_find_by_name(name, &policy_map);
        service_policy_create_reserved(name, ACL_DIRECTION_INGRESS_E, VOICE_VLAN_RSV_SERVICE_POLICY_ID);
     }

	return ret;	
}


void npd_qos_handle_port_event
(
	unsigned int  eth_g_index,
	enum PORT_NOTIFIER_ENT event,
	char *private, int len
)
{
    int ret;
    int vid = 0;
	QOS_PORT_CFG_STC qosPortCfg;

	syslog_ax_qos_dbg("qos handle port event: port_index 0x%x, event %d\n", eth_g_index, event );

	switch(event)
    {
        case PORT_NOTIFIER_CREATE:
        	if( NPD_NETIF_ETH_TYPE != npd_netif_type_get(eth_g_index) )
        	{
        		return;
        	}
			if( 0 != npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
			{
				npd_qos_netif_cfg_init(eth_g_index, &qosPortCfg);				
				ret = npd_qos_netif_cfg_set_by_index( eth_g_index, &qosPortCfg);
			}
			break;
        case PORT_NOTIFIER_DELETE:
			if( NPD_NETIF_VLAN_TYPE == npd_netif_type_get(eth_g_index))
			{
				vid = npd_netif_vlan_get_vid(eth_g_index);
                ret = npd_classmap_rule_del_by_vid(vid);
			}
			if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(eth_g_index))
			{
    			if( 0 == npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
    			{
    				ret = npd_qos_netif_cfg_del_by_index( eth_g_index );
    			}
			}
            break;
	    case PORT_NOTIFIER_LINKUP_E:
    		if(0 != npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg) )
    		{				
    			break;
    		}
			npd_qos_netif_cfg_set_by_index(eth_g_index, &qosPortCfg);
            break;            

		default:
			break;
	}
	
	return ;
}


void npd_qos_handle_port_relate_event
(
    unsigned int vlan_index,
	unsigned int  eth_g_index,
	enum PORT_RELATE_ENT event,
	char *private, int len
)
{
	return ;	
}

void npd_qos_init(void) 
{
	unsigned int ret = QOS_RETURN_CODE_SUCCESS;
	ret = npd_qos_dbtbl_init();
	if( 0 != ret ) 
	{
		npd_syslog_dbg("Init QoS db table fail\n");
		return;
	}

	register_netif_notifier(&qos_port_notifier);

    syslog_ax_main_dbg("Create the timer thread for acl-rule-based-time\n");
    nam_thread_create("SysAclBasedTime", (void *)npd_acl_rule_based_tm_timer,NULL,NPD_TRUE,NPD_FALSE);

	return;	
}


unsigned int npd_netif_bind_poMapId_get(unsigned int eth_g_index,unsigned int *ID)
{
	QOS_PORT_CFG_STC qosPortCfg;
	unsigned int	ret = QOS_RETURN_CODE_SUCCESS;

	if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index,&qosPortCfg))
	{
		if( qosPortCfg.poMapCfg.poMapId != 0 ) 
		{
			npd_syslog_dbg("port has bind another policy map");
			*ID= qosPortCfg.poMapCfg.poMapId;
			ret = QOS_RETURN_CODE_POLICY_MAP_BIND;
		}
		else 
		{
			ret = QOS_RETURN_CODE_NOT_BIND_POLICYMAP;
		}
	}
	else 
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
		ret = QOS_RETURN_CODE_ERROR;
	}
		
	return ret;
}


unsigned int npd_netif_bind_poMap_check(unsigned int eth_g_index)
{
	QOS_PORT_CFG_STC qosPortCfg;
	int ret = NPD_FALSE;

	if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
	{
		if( qosPortCfg.poMapCfg.poMapId != 0 ) 
		{
			ret = NPD_TRUE;
		}
		else 
		{
			ret = NPD_FALSE;
		}
	}
	else 
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
	}
		
	return ret;
}

unsigned int npd_qos_profile_bind_check( unsigned int qosIndex)
{
	QOS_PROFILE_STC qosProfile;
	
	if( 0 != npd_qosprofile_get_by_index( qosIndex, &qosProfile))
	{
		return QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}

	if( qosProfile.swPortNum > 0 )
	{
		return QOS_RETURN_CODE_PROFILE_IN_USE;
	}

	return QOS_RETURN_CODE_SUCCESS;
}

unsigned int npd_qos_policer_bind_check( unsigned int policerId)
{
	QOS_POLICER_STC policer;
	
	if( 0 != npd_policer_get_by_index( policerId, &policer))
	{
		return QOS_RETURN_CODE_POLICER_NOT_EXISTED;
	}

	if( policer.swPortNum > 0 )
	{
		return QOS_RETURN_CODE_POLICER_USE_IN_ACL;
	}

	return QOS_RETURN_CODE_SUCCESS;
}

unsigned int npd_qos_counter_bind_check( unsigned int counterIndex)
{
	QOS_COUNTER_STC counter;
	
	if( 0 != npd_counter_get_by_index( counterIndex, &counter))
	{
		return QOS_RETURN_CODE_COUNTER_NOT_EXISTED;
	}

	if( counter.swPortNum > 0 )
	{
		npd_syslog_dbg("npd_qos_counter_bind_check: counter %d bind %d\n", counterIndex, counter.swPortNum);
		return QOS_RETURN_CODE_POLICER_USE_IN_ACL;
	}

	return QOS_RETURN_CODE_SUCCESS;
}

unsigned int npd_qos_policy_bind_check(unsigned int policyIndex)
{
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC	poMap;
	int ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;

	if( 0 == npd_policymap_get_by_index( policyIndex, &poMap))
	{
		if (poMap.eth_count == 0 ) 
		{
			ret = QOS_RETURN_CODE_SUCCESS;
		}
		else
		{
			ret = QOS_RETURN_CODE_POLICY_MAP_BIND;		
		}
	}
	else 
	{
		ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
	}

	return ret;
	
}

unsigned int npd_qos_profile_index_check(unsigned int profileIndex)
{
	QOS_PROFILE_STC profile;
	if( 0 != npd_qosprofile_get_by_index( profileIndex, &profile)) 
	{
		return QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}

	return QOS_RETURN_CODE_SUCCESS;
}

unsigned int npd_qos_port_bind_opt(unsigned int eth_g_index,unsigned int policyMapIndex)
{
	unsigned int					   ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap; 
	unsigned int   eth_arr_index = 0;
	

	if( 0 != npd_policymap_get_by_index( policyMapIndex, &poMap))
	{
		npd_syslog_dbg("policy map %d does not exist\n", policyMapIndex );
		ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
	}	
	else
	{			
		/*policymap maintain port index*/
		eth_arr_index = eth_port_array_index_from_ifindex( eth_g_index );
		NPD_PBMP_PORT_ADD(poMap.eth_mbr, eth_arr_index);
		poMap.eth_count += 1;
		npd_syslog_dbg("poMap %p poEth 0x%x eth_count %d\n",&poMap,eth_g_index, poMap.eth_count);
		npd_policymap_set_by_index( policyMapIndex, &poMap);
		ret = QOS_RETURN_CODE_SUCCESS;
	 }		 	

	return ret;
}
unsigned int npd_netif_bind_qosprofile(unsigned int eth_g_index)
{
	QOS_PORT_CFG_STC qosPortCfg;	
	
	if( 0 != npd_qos_netif_cfg_get_by_index(eth_g_index,&qosPortCfg))
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
		return 0;
	}
    
    return qosPortCfg.qosProfileIndex;
}
unsigned int npd_qos_port_bind_qosprofile(unsigned int eth_g_index,unsigned int profile)
{
	QOS_PORT_CFG_STC qosPortCfg;	
	unsigned int					   ret = QOS_RETURN_CODE_SUCCESS;
	
	if( 0 != npd_qos_netif_cfg_get_by_index(eth_g_index,&qosPortCfg))
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
		return QOS_RETURN_CODE_ERROR;
	}

	{			
		/*port maintain policy map index*/
		qosPortCfg.qosProfileIndex = profile;

		/*policymap maintain port index*/
		ret = npd_qos_netif_cfg_set_by_index(eth_g_index,&qosPortCfg);
	}		 	

	return ret;
}

unsigned int npd_qos_port_unbind_opt(unsigned int eth_g_index,unsigned int policyMapIndex)
{
	unsigned int					   ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap;
	unsigned int                       eth_arr_index = 0;

	
	{
		{
			/*clear map info about eth*/
			if (0 == npd_policymap_get_by_index(policyMapIndex, &poMap))
			{			
				syslog_ax_qos_dbg("policymap %d has eth port count %d\n",poMap.index, poMap.eth_count);
				eth_arr_index = eth_port_array_index_from_ifindex( eth_g_index );
				NPD_PBMP_PORT_REMOVE(poMap.eth_mbr, eth_arr_index);
				poMap.eth_count -=1;
				npd_policymap_set_by_index(policyMapIndex, &poMap);
				ret = QOS_RETURN_CODE_SUCCESS;
			}
		}
    }
	
	return ret;
}


unsigned int npd_qos_port_unbind_qosprofile(unsigned int eth_g_index,unsigned int profile)
{
	QOS_PORT_CFG_STC qosPortCfg;
	unsigned int					   ret = QOS_RETURN_CODE_SUCCESS;

	if( 0 != npd_qos_netif_cfg_get_by_index(eth_g_index,&qosPortCfg))
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
		return QOS_RETURN_CODE_ERROR;
	}
	
	{			
		/*port maintain policy map index*/
		qosPortCfg.qosProfileIndex = 0;

		/*policymap maintain port index*/
		npd_qos_netif_cfg_set_by_index(eth_g_index,&qosPortCfg);
		ret = QOS_RETURN_CODE_SUCCESS;
	 }		 	
	
	return ret;
}

DBusMessage * npd_dbus_config_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;    
	DBusMessageIter		iter;
	DBusError			err;          
	unsigned int        profileIndex =0;
	unsigned int        ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PROFILE_STC     profilePtr;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	DBUS_TYPE_UINT32,&profileIndex,
	DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
	
	if( 0 != npd_qosprofile_get_by_index( profileIndex, &profilePtr)) 
	{
		memset(&profilePtr, 0, sizeof(QOS_PROFILE_STC));
		profilePtr.index = profileIndex;
        profilePtr.dropPrecedence = QOS_DP_DONOT_CARE_E;
        profilePtr.userPriority = QOS_PROFILE_DONOT_CARE_E;
        profilePtr.trafficClass = QOS_PROFILE_DONOT_CARE_E;
        profilePtr.dscp = QOS_PROFILE_DONOT_CARE_E;
		if( 0 != npd_qosprofile_set_by_index( profileIndex, &profilePtr))
		{
			ret = QOS_RETURN_CODE_ERROR;
		}
	}
	
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage * npd_dbus_qos_profile_attributes(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	QOS_PROFILE_STC profilePtr;
	unsigned int    profileIndex =0;
    unsigned int    param0 = 0;
    unsigned int    att_flag = 0;
	unsigned int    ret = QOS_RETURN_CODE_SUCCESS;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,  
        	DBUS_TYPE_UINT32,&profileIndex,
        	DBUS_TYPE_UINT32,&param0,
        	DBUS_TYPE_UINT32,&att_flag,
        	DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
	npd_syslog_dbg("output profileIndex %d ,param0= %d, att_flag= %d", profileIndex, param0, att_flag);

	if (0 != npd_qosprofile_get_by_index(profileIndex, &profilePtr))
	{
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}
	else 
	{
		if (ret == QOS_RETURN_CODE_SUCCESS) 
		{
            switch (att_flag)
            {
                case 1:
                {
                    if (1 == param0) 
        			{
        				profilePtr.dropPrecedence = QOS_DP_GREEN_E;
        			}
                    else if (2 == param0)
                    {
         				profilePtr.dropPrecedence = QOS_DP_YELLOW_E;
                   }
        			else if (3 == param0)
        			{
        				profilePtr.dropPrecedence = QOS_DP_RED_E;
        			}
                    else
                    {
                    	profilePtr.dropPrecedence = QOS_DP_DONOT_CARE_E;
                    }
                    break;
                }
                case 2:
                {
                    profilePtr.userPriority = param0;
                    break;
                }
                case 3:
                {
                    profilePtr.trafficClass = param0;
                    break;
                }
                case 4:
                {
                    profilePtr.dscp = param0;
                    break;
                }
                default:
                {
                    ret = QOS_RETURN_CODE_ERROR;
                    break;
                }
            }

			if (ret == QOS_RETURN_CODE_SUCCESS) 
			{
                if (0 == npd_qosprofile_set_by_index(profileIndex, &profilePtr))
                {
				    ret = QOS_RETURN_CODE_SUCCESS;
                }
                else
                {
                    ret = QOS_RETURN_CODE_ERROR;
                }
			}		
		}
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage * npd_dbus_defaule_profile_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	unsigned int       dscp = 0, profileIndex = 0, eth_g_index = 0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PROFILE_STC          qosProfile = {0};	
    int remark;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32, &profileIndex,
		DBUS_TYPE_UINT32, &eth_g_index,
		DBUS_TYPE_UINT32, &remark,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_syslog_dbg("output profileIndex %d,dscp %d",profileIndex,dscp);

	if( 0 == npd_qosprofile_get_by_index( profileIndex, &qosProfile) )
	{
		ret = npd_qos_port_bind_qosprofile(eth_g_index, profileIndex);
		if (QOS_RETURN_CODE_SUCCESS != ret) 
		{
			npd_syslog_dbg(" bind port to policymap fail, ret = %d\n", ret);
		}
        NPD_QOS_INC(qosProfile.swPortNum);
		npd_qosprofile_set_by_index( profileIndex, &qosProfile);
	}
	else
	{
		npd_syslog_dbg("qos profile not existed");
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}
   	


	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage * npd_dbus_delete_default_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	unsigned int       index=0, qosIndex = 0;
    unsigned int       eth_g_index = 0;
    unsigned int	   ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PROFILE_STC    qosProfile = {0};

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&index,	  
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}  
    qosIndex = npd_netif_bind_qosprofile(eth_g_index);
    
	if( 0 == npd_qosprofile_get_by_index(qosIndex, &qosProfile))
	{
		NPD_QOS_DEC(qosProfile.swPortNum);
		npd_qosprofile_set_by_index(qosIndex, &qosProfile);
	}
	else
	{
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}    
    {           
        ret = npd_qos_port_unbind_qosprofile(eth_g_index, 0);
        if(ret != QOS_RETURN_CODE_SUCCESS)
        {
            npd_syslog_dbg("sw fail!\n");
        }
        else 
        {
            npd_syslog_dbg("success clear sw data!\n"); 
        }
    }               

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;
}

DBusMessage * npd_dbus_qos_port_eg_remark(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

    unsigned int       eth_g_index = 0;
    unsigned int	   ret = QOS_RETURN_CODE_SUCCESS;
    int remark;
	QOS_PORT_CFG_STC    qosPortCfg = {0};
    int dir;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&remark,	
		DBUS_TYPE_UINT32,&dir,
		DBUS_TYPE_UINT32,&eth_g_index,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	} 
	if( 0 != npd_qos_netif_cfg_get_by_index(eth_g_index,&qosPortCfg))
	{
		npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
		return NULL;
	}
    if(dir == ACL_DIRECTION_INGRESS_E)
	    qosPortCfg.ingressRemark = remark;
        
    else
	    qosPortCfg.egressRemark = remark;
    ret = npd_qos_netif_cfg_set_by_index(eth_g_index, &qosPortCfg);

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;
}


DBusMessage * npd_dbus_dscp_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	unsigned int       profileIndex = 0,ingress_dscp = 0, egress_dscp = 0, policymapIndex = 0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PROFILE_STC          qosProfile = {0};	
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap = {0};	

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32, &ingress_dscp,
		DBUS_TYPE_UINT32, &egress_dscp,
		DBUS_TYPE_UINT32, &profileIndex,
		DBUS_TYPE_UINT32, &policymapIndex,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_syslog_dbg("output profileIndex %d,dscp %d\n",profileIndex, ingress_dscp);
	
	if( 0 == npd_qosprofile_get_by_index( profileIndex, &qosProfile) )
	{
		if( 0 == npd_policymap_get_by_index(policymapIndex, &poMap) )
		{
            if(ingress_dscp != -1)
			    poMap.dscpMapQosProfile[ingress_dscp].profileIndex = profileIndex;
			npd_policymap_set_by_index(policymapIndex, &poMap);
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
		}
	}
	else
	{
		npd_syslog_dbg("qos profile not existed\n");
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}


DBusMessage * npd_dbus_delete_dscp_to_qos_profile_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	unsigned int       dscp=0, policymapIndex=0, qosIndex = 0;
	unsigned int	   ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap = {0};
	QOS_PROFILE_STC    qosProfile = {0};

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&dscp,	  
		DBUS_TYPE_UINT32,&policymapIndex,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s\n",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}  

	if( 0 == npd_policymap_get_by_index(policymapIndex, &poMap))
	{
		if( poMap.dscpMapQosProfile[dscp].profileIndex != 0 ) 
		{
			qosIndex = poMap.dscpMapQosProfile[dscp].profileIndex;
			poMap.dscpMapQosProfile[dscp].profileIndex = 0;

			if( 0 == npd_qosprofile_get_by_index(qosIndex, &qosProfile))
			{
				npd_policymap_set_by_index(policymapIndex, &poMap);
			}
			else
			{
				npd_syslog_dbg("(npd_dbus_delete_dscp_to_qos_profile_table)no qos profile existed\n");
				ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
			}
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICY_NO_MAPPED;
		}
	}
	else 
	{
		ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;	    
}

DBusMessage * npd_dbus_dscp_to_dscp_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	unsigned int       oldDscp =0,newDscp=0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
	unsigned int        policymapIndex = 0;

	memset(&poMap, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err, 
		DBUS_TYPE_UINT32,&policymapIndex,
		DBUS_TYPE_UINT32,&oldDscp,
		DBUS_TYPE_UINT32,&newDscp,
		DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args \n");
		if (dbus_error_is_set(&err))
		{
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
		}
		return NULL;
	}

	if( !NPD_QOS_POLICER_INDEX_VALID(policymapIndex) )
	{
		ret = QOS_RETURN_CODE_BAD_PARAM;
	}
	else if( !NPD_QOS_DSCP_VALID(oldDscp) || !NPD_QOS_DSCP_VALID(newDscp) )
	{
		ret = QOS_RETURN_CODE_BAD_PARAM;
	}

	if( ret == QOS_RETURN_CODE_SUCCESS )
	{
		if( 0 == npd_policymap_get_by_index(policymapIndex, &poMap))
		{
			poMap.dscpMapdscp[oldDscp] = newDscp;
			npd_policymap_set_by_index(policymapIndex, &poMap);
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
		}
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}


DBusMessage * npd_dbus_delete_dscp_to_dscp_table(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      
    
    unsigned int       oldDscp =0;
    unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	unsigned int       policymapIndex = 0;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
   
    dbus_error_init(&err);
    if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policymapIndex,
       DBUS_TYPE_UINT32,&oldDscp,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }   

	if( !NPD_QOS_POLICER_INDEX_VALID(policymapIndex) )
	{
		ret = QOS_RETURN_CODE_BAD_PARAM;
	}
	else if( !NPD_QOS_DSCP_VALID(oldDscp) )
	{
		ret = QOS_RETURN_CODE_BAD_PARAM;
	}

	if( ret == QOS_RETURN_CODE_SUCCESS )
	{
		if( 0 == npd_policymap_get_by_index(policymapIndex, &poMap))
		{
			poMap.dscpMapdscp[oldDscp] = oldDscp;
			npd_policymap_set_by_index(policymapIndex, &poMap);
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}


DBusMessage *npd_dbus_up_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      
	   
	unsigned int    ingress_up = 0, egress_up = 0, profileIndex = 0, policyMapIndex = 0;
	unsigned int    ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap = {0};
	QOS_PROFILE_STC qosProfile = {0};
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&ingress_up,
		DBUS_TYPE_UINT32,&egress_up,
		DBUS_TYPE_UINT32,&profileIndex,
		DBUS_TYPE_UINT32,&policyMapIndex,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
	npd_syslog_dbg("output up %d,profileIndex %d", ingress_up, profileIndex);
	if( 0 == npd_qosprofile_get_by_index( profileIndex, &qosProfile) )
	{
		if( 0 == npd_policymap_get_by_index(policyMapIndex, &poMap))
		{
            if(ingress_up != -1)
			    poMap.upMapQosProfile[ingress_up].profileIndex = profileIndex;
			npd_policymap_set_by_index(policyMapIndex, &poMap);
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
		}
	}
	else
	{
		npd_syslog_dbg("no qos profile existed");
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}
	
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage *npd_dbus_qos_profile_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      
	   
	unsigned int    profile_index = 0, remarkParamValue = 0, policymap_index = 0;
	unsigned int    ret = QOS_RETURN_CODE_SUCCESS;


	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&profile_index,
		DBUS_TYPE_UINT32,&remarkParamValue,
        DBUS_TYPE_UINT32,&policymap_index,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
    
	npd_syslog_dbg("profile index %d,remarkParamValue %d", profile_index, remarkParamValue);
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage *npd_dbus_delete_up_to_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      
       
	unsigned int    up = 0, policymapIndex = 0, qosIndex = 0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC  poMap;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&up,
	   DBUS_TYPE_UINT32,&policymapIndex,
	   DBUS_TYPE_INVALID))) 
	{
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) 
	   {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}   

	if( 0 == npd_policymap_get_by_index(policymapIndex, &poMap))
	{
		if( poMap.upMapQosProfile[up].profileIndex != 0 )
		{
			qosIndex = poMap.upMapQosProfile[up].profileIndex;
            if(up == 0)
			    poMap.upMapQosProfile[up].profileIndex = up*8+1;
            else
			    poMap.upMapQosProfile[up].profileIndex = up*8+3;
			npd_policymap_set_by_index(policymapIndex, &poMap);
		}
		else
		{
			ret = QOS_RETURN_CODE_POLICY_NO_MAPPED;
		}
	}
	else
	{
		ret = QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}

DBusMessage * npd_dbus_delete_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;	 
	DBusMessageIter iter;
	DBusError		err; 	 

	unsigned int 	  profileIndex =0;
	unsigned int 	  ret = QOS_RETURN_CODE_SUCCESS;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&profileIndex,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
	npd_syslog_dbg("output profileIndex %d",profileIndex);

	ret= npd_qos_profile_bind_check(profileIndex);

	if (QOS_RETURN_CODE_SUCCESS == ret)
	{
		ret = -1;
		if( 0 == npd_qosprofile_del_by_index( profileIndex ) ) 
		{
			ret = QOS_RETURN_CODE_SUCCESS; 
		}		
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	
}

DBusMessage * npd_dbus_config_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	  reply;	
	DBusMessageIter	  iter;
	DBusError		  err;
	int ni = 0;
	unsigned int      policyIndex=0;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
	unsigned int	  ret =QOS_RETURN_CODE_SUCCESS;

	dbus_error_init(&err);	   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policyIndex,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if( !NPD_QOS_POLICER_INDEX_VALID(policyIndex))
	{
		ret = QOS_RETURN_CODE_BAD_PARAM;
	}
	else if(0 == npd_policymap_get_by_index( policyIndex, &poMap))
	{
		ret = QOS_RETURN_CODE_SUCCESS;
	}
	else 
	{
		memset(&poMap, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));
		poMap.index = policyIndex;
		poMap.modifyDscp = DISABLE_E;
		poMap.modifyUp	 = DISABLE_E;

		for (ni = 0; ni < MAX_DSCP_DSCP_NUM; ni++)
		{
			poMap.dscpMapdscp[ni] = ni;
		}
		npd_policymap_set_by_index( policyIndex, &poMap);
		ret = QOS_RETURN_CODE_SUCCESS;
	}

	reply = dbus_message_new_method_return(msg);	   
	dbus_message_iter_init_append (reply, &iter);	   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);	
	return reply;
}

DBusMessage * npd_dbus_policy_modify_qos(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	   reply;    
	DBusMessageIter	   iter;
	DBusError		   err;          
	unsigned int       IsEnable =0, policyMapIndex=0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&IsEnable,
		DBUS_TYPE_UINT32,&policyMapIndex,
		DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}   
	npd_syslog_dbg("output IsEnable %d,policyMapIndex %d",IsEnable,policyMapIndex);

	if( 0 == npd_policymap_get_by_index( policyMapIndex, &poMap) )
	{
		if (QOS_TRUE == IsEnable)
		{
			poMap.modifyDscp = ENABLE_E;
			poMap.modifyUp = ENABLE_E;
		}
		else
		{
			poMap.modifyDscp = KEEP_E;
			poMap.modifyUp = KEEP_E;
		}
		npd_policymap_set_by_index( policyMapIndex, &poMap);
		ret = QOS_RETURN_CODE_SUCCESS;
	}
	else
	{
		ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
	}

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;	    
}


DBusMessage * npd_dbus_policy_trust_mode_l2_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	   reply;	 
	DBusMessageIter    iter;
	DBusError		   err; 		 
	unsigned int	   upEnable =1, policyMapIndex=0;
    unsigned int       eth_g_index = 0;    
	unsigned int	   ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_CFG_STC	qosPortCfg = {0};

	memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
    
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&upEnable,
	   DBUS_TYPE_UINT32,&policyMapIndex,
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	npd_syslog_dbg("output upEnable %d,policyMapIndex %d",upEnable,policyMapIndex);

	if (0 != npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
	{
		qosPortCfg.ifIndex = eth_g_index;
        
	}
    
    if(ENABLE_E == (QOS_NORMAL_ENABLE_E)upEnable)
    {
        switch(qosPortCfg.trust)
        {
            case QOS_TRUST_L3:
                qosPortCfg.trust = QOS_TRUST_L2L3;
                break;
            default:
                qosPortCfg.trust = QOS_TRUST_L2;
                break;
        }
    }
    else
    {
        switch(qosPortCfg.trust)
        {
            case QOS_TRUST_L2L3:
                qosPortCfg.trust = QOS_TRUST_L3;
                break;
            case QOS_TRUST_L3:
                break;
            default:
                qosPortCfg.trust = QOS_TRUST_PORT;
                break;
        }
    }

    npd_qos_netif_cfg_set_by_index(eth_g_index, &qosPortCfg);
    if(qosPortCfg.trust != QOS_TRUST_PORT) 
    	ret = npd_qos_port_bind_opt(eth_g_index, SYSTEM_QOS_POLICY);
    else
    	ret = npd_qos_port_unbind_opt(eth_g_index, SYSTEM_QOS_POLICY);
        
    reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;		
}

DBusMessage * npd_dbus_policy_trust_mode_l3dscp_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	   reply;	 
	DBusMessageIter    iter;
	DBusError		   err; 		 
	unsigned int	   dscpEnable =1,dscpRemap=1,policyMapIndex=0;
	unsigned int	   ret = QOS_RETURN_CODE_SUCCESS;
    unsigned int       eth_g_index = 0;
	QOS_PORT_CFG_STC	qosPortCfg = {0};
    
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&dscpEnable,
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_UINT32,&policyMapIndex,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		} 
		return NULL;
	}

	npd_syslog_dbg("output dscpEnable %d,dscpRemap %d,policyMapIndex %d",dscpEnable,dscpRemap,policyMapIndex);

	if (0 != npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
	{
		qosPortCfg.ifIndex = eth_g_index;
	}
    
    if(ENABLE_E == (QOS_NORMAL_ENABLE_E)dscpEnable)
    {
        switch(qosPortCfg.trust)
        {
            case QOS_TRUST_L2:
                qosPortCfg.trust = QOS_TRUST_L2L3;
                break;
            default:
                qosPortCfg.trust = QOS_TRUST_L3;
                break;
        }
    }
    else
    {
        switch(qosPortCfg.trust)
        {
            case QOS_TRUST_L2L3:
                qosPortCfg.trust = QOS_TRUST_L2;
                break;
            case QOS_TRUST_L2:
                break;
            default:
                qosPortCfg.trust = QOS_TRUST_PORT;
                break;
        }
    }

    npd_qos_netif_cfg_set_by_index(eth_g_index, &qosPortCfg);
    if(qosPortCfg.trust != QOS_TRUST_PORT) 
    	ret = npd_qos_port_bind_opt(eth_g_index, SYSTEM_QOS_POLICY);
    else
    	ret = npd_qos_port_unbind_opt(eth_g_index, SYSTEM_QOS_POLICY);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
    
	return reply;
}

DBusMessage * npd_dbus_delete_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;	  
	DBusMessageIter iter;
	DBusError		err;
	unsigned int    policyIndex =0, qosIndex = 0, i;	
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC poMap;
	QOS_PROFILE_STC  qosProfile;
	unsigned int    ret = QOS_RETURN_CODE_SUCCESS;
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policyIndex,
	   DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if( 0 != npd_policymap_get_by_index( policyIndex, &poMap)) 
	{
		ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
		npd_syslog_err("not existed this policy map\n");
	}
	else if(QOS_RETURN_CODE_POLICY_MAP_BIND == npd_qos_policy_bind_check(policyIndex)) 
	{
		ret = QOS_RETURN_CODE_POLICY_MAP_BIND;
		npd_syslog_err("since bind to some port ,can not delete");
	}
	else 
	{
		for(i=0;i<MAX_DSCP_PROFILE_NUM;i++)
		{
			qosIndex = poMap.dscpMapQosProfile[i].profileIndex;
			if( qosIndex == 0 )
			{
				continue;
			}
			
			if( 0 == npd_qosprofile_get_by_index(qosIndex,&qosProfile) )
			{
				NPD_QOS_DEC(qosProfile.swPortNum);
				npd_qosprofile_set_by_index( qosIndex, &qosProfile);
			}
		}
		
		for(i=0;i<MAX_UP_PROFILE_NUM;i++)
		{
			qosIndex = poMap.upMapQosProfile[i].profileIndex;
			if( qosIndex == 0 )
			{
				continue;
			}

			if( 0 == npd_qosprofile_get_by_index(qosIndex,&qosProfile) )
			{
				NPD_QOS_DEC(qosProfile.swPortNum);
				npd_qosprofile_set_by_index( qosIndex, &qosProfile);
			}
		}
		npd_policymap_del_by_index( policyIndex );
		ret = QOS_RETURN_CODE_SUCCESS;
	}		 
	
   reply = dbus_message_new_method_return(msg);
   
   dbus_message_iter_init_append (reply, &iter);
   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);

   return reply;
}

DBusMessage * npd_dbus_ethport_unbind_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	   reply;	 
	DBusMessageIter    iter;
	DBusError		   err; 	

	unsigned int	   g_eth_index=0,policyIndex=0;
	unsigned int	   ret=QOS_RETURN_CODE_SUCCESS;
	unsigned int 		bindId=0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&g_eth_index,
	   DBUS_TYPE_UINT32,&policyIndex,
	   DBUS_TYPE_INVALID))) {
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}	
	npd_syslog_dbg("output g_eth_index %d,policyIndex %d",g_eth_index,policyIndex);

	/*check if bind policymap*/
	ret=npd_netif_bind_poMapId_get(g_eth_index,&bindId);
	if (QOS_RETURN_CODE_ERROR == ret)
	{
		npd_syslog_dbg("wrong eth_g_index and port struct");
	}
	else if(ret==QOS_RETURN_CODE_POLICY_MAP_BIND)
	{
		npd_syslog_dbg("policy map %d on port",bindId);
		if(bindId!=policyIndex)
		{
			ret = QOS_RETURN_CODE_POLICY_MAP_PORT_WRONG;
			npd_syslog_dbg("wrong policy index");
		}
		else
		{			
			ret = npd_qos_port_unbind_opt(g_eth_index,policyIndex);
			if(ret!=QOS_RETURN_CODE_SUCCESS)
			{
				npd_syslog_dbg("sw fail!\n");
			}
			else 
			{
				npd_syslog_dbg("success clear sw data!\n");	
			}
		}				
	}			
	else
	{
		npd_syslog_dbg("policy map not bind on this port!\n");	
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	
	return reply;
}

DBusMessage * npd_dbus_ethport_bind_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	   reply;	 
	DBusMessageIter    iter;
	DBusError		   err; 	

	unsigned int	    g_eth_index=0;
	unsigned int		policyIndex=0;
	unsigned int	    ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_CFG_STC	qosPortCfg;

	memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&g_eth_index,
	   DBUS_TYPE_UINT32,&policyIndex,
	   DBUS_TYPE_INVALID))) 
	{
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) 
	   {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}

	if (0 != npd_qos_netif_cfg_get_by_index(g_eth_index, &qosPortCfg))
	{
		npd_syslog_dbg("% Can not get qos cfg on port index %d", g_eth_index);
		ret = QOS_RETURN_CODE_ERROR;
	}

    if (QOS_RETURN_CODE_SUCCESS == ret)
	{
		ret = npd_qos_port_bind_opt(g_eth_index,policyIndex);
		if (QOS_RETURN_CODE_SUCCESS != ret) 
		{
			npd_syslog_dbg(" bind port to policymap fail, ret = %d", ret);
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;
}

DBusMessage * npd_dbus_show_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*			reply = NULL;
	DBusError				err;	
    DBusMessageIter         iter;
	unsigned int			policymap_index = 0;
	unsigned int			next_index = 0;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC 		 poMap;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC 		 poMap_next;

	memset(&poMap, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));
	memset(&poMap_next, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
			DBUS_TYPE_UINT32, &policymap_index,
			DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (0 == policymap_index)
	{
		for (next_index = policymap_index; next_index < MAX_POLICY_MAP_NUM; next_index++)
		{
			if (0 == npd_policymap_get_by_index(next_index, &poMap_next))
			{
				policymap_index = next_index;
				break;
			}
		}
	}
 
	npd_policymap_get_by_index(policymap_index, &poMap);
	
	for (next_index = policymap_index + 1; next_index <= MAX_POLICY_MAP_NUM; next_index++)
	{
		if (0 == npd_policymap_get_by_index(next_index, &poMap_next))
		{
			break;
		}
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &next_index);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &poMap.index);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &poMap.domain);
	{
        int j;
        for(j = 0; j < 8; j++)
		    dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &poMap.upMapQosProfile[j].profileIndex);
        for(j = 0; j < 64; j++)
		    dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &poMap.dscpMapQosProfile[j].profileIndex);
	}

	return reply;	
}

DBusMessage * npd_dbus_show_qos_profile(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*			reply = NULL;
	DBusMessageIter			iter;		 
	DBusError				err;	
	unsigned int			rule_index = 0;
	unsigned int			next_index = 0;
	QOS_PROFILE_STC			qosProfile;
	QOS_PROFILE_STC			qosProfile_next;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
			DBUS_TYPE_UINT32, &rule_index,
			DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (0 == rule_index)
	{
		for (next_index = rule_index; next_index < MAX_PROFILE_NUMBER; next_index++)
		{
			if (0 == npd_qosprofile_get_by_index(next_index, &qosProfile_next))
			{
				rule_index = next_index;
				break;
			}
		}
	}
 
	npd_qosprofile_get_by_index(rule_index, &qosProfile);
	
	for (next_index = rule_index + 1; next_index <= MAX_PROFILE_NUMBER; next_index++)
	{
		if (0 == npd_qosprofile_get_by_index(next_index, &qosProfile_next))
		{
			break;
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &next_index);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.index);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.dropPrecedence);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.userPriority);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.trafficClass);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.dscp);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.exp);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &qosProfile.swPortNum);

	return reply;	
}

DBusMessage * npd_dbus_ethport_show_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*			reply = NULL;
	DBusMessageIter 		iter;
	DBusError				err;
	unsigned int    		g_index = 0;
	unsigned int 			ret = QOS_RETURN_CODE_SUCCESS;
	QOS_PORT_CFG_STC qosPortCfg;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32,&g_index,
			DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}		

   
    ret = npd_qos_netif_cfg_get_by_index(g_index, &qosPortCfg);
    if(0 != ret)
    {
        ret = QOS_RETURN_CODE_POLICY_NOT_EXISTED;
    }
    
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&qosPortCfg.egressRemark);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&qosPortCfg.ingressRemark);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&qosPortCfg.qosProfileIndex);
 	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&qosPortCfg.poMapCfg.poMapId);
 	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&qosPortCfg.trust);
   

	return reply;	
}

DBusMessage * npd_dbus_policer_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int    	policerIndex =0;
    unsigned int    	ret = QOS_RETURN_CODE_SUCCESS;
	QOS_POLICER_STC 	policer;
    int dir = 0;
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policerIndex,
	   DBUS_TYPE_UINT32,&dir,
	   DBUS_TYPE_INVALID))) 
	{
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) 
	   {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}
	npd_syslog_dbg("policer index %d dir %d\n",policerIndex, dir);
	memset( &policer,0,sizeof(QOS_POLICER_STC));

	if( 0 != npd_policer_get_by_index(policerIndex, &policer) )
	{
		policer.cir = QOS_CIR_MAX_RATE;
		policer.cbs = QOS_CBS_BURST;
		policer.pbs = QOS_PBS_BURST;
		policer.pir = QOS_PIR_MAX_RATE;

		policer.index = policerIndex;
        policer.direction = dir; 
	}
	
	policer.policerEnable = QOS_POLICER_ENABLE;
	npd_policer_set_by_index(policerIndex, &policer);
	ret = QOS_RETURN_CODE_SUCCESS;

   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append (reply, &iter);   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);

   return reply;

}


DBusMessage * npd_dbus_policer_set_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		policerIndex= 0, startPid = 0, endPid = 0;
	unsigned int 		ret = QOS_RETURN_CODE_SUCCESS;

	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
								DBUS_TYPE_UINT32, &policerIndex,
								DBUS_TYPE_UINT32, &startPid,
								DBUS_TYPE_UINT32, &endPid,
								DBUS_TYPE_INVALID))) {
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
	return reply;

}


DBusMessage * npd_dbus_policer_cir_cbs(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	QOS_POLICER_STC    policer;
	unsigned int       policerIndex =0;
	unsigned int      cir=0,cbs=0;
    unsigned int      pir = 0, pbs = 0;
	unsigned int       ret = QOS_RETURN_CODE_SUCCESS;
    int dir;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32, &policerIndex,
		DBUS_TYPE_UINT32, &dir,
		DBUS_TYPE_UINT32, &cir,
		DBUS_TYPE_UINT32, &cbs,
		DBUS_TYPE_UINT32, &pir,
		DBUS_TYPE_UINT32, &pbs,
	    DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        return NULL;
	}   
	npd_syslog_dbg("output profileIndex %d ,cir %ld,cbs %ld",policerIndex,cir,cbs);


	ret = npd_qos_policer_bind_check( policerIndex) ;
	
	if(QOS_RETURN_CODE_SUCCESS == ret )
	{
		if( 0 == npd_policer_get_by_index(policerIndex, &policer))
		{	
            policer.cir = cir;
            policer.cbs = cbs;
            policer.pir = pir;
            policer.pbs = pbs;
            policer.direction = dir;
			npd_policer_set_by_index(policerIndex, &policer);
			ret = QOS_RETURN_CODE_SUCCESS;
		}
		else
		{
			ret = QOS_RETURN_CODE_POLICER_NOT_EXISTED;
		}
	}

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;	    
}

DBusMessage * npd_dbus_policer_packetcolor(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;      

	int packetcolor = 0;
	unsigned int policerIndex = 0;
	unsigned int profileId = 0;
	unsigned int ret = QOS_RETURN_CODE_SUCCESS;
	QOS_POLICER_STC    policer;
    QOS_PROFILE_STC    profilePtr = { 0 };
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
       	  DBUS_TYPE_UINT32,&policerIndex,
	      DBUS_TYPE_UINT32,&packetcolor,
	      DBUS_TYPE_UINT32,&profileId,
	      DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
        return NULL;
	}   
	npd_syslog_dbg("output packetcolor %d ,profileId %d\n",packetcolor,profileId);

	ret = npd_qos_policer_bind_check(policerIndex);
	
	if(QOS_RETURN_CODE_SUCCESS == ret )
	{
		{
			if(0 == npd_policer_get_by_index(policerIndex, &policer))
			{
                if(0 == npd_qosprofile_get_by_index(profileId, &profilePtr))
                {
                    if(packetcolor == 0)
                    {
                        policer.gPktParam.cmd = OUT_PROFILE_REMAP_ENTRY;
                        policer.gPktParam.qosProfileID = profileId;
                    }
                    else if(packetcolor == 1)
                    {
                        if(profilePtr.userPriority != QOS_PROFILE_DONOT_CARE_E)
                        {                         
                            policer.modifyUp = ENABLE_E;
                        }
                        if(profilePtr.dscp != QOS_PROFILE_DONOT_CARE_E)
                        {                         
                            policer.modifyDscp = ENABLE_E;
                        }
                        policer.yPktParam.cmd = OUT_PROFILE_REMAP_ENTRY;   
                        policer.yPktParam.qosProfileID = profileId;
                    }
                    else if(packetcolor == 2)
                    {
                        if(profilePtr.userPriority != QOS_PROFILE_DONOT_CARE_E)
                        {                       
                            policer.modifyUp = ENABLE_E;
                        }
                        if(profilePtr.dscp != QOS_PROFILE_DONOT_CARE_E)
                        {                       
                            policer.modifyDscp = ENABLE_E;
                        }
                        policer.rPktParam.cmd = OUT_PROFILE_REMAP_ENTRY;     
                        policer.rPktParam.qosProfileID = profileId;
                    }
    				npd_policer_set_by_index(policerIndex, &policer);
                    profilePtr.swPortNum++;
                    npd_qosprofile_set_by_index(profileId, &profilePtr);
    				ret = QOS_RETURN_CODE_SUCCESS;
                }
			}
			else
			{
				ret = QOS_RETURN_CODE_POLICER_NOT_EXISTED;
			}
		}
	}

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;	    
}

DBusMessage * npd_dbus_policer_out_profile_cmd_keep_drop(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	  reply;	  
	DBusMessageIter   iter;
	DBusError		  err;   
	QOS_POLICER_STC   policer;
	unsigned int	  policerIndex =0,action = 0;
    int color;
	unsigned int	  ret = QOS_RETURN_CODE_SUCCESS;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
		  DBUS_TYPE_UINT32,&policerIndex,
		  DBUS_TYPE_UINT32, &color,
		  DBUS_TYPE_UINT32,&action,
		  DBUS_TYPE_INVALID))) 
	{
		  npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	npd_syslog_dbg("output profileIndex %d,action %d",policerIndex,action);

	ret = npd_qos_policer_bind_check( policerIndex) ;
	
	if(QOS_RETURN_CODE_SUCCESS == ret )
	{
		if( 0 == npd_policer_get_by_index( policerIndex, &policer))
		{
            if(color == QOS_DP_RED_E)
            {
		        policer.rPktParam.cmd = action;
                if(action == 0)
                {
                    policer.rPktParam.transmite_flag = 1;
                }
            }
            else
            {
                policer.yPktParam.cmd = action;
                if(action == 0)
                {
                    policer.yPktParam.transmite_flag = 1;
                }
            }
			npd_policer_set_by_index(policerIndex, &policer);
			ret = QOS_RETURN_CODE_SUCCESS;
		}
		else 
		{
			ret = QOS_RETURN_CODE_POLICER_NOT_EXISTED;
		}
	}

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;
}


DBusMessage * npd_dbus_policer_counter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		policerIndex=0,countIndex=0,ret=QOS_RETURN_CODE_SUCCESS,IsEnable=QOS_POLICER_DISABLE;
	QOS_POLICER_STC     policer;
	dbus_error_init(&err);
		   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policerIndex, 
	   DBUS_TYPE_UINT32,&countIndex,
	   DBUS_TYPE_UINT32,&IsEnable,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	npd_syslog_dbg("output policerIndex %d,countIndex %d",policerIndex,countIndex);

	ret = npd_qos_policer_bind_check( policerIndex);
	
	if(QOS_RETURN_CODE_SUCCESS == ret )
	{
		if( 0 == npd_policer_get_by_index(policerIndex, &policer))
		{			
			if(IsEnable)
			{
				{
					policer.counterEnable = TRUE;
					if(0 == ret)
					{
						npd_policer_set_by_index(policerIndex, &policer);
					}
				}
			}
			else
			{
				{
					policer.counterEnable = 0;
					npd_policer_set_by_index(policerIndex, &policer);
				}
			}
				
		}
		else
		{
			npd_syslog_err("policer not existed");
			ret = QOS_RETURN_CODE_POLICER_NOT_EXISTED;
		}
	}		

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

}

DBusMessage * npd_dbus_set_counter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		countIndex=0,ret = QOS_RETURN_CODE_SUCCESS;
	unsigned long 		inIndex=0,outIndex=0;
	QOS_COUNTER_STC		counter = {0};
	
	dbus_error_init(&err);
		   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32, &countIndex, 
	   DBUS_TYPE_UINT32, &inIndex,
	   DBUS_TYPE_UINT32, &outIndex,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	npd_syslog_dbg("output countIndex %d,inIndex %ld,outIndex %ld",countIndex,inIndex,outIndex);

	if( 0 == npd_counter_get_by_index(countIndex, &counter))
	{
		if (0 == counter.swPortNum)
		{
			npd_syslog_dbg("existed this policer\n");
			counter.inProfileBytesCnt = inIndex;
			counter.outOfProfileBytesCnt =outIndex;
		}
		else
		{
			ret = QOS_RETURN_CODE_COUNTER_USED;
		}
	}
	else 
	{
		memset(&counter, 0, sizeof(QOS_COUNTER_STC));
		counter.index = countIndex;
		counter.inProfileBytesCnt = inIndex;
		counter.outOfProfileBytesCnt =outIndex;
		
		npd_syslog_dbg("inProfileBytesCnt %lu\n", counter.inProfileBytesCnt);
		npd_syslog_dbg("outOfProfileBytesCnt %lu\n",counter.outOfProfileBytesCnt);
	}
	npd_counter_set_by_index(countIndex, &counter);
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	
	return reply;
}

DBusMessage * npd_dbus_read_counter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		policerId=0,ret = 0;
	QOS_POLICER_STC 	policer;
	unsigned long long green_bytes;
	unsigned long long green_pkts;
	unsigned long long yellow_bytes;
	unsigned long long yellow_pkts;
	unsigned long long red_bytes;
	unsigned long long red_pkts;

	dbus_error_init(&err);
		   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policerId, 
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	if( 0 != npd_policer_get_by_index( policerId, &policer)) 
	{
		ret = QOS_RETURN_CODE_COUNTER_NOT_EXISTED;
	}
	else
	{
		if(policer.counterEnable != TRUE)
		{
			ret = QOS_RETURN_CODE_COUNTER_NOT_EXISTED;
		}
		else
		{
    		ret = nam_qos_read_counter(policerId, 0, &green_bytes, &green_pkts,
    			&yellow_bytes, &yellow_pkts, &red_bytes, &red_pkts);
    		if(ret!=QOS_RETURN_CODE_SUCCESS) 
    		{
		        ret = QOS_RETURN_CODE_COUNTER_NOT_EXISTED;
				
    			npd_syslog_err("fail to read counter info!\n");
    		}
		}
	}
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&green_bytes);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&green_pkts);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&yellow_bytes);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&yellow_pkts);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&red_bytes);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT64,&red_pkts);
	
	return reply;
}

DBusMessage * npd_dbus_get_policer(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int		policer_index = 0;
	DBusMessage*		reply;	  
	DBusMessageIter		iter;
	DBusError			err;
	QOS_POLICER_STC		policer;
    unsigned int        op_ret = 0;

	memset(&policer, 0, sizeof(QOS_POLICER_STC));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,  
	   DBUS_TYPE_UINT32, &policer_index, 
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    

	{
		if (0 != npd_policer_get_by_index(policer_index, &policer))
		{
            op_ret = QOS_RETURN_CODE_POLICER_NOT_EXISTED;
		}
	}


	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &op_ret);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.index);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.policerEnable);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.cir);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.cbs);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.pir);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.pbs);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.gPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.yPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.rPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.yPktParam.cmd);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.rPktParam.cmd);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.counterEnable);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.counterSetIndex);
	
	return reply;
}


DBusMessage * npd_dbus_show_policer(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int		next_index = 0;
	unsigned int		policer_index = 0;
	DBusMessage*		reply;	  
	DBusMessageIter		iter;
	DBusError			err;
	QOS_POLICER_STC		policer;
	QOS_POLICER_STC		policer_next;

	memset(&policer, 0, sizeof(QOS_POLICER_STC));
	memset(&policer_next, 0, sizeof(QOS_POLICER_STC));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,  
	   DBUS_TYPE_UINT32, &policer_index, 
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    

	while (policer_index <= MAX_POLICER_NUM)
	{
		if (0 == npd_policer_get_by_index(policer_index, &policer))
		{
            policer_index++;
			break;
		}
        else
        {
            policer_index++;
        }
	}
    next_index = policer_index;

    if (policer_index > MAX_POLICER_NUM)
    {
        next_index = 0xffff;
    }


	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &next_index);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.index);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.policerEnable);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.cir);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.cbs);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.pir);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.pbs);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.gPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.yPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.rPktParam.qosProfileID);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.yPktParam.cmd);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.rPktParam.cmd);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.counterEnable);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &policer.counterSetIndex);
	
	return reply;
}

DBusMessage * npd_dbus_delete_policer(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		policerIndex =0;
	unsigned int        counterIndex = 0, qosIndex = 0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	QOS_POLICER_STC     policer;
	QOS_PROFILE_STC     qosProfile;
	QOS_COUNTER_STC     counter;
			
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&policerIndex,
	   DBUS_TYPE_INVALID))) {
	   npd_syslog_err("Unable to get input args ");
	   if (dbus_error_is_set(&err)) {
		   npd_syslog_err("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
	}

	ret = npd_qos_policer_bind_check( policerIndex) ;
		
	if(QOS_RETURN_CODE_SUCCESS == ret )
	{
		if( 0 == npd_policer_get_by_index( policerIndex, &policer))
		{
			counterIndex = policer.counterSetIndex;
			if (0 != counterIndex)
			{
				if( 0 == npd_counter_get_by_index( counterIndex, &counter))
				{
					NPD_QOS_DEC(counter.swPortNum);
					npd_counter_set_by_index(counterIndex, &counter);
				}
			}

			qosIndex = policer.gPktParam.qosProfileID;
			if (0 != qosIndex)
			{
				if( 0 == npd_qosprofile_get_by_index( qosIndex, &qosProfile))
				{
					NPD_QOS_DEC(qosProfile.swPortNum);
					npd_qosprofile_set_by_index( qosIndex, &qosProfile);
				}
			}

			qosIndex = policer.yPktParam.qosProfileID;
			if (0 != qosIndex)
			{
				if( 0 == npd_qosprofile_get_by_index( qosIndex, &qosProfile))
				{
					NPD_QOS_DEC(qosProfile.swPortNum);
					npd_qosprofile_set_by_index( qosIndex, &qosProfile);
				}
			}

			qosIndex = policer.rPktParam.qosProfileID;
			if (0 != qosIndex)
			{
				if( 0 == npd_qosprofile_get_by_index( qosIndex, &qosProfile))
				{
					NPD_QOS_DEC(qosProfile.swPortNum);
					npd_qosprofile_set_by_index( qosIndex, &qosProfile);
				}
			}
		}
		npd_policer_del_by_index( policerIndex );
		
		ret = QOS_RETURN_CODE_SUCCESS;
	}

		/* npd_error  can not delete,npd_succes ,can*/
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append (reply, &iter);   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);

   return reply;
}

DBusMessage * npd_dbus_queue_scheduler(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		algFlag=0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	unsigned int 		i = 0;
	unsigned int		eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg = {0};
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&algFlag,
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_qos_dbg("Set qos cfg type %d on port index 0x%x", algFlag, eth_g_index);

	if( 0 == npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
	{
		if (algFlag == PORT_MODE_SP)
		{
			qosPortCfg.cosCfg.queue_type = QOS_PORT_TX_SP_ARB_GROUP_E;
			for (i=0; i<MAX_COS_QUEUE_NUM; i++) 
			{
			    qosPortCfg.cosCfg.queue[i].groupFlag = QOS_PORT_TX_SP_ARB_GROUP_E;
				qosPortCfg.cosCfg.queue[i].weight = 0;
			}
		}
		else if (algFlag == PORT_MODE_HYBRID)
		{
			if (qosPortCfg.cosCfg.queue_type != QOS_PORT_TX_SP_WRR_ARB_E) 
			{
				for (i=0; i<MAX_COS_QUEUE_NUM; i++) 
				{
					qosPortCfg.cosCfg.queue[i].groupFlag = QOS_PORT_TX_WRR_ARB_GROUP0_E;
					qosPortCfg.cosCfg.queue[i].weight = i+1;
				}
				qosPortCfg.cosCfg.queue_type = QOS_PORT_TX_SP_WRR_ARB_E;
			}
		}	
		else if(algFlag == PORT_MODE_WRR)
		{
			if (qosPortCfg.cosCfg.queue_type != QOS_PORT_TX_WRR_ARB_E) 
			{
				for(i=0 ;i<MAX_COS_QUEUE_NUM; i++) 
				{
					qosPortCfg.cosCfg.queue[i].groupFlag = QOS_PORT_TX_WRR_ARB_GROUP0_E;
					qosPortCfg.cosCfg.queue[i].weight = i + 1;
				}
				qosPortCfg.cosCfg.queue_type = QOS_PORT_TX_WRR_ARB_E;
			}
		}

		npd_qos_netif_cfg_set_by_index( eth_g_index, &qosPortCfg);
	}
	else 
	{
		syslog_ax_qos_dbg("can not find qos cfg on port index 0x%x, set type, %d fail", eth_g_index, algFlag);
		ret = QOS_RETURN_CODE_ERROR;
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
	
 	 return reply;
}

DBusMessage * npd_dbus_queue_drop(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		algFlag=0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	unsigned int		eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg = {0};
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&algFlag,
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_qos_dbg("Set qos cfg type %d on port index 0x%x", algFlag, eth_g_index);

	if( 0 == npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
	{
		qosPortCfg.cosCfg.queue_drop_mode = algFlag;
		npd_qos_netif_cfg_set_by_index( eth_g_index, &qosPortCfg);
	}
	else 
	{
		syslog_ax_qos_dbg("can not find qos cfg on port index 0x%x, set type, %d fail", eth_g_index, algFlag);
		ret = QOS_RETURN_CODE_ERROR;
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
	
 	 return reply;
}

DBusMessage * npd_dbus_queue_sche_wrr_group(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		tc =0;
	unsigned int		weight = 0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	unsigned int		eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg;
	
	dbus_error_init(&err);
    
	if (!(dbus_message_get_args ( msg, &err, 
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_UINT32,&tc,
	   DBUS_TYPE_UINT32,&weight,
	   DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	

	syslog_ax_qos_dbg("queue sche update tc %d weight %d on port index 0x%x", tc, weight, eth_g_index);
	
	if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
	{
		if( qosPortCfg.cosCfg.queue_type == QOS_PORT_TX_DEFAULT_WRR_ARB_E)
		{
			ret = QOS_RETURN_CODE_MODE_CONFLICT;
		}
		else if( qosPortCfg.cosCfg.queue_type == QOS_PORT_TX_SP_ARB_GROUP_E )
		{
			ret = QOS_RETURN_CODE_MODE_CONFLICT;
		}
		else if( qosPortCfg.cosCfg.queue_type == QOS_PORT_TX_WRR_ARB_E &&
			     weight == QOS_SCH_GROUP_IS_SP)
		{
			ret = QOS_RETURN_CODE_MODE_CONFLICT;
		}
		else if ( weight == QOS_SCH_GROUP_IS_SP ) 
		{
			qosPortCfg.cosCfg.queue[tc].groupFlag = QOS_PORT_TX_SP_ARB_GROUP_E;
			qosPortCfg.cosCfg.queue[tc].weight = 0;
		}
		else 
		{
			qosPortCfg.cosCfg.queue[tc].groupFlag = QOS_PORT_TX_WRR_ARB_GROUP0_E;
			qosPortCfg.cosCfg.queue[tc].weight = weight;
		}
		npd_qos_netif_cfg_set_by_index(eth_g_index, &qosPortCfg);
	}
	else 
	{
		ret = QOS_RETURN_CODE_ERROR;
	}
	
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append (reply, &iter);   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);

   return reply;
}

DBusMessage * npd_dbus_queue_def_sche(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int		algFlag=0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	unsigned int 		i = 0, eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg = {0};

	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&algFlag,
	   DBUS_TYPE_UINT32,&eth_g_index,
	   DBUS_TYPE_INVALID))) 
	{
	 	npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if( 0 == npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
	{
		qosPortCfg.cosCfg.queue_type = QOS_PORT_TX_SP_ARB_GROUP_E;			
		for (i = 0; i < MAX_COS_QUEUE_NUM; i++)
		{
			qosPortCfg.cosCfg.queue[i].groupFlag = QOS_PORT_TX_SP_ARB_GROUP_E;
			qosPortCfg.cosCfg.queue[i].weight = 0;
		}
		npd_qos_netif_cfg_set_by_index( eth_g_index, &qosPortCfg);
	}
	else 
	{
		ret = QOS_RETURN_CODE_ERROR;
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
	
 	 return reply;
}

DBusMessage * npd_dbus_show_queue_scheduler(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter, iter_array;
	DBusError			err;
	unsigned int		algFlag = 0;
	unsigned int		i = 0;
	unsigned int        eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg = {0};
	QOS_WRR_TX_WEIGHT_E showQueue[MAX_COS_QUEUE_NUM];
	
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
	   return NULL;
	}

	syslog_ax_qos_dbg("show queue scheduler on index 0x%x", eth_g_index);
	memset(showQueue, 0, sizeof(QOS_WRR_TX_WEIGHT_E)*MAX_COS_QUEUE_NUM);	
	
	if( 0 == npd_qos_netif_cfg_get_by_index( eth_g_index, &qosPortCfg))
	{
		syslog_ax_qos_dbg("find queue scheduler on index 0x%x", eth_g_index);
		
		if(qosPortCfg.cosCfg.queue_type==QOS_PORT_TX_DEFAULT_WRR_ARB_E) 
		{
			algFlag = PORT_MODE_DEFAULT;
		}
		else if(qosPortCfg.cosCfg.queue_type==QOS_PORT_TX_SP_ARB_GROUP_E) 
		{
			algFlag = PORT_MODE_SP;
		}
		else if(qosPortCfg.cosCfg.queue_type==QOS_PORT_TX_WRR_ARB_E) 
		{
			algFlag = PORT_MODE_WRR;
			memcpy(showQueue, qosPortCfg.cosCfg.queue, MAX_COS_QUEUE_NUM*sizeof(QOS_WRR_TX_WEIGHT_E));		
		}
		else if(qosPortCfg.cosCfg.queue_type==QOS_PORT_TX_SP_WRR_ARB_E)
		{
			algFlag = PORT_MODE_HYBRID;
			memcpy(showQueue, qosPortCfg.cosCfg.queue, MAX_COS_QUEUE_NUM*sizeof(QOS_WRR_TX_WEIGHT_E));	
		}
	}
	else
	{
		syslog_ax_qos_dbg("not find queue scheduler on index 0x%x", eth_g_index);
		algFlag = PORT_MODE_DEFAULT;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &algFlag);	

	dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING     /*begin*/
										DBUS_TYPE_UINT32_AS_STRING    /*groupflag*/
										DBUS_TYPE_UINT32_AS_STRING    /*weight*/
										DBUS_STRUCT_END_CHAR_AS_STRING,     /*end*/
										&iter_array);
	for (i = 0; i < MAX_COS_QUEUE_NUM; i++ ) 
	{
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array, DBUS_TYPE_STRUCT, NULL, &iter_struct);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(showQueue[i].groupFlag));
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(showQueue[i].weight));

		dbus_message_iter_close_container(&iter_array, &iter_struct);	
	}
	dbus_message_iter_close_container (&iter, &iter_array);

	return reply;	
   
}

DBusMessage * npd_dbus_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int	    algFlag=0,queueId=0,g_eth_index=0,burst=0, kmstate=0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	unsigned long       maxrate=0;
	struct eth_port_s	 *portInfo = NULL;
	QOS_PORT_CFG_STC  shape;
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&g_eth_index,
		DBUS_TYPE_UINT32,&algFlag,
	    DBUS_TYPE_UINT32,&queueId,
	    DBUS_TYPE_UINT32,&maxrate,
	    DBUS_TYPE_UINT32,&kmstate,
	    DBUS_TYPE_UINT32,&burst,
	    DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	npd_syslog_dbg("g_eth_index %d,algFlag %d,queueId %d,maxRate %d,burst %d\n",g_eth_index,algFlag,queueId,maxrate,burst);

	portInfo = npd_get_port_by_index(g_eth_index);
	if(NULL == portInfo) 
	{
		ret = QOS_RETURN_CODE_ERROR;
    }
	else
	{			
		if(0 != npd_qos_netif_cfg_get_by_index(g_eth_index, &shape) )
		{				
			memset( &shape,0,sizeof(QOS_PORT_CFG_STC));
		}
		if (kmstate == 0) {
			maxrate = maxrate*64;
		}
		else if (kmstate == 1) 
			maxrate = maxrate*1000;
		else if (kmstate == 3)
		{
			maxrate = maxrate;
			burst = 100;
		}
        
		if (ret == QOS_RETURN_CODE_SUCCESS) 
		{
			if(algFlag==0)
			{
				shape.ifIndex = g_eth_index;
				shape.tcCfg.portEnable = QOS_TRUE;   /*ture*/
				shape.tcCfg.burstSize=burst;
				shape.tcCfg.kmstate = kmstate;
				shape.tcCfg.Maxrate = maxrate;
				
			}/*port*/
			else if(algFlag==1)
			{
				shape.ifIndex = g_eth_index;
				shape.tcCfg.queue[queueId].queueEnable=QOS_TRUE;
				shape.tcCfg.queue[queueId].Maxrate=maxrate;
				shape.tcCfg.queue[queueId].burstSize=burst;
				shape.tcCfg.queue[queueId].kmstate = kmstate;
			}/*queue on port*/
			npd_qos_netif_cfg_set_by_index( g_eth_index, &shape);
		}
		free( portInfo );
	}
		
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append (reply, &iter);   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
   return reply;

}

DBusMessage * npd_dbus_show_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter, iter_array;
	DBusError			err;
	unsigned int		i = 0,j = 0,k = 0,ret = 0,g_eth_index=0;
	QOS_SHAPER_QUEUE    show[MAX_COS_QUEUE_NUM];
   	struct eth_port_s	 *portInfo = NULL;
	QOS_PORT_CFG_STC	shape;
	unsigned int 		burstSize=0,portEnable=0;
	unsigned long       maxRate=0;
	unsigned int		kmstate = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,  
			DBUS_TYPE_UINT32,&g_eth_index,			
			DBUS_TYPE_INVALID))) {
		   npd_syslog_err("Unable to get input args ");
		   if (dbus_error_is_set(&err)) {
			   npd_syslog_err("%s raised: %s",err.name,err.message);
			   dbus_error_free(&err);
		   }
		   return NULL;
	}
	for(k=0;k<MAX_COS_QUEUE_NUM;k++){
		memset(&show[k],0,sizeof(QOS_SHAPER_QUEUE));
	}
	syslog_ax_qos_dbg("npd dbus show port traffic shape: ethIndex 0x%x", g_eth_index);
	
	portInfo = npd_get_port_by_index(g_eth_index);	
	if(NULL == portInfo) {
		ret = QOS_RETURN_CODE_ERROR;
		npd_syslog_dbg("QOS_RETURN_CODE_ERROR\n");
	}
	else
	{		
		if( 0 != npd_qos_netif_cfg_get_by_index( g_eth_index, &shape) )
		{
			ret=QOS_RETURN_CODE_ERROR;
			npd_syslog_dbg("no infos on port!\n");
		}
		else
		{
			burstSize = shape.tcCfg.burstSize;
			maxRate   =shape.tcCfg.Maxrate;
			portEnable=shape.tcCfg.portEnable;
			kmstate = shape.tcCfg.kmstate;
			for(j=0;j<MAX_COS_QUEUE_NUM;j++)
			{
				show[j].burstSize = shape.tcCfg.queue[j].burstSize;
				show[j].Maxrate = shape.tcCfg.queue[j].Maxrate;
				show[j].queueEnable =shape.tcCfg.queue[j].queueEnable;
				show[j].kmstate=shape.tcCfg.queue[j].kmstate;
			}
				ret=QOS_RETURN_CODE_SUCCESS;
		}
		free( portInfo );
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &portEnable);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &burstSize);	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &maxRate);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &kmstate);
	
	dbus_message_iter_open_container (&iter,
								   DBUS_TYPE_ARRAY,
								    DBUS_STRUCT_BEGIN_CHAR_AS_STRING     /*begin*/
									   DBUS_TYPE_UINT32_AS_STRING     /*queueEnable*/
									   DBUS_TYPE_UINT32_AS_STRING    /*burstsize*/
									   DBUS_TYPE_UINT32_AS_STRING    /*maxrate*/
									    DBUS_TYPE_UINT32_AS_STRING    /*kmstate*/
									DBUS_STRUCT_END_CHAR_AS_STRING,     /*end*/
								    &iter_array);
 	 for (i = 0; i < MAX_COS_QUEUE_NUM; i++ ) 
	 {
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
									   &iter_struct);
		dbus_message_iter_append_basic
				  (&iter_struct,
				  DBUS_TYPE_UINT32,
				  &(show[i].queueEnable));
		dbus_message_iter_append_basic
				  (&iter_struct,
				  DBUS_TYPE_UINT32,
				  &(show[i].burstSize));
		dbus_message_iter_append_basic
				  (&iter_struct,
				  DBUS_TYPE_UINT32,
				  &(show[i].Maxrate));
		dbus_message_iter_append_basic
				  (&iter_struct,
				  DBUS_TYPE_UINT32,
				  &(show[i].kmstate));
				
		dbus_message_iter_close_container (&iter_array, &iter_struct);	
	}
	dbus_message_iter_close_container (&iter, &iter_array);

	return reply;  
}

DBusMessage * npd_dbus_delete_traffic_shape(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter;
	DBusError			err;
	unsigned int	    algFlag=0,queueId=0,g_eth_index=0;
	unsigned int		ret = QOS_RETURN_CODE_SUCCESS;
	struct eth_port_s*	portInfo = NULL;
	QOS_PORT_CFG_STC	shape;
	
	dbus_error_init(&err);
   
	if (!(dbus_message_get_args ( msg, &err,  
		DBUS_TYPE_UINT32,&g_eth_index,
		DBUS_TYPE_UINT32,&algFlag,
	    DBUS_TYPE_UINT32,&queueId,
	    DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    npd_syslog_dbg("g_eth_index %d,algFlag %d,queueId %d\n",g_eth_index,algFlag,queueId);

	portInfo = npd_get_port_by_index(g_eth_index);
	if(NULL == portInfo) 
	{
		ret = QOS_RETURN_CODE_ERROR;
    }
	else
	{
		if(0 != npd_qos_netif_cfg_get_by_index( g_eth_index, &shape))
		{
			npd_syslog_dbg("port has no traffic shape infos \n");
			ret = QOS_RETURN_CODE_ERROR;
		}
		else
		{
			if(algFlag==0)
			{
				shape.tcCfg.portEnable = QOS_FALSE;   /*ture*/
				shape.tcCfg.burstSize = 0;
				shape.tcCfg.kmstate = 0;
				shape.tcCfg.Maxrate = 0;
				npd_qos_netif_cfg_set_by_index( g_eth_index, &shape);
			}
			else if(algFlag==1)
			{
				shape.tcCfg.queue[queueId].queueEnable=0;
				shape.tcCfg.queue[queueId].Maxrate=0;
				shape.tcCfg.queue[queueId].burstSize=0;
				npd_qos_netif_cfg_set_by_index( g_eth_index, &shape);			
			}/*queue on port*/
			ret = QOS_RETURN_CODE_SUCCESS;
		}
		free( portInfo );
	}

		
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append (reply, &iter);   
   dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&ret);
   return reply;
}


char* npd_qos_counter_show_running_config(char* showStr, int* safe_len)
{
	int		ni = 0;
	int		totalLen = 0;
	char*	cursor = NULL;
	QOS_COUNTER_STC		counter;
	                   
	memset(&counter, 0, sizeof(QOS_COUNTER_STC));
	cursor = showStr;

	for (ni = 0; ni < 16; ni++)
	{
		if (*safe_len < (totalLen + 150))
		{
			break;
		}
		
		if (0 == npd_counter_get_by_index(ni, &counter))
		{
			totalLen += sprintf(cursor, "counter %d inprofile %lu outprofile %lu\n", ni, counter.inProfileBytesCnt, counter.outOfProfileBytesCnt);                       
			cursor = showStr + totalLen; 
		}
		else
		{
			continue;
		}
	}

	*safe_len = totalLen;

	return showStr;
}

char* npd_qos_profile_show_running_config(char* showStr, int* safe_len)
{	
	QOS_PROFILE_STC   qosProfile; 
	char*	cursor = NULL;
	int		ni = 0;
	int		totalLen = 0;

	cursor = showStr;
	memset(&qosProfile, 0, sizeof(QOS_PROFILE_STC));

	for (ni = 1; ni < MAX_PROFILE_NUMBER; ni++) 
	{
		if (*safe_len < (totalLen + 100))
		{
			break;
		}
		if ( 0 == npd_qosprofile_get_by_index(ni, &qosProfile))
		{
			if ((0 != qosProfile.index) 
                && (VOICE_VLAN_RSV_QOS_PROFILE_ID != qosProfile.index)
                && (((ni-1)/4 != qosProfile.userPriority)
                     || ((ni-1)/4*8+((ni-1)%4)*2 != qosProfile.dscp)
				   )
                )
			{
				totalLen += sprintf(cursor,"qos-profile %d\n", ni);   
				cursor = showStr + totalLen;   
				if (QOS_PROFILE_DONOT_CARE_E != qosProfile.userPriority)
				{
					totalLen += sprintf(cursor,"  user-priority %d\n", qosProfile.userPriority);
					cursor = showStr + totalLen;  
				}
				if (QOS_PROFILE_DONOT_CARE_E != qosProfile.dscp)
				{
				    totalLen += sprintf(cursor,"  dscp %d\n", qosProfile.dscp);
				    cursor = showStr + totalLen; 
				}
					
				totalLen += sprintf(cursor,"exit\n");
				cursor = showStr + totalLen;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}/*for*/

	*safe_len = totalLen;

	return showStr;
}

char* npd_qos_policy_map_show_running_config(char* showStr, int* safe_len)
{
	char*	cursor = NULL;
    char    portName[32];
	int		ni = 0;
	int		nj = 0;
	int		totalLen = 0;
    int     sumcount = 0;
    unsigned int    eth_g_index = 0;
	QOS_PORT_POLICY_MAP_ATTRIBUTE_STC	poMap;
    QOS_PORT_CFG_STC    qosPortCfg;
    int ret;
    struct eth_port_s eth_port = {0};
    struct vlan_s vlan = {0};
    int i = 0;

	memset(portName, 0, sizeof(portName)); 
	memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));

	memset(&poMap, 0, sizeof(QOS_PORT_POLICY_MAP_ATTRIBUTE_STC));
	cursor = showStr;

	for (ni = 0; ni < MAX_POLICY_MAP_NUM; ni++)
	{
		if (*safe_len < (totalLen + 10000))
		{
			break;
		}
		
		if (0 == npd_policymap_get_by_index(ni, &poMap))
		{
            sumcount++;
			totalLen += sprintf(cursor, "qos-policy system\n");
			cursor = showStr + totalLen;

			for (nj = 0; nj < MAX_UP_PROFILE_NUM; nj++)
			{
                {
    				if ((nj*4 + 1) != poMap.upMapQosProfile[nj].profileIndex)
    				{
    					totalLen += sprintf(cursor, " in user-priority %d qos-profile %d\n", nj, poMap.upMapQosProfile[nj].profileIndex);
    					cursor = showStr + totalLen; 
    				}
                }
			}
            /*
			for (nj = 0; nj < MAX_DSCP_PROFILE_NUM; nj++)
			{
                if(0 == nj/8)
                {
                    if ((poMap.outUpMapQosProfile[nj].profileIndex*8 + 1) != nj)
    				{
    					totalLen += sprintf(cursor, "  out user-priority %d qos-profile %d\n", poMap.upMapQosProfile[nj].profileIndex,nj);
    					cursor = showStr + totalLen; 
    				}
                }
                else
                {
    				if ((poMap.outUpMapQosProfile[nj].profileIndex*8 + 3) != nj)
    				{
    					totalLen += sprintf(cursor, "  out user-priority %d qos-profile %d\n", poMap.upMapQosProfile[nj].profileIndex, nj+1);
    					cursor = showStr + totalLen; 
    				}
                }
			}
			*/
			for (nj = 0; nj < MAX_DSCP_PROFILE_NUM; nj++)
			{
				if (((nj/8)*4+1) != poMap.dscpMapQosProfile[nj].profileIndex)
				{
					totalLen += sprintf(cursor, " in dscp %d qos-profile %d\n", nj, poMap.dscpMapQosProfile[nj].profileIndex);
					cursor = showStr + totalLen; 
				}
                /*
				if ((nj+1) != poMap.outDscpMapQosProfile[nj].profileIndex)
				{
					totalLen += sprintf(cursor, "  out dscp %d qos-profile %d\n", nj, poMap.dscpMapQosProfile[nj].profileIndex);
					cursor = showStr + totalLen; 
				}
				*/
				else
				{
					continue;
				}
			}

		
			totalLen += sprintf(cursor, "exit\n");
			cursor = showStr + totalLen;
		}
		else
		{
			continue;
		}
	}

   
    ret = dbtable_sequence_traverse_next(g_eth_ports, -1, &eth_port);
    while(0 == ret)
	{
        int exit = 0;
		if (*safe_len < (totalLen + 2000))
		{
			break;
		}
	
		eth_g_index = eth_port.eth_port_ifindex;
		ni = eth_port_array_index_from_ifindex(eth_g_index);
        for(i = 0; i < dbtable_array_totalcount(service_policy_index); i++)
        {
           int retTemp;
           struct service_policy_s policy ;
           memset(&policy, 0, sizeof(policy));
           retTemp = dbtable_array_get(service_policy_index, i, &policy);
           if(0 != retTemp)
                continue;
			
		   if (!strncmp(policy.policy_map_name, "SERV_TC", strlen("SERV_TC")))
           {
                continue;
           }
		   if (!strncmp(policy.policy_map_name, "IPV6_", strlen("IPV6_")))
           {
                continue;
           }
           if (0 == strncmp(policy.policy_map_name, "SG_", strlen("SG_")))
           {
                continue;
           }
#ifdef HAVE_PORTAL 
           if (0 == strncmp(policy.policy_map_name, "PORTAL_", strlen("PORTAL_")))
           {
                continue;
           }
#endif

           if (TRUE == npd_is_rule_in_acl_group(policy.policy_index))
           {
                continue;
           }
           if(NPD_PBMP_MEMBER(policy.group, ni))
           {
               if(exit == 0)
               {
                    memset(portName, 0, 32);
    				parse_eth_index_to_name(eth_g_index, portName);
    				totalLen += sprintf(cursor, "interface ethernet %s\n", portName);
    				cursor = showStr + totalLen;
                    exit = 1;
               }
               if(!strncmp(policy.policy_map_name, "ACL_", 4))
               {
                   if(policy.dir_type == 0)
                   {
                       totalLen += sprintf(cursor, "  ingress acl %s\n", &policy.policy_map_name[4]);
                   }
                   else
                   {
		                totalLen += sprintf(cursor, "  egress acl %s\n", &policy.policy_map_name[4]);
                   }
                   cursor = showStr+totalLen;
               }
               else if (!strcmp(policy.policy_map_name, "VOICE VLAN"))
               {
                   totalLen += sprintf(cursor, "  voice vlan\n");
                   cursor = showStr+totalLen;
               }
               else
               {
                   if(policy.dir_type == 0)
                   {
                       totalLen += sprintf(cursor, "  service-policy ingress %s\n", policy.policy_map_name);
                   }
                   else
                   {
		                totalLen += sprintf(cursor, "  service-policy egress %s\n", policy.policy_map_name);
                   }
                   cursor = showStr+totalLen;
               }
           }
        }
        for(i = 0; i < dbtable_array_totalcount(acl_group_index); i++)
        {
            int tmpRet = 0;
            struct acl_group_stc acl_group;
            memset(&acl_group, 0, sizeof(struct acl_group_stc));
            tmpRet = dbtable_array_get(acl_group_index, i, &acl_group);
            if(0 != tmpRet)
            {
                continue;
            }
            if(0 == acl_group.is_deployed)
            {
                continue;
            }
            if(NPD_PBMP_MEMBER(acl_group.portbmp, ni))
            {
               if(exit == 0)
               {
                    memset(portName, 0, 32);
    				parse_eth_index_to_name(eth_g_index, portName);
    				totalLen += sprintf(cursor, "interface ethernet %s\n", portName);
    				cursor = showStr + totalLen;
                    exit = 1;
               }
               if(acl_group.dir_type == 0)
               {
                   totalLen += sprintf(cursor, "  ingress acl-group %s\n", acl_group.name);
               }
               else
               {
	               totalLen += sprintf(cursor, "  egress acl-group %s\n", acl_group.name);
               }
               cursor = showStr+totalLen;
            }
        }
		if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
		{
            int nQueueFlag = 0;

			for (nj = 0; nj < MAX_COS_QUEUE_NUM; nj++)
			{
				if ( 1 == qosPortCfg.tcCfg.queue[nj].queueEnable)
				{
					nQueueFlag = 1;
					break;
				}
				else
				{
					continue;
				}
			}            
            
            if(
                ( 0 != qosPortCfg.poMapCfg.poMapId)
                || (0 != qosPortCfg.qosProfileIndex)
                || (0 != qosPortCfg.egressRemark)
                || (0 != qosPortCfg.ingressRemark)
                || (QOS_TRUST_PORT != qosPortCfg.trust)
                || (0 != qosPortCfg.poMapCfg.poMapId)
			    || (QOS_PORT_TX_WRR_ARB_E == qosPortCfg.cosCfg.queue_type )
				|| (QOS_PORT_TX_SP_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
				|| (DROP_MODE_TAIL != qosPortCfg.cosCfg.queue_drop_mode)
				|| (1 == qosPortCfg.tcCfg.portEnable) 
				|| (1 == nQueueFlag)
               )
            {
                if(exit == 0)
                {
                    memset(portName, 0, 32);
    				parse_eth_index_to_name(eth_g_index, portName);
    				totalLen += sprintf(cursor, "interface ethernet %s\n", portName);
    				cursor = showStr + totalLen;
                    exit = 1;
                }
            }
            if (1 == qosPortCfg.tcCfg.portEnable || 1 == nQueueFlag)
		    {
				if (1 == qosPortCfg.tcCfg.portEnable)
				{
					totalLen += sprintf(cursor, "  traffic-shape ");
					cursor = showStr + totalLen; 
					
					
					if (0 == qosPortCfg.tcCfg.kmstate)
					{
						totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.Maxrate/64);
						cursor = showStr + totalLen; 
						totalLen += sprintf(cursor, "64k ");
						cursor = showStr + totalLen; 
					    totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.burstSize);
					}
					else if(1==qosPortCfg.tcCfg.kmstate)
					{
						totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.Maxrate/1000);
						cursor = showStr + totalLen; 
						totalLen += sprintf(cursor, "1m ");
						cursor = showStr + totalLen; 
					    totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.burstSize);
					}
					else if(3 == qosPortCfg.tcCfg.kmstate)
					{
						totalLen += sprintf(cursor, "percentage %ld \n",qosPortCfg.tcCfg.Maxrate);
					}

					cursor = showStr + totalLen; 		
				}

				for (nj = 0; nj < MAX_COS_QUEUE_NUM; nj++)
				{
					if ( 1 == qosPortCfg.tcCfg.queue[nj].queueEnable)
					{
						totalLen += sprintf(cursor, "  traffic-shape queue ");
						cursor = showStr + totalLen; 


						totalLen += sprintf(cursor, "%d ", nj);
						cursor = showStr + totalLen; 


						if (0 == qosPortCfg.tcCfg.queue[nj].kmstate)
						{
							totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.queue[nj].Maxrate/64);
							cursor = showStr + totalLen; 
							totalLen += sprintf(cursor, "64k ");
							cursor = showStr + totalLen; 
						    totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.queue[nj].burstSize);
						}
						else if(1 == qosPortCfg.tcCfg.queue[nj].kmstate)
						{
							totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.queue[nj].Maxrate/1000);
							cursor = showStr + totalLen; 
							totalLen += sprintf(cursor, "1m ");
							cursor = showStr + totalLen; 
						    totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.queue[nj].burstSize);
						}
    					else if(3 == qosPortCfg.tcCfg.queue[nj].kmstate)
    					{
    						totalLen += sprintf(cursor, "percentage %ld \n",qosPortCfg.tcCfg.Maxrate);
    					}

						cursor = showStr + totalLen; 
					}
					else
					{
						continue;
					}
				}        
            }
            if ( QOS_PORT_TX_WRR_ARB_E == qosPortCfg.cosCfg.queue_type \
					|| QOS_PORT_TX_SP_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
    		{
    			if (QOS_PORT_TX_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
    			{
    				totalLen += sprintf(cursor, "  queue-scheduler wrr\n");
    				cursor = showStr + totalLen; 
    				
    				for (nj = 0; nj < 8; nj++)
    				{
    					if ((nj + 1) != qosPortCfg.cosCfg.queue[nj].weight)
    					{
    						totalLen += sprintf(cursor,"  wrr %d %u\n", nj, qosPortCfg.cosCfg.queue[nj].weight);
    						cursor = showStr + totalLen;
    					}
    					else
    					{
    						continue;
    					}
    				}
    			}
    			else if (QOS_PORT_TX_SP_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
    			{
    				totalLen += sprintf(cursor, "  queue-scheduler hybrid\n");
    				cursor = showStr + totalLen; 
    				for (nj = 0; nj < 8; nj++)
    				{
    					
    					if (0 == qosPortCfg.cosCfg.queue[nj].weight)
    					{
    						totalLen += sprintf(cursor, "  wrr %d sp\n", nj);
    						cursor = showStr + totalLen;
    					}
    					else if ((nj + 1) != qosPortCfg.cosCfg.queue[nj].weight)
    					{
    						totalLen += sprintf(cursor, "  wrr %d %u\n", nj, qosPortCfg.cosCfg.queue[nj].weight);
    						cursor = showStr + totalLen;
    					}
    					else
    					{
    						continue;
    					}
    				}
    			} 
            }
            if(DROP_MODE_TAIL != qosPortCfg.cosCfg.queue_drop_mode)
            {
                if(DROP_MODE_RED == qosPortCfg.cosCfg.queue_drop_mode)
                {
    				totalLen += sprintf(cursor, "  queue-drop-mode red\n");
    				cursor = showStr + totalLen; 
                }
                if(DROP_MODE_WRED == qosPortCfg.cosCfg.queue_drop_mode)
                {
    				totalLen += sprintf(cursor, "  queue-drop-mode wred\n");
    				cursor = showStr + totalLen; 
                }
                
            }
            if(0 != qosPortCfg.qosProfileIndex)
            {
				totalLen += sprintf(cursor, "  ingress default qos-profile %d\n", qosPortCfg.qosProfileIndex);
				cursor = showStr + totalLen; 
            }
            if(0 != qosPortCfg.ingressRemark)
            {
				totalLen += sprintf(cursor, "  ingress remark\n");
				cursor = showStr + totalLen; 
            }

            if(0 != qosPortCfg.egressRemark)
            {
				totalLen += sprintf(cursor, "  egress remark\n");
				cursor = showStr + totalLen; 
            }
            switch(qosPortCfg.trust)
            {
                case QOS_TRUST_L2:
					totalLen += sprintf(cursor, "  trust-mode user-priority\n");
					cursor = showStr + totalLen; 
                    break;
                case QOS_TRUST_L3:
					totalLen += sprintf(cursor, "  trust-mode dscp\n");
					cursor = showStr + totalLen; 
                    break;
                case QOS_TRUST_L2L3:
					totalLen += sprintf(cursor, "  trust-mode user-priority\n");
					cursor = showStr + totalLen; 
					totalLen += sprintf(cursor, "  trust-mode dscp\n");
					cursor = showStr + totalLen; 
                    break;
                default:
                    break;
            }
                
            if((-1 != qosPortCfg.poMapCfg.poMapId)
                && (0 != qosPortCfg.poMapCfg.poMapId))
            {
    		    totalLen += sprintf(cursor, "  qos-policy %d\n", qosPortCfg.poMapCfg.poMapId);
    			cursor = showStr + totalLen; 
            }

        }

        if(exit)
        {
 			totalLen += sprintf(cursor, "exit\n");
 			cursor = showStr + totalLen;
        }
             
		memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
        ret = dbtable_sequence_traverse_next(g_eth_ports, eth_g_index, &eth_port);
	}
    ret = dbtable_sequence_traverse_next(g_vlans, -1, &vlan);
    while(0 == ret)
    {
        int vid = 0;
        int j = 0;
        unsigned int if_index= 0;
        int exit_node = 0;
        vid = vlan.vid;
        for(j = 0; j < dbtable_array_totalcount(service_policy_index); j++)
	    {
            int retTemp = 0;
            struct service_policy_s policy ;
            memset(&policy, 0, sizeof(policy));
            retTemp = dbtable_array_get(service_policy_index, j, &policy);
            if(0 != retTemp)
                continue;
            if( !(strcmp(policy.policy_map_name, "VOICE VLAN"))
             || !(strncmp(policy.policy_map_name, "SERV_TC", strlen("SERV_TC")))
             || !(strncmp(policy.policy_map_name, "IPV6_", 5))			
             || !(strncmp(policy.policy_map_name, "SG_", strlen("SG_")))
#ifdef HAVE_PORTAL 
             || !(strncmp(policy.policy_map_name, "PORTAL_", strlen("PORTAL_")))
#endif
               )
    	    {
                continue;
    	    }

            if_index = npd_netif_vlan_get_index(vid);
            if(TRUE == npd_is_rule_in_acl_group(policy.policy_index))
            {
                continue;
            }
            
            if(NPD_VBMP_MEMBER(policy.vlanbmp, vid))
            {
                if(exit_node == 0)
                {
                    totalLen += sprintf(cursor, "vlan %d\n", vid);
                    cursor = showStr + totalLen;
                    exit_node = 1;
                }
                if(!strncmp(policy.policy_map_name, "ACL_", 4))
                {
                    if(policy.dir_type == 0)
                    {
                        totalLen += sprintf(cursor, "  ingress acl %s\n", &policy.policy_map_name[4]);
                    }
                    else
                    {
		                totalLen += sprintf(cursor, "  egress acl %s\n", &policy.policy_map_name[4]);
                    }
                    cursor = showStr+totalLen;
                }
                else
                {
                    if(policy.dir_type == 0)
                    {
                       totalLen += sprintf(cursor, "  service-policy ingress %s\n", policy.policy_map_name);
                    }
                    else
                    {
                        totalLen += sprintf(cursor, "  service-policy egress %s\n", policy.policy_map_name);
                    }
                    cursor = showStr+totalLen;
                }
            }
        }
        for(i = 0; i < dbtable_array_totalcount(acl_group_index); i++)
        {
            int tmpRet = 0;
            struct acl_group_stc acl_group;
            memset(&acl_group, 0, sizeof(struct acl_group_stc));
            tmpRet = dbtable_array_get(acl_group_index, i, &acl_group);
            if(0 != tmpRet)
            {
                continue;
            }
            if(0 == acl_group.is_deployed)
            {
                continue;
            }
            if(NPD_VBMP_MEMBER(acl_group.vlanbmp, vid))
            {
               if(exit_node == 0)
               {
                    totalLen += sprintf(cursor, "vlan %d\n", vid);
                    cursor = showStr + totalLen;
                    exit_node = 1;
               }
               if(acl_group.dir_type == 0)
               {
                   totalLen += sprintf(cursor, "  ingress acl-group %s\n", acl_group.name);
               }
               else
               {
	               totalLen += sprintf(cursor, "  egress acl-group %s\n", acl_group.name);
               }
               cursor = showStr+totalLen;
            }
        }
        if(exit_node)
        {
 			totalLen += sprintf(cursor, "exit\n");
 			cursor = showStr + totalLen;
        }
        ret = dbtable_sequence_traverse_next(g_vlans, vid, &vlan);
    }

	*safe_len = totalLen;

	return showStr;
}

char* npd_qos_policer_show_running_config(char* showStr, int* safe_len)
{
	int		totalLen = 0;
	int		ni = 0;
	char*	cursor = NULL;
	QOS_POLICER_STC 	policer;

	memset(&policer, 0, sizeof(QOS_POLICER_STC));
	cursor = showStr;


	for (ni = 0; ni < 256; ni++)
	{
		if (*safe_len < (totalLen + 200))
		{
			break;
		}
		
		if (0 == npd_policer_get_by_index(ni, &policer))
		{
			totalLen += sprintf(cursor, "policer %d\n", ni);
			cursor = showStr + totalLen; 

			if (0 != policer.cir && 0 != policer.cbs && 0 != policer.pir && 0 != policer.pbs) 
			{
				totalLen += sprintf(cursor, "  policer cir %u pir %u cbs %u pbs %u\n", 
                    policer.cir, policer.pir, policer.cbs, policer.pbs);
				cursor = showStr + totalLen; 
			}
            else if( 0 != policer.cir && 0 != policer.cbs && 0 != policer.pbs)
            {
 				totalLen += sprintf(cursor, "  policer cir %u cbs %u ebs %u\n", policer.cir, policer.cbs, policer.pbs);
				cursor = showStr + totalLen; 
               
            }

            if((policer.gPktParam.cmd == OUT_PROFILE_REMAP_ENTRY) && (policer.gPktParam.qosProfileID != 0))
            {
                totalLen += sprintf(cursor, "  conform-action remap qos-profile %d\n", policer.gPktParam.qosProfileID);
                cursor = showStr + totalLen;
            }
            
            if((policer.yPktParam.cmd == OUT_PROFILE_REMAP_ENTRY) && (policer.yPktParam.qosProfileID != 0))
            {
                totalLen += sprintf(cursor, "  exceed-action remap qos-profile %d\n", policer.yPktParam.qosProfileID);
                cursor = showStr + totalLen;
            }
            if(policer.yPktParam.cmd == OUT_PROFILE_DROP)
            {
                totalLen += sprintf(cursor, "  exceed-action drop\n");
                cursor = showStr + totalLen;
            }
            if((policer.yPktParam.cmd == OUT_PROFILE_KEEP_E) && (policer.yPktParam.transmite_flag == 1))
            {
                totalLen += sprintf(cursor, "  exceed-action transmit\n");
                cursor = showStr + totalLen;
            }
            if((policer.rPktParam.cmd == OUT_PROFILE_REMAP_ENTRY) && (policer.rPktParam.qosProfileID != 0))
            {
                totalLen += sprintf(cursor, "  violate-action remap qos-profile %d\n", policer.rPktParam.qosProfileID);
                cursor = showStr + totalLen;
            }
            if(policer.rPktParam.cmd == OUT_PROFILE_DROP)
            {
                totalLen += sprintf(cursor, "  violate-action drop\n");
                cursor = showStr + totalLen;
            }
            if((policer.rPktParam.cmd == OUT_PROFILE_KEEP_E) && (policer.rPktParam.transmite_flag == 1))
            {
                totalLen += sprintf(cursor, "  violate-action transmit\n");
                cursor = showStr + totalLen;
            }
            
			if (1 == policer.counterEnable&& 0 != policer.counterSetIndex)
			{
				totalLen += sprintf(cursor, "  counter %d\n", policer.counterSetIndex);
				cursor = showStr + totalLen; 
			}

			totalLen += sprintf(cursor, "exit\n");
			cursor = showStr + totalLen; 
		}
		else
		{
			continue;
		}
	}
	
	/* we must have a think! */
#if 0
	npd_qos_global_param_get( &qosGlobalParm );

	/*global setting*/
	if(POLICER_TB_STRICT_E==qosGlobalParm.meter){
		if(POLICER_PACKET_SIZE_L1_INCLUDE_E!=qosGlobalParm.policing)
		{
			totalLen += sprintf(cursor,"policer strict packetsize ");                       
			cursor = showStr + totalLen; 

			if(POLICER_PACKET_SIZE_L3_ONLY_E==qosGlobalParm.policing)
			{
				totalLen += sprintf(cursor,"l3\n");                       
				cursor = showStr + totalLen; 
			}
			if(POLICER_PACKET_SIZE_L2_INCLUDE_E==qosGlobalParm.policing)
			{
				totalLen += sprintf(cursor,"l2\n");                       
				cursor = showStr + totalLen; 
			}
		}
	}
	else if(POLICER_TB_LOOSE_E==qosGlobalParm.meter){
		
		totalLen += sprintf(cursor,"policer loose mru ");
		cursor = showStr + totalLen; 
		
		if(POLICER_MRU_1536_E==qosGlobalParm.mru)
		{
			totalLen += sprintf(cursor,"0\n");
			cursor = showStr + totalLen;
		}
		else if(POLICER_MRU_2K_E==qosGlobalParm.mru)
		{
			totalLen += sprintf(cursor,"1\n");						
			cursor = showStr + totalLen;	
		}
		else if(POLICER_MRU_10K_E==qosGlobalParm.mru)
		{
			totalLen += sprintf(cursor,"2\n");						
			cursor = showStr + totalLen;	
		}
	}
#endif

	*safe_len = totalLen;

	return showStr;
}

char* npd_qos_cpu_flow_contrl_show_running_config(char* showStr, int* safe_len)
{
	int totalLen = 0;
	int ret;
	cpu_flow_control_t cfctl;
	char entry[128] = {'\0'};
		
	memset(&cfctl, 0, sizeof(cpu_flow_control_t));
	strcpy(cfctl.protocol, "total");
	ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
	if(cfctl.bandwith != 4000) {
				totalLen += sprintf(entry, "cpu-flow-control total bandwith %u\n", cfctl.bandwith);
				strcat(showStr, entry);
	}

	memset(&cfctl, 0, sizeof(cpu_flow_control_t));
	ret = dbtable_hash_head(cpu_flow_control_cfg_index, NULL, &cfctl, NULL);
	while(0 == ret) {
		if (*safe_len < (totalLen + 200))
			break;
	
		bzero(entry, sizeof(entry));

		if(!strncmp(cfctl.protocol , "arp", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 320 || cfctl.priority != 1) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "pim", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 640 || cfctl.priority != 2) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "rip", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 320 || cfctl.priority != 2) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "dhcp", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 320 || cfctl.priority != 1) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "ip-to-me", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 1024 || cfctl.priority != 0) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "bpdu", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 192 || cfctl.priority != 3) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "lldp", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 64 || cfctl.priority != 1) {
					totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
					strcat(showStr, entry);
				}
			}
		if(!strncmp(cfctl.protocol , "igmp", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 256 || cfctl.priority != 1) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "ospf", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 3000 || cfctl.priority != 2) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		if(!strncmp(cfctl.protocol , "bgp", strlen(cfctl.protocol))) {
			if(cfctl.bandwith != 64 || cfctl.priority != 2) {
				totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
				strcat(showStr, entry);
			}
		}
		
		ret = dbtable_hash_next(cpu_flow_control_cfg_index, &cfctl, &cfctl, NULL);	
	}

	memset(&cfctl, 0, sizeof(cpu_flow_control_t));
	strcpy(cfctl.protocol, "unknow");
	ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
	if(cfctl.bandwith != 100 || cfctl.priority!= 0) {
		totalLen += sprintf(entry, "cpu-flow-control protocol %s bandwith %u priority %u\n", \
									cfctl.protocol, cfctl.bandwith, cfctl.priority);
		strcat(showStr, entry);
	}
	
	*safe_len = totalLen;
	return showStr;
}

#if 0
char* npd_qos_traffic_shaping_show_running_config(char* showStr, int* safe_len)
{
	unsigned char		portName[32];
	char*	cursor = NULL;
	int 	ni = 0;
	int		nj = 0;
	int		nQueueFlag = 0;
	int		totalLen = 0;
	unsigned int		eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg;

	memset(portName, 0, 32); 
	memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
	cursor = showStr;

	for(ni = 0; ni < MAX_ETH_GLOBAL_INDEX; ni++) 
	{
		if (*safe_len < (totalLen + 1000))
		{
			break;
		}
	
		eth_g_index = eth_port_array_index_to_ifindex(ni);

		if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(eth_g_index))
		{
			if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
			{
				nQueueFlag = 0;
				for (nj = 0; nj < MAX_COS_QUEUE_NUM; nj++)
				{
					if ( 1 == qosPortCfg.tcCfg.queue[nj].queueEnable)
					{
						nQueueFlag = 1;
						break;
					}
					else
					{
						continue;
					}
				}
				
				if (1 == qosPortCfg.tcCfg.portEnable || 1 == nQueueFlag)
				{
					parse_eth_index_to_name(eth_g_index, portName);
					totalLen += sprintf(cursor, "interface ethernet %s\n", portName);
					cursor = showStr + totalLen;
					memset(portName, 0, 32); 
					
					if (1 == qosPortCfg.tcCfg.portEnable)
					{
						totalLen += sprintf(cursor, "traffic-shape ");
						cursor = showStr + totalLen; 
						
						
						if (0 == qosPortCfg.tcCfg.kmstate)
						{
    						totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.Maxrate/64);
    						cursor = showStr + totalLen; 
							totalLen += sprintf(cursor, "64k ");
							cursor = showStr + totalLen; 
						}
						else
						{
    						totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.Maxrate/1000);
    						cursor = showStr + totalLen; 
							totalLen += sprintf(cursor, "1m ");
							cursor = showStr + totalLen; 
						}

						totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.burstSize);
						cursor = showStr + totalLen; 		
					}

				for (nj = 0; nj < MAX_COS_QUEUE_NUM; nj++)
				{
					if ( 1 == qosPortCfg.tcCfg.queue[nj].queueEnable)
					{
						totalLen += sprintf(cursor, "traffic-shape queue ");
						cursor = showStr + totalLen; 


							totalLen += sprintf(cursor, "%d ", nj);
							cursor = showStr + totalLen; 


							if (0 == qosPortCfg.tcCfg.queue[nj].kmstate)
							{
    							totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.queue[nj].Maxrate/64);
    							cursor = showStr + totalLen; 
								totalLen += sprintf(cursor, "64k ");
								cursor = showStr + totalLen; 
							}
							else
							{
    							totalLen += sprintf(cursor, "%ld ", qosPortCfg.tcCfg.queue[nj].Maxrate/1000);
    							cursor = showStr + totalLen; 
								totalLen += sprintf(cursor, "1m ");
								cursor = showStr + totalLen; 
							}

							totalLen += sprintf(cursor, "%d \n", qosPortCfg.tcCfg.queue[nj].burstSize);
							cursor = showStr + totalLen; 
						}
						else
						{
							continue;
						}
					}
					
					totalLen += sprintf(cursor, "exit\n");
					cursor = showStr + totalLen;
				}
			}
			else 
			{
				npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
			}

			memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
		}
		else
		{
			continue;
		}
	}

	*safe_len = totalLen;

	return showStr;
}

char* npd_qos_queue_sch_show_running_config(char* showStr, int* safe_len)
{
	unsigned char		portName[32];
	char*	cursor = NULL;
	int 	ni = 0;
	int		nj = 0;
	int		totalLen = 0;
	unsigned int		eth_g_index = 0;
	QOS_PORT_CFG_STC    qosPortCfg = {0};

	memset(portName, 0, 32); 
	memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
	cursor = showStr;

	for(ni = 0; ni < MAX_ETH_GLOBAL_INDEX; ni++) 
	{
		if (*safe_len < (totalLen + 1000))
		{
			break;
		}
	
		eth_g_index = eth_port_array_index_to_ifindex(ni);

		if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(eth_g_index))
		{
			if( 0 == npd_qos_netif_cfg_get_by_index(eth_g_index, &qosPortCfg))
			{
				if (QOS_PORT_TX_SP_ARB_GROUP_E == qosPortCfg.cosCfg.queue_type \
					|| QOS_PORT_TX_WRR_ARB_E == qosPortCfg.cosCfg.queue_type \
					|| QOS_PORT_TX_SP_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
				{
					parse_eth_index_to_name(eth_g_index, portName);
					totalLen += sprintf(cursor, "interface ethernet %s\n", portName);
					memset(portName, 0, 32);
					cursor = showStr + totalLen;
					
					if (QOS_PORT_TX_SP_ARB_GROUP_E == qosPortCfg.cosCfg.queue_type)
					{
						totalLen += sprintf(cursor, "queue-scheduler sp\n");
						cursor = showStr + totalLen; 
					}
					else if (QOS_PORT_TX_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
					{
						totalLen += sprintf(cursor, "queue-scheduler wrr\n");
						cursor = showStr + totalLen; 
						
						for (nj = 0; nj < 8; nj++)
						{
							if ((nj + 1) != qosPortCfg.cosCfg.queue[nj].weight)
							{
								totalLen += sprintf(cursor,"wrr %d %u\n", nj, qosPortCfg.cosCfg.queue[nj].weight);
								cursor = showStr + totalLen;
							}
							else
							{
								continue;
							}
						}
					}
					else if (QOS_PORT_TX_SP_WRR_ARB_E == qosPortCfg.cosCfg.queue_type)
					{
						totalLen += sprintf(cursor, "queue-scheduler hybrid\n");
						cursor = showStr + totalLen; 
						for (nj = 0; nj < 8; nj++)
						{
							
							if (0 == qosPortCfg.cosCfg.queue[nj].weight)
							{
								totalLen += sprintf(cursor, "wrr %d sp\n", nj);
								cursor = showStr + totalLen;
							}
							else if ((nj + 1) != qosPortCfg.cosCfg.queue[nj].weight)
							{
								totalLen += sprintf(cursor, "wrr %d %u\n", nj, qosPortCfg.cosCfg.queue[nj].weight);
								cursor = showStr + totalLen;
							}
							else
							{
								continue;
							}
						}
					}
					
					totalLen += sprintf(cursor, "exit\n");
					cursor = showStr + totalLen;
				}
			}
			else 
			{
				npd_syslog_dbg("% Can not get port index %d qos cfg", eth_g_index);
			}

			memset(&qosPortCfg, 0, sizeof(QOS_PORT_CFG_STC));
		}
	}

	*safe_len = totalLen;

	return showStr;
}
#endif
DBusMessage* npd_dbus_qos_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	int		total_len = 0;
	int 	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE;
	char*	showStr = NULL;
	char*	cursor = NULL;

	DBusMessage*	reply;
	DBusMessageIter	iter;

	showStr = (char*)malloc(NPD_ACL_RULE_SHOWRUN_CFG_SIZE);
	if(NULL == showStr) 
	{
		syslog_ax_acl_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_ACL_RULE_SHOWRUN_CFG_SIZE);


	cursor = showStr;
	cursor = npd_qos_cpu_flow_contrl_show_running_config(cursor, &safe_len);
	
	total_len += safe_len;
	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
	cursor = showStr + total_len;
	cursor = npd_qos_profile_show_running_config(cursor, &safe_len);

	total_len += safe_len;
	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
	cursor = showStr + total_len;
	cursor = npd_qos_policer_show_running_config(cursor, &safe_len);

	total_len += safe_len;
	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
	cursor = showStr + total_len;
	cursor = npd_qos_counter_show_running_config(cursor, &safe_len);
    {

        total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
		cursor = showStr + total_len;
		cursor = abs_time_range_info_show_running(cursor, &safe_len);
        
        total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
    	cursor = showStr + total_len;
        cursor = periodic_time_range_info_show_running(cursor, &safe_len);

    }

	{
        total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
		cursor = showStr + total_len;
		cursor = acl_rule_show_running_config(cursor, &safe_len);

        total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
    	cursor = showStr + total_len;
        cursor = voice_vlan_showrunning(cursor, &safe_len);
	}

	{
		total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
    	cursor = showStr + total_len;
    	cursor = acl_match_show_running_config(cursor, &safe_len);
    	
		total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
		cursor = showStr + total_len;
		cursor = acl_action_show_running_config(cursor, &safe_len);
	}
    {
        total_len += safe_len;
		safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;       
    	cursor = showStr + total_len;   
        cursor = time_range_info_associate_show_running(cursor, &safe_len);
    }
    {
    	total_len += safe_len;
    	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
    	cursor = showStr + total_len;
    	cursor = npd_acl_group_show_running_config(cursor, &safe_len);
    }
	total_len += safe_len;
	safe_len = NPD_ACL_RULE_SHOWRUN_CFG_SIZE - total_len;
	cursor = showStr + total_len;
	cursor = npd_qos_policy_map_show_running_config(cursor, &safe_len);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}


DBusMessage * npd_dbus_flow_control_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int priority = -1; 
	unsigned int bandwith = -1; 
	char *protocol = NULL;
	int ret = 0;
	cpu_flow_control_t cfctl;
		
	memset(&cfctl, 0, sizeof(struct cpu_flow_control_s));
	dbus_error_init(&error);
	
	if (!(dbus_message_get_args(msg, &error, \
								DBUS_TYPE_STRING, &protocol, \
								DBUS_TYPE_UINT32, &bandwith, \
								DBUS_TYPE_INT32, &priority, \
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error)) {
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		op_ret = -1;
		goto retcode;
	}
	
   	strcpy(cfctl.protocol, protocol);
	ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
	
	if(ret != 0) { 
		cfctl.bandwith = bandwith;
		cfctl.priority = priority;
		
		ret = dbtable_hash_insert(cpu_flow_control_cfg_index, &cfctl);
		if(ret != 0) {
			op_ret = ret;
        	goto retcode;
		}
    }
	else {
		cfctl.bandwith = bandwith;
		cfctl.priority = priority;
		ret = dbtable_hash_update(cpu_flow_control_cfg_index, &cfctl, &cfctl);
		if(ret != 0) {  
			op_ret = ret;
        	goto retcode;
    	}
	}

retcode:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, \
									DBUS_TYPE_UINT32, \
									&op_ret);
	return reply;
}

DBusMessage* npd_dbus_show_flow_control(DBusConnection *conn, DBusMessage *msg, void *user_data) 
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int priority = -1; 
	int bandwith = -1; 
	char *protocol = NULL;
	int ret = -1;
	int num = -1;
	cpu_flow_control_t cfctl;

	dbus_error_init(&error);
	if(!(dbus_message_get_args(msg, &error,
								DBUS_TYPE_STRING, &protocol, \
								DBUS_TYPE_INT32, &num, \
								DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error)) {
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
		ret = SFLOW_RETURN_CODE_ERR_GENERAL;
		op_ret = -1;
		goto retcode;
	}
	
	if(!strcmp(protocol, "total") && num == 0) 
	{ //total configuration
		strcpy(cfctl.protocol, protocol);
		ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
		if(ret == 0) {
			bandwith = cfctl.bandwith;
			priority = cfctl.priority;
		}
		else {
			bandwith = 0; 
			priority = 0;
		}
		op_ret = 0;
		goto retcode;
	}
	else if(!strcmp(protocol, "unknow") && num == 64) 
	{ //unknow configuration
		strcpy(cfctl.protocol, protocol);
		ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
		if(ret == 0) {
			bandwith = cfctl.bandwith;
			priority = cfctl.priority;
		}
		else {
			op_ret = -1;
			goto retcode;
		}
		op_ret = 0;
		goto retcode;
	}
	else if(protocol[0] == '\0') {//the first protocol configuration
		ret = dbtable_hash_head(cpu_flow_control_cfg_index, NULL, &cfctl, NULL);
		if(ret == 0) {
			if(!strcmp(cfctl.protocol, "total")){
				ret = dbtable_hash_next(cpu_flow_control_cfg_index, &cfctl, &cfctl, NULL);
				if(ret != 0) {
					op_ret = -1; 
					goto retcode;
				}
			}	
			bzero(protocol, strlen(protocol) + 1);
			strcpy(protocol, cfctl.protocol);
			bandwith = cfctl.bandwith;
			priority = cfctl.priority;
			op_ret = 0;
			goto retcode;
		}
		else {
			op_ret = -1; 
			goto retcode;
		}
	}
	else{ //the other's protocol configuration
		strcpy(cfctl.protocol, protocol);
		ret = dbtable_hash_search(cpu_flow_control_cfg_index, &cfctl, cpu_flow_control_cfg_hash_cmp, &cfctl);
		if(ret == 0) {
			ret = dbtable_hash_next(cpu_flow_control_cfg_index, &cfctl, &cfctl, NULL);
			if(ret == 0) {
				bzero(protocol, strlen(protocol) + 1);
				strcpy(protocol, cfctl.protocol);
				bandwith = cfctl.bandwith;
				priority = cfctl.priority;
				op_ret = 0; 
			}
			else {
				op_ret = -1; 
				goto retcode;
			}
		}
		else {
			op_ret = -1; 
			goto retcode;
		}	
	}

retcode:	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &protocol);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &bandwith);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &priority);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_INT32,  &op_ret);

	return reply;
}

#ifdef __cplusplus
}
#endif
#endif

