
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_dhcp_snp_tbl.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		dhcp snooping table for NPD module.
*
* DATE:
*		06/04/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef HAVE_DHCP_SNP
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

#include "npd_dhcp_snp_tbl.h"
/*********************************************************
*	functions define												*
**********************************************************/

db_table_t   *npd_dhcp_snp_status_item = NULL;
hash_table_index_t *npd_dhcp_snp_status_item_index = NULL;


db_table_t   *npd_dhcp_snp_global_status_table = NULL;
array_table_index_t *npd_dhcp_snp_global_status_index = NULL;

db_table_t   *npd_dhcp_snp_dbtable = NULL;
hash_table_index_t *npd_dhcp_snp_dbhash_mac_index = NULL;
hash_table_index_t *npd_dhcp_snp_dbhash_port_index = NULL;

pthread_mutex_t    nds_lock = PTHREAD_MUTEX_INITIALIZER;
struct npd_dhcp_snp_timeout_s* nds_timeout = NULL;
struct npd_dhcp_snp_timeout_s* nds_free_timeout = NULL;
int npd_dhcp_snp_self_sock = -1;
struct sockaddr_un local;

unsigned int npd_dhcp_snp_self_sock_init()
{
    int name_len = 0;
	
	npd_dhcp_snp_self_sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (npd_dhcp_snp_self_sock < 0)
    {
        syslog_ax_dhcp_snp_err ("Npd dhcp snooping socket failed!\n");
        return -1;
	}

    memset(&local, 0, sizeof(local));
	local.sun_family = PF_UNIX;
	strcpy(local.sun_path, "/var/run/npd_dhcp_snp_self_socket");
	unlink(local.sun_path);	
    
	name_len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(npd_dhcp_snp_self_sock, (struct sockaddr *) &local, name_len) < 0)
	{
        syslog_ax_dhcp_snp_err ("Npd dhcp snooping bind failed!\n");
		return -1;
	}

    return 0;
}

unsigned int npd_dhcp_snp_get_relative()
{
    struct timespec tp;
    
    if (!clock_gettime(CLOCK_MONOTONIC, &tp))
    {
        return tp.tv_sec;
    }

    return 0;
}

int npd_dhcp_snp_get_timeout (struct npd_dhcp_snp_timeout_s* entry)
{
    pthread_mutex_lock(&nds_lock);
	if (NULL != nds_timeout)
	{
        entry->expire_time = nds_timeout->expire_time;
        memcpy(entry->hw_addr, nds_timeout->hw_addr, sizeof(nds_timeout->hw_addr));
        entry->next = NULL;

        pthread_mutex_unlock(&nds_lock);
		return 0;
	}
    
    pthread_mutex_unlock(&nds_lock);
    return -1;
}

void npd_dhcp_snp_add_timeout
(
    unsigned int expire_time,
    const char* hw_addr
)
{
	struct npd_dhcp_snp_timeout_s* pri_node = NULL;
    struct npd_dhcp_snp_timeout_s* node = NULL;

    pthread_mutex_lock(&nds_lock);
	for (node = nds_timeout; node; node = node->next)
	{
		if (0 == memcmp(node->hw_addr, hw_addr, sizeof(node->hw_addr)))
		{
			if (pri_node)
			{
				pri_node->next = node->next;
			}
			else
			{
				nds_timeout = node->next;
			}
			break;
		}
		pri_node = node;
	}

	/* If we didn't supersede a timeout, allocate a timeout
	   structure now. */
	if (!node)
	{
		if (NULL != nds_free_timeout)
		{
			node = nds_free_timeout;
			nds_free_timeout = node->next;
		}
        else
        {
			node = (struct npd_dhcp_snp_timeout_s *)
			     malloc(sizeof (struct npd_dhcp_snp_timeout_s));
			if (NULL == node)
			{
				syslog_ax_dhcp_snp_err ("add_timeout: no memory!");

                pthread_mutex_unlock(&nds_lock);
                return ;
			}
		}
        
		memset (node, 0, sizeof(*node));
        memcpy(node->hw_addr, hw_addr, sizeof(node->hw_addr));
	}

    node->expire_time = expire_time;

	/* Now sort this timeout into the timeout list. */

	/* Beginning of list? */
	if ((NULL == nds_timeout)
        || (nds_timeout->expire_time > node->expire_time)) 
    {
        int self_sock_flag = 0;
		node->next = nds_timeout;
		nds_timeout = node;

        pthread_mutex_unlock(&nds_lock);
        self_sock_flag = sendto(npd_dhcp_snp_self_sock,
                    &self_sock_flag,
                    sizeof(int),
                    0,
                    (struct sockaddr *) &local,
                    strlen(local.sun_path) + sizeof(local.sun_family));
		return ;
	}

	/* Middle of list? */
	for (pri_node = nds_timeout; pri_node->next; pri_node = pri_node->next)
	{
		if (pri_node->next->expire_time > node->expire_time)
		{
			node->next = pri_node->next;
			pri_node->next = node;
            
            pthread_mutex_unlock(&nds_lock);
			return ;
		}
	}

	/* End of list. */
	pri_node->next = node;
	node->next = NULL;

    pthread_mutex_unlock(&nds_lock);
    return ;
}

