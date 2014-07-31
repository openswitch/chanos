
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
*   	APIs used in NPD for capwap process.
*
* DATE:
*  		03/01/2012	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.40 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"
#include "npd_capwap.h"

#define	CAPWAP_TO_NPD_MSG_SOCK		"/var/run/wcpss/capwap_to_npd_ctl"
#define NPD_TO_CAPWAP_MSG_SOCK		"/var/run/wcpss/npd_to_capwap_ctl"

#define	CAPWAP_MNG_MAX_SIZE				sizeof(struct npd_mng_capwap)
#define	CAPWAP_MNG_MAX_LEN				(1024)

#define CAPWAP_DEFAULT_DATA_PORT_NUM	(5247)

int		npd_capwap_fd = 0;
int		npd_to_wsm_fd = 0;
int		wsm_to_npd_fd = 0;
static pid_t npd_capwap_pid = -1;

/*local variables definition end */
struct	sockaddr_un 	npd_to_capwap_table_addr;   /*local addr*/	
struct	sockaddr_un		capwap_to_npd_table_addr;   /*remote addr*/
struct	sockaddr_un 	npd_to_wsm_table_addr;   /*local addr*/	
struct	sockaddr_un		wsm_to_npd_table_addr;   /*remote addr*/

db_table_t         		*npd_capwap_global_conf_dbtbl	= NULL;
db_table_t         		*npd_capwap_L3_conf_dbtbl		= NULL;

db_table_t         		*npd_capwap_tunnel_dbtbl		= NULL;
db_table_t         		*npd_capwap_bssid_dbtbl			= NULL;
db_table_t         		*npd_capwap_ts_qos_dbtbl		= NULL;
db_table_t         		*npd_capwap_tt_qos_dbtbl		= NULL;

array_table_index_t		*npd_capwap_global_conf_arr_index = NULL;
array_table_index_t		*npd_capwap_L3_conf_arr_index	= NULL;

array_table_index_t 	*npd_capwap_tunnel_array_index	=NULL;
array_table_index_t 	*npd_capwap_bssid_array_index	=NULL;

hash_table_index_t 		*npd_capwap_tunnel_hash_index	= NULL;
//hash_table_index_t 		*npd_capwap_bssid_hash_index	= NULL;
hash_table_index_t 		*npd_capwap_bss_netif_hash_index = NULL;
hash_table_index_t 		*npd_capwap_ts_qos_hash_index	= NULL;
hash_table_index_t 		*npd_capwap_tt_qos_hash_index	= NULL;

void npd_capwap_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt
    );
void npd_capwap_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event
    );
netif_event_notifier_t npd_capwap_netif_notifier =
{
    .netif_event_handle_f = &npd_capwap_notify_event,
    .netif_relate_handle_f = &npd_capwap_relate_event
};


void npd_capwap_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt
    )
{
	WIRED_TableMsg wVLAN;
	int type;


	syslog_ax_asd_dbg("npd notify asd index event: index 0x%x event %d\n", netif_index, evt);
	
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return;
	}
	
	memset(&wVLAN, 0, sizeof(wVLAN));
	type = npd_netif_type_get(netif_index);

	if(NPD_NETIF_VLAN_TYPE == type){
		wVLAN.wType = VLAN_TYPE;
		wVLAN.u.wVLAN.VLANID = npd_netif_vlan_get_vid(netif_index);
	}
	else{
		return;
	}

    switch(evt)
    {
	    case PORT_NOTIFIER_DELETE:
			wVLAN.wOp= WID_DEL;
	        break;     
	    default:
	        return;
    }		

	if(sendto(npd_capwap_fd, &wVLAN, sizeof(wVLAN), 0, (struct sockaddr *) &npd_to_capwap_table_addr, sizeof(npd_to_capwap_table_addr)) < 0){
		syslog_ax_asd_dbg("Fail to send message to asd!\n");
	}	
    return;
}


void npd_capwap_relate_event(
    unsigned int father_index,
    unsigned int son_ifindex,
    enum PORT_RELATE_ENT event
    )
{
    return;
}


int npd_capwap_tunnel_keyidx_generate( struct capwap_db_tunnel_entry_s *tunnel_item )
{
	unsigned int key = 0;//i = 0;

	if(NULL == tunnel_item) {
		syslog_ax_asd_err("npd capwap tunnel items make key null pointers error.");
		return ~0UI;
	}

	key = tunnel_item->tunnel_entry.remote_ip4;

	key %= (NPD_CAPWAP_TUNNEL_HASH_TABLE_SIZE);
	
	return key;	
}


int npd_capwap_bssid_keyidx_generate( struct npd_bssid_item_s *bssid_item )
{
	unsigned int key = 0;

	if(NULL == bssid_item) {
		syslog_ax_asd_err("npd capwap bssid items make key null pointers error.");
		return ~0UI;
	}

	key = bssid_item->BSS_ID.bssid[0]^bssid_item->BSS_ID.bssid[1]^bssid_item->BSS_ID.bssid[2]\
		 ^bssid_item->BSS_ID.bssid[3]^bssid_item->BSS_ID.bssid[4]^bssid_item->BSS_ID.bssid[5];

	key %= NPD_CAPWAP_BSSID_HASH_TABLE_SIZE;
	
	return key;	
}

int npd_capwap_bssid_netif_keyidx_generate( struct npd_bssid_item_s *bssid_item )
{
	unsigned int key = 0;

	if(NULL == bssid_item) {
		syslog_ax_asd_err("npd capwap bssid items make key null pointers error.");
		return ~0UI;
	}

	key = (bssid_item->netif_index >> 17)&(0x7ff);

	key %= NPD_CAPWAP_BSSID_HASH_TABLE_SIZE;
	
	return key;	
}

