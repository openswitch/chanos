/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for logging process.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.41 $	
*******************************************************************************/
#ifdef HAVE_MIRROR
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_mirror.h"


extern hash_table_index_t *npd_fdb_hashmac_index;

int npd_mirror_src_fdb_remove
( 
	unsigned int profile,
	unsigned short  vid,
	unsigned char*  mac,
	unsigned int eth_g_index
);

void npd_mirror_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
	char *private, int len
);


void npd_mirror_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
	char *private, int len
);

void npd_mirror_remote_vlan_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
);


unsigned char npd_mirror_multi_dest = FALSE;

hash_table_index_t *npd_mirror_hashId_index = NULL;
sequence_table_index_t *npd_mirror_seqcfg_index = NULL;

db_table_t         *npd_mirror_dbtbl = NULL;
db_table_t         *npd_mirror_cfgtbl = NULL;

unsigned int* source_port_for_bcm = NULL;

netif_event_notifier_t npd_mirror_netif_notifier =
{
    .netif_event_handle_f = &npd_mirror_notify_event,
    .netif_relate_handle_f = &npd_mirror_relate_event
};

unsigned int npd_mirror_fdb_profile_filter(void *in, void* out)
{
    struct fdb_entry_item_s *item_a = (struct fdb_entry_item_s*)in;
    struct fdb_entry_item_s *item_b = (struct fdb_entry_item_s*)out;

    if(item_a->mirrorProfile == item_b->mirrorProfile)
        return TRUE;
    else
        return FALSE;
}