void npd_dhcp_snp_cancel_timeout(const char* hw_addr)
{
	struct npd_dhcp_snp_timeout_s* pri_node = NULL;
    struct npd_dhcp_snp_timeout_s* node = NULL;

    pthread_mutex_lock(&nds_lock);

	/* Look for this timeout on the list, and unlink it if we find it. */
	for (node = nds_timeout; node; node = node->next)
	{
        if (0 == memcmp(node->hw_addr, (char*)hw_addr, sizeof(node->hw_addr)))
		{
			if (pri_node)
			{
				pri_node->next = node->next;
			}
			else
			{
				nds_timeout = node->next;
			}
			break;
		}
		pri_node = node;
	}

	/* If we found the timeout, put it on the free list. */
	if (node)
	{
		node->next = nds_free_timeout;
		nds_free_timeout = node;
	}

    pthread_mutex_unlock(&nds_lock);
    return ;
}

unsigned int npd_dhcp_snp_timer_thread(void* arg)
{
    int ret = 0;
    int max_fd = npd_dhcp_snp_self_sock + 1;
    unsigned int current_time = 0;
	struct timeval tv;
	NPD_DHCP_SNP_USER_ITEM_T lease;
    struct npd_dhcp_snp_timeout_s entry;
    fd_set rfds;
    int change_list_header = 0;
		
	/* tell my thread id */
	npd_init_tell_whoami("NpdDhcpSnp", 0);

	while (1)
	{
        if (!SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
            sleep(3);
            continue;
		}

        if (0 != npd_dhcp_snp_get_timeout(&entry))      /* judge timer-list is null ? */
        {
            FD_ZERO(&rfds);
            FD_SET(npd_dhcp_snp_self_sock, &rfds);
            
            ret = select(max_fd, &rfds, NULL, NULL, NULL);
            if (ret > 0)
            {
                if (FD_ISSET(npd_dhcp_snp_self_sock, &rfds))
                {
                    change_list_header = recv(npd_dhcp_snp_self_sock, &change_list_header, sizeof(int), 0);
                }
            }
            else
            {
                continue;
            }
        }

        current_time = npd_dhcp_snp_get_relative();
        if (0 == current_time)
        {
            continue;
        }

        if (0 != npd_dhcp_snp_get_timeout(&entry))
        {
            continue;
        }

        if (entry.expire_time > current_time)
        {
            FD_ZERO(&rfds);
            FD_SET(npd_dhcp_snp_self_sock, &rfds);
        
            tv.tv_sec = entry.expire_time - current_time;
            tv.tv_usec = 0;
            ret = select(max_fd, &rfds, NULL, NULL, &tv);
            if (ret > 0)
            {
                if (FD_ISSET(npd_dhcp_snp_self_sock, &rfds))
                {
                    change_list_header = recv(npd_dhcp_snp_self_sock, &change_list_header, sizeof(int), 0);
                }
            }
        }

        if (0 != npd_dhcp_snp_get_timeout(&entry))      /* maybe deleted ? */
        {
            continue;
        }

        current_time = npd_dhcp_snp_get_relative();     /* time change */
        if (0 == current_time)
        {
            continue;
        }
        
        while (entry.expire_time <= current_time)
        {
            memset(&lease, 0, sizeof(lease));
        	memcpy(&lease.chaddr, entry.hw_addr, sizeof(lease.chaddr));

        	if (DHCP_SNP_RETURN_CODE_OK == npd_dhcp_snp_tbl_item_find(&lease))
        	{
                if (!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        		{
                    break;
        		}
                ret = npd_dhcp_snp_tbl_item_delete(&lease);
        	}

            if (0 != npd_dhcp_snp_get_timeout(&entry))
            {
                break;
            }
        }
	}

    return 0;
}



/********************************************************************/

unsigned int npd_dhcp_snp_dbhash_index_mac_key  
(
	void *data
)
{
	NPD_DHCP_SNP_USER_ITEM_T *user;
	if(NULL == data ) {
		return FALSE;
	}
	user= (NPD_DHCP_SNP_USER_ITEM_T *)data;
		
    unsigned int h = NPD_DHCP_SNP_INIT_0;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	
	for(i = 0; i < 6; i++)
	{
		h = h + user->chaddr[i];
	}	
    h = (h) % NPD_DHCP_SNP_DBHASH_MAC_SIZE;
    return h;
}

/***************************************************************
***************************************************************/


unsigned int npd_dhcp_snp_dbhash_index_key 
(
	void * data
)
{
	NPD_DHCP_SNP_USER_ITEM_T *user;
	if(NULL == data ) {
		return FALSE;
	}
	user = (NPD_DHCP_SNP_USER_ITEM_T *)data;
	
	
   unsigned int h = NPD_DHCP_SNP_INIT_0;
	
	h = ((user->ifindex<<12)>>26) + ((user->vlanId<<26)>>20);
	
    h = (h) % NPD_DHCP_SNP_INDEX_SIZE;

    return h;
}

/**********************************************************************************
 * npd_dhcp_snooping_compare_byvlanId
 *
 * compare two of dhcp snooping database(Hash table) items
 *
 *	INPUT:
 *		itemA	- dhcp snooping database item
 *		itemB	- dhcp snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
int npd_dhcp_snooping_compare_byvlanId
(
	struct NPD_DHCP_SNP_USER_ITEM_S *itemA,
	struct NPD_DHCP_SNP_USER_ITEM_S *itemB
)
{
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_igmp_err("npd dhcp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->vlanId != itemB->vlanId) 
	{
		equal = FALSE;
	}
	
	return equal;

} 

/**********************************************************************************
 *npd_dhcp_snp_dbhash_mac_compare ()
 *
 *	DESCRIPTION:
 *		compare the mac
 *
 *	INPUTS:
 *		userA,userB
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		true - success
 *		false - fail
 ***********************************************************************************/

unsigned int npd_dhcp_snp_dbhash_mac_compare
(
	void * data1,
	void * data2	
)
{
	NPD_DHCP_SNP_USER_ITEM_T *userA;
	NPD_DHCP_SNP_USER_ITEM_T *userB;
	unsigned int equal = FALSE;
	
	if(NULL == data1 || NULL == data2 ) {
		return equal;
	}
	userA = (NPD_DHCP_SNP_USER_ITEM_T *)data1;
	userB = (NPD_DHCP_SNP_USER_ITEM_T *)data2;
	
	if((NULL==userA)||(NULL==userB)) {
		syslog_ax_dhcp_snp_err("npd arp snooping items compare null pointers error.");
		return equal;
	}
	
	if(0 == memcmp((char*)userA->chaddr,(char*)userB->chaddr,NPD_DHCP_SNP_MAC_ADD_LEN) )
	{
		equal = TRUE;
	}
	
	return equal;
}


int npd_dhcp_snp_dbhash_index_compare
(
	void * data1,
	void * data2	
)
{
	NPD_DHCP_SNP_USER_ITEM_T *itemA;
	NPD_DHCP_SNP_USER_ITEM_T *itemB;
	int equal = FALSE;
	
	if(NULL == data1 || NULL == data2 ) {
		return equal;
	}
	itemA = (NPD_DHCP_SNP_USER_ITEM_T *)data1;
	itemB = (NPD_DHCP_SNP_USER_ITEM_T *)data2;
	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_dhcp_snp_err("npd dhcp status items compare null pointers error.");
		return equal;
	}
	
	if(0 == memcmp((char*)itemA->chaddr,(char*)itemB->chaddr,MAC_ADDRESS_LEN))
	{
		equal = TRUE;
	}
	
	return equal;
}