/**********************************************************************************
 * npd_capwap_tunnel_compare
 *
 * compare two of igmp snooping database(Hash table) items
 *
 *	INPUT:
 *		itemA	- igmp snooping database item
 *		itemB	- igmp snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_capwap_tunnel_compare
(
	struct capwap_db_tunnel_entry_s *itemA,
	struct capwap_db_tunnel_entry_s *itemB
)
{
	unsigned int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd capwap tunnel items compare null pointers error.");
		return FALSE;
	}

	if( itemA->tunnel_entry.remote_ip4!= itemB->tunnel_entry.remote_ip4)
	{
		equal = FALSE;
	}
	
	return equal;

} 

/**********************************************************************************
 * npd_igmp_snooping_compare
 *
 * compare two of igmp snooping database(Hash table) items
 *
 *	INPUT:
 *		itemA	- igmp snooping database item
 *		itemB	- igmp snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
int npd_capwap_bssid_compare
(
	struct npd_bssid_item_s *itemA,
	struct npd_bssid_item_s *itemB
)
{
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd capwap bssid items compare null pointers error.");
		return FALSE;
	}

	if( memcmp(itemA->BSS_ID.bssid, itemB->BSS_ID.bssid, MAC_LEN) != 0)
	{
		equal = FALSE;
	}
	
	return equal;

} 

/**********************************************************************************
 * npd_capwap_bss_netif_compare
 *
 * compare two of bss database(Hash table) items
 *
 *	INPUT:
 *		itemA	- bss database item
 *		itemB	- bss database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_capwap_bss_netif_compare
(
	struct npd_bssid_item_s *itemA,
	struct npd_bssid_item_s *itemB
)
{
	unsigned int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_asd_err("npd capwap bssid items compare null pointers error.");
		return FALSE;
	}

	if( itemA->netif_index != itemB->netif_index)
	{
		equal = FALSE;
	}
	
	return equal;

} 




 long npd_capwap_tunnel_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;	
	 int flag = 0;
	 struct capwap_db_tunnel_entry_s *origItem = NULL, *updateItem = NULL;
	 

	 if( (newItem == NULL ) || ( oldItem == NULL ) )
	 	return CAPWAP_RETURN_CODE_ERROR;

	 origItem = (struct capwap_db_tunnel_entry_s *)oldItem;
	 updateItem = (struct capwap_db_tunnel_entry_s *)newItem;

	 return ret;
 }

 long npd_capwap_tunnel_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;
	 struct capwap_db_tunnel_entry_s *opItem = NULL;

	 if( newItem == NULL ){
	 	syslog_ax_asd_dbg("The new item to be insert is NULL\r\n");
	 	return CAPWAP_RETURN_CODE_ERROR;
	 }

	 opItem = (struct capwap_db_tunnel_entry_s *)newItem;
		
	 return ret;
 }

  long npd_capwap_tunnel_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;
	struct capwap_db_tunnel_entry_s *opItem = NULL; 

	if( delItem == NULL ){
		syslog_ax_asd_dbg("The to be deleted item is NULL\r\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	opItem = (struct capwap_db_tunnel_entry_s *)delItem;
	return ret;
 }

 long npd_capwap_bssid_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;	 
	 int flag = 0;
	 struct npd_bssid_item_s *origItem = NULL, *updateItem = NULL;
	 

	 if( (newItem == NULL ) || ( oldItem == NULL ) )
	 	return CAPWAP_RETURN_CODE_ERROR;

	 origItem = (struct npd_bssid_item_s *)oldItem;
	 updateItem = (struct npd_bssid_item_s *)newItem;
	 return ret;
 }

 long npd_capwap_bssid_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;
	 struct npd_bssid_item_s *opItem = NULL;

	 if( newItem == NULL ){
	 	syslog_ax_asd_dbg("The new item to be insert is NULL\r\n");
	 	return CAPWAP_RETURN_CODE_ERROR;
	 }

	 opItem = (struct npd_bssid_item_s *)newItem;

	 return ret;
 }

  long npd_capwap_bssid_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;
	struct npd_bssid_item_s *opItem = NULL; 

	if( delItem == NULL ){
		syslog_ax_asd_dbg("The to be deleted item is NULL\r\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	opItem = (struct npd_bssid_item_s *)delItem;

	return ret;
 }


 long npd_capwap_ts_qos_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;	 
	 int flag = 0;
	 struct npd_ts_qos_item_s *origItem = NULL, *updateItem = NULL;
	 

	 if( (newItem == NULL ) || ( oldItem == NULL ) )
	 	return CAPWAP_RETURN_CODE_ERROR;

	 origItem = (struct npd_ts_qos_item_s *)oldItem;
	 updateItem = (struct npd_ts_qos_item_s *)newItem;

	 return ret;
 }

 long npd_capwap_ts_qos_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;
	 struct npd_ts_qos_item_s *opItem = NULL;

	 if( newItem == NULL ){
	 	syslog_ax_asd_dbg("The new item to be insert is NULL\r\n");
	 	return CAPWAP_RETURN_CODE_ERROR;
	 }

	 opItem = (struct npd_ts_qos_item_s *)newItem;

	 return ret;
 }

  long npd_capwap_ts_qos_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;
	struct npd_ts_qos_item_s *opItem = NULL; 

	if( delItem == NULL ){
		syslog_ax_asd_dbg("The to be deleted item is NULL\r\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	opItem = (struct npd_ts_qos_item_s *)delItem;

	return ret;
 }


 long npd_capwap_tt_qos_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag = 0;
	 struct npd_tt_qos_item_s *origItem = NULL, *updateItem = NULL;
	 

	 if( (newItem == NULL ) || ( oldItem == NULL ) )
	 	return CAPWAP_RETURN_CODE_ERROR;

	 origItem = (struct npd_tt_qos_item_s *)oldItem;
	 updateItem = (struct npd_tt_qos_item_s *)newItem;

	 return ret;
 }

 long npd_capwap_tt_qos_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;
	 struct npd_tt_qos_item_s *opItem = NULL;

	 if( newItem == NULL ){
	 	syslog_ax_asd_dbg("The new item to be insert is NULL\r\n");
	 	return CAPWAP_RETURN_CODE_ERROR;
	 }

	 opItem = (struct npd_tt_qos_item_s *)newItem;

	 return ret;
 }

  long npd_capwap_tt_qos_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;
	struct npd_tt_qos_item_s *opItem = NULL; 

	if( delItem == NULL ){
		syslog_ax_asd_dbg("The to be deleted item is NULL\r\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	opItem = (struct npd_tt_qos_item_s *)delItem;

	return ret;
 }


long npd_capwap_global_conf_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag = 0;

	 return ret;
 }

 long npd_capwap_global_conf_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;

	 return ret;
 }

  long npd_capwap_global_conf_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

 	return ret;
 }

long npd_capwap_L3_conf_dbtbl_handle_update( void *newItem, void *oldItem)
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag = 0;

	 return ret;
 }

 long npd_capwap_L3_conf_dbtbl_handle_insert( void *newItem )
 {
	 int ret = CAPWAP_RETURN_CODE_SUCCESS;
	 int flag;

	 return ret;
 }

  long npd_capwap_L3_conf_dbtbl_handle_delete( void *delItem )
 {
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

 	return ret;
 }

int npd_capwap_table_init()
{
 	int ret;
 	
	
	ret = create_dbtable(NPD_CAPWAP_TUNNEL_NAME, NPD_CAPWAP_TUNNEL_TABLE_SIZE, sizeof(struct capwap_db_tunnel_entry_s),\
					npd_capwap_tunnel_dbtbl_handle_update, 
					NULL,
					npd_capwap_tunnel_dbtbl_handle_insert, 
					npd_capwap_tunnel_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_tunnel_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap tunnel database fail\n");
		return NPD_FAIL;
	}

	
	ret = dbtable_create_array_index("tunnel_array", npd_capwap_tunnel_dbtbl, &npd_capwap_tunnel_array_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd capwap global conf array table fail\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}
	

	ret = dbtable_create_hash_index("tunnel_hash", npd_capwap_tunnel_dbtbl,NPD_CAPWAP_TUNNEL_HASH_TABLE_SIZE, npd_capwap_tunnel_keyidx_generate,\
													npd_capwap_tunnel_compare, &npd_capwap_tunnel_hash_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd capwap tunnel hash table fail\n");
		return NPD_FAIL;
	}	

	ret = create_dbtable(NPD_CAPWAP_BSSID_NAME, NPD_CAPWAP_BSSID_TABLE_SIZE, sizeof(struct npd_bssid_item_s),\
					npd_capwap_bssid_dbtbl_handle_update, 
					NULL,
					npd_capwap_bssid_dbtbl_handle_insert, 
					npd_capwap_bssid_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_bssid_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap bssid database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("netif_hash", npd_capwap_bssid_dbtbl,NPD_CAPWAP_BSSID_HASH_TABLE_SIZE, npd_capwap_bssid_netif_keyidx_generate,\
													npd_capwap_bss_netif_compare, &npd_capwap_bss_netif_hash_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd capwap bssid hash table fail\n");
		return NPD_FAIL;
	}		

	ret = create_dbtable(NPD_CAPWAP_TS_QOS_NAME, NPD_CAPWAP_TT_QOS_TABLE_SIZE, sizeof(struct npd_ts_qos_item_s),\
					npd_capwap_ts_qos_dbtbl_handle_update, 
					NULL,
					npd_capwap_ts_qos_dbtbl_handle_insert, 
					npd_capwap_ts_qos_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_ts_qos_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap ts qos database fail\n");
		return NPD_FAIL;
	}

	ret = create_dbtable(NPD_CAPWAP_TT_QOS_NAME, NPD_CAPWAP_TT_QOS_TABLE_SIZE, sizeof(struct npd_tt_qos_item_s),\
					npd_capwap_tt_qos_dbtbl_handle_update, 
					NULL,
					npd_capwap_tt_qos_dbtbl_handle_insert, 
					npd_capwap_tt_qos_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_tt_qos_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap tt qos database fail\n");
		return NPD_FAIL;
	}


	ret = create_dbtable(NPD_CAPWAP_GLOBAL_CONF, NPD_CAPWAP_GLOBAL_CONF_TABLE_SIZE, sizeof(struct npd_capwap_global_conf_s),\
					npd_capwap_global_conf_dbtbl_handle_update, 
					NULL,
					npd_capwap_global_conf_dbtbl_handle_insert, 
					npd_capwap_global_conf_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_global_conf_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap tt qos database fail\n");
		return NPD_FAIL;
	}	
	
	ret = dbtable_create_array_index(NPD_CAPWAP_GLOBAL_ARR_TABLE_NAME, npd_capwap_global_conf_dbtbl, &npd_capwap_global_conf_arr_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd capwap global conf array table fail\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}


	ret = create_dbtable(NPD_CAPWAP_L3_CONF, NPD_CAPWAP_L3_CONF_TABLE_SIZE, sizeof(struct npd_capwap_L3_conf_s),\
					npd_capwap_L3_conf_dbtbl_handle_update, 
					NULL,
					npd_capwap_L3_conf_dbtbl_handle_insert, 
					npd_capwap_L3_conf_dbtbl_handle_delete,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_capwap_L3_conf_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_asd_err("create npd capwap L3 configure database fail\n");
		return NPD_FAIL;
	}	
	
	ret = dbtable_create_array_index(NPD_CAPWAP_L3_ARR_TABLE_NAME, npd_capwap_L3_conf_dbtbl, &npd_capwap_L3_conf_arr_index);
	if( 0  != ret )
	{
		syslog_ax_asd_err("create npd capwap L3 conf array table fail\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}	

	syslog_ax_asd_dbg("Register npd capwap netif event notifier\n");
	register_netif_notifier(&npd_capwap_netif_notifier);
	return NPD_OK;
 }

int npd_capwap_get_global_conf(unsigned int conf_id, struct npd_capwap_global_conf_s *global_conf)
{
	if(conf_id >= NPD_CAPWAP_GLOBAL_CONF_TABLE_SIZE ){
		syslog_ax_asd_err("func:%s, Config ID %d is out of range\r\n", __func__, conf_id);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_get(npd_capwap_global_conf_arr_index, conf_id, global_conf);
}

int npd_capwap_set_global_conf(unsigned int conf_id, struct npd_capwap_global_conf_s * global_conf)
{
	int ret = 0;
	struct npd_capwap_global_conf_s pre_conf;

	if( global_conf == NULL ){
		syslog_ax_asd_err("func:%s global conf is NULL\r\n", __func__);
		return -1;
	}
	if( 0 == dbtable_array_get(npd_capwap_global_conf_arr_index, conf_id, &pre_conf))
	{
		ret = dbtable_array_update(npd_capwap_global_conf_arr_index, conf_id, &pre_conf, global_conf);
	}
	else
	{
		ret = dbtable_array_insert_byid(npd_capwap_global_conf_arr_index, conf_id, global_conf);
	}

	return ret;
}

int npd_capwap_del_global_conf(unsigned int conf_id, struct npd_capwap_global_conf_s *global_conf)
{
	if( global_conf == NULL ){
		syslog_ax_asd_err("func:%s global conf is NULL\r\n", __func__);
		return -1;
	}

	return dbtable_array_delete(npd_capwap_global_conf_arr_index, conf_id, global_conf);
}


int npd_capwap_get_L3_conf(unsigned int conf_id, struct npd_capwap_L3_conf_s *L3_conf)
{
	if(conf_id >= NPD_CAPWAP_L3_CONF_TABLE_SIZE ){
		syslog_ax_asd_err("func:%s L3 config ID %d is out of range\r\n", __func__,conf_id);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_get(npd_capwap_L3_conf_arr_index, conf_id, L3_conf);
}

int npd_capwap_set_L3_conf(unsigned int conf_id, struct npd_capwap_L3_conf_s * L3_conf)
{
	int ret = 0;
	struct npd_capwap_L3_conf_s pre_conf;

	if( L3_conf == NULL ){
		syslog_ax_asd_err("func:%s L3 conf is NULL\r\n", __func__);
		return -1;
	}
	if( 0 == dbtable_array_get(npd_capwap_L3_conf_arr_index, conf_id, &pre_conf))
	{
		ret = dbtable_array_update(npd_capwap_L3_conf_arr_index, conf_id, &pre_conf, L3_conf);
	}
	else
	{
		ret = dbtable_array_insert_byid(npd_capwap_L3_conf_arr_index, conf_id, L3_conf);
	}

	return ret;
}

int npd_capwap_del_L3_conf(unsigned int conf_id, struct npd_capwap_L3_conf_s *L3_conf)
{
	if( L3_conf == NULL ){
		npd_syslog_err("func:%s L3 conf is NULL\r\n", __func__);
		return -1;
	}

	return dbtable_array_delete(npd_capwap_L3_conf_arr_index, conf_id, L3_conf);
}


int npd_capwap_get_tunnel_entry(struct capwap_db_tunnel_entry_s *tunnel_conf)
{
	if( tunnel_conf == NULL ){
		syslog_ax_asd_err("func:%s tunnel conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	
	return dbtable_hash_search(npd_capwap_tunnel_hash_index, tunnel_conf, NULL, tunnel_conf);
}


int npd_capwap_get_tunnel_entry_by_id(unsigned int tunnel_id, struct capwap_db_tunnel_entry_s *tunnel_conf)
{
	if(tunnel_id >= NPD_CAPWAP_TUNNEL_TABLE_SIZE ){
		syslog_ax_asd_err("func:%s tunnel entry ID %d is out of range\r\n", __func__, tunnel_id);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	if( tunnel_conf == NULL ){
		syslog_ax_asd_err("func:%s tunnel conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_get(npd_capwap_tunnel_array_index, tunnel_id, tunnel_conf);
}

int npd_capwap_set_tunnel_entry_by_id(unsigned int tunnel_id, struct capwap_db_tunnel_entry_s * tunnel_conf)
{
	int ret = 0;
	struct capwap_db_tunnel_entry_s pre_conf;

	if(tunnel_id >= NPD_CAPWAP_TUNNEL_TABLE_SIZE){
		syslog_ax_asd_err("func:%s tunnel entry ID %d is out of range\r\n", __func__, tunnel_id);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	if( tunnel_conf == NULL ){
		syslog_ax_asd_err("func:%s tunnel conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	if( 0 == dbtable_array_get(npd_capwap_tunnel_array_index, tunnel_id, &pre_conf))
	{
		ret = dbtable_array_update(npd_capwap_tunnel_array_index, tunnel_id, &pre_conf, tunnel_conf);
	}
	else
	{
		ret = dbtable_array_insert_byid(npd_capwap_tunnel_array_index, tunnel_id, tunnel_conf);
	}

	return ret;
}


int npd_capwap_del_tunnel_entry_by_id(unsigned int tunnel_id, struct capwap_db_tunnel_entry_s *tunnel_conf)
{
	if(tunnel_id >= NPD_CAPWAP_TUNNEL_TABLE_SIZE){
		syslog_ax_asd_err("func:%s tunnel entry ID %d is out of range\r\n", __func__, tunnel_id);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	
	if( tunnel_conf == NULL ){
		syslog_ax_asd_err("func:%s tunnel conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_delete(npd_capwap_tunnel_array_index, tunnel_id, tunnel_conf);
}

int npd_capwap_bssid_filter(struct npd_bssid_item_s *pre_bssid_conf, struct npd_bssid_item_s *new_bssid_conf)
{
	if(memcmp(pre_bssid_conf->BSS_ID.bssid, new_bssid_conf->BSS_ID.bssid, MAC_LEN) == 0){
		return TRUE;
	}
	
	return FALSE;
}

int npd_capwap_get_bssid_entry(struct npd_bssid_item_s *bssid_conf)
{
	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_hash_search(npd_capwap_bss_netif_hash_index, bssid_conf, NULL, bssid_conf);
}

int npd_capwap_get_bssid_entry_by_bssid(struct npd_bssid_item_s *bssid_conf)
{
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	
	ret = dbtable_hash_head(npd_capwap_bss_netif_hash_index, bssid_conf, bssid_conf, npd_capwap_bssid_filter);
	if(ret !=0 ){
		syslog_ax_asd_err("func:%s Fail to get bssid_conf ret %d\r\n", __func__, ret);
		return -1;
	}
	
	return CAPWAP_RETURN_CODE_SUCCESS;
}


int npd_capwap_get_bssid_entry_by_id(unsigned int bssid_index, struct npd_bssid_item_s *bssid_conf)
{
	if(bssid_index >= NPD_CAPWAP_TUNNEL_TABLE_SIZE ){
		syslog_ax_asd_err("func:%s bssid entry ID %d is out of range\r\n", __func__, bssid_index);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_get(npd_capwap_bssid_array_index, bssid_index, bssid_conf);
}

int npd_capwap_set_bssid_entry(struct npd_bssid_item_s * bssid_conf)
{
	int ret = 0;
	struct npd_bssid_item_s pre_conf;

	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	if( 0 == dbtable_hash_search(npd_capwap_bss_netif_hash_index,bssid_conf, NULL, &pre_conf))
	{
		ret = dbtable_hash_update(npd_capwap_bss_netif_hash_index, &pre_conf, bssid_conf);
	}
	else
	{
		ret = dbtable_hash_insert(npd_capwap_bss_netif_hash_index, bssid_conf);
	}

	return ret;
}

int npd_capwap_set_bssid_entry_by_id(unsigned int bssid_index, struct npd_bssid_item_s * bssid_conf)
{
	int ret = 0;
	struct npd_bssid_item_s pre_conf;

	if(bssid_index >= NPD_CAPWAP_TUNNEL_TABLE_SIZE){
		syslog_ax_asd_err("func:%s, bssid entry ID %d is out of range\r\n", __func__, bssid_index);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	if( 0 == dbtable_array_get(npd_capwap_bssid_array_index, bssid_index, &pre_conf))
	{
		ret = dbtable_array_update(npd_capwap_bssid_array_index, bssid_index, &pre_conf, bssid_conf);
	}
	else
	{
		ret = dbtable_array_insert_byid(npd_capwap_bssid_array_index, bssid_index, bssid_conf);
	}

	return ret;
}


int npd_capwap_del_bssid_entry(struct npd_bssid_item_s *bssid_conf)
{
	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_hash_delete(npd_capwap_bss_netif_hash_index, bssid_conf, bssid_conf);
}


int npd_capwap_del_bssid_entry_by_id(unsigned int bssid_index, struct npd_bssid_item_s *bssid_conf)
{
	if(bssid_index >= NPD_CAPWAP_TUNNEL_TABLE_SIZE){
		syslog_ax_asd_err("func:%s, bssid entry ID %d is out of range\r\n", __func__, bssid_index);
		return CAPWAP_RETURN_CODE_ERROR;
	}
	if( bssid_conf == NULL ){
		syslog_ax_asd_err("func:%s bssid_conf is NULL\r\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	return dbtable_array_delete(npd_capwap_bssid_array_index, bssid_index, bssid_conf);
}


int npd_capwap_get_netif_by_bssid(unsigned char *bssid)
{
	struct npd_bssid_item_s bssid_conf;
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

	memset(&bssid_conf, 0, sizeof(bssid_conf));
	memcpy(bssid_conf.BSS_ID.bssid, bssid, MAC_ADDR_LEN);
	ret = npd_capwap_get_bssid_entry_by_bssid(&bssid_conf);
	if(ret != 0){
		syslog_ax_asd_err("func:%s, fail to get bssid_conf by bssid"MACSTR"\r\n", __func__, MAC2STR(bssid));
		return -1;
	}
	
	/*ret = dbtable_hash_head_key(npd_capwap_bss_netif_hash_index, &bssid_conf, &bssid_conf, npd_capwap_bssid_filter);
	if(ret !=0 ){
		return -1;
	}*/
	
	return bssid_conf.netif_index;
}