int npd_mirror_get_profile_node(unsigned int profile, struct npd_mirror_item_s *item )
{
	int status;

	if( item == NULL )
		return MIRROR_RETURN_CODE_ERROR;

	memset(item, 0, sizeof(struct npd_mirror_item_s));
	item->profileId = profile;

	status = dbtable_hash_search( npd_mirror_hashId_index, item, 0, item);
	if( 0 != status ) {
		syslog_ax_mirror_err(" Not found mirror profile %d\n", profile);
		return MIRROR_RETURN_CODE_PROFILE_NOT_CREATED;
	}

	syslog_ax_mirror_dbg("Found mirror profile %d\n", profile);
	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_profile_create
 *
 *	DESCRIPTION:
 * 		This method create mirror profile with global mirror profile id
 *
 *	INPUT:
 *		profileId - global mirror profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE - mirror profile id is out of range
 *		MIRROR_RETURN_CODE_PROFILE_NODE_CREATED - found mirror profile has been created before
 *		MIRROR_RETURN_CODE_MALLOC_FAIL - memory allocation fail when create new mirror profile
 *		MIRROR_RETURN_CODE_SUCCESS - create mirror profile successfully
 *
 **********************************************************************************/
unsigned int npd_mirror_profile_create
(
	unsigned int profileId
)
{

	struct npd_mirror_item_s mirrorItem, dupItem;
	int status;

	if(profileId > MAX_MIRROR_PROFILE) {
		syslog_ax_mirror_err("create mirror profile %d error: profile id out of range\n",profileId);
		return MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE;
	}

	memset( &mirrorItem, 0, sizeof(struct npd_mirror_item_s));
	mirrorItem.profileId = profileId;

	status = dbtable_hash_search( npd_mirror_hashId_index, &mirrorItem, NULL, &dupItem );
	if( 0 == status ) {
		syslog_ax_mirror_err("mirror profile %d exists when new creation!\n",profileId);
		return MIRROR_RETURN_CODE_PROFILE_CREATED;
	}
	
	mirrorItem.in_eth_index = MIRROR_DEST_INPORT_DEFAULT;
	mirrorItem.eg_eth_index = MIRROR_DEST_EGPORT_DEFAULT;

	mirrorItem.in_remote_vid = MIRROR_REMOTE_VLAN_DEFAULT;
	mirrorItem.eg_remote_vid = MIRROR_REMOTE_VLAN_DEFAULT;
	
	status = dbtable_hash_insert( npd_mirror_hashId_index, &mirrorItem );
	if( 0 != status ) {
		syslog_ax_mirror_err("mirror profile %d hash insert fail\n", mirrorItem.profileId);
		return MIRROR_RETURN_CODE_ERROR;
	}

	syslog_ax_mirror_dbg("mirror profile %d create successfully\n", mirrorItem.profileId);
	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_profile_delete
 *
 *	DESCRIPTION:
 * 		This method delete mirror profile with global mirror profile id
 *
 *	INPUT:
 *		profileId - global mirror profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE - mirror profile id is out of range
 *		MIRROR_RETURN_CODE_PROFILE_NODE_CREATED - found mirror profile has been created before
 *		MIRROR_RETURN_CODE_MALLOC_FAIL - memory allocation fail when create new mirror profile
 *		MIRROR_RETURN_CODE_SUCCESS - create mirror profile successfully
 *
 **********************************************************************************/
unsigned int npd_mirror_profile_delete
(
	unsigned int profileId
)
{

	struct npd_mirror_item_s mirrorItem;
	struct fdb_entry_item_s fdbItem;
	int status;

	if(profileId > MAX_MIRROR_PROFILE) {
		syslog_ax_mirror_err("create mirror profile %d error: profile id out of range\n",profileId);
		return MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE;
	}
	
	status = npd_mirror_get_profile_node(profileId, &mirrorItem);
	if( MIRROR_RETURN_CODE_SUCCESS != status ) {
		syslog_ax_mirror_err("mirror profile %d not exist when delete!\n",profileId);
		return status;
	}


	while(0 == status)
	{
		 memset(&fdbItem, 0, sizeof(struct fdb_entry_item_s));
	     fdbItem.mirrorProfile = mirrorItem.profileId;
	     status = dbtable_hash_head(npd_fdb_hashmac_index, &fdbItem,  
	     &fdbItem, npd_mirror_fdb_profile_filter);
	     if(0 != status)
	         break;
	     if(NPD_TRUE != npd_check_vlan_exist(fdbItem.vlanid))
		 {
		     status = MIRROR_RETURN_CODE_VLAN_NOT_EXIST;
	     }
	     if(MIRROR_RETURN_CODE_SUCCESS == status) 
		 {
		     status = npd_mirror_src_fdb_remove(fdbItem.mirrorProfile , fdbItem.vlanid, fdbItem.mac, fdbItem.ifIndex);
	     }
	}	
	status = dbtable_hash_delete( npd_mirror_hashId_index, &mirrorItem, &mirrorItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;
	
	syslog_ax_mirror_dbg("mirror profile %d delete successfully\n", profileId );

	return MIRROR_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 *  npd_mirror_destination_port_check
 *
 *	DESCRIPTION:
 * 		this routine check destination port node
 *
 *	INPUT:
 *		profile -- mirror profile
 *		eth_g_index -- destination port index
 *		direct			--  ingress or egress	
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_destport_direct_check
(
	unsigned int profile,
	unsigned int direct
)
{
	struct npd_mirror_item_s destItem;
	int status;

	if(profile > MAX_MIRROR_PROFILE){
		 return MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE;
	}

	syslog_ax_mirror_dbg("mirror profile check profile %d direct %d\n", profile, direct);

	memset( &destItem, 0, sizeof(struct npd_mirror_item_s));
	status = npd_mirror_get_profile_node(profile, &destItem);
	if( MIRROR_RETURN_CODE_SUCCESS != status )
	{
		return status;
	}
    else {
		if((MIRROR_INGRESS_E == direct)&&(MIRROR_DEST_INPORT_DEFAULT != destItem.in_eth_index)){
			return MIRROR_RETURN_CODE_DEST_PORT_EXIST;
		}
		else if((MIRROR_EGRESS_E == direct)&&(MIRROR_DEST_EGPORT_DEFAULT != destItem.eg_eth_index)){
			return MIRROR_RETURN_CODE_DEST_PORT_EXIST;
		}
		else if((MIRROR_BIDIRECTION_E == direct)&&(MIRROR_DEST_EGPORT_DEFAULT != destItem.in_eth_index ||\
														MIRROR_DEST_EGPORT_DEFAULT != destItem.eg_eth_index))
		{
			return MIRROR_RETURN_CODE_DEST_PORT_EXIST;
		}        
	}

	return MIRROR_RETURN_CODE_SUCCESS;

}

int npd_mirror_src_port_direct_check
(
	unsigned int profile,
	unsigned int direct
)
{
	struct npd_mirror_item_s destItem;
	int status;

	if(profile > MAX_MIRROR_PROFILE){
		 return MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE;
	}

	syslog_ax_mirror_dbg("mirror profile check profile %d direct %d\n", profile, direct);

	memset( &destItem, 0, sizeof(struct npd_mirror_item_s));
	status = npd_mirror_get_profile_node(profile, &destItem);
	if( MIRROR_RETURN_CODE_SUCCESS != status )
	{
		return status;
	}
    else {
		if((MIRROR_INGRESS_E == direct)&&(MIRROR_DEST_INPORT_DEFAULT != destItem.in_eth_index)){
			return MIRROR_RETURN_CODE_SUCCESS;
		}
		else if((MIRROR_EGRESS_E == direct)&&(MIRROR_DEST_EGPORT_DEFAULT != destItem.eg_eth_index)){
			return MIRROR_RETURN_CODE_SUCCESS;
		}
		else if((MIRROR_BIDIRECTION_E == direct)&&(destItem.eg_eth_index == destItem.in_eth_index &&\
														MIRROR_DEST_EGPORT_DEFAULT != destItem.in_eth_index))
		{
			return MIRROR_RETURN_CODE_SUCCESS;
		}
        else if((MIRROR_BIDIRECTION_E == direct)&&(destItem.eg_eth_index != destItem.in_eth_index &&\
														MIRROR_DEST_EGPORT_DEFAULT != destItem.in_eth_index) &&\
														MIRROR_DEST_EGPORT_DEFAULT != destItem.eg_eth_index)
        {
            return MIRROR_RETURN_CODE_SUCCESS;
        }
	}

	return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;

}


int npd_mirror_destport_conflict_check(unsigned int profile, unsigned int eth_g_index)
{
	int idx = 0;
	unsigned int dest_port_num;
	struct npd_mirror_item_s destItem;

	if( MIRROR_DEST_INPORT_DEFAULT == eth_g_index )
		return TRUE;

	dest_port_num = eth_port_array_index_from_ifindex(eth_g_index);

	syslog_ax_mirror_dbg("mirror destport check profile %d eth %#x portnum %d\n", profile, eth_g_index, dest_port_num);
	
	for(idx = MIN_MIRROR_PROFILE; idx <= MAX_MIRROR_PROFILE; idx++)
	{
		if( MIRROR_RETURN_CODE_SUCCESS == npd_mirror_get_profile_node(idx, &destItem))
		{
			if( NPD_PBMP_MEMBER(destItem.in_eth_mbr, dest_port_num) ||
				NPD_PBMP_MEMBER(destItem.eg_eth_mbr, dest_port_num) ||
				NPD_PBMP_MEMBER(destItem.bi_eth_mbr, dest_port_num) )
				return FALSE;

			if( destItem.profileId != profile )
			{
				if( destItem.eg_eth_index == eth_g_index || destItem.in_eth_index == eth_g_index )
					return FALSE;
			}
			
		}
	}

	return TRUE;
}

int npd_mirror_srcport_conflict_check(unsigned int profile, unsigned int eth_g_index)
{
	int idx = 0;
	unsigned int src_port_num;
	struct npd_mirror_item_s destItem;

	if( MIRROR_DEST_INPORT_DEFAULT == eth_g_index )
		return TRUE;

	src_port_num = eth_port_array_index_from_ifindex(eth_g_index);

	syslog_ax_mirror_dbg("mirror srcport check profile %d eth %#x portnum %d\n", profile, eth_g_index, src_port_num);

	for(idx = MIN_MIRROR_PROFILE; idx <= MAX_MIRROR_PROFILE; idx++)
	{
		if( MIRROR_RETURN_CODE_SUCCESS == npd_mirror_get_profile_node(idx, &destItem))
		{			
			if(  destItem.in_eth_index == eth_g_index ||destItem.eg_eth_index == eth_g_index )
				return FALSE;
			if( idx != profile && !npd_mirror_multi_dest) 
			{
				if( NPD_PBMP_MEMBER(destItem.in_eth_mbr, src_port_num) ||
					NPD_PBMP_MEMBER(destItem.eg_eth_mbr, src_port_num) ||
					NPD_PBMP_MEMBER(destItem.bi_eth_mbr, src_port_num) )
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}
int npd_mirror_port_exist_check(unsigned int eth_g_index)
{
	int status;
	status = npd_check_ethport_exist(eth_g_index);
    if(FALSE == status)
    {
        status = MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;
		syslog_ax_mirror_err("can not found dest port %d\n", eth_g_index);
        return status;
    }
	return MIRROR_RETURN_CODE_SUCCESS;
}
int npd_mirror_destination_exist_check(unsigned int profile, unsigned int direct,unsigned int eth_g_index)
{
	int status;
    struct npd_mirror_item_s item = { 0 };
	item.profileId = profile;

	status = dbtable_hash_search( npd_mirror_hashId_index, &item, 0, &item);
	if( 0 != status ) {
		syslog_ax_mirror_err(" Not found mirror profile %d\n", profile);
		return MIRROR_RETURN_CODE_PROFILE_NOT_CREATED;
	}
	
	if(MIRROR_INGRESS_E == direct && eth_g_index != item.in_eth_index)
	{
		return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;
	}   
	else if(MIRROR_EGRESS_E == direct && eth_g_index != item.eg_eth_index) 
	{
	    return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;	
	}
	else if(MIRROR_BIDIRECTION_E == direct && (eth_g_index != item.in_eth_index || eth_g_index != item.eg_eth_index)) 
	{
	    return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;
	}

	return MIRROR_RETURN_CODE_SUCCESS;
}
/**********************************************************************************
 *  npd_mirror_destination_port_set
 *
 *	DESCRIPTION:
 * 		this routine set destination port node
 *
 *	INPUT:
 *		profile			-- mirror profile
 *		eth_g_index -- destination port index
 *		direct			--  ingress or egress	
 *
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_DESTINATION_NODE_EXIST
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
int npd_mirror_destination_port_set
(
	unsigned int profile,
	unsigned int eth_g_index,
	MIRROR_DIRECTION_TYPE direct
)
{
   struct npd_mirror_item_s destItem;
   int status;

	if(profile > MAX_MIRROR_PROFILE) {
		return MIRROR_RETURN_CODE_ERROR;
	}
	if (profile > MIRROR_MAX_DEST_PORT) {
		return MIRROR_RETURN_CODE_ACTION_NOT_SUPPORT;
	}

	syslog_ax_mirror_dbg("mirror profile set port in profile %d port 0x%x direct %d\n", profile, eth_g_index, direct);

	memset( &destItem, 0, sizeof(struct npd_mirror_item_s));

	status = npd_mirror_destport_conflict_check(profile, eth_g_index);
	if( FALSE == status )
	{
		return MIRROR_RETURN_CODE_DEST_PORT_CONFLICT;
	}

	status = npd_mirror_get_profile_node( profile, &destItem);
	if( MIRROR_RETURN_CODE_SUCCESS != status ) {
		syslog_ax_mirror_err("Error!No mirror-profile node %d yet!\n", profile);
		 return status;
	}

	if(MIRROR_INGRESS_E == direct && eth_g_index != destItem.in_eth_index)
	{
		if( eth_g_index == MIRROR_DEST_INPORT_DEFAULT )
		{			
			NPD_PBMP_OR(destItem.eg_eth_mbr, destItem.bi_eth_mbr);
			NPD_PBMP_CLEAR(destItem.in_eth_mbr);
			NPD_PBMP_CLEAR(destItem.bi_eth_mbr);
		}
		destItem.in_eth_index = eth_g_index;
	}   
	else if(MIRROR_EGRESS_E == direct && eth_g_index != destItem.eg_eth_index) 
	{
		if( eth_g_index == MIRROR_DEST_INPORT_DEFAULT )
		{			
			NPD_PBMP_OR(destItem.in_eth_mbr, destItem.bi_eth_mbr);
			NPD_PBMP_CLEAR(destItem.eg_eth_mbr);
			NPD_PBMP_CLEAR(destItem.bi_eth_mbr);
		}
		destItem.eg_eth_index = eth_g_index;
	}
	else if(MIRROR_BIDIRECTION_E == direct && (eth_g_index != destItem.in_eth_index || eth_g_index != destItem.eg_eth_index)) 
	{
		if( eth_g_index == MIRROR_DEST_INPORT_DEFAULT )
		{			
			NPD_PBMP_CLEAR(destItem.eg_eth_mbr);
			NPD_PBMP_CLEAR(destItem.in_eth_mbr);
			NPD_PBMP_CLEAR(destItem.bi_eth_mbr);
		}
		
		destItem.in_eth_index = eth_g_index;
		destItem.eg_eth_index = eth_g_index;
	}
	else {
		return (eth_g_index==MIRROR_DEST_INPORT_DEFAULT)?MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST:\
								MIRROR_RETURN_CODE_DEST_PORT_EXIST;
	}
	
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status ) {
		syslog_ax_mirror_dbg("mirror hash update node %d fail\n", destItem.profileId);
		return MIRROR_RETURN_CODE_ERROR;
	}
	
	return MIRROR_RETURN_CODE_SUCCESS;		

}

/**********************************************************************************
 *  npd_mirror_destination_node_member_check
 *
 *	DESCRIPTION:
 * 		this routine check destination node members
 *
 *	INPUT:
 *		profile			-- mirror profile
 *		direct			--  ingress or egress	
 *
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_destination_node_member_check
(
	unsigned int profile,
	unsigned int direct
)
{	
	struct npd_mirror_item_s destItem;
	int status;

	syslog_ax_mirror_dbg("mirror profile check dest node in profile %d direct %d\n", profile, direct);
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status ) {
		syslog_ax_mirror_dbg("mirror profile node %d not exist\n", profile);
		return NPD_FALSE;
	}
	else {
		if(MIRROR_INGRESS_E == direct ||
			MIRROR_BIDIRECTION_E == direct){
			if((MIRROR_INGRESS_E == direct) && !NPD_PBMP_IS_NULL(destItem.in_eth_mbr))
				return NPD_TRUE;
			if (MIRROR_BIDIRECTION_E == direct)
			{
				if (!NPD_PBMP_IS_NULL(destItem.bi_eth_mbr) ||
					!NPD_PBMP_IS_NULL(destItem.in_eth_mbr) ||
					!NPD_PBMP_IS_NULL(destItem.eg_eth_mbr))
				{
					return NPD_TRUE;
				}
			}
				
			if( !NPD_VBMP_IS_NULL(destItem.vlan_mbr))
				return NPD_TRUE;
			if( !NPD_VBMP_IS_NULL(destItem.acl_mbr))
				return NPD_TRUE;
			if( NPD_OK == npd_fdb_check_contain_mirror( profile ) )
			         return NPD_TRUE;					
		}
		else if(MIRROR_EGRESS_E == direct){
             if( !NPD_PBMP_IS_NULL(destItem.eg_eth_mbr))
				return NPD_TRUE;
		}
	}	
	return NPD_FALSE;
}



/**********************************************************************************
 *  npd_mirror_destination_node_port_get
 *
 *	DESCRIPTION:
 * 		this routine check destination node members
 *
 *	INPUT:
 *		profile			-- mirror profile
 *		direct			--  ingress or egress	
 *
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_destination_node_port_get
(
	unsigned int profile,
	unsigned int direct,
	unsigned int *eth_g_index
)
{
	struct npd_mirror_item_s destItem;
	int status;

	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status ) {
		return status;
	}
	else {
        if(MIRROR_BIDIRECTION_E == direct){
           if((destItem.in_eth_index == destItem.eg_eth_index)&&(MIRROR_DEST_INPORT_DEFAULT != destItem.in_eth_index)){
              *eth_g_index  = destItem.in_eth_index;
		   }
		   else {
               return MIRROR_RETURN_CODE_ERROR;
		   }
		}
		else if(MIRROR_INGRESS_E == direct){
           if(MIRROR_DEST_INPORT_DEFAULT != destItem.in_eth_index){
               *eth_g_index  = destItem.in_eth_index;
		   }
		  else {
               return MIRROR_RETURN_CODE_ERROR;
		   }
		}
		else if(MIRROR_EGRESS_E == direct){
           if(MIRROR_DEST_INPORT_DEFAULT != destItem.eg_eth_index){
               *eth_g_index  = destItem.eg_eth_index;
		   }
		  else {
               return MIRROR_RETURN_CODE_ERROR;
		   }
		}
	}

	syslog_ax_mirror_dbg("mirror profile get node %d dest port direct %d Port_index 0x%x\n", profile, direct, *eth_g_index);
	
	return MIRROR_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 *  npd_mirror_profile_config_save
 *
 *	DESCRIPTION:
 * 		this routine save destination port node
 *
 *	INPUT:
 *		showStr -- buf
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
void npd_mirror_profile_config_save
(
	char* showStr
)
{
	char* pos = showStr;
	unsigned int totalLen = 0;
	int i = 0;
	unsigned char enter_node = 0;/* need enter mirror config node or not*/
	char showBuf[2048] = {0},*curPos = NULL;
	unsigned int curLen = 0;
	struct npd_mirror_item_s dbItem;

	if(NULL == showStr)
		return ;

	for (i = 0 ; i <= MAX_MIRROR_PROFILE; i++ ) 
    {
		if( MIRROR_RETURN_CODE_SUCCESS != npd_mirror_get_profile_node( i , &dbItem))
			continue;
		else if( (dbItem.eg_eth_index == MIRROR_DEST_INPORT_DEFAULT) && (dbItem.in_eth_index == MIRROR_DEST_INPORT_DEFAULT) )
			continue;
		
		curPos = showBuf;
		curLen = 0;
		/*next time to enter,reset enter_node */
		enter_node = 0;

		/* bi-directional destination port, Assure destination port is legal!!!*/
		if((dbItem.in_eth_index == dbItem.eg_eth_index) && 
		   (MIRROR_DEST_INPORT_DEFAULT != dbItem.in_eth_index)) 
		{
			enter_node = 1;
            {
                char name[50];
                parse_eth_index_to_name(dbItem.in_eth_index, name);
				curLen += sprintf(curPos," destination-port %s %s\n", name, "bidirection");
				curPos = showBuf + curLen;
			}
		}
		else 
        {/* uni-directional destination port*/
			if(MIRROR_DEST_INPORT_DEFAULT != dbItem.in_eth_index)
			{
                char name[50];
			    enter_node = 1;
                parse_eth_index_to_name(dbItem.in_eth_index, name);
				curLen += sprintf(curPos," destination-port %s %s\n", name, "ingress");
				curPos = showBuf + curLen;
			}
			if(MIRROR_DEST_EGPORT_DEFAULT != dbItem.eg_eth_index)
			{
                char name[50];
    			enter_node = 1;
                parse_eth_index_to_name(dbItem.eg_eth_index, name);
				curLen += sprintf(curPos," destination-port %s %s\n", name, "egress");
				curPos = showBuf + curLen;
			}
		}
		
		if(enter_node) {
			if(0 == i){
				totalLen += sprintf(pos,"mirror-profile \n");
				pos = showStr + totalLen;
			}
			else{
				totalLen += sprintf(pos,"mirror-profile %d\n",i);
				pos = showStr + totalLen;
			}
			totalLen += sprintf(pos,"%s",showBuf);
			pos = showStr + totalLen;
		}
		
		npd_mirror_src_port_cfg_save(&dbItem,&pos,&totalLen,&enter_node);
		
		npd_mirror_src_vlan_cfg_save(&dbItem,&pos,&totalLen,&enter_node);

		npd_mirror_src_fdb_cfg_save(&dbItem,&pos,&totalLen,&enter_node);
#if 0
		npd_mirror_src_policy_cfg_save(&dbItem,&pos,&totalLen,&enter_node);		
#endif
		npd_mirror_remote_vlan_cfg_save(&dbItem,&pos,&totalLen,&enter_node);
			
		if(enter_node) {
			totalLen += sprintf(pos,"exit\n");
			pos = showStr + totalLen;
		}
	}

	return ;
}

/**********************************************************************************
 *  npd_mirror_src_port_check
 *
 *	DESCRIPTION:
 * 		this routine check src port node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		src_eth_g-index  --  source port index
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_port_check
(
	unsigned int profile,
	unsigned int direct,
	unsigned int src_eth_g_index
)
{
	unsigned int src_eth_portNum = 0;
	struct npd_mirror_item_s destItem;
	int status;

	syslog_ax_mirror_dbg("mirror profile check profile %d source port 0x%x direct %d\n", profile, src_eth_g_index, direct);

	src_eth_portNum = eth_port_array_index_from_ifindex(src_eth_g_index);
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status ) {
		return NPD_FALSE;
	}

	if( NPD_PBMP_MEMBER(destItem.bi_eth_mbr, src_eth_portNum) )
	{
		return NPD_TRUE;
	}
	else if( direct == MIRROR_INGRESS_E && NPD_PBMP_MEMBER(destItem.in_eth_mbr, src_eth_portNum) )
	{
		return NPD_TRUE;
	}
	else if( direct == MIRROR_EGRESS_E && NPD_PBMP_MEMBER(destItem.eg_eth_mbr, src_eth_portNum) )
	{
		return NPD_TRUE;
	}

	return NPD_FALSE;
}

/**********************************************************************************
 *  npd_mirror_src_port_get
 *
 *	DESCRIPTION:
 * 		this routine check src port node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		src_eth_g-index  --  source port index
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_port_get
(
	unsigned int profile,
	unsigned int direct,
	unsigned int* src_eth_g_index,
	unsigned int* src_count
)
{
	struct npd_mirror_item_s destItem;
	unsigned int src_eth_portNum;
	npd_pbmp_t *portBmpList;
	int status, i = 0, count= 0 ;	
	
	memset(src_eth_g_index,0,(*src_count)*sizeof(unsigned int));
    *src_count = 0;
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status ) {
		return status;
	}

	portBmpList = &(destItem.in_eth_mbr);
	if( direct == MIRROR_EGRESS_E )
		portBmpList = &(destItem.eg_eth_mbr);

	NPD_PBMP_ITER(*portBmpList,src_eth_portNum)
	{
		if( direct == MIRROR_BIDIRECTION_E )
		{
			if( !NPD_PBMP_MEMBER( destItem.eg_eth_mbr, src_eth_portNum))
				continue;
		}
		src_eth_g_index[i++] = eth_port_array_index_to_ifindex(src_eth_portNum);
		count++;
	}
	*src_count = count;
	syslog_ax_mirror_dbg("mirror profile get source port in profile %d direct %d, port count %d\n", profile, direct, *src_count);
	
	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_port_create
 *
 *	DESCRIPTION:
 * 		this routine create src port node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		src_eth_g-index  --  source port index
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
int npd_mirror_src_port_create
(
	unsigned int profile,
	unsigned int direct,
	unsigned int src_eth_g_index
)
{
	struct npd_mirror_item_s destItem;
	unsigned int src_eth_portNum;
	int status;

	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		 return status;
	}

	status = npd_mirror_src_port_direct_check(profile,direct);
	if(MIRROR_RETURN_CODE_SUCCESS != status )
	{
		return status;
	}

	status = npd_mirror_srcport_conflict_check(profile, src_eth_g_index);
	if( FALSE == status )
	{
		return MIRROR_RETURN_CODE_SRC_PORT_CONFLICT;
	}
	
	if(NPD_TRUE == npd_mirror_src_port_check(profile,direct,src_eth_g_index)) {
 	   return MIRROR_RETURN_CODE_SRC_PORT_EXIST;	/*if has already exist, do nothing:)*/
    }

	src_eth_portNum = eth_port_array_index_from_ifindex(src_eth_g_index);
	
	if( direct == MIRROR_EGRESS_E ) {
		if( NPD_PBMP_MEMBER(destItem.in_eth_mbr, src_eth_portNum) )
		{
			NPD_PBMP_PORT_REMOVE(destItem.in_eth_mbr, src_eth_portNum);
			NPD_PBMP_PORT_ADD(destItem.bi_eth_mbr, src_eth_portNum);
		}
		else {
			NPD_PBMP_PORT_ADD( destItem.eg_eth_mbr, src_eth_portNum);
		}
	}
	else if( direct == MIRROR_INGRESS_E ) {
		if( NPD_PBMP_MEMBER(destItem.eg_eth_mbr, src_eth_portNum) )
		{
			NPD_PBMP_PORT_REMOVE(destItem.eg_eth_mbr, src_eth_portNum);
			NPD_PBMP_PORT_ADD(destItem.bi_eth_mbr, src_eth_portNum);
		}
		else {
			NPD_PBMP_PORT_ADD( destItem.in_eth_mbr, src_eth_portNum);
		}
	}
	else if( direct == MIRROR_BIDIRECTION_E ) {
		NPD_PBMP_PORT_REMOVE(destItem.in_eth_mbr, src_eth_portNum);
		NPD_PBMP_PORT_REMOVE(destItem.eg_eth_mbr, src_eth_portNum);
		NPD_PBMP_PORT_ADD( destItem.bi_eth_mbr, src_eth_portNum);
	}
		
    status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile create source port 0x%x in profile %d direct %d success\n",src_eth_g_index, profile, direct);
	
    return MIRROR_RETURN_CODE_SUCCESS;	
}

/**********************************************************************************
 *  npd_mirror_src_port_remove
 *
 *	DESCRIPTION:
 * 		this routine remov src port node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		src_eth_g-index  --  source port index
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
int npd_mirror_src_port_remove
(
	unsigned int profile,
	unsigned int direct,
	unsigned int src_eth_g_index
)
{
	struct npd_mirror_item_s destItem;
	unsigned int src_eth_portNum;
	int status;

	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		 return status;
	}

	if(NPD_TRUE != npd_mirror_src_port_check(profile,direct,src_eth_g_index)) {
 	   return MIRROR_RETURN_CODE_SRC_PORT_NOTEXIST;	
    }

	src_eth_portNum = eth_port_array_index_from_ifindex(src_eth_g_index);

	if( direct == MIRROR_EGRESS_E ) {
		if( NPD_PBMP_MEMBER(destItem.bi_eth_mbr, src_eth_portNum) )
		{
			NPD_PBMP_PORT_REMOVE(destItem.bi_eth_mbr, src_eth_portNum);
			NPD_PBMP_PORT_ADD(destItem.in_eth_mbr, src_eth_portNum);
		}
		else {
			NPD_PBMP_PORT_REMOVE( destItem.eg_eth_mbr, src_eth_portNum);
		}
	}
	else if( direct == MIRROR_INGRESS_E ) {
		if( NPD_PBMP_MEMBER(destItem.bi_eth_mbr, src_eth_portNum) )
		{
			NPD_PBMP_PORT_REMOVE(destItem.bi_eth_mbr, src_eth_portNum);
			NPD_PBMP_PORT_ADD(destItem.eg_eth_mbr, src_eth_portNum);
		}
		else {
			NPD_PBMP_PORT_REMOVE( destItem.in_eth_mbr, src_eth_portNum);
		}
	}
	else if( direct == MIRROR_BIDIRECTION_E ) {
		NPD_PBMP_PORT_REMOVE(destItem.bi_eth_mbr, src_eth_portNum);
	}
		
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile remove source port 0x%x in profile %d direct %d success\n",src_eth_g_index, profile, direct);

	return MIRROR_RETURN_CODE_SUCCESS;
}


int npd_mirror_port_delete(unsigned int eth_g_index)
{
    int ret = MIRROR_RETURN_CODE_SUCCESS;
    int idx = 0,  bi_direct;
	unsigned int port_num = 0;
	struct npd_mirror_item_s destItem;

	port_num = eth_port_array_index_from_ifindex(eth_g_index);
    if(port_num == NPD_FAIL)
    {
        ret = ETHPORT_RETURN_CODE_ERR_NONE;
        return ret;
    }
	syslog_ax_mirror_dbg("mirror del port profile eth %#x portnum %d\n", eth_g_index, port_num);

	for(idx = MIN_MIRROR_PROFILE; idx <= MAX_MIRROR_PROFILE; idx++)
	{
		if( MIRROR_RETURN_CODE_SUCCESS == npd_mirror_get_profile_node(idx, &destItem) )
		{
			bi_direct = 0;
			
			if( destItem.in_eth_index == destItem.eg_eth_index )
				bi_direct = 1;
			
			if( destItem.in_eth_index == eth_g_index )
			{
				if( bi_direct )
					ret = npd_mirror_destination_port_set( idx, MIRROR_DEST_INPORT_DEFAULT, MIRROR_BIDIRECTION_E);
				else
					ret = npd_mirror_destination_port_set( idx, MIRROR_DEST_INPORT_DEFAULT, MIRROR_INGRESS_E);
			}
			else if(destItem.eg_eth_index == eth_g_index)
			{
				ret = npd_mirror_destination_port_set( idx, MIRROR_DEST_INPORT_DEFAULT, MIRROR_EGRESS_E);
			}
			else if( NPD_PBMP_MEMBER(destItem.in_eth_mbr, port_num) )
			{
				ret = npd_mirror_src_port_remove( idx,MIRROR_INGRESS_E,eth_g_index);
			}
			else if( NPD_PBMP_MEMBER(destItem.eg_eth_mbr, port_num) )
			{
				ret = npd_mirror_src_port_remove( idx,MIRROR_EGRESS_E,eth_g_index);
			}
			else if( NPD_PBMP_MEMBER(destItem.bi_eth_mbr, port_num) )
			{
				ret = npd_mirror_src_port_remove( idx,MIRROR_BIDIRECTION_E,eth_g_index);
			}			
		}
	}
    return ret;
}

/**********************************************************************************
 *  npd_mirror_src_port_cfg_save
 *
 *	DESCRIPTION:
 * 		this routine save all src port configuration
 *
 *	INPUT:
 *		 srcPortList -- Port list head
 *		currentPos -- first no used char addr
 *		totalLen      -- used buf len
 *		entered  - need to enter mirror node or not( 1 - no need re-enter, 0 - nee enter)
 *
 *	OUTPUT:
 *		entered - does mirror node entered or not
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
void npd_mirror_src_port_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int *totalLen,
	unsigned char *entered
)
{
	char* buf = *currentPos;
	unsigned int curLen = 0;
	unsigned int pIndex, eth_g_index = 0;
	unsigned char enter_node = 0;
	
	if((NULL == dbItem) || (NULL == *currentPos)) {
		return ;
	}

	if((*totalLen + curLen + 30) > MIRROR_SAVE_CFG_MEM) {
		*currentPos = buf;
		*totalLen += curLen;
		return ;
	}
	
	if(NULL != dbItem) {
		NPD_PBMP_ITER( dbItem->in_eth_mbr, pIndex )
		{
            char name[50];
            eth_g_index = eth_port_array_index_to_ifindex( pIndex );
            parse_eth_index_to_name(eth_g_index, name);
			{
				if((0 == *entered) && !enter_node) {
					curLen += sprintf(buf, "config mirror-profile\n");
					buf = (*currentPos) + curLen;
					enter_node = 1;
				}
				curLen += sprintf(buf," mirror port-source %s %s\n", name,"ingress");
				buf = (*currentPos) + curLen;
			}				
		}
		NPD_PBMP_ITER( dbItem->eg_eth_mbr, pIndex )
		{
            char name[50];
            eth_g_index = eth_port_array_index_to_ifindex( pIndex );
            parse_eth_index_to_name(eth_g_index, name);
			{
				if((0 == *entered) && !enter_node) {
					curLen += sprintf(buf, "config mirror-profile\n");
					buf = (*currentPos) + curLen;
					enter_node = 1;
				}
				curLen += sprintf(buf," mirror port-source %s %s\n", name, "egress");
				buf = (*currentPos) + curLen;
			}				
		}
		NPD_PBMP_ITER( dbItem->bi_eth_mbr, pIndex )
		{
            char name[50];
            eth_g_index = eth_port_array_index_to_ifindex( pIndex );
            parse_eth_index_to_name(eth_g_index, name);
			{
				if((0 == *entered) && !enter_node) {
					curLen += sprintf(buf, "config mirror-profile\n");
					buf = (*currentPos) + curLen;
					enter_node = 1;
				}
				curLen += sprintf(buf," mirror port-source %s %s\n", name, "bidirection");
				buf = (*currentPos) + curLen;
			}				
		}

		*currentPos = buf;
		*totalLen += curLen;
		if(0 == *entered) { /* not entered before, mark current enter state( current may or may not enter)*/
			*entered = enter_node;
		}
	}
	
	return ;
}

/**********************************************************************************
 *  npd_mirror_src_vlan_check
 *
 *	DESCRIPTION:
 * 		this routine check src vlan node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid  --  vlan id
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_vlan_check
(
	unsigned int profile,
	unsigned short vid
)
{
	struct npd_mirror_item_s destItem;
	int status;

	syslog_ax_mirror_dbg("mirror profile check source vlan %d in profile %d\n", vid, profile );

	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return NPD_FALSE;
	}

	if( NPD_VBMP_MEMBER( destItem.vlan_mbr, vid))
		return NPD_TRUE;

	return NPD_FALSE;
}

/**********************************************************************************
 *  npd_mirror_src_vlan_create
 *
 *	DESCRIPTION:
 * 		this routine create src vlan node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid 						--  vlan id 
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
int npd_mirror_src_vlan_create
(
	unsigned int profile,
	unsigned short vid
)
{	
	struct npd_mirror_item_s destItem;
	int status;

    if(!MIRROR_VLAN)
        return MIRROR_RETURN_CODE_ACTION_NOT_SUPPORT;
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return status;
	}

	status = npd_mirror_destport_direct_check(profile,MIRROR_INGRESS_E);
	if(MIRROR_RETURN_CODE_SUCCESS != status){
		return status;
	}

	if(NPD_TRUE == npd_mirror_src_vlan_check(profile, vid) )
	{
		return MIRROR_RETURN_CODE_SRC_VLAN_EXIST;
	}

	NPD_VBMP_VLAN_ADD( destItem.vlan_mbr, vid);
	
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile create source vlan %d in profile %d success\n", vid, profile );

	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_port_remove
 *
 *	DESCRIPTION:
 * 		this routine remove srv vlan node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid  --  vlan id
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_vlan_remove
(
	unsigned int profile,
	unsigned int vid
)
{
	struct npd_mirror_item_s destItem;
	int status;
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return status;
	}

	if(NPD_TRUE != npd_mirror_src_vlan_check(profile, vid) )
	{
		return MIRROR_RETURN_CODE_SRC_VLAN_NOTEXIST;
	}

	NPD_VBMP_VLAN_REMOVE( destItem.vlan_mbr, vid);
	
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile remove source vlan %d in profile %d success\n", vid, profile );

	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_all_vlan_destroy
 *
 *	DESCRIPTION:
 * 		this routine destroy all src vlan node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_all_vlan_destroy
(
	unsigned int profile
)
{
	struct npd_mirror_item_s destItem;
	int status;
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return status;
	}

	NPD_VBMP_CLEAR( destItem.vlan_mbr);
	
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile remove all source vlan in profile %d success\n", profile );

	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_vlans_cfg_save
 *
 *	DESCRIPTION:
 * 		this routine save all src vlan configuration
 *
 *	INPUT:
 *		srcVlanList -- vlan list head
 *		currentPos -- first no used char addr
 *		totalLen      -- used buf len
 *		entered  - need to enter mirror node or not( 1 - no need re-enter, 0 - nee enter)
 *
 *	OUTPUT:
 *		entered - does mirror node entered or not
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
void npd_mirror_src_vlan_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
)
{
	char* buf = *currentPos;
	unsigned int curLen = 0;
	unsigned char enter_node = 0;
	unsigned int vIndex;

	if((NULL == dbItem) || (NULL == *currentPos))
		return ;

	NPD_VBMP_ITER(dbItem->vlan_mbr, vIndex )
	{		
		if((*totalLen + curLen + 30) > MIRROR_SAVE_CFG_MEM) {
			*currentPos = buf;
			*totalLen += curLen; 
			return ;
		}
		
		if((0 == *entered) && (!enter_node)) {
			curLen += sprintf(buf,"config mirror-profile\n");
			buf = (*currentPos) + curLen;
			enter_node = 1;
		}
		
		curLen += sprintf(buf,"  mirror vlan-source %d\n",vIndex);
		buf = (*currentPos) + curLen;
	}			

	*currentPos = buf;
	*totalLen += curLen;

	if(0 == *entered) { /* not entered before, mark current enter state( current may or may not enter)*/
		*entered = enter_node;
	}
	
	return ;
}