long npd_dhcp_snp_handle_update(void * new_data, void * old_data)
{
    struct NPD_DHCP_SNP_USER_ITEM_S* new_entry = NULL;
    struct NPD_DHCP_SNP_USER_ITEM_S* old_entry = NULL;

    if (!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        return 0;
    }
    
    if (NULL == new_data || NULL == old_data)
    {
        return 1;
    }

    new_entry = (struct NPD_DHCP_SNP_USER_ITEM_S*)new_data;
    old_entry = (struct NPD_DHCP_SNP_USER_ITEM_S*)old_data;

    if (new_entry->lease_time != old_entry->lease_time)
    {
        if (old_entry->lease_time != (~0UL))
        {
            npd_dhcp_snp_cancel_timeout(old_entry->chaddr);
        }

        if (new_entry->lease_time != (~0UL))
        {
            npd_dhcp_snp_add_timeout(new_entry->lease_time + new_entry->sys_escape, new_entry->chaddr);
        }
    }

    return 0;
}

long npd_dhcp_snp_handle_insert(void * data)
{
    struct NPD_DHCP_SNP_USER_ITEM_S* entry = NULL;

    if (!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        return 0;
    }
    
    if (NULL == data)
    {
        return 1;
    }

    entry = (struct NPD_DHCP_SNP_USER_ITEM_S*)data;

    if (entry->lease_time != (~0UL))
    {
        npd_dhcp_snp_add_timeout(entry->lease_time + entry->sys_escape, entry->chaddr);
    }
    
    return 0;
}

long npd_dhcp_snp_handle_delete(void * data)
{
    struct NPD_DHCP_SNP_USER_ITEM_S* entry = NULL;

    if (!SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        return 0;
    }
    
    if (NULL == data)
    {
        return 1;
    }

    entry = (struct NPD_DHCP_SNP_USER_ITEM_S*)data;

    if (entry->lease_time != (~0UL))
    {
        npd_dhcp_snp_cancel_timeout(entry->chaddr);
    }
    
    return 0;
}