int npd_capwap_get_bssid_by_netif( unsigned int netif_index, char *bssid)
{

	struct npd_bssid_item_s bssid_conf;
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

	memset(&bssid_conf, 0, sizeof(bssid_conf));
	bssid_conf.netif_index = netif_index;
	
	ret = npd_capwap_get_bssid_entry(&bssid_conf);
	if(ret != 0){
		syslog_ax_asd_err("Fail to get bssid conf by netif_index 0x%x ret:%d\r\n", netif_index, ret);
		return ret;
	}
	
	memcpy(bssid, bssid_conf.BSS_ID.bssid, MAC_ADDR_LEN);
	return CAPWAP_RETURN_CODE_SUCCESS;
}

int npd_capwap_get_info_by_bssid
(
    char *bssid, 
    unsigned int *wtp_ipaddr, 
    unsigned int *sourceIp, 
    unsigned short *sPort, 
    unsigned short *dPort, 
    unsigned int *encap_format
)
{

	struct npd_bssid_item_s bssid_conf;
	struct capwap_db_tunnel_entry_s tunnel_conf;
	struct npd_capwap_L3_conf_s L3_conf;
	int netif_index 			= -1;
	int ret 					= CAPWAP_RETURN_CODE_SUCCESS;

	/*get bssid entry according to bssid*/
	memset(&bssid_conf, 0, sizeof(bssid_conf));
	memcpy(bssid_conf.BSS_ID.bssid, bssid, MAC_ADDR_LEN);
	ret = npd_capwap_get_bssid_entry_by_bssid(&bssid_conf);
	if(ret != 0){
		syslog_ax_asd_err("Fail to get bssid conf by bssid "MACSTR" ret:%d\r\n", MAC2STR(bssid), ret);
		return -1;
	}
	
	/*get tunnel entry according to tunnel id*/
	memset(&tunnel_conf, 0, sizeof(tunnel_conf));
	ret = npd_capwap_get_tunnel_entry_by_id(bssid_conf.tunnel_id, &tunnel_conf);
	if(ret != 0){
		syslog_ax_asd_err("Fail to get tunnel conf by tuunel_id:%d ret:%d\r\n", bssid_conf.tunnel_id, ret);
		return -1;
	}

	/*get  L3 interface that enabled capwap*/
	memset(&L3_conf, 0, sizeof(L3_conf));
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("Fail to get L3 conf by config ID:%d ret:%d\r\n", NPD_CAPWAP_L3_CONF_ID, ret);
		return -1;
	}
	
	*wtp_ipaddr 	= tunnel_conf.tunnel_entry.remote_ip4;
	*sourceIp 		= L3_conf.ip_addr;
	*sPort			= CAPWAP_DEFAULT_DATA_PORT_NUM;
	*dPort			= tunnel_conf.tunnel_entry.remote_l4_port;
	*encap_format	= tunnel_conf.tunnel_entry.frame_format;

	return CAPWAP_RETURN_CODE_SUCCESS;
	
}

int npd_capwap_get_pvid_by_netif( unsigned int netif_index, unsigned short *pvid)
{

	struct npd_bssid_item_s bssid_conf;
	int ret = CAPWAP_RETURN_CODE_SUCCESS;

	memset(&bssid_conf, 0, sizeof(bssid_conf));
	bssid_conf.netif_index = netif_index;
	
	ret = npd_capwap_get_bssid_entry(&bssid_conf);
	if(ret != 0){
		syslog_ax_asd_err("func:%s Fail to get bssid conf netif index:0x%x ret:%d\r\n", __func__, netif_index, ret);
		return ret;
	}

	*pvid = bssid_conf.pvid;
	return CAPWAP_RETURN_CODE_SUCCESS;
}