/**********************************************************************************
 *  npd_mirror_src_fdb_check
 *
 *	DESCRIPTION:
 * 		this routine check src fdb
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid  --  vlan id
 *		mac -- ether address
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_fdb_check
(
	unsigned int profile,
	unsigned short vid,
	unsigned char* mac,
	unsigned int eth_g_index
)
{
	
	struct npd_mirror_item_s destItem;
	unsigned int profileId = 0;
	int status;

	syslog_ax_mirror_dbg("mirror profile check source fdb vid %d mac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x port 0x%x in profile %d\n",\
						vid, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],\
						eth_g_index, profile);	
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return NPD_FALSE;
	}

	npd_fdb_check_static_mirror_entry_exist(mac, vid, &profileId);
	if( profile == profileId ) {
		return NPD_TRUE;
	}
	
	return NPD_FALSE;
}

/**********************************************************************************
 *  npd_mirror_src_fdb_create
 *
 *	DESCRIPTION:
 * 		this routine create src fdc  node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid 						--  vlan id 
 *		mac 						--  ether address
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		MIRROR_RETURN_CODE_ERROR
 *		MIRROR_RETURN_CODE_SUCCESS
 *		
 *
 **********************************************************************************/
int npd_mirror_src_fdb_create
(
	unsigned int profile,
	unsigned short vid,
	unsigned char* mac,
	unsigned int eth_g_index
)
{
	unsigned int global_index = 0;
	struct npd_mirror_item_s dbItem;
	int status;

    if(!MIRROR_FDB)
        return MIRROR_RETURN_CODE_ACTION_NOT_SUPPORT;
    
	syslog_ax_mirror_dbg("npd_mirror_src_fdb_create: profile %d vlan %d mac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x port_index 0x%x",\
					profile, vid, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], eth_g_index );
	if((NPD_FDB_ERR_NODE_EXIST == npd_fdb_check_static_entry_exist(mac,vid,&global_index))&&(global_index != eth_g_index)){
		 return MIRROR_RETURN_CODE_SRC_PORT_CONFLICT;
	}
	if(FALSE == npd_mirror_srcport_conflict_check(profile, eth_g_index))
	{
		return MIRROR_RETURN_CODE_SRC_PORT_CONFLICT;
	}
	if(MIRROR_RETURN_CODE_SUCCESS != npd_mirror_get_profile_node(profile, &dbItem)) {
		return MIRROR_RETURN_CODE_PROFILE_NOT_CREATED;
	}

	status = npd_mirror_destport_direct_check(profile,MIRROR_EGRESS_E);
	if(MIRROR_RETURN_CODE_DEST_PORT_EXIST != status)
	{
		return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;
	}

	if( NPD_FALSE != npd_mirror_src_fdb_check( profile, vid, mac, eth_g_index) )
	{
		return MIRROR_RETURN_CODE_SRC_FDB_EXIST;
	}
	
	if(NPD_OK != npd_fdb_static_mirror_entry_add( mac, vid, eth_g_index, profile))
	{
		return MIRROR_RETURN_CODE_ERROR;
	}
    syslog_ax_mirror_dbg("mirror profile create source fdb vid %d mac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x port 0x%x in profile %d\n",\
		vid, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],\
		eth_g_index, profile);	
	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_fdb_remove
 *
 *	DESCRIPTION:
 * 		this routine remove src fdb node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid  --  vlan id
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_src_fdb_remove
( 
	unsigned int profile,
	unsigned short  vid,
	unsigned char*  mac,
	unsigned int eth_g_index
)
{
	struct npd_mirror_item_s dbItem;
	
	if(MIRROR_RETURN_CODE_SUCCESS != npd_mirror_get_profile_node(profile, &dbItem)) {
		return MIRROR_RETURN_CODE_PROFILE_NOT_CREATED;
	}
	if( NPD_FALSE == npd_mirror_src_fdb_check( profile, vid, mac, eth_g_index) )
	{
		return MIRROR_RETURN_CODE_SRC_FDB_NOTEXIST;
	}
	if (NPD_OK != npd_mirror_fdb_source_port_exist_check(vid, mac, eth_g_index, profile))
	{
        return MIRROR_RETURN_CODE_DEST_PORT_NOTEXIST;
	}
    if(NPD_OK != npd_fdb_static_mirror_entry_del( mac, vid ))
    {
		return MIRROR_RETURN_CODE_ERROR;
    }
    syslog_ax_mirror_dbg("mirror profile remove source fdb vid %d mac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x port 0x%x in profile %d\n",\
		vid, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],\
		eth_g_index, profile);	
	return MIRROR_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 *  npd_mirror_src_fdb_cfg_save
 *
 *	DESCRIPTION:
 * 		this routine save src fdb configuration node
 *
 *	INPUT:
 *		srcFdbList -- fdb list head
 *		currentPos -- first no used char addr
 *		totalLen      -- used buf len
 *		entered  - need to enter mirror node or not( 1 - no need re-enter, 0 - nee enter)
 *
 *	OUTPUT:
 *		entered - does mirror node entered or not
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *
 **********************************************************************************/