int npd_dhcp_snp_tbl_initialize()
{
	int ret;
	ret = create_dbtable( NPD_DHCP_SNP_DBTABLE_NAME, \
							NPD_DHCP_SNP_DBTABLE_SIZE, 
							sizeof(struct NPD_DHCP_SNP_USER_ITEM_S),
							npd_dhcp_snp_handle_update,
							NULL,
							npd_dhcp_snp_handle_insert, 
							npd_dhcp_snp_handle_delete,
							NULL, NULL, NULL,
							NULL, NULL, DB_SYNC_ALL,
							&(npd_dhcp_snp_dbtable));
	if( 0 != ret )
	{
		syslog_ax_dhcp_snp_err("create npd dhcp dbtable fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index( NPD_DHCP_SNP_DBHASH_INDEX_NAME_MAC, \
										npd_dhcp_snp_dbtable,
										NPD_DHCP_SNP_DBHASH_MAC_SIZE,
										npd_dhcp_snp_dbhash_index_mac_key,
										npd_dhcp_snp_dbhash_mac_compare,
										&npd_dhcp_snp_dbhash_mac_index);

	ret = dbtable_create_hash_index( NPD_DHCP_SNP_DBHASH_INDEX_NAME_PORT, \
										npd_dhcp_snp_dbtable,
										NPD_DHCP_SNP_INDEX_SIZE,
										npd_dhcp_snp_dbhash_index_key,
										npd_dhcp_snp_dbhash_mac_compare,
										&npd_dhcp_snp_dbhash_port_index);
	if( 0  != ret )
	{
		syslog_ax_dhcp_snp_err("create npd dhcp hash index fail\n");
		return NPD_FAIL;
	}	
	return DHCP_SNP_RETURN_CODE_OK;
}
/**********************************************************************************
 *npd_dhcp_snp_tbl_hash ()
 *
 *	DESCRIPTION:
 *		get the hash value according the user mac address
 *
 *	INPUTS:
 *		unsigned char *mac		- user mac address
 *
 *	OUTPUTS:
 *		h						- hash key value
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_tbl_hash  
(
	unsigned char *mac
)
{
    unsigned int h = NPD_DHCP_SNP_INIT_0;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	
	for(i = 0; i < 6; i++)
	{
		h = h + mac[i];
	}
	
    h = (h) % NPD_DHCP_SNP_HASH_TABLE_SIZE;

    return h;
}

void npd_dhcp_snp_dbtable_print_tbl
(
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{
	syslog_ax_dhcp_snp_dbg("=================dhcp-snooping bind table ===================\n");
	if (user != NULL)
	{
		/*scan all the bind table, age the expired users*/
		syslog_ax_dhcp_snp_dbg("MAC:%02x:%02x:%02x:%02x:%02x:%02x ", user->chaddr[0], 
			user->chaddr[1], user->chaddr[2],user->chaddr[3],user->chaddr[4],user->chaddr[5]);
		syslog_ax_dhcp_snp_dbg("ip:%08x ", user->ip_addr);
		syslog_ax_dhcp_snp_dbg("vlan:%04d ", user->vlanId);
		syslog_ax_dhcp_snp_dbg("port:%08d ", user->ifindex);
		syslog_ax_dhcp_snp_dbg("systime:%08d ", user->sys_escape);
		syslog_ax_dhcp_snp_dbg("lease:%08d ", user->lease_time);
		syslog_ax_dhcp_snp_dbg("bind_type:%04d ", user->bind_type);
		syslog_ax_dhcp_snp_dbg("state:%04d ", user->state);
		syslog_ax_dhcp_snp_dbg("cur_expire:%08d ", user->cur_expire);
		syslog_ax_dhcp_snp_dbg("flags:%08d \n", user->flags);
		syslog_ax_dhcp_snp_dbg("-------------------------------------------------------------\n");
	}
	syslog_ax_dhcp_snp_dbg("=============================================================\n");
	return ;
}

int npd_dhcp_snp_delete_dynamic_binding
(
	hash_table_index_t *hash,
	void * data	,
	unsigned int flag
)
{
	NPD_DHCP_SNP_USER_ITEM_T *user;
    
	if (NULL == data)
	{
		return FALSE;
	}
	user = (NPD_DHCP_SNP_USER_ITEM_T *)data;

    if (NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == user->bind_type)
	{
        npd_dhcp_snp_tbl_item_delete(user);
	}
    
    return 0;
}