int npd_capwap_keep_capwap_port_vlan_relation(unsigned short vlan_id, unsigned int op)
{
	struct npd_capwap_L3_conf_s L3_conf;
	unsigned char	search_flag = FALSE;
	unsigned char	array_index = 0;
	unsigned char	slot_num = 0xFF;
	unsigned int	unit_id = 0;
	unsigned int ret = 0;

	if(0 == vlan_id){
		syslog_ax_asd_err("func:%s VLAN ID is 0\r\n", __func__);
		return ret;
	}
	
	memset(&L3_conf, 0, sizeof(L3_conf));
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("func:%s Fail to get L3 conf ret:%d\r\n", __func__, ret);
		return ret;
	}

	for(array_index = 0; array_index < CAPWAP_PORT_MAX_VLAN_NUM; array_index++){
		//search if capwap port already join in vlan
		if((L3_conf.capwap_port_vlan_ref.vlan_array[array_index] == vlan_id)
			&&(L3_conf.capwap_port_vlan_ref.ref_cnt[array_index] != 0)){
			search_flag = TRUE;
			slot_num = array_index;
			break;
		}
		//search free slot for keep new vlan to join when add
		if((0 == L3_conf.capwap_port_vlan_ref.vlan_array[array_index])
			&&(0 == L3_conf.capwap_port_vlan_ref.ref_cnt[array_index])
			&&(0xFF == slot_num)){
			slot_num = array_index;
		}
	}

	if(0xFF == slot_num){
		syslog_ax_asd_err("func:%s Fail to get exist or free slot number\r\n", __func__);
		return -1;
	}


	switch(op){
		case CAPWAP_ADD:{
			if(TRUE == search_flag){
				L3_conf.capwap_port_vlan_ref.ref_cnt[slot_num]++;
			}
			else{
				L3_conf.capwap_port_vlan_ref.vlan_array[slot_num]= vlan_id;
				L3_conf.capwap_port_vlan_ref.ref_cnt[slot_num] = 1;
				ret = nam_set_port25_vlan_id(unit_id, vlan_id);
                ret = nam_atheros_l2_arp_packet_to_cpu(unit_id, vlan_id, TRUE);
			}
			break;
		}
		case CAPWAP_DEL:{
			if(TRUE == search_flag){
				L3_conf.capwap_port_vlan_ref.ref_cnt[slot_num]--;
				if(L3_conf.capwap_port_vlan_ref.ref_cnt[slot_num] == 0){
					L3_conf.capwap_port_vlan_ref.vlan_array[slot_num]= 0;
					ret = nam_del_port25_vlan_id(unit_id, vlan_id);
                    ret = nam_atheros_l2_arp_packet_to_cpu(unit_id, vlan_id, FALSE);
				}
			}
			break;
		}
		default:{
			break;
		}
	}

	return npd_capwap_set_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	
}

/**********************************************************************************
*npd_capwap_init()
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*	
*DESCRIPTION:
*	CAPWAP init Global Flag 
*	and user table database
*
***********************************************************************************/
void npd_capwap_init(void)
{
	struct npd_capwap_L3_conf_s L3_conf;
	unsigned int	unit_id = 0;
	int				ret;
	
	npd_capwap_table_init();
	memset(&L3_conf, 0, sizeof(L3_conf));
	L3_conf.L3_status = TRUE;
	L3_conf.wifi_indicator_mode = WIFI_INDICATOR_AP_MODE;
	L3_conf.ap_id = 1;
	L3_conf.station_threshold = WIFI_INDICATOR_STATION_THRESHOLD;
	npd_capwap_set_L3_conf(0, &L3_conf);
	ret = nam_capwap_wlan_isolation_global_control(unit_id, TRUE);
	nam_thread_create("capwapSock", (void *)npd_capwap_mng_thread_dgram, NULL, NPD_TRUE, NPD_FALSE);
	
    nam_thread_create("SysLEDShow", (void *)npd_capwap_wifi_indicator_control_thread, NULL, NPD_TRUE, NPD_FALSE);
}