void npd_mirror_src_fdb_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
)
{
	char* buf = *currentPos;
	unsigned int curLen = 0;
	unsigned char enter_node = 0;
	struct fdb_entry_item_s *staticFdbArray = NULL;	
	int count = 0, i;

	if((NULL == dbItem) || (NULL == *currentPos))
		return ;

	count = npd_fdb_static_mirror_entry_count( dbItem->profileId );
	if( count == 0 )
		return;
	
	staticFdbArray = malloc(count * sizeof(struct fdb_entry_item_s) );
	if( staticFdbArray == NULL )
		return;

	npd_fdb_get_static_mirror_item( staticFdbArray, count, dbItem->profileId);
	for( i=0; i<count; i++) {
		char name[50];
		if((*totalLen + curLen + 50) > MIRROR_SAVE_CFG_MEM) {
			*currentPos = buf;
			*totalLen += curLen;
			free(staticFdbArray);
			return ;
		}

		
		if((0 == *entered) && (!enter_node)) {
			curLen += sprintf(buf,"config mirror-profile\n");
			buf = (*currentPos) + curLen;
			enter_node = 1;
		}

        parse_eth_index_to_name(staticFdbArray[i].ifIndex, name);
		{
			curLen += sprintf(buf,"  mirror mac-source %02x:%02x:%02x:%02x:%02x:%02x %d %s\n",		\
				staticFdbArray[i].mac[0],staticFdbArray[i].mac[1],		\
				staticFdbArray[i].mac[2],staticFdbArray[i].mac[3],		\
				staticFdbArray[i].mac[4],staticFdbArray[i].mac[5],       \
				staticFdbArray[i].vlanid, name);
			buf = *currentPos + curLen;
		}
	}			

	*currentPos = buf;
	*totalLen += curLen;

	if(0 == *entered) { /* not entered before, mark current enter state( current may or may not enter)*/
		*entered = enter_node;
	}
    free(staticFdbArray);
	return ;
}
/**********************************************************************************
 *  npd_mirror_remote_vlan_cfg_save
 *
 *	DESCRIPTION:
 * 		this routine save remote vlan configuration node
 *
 *	INPUT:
 *		dbItem - mirror db item
 *		currentPos - first no used char addr
 *		totalLen      - used buf len
 *		entered  - need to enter mirror node or not( 1 - no need re-enter, 0 - nee enter)
 *
 *	OUTPUT:
 *		entered - does mirror node entered or not
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *
 **********************************************************************************/
void npd_mirror_remote_vlan_cfg_save
(
	struct npd_mirror_item_s *dbItem,
	char** currentPos,
	unsigned int* totalLen,
	unsigned char *entered
)
{
	char* buf = *currentPos;
	unsigned int curLen = 0;

	if((NULL == dbItem) || (NULL == *currentPos))
		return ;

	if(*entered == 0){
		return ;
	}	

	if (MIRROR_REMOTE_VLAN_DEFAULT != dbItem->in_remote_vid)
	{
		curLen += sprintf(*currentPos," remote-vlan %d\n", dbItem->in_remote_vid);
		buf = *currentPos + curLen;
	}
	
	*currentPos = buf;
	*totalLen += curLen;

	return ;
}


/**********************************************************************************
 *  npd_mirror_remote_vlan_check
 *
 *	DESCRIPTION:
 * 		this routine check src vlan node
 *
 *	INPUT:
 *		profile					--  mirror profile
 *		vid  --  vlan id
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NPD_FALSE
 *		NPD_TRUE
 *		
 *
 **********************************************************************************/
int npd_mirror_remote_vlan_exist_check
(
	unsigned int profile,
	unsigned int direct,
	unsigned short vid
)
{
	struct npd_mirror_item_s destItem;
	int status;

	syslog_ax_mirror_dbg("mirror profile check source vlan %d in profile %d\n", vid, profile );

	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return NPD_FALSE;
	}

	if(MIRROR_INGRESS_E == direct && vid != destItem.in_remote_vid)
	{
		return MIRROR_RETURN_CODE_REMOTE_VLAN_NOEXIST;
	}   
	else if(MIRROR_EGRESS_E == direct && vid != destItem.eg_remote_vid) 
	{
	    return MIRROR_RETURN_CODE_REMOTE_VLAN_NOEXIST;	
	}
	else if(MIRROR_BIDIRECTION_E == direct && (vid != destItem.in_remote_vid || vid != destItem.eg_remote_vid)) 
	{
	    return MIRROR_RETURN_CODE_REMOTE_VLAN_NOEXIST;
	}	

	return MIRROR_RETURN_CODE_SUCCESS;
}