unsigned int npd_dhcp_snp_tbl_destroy
(
	void
)
{
    dbtable_hash_traversal(npd_dhcp_snp_dbhash_mac_index,1,NULL,NULL,npd_dhcp_snp_delete_dynamic_binding);
    syslog_ax_dhcp_snp_dbg("destroy the table success! \n");

    return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 *npd_dhcp_snp_tbl_item_find ()
 *
 *	DESCRIPTION:
 *		Get the item of specifical user
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_tbl_item_find	
(
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{
	syslog_ax_dhcp_snp_dbg("find item from dhcp snooping hash table.\n");
	int status = 0;
	status = dbtable_hash_search(npd_dhcp_snp_dbhash_mac_index, user, NULL, user);
	if(status == 0)
	{
		syslog_ax_dhcp_snp_dbg("found item from dhcp snooping hash table, success.\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}
	syslog_ax_dhcp_snp_err("no found the special entry\n");
	return DHCP_SNP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 *npd_dhcp_snp_tbl_item_insert ()
 *
 *	DESCRIPTION:
 *		insert the user bind information into the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/


unsigned int npd_dhcp_snp_tbl_item_insert
(
	NPD_DHCP_SNP_USER_ITEM_T *user	
)
{
	int status = 0;
	syslog_ax_dhcp_snp_dbg("insert item into dhcp snooping hash table.\n");		  

	if (user == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp insert item in bind table error, parameter is null\n");
		return 0;
	}
    
//  user->sys_escape = time(0);
    user->sys_escape = npd_dhcp_snp_get_relative();

    /* XXX: for dhcp-snp source guaid */
    if (NPD_DHCP_SNP_BIND_STATE_BOUND == user->state)
    {
        if (NPD_IFINDEX_TYPE(user->ifindex) == NPD_NETIF_ETH_TYPE)
        {
            struct eth_port_s *eth_port = NULL;
            npd_key_database_lock();
            eth_port = npd_get_port_by_index(user->ifindex);

            if (NULL == eth_port)
            {
				npd_key_database_unlock();
                return DHCP_SNP_RETURN_CODE_ERROR;
            }

            if (eth_port->ip_sg)
            {
                user->have_sg = 1;
                npd_source_guard_entry_add(user->ip_addr, user->ifindex, user->vlanId, user->chaddr, SOURCE_GUARD_DYNAMIC);
            }
			npd_key_database_unlock();
            free(eth_port);
        }
    }
    status = dbtable_hash_insert(npd_dhcp_snp_dbhash_mac_index,user);
	if( 0 != status )
	{
		syslog_ax_dhcp_snp_err("dhcp snooping table insert error\n");		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
    /* for debug */
	npd_dhcp_snp_dbtable_print_tbl(user);
	syslog_ax_dhcp_snp_dbg("insert item into dhcp snooping hash table, success.\n");

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npe_dhcp_snp_tbl_item_delete ()
 *
 *	DESCRIPTION:
 *		delete the user bind item from the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *	
 ***********************************************************************************/

unsigned int npd_dhcp_snp_tbl_item_delete
(
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{
	int status = 0;
	syslog_ax_dhcp_snp_dbg("delete item from dhcp snooping hash table, success.\n");

	if (user  == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    /* XXX: for dhcp-snp source guaid */
    if (0 != user->have_sg)
    {
        npd_source_guard_entry_del(user->ip_addr, user->ifindex, SOURCE_GUARD_DYNAMIC);
        user->have_sg = 0;
        dbtable_hash_update(npd_dhcp_snp_dbhash_mac_index, NULL, user); 
    }

    if (NPD_DHCP_SNP_REQUEST_TIMEOUT_NAK == user->lease_time)
    {
        npd_dhcp_snp_send_nak_to_client(user);
    }
            
	status = dbtable_hash_delete(npd_dhcp_snp_dbhash_mac_index,user,NULL);
	if(0 == status) 
	{
		return DHCP_SNP_RETURN_CODE_OK;
	}
	syslog_ax_dhcp_snp_warning("no found the special entry, delete fail\n");	
	return DHCP_SNP_RETURN_CODE_ERROR;
}

unsigned int npd_dhcp_snp_tbl_identity_item
(
	NPD_DHCP_SNP_USER_ITEM_T *userA,
	NPD_DHCP_SNP_USER_ITEM_T *userB
)
{
	syslog_ax_dhcp_snp_dbg("identity item in dhcp snooping hash table.\n");		  

    if ((userA == NULL) || (userB  == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp identity bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    if ((userA->vlanId != userB->vlanId) ||(userA->haddr_len != userB->haddr_len))
    {
        syslog_ax_dhcp_snp_err("vid is not the same\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }
	
	if ((userB->ifindex != 0) && (userA->ifindex != userB->ifindex)) {
    /*if (item->ifindex != user->ifindex) {*/
		syslog_ax_dhcp_snp_err("have different ifindex\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

    if (memcmp(userA->chaddr, userB->chaddr, userA->haddr_len) != 0) {
		syslog_ax_dhcp_snp_err("have different chaddr value\n");        
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

	syslog_ax_dhcp_snp_dbg("identity item in dhcp snooping hash table, success.\n");		  
    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *npd_dhcp_snp_tbl_refresh_bind ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_tbl_refresh_bind
(
	NPD_DHCP_SNP_USER_ITEM_T *preuser,
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T tempUser;

	syslog_ax_dhcp_snp_dbg("refresh item in dhcp snooping hash table.\n");		  

    if ((user == NULL) || (preuser  == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp refresh item in bind table error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
/*	user->sys_escape = time(0);     */
    user->sys_escape = npd_dhcp_snp_get_relative();
	syslog_ax_dhcp_snp_dbg("systime %d \n",user->sys_escape);		  

	if( 0 == dbtable_hash_search(npd_dhcp_snp_dbhash_mac_index, preuser, NULL, &tempUser) )
	{
    	if (user->ifindex == 0)
    	{
            user->ifindex = tempUser.ifindex;
    	}
        user->vlanId = tempUser.vlanId;

        /* XXX: for dhcp-snp source guaid */
        if (NPD_DHCP_SNP_BIND_STATE_BOUND == user->state)
        {
            if (NPD_IFINDEX_TYPE(user->ifindex) == NPD_NETIF_ETH_TYPE)
            {
                struct eth_port_s *eth_port = NULL;

				npd_key_database_lock();
                eth_port = npd_get_port_by_index(user->ifindex);
            
                if (NULL == eth_port)
                {
					npd_key_database_unlock();
                    return DHCP_SNP_RETURN_CODE_ERROR;
                }
            
                if (eth_port->ip_sg)
                {
                    user->have_sg = 1;
                    npd_source_guard_entry_add(user->ip_addr, user->ifindex, user->vlanId, user->chaddr, SOURCE_GUARD_DYNAMIC);
                }
				npd_key_database_unlock();
                free(eth_port);
            }
        }
		ret = dbtable_hash_update(npd_dhcp_snp_dbhash_mac_index, preuser, user);
	}
#if 0
	else
	{
		ret = dbtable_hash_insert(npd_dhcp_snp_dbhash_mac_index, user);
	}
#endif
	
	if (0 == ret) {
		syslog_ax_dhcp_snp_dbg("refresh item in dhcp snooping hash table, success.\n"); 	  
		return DHCP_SNP_RETURN_CODE_OK;
	}

 	return DHCP_SNP_RETURN_CODE_ERROR;
}

unsigned int npd_dhcp_snp_bind_show
(
	struct NPD_DHCP_SNP_USER_ITEM_S *user
)
{
	if(user->chaddr[0] == 0&&
		user->chaddr[1] == 0&&
		user->chaddr[2] == 0&&
		user->chaddr[3] == 0&&
		user->chaddr[4] == 0&&
		user->chaddr[5] == 0
	)
	{
		return dbtable_hash_head(npd_dhcp_snp_dbhash_mac_index, NULL, user, NULL);
	}
	else
	{
		return dbtable_hash_next(npd_dhcp_snp_dbhash_mac_index, user, user, NULL);
	}
}

/**********************************************************************************
 *npd_dhcp_snp_tbl_static_binding_insert()
 *
 *	DESCRIPTION:
 *		insert a static binding item to dhcp snp db
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need insert into dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR		- exec fail
 ***********************************************************************************/
unsigned int npd_dhcp_snp_tbl_static_binding_insert
(
	NPD_DHCP_SNP_USER_ITEM_T user
)

{
	return (npd_dhcp_snp_tbl_item_insert(&user));
}

/**********************************************************************************
 *npd_dhcp_snp_tbl_static_binding_delete()
 *
 *	DESCRIPTION:
 *		delete a static binding item from dhcp snp db
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need delete from dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR		- exec fail
 ***********************************************************************************/
unsigned int npd_dhcp_snp_tbl_static_binding_delete
(
	NPD_DHCP_SNP_USER_ITEM_T user
)
{
	return npd_dhcp_snp_tbl_item_delete(&user);
}

unsigned int npd_dhcp_snp_status_item_key 
(
	void *data 

)
{
    unsigned int h = NPD_DHCP_SNP_INIT_0;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *itemA = (struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *)data;
	
	h = ((itemA->global_port_ifindex<<12)>>26);
	
    h = (h) % NPD_DHCP_SNP_INDEX_SIZE;

    return h;
}

unsigned int npd_dhcp_snp_status_item_hash_compare
(
	void * data1,
	void * data2	
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *itemA;
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *itemB;
	unsigned int equal = TRUE;


	if(NULL == data1 || NULL == data2 ) {
		return FALSE;
	}
	itemA = (struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *)data1;
	itemB = (struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *)data2;
	
	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_dhcp_snp_err("npd dhcp status items compare null pointers error.");
		return FALSE;
	}
	if(itemA->global_port_ifindex == itemB->global_port_ifindex)
	{
		equal = TRUE;
	}
	else 
	{ 
		equal = FALSE;
	}

	return equal;
}


/**********************************************************************************
 *npd_dhcp_snp_status_item_table_initialize ()
 *
 *	DESCRIPTION:
 *		npd_dhcp_snp_status_item_table_initialize
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK - success
 *
 ***********************************************************************************/


int npd_dhcp_snp_status_item_table_initialize()
{
	unsigned int ret;
	ret = create_dbtable( NPD_DHCP_SNP_STATUS_ITEM_NAME, \
							NPD_DHCP_SNP_STATUS_ITEM_SIZE, 
							sizeof(struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT),
							NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							NULL, NULL, DB_SYNC_ALL,
							&(npd_dhcp_snp_status_item));
	if( 0 != ret )
	{
		syslog_ax_dhcp_snp_err("create npd dhcp dbtable fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index( NPD_DHCP_SNP_STATUS_ITEM_INDEX_NAME, \
										npd_dhcp_snp_status_item,
										NPD_DHCP_SNP_INDEX_SIZE,
										 npd_dhcp_snp_status_item_key,
										 npd_dhcp_snp_status_item_hash_compare,
										&npd_dhcp_snp_status_item_index);
	if( 0  != ret )
	{
	syslog_ax_dhcp_snp_err("create  dhcp status item fail\n");
	return NPD_FAIL;
	}	

	return DHCP_SNP_RETURN_CODE_OK;
}

int npd_dhcp_snp_switchcontrol_port(struct NPD_DHCP_SNP_GLOBAL_STATUS * entry, int flag)
{
    unsigned short vlan_id = 0;
    int array_index = 0;
    unsigned int netif_index = 0;

    for (array_index = 0; array_index < MAX_ETHPORT_PER_SYSTEM; array_index++)
    {
        if (entry->switch_port_control_count[array_index])
        {
            netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
            /* XXX:vlan id isn't useful, but cann't be 1(default vlan) */
            npd_port_dhcp_trap_set(0, netif_index , flag);
        }
    }

    NPD_VBMP_ITER(entry->vlan_admin_status, vlan_id)
    {
        nam_set_UdpBcPkt_enable(vlan_id, flag);
    }
   
    return 0;
}

long npd_dhcp_snp_global_sta_update(void * new, void * old)
{
    unsigned short vlan_id = 0;
    unsigned int netif_index = 0;
    int array_index = 0;
    struct NPD_DHCP_SNP_GLOBAL_STATUS * new_entry = NULL;
    struct NPD_DHCP_SNP_GLOBAL_STATUS * old_entry = NULL;
    npd_vbmp_t vlanbitmap;

    if (NULL == new || NULL == old)
    {
        return 0;
    }
    else
    {
        new_entry = (struct NPD_DHCP_SNP_GLOBAL_STATUS *)new;
        old_entry = (struct NPD_DHCP_SNP_GLOBAL_STATUS *)old;
    }

    if (new_entry->dhcp_snp_enable != old_entry->dhcp_snp_enable)
    {
        npd_dhcp_packet_enable(new_entry->dhcp_snp_enable);
        if (new_entry->dhcp_snp_enable)
        {
            npd_dhcp_snp_switchcontrol_port(new_entry, TRUE);
        }
        else
        {
            npd_dhcp_snp_switchcontrol_port(new_entry, FALSE);
        }
    }
    else if (new_entry->dhcp_snp_enable)
    {
        for (array_index = 0; array_index < MAX_ETHPORT_PER_SYSTEM; array_index++)
        {
            if ((0 != new_entry->switch_port_control_count[array_index])
                && (0 == old_entry->switch_port_control_count[array_index]))
            {
                netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
                /* XXX:vlan id isn't useful, but cann't be 1(default vlan) */
                npd_port_dhcp_trap_set(0, netif_index , TRUE);
            }
            else if ((0 == new_entry->switch_port_control_count[array_index])
                && (0 != old_entry->switch_port_control_count[array_index]))
            {
                netif_index = (unsigned int)netif_array_index_to_ifindex(array_index);
                /* XXX:vlan id isn't useful, but cann't be 1(default vlan) */
                npd_port_dhcp_trap_set(0, netif_index , FALSE);
            }
        }

        NPD_VBMP_ASSIGN(vlanbitmap, old_entry->vlan_admin_status);
        NPD_VBMP_XOR(vlanbitmap, new_entry->vlan_admin_status);

        NPD_VBMP_ITER(vlanbitmap, vlan_id)
        {
            if (NPD_VBMP_MEMBER(new_entry->vlan_admin_status, vlan_id))
            {
                nam_set_UdpBcPkt_enable(vlan_id, TRUE);
            }
            else
            {
                nam_set_UdpBcPkt_enable(vlan_id, FALSE);
            }
        }
    }

    return 0;
}

long npd_dhcp_snp_global_sta_insert(void * new)
{
    struct NPD_DHCP_SNP_GLOBAL_STATUS *item = NULL;

    if (NULL == new)
    {
        return 0;
    }
    else
    {
        item = (struct NPD_DHCP_SNP_GLOBAL_STATUS *)new;
    }
    
    return npd_dhcp_packet_enable(item->dhcp_snp_enable);
}

int npd_dhcp_snp_global_status_initialize()
{
	unsigned int ret;
	ret = create_dbtable( NPD_DHCP_SNP_GLOBAL_STATUS_NAME, \
							NPD_DHCP_SNP_GLOBAL_STATUS_SIZE, 
							sizeof(struct NPD_DHCP_SNP_GLOBAL_STATUS),
							npd_dhcp_snp_global_sta_update, 
							NULL,
							npd_dhcp_snp_global_sta_insert, 
							NULL, 
							NULL,
							NULL, NULL, 
							NULL, NULL, DB_SYNC_ALL,
							&(npd_dhcp_snp_global_status_table));
	if( 0 != ret )
	{
		syslog_ax_dhcp_snp_err("create npd dhcp global dbtable fail\n");
		return NPD_FAIL;
	}


    ret = dbtable_create_array_index(NPD_DHCP_SNP_GLOBAL_STATUS_INDEX_NAME, 
							npd_dhcp_snp_global_status_table,  
							&npd_dhcp_snp_global_status_index);


	if( 0  != ret )
	{
		syslog_ax_dhcp_snp_err("create  dhcp status item fail\n");
		return NPD_FAIL;
	}	
	
	return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 *npd_dhcp_snp_status_item_insert ()
 *
 *	DESCRIPTION:
 *		insert the status item bind information into the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM *item
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/


unsigned int npd_dhcp_snp_status_item_insert
(  
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item	
)
{
	unsigned int status;
	syslog_ax_dhcp_snp_dbg("insert item into dhcp status item table.\n");		  

	if (item == NULL)
	{
		syslog_ax_dhcp_snp_err("dhcp snp insert item in status item table error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	syslog_ax_dhcp_snp_dbg("ifindex = %d,trust_mode = %d\n", item->global_port_ifindex,item->trust_mode);
    
	status = dbtable_hash_insert(npd_dhcp_snp_status_item_index,item);
	if (0 != status)
	{
		syslog_ax_dhcp_snp_err("dhcp snp table fill error, item %x\n", item);		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	/* for debug */
	syslog_ax_dhcp_snp_dbg("insert item into dhcp snp status item table, success.\n");		  
	return DHCP_SNP_RETURN_CODE_OK;
}




/**********************************************************************************
 *npd_dhcp_snp_status_item_delete ()
 *
 *	DESCRIPTION:
 *		delete the item bind from the  status item table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *	
 ***********************************************************************************/

unsigned int npd_dhcp_snp_status_item_delete
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
)
{
	if (item  == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
	if(!dbtable_hash_delete(npd_dhcp_snp_status_item_index,item,NULL))
		return DHCP_SNP_RETURN_CODE_OK;
	
	syslog_ax_dhcp_snp_warning("no found the special entry, delete fail\n");	
	return DHCP_SNP_RETURN_CODE_ERROR;
}





/**********************************************************************************
 *npd_dhcp_snp_status_item_find ()
 *
 *	DESCRIPTION:
 *		Get the item of specifical item
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_status_item_find	
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
)
{
	unsigned int ret;
	ret = dbtable_hash_search(npd_dhcp_snp_status_item_index, item, NULL, item);
	if (ret == 0)
	{
		syslog_ax_dhcp_snp_dbg("found item success\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}
	syslog_ax_dhcp_snp_err("no found the special entry\n");
	return DHCP_SNP_RETURN_CODE_ERROR;
}

/**********************************************************************************
 *npd_dhcp_snp_status_item_destroy ()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping bind table momery
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK - success
 *
 ***********************************************************************************/
int npd_dhcp_snp_status_item_travel_del(hash_table_index_t *index, void *item, unsigned int flag)
{
    return dbtable_hash_delete(index, item, NULL);
}

unsigned int npd_dhcp_snp_status_item_destroy
(
	void
)
{
	dbtable_hash_traversal(npd_dhcp_snp_status_item_index,0,NULL,NULL,npd_dhcp_snp_status_item_travel_del);		
	syslog_ax_dhcp_snp_dbg("destroy the table success! \n");
    return DHCP_SNP_RETURN_CODE_OK;

}

/**********************************************************************************
 *npd_dhcp_snp_status_item_refresh ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
 *
 *	OUTPUTS:
 *		
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *
 ***********************************************************************************/
unsigned int npd_dhcp_snp_status_item_refresh
(
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *preitem,
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT *item
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	syslog_ax_dhcp_snp_dbg("refresh item in dhcp snp status item table.\n");		  

    if ((item == NULL) || (preitem  == NULL)) {
		syslog_ax_dhcp_snp_err("refresh item in dhcp snp status item table error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
	ret = dbtable_hash_update(npd_dhcp_snp_status_item_index, preitem, item);
	if (0 != ret) {
		syslog_ax_dhcp_snp_err("delete dhcp snp status item table error, ret %x\n", ret);			
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("refresh item in dhcp snp status item table, success.\n"); 	  

	return DHCP_SNP_RETURN_CODE_OK;
}

unsigned int npd_dhcp_snp_lease_delete_by_port
(
	unsigned int ifindex,
	unsigned char flag
)
{
	int ret = 0;
    struct NPD_DHCP_SNP_USER_ITEM_S leaseEntry, nextEntry;;

	memset(&leaseEntry, 0, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	
	ret = dbtable_hash_head(npd_dhcp_snp_dbhash_port_index, NULL, &leaseEntry, NULL);
	while(0 == ret)
	{
		ret = dbtable_hash_next(npd_dhcp_snp_dbhash_port_index, &leaseEntry, &nextEntry, NULL);
        if ((leaseEntry.ifindex == ifindex)
            && (leaseEntry.bind_type == flag))
		{
            npd_dhcp_snp_tbl_item_delete(&leaseEntry);
		}
		memcpy(&leaseEntry, &nextEntry, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	}

    return 0;
}

unsigned int npd_dhcp_snp_lease_delete_by_vlan
(
	unsigned short vid,
	unsigned char flag
)
{
	int ret = 0;
    struct NPD_DHCP_SNP_USER_ITEM_S leaseEntry, nextEntry;;

	memset(&leaseEntry, 0, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	
	ret = dbtable_hash_head(npd_dhcp_snp_dbhash_port_index, NULL, &leaseEntry, NULL);
	while(0 == ret)
	{
		ret = dbtable_hash_next(npd_dhcp_snp_dbhash_port_index, &leaseEntry, &nextEntry, NULL);
        
		if ((leaseEntry.vlanId == vid)
            && (leaseEntry.bind_type == flag))
		{
            npd_dhcp_snp_tbl_item_delete(&leaseEntry);
		}
		memcpy(&leaseEntry, &nextEntry, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	}

    return 0;
}

unsigned int npd_dhcp_snp_lease_delete_by_vlan_port
(
	unsigned short vid,
	unsigned int ifindex,
	unsigned char flag
)
{
	int ret = 0;
    struct NPD_DHCP_SNP_USER_ITEM_S leaseEntry, nextEntry;;

	memset(&leaseEntry, 0, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
    leaseEntry.ifindex = ifindex;
	leaseEntry.vlanId = vid;
	
	ret = dbtable_hash_head_key(npd_dhcp_snp_dbhash_port_index, &leaseEntry, &leaseEntry, NULL);
	while(0 == ret)
	{
		ret = dbtable_hash_next_key(npd_dhcp_snp_dbhash_port_index, &leaseEntry, &nextEntry, NULL);
        if ((leaseEntry.vlanId == vid)
            && (leaseEntry.ifindex == ifindex)
            && (leaseEntry.bind_type == flag))
		{
            npd_dhcp_snp_tbl_item_delete(&leaseEntry);
		}
		memcpy(&leaseEntry, &nextEntry, sizeof(struct NPD_DHCP_SNP_USER_ITEM_S));
	}

    return 0;
}

unsigned int npd_dhcp_snp_get_trust_mode
(
	unsigned short vid,
	unsigned int ifindex,
	unsigned int *trust_mode
)
{
	struct NPD_DHCP_SNP_STATUS_ITEM_STRUCT item;
	memset(&item,0,sizeof(item));
	item.global_port_ifindex = ifindex;

	if(!dbtable_hash_search(npd_dhcp_snp_status_item_index,&item,NULL,&item))
	{
		*trust_mode = item.trust_mode;
		syslog_ax_dhcp_snp_dbg("get trust mode %d",item.trust_mode);
		return DHCP_SNP_RETURN_CODE_OK;
	}
	syslog_ax_dhcp_snp_dbg("get trust mode faile! trust_mode = %d",*trust_mode);

	return DHCP_SNP_RETURN_CODE_ERROR;
}




#ifdef __cplusplus
}
#endif
#endif