/*******************************************************************************
 * npd_capwap_sock_init
 *
 * DESCRIPTION:
 *   		create socket communication with capwap module
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	ser_sock - socket number
 *
 * RETURNS:
 * 		CAPWAP_RETURN_CODE_SUCCESS   - create successfully 
 * 		CAPWAP_RETURN_CODE_ERROR	     - create socket or bind error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int	npd_capwap_sock_init(int *ser_sock)
{
	memset(&capwap_to_npd_table_addr, 0, sizeof(capwap_to_npd_table_addr));
	memset(&npd_to_capwap_table_addr, 0, sizeof(npd_to_capwap_table_addr));

	if((*ser_sock = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		syslog_ax_asd_err("create npd to capwap socket fail\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	capwap_to_npd_table_addr.sun_family = AF_LOCAL;
	strcpy(capwap_to_npd_table_addr.sun_path, "/tmp/capwap_to_npd");
	npd_to_capwap_table_addr.sun_family = AF_LOCAL;
	strcpy(npd_to_capwap_table_addr.sun_path, "/var/run/wcpss/wid");


    unlink(capwap_to_npd_table_addr.sun_path);
    unlink(npd_to_capwap_table_addr.sun_path);

	if(bind(*ser_sock , (struct sockaddr *)&capwap_to_npd_table_addr, sizeof(capwap_to_npd_table_addr)) == -1) 
	{
		syslog_ax_asd_err("npd to capwap socket created but failed when bind\n");
		return CAPWAP_RETURN_CODE_ERROR;
	}

	chmod(capwap_to_npd_table_addr.sun_path, 0777);
	chmod(npd_to_capwap_table_addr.sun_path, 0777);
	return CAPWAP_RETURN_CODE_SUCCESS;	
	
}

/*******************************************************************************
 * npd_capwap_recv_info
 *
 * DESCRIPTION:
 *   		use recvfrom to receive information from capwap module 
 *
 * INPUTS:
 *		infoLen - the receive max size
 *
 * OUTPUTS:
 * 		msg - pointer to capwap message get from capwap module
 *		len - actual receive the data size
 *
 * RETURNS:
 *		CAPWAP_RETURN_CODE_SUCCESS - complete the receive
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int npd_capwap_recv_info
(
	struct npd_mng_capwap *capwap_msg,
	unsigned int  infoLen,
	int *len
)
{
	unsigned int addrLen = sizeof(capwap_to_npd_table_addr);
	while(1)
	{
		*len = recvfrom(npd_capwap_fd, (char*)capwap_msg, CAPWAP_MNG_MAX_LEN, 0,(struct sockaddr *)&capwap_to_npd_table_addr, &addrLen);
		if(*len < 0 && errno == EINTR) 
		{
			continue;
		}
		break;
	}
	
	if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
		syslog_ax_asd_dbg("The slot is not Active master\r\n");	
		return CAPWAP_RETURN_CODE_SUCCESS;
	}

	syslog_ax_asd_dbg("npd receive message from capwap module:len:%d,type:%d, op:%d\r\n",
						*len,capwap_msg->msg_type, capwap_msg->cap_action);	
	return CAPWAP_RETURN_CODE_SUCCESS;	
}

int npd_capwap_switchover(struct npd_mng_capwap * capwap_msg)
{
//	return npd_fdb_static_entry_del_by_port(asd_msg->ifIndex);
	return 0;
}

unsigned int bss_generate_ifindex(
    unsigned int wtp_id,
    unsigned int local_radio_id,
    unsigned int local_bss_index)
{
    NPD_NETIF_INDEX_U bss_ifindex;
	bss_ifindex.netif_index = 0;
	bss_ifindex.wireless_if.type = NPD_NETIF_WIRELESS_TYPE;
	bss_ifindex.wireless_if.wtpid = wtp_id;
	bss_ifindex.wireless_if.radio_local_id = local_radio_id;
	bss_ifindex.wireless_if.bss_local_index = local_bss_index;
	return bss_ifindex.netif_index;
}

int npd_capwap_ap_netif_add_record(npd_capwap_tunnel_entry_t entry)
{
	NPD_FDB fdb;
	int ret;
	char buf[512] = {0};
	
	ret = nam_show_fdb_mac(&fdb, entry.mac, 1);
	if(1 == ret)
	{
		unsigned int netif_index = 0;
		if(fdb.inter_type == NAM_INTERFACE_PORT_E)
			netif_index = fdb.value;
		else
			netif_index = npd_netif_trunk_get_index(fdb.value);
		if(netif_index)
		{
		   sprintf(buf, "sudo echo %u > /var/run/%x.%x.%x", netif_index, *(unsigned short *)&entry.mac[0],
		                                                        *(unsigned short *)&entry.mac[2],
		                                                        *(unsigned short *)&entry.mac[4]);
		   system(buf);
		}

	}
}

int npd_capwap_ap_netif_del_record(npd_capwap_tunnel_entry_t entry)
{
    char buf[512] = {0};
	
    sprintf(buf, "rm -rf /var/run/%x.%x.%x",  *(unsigned short *)&entry.mac[0],
 		                                                        *(unsigned short *)&entry.mac[2],
		                                                        *(unsigned short *)&entry.mac[4]);
	system(buf);
}


int npd_capwap_tunnel_configure(struct npd_mng_capwap *capwap_msg)
{
	int		ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id = 0;
	unsigned int tunnel_id = 0;
	struct capwap_db_tunnel_entry_s	tunnel_conf;


	printf("Enter func:%s line:%d\r\n", __func__, __LINE__);
	memcpy(&tunnel_conf, &capwap_msg->u.tunnel_db_entry, sizeof(tunnel_conf));
	tunnel_id = tunnel_conf.tunnel_id;

	switch(capwap_msg->cap_action)
	{
		case CAPWAP_ADD:
			if(npd_capwap_get_tunnel_entry(&tunnel_conf) == CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("func:%s line:%d tunnel entry already exist\r\n", __func__, __LINE__);
			}
			else{
				ret = npd_capwap_set_tunnel_entry_by_id(tunnel_id, &capwap_msg->u.tunnel_db_entry);
				ret = nam_capwap_tunnel_entry_add(dev_id, tunnel_id, &capwap_msg->u.tunnel_db_entry.tunnel_entry);
				syslog_ax_asd_dbg("func:%s line:%d tunnel add result %d\r\n", __func__, __LINE__, ret);
			}
			ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_AP_LOGIN, (void *)tunnel_id);
			ret = npd_capwap_ap_netif_add_record(capwap_msg->u.tunnel_db_entry.tunnel_entry);
			syslog_ax_asd_dbg("func:%s line:%d wifi event process result %d\r\n", __func__, __LINE__, ret);
			break;
		case CAPWAP_DEL:
			if(npd_capwap_get_tunnel_entry(&tunnel_conf) != CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("func:%s line:%d tunnel entry does't exist\r\n", __func__, __LINE__);
			}
			else{
				ret = nam_capwap_tunnel_entry_del(dev_id, tunnel_id);
			    ret = npd_capwap_ap_netif_del_record(tunnel_conf.tunnel_entry);
				syslog_ax_asd_dbg("func:%s line:%d tunnel del result %d\r\n", __func__, __LINE__, ret);
				ret = npd_capwap_del_tunnel_entry_by_id(tunnel_id, &tunnel_conf);
			}
			ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_AP_LOGOUT, (void*)tunnel_id);
			syslog_ax_asd_dbg("func:%s line:%d wifi event process result %d\r\n", __func__, __LINE__, ret);
			break;
		case CAPWAP_MOD:
			if(npd_capwap_get_tunnel_entry(&tunnel_conf) != CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("func:%s line:%d tunnel entry does't exist\r\n", __func__, __LINE__);
			}
			else{
				ret = nam_capwap_tunnel_entry_del(dev_id, tunnel_id);
				ret = npd_capwap_del_tunnel_entry_by_id(tunnel_id, &tunnel_conf);
				syslog_ax_asd_dbg("func:%s line:%d tunnel del result %d\r\n", __func__, __LINE__, ret);
			}
			ret = npd_capwap_set_tunnel_entry_by_id(tunnel_id, &capwap_msg->u.tunnel_db_entry);
			ret = nam_capwap_tunnel_entry_add(dev_id, tunnel_id, &capwap_msg->u.tunnel_db_entry.tunnel_entry);
			syslog_ax_asd_dbg("func:%s line:%d tunnel del result %d\r\n", __func__, __LINE__, ret);
			break;
		default :
		{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	
	return ret;
}

#define FAL_CAPWAP_TUNNEL_TYPE_WLAN    0
#define FAL_CAPWAP_TUNNEL_TYPE_ROAMING 1

#define FAL_WLAN_VID_CMD_DISABLE      0
#define FAL_WLAN_VID_CMD_UNTAG_PRITAG 1
#define FAL_WLAN_VID_CMD_TAG          2
#define FAL_WLAN_VID_CMD_ALL          3
#define FAL_WLAN_VID_CMD_MAX          4
int npd_capwap_bssid_configure(struct npd_mng_capwap *capwap_msg)
{
	int		ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int unit_id = 0;
	npd_capwap_wlan_t	bssid_info;
	struct npd_bssid_item_s bssid_conf;

	memcpy(&bssid_conf, &capwap_msg->u.bssid_entry, sizeof(bssid_conf));

	switch(capwap_msg->cap_action)
	{
		case CAPWAP_BSS_CREATE:{
			struct npd_bssid_item_s pre_conf;
			
			pre_conf.netif_index = bssid_conf.netif_index;
			if(npd_capwap_get_bssid_entry(&pre_conf) == CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("The bssid entry already exists\n");	
				return CAPWAP_RETURN_CODE_ENTRY_ALREADY_EXIST;
			}
			else{
				npd_key_database_lock();
				npd_create_switch_port(bssid_conf.netif_index, "bss", &bssid_conf.swith_port_index, bssid_conf.cflag);
				netif_notify_event(bssid_conf.netif_index, PORT_NOTIFIER_L2CREATE);
				if(bssid_conf.pvid != DEFAULT_VLAN_ID){
					npd_netif_allow_vlan(bssid_conf.netif_index, bssid_conf.pvid, FALSE, TRUE);
				}
				npd_key_database_unlock();
				ret = npd_capwap_set_bssid_entry(&bssid_conf);
			}
			break;
		}
		case CAPWAP_BSS_DELETE:
			if(npd_capwap_get_bssid_entry(&bssid_conf) != CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("The bssid entry does't exist\n");
				return CAPWAP_RETURN_CODE_ENTRY_NOT_EXIST;
			}
			else{
				npd_key_database_lock();
				netif_notify_event(bssid_conf.netif_index, PORT_NOTIFIER_L2DELETE);
				npd_delete_switch_port(bssid_conf.swith_port_index);
				npd_key_database_unlock();
				if(bssid_conf.cflag == TRUE){
					ret = nam_wlan_entry_delete(unit_id, &bssid_conf.BSS_ID);
				}
				ret = npd_capwap_del_bssid_entry(&bssid_conf);
			}
			break;
		case CAPWAP_BSS_ACTIVE:{
			struct npd_bssid_item_s pre_conf;

			pre_conf.netif_index = bssid_conf.netif_index;
			if(npd_capwap_get_bssid_entry(&pre_conf) != CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("The bssid entry does't exist\n");
				return CAPWAP_RETURN_CODE_ENTRY_NOT_EXIST;
			}
			else if(TRUE == pre_conf.cflag ){
				syslog_ax_asd_err("The bssid entry are active\n");
				return CAPWAP_RETURN_CODE_SUCCESS;
			}
			else{
				npd_wlan_vlan_t entry;

				/*get bssid tunnel id and flag*/
				memcpy(pre_conf.BSS_ID.bssid, bssid_conf.BSS_ID.bssid, MAC_LEN);
				pre_conf.tunnel_id	= bssid_conf.tunnel_id;
				pre_conf.cflag		= bssid_conf.cflag;
				ret = npd_capwap_set_bssid_entry(&pre_conf);
				
				bssid_info.cflag= pre_conf.cflag;
				bssid_info.radio_id = npd_netif_local_radio_id_get(pre_conf.netif_index);
				bssid_info.tunnel_id = pre_conf.tunnel_id;
				
				ret = nam_wlan_entry_add(unit_id, &pre_conf.BSS_ID, &bssid_info);
				
				memset(&entry, 0, sizeof(entry));
				entry.svid_cmd	= FAL_WLAN_VID_CMD_ALL;
				entry.svid		= pre_conf.pvid; 
				ret = nam_wlan_vlan_set(unit_id, &bssid_conf.BSS_ID, &entry);
				npd_key_database_lock();
				netif_notify_event(pre_conf.netif_index, PORT_NOTIFIER_LINKUP_E);
				npd_key_database_unlock();

			}
			break;
		}
		case CAPWAP_BSS_INACTIVE:{
			struct npd_bssid_item_s pre_conf;

			pre_conf.netif_index = bssid_conf.netif_index;
			if(npd_capwap_get_bssid_entry(&pre_conf) != CAPWAP_RETURN_CODE_SUCCESS){
				syslog_ax_asd_err("The bssid entry does't exist\n");
				return CAPWAP_RETURN_CODE_ENTRY_NOT_EXIST;
			}
			else if(FALSE == pre_conf.cflag ){
				syslog_ax_asd_err("The bssid entry are inactive\n");
				return CAPWAP_RETURN_CODE_SUCCESS;
			}
			else{
				npd_wlan_vlan_t entry;

				npd_key_database_lock();
				netif_notify_event(pre_conf.netif_index, PORT_NOTIFIER_LINKDOWN_E);
				npd_key_database_unlock();
				/*get bssid tunnel id and flag*/
				pre_conf.cflag		= bssid_conf.cflag;
				ret = npd_capwap_set_bssid_entry(&pre_conf);
				
				ret = nam_wlan_entry_delete(unit_id, &pre_conf.BSS_ID);
			}
			break;
		}
		case CAPWAP_WLAN_SET_PVID:{
			struct npd_bssid_item_s pre_conf;

			pre_conf.netif_index = bssid_conf.netif_index;
			if(npd_capwap_get_bssid_entry(&pre_conf) == CAPWAP_RETURN_CODE_SUCCESS){
				unsigned short	pre_vlan_id = 0;
				
				syslog_ax_asd_dbg("The bssid entry exist, modify pvid\n");
				pre_vlan_id = pre_conf.pvid;
				pre_conf.pvid = bssid_conf.pvid;
				ret = npd_capwap_set_bssid_entry(&pre_conf);
				if(pre_vlan_id != DEFAULT_VLAN_ID){
					npd_netif_free_vlan(pre_conf.netif_index, pre_vlan_id, FALSE);
				}
				if(pre_conf.pvid != DEFAULT_VLAN_ID){
					npd_netif_allow_vlan(pre_conf.netif_index, pre_conf.pvid, FALSE, TRUE);
				}
			}
			else{
				return CAPWAP_RETURN_CODE_ENTRY_NOT_EXIST;
			}
			
			if(pre_conf.cflag == TRUE){
				npd_wlan_vlan_t entry;
				
				memset(&entry, 0, sizeof(entry));
				entry.svid_cmd	= FAL_WLAN_VID_CMD_ALL;
				entry.svid		= pre_conf.pvid; 
				ret = nam_wlan_vlan_set(unit_id, &pre_conf.BSS_ID, &entry);
			}
			break;
		}
		default:{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	
	return ret;
}
int npd_capwap_ts_qos_configure(struct npd_mng_capwap *capwap_msg)
{
	int		ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id = 0;
	unsigned int tunnel_id = 0;
	

	switch(capwap_msg->cap_action)
	{
		case CAPWAP_ADD:
			//ret = adpt_music_capwap_tunnel_entry_add(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		case CAPWAP_DEL:
			//ret = adpt_music_capwap_tunnel_entry_del(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		case CAPWAP_MOD:
			//ret = adpt_music_capwap_tunnel_entry_del(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			//ret = adpt_music_capwap_tunnel_entry_add(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		default :
		{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	
	return ret;
}
int npd_capwap_tt_qos_configure(struct npd_mng_capwap *capwap_msg)
{
	int		ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id = 0;
	unsigned int tunnel_id = 0;
	

	switch(capwap_msg->cap_action)
	{
		case CAPWAP_ADD:
			//ret = adpt_music_capwap_tunnel_entry_add(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		case CAPWAP_DEL:
			//ret = adpt_music_capwap_tunnel_entry_del(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		case CAPWAP_MOD:
			//ret = adpt_music_capwap_tunnel_entry_del(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			//ret = adpt_music_capwap_tunnel_entry_add(dev_id, tunnel_id, capwap_msg->u.tunnel_entry);
			break;
		default :
		{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	
	return ret;
}

#define NAM_CAPWAP_FRAGMENT_BUFF_BASE_ADDR	0xFC00000
int npd_capwap_global_configure(struct npd_mng_capwap * capwap_msg)
{
	struct npd_capwap_global_conf_s global_conf;
	int		ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id = 0;
	unsigned int unit_id = 0;

	switch(capwap_msg->cap_action){
		case CAPWAP_ENABLE:
			global_conf.global_status = NPD_TRUE;
			ret =npd_capwap_set_global_conf(0, &global_conf);
			
			/*set port 25 capwap mode & enable capwap global*/
			if(nam_support_capwap(UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, 
				  SYS_LOCAL_MODULE_SLOT_INDEX, 0, 0)))
			{
    			ret = nam_mac25_capwap_csr_en_set(dev_id, DAL_ENABLE);
    			ret = nam_wlan_en_set(unit_id, DAL_ENABLE);
    			ret = nam_capwap_fragment_buff_base_addr_set(dev_id, NAM_CAPWAP_FRAGMENT_BUFF_BASE_ADDR);
                ret = nam_capwap_tunnel_term_ctrl_port_set(dev_id, CAPWAP_TUNNEL_TERM_CTRL_PORT);
                ret = nam_capwap_tunnel_term_data_port_set(dev_id, CAPWAP_TUNNEL_TERM_DATA_PORT);
			}
			break;
		case CAPWAP_DISABLE:
			global_conf.global_status = NPD_FALSE;
			ret =npd_capwap_set_global_conf(0, &global_conf);
			
			/*set port 25 non-capwap mode & disable capwap global*/
			ret = nam_mac25_capwap_csr_en_set(dev_id, DAL_DISABLE);
			ret = nam_wlan_en_set(unit_id, DAL_DISABLE);
            ret = nam_capwap_tunnel_term_ctrl_port_set(dev_id, 0);
            ret = nam_capwap_tunnel_term_data_port_set(dev_id, 0);            
			break;
		default :{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	
	return ret;
}

int npd_capwap_l3_int_configure(struct npd_mng_capwap * capwap_msg)
{
	int	ret 					= CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id			= 0;
	unsigned int unit_id 		= 0;
	unsigned char mac_addr[MAC_LEN];
	unsigned short vlan_id 		= 0;
	unsigned int netif_index 	= 0;
	struct npd_capwap_L3_conf_s L3_conf;
	

	memset(&L3_conf, 0, sizeof(L3_conf));
	
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	memcpy(L3_conf.L3_ifname, capwap_msg->u.capwap_l3_conf.l3_interface_name, sizeof(L3_conf.L3_ifname)-1);
	
	ret = npd_system_get_basemac(mac_addr, MAC_LEN);

	ret = npd_intf_netif_get_by_name(&netif_index, L3_conf.L3_ifname);
	if(NPD_FALSE == ret )
	{
		syslog_ax_asd_err("npd can not get netif index by interface name\r\n");
		return CAPWAP_RETURN_CODE_GET_NETIF_ERR;
	}
	vlan_id = npd_netif_vlan_get_vid(netif_index);
	
	L3_conf.ip_addr = capwap_msg->u.capwap_l3_conf.ip_addr;
	switch(capwap_msg->cap_action){
		case CAPWAP_ENABLE:{
			/*need to add port 25 to the vlan which the interface belongs to
			   and disable ufdb auto learning, set the capwap sip with the ip address of the interface,
			   set capwap SA and DA with system mac*/
			L3_conf.L3_status = TRUE;
			ret =npd_capwap_set_L3_conf(0, &L3_conf);
			
			ret = npd_capwap_keep_capwap_port_vlan_relation(vlan_id, CAPWAP_ADD);
			ret = nam_set_port25_local_switch_status(unit_id, DAL_ENABLE);
			ret = nam_set_port25_autolearn_status(unit_id, DAL_DISABLE);
			ret = nam_capwap_tunnel_local_ip4_set(unit_id, &(L3_conf.ip_addr));
			ret = nam_mac_da_set(dev_id, mac_addr);
			ret = nam_mac_sa_set(dev_id, mac_addr);
			ret = nam_capwap_tunnel_uc_vid_set(dev_id, vlan_id);
			ret = nam_capwap_tunnel_local_ttl_set(unit_id, CAPWAP_TTL_DEFAULT_VALUE);
			break;
		}
		case CAPWAP_DISABLE:{
			L3_conf.L3_status = FALSE;
			ret =npd_capwap_set_L3_conf(0, &L3_conf);
			
			ret = npd_capwap_keep_capwap_port_vlan_relation(vlan_id, CAPWAP_DEL);
			ret = nam_set_port25_local_switch_status(unit_id, DAL_DISABLE);
			ret = nam_set_port25_autolearn_status(unit_id, DAL_ENABLE);
			ret = nam_capwap_tunnel_local_ip4_set(unit_id, &(L3_conf.ip_addr));
			memset(mac_addr, 0, MAC_ADDR_LEN);
			ret = nam_mac_da_set(dev_id, mac_addr);
			ret = nam_mac_sa_set(dev_id, mac_addr);
			ret = nam_capwap_tunnel_local_ttl_set(unit_id, 0);
			break;
		}
		default:{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	return ret;
}


int npd_capwap_wlan_configure(struct npd_mng_capwap * capwap_msg)
{
	unsigned short	join_vlan	= capwap_msg->u.capwap_port_pvid_conf.join_pvid;
	unsigned short	leave_vlan	= capwap_msg->u.capwap_port_pvid_conf.leave_pvid;
	unsigned int	unit_id		= 0;
	int	ret 					= CAPWAP_RETURN_CODE_SUCCESS;

	
	switch(capwap_msg->cap_action){
		case CAPWAP_WLAN_SET_PVID:{
			if(join_vlan != 0){
				ret = npd_capwap_keep_capwap_port_vlan_relation(join_vlan, CAPWAP_ADD);
			}
			if(leave_vlan !=0){
				ret = npd_capwap_keep_capwap_port_vlan_relation(leave_vlan, CAPWAP_DEL);
			}
			break;
		}
		default:{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	return ret;
}

#define MAX_AP_NUM									24

#define WIFI_INDICATOR_OFF							1
#define WIFI_INDICATOR_LIGHT						2
#define WIFI_INDICATOR_1HZ							3
#define	WIFI_INDICATOR_2HZ							4
#define WIFI_INDICATOR_5HZ							5
#define WIFI_INDICATOR_10HZ							6
#define WIFI_INDICATOR_HALF_HZ						7


int npd_wifi_indicator_op(unsigned int led_index, unsigned char mode)
{
	wled8713_port_args args;
	unsigned int ret = 0;

	
	memset(&args, 0, sizeof(args));
	args.led_mode = mode;
	args.led_index= led_index-1;
	ret = nbm_wireless_led_operate(&args);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to change wifi indicator mode\r\n", __func__);
		return -1;
	}
	return 0;
}


int npd_capwap_ap_online_check(unsigned int ap_id, unsigned char *flag)
{
	struct capwap_db_tunnel_entry_s tunnel_conf;
	int ret;
	
	ret = npd_capwap_get_tunnel_entry_by_id(ap_id, &tunnel_conf);
	if(ret != 0){
		syslog_ax_asd_dbg("%s ap online\r\n", __func__);
		*flag = NPD_FALSE;
	}
	else{
		syslog_ax_asd_dbg("%s ap offline\r\n", __func__);
		*flag = NPD_TRUE;
	}

	return 0;
}

int npd_wifi_indicator_ap_mode_init(unsigned int ap_id)
{
	unsigned int	ap_index;
	unsigned int	station_num;
	unsigned int	station_threshold;
	unsigned char	ap_online = NPD_FALSE;
	unsigned int	ret = 0;


	ret = npd_capwap_wifi_indicator_sta_threshold_get(&station_threshold);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get station threshold\r\n", __func__);
		return -1;
	}
	
	for(ap_index=0; ap_index<MAX_AP_NUM; ap_index++){
		ap_online = NPD_FALSE;
		
		ret = npd_capwap_ap_online_check(ap_index, &ap_online);
		if(ret != 0){
			syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
			return -1;
		}

		if(NPD_FALSE == ap_online){
			syslog_ax_asd_dbg("%s wifi indicator off\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_OFF);	
			continue;
		}
		
		ret = npd_asd_get_station_on_ap(ap_index, &station_num);
		if(ret != 0){
			syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
			return -1;
		}

		if(0 == station_num){
			syslog_ax_asd_dbg("%s wifi indicator light\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_LIGHT);
		}
		else if(station_num < station_threshold){
			syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_1HZ);
		}
		else{
			syslog_ax_asd_dbg("%s wifi indicator 5HZ\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_5HZ);
		}
		
	}
	return 0;
}

int npd_wifi_indicator_ap_mode_modify_station_threshold(unsigned int ap_id)
{
	unsigned int	ap_index;
	unsigned int	station_num;
	unsigned int	station_threshold;
	unsigned char	ap_online = NPD_FALSE;
	unsigned int	ret = 0;


	ret = npd_capwap_wifi_indicator_sta_threshold_get(&station_threshold);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get station threshold\r\n", __func__);
		return -1;
	}
	
	for(ap_index=0; ap_index<MAX_AP_NUM; ap_index++){
		ap_online = NPD_FALSE;
		
		ret = npd_capwap_ap_online_check(ap_index, &ap_online);
		if(ret != 0){
			syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
			return -1;
		}

		if(NPD_FALSE == ap_online){
			syslog_ax_asd_dbg("%s wifi indicator off\r\n", __func__);
			continue;
		}
		
		ret = npd_asd_get_station_on_ap(ap_index, &station_num);
		if(ret != 0){
			syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
			return -1;
		}

		if(0 == station_num){
			syslog_ax_asd_dbg("%s wifi indicator light\r\n", __func__);
		}
		else if(station_num < station_threshold){
			syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_1HZ);
		}
		else{
			syslog_ax_asd_dbg("%s wifi indicator 5HZ\r\n", __func__);
			npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_5HZ);
		}
		
	}
	return 0;
}

int npd_wifi_indicator_ap_mode_ap_login(unsigned int ap_id, void *argv)
{
	unsigned int	ret = 0;
	unsigned int	ap_index;


	syslog_ax_asd_dbg("%s wifi indicator on\r\n", __func__);
	
	ap_index = (unsigned int)argv;
	ret = npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_LIGHT);	
	if(ret != 0){
		syslog_ax_asd_dbg("%s fail to light wifi indicator on\r\n", __func__);
		return -1;
	}
	
	return 0;
}

int npd_wifi_indicator_ap_mode_ap_logout(unsigned int ap_id, void *argv)
{
	unsigned int	ret = 0;
	unsigned int	ap_index;


	syslog_ax_asd_dbg("%s wifi indicator off\r\n", __func__);

	ap_index = (unsigned int)argv;
	ret = npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_OFF);	
	if(ret != 0){
		syslog_ax_asd_dbg("%s fail to light wifi indicator off\r\n", __func__);
		return -1;
	}
	
	return 0;
}

int npd_wifi_indicator_ap_mode_station_login(unsigned int ap_id, void *argv)
{
	unsigned int	station_num;
	unsigned int	station_threshold;
	unsigned char	ap_online = NPD_FALSE;
	unsigned int	ap_index;
	unsigned int	ret = 0;


	ret = npd_capwap_wifi_indicator_sta_threshold_get(&station_threshold);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get station threshold\r\n", __func__);
		return -1;
	}
	
	ap_index = (unsigned int)argv;
	ret = npd_asd_get_station_on_ap(ap_index, &station_num);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
		return -1;
	}

	if(station_num == 1){
		syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
		npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_1HZ);
	}
	else if(station_num == station_threshold){
		syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
		npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_5HZ);
	}
	
	return 0;
}