int npd_mirror_remote_vlan_conflict_check(unsigned int profile, unsigned short vid)
{
	int idx = 0;
	struct npd_mirror_item_s destItem;

	if( 0 == vid )
		return TRUE;


	syslog_ax_mirror_dbg("mirror destport check profile %d vlan %d\n", profile, vid);
	
	for(idx = MIN_MIRROR_PROFILE; idx <= MAX_MIRROR_PROFILE; idx++)
	{
		if( MIRROR_RETURN_CODE_SUCCESS == npd_mirror_get_profile_node(idx, &destItem))
		{
			if( NPD_VBMP_MEMBER(destItem.vlan_mbr, vid))
				return FALSE;			
		}
	}

	return TRUE;
}

int npd_mirror_remote_vlan_direct_check
(
	unsigned int profile,
	unsigned int direct
)
{
	struct npd_mirror_item_s destItem;
	int status;

	if(profile > MAX_MIRROR_PROFILE){
		 return MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE;
	}

	syslog_ax_mirror_dbg("mirror profile check profile %d direct %d\n", profile, direct);

	memset( &destItem, 0, sizeof(struct npd_mirror_item_s));
	status = npd_mirror_get_profile_node(profile, &destItem);
	if( MIRROR_RETURN_CODE_SUCCESS != status )
	{
		return status;
	}
    else {
		if((MIRROR_INGRESS_E == direct)&&(MIRROR_REMOTE_VLAN_DEFAULT != destItem.in_remote_vid)){
			return MIRROR_RETURN_CODE_REMOTE_VLAN_EXIST;
		}
		else if((MIRROR_EGRESS_E == direct)&&(MIRROR_REMOTE_VLAN_DEFAULT != destItem.eg_remote_vid)){
			return MIRROR_RETURN_CODE_REMOTE_VLAN_EXIST;
		}
		else if((MIRROR_BIDIRECTION_E == direct)&&(MIRROR_REMOTE_VLAN_DEFAULT != destItem.in_remote_vid ||\
														MIRROR_REMOTE_VLAN_DEFAULT != destItem.eg_remote_vid))
		{
			return MIRROR_RETURN_CODE_REMOTE_VLAN_EXIST;
		}        
	}

	return MIRROR_RETURN_CODE_SUCCESS;

}

int npd_mirror_remote_vlan_set
(
	unsigned int profile,
	unsigned short vid,
	unsigned int direct
)
{
	struct npd_mirror_item_s destItem;
	int status;

	if(profile > MAX_MIRROR_PROFILE) {
		return MIRROR_RETURN_CODE_ERROR;
	}

	syslog_ax_mirror_dbg("mirror profile set remote vlan in profile %d vlan 0x%x direct %d\n", 
		profile, vid, direct);

	memset( &destItem, 0, sizeof(struct npd_mirror_item_s));
	status = npd_mirror_remote_vlan_conflict_check(profile, vid);
	if( FALSE == status )
	{
		return MIRROR_RETURN_CODE_DEST_PORT_CONFLICT;
	}
	
	status = npd_mirror_get_profile_node(profile, &destItem);
	if(MIRROR_RETURN_CODE_SUCCESS != status) {
		return status;
	}


	if(MIRROR_INGRESS_E == direct && vid != destItem.in_remote_vid)
	{
		destItem.in_remote_vid = vid;
	}   
	else if(MIRROR_EGRESS_E == direct && vid != destItem.eg_remote_vid) 
	{
		destItem.eg_remote_vid = vid;
	}
	else if(MIRROR_BIDIRECTION_E == direct && (vid != destItem.in_remote_vid || vid != destItem.eg_remote_vid)) 
	{	
		destItem.in_remote_vid = vid;
		destItem.eg_remote_vid = vid;
	}
	else {
		return (vid==MIRROR_REMOTE_VLAN_DEFAULT)?MIRROR_RETURN_CODE_REMOTE_VLAN_EXIST:\
								MIRROR_RETURN_CODE_REMOTE_VLAN_EXIST;
	}
	
		
	status = dbtable_hash_update( npd_mirror_hashId_index, &destItem, &destItem );
	if( 0 != status )
		return MIRROR_RETURN_CODE_ERROR;

	syslog_ax_mirror_dbg("mirror profile create source vlan %d in profile %d success\n", vid, profile );

	return MIRROR_RETURN_CODE_SUCCESS;
}

int npd_mirror_dest_port_hw_create
( 
	struct npd_mirror_item_s *dbItem, 
	int direct, 
	unsigned int eth_g_index
)
{
	unsigned int  ret = MIRROR_RETURN_CODE_SUCCESS;	 

	syslog_ax_mirror_dbg("dest port create: ifindex 0x%x direct %d\n", eth_g_index, direct);
    ret = nam_mirror_analyzer_port_set(dbItem->profileId, MIRROR_DEST_INPORT_DEFAULT, eth_g_index, 1, direct);
	if(ret != MIRROR_RETURN_CODE_SUCCESS)
	{
		npd_syslog_mirror_err("Error to set the destination port!\n");
        ret = MIRROR_RETURN_CODE_ERROR;
	}	
	syslog_ax_mirror_dbg("Set destination port success!profile %d,eth_g_index %d,direct %s\n",dbItem->profileId,\
					eth_g_index,(MIRROR_INGRESS_E == direct)? "ingress":(MIRROR_INGRESS_E == direct)? "egress":"bidirection");
	return ret;
}

int npd_mirror_dest_port_hw_delete
( 
	struct npd_mirror_item_s *dbItem, 
	int direct, 
	unsigned int eth_g_index
)
{		
	unsigned int		ret = MIRROR_RETURN_CODE_SUCCESS;	

	syslog_ax_mirror_dbg("dest port delete: ifindex 0x%x direct %d\n", eth_g_index, direct);

	ret = nam_mirror_analyzer_port_set(dbItem->profileId, MIRROR_DEST_INPORT_DEFAULT, eth_g_index, 0, 0);	
	if( 0 != ret )
	{
		return MIRROR_RETURN_CODE_ERROR;
	}

	if( MIRROR_EGRESS_E == direct || MIRROR_BIDIRECTION_E == direct )
	{
		if( NPD_OK != npd_fdb_static_mirror_entry_del_by_profile(dbItem->profileId))
		{
		   ret = MIRROR_RETURN_CODE_ERROR;
		}
	}

	return ret;
}

int npd_mirror_src_port_hw_create
( 
	struct npd_mirror_item_s *dbItem, 
	unsigned int eth_g_index, 
	MIRROR_DIRECTION_TYPE direct 
)
{
	unsigned int ret = 0;
	unsigned int analyzer_eth_g_index = 0;
	syslog_ax_mirror_dbg("src port create: ifindex 0x%x direct %d\n", eth_g_index, direct);
	if(direct == MIRROR_EGRESS_E)
	{
        analyzer_eth_g_index = dbItem->eg_eth_index;
	}else if((direct == MIRROR_INGRESS_E) || (direct == MIRROR_BIDIRECTION_E)) 
	{
        analyzer_eth_g_index = dbItem->in_eth_index;
	}
	if(0 != nam_mirror_port_set(dbItem->profileId, eth_g_index, analyzer_eth_g_index, 1, direct))
	{
		ret = MIRROR_RETURN_CODE_ERROR;
	}
	return ret;
}

int npd_mirror_src_port_hw_delete
( 
	struct npd_mirror_item_s *dbItem, 
	unsigned int eth_g_index,  
	MIRROR_DIRECTION_TYPE direct 
)
{	
	int ret = 0;
	unsigned int analyzer_eth_g_index = 0;

	syslog_ax_mirror_dbg("src port delete: ifindex 0x%x direct %d\n", eth_g_index, direct);
	if(direct == MIRROR_EGRESS_E)
	{
        analyzer_eth_g_index = dbItem->eg_eth_index;
	}else if((direct == MIRROR_INGRESS_E) || (direct == MIRROR_BIDIRECTION_E)) 
	{
        analyzer_eth_g_index = dbItem->in_eth_index;
	}

	if(0 != nam_mirror_port_set(dbItem->profileId, eth_g_index, analyzer_eth_g_index, 0, direct))
	{
		ret = MIRROR_RETURN_CODE_ERROR;
	}
	
	return ret;
}

int npd_mirror_src_vlan_hw_create
( 
	struct npd_mirror_item_s *dbItem, 
	unsigned int vlanId 
)
{
	int ret = MIRROR_RETURN_CODE_SUCCESS;
	
	syslog_ax_mirror_dbg("src VLAN create: vlanId %d\n", vlanId);
	
	if(MIRROR_DEST_INPORT_DEFAULT != dbItem->in_eth_index)
		{
	
			 ret = nam_mirror_vlan_set(dbItem->profileId, vlanId,dbItem->in_eth_index,1);
			 if(MIRROR_DEST_INPORT_DEFAULT != dbItem->eg_eth_index)
			 {
				 if( ret == NPD_SUCCESS )
				 {
					 ret = nam_mirror_vlan_set(dbItem->profileId, vlanId,dbItem->eg_eth_index,1);
					 if(0 != ret)
					 {
						 npd_syslog_dbg("Delete src vlan for ingress error! vid %d\n",vlanId);
						 ret = MIRROR_RETURN_CODE_ERROR;
					 }
				 }
				 else
					ret = MIRROR_RETURN_CODE_SUCCESS;	
			}
		}
	
		return ret;
}

int npd_mirror_src_vlan_hw_delete
( 
	struct npd_mirror_item_s *dbItem, 
	unsigned int vlanId 
)
{
	int ret = 0;

	syslog_ax_mirror_dbg("src VLAN delete: vlanId %d\n", vlanId);


	if(MIRROR_DEST_INPORT_DEFAULT != dbItem->in_eth_index)
	{

		 ret = nam_mirror_vlan_set(dbItem->profileId, vlanId,dbItem->in_eth_index,0);
		 if(MIRROR_DEST_INPORT_DEFAULT != dbItem->eg_eth_index)
		 {
			 if( ret == NPD_SUCCESS )
			 {
				 ret = nam_mirror_vlan_set(dbItem->profileId, vlanId,dbItem->eg_eth_index,0);
				 if(0 != ret)
				 {
					 npd_syslog_dbg("Delete src vlan for ingress error! vid %d\n",vlanId);
					 ret = MIRROR_RETURN_CODE_ERROR;
				 }
			 }
			 else
			 	ret = MIRROR_RETURN_CODE_SUCCESS;	
		}
	}

	return ret;
}


int npd_mirror_remote_vlan_hw_create
( 
	struct npd_mirror_item_s *dbItem, 
	int direct, 
	unsigned short vid
)
{
	unsigned int  ret = MIRROR_RETURN_CODE_SUCCESS;	 

	syslog_ax_mirror_dbg("remote vlan create: vlan %d direct %d\n", vid, direct);

	if (direct != MIRROR_EGRESS_E)
		ret = nam_mirror_remote_vlan_set(dbItem->profileId, dbItem->in_eth_index, vid, direct, 1);
	else
		ret = nam_mirror_remote_vlan_set(dbItem->profileId, dbItem->eg_eth_index, vid, direct, 1);
		
	
	if(ret != MIRROR_RETURN_CODE_SUCCESS)
	{
		npd_syslog_mirror_err("Error to set the remote vlan!\n");
        ret = MIRROR_RETURN_CODE_ERROR;
	}	
	syslog_ax_mirror_dbg("Set remote vlan success!profile %d,vlan %d,direct %s\n",dbItem->profileId,\
					vid,(MIRROR_INGRESS_E == direct)? "ingress":(MIRROR_INGRESS_E == direct)? "egress":"bidirection");
	return ret;
}

int npd_mirror_remote_vlan_hw_delete
( 
	struct npd_mirror_item_s *dbItem, 
	int direct, 
	unsigned short vid
)
{		
	unsigned int		ret = MIRROR_RETURN_CODE_SUCCESS;	

	syslog_ax_mirror_dbg("remote vlan delete: vlan %d direct %d\n", vid, direct);

	if (direct != MIRROR_EGRESS_E)
		ret = nam_mirror_remote_vlan_set(dbItem->profileId, dbItem->in_eth_index, vid, direct, 0);
	else
		ret = nam_mirror_remote_vlan_set(dbItem->profileId, dbItem->eg_eth_index, vid, direct, 0);

	if( 0 != ret )
	{
		return MIRROR_RETURN_CODE_ERROR;
	}

	if( MIRROR_EGRESS_E == direct || MIRROR_BIDIRECTION_E == direct )
	{
		if( NPD_OK != npd_fdb_static_mirror_entry_del_by_profile(dbItem->profileId))
		{
		   ret = MIRROR_RETURN_CODE_ERROR;
		}
	}

	return ret;
}