int npd_wifi_indicator_ap_mode_station_logout(unsigned int ap_id, void *argv)
{
	unsigned int	station_num;
	unsigned int	station_threshold;
	unsigned char	ap_online = NPD_FALSE;
	unsigned int	ap_index;
	unsigned int	ret = 0;


	ret = npd_capwap_wifi_indicator_sta_threshold_get(&station_threshold);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get station threshold\r\n", __func__);
		return -1;
	}

	ap_index = (unsigned int)argv;
	ret = npd_asd_get_station_on_ap(ap_index, &station_num);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get ap on line flag\r\n", __func__);
		return -1;
	}
	
	if(station_num == 0){
		syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
		npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_LIGHT);
	}
	else if(station_num == station_threshold-1){
		syslog_ax_asd_dbg("%s wifi indicator 1HZ\r\n", __func__);
		npd_wifi_indicator_op(ap_index, WIFI_INDICATOR_1HZ);
	}
	
	return 0;
}
int npd_wifi_indicator_ap_mode_station_arp_add(unsigned int ap_id)
{
	return 0;
}
int npd_wifi_indicator_ap_mode_station_arp_del(unsigned int ap_id)
{
	return 0;
}
int npd_wifi_indicator_ap_mode_station_throughput_check(unsigned int ap_id)
{
	return 0;
}