void npd_mirror_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
    int type = npd_netif_type_get(netif_index);
    int ret;
    
    if((type == NPD_NETIF_VLAN_TYPE))
    {
        if(evt == PORT_NOTIFIER_DELETE)
        {
	        struct npd_mirror_item_s destItem;
			unsigned short vid = npd_netif_vlan_get_vid(netif_index);
            memset(&destItem, 0, sizeof(struct npd_mirror_item_s));
			
            ret = dbtable_hash_head(npd_mirror_hashId_index, &destItem, &destItem, NULL);
			while(ret == 0)
			{
			    unsigned int direct = 0;
			    if(vid == destItem.in_remote_vid && vid == destItem.eg_remote_vid)
			    {
			        direct = MIRROR_BIDIRECTION_E;
			    }
				else if(vid == destItem.in_remote_vid)
				{
			        direct = MIRROR_INGRESS_E;
				}
				else if(vid == destItem.eg_remote_vid)
				{
			        direct = MIRROR_EGRESS_E;
				}
				if(direct)
			        npd_mirror_remote_vlan_set(destItem.profileId, MIRROR_REMOTE_VLAN_DEFAULT, direct);
				ret = dbtable_hash_next(npd_mirror_hashId_index, &destItem, &destItem, NULL);
			}
        }
        return;
    }
    if((type != NPD_NETIF_ETH_TYPE))
        return;

	syslog_ax_arpsnooping_dbg("npd notify mirror index event: index 0x%x event %d\n", netif_index, evt);
	switch(evt)
    {
	    case PORT_NOTIFIER_L2CREATE:       
	    case PORT_NOTIFIER_LINKUP_E:
	    case PORT_NOTIFIER_INSERT:
        case PORT_NOTIFIER_DISCARD:
	    case PORT_NOTIFIER_LINKDOWN_E:
		case PORT_NOTIFIER_REMOVE:
			break;
	    case PORT_NOTIFIER_L2DELETE:
            break;
        case PORT_NOTIFIER_DELETE:
			ret = npd_mirror_port_delete(netif_index);	
	        break;
	    default:
	        break;
    }

    return;
}

void npd_mirror_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
    int subtype = npd_netif_type_get(netif_index);
    unsigned long type = npd_netif_type_get(vlan_index);

	syslog_ax_arpsnooping_dbg("npd notify mirror relate event: ifindex 0x%x subindex 0x%x event %d\n", \
											vlan_index, netif_index, event);

    if( NPD_NETIF_TRUNK_TYPE != type || subtype != NPD_NETIF_ETH_TYPE )
        return;

	if( PORT_NOTIFIER_JOIN == event )
	{
		npd_mirror_port_delete( netif_index );
	}
}


unsigned int npd_mirror_key_generate(void * in )
{
	unsigned int key = 0;
	unsigned int profileId = 0;
	struct npd_mirror_item_s *mirrorItem = (struct npd_mirror_item_s *)in;
	
	if(NULL == mirrorItem) {
		syslog_ax_igmp_err("npd igmp snooping items make key null pointers error.");
		return ~0UI;
	}


	profileId = (mirrorItem->profileId);

	key = profileId & 0xF;

	key %= (NPD_MIRROR_HASH_ID_SIZE);
	
	return key;	
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
unsigned int npd_mirror_compare
(
	void *in1,
	void *in2
)
{
	struct npd_mirror_item_s *itemA = (struct npd_mirror_item_s *)in1;
	struct npd_mirror_item_s *itemB = (struct npd_mirror_item_s *)in2;
	
	unsigned int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_igmp_err("npd igmp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->profileId != itemB->profileId ) 
	{
		equal = FALSE;
	}
	
	return equal;
} 

int npd_mirror_src_port_update( );

long npd_mirror_dbtbl_handle_update( void *newItem, void *oldItem )
{
	struct npd_mirror_item_s *origItem = NULL, *updateItem = NULL;
	npd_pbmp_t pList;
	npd_vbmp_t vList;
	unsigned int eth_g_index = 0;
	unsigned int pId = 0, vId = 0;
	unsigned int bidirect = 0;
    unsigned int old_bidirect = 0;

	if( oldItem == NULL || newItem == NULL )
		return NPD_ERR;	
	
	origItem = (struct npd_mirror_item_s *)oldItem;
	updateItem = (struct npd_mirror_item_s *)newItem;

	syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_update: update item %d\n", updateItem->profileId);

	if( updateItem->in_eth_index == updateItem->eg_eth_index)
		bidirect = 1;
    if( origItem->in_eth_index == origItem->eg_eth_index)
        old_bidirect = 1;

	if((origItem->in_eth_index!=updateItem->in_eth_index) && (updateItem->in_eth_index!=MIRROR_DEST_INPORT_DEFAULT)) 
	{		
		syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_update: change dest ingress 0x%x to 0x%x\n",\
								origItem->in_eth_index, updateItem->in_eth_index);
		if( bidirect )
			npd_mirror_dest_port_hw_create( updateItem, MIRROR_BIDIRECTION_E, updateItem->in_eth_index);
		else
			npd_mirror_dest_port_hw_create( updateItem, MIRROR_INGRESS_E, updateItem->in_eth_index);
	}

	if((origItem->eg_eth_index!=updateItem->eg_eth_index) && (updateItem->eg_eth_index!=MIRROR_DEST_INPORT_DEFAULT))
	{
		syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_update: change dest egress 0x%x to 0x%x\n",\
								origItem->eg_eth_index, updateItem->eg_eth_index);
		if(!bidirect)
			npd_mirror_dest_port_hw_create( updateItem, MIRROR_EGRESS_E, updateItem->eg_eth_index);
	}
	
	NPD_PBMP_CLEAR(pList);
	NPD_PBMP_ASSIGN(pList, updateItem->in_eth_mbr);
	NPD_PBMP_OR(pList, updateItem->eg_eth_mbr);
	NPD_PBMP_OR(pList, updateItem->bi_eth_mbr);

	NPD_PBMP_OR(pList, origItem->in_eth_mbr);
	NPD_PBMP_OR(pList, origItem->eg_eth_mbr);
	NPD_PBMP_OR(pList, origItem->bi_eth_mbr);

	NPD_PBMP_ITER(pList, pId)
	{
		if( NPD_PBMP_MEMBER(updateItem->in_eth_mbr, pId) && !NPD_PBMP_MEMBER(origItem->in_eth_mbr, pId))
		{
			eth_g_index = eth_port_array_index_to_ifindex(pId);
			npd_mirror_src_port_hw_create( updateItem, eth_g_index, MIRROR_INGRESS_E);
		}		
		else if( NPD_PBMP_MEMBER(updateItem->eg_eth_mbr, pId) && !NPD_PBMP_MEMBER(origItem->eg_eth_mbr, pId))
		{
			eth_g_index = eth_port_array_index_to_ifindex(pId);
			npd_mirror_src_port_hw_create( updateItem, eth_g_index, MIRROR_EGRESS_E);
		}
		else if( NPD_PBMP_MEMBER(updateItem->bi_eth_mbr, pId) && !NPD_PBMP_MEMBER(origItem->bi_eth_mbr, pId))
		{
			eth_g_index = eth_port_array_index_to_ifindex(pId);
			npd_mirror_src_port_hw_create( updateItem, eth_g_index, MIRROR_BIDIRECTION_E);
		}
		else if( !NPD_PBMP_MEMBER(updateItem->in_eth_mbr, pId) && !NPD_PBMP_MEMBER(updateItem->eg_eth_mbr, pId) &&\
			        !NPD_PBMP_MEMBER(updateItem->bi_eth_mbr, pId))
		{
			eth_g_index = eth_port_array_index_to_ifindex(pId);
			if (NPD_PBMP_MEMBER(origItem->in_eth_mbr, pId)) 
			{
				npd_mirror_src_port_hw_delete( updateItem, eth_g_index, MIRROR_INGRESS_E);				
			}
			else if (NPD_PBMP_MEMBER(origItem->eg_eth_mbr, pId))
			{
				npd_mirror_src_port_hw_delete( updateItem, eth_g_index, MIRROR_EGRESS_E);
			}
			else
			{
				npd_mirror_src_port_hw_delete( updateItem, eth_g_index, MIRROR_BIDIRECTION_E);
			}
		}		
	}

	if( !NPD_VBMP_EQ(origItem->vlan_mbr, updateItem->vlan_mbr) )
	{
		NPD_VBMP_ASSIGN(vList, origItem->vlan_mbr);
		NPD_VBMP_XOR(vList, updateItem->vlan_mbr);
		NPD_VBMP_ITER(vList,vId)
		{
			if( NPD_VBMP_MEMBER( origItem->vlan_mbr, vId))  //Delete vlan
			{
				npd_mirror_src_vlan_hw_delete( updateItem, vId);
			}
			else //Add vlan
			{
				npd_mirror_src_vlan_hw_create( updateItem, vId);
			}
		}
	}

	if((origItem->in_eth_index!=updateItem->in_eth_index) && (updateItem->in_eth_index==MIRROR_DEST_INPORT_DEFAULT))
	{
		syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_update: change dest ingress 0x%x to 0x%x\n",\
								origItem->in_eth_index, updateItem->in_eth_index);
		if( bidirect )
			npd_mirror_dest_port_hw_delete( updateItem, MIRROR_BIDIRECTION_E, origItem->in_eth_index);
		else if(old_bidirect && (origItem->eg_eth_index == updateItem->eg_eth_index)) /*for delete ingress, need set EGRESS*/
			npd_mirror_dest_port_hw_create( updateItem, MIRROR_EGRESS_E, updateItem->eg_eth_index);
        else
			npd_mirror_dest_port_hw_delete( updateItem, MIRROR_BIDIRECTION_E, origItem->in_eth_index);
	}

	if((origItem->eg_eth_index!=updateItem->eg_eth_index) && (updateItem->eg_eth_index==MIRROR_DEST_INPORT_DEFAULT))
	{
		syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_update: change dest egress 0x%x to 0x%x\n",\
								origItem->eg_eth_index, updateItem->eg_eth_index);
		if( !bidirect ) 
        {
            if(old_bidirect && origItem->in_eth_index == updateItem->in_eth_index)
			    npd_mirror_dest_port_hw_create( updateItem, MIRROR_INGRESS_E, origItem->in_eth_index);
            else
                npd_mirror_dest_port_hw_delete( updateItem, MIRROR_BIDIRECTION_E, origItem->eg_eth_index);
		}
	}

	
	
	if (origItem->in_remote_vid !=updateItem->in_remote_vid)
	{
		/* This branch process the remote vid change */
		int direct = 0;
			
		/* if the remote vid change, then adjust the */
		if (bidirect)
		{
			if (updateItem->in_eth_index != MIRROR_DEST_INPORT_DEFAULT)
			{
				direct = MIRROR_BIDIRECTION_E;
			}
		}
		else
		{
			if (updateItem->in_eth_index != MIRROR_DEST_INPORT_DEFAULT)
			{
				direct = MIRROR_INGRESS_E;
			}
			if (updateItem->eg_eth_index != MIRROR_DEST_INPORT_DEFAULT)
			{
				direct = MIRROR_EGRESS_E;
			}				
		}

		if (direct)
		{
			if (MIRROR_REMOTE_VLAN_DEFAULT != updateItem->in_remote_vid)
			{
				npd_mirror_remote_vlan_hw_create( updateItem, direct, updateItem->in_remote_vid);
			}
			else
			{
				npd_mirror_remote_vlan_hw_delete( updateItem, direct, updateItem->in_remote_vid);
			}
		}
	}
	else if (updateItem->in_remote_vid != MIRROR_REMOTE_VLAN_DEFAULT)
	{
		if (updateItem->in_eth_index != origItem->in_eth_index)
		{
			if (updateItem->in_eth_index != MIRROR_REMOTE_VLAN_DEFAULT)
			{
				npd_mirror_remote_vlan_hw_create( updateItem, MIRROR_INGRESS_E, updateItem->in_remote_vid);
			}
			else
			{
				if (old_bidirect && (origItem->in_eth_index != MIRROR_REMOTE_VLAN_DEFAULT))
				{
					/* from bidirect to delete ingress, not del remote vid */
				}
				else
				{
					npd_mirror_remote_vlan_hw_delete( updateItem, MIRROR_INGRESS_E, updateItem->in_remote_vid);
				}				
			}			
		}

		if (updateItem->eg_eth_index != origItem->eg_eth_index)
		{
			if (updateItem->eg_eth_index != MIRROR_REMOTE_VLAN_DEFAULT)
			{
				npd_mirror_remote_vlan_hw_create( updateItem, MIRROR_EGRESS_E, updateItem->in_remote_vid);
			}
			else
			{
				if (old_bidirect && (origItem->eg_eth_index != MIRROR_REMOTE_VLAN_DEFAULT))
				{
					/* from bidirect to delete egress, not del remote vid */
				}
				else
				{
					npd_mirror_remote_vlan_hw_delete( updateItem, MIRROR_EGRESS_E, updateItem->in_remote_vid);
				}				
			}			
		}
	}

	return NPD_SUCCESS;
}

long npd_mirror_dbtbl_handle_insert( void *newItem )
{
	struct npd_mirror_item_s *opItem = NULL;
	npd_pbmp_t pList;
	npd_vbmp_t vList;
	unsigned int eth_g_index = 0;
	unsigned int pId = 0, vId = 0;
    unsigned int bidirect = 0;
    int ret = ETHPORT_RETURN_CODE_ERR_NONE;
	
	if( newItem == NULL )
		return NPD_ERR;
	
	opItem = (struct npd_mirror_item_s *)newItem;
	
	syslog_ax_mirror_dbg("npd_mirror_dbtle_handle_insert: insert item %d\n", opItem->profileId);
    if(opItem->in_eth_index == opItem->eg_eth_index)
    {
        bidirect = 1;
    }
	if(opItem->in_eth_index != 0 && opItem->in_eth_index != MIRROR_DEST_INPORT_DEFAULT)
	{
	    if(bidirect)
	    {
		    npd_mirror_dest_port_hw_create( opItem, MIRROR_BIDIRECTION_E, opItem->in_eth_index);
		}
		else 
		{
		    npd_mirror_dest_port_hw_create( opItem, MIRROR_INGRESS_E, opItem->in_eth_index);
		}
	}
	if(opItem->eg_eth_index != 0 && opItem->eg_eth_index != MIRROR_DEST_INPORT_DEFAULT)
	{
	    if(!bidirect)
	    {
		    npd_mirror_dest_port_hw_create( opItem, MIRROR_EGRESS_E, opItem->eg_eth_index);
		}
	}
	
	if(opItem->in_remote_vid != 0)
	{
	    if(bidirect)
	    {
		    npd_mirror_remote_vlan_hw_create( opItem, MIRROR_BIDIRECTION_E, opItem->in_remote_vid);
		}
		else 
		{
		    npd_mirror_remote_vlan_hw_create( opItem, MIRROR_INGRESS_E, opItem->in_remote_vid);
		}
	}
	if(opItem->eg_remote_vid != 0)
	{
	    if(!bidirect)
	    {
		    npd_mirror_remote_vlan_hw_create( opItem, MIRROR_EGRESS_E, opItem->eg_remote_vid);
		}
	}


	NPD_PBMP_ASSIGN(pList, opItem->in_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
		unsigned char tmp_devNum, tmp_portNum; 
		eth_g_index = eth_port_array_index_to_ifindex(pId); 
	
		ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);

        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
        {
            continue;
        }
        
	    npd_mirror_src_port_hw_create( opItem, eth_g_index, MIRROR_INGRESS_E);
	}

	NPD_PBMP_ASSIGN(pList, opItem->eg_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
		unsigned char tmp_devNum, tmp_portNum; 
        eth_g_index = eth_port_array_index_to_ifindex(pId);
	
		ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);

        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
        {
            continue;
        }
		
		npd_mirror_src_port_hw_create( opItem, eth_g_index, MIRROR_EGRESS_E);
	}
    
    NPD_PBMP_ASSIGN(pList, opItem->bi_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
		unsigned char tmp_devNum, tmp_portNum; 
	    eth_g_index = eth_port_array_index_to_ifindex(pId);

		ret = npd_get_devport_by_global_index(eth_g_index, &tmp_devNum, &tmp_portNum);
        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
        {
            continue;
        }
        
		npd_mirror_src_port_hw_create( opItem, eth_g_index, MIRROR_BIDIRECTION_E);
	}
     
	NPD_VBMP_ASSIGN(vList, opItem->vlan_mbr);
	NPD_VBMP_ITER(vList,vId)
	{
		npd_mirror_src_vlan_hw_create( opItem, vId);
	}
	return NPD_SUCCESS;
}