const struct wifi_indicator_ops wifi_indicator_ap_mode_ops = {
	.init 						= npd_wifi_indicator_ap_mode_init,
	.modify_station_threshold 	= npd_wifi_indicator_ap_mode_modify_station_threshold,
	.ap_login 					= npd_wifi_indicator_ap_mode_ap_login,
	.ap_logout 					= npd_wifi_indicator_ap_mode_ap_logout,
	.station_login 				= npd_wifi_indicator_ap_mode_station_login,
	.station_logout 			= npd_wifi_indicator_ap_mode_station_logout,
	.station_arp_add	 		= npd_wifi_indicator_ap_mode_station_arp_add,
	.station_arp_del 			= npd_wifi_indicator_ap_mode_station_arp_del,
	.throughput_check 			= npd_wifi_indicator_ap_mode_station_throughput_check,
};
//192.168.1.1~192.168.1.24
wifi_indicator_ip_status_t wifi_indicator_ip_array[24] = {
	{0xC0A80101, NPD_FALSE}, {0xC0A80102, NPD_FALSE}, {0xC0A80103, NPD_FALSE}, {0xC0A80104, NPD_FALSE},
	{0xC0A80105, NPD_FALSE}, {0xC0A80106, NPD_FALSE}, {0xC0A80107, NPD_FALSE}, {0xC0A80108, NPD_FALSE},
	{0xC0A80109, NPD_FALSE}, {0xC0A8010A, NPD_FALSE}, {0xC0A8010B, NPD_FALSE}, {0xC0A8010C, NPD_FALSE},
	{0xC0A8010D, NPD_FALSE}, {0xC0A8010E, NPD_FALSE}, {0xC0A8010F, NPD_FALSE}, {0xC0A80110, NPD_FALSE},
	{0xC0A80111, NPD_FALSE}, {0xC0A80112, NPD_FALSE}, {0xC0A80113, NPD_FALSE}, {0xC0A80114, NPD_FALSE},
	{0xC0A80115, NPD_FALSE}, {0xC0A80116, NPD_FALSE}, {0xC0A80117, NPD_FALSE}, {0xC0A80118, NPD_FALSE}
};
int npd_wifi_indicator_station_mode_init(unsigned int ap_id)
{
    struct arp_snooping_item_s          dbItem;
	struct npd_asd_ufdb_s 	ufdb_entry;
    unsigned int	ip_index;
    unsigned int	netif_type;
    unsigned int	ap_index;
    int ret;

    for(ip_index = 0; ip_index <ELEMENT_NUM(wifi_indicator_ip_array); ip_index++){
    	wifi_indicator_ip_array[ip_index].status = NPD_FALSE;
		ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_OFF);
		syslog_ax_asd_dbg("%s wifi indicator light off result: %d\n", __func__, ret);
    	
    	ret = npd_arp_snooping_find_item_byip(wifi_indicator_ip_array[ip_index].ip_addr, &dbItem);
    	if(ret != 0){
			syslog_ax_asd_dbg("%s arp snooping search result: %d\n", __func__, ret);
    		continue;
    	}

		memset(&ufdb_entry, 0, sizeof(ufdb_entry));
		memcpy(ufdb_entry.mac, dbItem.mac, MAC_ADDR_LEN);
		ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
		if(ret != 0){
			syslog_ax_asd_dbg("%s ufdb entry search result: %d\n", __func__, ret);
			continue;
		}
		
    	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
    	if(ap_index != ap_id){
			syslog_ax_asd_dbg("%s Wifi netif type: %d\n", __func__, netif_type);
    		continue;
    	}
    	
		ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_LIGHT);
    	wifi_indicator_ip_array[ip_index].status = NPD_TRUE;
		syslog_ax_asd_dbg("%s wifi indicator light on result: %d\n", __func__, ret);

    }
	
	return 0;
}
int npd_wifi_indicator_station_mode_modify_station_threshold(unsigned int ap_id)
{
	return 0;
}
int npd_wifi_indicator_station_mode_ap_login(unsigned int ap_id, void *argv)
{
	return 0;
}
int npd_wifi_indicator_station_mode_ap_logout(unsigned int ap_id, void *argv)
{
	return 0;
}
int npd_wifi_indicator_station_mode_station_login(unsigned int ap_id, void *argv)
{
	return 0;
}
int npd_wifi_indicator_station_mode_station_logout(unsigned int ap_id, void *argv)
{
	return 0;
}
int npd_wifi_indicator_station_mode_station_arp_add(unsigned int ap_id, void *argv)
{
    struct arp_snooping_item_s          *dbItem;
	struct npd_asd_ufdb_s 	ufdb_entry;
    unsigned int	ip_index;
    unsigned int	ap_index;
    unsigned int	station_ip;
    int ret;


    dbItem = (struct arp_snooping_item_s*)argv;
    
	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	memcpy(ufdb_entry.mac, dbItem->mac, MAC_ADDR_LEN);
	ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
	if(ret != 0){
		syslog_ax_asd_dbg("%s ufdb entry search result: %d\n", __func__, ret);
		return CAPWAP_RETURN_CODE_SUCCESS;
	}
    
	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
	if(ap_index != ap_id){
		syslog_ax_asd_dbg("%s ap index: %d\n", __func__, ap_index);
		return CAPWAP_RETURN_CODE_SUCCESS;
	}
	
    for(ip_index = 0; ip_index <ELEMENT_NUM(wifi_indicator_ip_array); ip_index++){
		if(wifi_indicator_ip_array[ip_index].ip_addr == dbItem->ipAddr){	    	
			ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_LIGHT);
	    	wifi_indicator_ip_array[ip_index].status = NPD_TRUE;
			syslog_ax_asd_dbg("%s wifi indicator light on result: %d\n", __func__, ret);	
			break;
		}
    }
	
	return CAPWAP_RETURN_CODE_SUCCESS;
}

int npd_wifi_indicator_station_mode_station_arp_del(unsigned int ap_id, void *argv)
{
    struct arp_snooping_item_s          *dbItem;
	struct npd_asd_ufdb_s 	ufdb_entry;
    unsigned int	ip_index;
    unsigned int	ap_index;
    unsigned int	station_ip;
    int ret;


    dbItem = (struct arp_snooping_item_s*)argv;
    
	memset(&ufdb_entry, 0, sizeof(ufdb_entry));
	memcpy(ufdb_entry.mac, dbItem->mac, MAC_ADDR_LEN);
	ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
	if(ret != 0){
		syslog_ax_asd_dbg("%s ufdb entry search result: %d\n", __func__, ret);
		return CAPWAP_RETURN_CODE_SUCCESS;
	}
    
	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
	if(ap_index != ap_id){
		syslog_ax_asd_dbg("%s ap index: %d\n", __func__, ap_index);
		return CAPWAP_RETURN_CODE_SUCCESS;
	}
	
    for(ip_index = 0; ip_index <ELEMENT_NUM(wifi_indicator_ip_array); ip_index++){
		if(wifi_indicator_ip_array[ip_index].ip_addr == dbItem->ipAddr){	    	
			ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_OFF);
	    	wifi_indicator_ip_array[ip_index].status = NPD_FALSE;
			syslog_ax_asd_dbg("%s wifi indicator light off result: %d\n", __func__, ret);	
			break;
		}
    }
	
	return CAPWAP_RETURN_CODE_SUCCESS;
}

int npd_wifi_indicator_station_mode_station_throughput_check(unsigned int ap_id)
{
    unsigned int ret = 0;
    unsigned int op_ret = 0;
    unsigned int	ip_index;
    unsigned int	ap_index;
    unsigned int	throughput_level;
    struct arp_snooping_item_s          dbItem;
	struct npd_asd_ufdb_s 	ufdb_entry;

    
    for(ip_index = 0; ip_index <ELEMENT_NUM(wifi_indicator_ip_array); ip_index++){
    	if(wifi_indicator_ip_array[ip_index].status == NPD_FALSE){
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
    		continue;
    	}

    	ret = npd_arp_snooping_find_item_byip(wifi_indicator_ip_array[ip_index].ip_addr, &dbItem);
    	if(ret != 0){
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
    		continue;
    	}

		memset(&ufdb_entry, 0, sizeof(ufdb_entry));
		memcpy(ufdb_entry.mac, dbItem.mac, MAC_ADDR_LEN);
		ret = npd_asd_db_get_ufdb_entry(&ufdb_entry);
		if(ret != 0){
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
			continue;
		}
		
    	ap_index = npd_netif_wtpid_get(ufdb_entry.netif_index);
    	if(ap_index != ap_id){
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
    		continue;
    	}
    	
    	ret =nam_qos_throughput_get_by_index(ip_index+1, &throughput_level);
    	if(ret != 0){
			syslog_ax_asd_dbg("func:%s result: %d\n", __func__, ret);
    		continue;
    	}
    	
        if(throughput_level == WIFI_INDICATOR_LEVEL_LIGHT){
            ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_LIGHT);
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
        }
        else if(throughput_level == WIFI_INDICATOR_LEVEL_SLOW)
        {
            ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_1HZ);
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
        }
        else if(throughput_level == WIFI_INDICATOR_LEVEL_FAST)
        {
            ret = npd_wifi_indicator_op(ip_index+1, WIFI_INDICATOR_5HZ);
			syslog_ax_asd_dbg("func:%s line: %d\n", __func__, __LINE__);
        }
        nam_qos_throughput_reset_by_index(ip_index+1);

    }
	return 0;
}

const struct wifi_indicator_ops wifi_indicator_station_mode_ops = {
	.init 						= npd_wifi_indicator_station_mode_init,
	.modify_station_threshold	= npd_wifi_indicator_station_mode_modify_station_threshold,
	.ap_login 					= npd_wifi_indicator_station_mode_ap_login,
	.ap_logout 					= npd_wifi_indicator_station_mode_ap_logout,
	.station_login 				= npd_wifi_indicator_station_mode_station_login,
	.station_logout			 	= npd_wifi_indicator_station_mode_station_logout,
	.station_arp_add 			= npd_wifi_indicator_station_mode_station_arp_add,
	.station_arp_del 			= npd_wifi_indicator_station_mode_station_arp_del,
	.throughput_check 			= npd_wifi_indicator_station_mode_station_throughput_check,
};
 
 
static struct wifi_indicator_ops *wifi_indicator_drivers[] =
{
	&wifi_indicator_ap_mode_ops,
	&wifi_indicator_station_mode_ops
};