long npd_mirror_dbtbl_handle_delete( void *delItem )
{
	struct npd_mirror_item_s *opItem = NULL;
	unsigned int pId, eth_g_index;
	npd_pbmp_t pList;
    unsigned int ret;
	unsigned int bidirect;
	
	if( delItem == NULL )
		return NPD_ERR;
	
	opItem = (struct npd_mirror_item_s *)delItem;
	if( opItem->in_eth_index == opItem->eg_eth_index)
		bidirect = 1;

	syslog_ax_mirror_dbg("npd_mirror_dbtbl_handle_delete: delete item %d\n", opItem->profileId);


	if(opItem->in_remote_vid != MIRROR_REMOTE_VLAN_DEFAULT)
	{      
		if (bidirect)
		{
			ret = npd_mirror_remote_vlan_hw_delete(
				opItem, MIRROR_BIDIRECTION_E, opItem->in_remote_vid);
		}
		else
		{
			ret = npd_mirror_remote_vlan_hw_delete(
				opItem, MIRROR_INGRESS_E, opItem->in_remote_vid);			
		}		
		if (ret != 0)
			ret = MIRROR_RETURN_CODE_ERROR;

	}

	if(opItem->eg_remote_vid != MIRROR_REMOTE_VLAN_DEFAULT)
	{      
		if (!bidirect)
		{
			ret = npd_mirror_remote_vlan_hw_delete(
				opItem, MIRROR_EGRESS_E, opItem->eg_remote_vid);
		}
		if (ret != 0)
			ret = MIRROR_RETURN_CODE_ERROR;
	}


    NPD_PBMP_ASSIGN(pList, opItem->bi_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
	    eth_g_index = eth_port_array_index_to_ifindex(pId);
	    ret = eth_port_local_check(eth_g_index);
        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
        {
            continue;
        }
        
		npd_mirror_src_port_hw_delete( opItem, eth_g_index, MIRROR_BIDIRECTION_E);
	}

    NPD_PBMP_ASSIGN(pList, opItem->eg_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
	    eth_g_index = eth_port_array_index_to_ifindex(pId);
	    ret = eth_port_local_check(eth_g_index);
        if(ETHPORT_RETURN_CODE_ERR_NONE != ret)
        {
            continue;
        }
        
		npd_mirror_src_port_hw_delete( opItem, eth_g_index, MIRROR_EGRESS_E);
	}	

   	NPD_PBMP_ASSIGN(pList, opItem->in_eth_mbr);
	NPD_PBMP_ITER(pList,pId)
	{
		eth_g_index = eth_port_array_index_to_ifindex(pId);
		npd_mirror_src_port_hw_delete( opItem, eth_g_index, MIRROR_INGRESS_E);
	}	

	if (opItem->in_eth_index != MIRROR_DEST_INPORT_DEFAULT)
	{
		int direct = bidirect ? MIRROR_BIDIRECTION_E : MIRROR_INGRESS_E;
		npd_mirror_dest_port_hw_delete(opItem, direct, opItem->in_eth_index);
	}
	if (opItem->eg_eth_index != MIRROR_DEST_INPORT_DEFAULT && !bidirect)
	{
		npd_mirror_dest_port_hw_delete(opItem, MIRROR_EGRESS_E, opItem->in_eth_index);		
	}
	
	return NPD_SUCCESS;
}

int npd_mirror_cfgtbl_handle_update(void *oldItem, void *newItem)
{
    return 0;
}

int npd_mirror_table_init()
{
   int ret;

   char name[31] = {0};

   memcpy(name, NPD_MIRROR_HASHTBL_NAME, strlen(NPD_MIRROR_HASHTBL_NAME));
 
   ret = create_dbtable( name, NPD_MIRROR_TABLE_SIZE, sizeof(struct npd_mirror_item_s),\
				   npd_mirror_dbtbl_handle_update, 
				   NULL,
				   npd_mirror_dbtbl_handle_insert, 
				   npd_mirror_dbtbl_handle_delete,
				   NULL,
				   NULL, 
				   NULL, 
					NULL,
					NULL,
				   DB_SYNC_ALL,
				   &(npd_mirror_dbtbl));
   if( 0 != ret )
   {
	   npd_syslog_mirror_err("create npd mirror snp database fail\n");
	   return NPD_FAIL;
   }

   ret = dbtable_create_hash_index("mirror", npd_mirror_dbtbl,NPD_MIRROR_HASH_ID_SIZE, npd_mirror_key_generate,\
												   npd_mirror_compare, &npd_mirror_hashId_index);
   if( 0  != ret )
   {
	   npd_syslog_mirror_err("create npd mirror snp hash table fail\n");
	   return NPD_FAIL;
   }   

   return NPD_OK;
   
}

/**********************************************************************************
 *  npd_mirror_init
 *
 *	DESCRIPTION:
 * 		this routine initialize all mirror mode
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
void npd_mirror_init(void){

	nam_mirror_init();	

	npd_mirror_table_init();

	register_netif_notifier(&npd_mirror_netif_notifier);

	return ;
}

/**************************************************************/
/* NPD Mirror DBUS operation                                                                        */
/**************************************************************/

DBusMessage * npd_dbus_mirror_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned int	    ret = 0;
	unsigned int 	profile = DEFAULT_MIRROR_PROFILE;
	unsigned char   isAdd = 0;
	struct npd_mirror_item_s dbItem;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_BYTE, &isAdd,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	if(MIRROR_RETURN_CODE_SUCCESS != nam_profile_check(profile)){
       syslog_ax_mirror_err("profile check error not supported value %d!\n",profile);
	   ret = MIRROR_RETURN_CODE_ACTION_NOT_SUPPORT;
	}
	else{
		ret = npd_mirror_get_profile_node(profile, &dbItem);
		if( isAdd && ret == MIRROR_RETURN_CODE_SUCCESS )
		{
			ret = MIRROR_RETURN_CODE_SUCCESS;
		}
		else if( !isAdd && ret != MIRROR_RETURN_CODE_SUCCESS )
		{
			ret = MIRROR_RETURN_CODE_PROFILE_NOT_CREATED;
		}
		else {
			if( isAdd ) {
				ret = npd_mirror_profile_create(profile);
			}
			else {
				ret = npd_mirror_profile_delete(profile);
			}
		}
	}
	
	dbus_error_init(&err);
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&profile);
	return reply;
}

DBusMessage * npd_dbus_mirror_create_dest_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned int	    eth_g_index=0, ret = MIRROR_RETURN_CODE_SUCCESS,profile = DEFAULT_MIRROR_PROFILE;	 
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&eth_g_index,
								 DBUS_TYPE_UINT32,&direct,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			npd_syslog_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	ret = npd_mirror_port_exist_check(eth_g_index);
	if(MIRROR_RETURN_CODE_SUCCESS == ret)
	{
	    ret = npd_mirror_destport_direct_check(profile,direct);
	}
	if(MIRROR_RETURN_CODE_SUCCESS == ret)
	{
		ret = npd_mirror_destination_port_set(profile,eth_g_index,direct);		
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &eth_g_index);
	return reply;
}

DBusMessage * npd_dbus_mirror_del_dest_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned int	    eth_g_index=0, ret = MIRROR_RETURN_CODE_SUCCESS,profile = DEFAULT_MIRROR_PROFILE;	
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&eth_g_index,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT32,&direct,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}			
    ret = npd_mirror_destination_exist_check(profile, direct, eth_g_index);
	if(ret != MIRROR_RETURN_CODE_SUCCESS)
	{	
		
		goto error;
		//ret = npd_mirror_destination_port_set( profile, MIRROR_DEST_INPORT_DEFAULT, direct);    	
	}
	if (NPD_TRUE == npd_mirror_destination_node_member_check(profile, direct))
	{
		ret = MIRROR_RETURN_CODE_SRC_PORT_EXIST;
		goto error;	
	}
	ret = npd_mirror_destination_port_set( profile, MIRROR_DEST_INPORT_DEFAULT, direct);    	


error:
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}

DBusMessage * npd_dbus_mirror_create_source_acl(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	return NULL;
}