int npd_capwap_wifi_indicator_mode_set(unsigned char mode, unsigned int ap_id)
{
	struct npd_capwap_L3_conf_s L3_conf;
	int ret = 0;
	

	memset(&L3_conf, 0, sizeof(L3_conf));	
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("npd capwap fail to get L3_conf\r\n");
		return -1;
	}

	L3_conf.wifi_indicator_mode = mode;
	L3_conf.ap_id = ap_id;
	
	ret = npd_capwap_set_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	syslog_ax_asd_dbg("%s wifi indicator mode switch result: %d\n", __func__, ret);

	return ret;
}

int npd_capwap_wifi_indicator_mode_get(unsigned char *mode, unsigned int *ap_id)
{
	struct npd_capwap_L3_conf_s L3_conf;
	int ret = 0;
	

	memset(&L3_conf, 0, sizeof(L3_conf));	
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("npd capwap fail to get L3_conf\r\n");
		return -1;
	}

	*mode = L3_conf.wifi_indicator_mode;
	*ap_id = L3_conf.ap_id;

	return 0;
}

int npd_capwap_wifi_indicator_mode_configure(struct npd_mng_capwap * capwap_msg)
{
	unsigned char 				indicator_mode = capwap_msg->u.wifi_indicator_conf.indicator_mode;
	unsigned int				ap_id = capwap_msg->u.wifi_indicator_conf.ap_id;
	int	ret 					= CAPWAP_RETURN_CODE_SUCCESS;

	ret = npd_capwap_wifi_indicator_mode_set(indicator_mode, ap_id);
	if(ret != 0){
		syslog_ax_asd_err("npd capwap fail to get L3_conf\r\n");
		return -1;
	}
	ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_SWITCH_MODE, NULL);
	syslog_ax_asd_dbg("%s Wifi event process result: %d\n", __func__, ret);

	return ret;
}

int npd_capwap_wifi_indicator_sta_threshold_set(unsigned int station_threshold)
{
	struct npd_capwap_L3_conf_s L3_conf;
	int ret = 0;
	

	memset(&L3_conf, 0, sizeof(L3_conf));	
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get L3_conf\r\n", __func__);
		return -1;
	}

	L3_conf.station_threshold = station_threshold;
	
	ret = npd_capwap_set_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	syslog_ax_asd_dbg("%s wifi indicator station threshold config result: %d\n", __func__, ret);

	return ret;
}

int npd_capwap_wifi_indicator_sta_threshold_get(unsigned int *station_threshold)
{
	struct npd_capwap_L3_conf_s L3_conf;
	int ret = 0;
	

	memset(&L3_conf, 0, sizeof(L3_conf));	
	ret = npd_capwap_get_L3_conf(NPD_CAPWAP_L3_CONF_ID, &L3_conf);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get L3_conf\r\n", __func__);
		return -1;
	}

	*station_threshold = L3_conf.station_threshold;

	return 0;
}


int npd_capwap_wifi_indicator_sta_threshold_configure(struct npd_mng_capwap * capwap_msg)
{
	unsigned char				indicator_mode = 0;
	unsigned int				ap_id = 0;
	unsigned int 				station_threshold = 0;
	int	ret 					= CAPWAP_RETURN_CODE_SUCCESS;

	
	station_threshold = capwap_msg->u.wifi_indicator_conf.station_threshold;
	ret = npd_capwap_wifi_indicator_sta_threshold_set(station_threshold);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to set station threshold\r\n", __func__);
		return -1;
	}
	
	ret = npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_MODIFY_STATION_THRESHOLD, NULL);
	syslog_ax_asd_dbg("%s Wifi event process result: %d\n", __func__, ret);

	return ret;
}
 
int npd_capwap_wifi_indicator_event_process(unsigned int wifi_event, void *argv)
{
	unsigned char	indicator_mode;
	unsigned int	ap_index;
	unsigned int	ret;
	

	ret = npd_capwap_wifi_indicator_mode_get(&indicator_mode, &ap_index);
	if(ret != 0){
		syslog_ax_asd_err("%s fail to get indicator mode\n", __func__);
		return CAPWAP_RETURN_CODE_ERROR;
	}

	syslog_ax_asd_dbg("wifi indicator mode %d index %d\n", indicator_mode, ap_index);
	
	switch(wifi_event){
		case WIFI_INDICATOR_EVENT_SWITCH_MODE:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->init(ap_index);
			break;
		}
		case WIFI_INDICATOR_EVENT_MODIFY_STATION_THRESHOLD:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->modify_station_threshold(ap_index);
			break;
		}
		case WIFI_INDICATOR_EVENT_AP_LOGIN:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->ap_login(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_AP_LOGOUT:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->ap_logout(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_STA_LOGIN:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->station_login(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_STA_LOGOUT:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->station_logout(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_STA_ARP_ADD:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->station_arp_add(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_STA_ARP_DEL:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->station_arp_del(ap_index, argv);
			break;
		}
		case WIFI_INDICATOR_EVENT_THROUGHPUT_CHECK:{
			syslog_ax_asd_dbg("func:%s event:%d \n", __func__, wifi_event);
			ret = wifi_indicator_drivers[indicator_mode]->throughput_check(ap_index);
			break;
		}
		default :{
			syslog_ax_asd_dbg("func:%s Unknown event:%d \n", __func__, wifi_event);
			ret = -1;
			break;
		}
	}

	if(ret != 0){
		syslog_ax_asd_dbg("%s Wifi event process result: %d\n", __func__, ret);
		return -1;
	}

	return CAPWAP_RETURN_CODE_SUCCESS;

}



void *npd_capwap_wifi_indicator_control_thread()
{
    npd_init_tell_whoami("SysLEDShow", 0);
    
    while(1)
    {
        npd_capwap_wifi_indicator_event_process(WIFI_INDICATOR_EVENT_THROUGHPUT_CHECK, NULL);
        sleep(5);
    }    

	return NULL;
}



int npd_capwap_ap_auto_detect_configure(struct npd_mng_capwap * capwap_msg)
{
	int	ret 					= CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int dev_id			= 0;
	unsigned int unit_id 		= 0;
	
	switch(capwap_msg->cap_action){
		case CAPWAP_ENABLE:{
			ret = ref_vlan_acl_dhcp_to_cpu(DAL_ENABLE);
			break;
		}
		case CAPWAP_DISABLE:{
			ret = ref_vlan_acl_dhcp_to_cpu(DAL_DISABLE);
			break;
		}
		default :{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	return ret;
}



/*******************************************************************************
 * npd_capwap_recvmsg_proc
 *
 * DESCRIPTION:
 *		 config dev according to running result of asd protocol
 *
 * INPUTS:
 * 		asd_msg - CAPWAP module notify message
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		NPD_CAPWAP_RETURN_CODE_SUCCESS - config success
 *
 * COMMENTS:
 *  
 **
 ********************************************************************************/
unsigned int npd_capwap_recvmsg_proc(struct npd_mng_capwap * capwap_msg)
{
	unsigned int ret = CAPWAP_RETURN_CODE_SUCCESS;
	unsigned int ifIndex = 0;
	unsigned short vlanId = 0;

	syslog_ax_asd_dbg("npd receive message from capwap:get type: %d, action %d\n",
						capwap_msg->msg_type, capwap_msg->cap_action);


	switch(capwap_msg->msg_type)
	{
		case CAPWAP_TUNNEL:
			ret = npd_capwap_tunnel_configure(capwap_msg);
			break;
		case CAPWAP_BSSID:
			ret = npd_capwap_bssid_configure(capwap_msg);
			break;
		case CAPWAP_TS_QOS:
			ret = npd_capwap_ts_qos_configure(capwap_msg);
			break;
		case CAPWAP_TT_QOS:
			ret = npd_capwap_tt_qos_configure(capwap_msg);
			break;
		case CAPWAP_GLOBAL_CONF:
			ret = npd_capwap_global_configure(capwap_msg);
			break;
		case CAPWAP_L3_INT_CONF:
			ret = npd_capwap_l3_int_configure(capwap_msg);
			break;
		case CAPWAP_WLAN:
			ret = npd_capwap_wlan_configure(capwap_msg);
			break;
		case CAPWAP_WIFI_INDICATOR_MODE_CONF:
			ret = npd_capwap_wifi_indicator_mode_configure(capwap_msg);
			break;
		case CAPWAP_WIFI_INDICATOR_STA_THRESHOLD:
			ret = npd_capwap_wifi_indicator_sta_threshold_configure(capwap_msg);
			break;
		case CAPWAP_AP_AUTO_DETECT_CONF:
			ret = npd_capwap_ap_auto_detect_configure(capwap_msg);
			break;
		default :{
			syslog_ax_asd_err("npd can not proccess the running result of Protocol");
			break;
		}
	}
	return ret;
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
void* npd_capwap_mng_thread_dgram(void)
{
	int ret;
	int sock = 0;
	int recv_len = 0;
	struct npd_mng_capwap *capwap_msg = NULL;
	char buf[CAPWAP_MNG_MAX_SIZE];
	fd_set rfds;
	//npd_log_set_debug_value(SYSLOG_DBG_ALL);//add for debug asd socket,will be removed soon
	/* tell my thread id*/
	npd_init_tell_whoami("CapwapSock",0);

	/*create socket communication*/
	if(ASD_RETURN_CODE_SUCCESS != npd_capwap_sock_init(&sock))
	{
		syslog_ax_asd_err("create asd manage thread socket failed.\r\n");
		return NULL;
	}

	syslog_ax_asd_dbg("create ads manage thread socket fd %d ok\n",sock);
	
	npd_capwap_fd = sock;
	npd_capwap_pid = getpid();
	if(npd_capwap_pid == -1 )
	{
		syslog_ax_asd_err("Cannot get npd capwap thread ID. \r\n");
		return NULL;
	}
	
	capwap_msg = (struct npd_mng_capwap* )buf;
	while(1)
	{
		memset(buf,0,sizeof(char)* CAPWAP_MNG_MAX_SIZE);
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
					ret = npd_capwap_recv_info(capwap_msg,CAPWAP_MNG_MAX_SIZE,&recv_len);					
					if( 0 == recv_len ){
						syslog_ax_asd_err("Capwap:recieve msg len is 0\r\n");
						break;
					}
					
					if(!SYS_LOCAL_MODULE_ISMASTERACTIVE){
						syslog_ax_asd_dbg("The slot is not Active master\r\n");	
						break;
					}

					ret = npd_capwap_recvmsg_proc(capwap_msg);					
				}
		}
		if(0 == recv_len)
			break;
	}
	close(sock);
	return NULL;
}


#ifdef __cplusplus
}
#endif