DBusMessage * npd_dbus_mirror_delete_source_acl(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    return NULL;
}

DBusMessage * npd_dbus_mirror_create_source_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned int	    src_eth_g_index = 0;
	int ret = 0;
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT32,&direct,
								 DBUS_TYPE_UINT32,&src_eth_g_index,  
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    ret = npd_mirror_port_exist_check(src_eth_g_index);
	if(ret == MIRROR_RETURN_CODE_SUCCESS)
	{
	    ret = npd_mirror_src_port_create(profile,direct,src_eth_g_index);
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}


DBusMessage * npd_dbus_mirror_del_source_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned int	    src_eth_g_index = 0;
	int ret = 0;
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;

	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								  DBUS_TYPE_UINT32,&direct,
								 DBUS_TYPE_UINT32,&src_eth_g_index,  
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	

	ret = npd_mirror_src_port_remove(profile,direct,src_eth_g_index);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}

DBusMessage * npd_dbus_mirror_create_source_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		 
	unsigned short vid = 0;
	int ret = 0;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT16,&vid,  
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	

	if(NPD_TRUE != npd_check_vlan_exist(vid)) {
		ret =MIRROR_RETURN_CODE_VLAN_NOT_EXIST;
	}

	ret = npd_mirror_src_vlan_create(profile,vid);	

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}


DBusMessage * npd_dbus_mirror_del_source_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   			 
	unsigned short vid = 0;
	int ret = 0;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT16,&vid,  
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	

	ret = npd_mirror_src_vlan_remove(profile,vid);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}

DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_set(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned short      vlanid = 0;
	unsigned int        eth_g_index = 0;
	int	   ret = MIRROR_RETURN_CODE_SUCCESS;
    ETHERADDR macAddr; 
	unsigned char *mac = NULL;
    memset(&macAddr,0,sizeof(ETHERADDR));
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT16,&vlanid,
								 DBUS_TYPE_UINT32,&eth_g_index,
								 DBUS_TYPE_BYTE,&(macAddr.arEther[0]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[1]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[2]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[3]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[4]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[5]),
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	mac = macAddr.arEther;
   /*check if sys mac*/
 	if(0 != npd_system_verify_basemac((char *)macAddr.arEther)){
        ret = MIRROR_RETURN_CODE_FDB_MAC_BE_SYSMAC;
	}
   /*check vlan if exist*/
	else if(NPD_TRUE != npd_check_vlan_exist(vlanid)) {
		ret =MIRROR_RETURN_CODE_VLAN_NOT_EXIST;
	}
	else
	{
		ret = MIRROR_RETURN_CODE_SUCCESS;
	}
    if( MIRROR_RETURN_CODE_SUCCESS == ret )
    {
		ret = npd_mirror_port_exist_check(eth_g_index);
	}
	if( MIRROR_RETURN_CODE_SUCCESS == ret )
	{
		ret = npd_mirror_src_fdb_create( profile, vlanid, mac, eth_g_index);
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);

	return reply;
}



DBusMessage * npd_dbus_mirror_fdb_mac_vlanid_port_cancel(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	unsigned short      vlanid = 0;
	unsigned int       global_index = 0;
	int	     ret = MIRROR_RETURN_CODE_SUCCESS;
    ETHERADDR macAddr;
	unsigned char *mac = NULL;
    memset(&macAddr,0,sizeof(ETHERADDR));
	unsigned int profile = DEFAULT_MIRROR_PROFILE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT16,&vlanid,
								 DBUS_TYPE_UINT32,&global_index,
								 DBUS_TYPE_BYTE,&(macAddr.arEther[0]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[1]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[2]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[3]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[4]),
								 DBUS_TYPE_BYTE,&(macAddr.arEther[5]),
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    mac = macAddr.arEther;
	if(NPD_TRUE != npd_check_vlan_exist(vlanid)) {
		ret =MIRROR_RETURN_CODE_VLAN_NOT_EXIST;
	}

	if( MIRROR_RETURN_CODE_SUCCESS== ret ) {
		ret = npd_mirror_src_fdb_remove( profile, vlanid, mac, global_index);
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);

	return reply;
}

DBusMessage * npd_dbus_mirror_create_remote_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		 
	unsigned short vid = 0;
	unsigned int direct = 0;
	unsigned int ret = 0;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT16,&vid,  
								 DBUS_TYPE_UINT32,&direct,								 
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	


	if (ret == MIRROR_RETURN_CODE_SUCCESS)
	{
	    ret = npd_mirror_remote_vlan_direct_check(profile,direct);
	}
	if (ret == MIRROR_RETURN_CODE_SUCCESS)
	{
	    if(npd_check_vlan_exist(vid) == TRUE)
		    ret = npd_mirror_remote_vlan_set(profile,vid,direct);
		else
			ret = MIRROR_RETURN_CODE_VLAN_NOT_EXIST;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}


DBusMessage * npd_dbus_mirror_del_remote_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   			 
	unsigned short vid = 0;
	unsigned int ret = 0;
	unsigned int direct = 0;
	
	unsigned int profile = DEFAULT_MIRROR_PROFILE;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_UINT16,&vid,  
								 DBUS_TYPE_UINT32,&direct,								 
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	
	
    ret = npd_mirror_remote_vlan_exist_check(profile, direct, vid);	
	if (ret == MIRROR_RETURN_CODE_SUCCESS)
	{
		ret = npd_mirror_remote_vlan_set(profile,MIRROR_REMOTE_VLAN_DEFAULT,direct);	
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_UINT32,
								   &ret);
	return reply;
}

DBusMessage * npd_dbus_mirror_show(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply = NULL;	  
	DBusError			err; 

    unsigned int *ptr_a, *ptr_b, *ptr_c, *ptr_d, *ptr_e;
    unsigned int ptr_a_len, ptr_b_len, ptr_c_len, ptr_d_len, ptr_e_len;
	unsigned int ret = MIRROR_RETURN_CODE_SUCCESS,profile = DEFAULT_MIRROR_PROFILE;	 
	struct npd_mirror_item_s dbItem = { 0 };
	int status;


	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
								 DBUS_TYPE_UINT32,&profile,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_mirror_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    syslog_ax_mirror_dbg("show mirror profile %d\n",profile);
	
	status = npd_mirror_get_profile_node(profile, &dbItem);
	
    if(MIRROR_RETURN_CODE_SUCCESS != status)
	{
		ret = status;
	}

	reply = dbus_message_new_method_return(msg);

	{
		
	ptr_a = (unsigned int *)&dbItem.in_eth_mbr;
    ptr_a_len = sizeof(dbItem.in_eth_mbr)/sizeof(unsigned int);
	ptr_b = (unsigned int *)&dbItem.eg_eth_mbr;
    ptr_b_len = sizeof(dbItem.in_eth_mbr)/sizeof(unsigned int);
	ptr_c = (unsigned int *)&dbItem.bi_eth_mbr;
    ptr_c_len = sizeof(dbItem.in_eth_mbr)/sizeof(unsigned int);
	ptr_d = (unsigned int *)&dbItem.vlan_mbr;
    ptr_d_len = sizeof(dbItem.in_eth_mbr)/sizeof(unsigned int);
	ptr_e = (unsigned int *)&dbItem.acl_mbr;
    ptr_e_len = sizeof(dbItem.in_eth_mbr)/sizeof(unsigned int);

    dbus_message_append_args(reply,
		
							DBUS_TYPE_UINT32,
							&ret,
							
							DBUS_TYPE_BYTE,
							&dbItem.profileId,
							
							DBUS_TYPE_UINT32,
							&dbItem.in_eth_index,
							
							DBUS_TYPE_UINT32,
							&dbItem.eg_eth_index,
							
							DBUS_TYPE_UINT32,
							&dbItem.in_remote_vid,
							
							DBUS_TYPE_UINT32,
							&dbItem.eg_remote_vid,
							
							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_a,
							ptr_a_len,

							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_b,
							ptr_b_len,

							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_c,
							ptr_c_len,

							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_d,
							ptr_d_len,

							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT32,
							&ptr_e,
							ptr_e_len,

							DBUS_TYPE_INVALID);
    }
	return reply;
}
DBusMessage * npd_dbus_mirror_show_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= { 0 };
	DBusError			err;   	
	
	int i;
	int status;
	struct fdb_entry_item_s *static_mirror_array = NULL;
	unsigned int ret = MIRROR_RETURN_CODE_SUCCESS;
	unsigned int profile = DEFAULT_MIRROR_PROFILE;	 
	struct npd_mirror_item_s dbItem;
	unsigned short vid = 0;
	unsigned int fdb_count = 0;

	dbus_error_init(&err);

	if(!(dbus_message_get_args(msg, &err,
								 DBUS_TYPE_UINT32, &profile,
								 DBUS_TYPE_INVALID))) 
	{
		syslog_ax_mirror_err("Unable to get input args ");
		if(dbus_error_is_set(&err))
		{
			syslog_ax_mirror_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    syslog_ax_mirror_dbg("show mirror profile %d\n",profile);
	status = npd_mirror_get_profile_node(profile, &dbItem);
    if(MIRROR_RETURN_CODE_SUCCESS != status) 
	{
		ret = status;
	}
	fdb_count = npd_fdb_static_mirror_entry_count(dbItem.profileId );
	syslog_ax_mirror_dbg("fdb_count %d\n",fdb_count);

	reply = dbus_message_new_method_return(msg);	

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&fdb_count);
	syslog_ax_mirror_dbg("ret %d,fdb_count %d\n", ret, fdb_count);

	if(fdb_count > 0)
	{
	  	DBusMessageIter		iter_array;
	    dbus_message_iter_open_container (&iter,
							   			DBUS_TYPE_ARRAY,
								  		DBUS_STRUCT_BEGIN_CHAR_AS_STRING /*begin*/									
										DBUS_TYPE_UINT16_AS_STRING /*vid*/										    									  		
										DBUS_TYPE_BYTE_AS_STRING /*mac[0]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mac[1]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mac[2]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mac[3]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mac[4]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mac[5]*/
									   	DBUS_TYPE_BYTE_AS_STRING /*blockMode*/
									   	DBUS_TYPE_BYTE_AS_STRING /*isStatic*/
									   	DBUS_TYPE_BYTE_AS_STRING /*isBlock*/
									   	DBUS_TYPE_BYTE_AS_STRING /*isAuthen*/
									   	DBUS_TYPE_BYTE_AS_STRING /*isMirror*/
									   	DBUS_TYPE_BYTE_AS_STRING /*mirrorProfile*/
								  		DBUS_TYPE_UINT32_AS_STRING /*ifindex*/										   	
								  		DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
							   			&iter_array);
		static_mirror_array = malloc(fdb_count * sizeof(struct fdb_entry_item_s));
		if( static_mirror_array != NULL ) 
		{				
			status = npd_fdb_get_static_mirror_item(static_mirror_array,fdb_count,dbItem.profileId);
			if( status > 0 )
			{
				for( i=0; i<status; i++ )
				{
					DBusMessageIter iter_struct;
					dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
					vid = static_mirror_array[i].vlanid;
					syslog_ax_mirror_dbg("srcFdbNode vid %d,mac %02x:%02x:%02x:%02x:%02x:%02x port_index 0x%x\n",vid,\
					                      static_mirror_array[i].mac[0],static_mirror_array[i].mac[1],static_mirror_array[i].mac[2],\
					                      static_mirror_array[i].mac[3],static_mirror_array[i].mac[4],static_mirror_array[i].mac[5],\
					                      static_mirror_array[i].ifIndex);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_UINT16,
							  &vid);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[0]));
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[1]));
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[2]));
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[3]));
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[4]));
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_BYTE,
							  &(static_mirror_array[i].mac[5]));
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].blockMode);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].isStatic);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].isBlock);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].isAuthen);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].isMirror);
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &static_mirror_array[i].mirrorProfile);
					dbus_message_iter_append_basic
							(&iter_struct,
							   DBUS_TYPE_UINT32,
							  &(static_mirror_array[i].ifIndex));						
					dbus_message_iter_close_container (&iter_array, &iter_struct);
				}
			}       
			free(static_mirror_array);
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	return reply;
}

DBusMessage * npd_dbus_mirror_show_running_cfg(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(MIRROR_SAVE_CFG_MEM);
	if(NULL == strShow) {
		syslog_ax_mirror_err("alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(strShow,0,MIRROR_SAVE_CFG_MEM);

	dbus_error_init(&err);

	npd_mirror_profile_config_save(strShow);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	return reply;
}

#ifdef __cplusplus
}
#endif 
#endif

